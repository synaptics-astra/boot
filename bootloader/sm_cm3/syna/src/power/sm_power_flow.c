// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013~2023 Synaptics Incorporated. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 or
 * later as published by the Free Software Foundation.
 *
 * INFORMATION CONTAINED IN THIS DOCUMENT IS PROVIDED "AS-IS," AND
 * SYNAPTICS EXPRESSLY DISCLAIMS ALL EXPRESS AND IMPLIED WARRANTIES,
 * INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE, AND ANY WARRANTIES OF NON-INFRINGEMENT OF ANY
 * INTELLECTUAL PROPERTY RIGHTS. IN NO EVENT SHALL SYNAPTICS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, PUNITIVE, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OF THE INFORMATION CONTAINED IN THIS DOCUMENT, HOWEVER CAUSED AND
 * BASED ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, AND EVEN IF SYNAPTICS WAS
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. IF A TRIBUNAL OF
 * COMPETENT JURISDICTION DOES NOT PERMIT THE DISCLAIMER OF DIRECT
 * DAMAGES OR ANY OTHER DAMAGES, SYNAPTICS' TOTAL CUMULATIVE LIABILITY
 * TO ANY PARTY SHALL NOT EXCEED ONE HUNDRED U.S. DOLLARS.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "sm_type.h"
#include "sm_io.h"
#include "SysMgr.h"
#include "sm_common.h"
#include "sm_rt_module.h"

#include "platform_config.h"

#include "sm_comm.h"

#include "sm_state.h"
#include "sm_printf.h"
#include "sm_power.h"
#include "sm_timer.h"
#if (defined(SM_RAM_TS_ENABLE) || defined(SM_RAM_PARAM_ENABLE))
#include "sm_mem_map_itcm.h"
#include "ram_ts.h"
#endif

BOOL bFastboot = FALSE;
BOOL bFastboot_Standby = FALSE;
BOOL bRequest_Poweron = FALSE;

/*-------------------------------*/

#define POWEROFF2POWERON 300
#define POWERON2OKTIME 60
#define ASSERT2DEASSERTTIME 100 //this time should be longer

#define REBOOTFLAG_MAGIC		(0xF5F5F5F5)
#define RTS_KEY_BOOTREASON		"boot_reason"

typedef struct {
        unsigned int magic;
        unsigned int flag;
}rebootflag_t;

static UINT32 power_mystatus = STA_INVALID;
static UINT32 power_myflow = FLOW_IDLE;

const static char* flow_name[] = {
	"---Active to Lowpower Standby---",
	"---Active to Normal Standby---",
	"---Active to Suspend---",

	"---Lowpower Standby to Active---",
	"---Normal Standby to Active---",
	"---Suspend to Active---",

	"---Sysreset to Lowpower Standby---",
	"---Sysreset to Normal Standby---",
	"---Sysreset to Active---",

	"---Cold to Lowpower Standby---",

	"---Idle---",
};

static unsigned long power_lasttime = 0;
static BOOL is_get_rsp = FALSE;

static BOOL is_linux_ready = FALSE;

static MSG_LINUX_2_SM standby_type = DEFAULT_STANDBY_REQUEST_TYPE;
static INT32 suspend_type = MV_SM_POWER_SUSPEND_TYPE_PWROFF;

volatile UINT32	linux_resume_addr = 0xFFFFFFFF;

volatile struct wakeup_info wi = {MV_SM_WAKEUP_SOURCE_INVALID, {0, 0}};

static inline BOOL waittime(INT32 interval)
{
	INT32 curr = mv_sm_timer_gettimems();
	if((curr - power_lasttime) >= interval) {
		power_lasttime = curr;
		return TRUE;
	}
	return FALSE;
}

BOOL mv_sm_get_linux_state(void)
{
	return is_linux_ready;
}

/* broadcast flow type and status. */
static void mv_sm_power_broadcast_power_state()
{
	UINT32 msg[3];

	msg[0] = MV_SM_ID_POWER;
	msg[1] = power_myflow;
	msg[2] = power_mystatus;

	sm_send_internal_msg(BOARDCASTMSG, (unsigned char *)msg, sizeof(msg));
}

hresult mv_sm_power_enterflow(SM_POWER_FLOW flow)
{
	if((power_myflow != FLOW_IDLE) || (power_mystatus != STA_INVALID)) {
		PRT_ERROR("Last flow didn't finish. flow = %d, status = %d\n",
			power_myflow, power_mystatus);
		return S_FALSE;
	}

	PRT_INFO("******enter flow %s\n", flow_name[flow]);

	is_linux_ready = FALSE;
	power_mystatus = STA_ENTERFLOW;
	power_myflow = flow;
	power_lasttime = mv_sm_timer_gettimems();
	return S_OK;
}

hresult mv_sm_power_enterflow_bysmstate()
{
	INT32 state = mv_sm_get_state();
	SM_POWER_FLOW flow = FLOW_IDLE;
	switch(state) {
	case MV_SM_STATE_LOWPOWERSTANDBY:
		flow = FLOW_LOWPOWERSTANDBY_2_ACTIVE;
		break;
	case MV_SM_STATE_NORMALSTANDBY:
		flow = FLOW_NORMALSTANDBY_2_ACTIVE;
		break;
	case MV_SM_STATE_SUSPEND:
		flow = FLOW_SUSPEND_2_ACTIVE;
		break;
	default:
		PRT_INFO("don't support 0x%x\n", state);
		break;
	}
	return mv_sm_power_enterflow(flow);
}


void mv_sm_power_leaveflow(void)
{
	PRT_INFO("******leave flow %s\n", flow_name[power_myflow]);
	mv_sm_power_broadcast_power_state();
	power_mystatus = STA_INVALID;
	power_myflow = FLOW_IDLE;
}

static hresult mv_sm_power_soc_poweron_2_powerok()
{
	switch(power_mystatus) {
	case STA_POWERON:
		if(waittime(POWEROFF2POWERON)) {
			mv_sm_power_broadcast_power_state();
			mv_sm_power_socpoweron();
			power_mystatus = STA_POWEROK;
		}
		break;
	case STA_POWEROK:
		if(waittime(POWERON2OKTIME)) {
			if(mv_sm_power_soc_power_status() != S_OK) {
				/* reset time and wait again. */
				power_lasttime = mv_sm_timer_gettimems();
			} else {
				mv_sm_power_broadcast_power_state();
				soc_deassert();
				power_mystatus = STA_WAITRSP;
			}
		}
		break;
	default:
		PRT_ERROR("should not come here. flow = %d, status = %d\n",
			power_myflow, power_mystatus);
		return S_FALSE;
	}
	return S_OK;
}

static hresult mv_sm_power_soc_assert_2_deassert()
{
	hresult ret = S_FALSE;
	switch(power_mystatus) {
	case STA_ASSERT:
		mv_sm_power_broadcast_power_state();
		peripheral_assert();
		soc_assert();
		power_mystatus = STA_DEASSERT;
		break;
	case STA_DEASSERT:
		/* hold assert 100ms */
		if(waittime(ASSERT2DEASSERTTIME)) {
			peripheral_deassert();
			mv_sm_power_broadcast_power_state();
			power_mystatus = STA_POWERON;

			//timer_id = mv_sm_timer_request();
		}
		break;
	case STA_WAITRSP:
		/* just loop here to wait rsp msg. do nothing */
		if(is_get_rsp) {
			is_get_rsp = FALSE;
			ret = S_OK;
		}
		break;
	default:
		PRT_ERROR("should not come here. flow = %d, status = %d\n",
			power_myflow, power_mystatus);
		break;
	}
	return ret;
}

/* FIXME: how to trace */

/* there are three different warmdown type
  * 1. lowpower standby; 2. normal standby; 3. suspend
*/
static hresult mv_sm_power_flow_warmdown(SM_POWER_FLOW flow)
{
	T32smSysCtl_SM_CTRL reg;

	if(flow != power_myflow) {
		PRT_ERROR("flow doens't match. %d != %d\n", flow, power_myflow);
		power_myflow = FLOW_IDLE;
		return S_FALSE;
	}

	switch(power_mystatus) {
	case STA_ENTERFLOW:
		//FIXME: clear SM to SoC interrupt to avoid corner scenary, especially Suspend Mode
		reg.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_CTRL);
		reg.uSM_CTRL_SM2SOC_SW_INTR = 0;
		MV_SM_WRITE_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_CTRL,reg.u32);

		#ifdef GETHENABLE
		geth_wakeup= FALSE;
		#endif
		#ifdef FPSTBYENABLE
		fpbutton_wakeup = FALSE;
		#endif

		mv_sm_power_broadcast_power_state();
		switch(flow) {
		case FLOW_ACTIVE_2_LOWPOWERSTANDBY:
			mv_sm_set_state(MV_SM_STATE_ACTIVE_2_LOWPOWERSTANDBY);
			/* FIXME */
			//mv_sm_set_boot_flag(MV_SOC_STATE_ACTIVE_2_LOWPOWERSTANDBY);
			mv_sm_set_boot_flag(MV_SOC_STATE_ACTIVE_2_NORMALSTANDBY);
			//set_linux_startaddr(0xFFFFFFFF);
			power_mystatus = STA_POWEROFF;
			break;
		case FLOW_ACTIVE_2_NORMALSTANDBY:
			mv_sm_set_state(MV_SM_STATE_ACTIVE_2_NORMALSTANDBY);
			mv_sm_set_boot_flag(MV_SOC_STATE_ACTIVE_2_NORMALSTANDBY);
			//set_linux_startaddr(0xFFFFFFFF);
			power_mystatus = STA_POWEROFF;
			break;
		case FLOW_ACTIVE_2_SUSPEND:
			mv_sm_set_state(MV_SM_STATE_ACTIVE_2_SUSPEND);
			mv_sm_set_boot_flag(MV_SOC_STATE_SUSPEND_REQUEST);

			if (suspend_type == MV_SM_POWER_SUSPEND_TYPE_WFI) {
				/* no power operations in WFI enabled case */
				power_mystatus = STA_LEAVEFLOW;
			} else {
				/*
				 * Assert & Deassert are skipped in suspend flow,
				 * since they would make SoC to do some re-init, such
				 * as memory controller, it could result in data loss.
				 */
				power_mystatus = STA_WAITRSP;
			}
			break;
		default:
			break;
		}
		break;
	case STA_ASSERT:
	case STA_DEASSERT:
	case STA_WAITRSP:
		if(S_OK == mv_sm_power_soc_assert_2_deassert()) {
			power_mystatus = STA_POWEROFF;
		}
		break;
	case STA_POWEROFF:
		mv_sm_power_socpoweroff();
		power_mystatus = STA_LEAVEFLOW;
		break;
	case STA_LEAVEFLOW:
		switch(power_myflow) {
		case FLOW_ACTIVE_2_LOWPOWERSTANDBY:
			mv_sm_set_state(MV_SM_STATE_LOWPOWERSTANDBY);
			break;
		case FLOW_ACTIVE_2_NORMALSTANDBY:
			mv_sm_set_state(MV_SM_STATE_NORMALSTANDBY);
			break;
		case FLOW_ACTIVE_2_SUSPEND:
			mv_sm_set_state(MV_SM_STATE_SUSPEND);
			break;
		default:
			PRT_ERROR("should not come here. flow = %d\n",
				power_myflow);
			break;
		}
		mv_sm_power_leaveflow();
		break;
	default:
		PRT_ERROR("should not come here. flow = %d, status = %d\n",
			power_myflow, power_mystatus);
		break;
	}

	return S_OK;
}

/* by default, cold down will turn into lowpower standby */
static hresult mv_sm_power_flow_colddown(SM_POWER_FLOW flow)
{
	if(flow != power_myflow) {
		PRT_ERROR("flow doens't match. %d != %d\n", flow, power_myflow);
		power_myflow = FLOW_IDLE;
		return S_FALSE;
	}

	switch(power_mystatus) {
	case STA_ENTERFLOW:
		mv_sm_power_broadcast_power_state();
		power_mystatus = STA_POWEROFF;
		break;
	case STA_POWEROFF:
		mv_sm_power_socpoweroff();
		power_mystatus = STA_LEAVEFLOW;
		break;
	case STA_LEAVEFLOW:
		mv_sm_set_state(MV_SM_STATE_LOWPOWERSTANDBY);
		mv_sm_power_leaveflow();
		if (bRequest_Poweron) {
			PRT_DEBUG("Enter active soon\n");
			mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_INVALID);
			mv_sm_power_enterflow_bysmstate();
			bRequest_Poweron = FALSE;
		}
		break;
	default:
		PRT_ERROR("should not come here. flow = %d, status = %d\n",
			power_myflow, power_mystatus);
		break;
	}

	return S_OK;
}

static void mv_sm_trigger_irq_2_soc(void)
{
	T32smSysCtl_SM_CTRL reg;

	//the SM to SoC interrupt is raised
	reg.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_CTRL);
	reg.uSM_CTRL_SM2SOC_SW_INTR = 1; //enable interrupt
	MV_SM_WRITE_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_CTRL, reg.u32);
}

/* Accordingly, there are three different warmup type */
static hresult mv_sm_power_flow_warmup(SM_POWER_FLOW flow)
{
	if(flow != power_myflow) {
		PRT_ERROR("flow doens't match. %d != %d\n", flow, power_myflow);
		power_myflow = FLOW_IDLE;
		return S_FALSE;
	}

	switch(power_mystatus) {
	case STA_ENTERFLOW:
		mv_sm_power_broadcast_power_state();
		mv_sm_set_state(MV_SM_STATE_STANDBY_2_ACTIVE);
		power_mystatus = STA_ASSERT;
		switch(flow) {
		case FLOW_LOWPOWERSTANDBY_2_ACTIVE:
			mv_sm_set_boot_flag(MV_SOC_STATE_LOWPOWERSTANDBY_2_ACTIVE);
			break;
		case FLOW_NORMALSTANDBY_2_ACTIVE:
			/*
			 * FIXME: After bootloader side for Normal Standby
			 *        feature completed, we should update this line
			 */
			mv_sm_set_boot_flag(MV_SOC_STATE_LOWPOWERSTANDBY_2_ACTIVE);
			break;
		case FLOW_SUSPEND_2_ACTIVE:
			mv_sm_set_boot_flag(MV_SOC_STATE_SUSPEND_2_ACTIVE);
			break;
		default:
			break;
		}
		break;
	case STA_POWERON:
		if (mv_sm_get_boot_flag() == MV_SOC_STATE_SUSPEND_2_ACTIVE) {
			if (suspend_type == MV_SM_POWER_SUSPEND_TYPE_WFI) {
				is_get_rsp = FALSE;
				/* just trigger a irq to soc, otherwise, there will be a redundant msg */
				mv_sm_trigger_irq_2_soc();
				power_mystatus = STA_LEAVEFLOW;
				break;
			}
			/*
			 * else regard suspend_type as PWROFF
			 * do nothing and let it fall through to power cycle
			 */
		}
	case STA_POWEROK:
		mv_sm_power_soc_poweron_2_powerok();
		break;
	case STA_ASSERT:
		/* assert is not needed while warm up */
		power_mystatus = STA_DEASSERT;
		break;
	case STA_DEASSERT:
	case STA_WAITRSP:
		if(S_OK == mv_sm_power_soc_assert_2_deassert()) {
			power_mystatus = STA_LEAVEFLOW;
		}
		break;
	case STA_LEAVEFLOW:
		mv_sm_set_state(MV_SM_STATE_ACTIVE);
		mv_sm_set_boot_flag(MV_SOC_STATE_POWERON);
		mv_sm_power_leaveflow();
		break;
	default:
		PRT_ERROR("should not come here. flow = %d, status = %d\n",
			power_myflow, power_mystatus);
		break;
	}

	return S_OK;
}

static hresult mv_sm_power_flow_sysreset(SM_POWER_FLOW flow)
{
	if(flow != power_myflow) {
		PRT_ERROR("flow doens't match. %d != %d\n", flow, power_myflow);
		power_myflow = FLOW_IDLE;
		return S_FALSE;
	}

	mv_sm_power_broadcast_power_state();

	switch(power_mystatus) {
	case STA_ENTERFLOW:
		power_mystatus = STA_ASSERT;
		switch(flow) {
		case FLOW_SYSRESET_2_ACTIVE:
			mv_sm_set_boot_flag(MV_SOC_STATE_SYSRESET_2_ACTIVE);
			break;
		case FLOW_SYSRESET_2_LOWPOWERSTANDBY:
			mv_sm_set_boot_flag(MV_SOC_STATE_SYSRESET_2_LOWPOWERSTANDBY);
			break;
		case FLOW_SYSRESET_2_NORMALSTANDBY:
			mv_sm_set_boot_flag(MV_SOC_STATE_SYSRESET_2_NORMALSTANDBY);
			break;
		default:
			break;
		}
		break;
	case STA_POWEROFF:
		mv_sm_power_broadcast_power_state();
		mv_sm_power_socpoweroff();
		power_lasttime = mv_sm_timer_gettimems();
		power_mystatus = STA_DEASSERT;
		break;
	case STA_POWERON:
	case STA_POWEROK:
		mv_sm_power_soc_poweron_2_powerok();
		break;
	case STA_ASSERT:
		soc_assert();
		power_lasttime = mv_sm_timer_gettimems();
		power_mystatus = STA_DEASSERT;
		break;
	case STA_WAITRSP:
		if(S_OK == mv_sm_power_soc_assert_2_deassert()) {
			power_mystatus = STA_LEAVEFLOW;
		}
		break;
	case STA_DEASSERT:
		mv_sm_power_soc_assert_2_deassert();
		break;
	case STA_LEAVEFLOW:
		mv_sm_power_leaveflow();
		break;
	default:
		PRT_ERROR("should not come here. flow = %d, status = %d\n",
			power_myflow, power_mystatus);
		break;
	}

	return S_OK;
}

static hresult mv_sm_power_flow_idle()
{
	INT32 state = mv_sm_get_state();

	if( (state == MV_SM_STATE_LOWPOWERSTANDBY) ||
		(state == MV_SM_STATE_NORMALSTANDBY) ||
		(state == MV_SM_STATE_SUSPEND)) {
		eth_wifibt_wakeup_handler();
#ifdef FPSTBYENABLE
		fpbutton_wakeup_handler();
#endif
	}

	return S_OK;
}

static int mv_sm_power_task(void * data)
{
	switch(power_myflow) {
	case FLOW_COLD_2_LOWPOWERSTANDBY:
		mv_sm_power_flow_colddown(power_myflow);
		break;
	case FLOW_ACTIVE_2_LOWPOWERSTANDBY:
	case FLOW_ACTIVE_2_NORMALSTANDBY:
	case FLOW_ACTIVE_2_SUSPEND:
		mv_sm_power_flow_warmdown(power_myflow);
		break;
	case FLOW_LOWPOWERSTANDBY_2_ACTIVE:
	case FLOW_NORMALSTANDBY_2_ACTIVE:
	case FLOW_SUSPEND_2_ACTIVE:
		mv_sm_power_flow_warmup(power_myflow);
		break;
	case FLOW_SYSRESET_2_ACTIVE:
		mv_sm_power_flow_sysreset(power_myflow);
		break;
	case FLOW_IDLE:
		mv_sm_power_flow_idle();
		break;
	default:
		PRT_ERROR("Unknow power flow: flow = %d, status = %d\n", power_myflow, power_mystatus);
		break;
	}

	return S_OK;
}

hresult mv_sm_power_setwakeupsource(INT32 ws)
{
	wi.wakeup_source = (MV_SM_WAKEUP_SOURCE_TYPE)ws;

	switch (wi.wakeup_source)
	{
	case MV_SM_WAKEUP_SOURCE_INVALID:
		PRT_INFO("%s: INVALID\n", __func__);
		break;
	case MV_SM_WAKEUP_SOURCE_IR:
		PRT_INFO("%s: IR\n", __func__);
		break;
	case MV_SM_WAKEUP_SOURCE_WIFI:
		PRT_INFO("%s: WIFI\n", __func__);
		break;
	case MV_SM_WAKEUP_SOURCE_BT:
		PRT_INFO("%s: BT\n", __func__);
		break;
	case MV_SM_WAKEUP_SOURCE_WOL:
		PRT_INFO("%s: WOL\n", __func__);
		break;
	case MV_SM_WAKEUP_SOURCE_VGA:
		PRT_INFO("%s: VGA\n", __func__);
		break;
	case MV_SM_WAKEUP_SOURCE_CEC:
		PRT_INFO("%s: CEC\n", __func__);
		break;
	case MV_SM_WAKEUP_SOURCE_TIMER:
		PRT_INFO("%s: TIMER\n", __func__);
		break;
	case MV_SM_WAKEUP_SOURCE_BUTTON:
		PRT_INFO("%s: FP STBY BUTTON\n", __func__);
		break;
	default:
		PRT_INFO("Unknow wake up source (%d)!\n", wi.wakeup_source);
		break;
	}

	return S_OK;
}

void mv_sm_power_setwakeuppayload(unsigned int value)
{
	wi.payload[0] = value;
}

void sm_set_reboot_reason(UINT32 reboot_reason)
{
#if defined(SM_RAM_TS_ENABLE)
	char rebootflag[8];

	sprintf(rebootflag,"%d", reboot_reason);
	if (ram_ts_set(RTS_KEY_BOOTREASON, rebootflag))
		PRT_ERROR("RTS set boot reason fail!!!\n");
#elif defined(SM_RAM_PARAM_ENABLE)
	rebootflag_t * g_rebootflag = (rebootflag_t *)(uintptr_t)(SM_REBOOTREASON_ADDR);

	g_rebootflag->magic = REBOOTFLAG_MAGIC;
	g_rebootflag->flag = reboot_reason;
#endif
	return;
}

static int __attribute__((used)) mv_sm_process_power_msg(void * data, INT32 len)
{
	INT32 isysstate, iMsgType, iMsgContent;
	isysstate = mv_sm_get_state();
	iMsgType = *(INT32*)data;
	iMsgContent=*(((INT32 *)data) + 1);

	PRT_DEBUG("sysstate = 0x%x, type = 0x%x, content = 0x%x\n", isysstate, iMsgType, iMsgContent);

	switch(isysstate)
	{
	case MV_SM_STATE_COLD:
		if (iMsgType == MV_SoC_STATE_COLDBOOT)
			mv_sm_power_enterflow(FLOW_COLD_2_LOWPOWERSTANDBY);
		else if (iMsgType == MV_SoC_STATE_SYSPOWERON)
			bRequest_Poweron = TRUE;
		break;
	case MV_SM_STATE_ACTIVE:
		// Only process warm down request
		switch(iMsgType)
		{
		case MV_SM_IR_Linuxready:
			{
				is_linux_ready = TRUE;

				is_wakeup = FALSE;
				#ifdef GETHENABLE
				geth_wakeup = FALSE;
				#endif
				#ifdef FPSTBYENABLE
				fpbutton_wakeup = FALSE;
				#endif
				//if system is woke up by timer, send an irq to infor linux
				if(wi.wakeup_source == MV_SM_WAKEUP_SOURCE_TIMER) {
					sm_send_msg(MV_SM_ID_POWER , (unsigned char *)(&wi), sizeof(struct wakeup_info));
				}
				break;
			}
		case MV_SM_POWER_LOWPOWERSTANDBY_REQUEST:
			{
				standby_type = MV_SM_POWER_LOWPOWERSTANDBY_REQUEST;
				break;
			}
		case MV_SM_POWER_NORMALSTANDBY_REQUEST:
			{
				standby_type = MV_SM_POWER_NORMALSTANDBY_REQUEST;
				break;
			}
		case MV_SM_POWER_STANDBY_REQUEST:
			{
				if(standby_type == MV_SM_POWER_LOWPOWERSTANDBY_REQUEST)
					mv_sm_power_enterflow(FLOW_ACTIVE_2_LOWPOWERSTANDBY);
				else
					mv_sm_power_enterflow(FLOW_ACTIVE_2_NORMALSTANDBY);
				break;
			}
		case MV_SM_POWER_SUSPEND_REQUEST:
			{
				linux_resume_addr = iMsgContent ;
				PRT_INFO("receive MV_SM_POWER_WARMDOWN_REQUEST_2 from Linux: %x\n", iMsgContent);
				suspend_type = *(((INT32 *)data) + 2);
				PRT_INFO("receive suspend type: 0x%08x\n", suspend_type);
				mv_sm_power_enterflow(FLOW_ACTIVE_2_SUSPEND);
				break;
			}
		case MV_SM_POWER_WAKEUP_SOURCE_REQUEST:
			{
				sm_send_msg(MV_SM_ID_POWER , (unsigned char *)(&wi), sizeof(struct wakeup_info));
				if(iMsgContent)  // clear the payload of wakeup source
					mv_sm_power_setwakeuppayload(0);
				break;
			}
		case MV_SM_POWER_BOOTUP_FLAG_REQUEST:
			{
				UINT32 power_bootup_flag = mv_sm_get_boot_flag();
				sm_send_msg(MV_SM_ID_POWER , (unsigned char *)(&power_bootup_flag), sizeof(power_bootup_flag));
				break;
			}
		case MV_SM_POWER_CURR_TIME_REQUEST:
			{
				timer_long_t t;
				t.ltime = mv_sm_timer_gettimems();
				//return a 64bits long time with ms
				sm_send_msg(MV_SM_ID_POWER, (unsigned char *)(&t), sizeof(timer_long_t));
				break;
			}
		case MV_SM_POWER_WARMDOWN_TIME://warmdown on time
			{
				//FIXME: there is no return value of the request
				sm_setup_power_down_timer(iMsgContent);
				break;
			}
		case MV_SM_POWER_WARMUP_TIME_REQUEST://get warmup remaining time
			{
				int remain_time = sm_get_warmup_remain_time();
				PRT_INFO("remain wakeup time is :%d seconds\n", remain_time);
				sm_send_msg(MV_SM_ID_POWER, (unsigned char*)(&remain_time), sizeof(remain_time));
				break;
			}
		case MV_SM_POWER_WARMUP_TIME://warmup on time
			{
				//FIXME: there is no return value of the request
				PRT_INFO("set wakeup time is :%d seconds\n", iMsgContent);
				sm_setup_power_up_timer(iMsgContent);
				break;
			}
		case MV_SM_POWER_SYS_RESET://reset whole system
			{
				PRT_INFO("%%%%%%%%SM:Msg reset system%%%%%%%%\n");

				sm_set_reboot_reason(REBOOT);
				mv_sm_power_enterflow(FLOW_SYSRESET_2_ACTIVE);
				break;
			}
		case MV_SM_POWER_FASTBOOT_ENABLE://fast boot mode enter
			{
				PRT_INFO("%%%%%%%%SM:Enter fast boot mode%%%%%%%%\n");
				bFastboot = TRUE;
				break;
			}
		case MV_SM_POWER_FASTBOOT_DISABLE://fast boot mode exit
			{
				PRT_INFO("%%%%%%%%SM:Exit fast boot mode%%%%%%%%\n");
				bFastboot = FALSE;
				break;
			}
		default:
			break;
		}
		break;
	case MV_SM_STATE_ACTIVE_2_LOWPOWERSTANDBY:
	case MV_SM_STATE_ACTIVE_2_NORMALSTANDBY:
		if((iMsgType == MV_SM_ACTIVE_2_LOWPOWERSTANDBY_RESP)
			|| (iMsgType == MV_SM_ACTIVE_2_NORMALSTANDBY_RESP)) {
			//set_linux_startaddr(0xFFFFFFFF);
			PRT_INFO("receive WARMDOWN resp, set warmup param: 0xFFFFFFFF\n");
		}
		break;
	case MV_SM_STATE_ACTIVE_2_SUSPEND:
		if(iMsgType == MV_SM_ACTIVE_2_SUSPEND_RESP) {
			//set_linux_startaddr(linux_resume_addr);
			PRT_INFO("receive WARMDOWN resp, set warmup param: 0x%x\n", linux_resume_addr);
			is_get_rsp = TRUE;
			mv_sm_set_boot_flag(MV_SOC_STATE_ACTIVE_2_SUSPEND);
		}
		break;
	case MV_SM_STATE_STANDBY_2_ACTIVE:  // we only need to know SOC has warmed up
		{
			//FIXME: response format
			if((iMsgType == MV_SM_LOWPOWERSTANDBY_2_ACTIVE_RESP)
				|| (iMsgType == MV_SM_NORMALSTANDBY_2_ACTIVE_RESP)
				|| (iMsgType == MV_SM_SUSPEND_2_ACTIVE_RESP)
				|| (iMsgType == MV_SM_COLD_2_ACTIVE_RESP)) {
				PRT_INFO("SM:Enter warmup state\n");
				is_get_rsp = TRUE;
			}
		}
		break;
	case MV_SM_STATE_SUSPEND:
		//FIXME: add this logic for corner case that suspend waked up by none-SM source.
		if(iMsgType == MV_SM_IR_Linuxready) {
			PRT_INFO("--FIXME: *****Power Msg: IR ready******\n");
			mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_INVALID);
			mv_sm_power_enterflow_bysmstate();
		}
		break;
	default:
		break;
	}

	return S_OK;
}

#define POWER_STACK_SIZE ((uint16_t) 512)

static void power_task(void * para)
{
	for ( ;; ) {
		mv_sm_power_task(para);
		vTaskDelay(1);
	}
}

static void __attribute__((used)) create_power_task(void)
{
	xTaskCreate(power_task, "power", POWER_STACK_SIZE, NULL, TASK_PRIORITY_1, NULL);
}

DECLARE_RT_MODULE(
	power,
	MV_SM_ID_POWER,
	create_power_task,
	mv_sm_process_power_msg,
	NULL
);
