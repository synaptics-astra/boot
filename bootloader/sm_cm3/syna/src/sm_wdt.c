/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright © 2013-2018 Synaptics Incorporated. All rights reserved.
 *
 * This file contains information that is proprietary to Synaptics
 * Incorporated ("Synaptics"). The holder of this file shall treat all
 * information contained herein as confidential, shall use the
 * information only for its intended purpose, and shall not duplicate,
 * disclose, or disseminate any of this information in any manner
 * unless Synaptics has otherwise provided express, written
 * permission.
 *
 * Use of the materials may require a license of intellectual property
 * from a third party or from Synaptics. This file conveys no express
 * or implied licenses to any intellectual property rights belonging
 * to Synaptics.
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
#ifdef WDTENABLE
#include "sm_type.h"
#include "sm_io.h"
#include "sm_common.h"
#include "sm_memmap.h"

#include "sm_printf.h"

#include "sm_power.h"
#include "sm_timer.h"

#include "sm_apb_ictl_defs.h"
#include "sm_apb_wdt_defs.h"

#include "sm_rt_module.h"
#include "sm_exception.h"

/* timeout = (2^(16 + i) -1) /(xx_SYSTEM_HZ * 1000).
  * i from 0 to 15.
  * SM (clock 25Mhz)
  * 0: 2.6214 ms; 1: 5.2428 ms; 2; 10.48 ms;
  * 3: 20.97 ms; 4: 41.94 ms; 5: 83.89 ms;
  * 6: 167.77 ms; 7: 335.54 ms; 8: 671.09 ms;
  * 9: 1.342 s; 10: 2.684 s; 11: 5.369 s;
  * 12: 10.737 s; 13: 21.475 s; 14: 42.950 s;
  * 15: 85.899 s
*/
// ms
#define WDT0TIMEOUT	(11)
#define WDT1TIMEOUT	(12)

typedef enum
{
	SM_WDT_0 = 0,
	SM_WDT_1 = 1,
	SM_WDT_2 = 2,
}SM_WDT_ID;

typedef enum
{
	SYSTEM_RESET_MODE = 0,
	INTERRUPT_MODE = 1,
}SM_WDT_RSP_MODE;

typedef enum
{
	PCLK_2 = 0,
	PCLK_4 = 1,
	PCLK_8 = 2,
	PCLK_16 = 3,
	PCLK_32 = 4,
	PCLK_64 = 5,
	PCLK_128 = 6,
	PCLK_256 = 7,
}SM_WDT_RSP_RPL;

//WDT0: for SM
//WDT1: for CPU0

static BOOL is_appwdt_enable = FALSE;

static unsigned int wdt0_isr = 0x0;

static void sm_wdt_enable(SM_WDT_ID id)
{
	unsigned int reg = MV_SM_READ_REG32(WDT_WDT_CR(id));

	MV_SM_WRITE_REG32(WDT_WDT_CR(id), reg | 0x01);
}


static void sm_wdt_set_rspmode(SM_WDT_ID id, SM_WDT_RSP_MODE mode)
{
	unsigned int reg = MV_SM_READ_REG32(WDT_WDT_CR(id));
	if(mode)
		reg |= (1 << 1);
	else
		reg &= ~(1 << 1);
	MV_SM_WRITE_REG32(WDT_WDT_CR(id), reg);
}

static void sm_wdt_set_rpl(SM_WDT_ID id, SM_WDT_RSP_RPL rpl)
{
	unsigned int reg = MV_SM_READ_REG32(WDT_WDT_CR(id));
	reg &= ~(0x7 << 2);
	reg |= (rpl << 2);
	MV_SM_WRITE_REG32(WDT_WDT_CR(id), reg);
}


static void sm_wdt_settimeout(SM_WDT_ID id, int settings)
{
	if((settings < 0) || (settings > 15)) {
		PRT_WARN("wrong timeout parameter\n");
		return;
	}

	MV_SM_WRITE_REG32(WDT_WDT_TORR(id), settings);
}

static void sm_wdt_restart(SM_WDT_ID id)
{
	// 0x76 is the value that wdt spec recommended
	MV_SM_WRITE_REG32(WDT_WDT_CRR(id), 0x76);
}

static unsigned int __attribute__((used)) sm_wdt_read(SM_WDT_ID id)
{
	return MV_SM_READ_REG32(WDT_WDT_CCVR(id));
}

static void sm_wdt_clearint(SM_WDT_ID       id)
{
	MV_SM_READ_REG32(WDT_WDT_EOI(id));
}

static unsigned int __attribute__((used)) sm_wdt_getintstatus(SM_WDT_ID id)
{
	return MV_SM_READ_REG32(WDT_WDT_STAT(id));
}

static BOOL sm_wdt_getenstatus(SM_WDT_ID id)
{
	return (MV_SM_READ_REG32(WDT_WDT_CR(id)) & 0x1);
}

static void __attribute__((used)) mv_sm_wdt_0_isr(void)
{
	//MV_SM_printf("SM:Current state id=%d, Current task id=%d, temperature=%d.\n",MV_SM_Get_State(),SM_Status.currtaskid,SM_Status.tempval);
	//MV_SM_printf("SM:Last message, module id=%d, first data=%d.\n",SM_Status.lastmsgmodid,SM_Status.lastmsgdata);

	wdt0_isr++;
	if(wdt0_isr > 1){
		PRT_ERROR("Watchdog 0 time out (SM dies)\nSystem automatically reset now!\n");

		sm_set_reboot_reason(WATCHDOG);
	}
	PRT_WARN("WDT0 TORR: 0x%x!\n", MV_SM_READ_REG32(WDT_WDT_TORR(SM_WDT_0)));

	// FIXME:
	//__rom_dump_sm_trace();
	//__rom_sm_wdt_clearint(SM_WDT_0);
	//MV_SM_Power_SYSResetByState();
}

//WDT1 timeout,means CPU0 dies, reset SoC
static void __attribute__((used)) mv_sm_wdt_1_isr(void)
{
	//MV_SM_Dump_MsgQ_All();
	//MV_SM_Dump_WDT_All();
	PRT_ERROR("Watchdog 1 time out (CPU0/1 dies)\nSystem automatically reset now!\n");

	sm_set_reboot_reason(WATCHDOG);

	mv_sm_power_enterflow(FLOW_SYSRESET_2_ACTIVE);

	// FIXME:
	//__rom_dump_sm_trace();
	sm_wdt_clearint(SM_WDT_1);
	//MV_SM_Power_SYSReset(1);
	is_appwdt_enable = FALSE;
}

static void __attribute__((used)) mv_sm_wdt_2_isr(void)
{
	sm_wdt_clearint(SM_WDT_2);
	PRT_DEBUG(("WDT_2_ISR\n"));
}

DECLARE_ISR(SMICTL_IRQ_SM_WDT0, mv_sm_wdt_0_isr);
DECLARE_ISR(SMICTL_IRQ_SM_WDT1, mv_sm_wdt_1_isr);
DECLARE_ISR(SMICTL_IRQ_SM_WDT2, mv_sm_wdt_2_isr);

static void mv_sm_wdt_task(void * para)
{
	sm_wdt_enable(SM_WDT_0);
	for ( ;; ) {
		sm_wdt_restart(SM_WDT_0); //kick off watchdog

		if(sm_wdt_getenstatus(SM_WDT_1))
		{
			if(!is_appwdt_enable)
			{
				sm_wdt_restart(SM_WDT_1);
			}
		}
		vTaskDelay(2000);
	}
}

static void mv_sm_wdt_idle_task(void)
{
	volatile unsigned int temp = 0x0;

	//disable APB interrupt except WDT0/1
	temp |= (1 << SMICTL_IRQ_SM_WDT0);
	temp |= (1 << SMICTL_IRQ_SM_WDT1);

	MV_SM_WRITE_REG32(APB_ICTL_IRQ_INTEN, temp);
	MV_SM_WRITE_REG32(APB_ICTL_IRQ_INTEN_H, 0);
	MV_SM_WRITE_REG32(APB_ICTL_FIQ_INTEN, 0);

	while(1) {
		sm_wdt_restart(SM_WDT_0); //kick off watchdog
		if(sm_wdt_getenstatus(SM_WDT_1))
		{
			sm_wdt_restart(SM_WDT_1);
		}

		mv_sm_delay_ms(3500); //Delay about 3.5s
		PRT_DEBUG("Service WDT Idle Task \n");
	}
}

static int __attribute__((used)) mv_sm_process_wdt_msg(void * data, INT32 len)
{
	INT32 iWDT,iMsgContent;

	iWDT = *(INT32 *)data;
	iMsgContent = *((INT32 *)data + 1);
	if(iWDT == 1)
	{
		switch(iMsgContent)
		{
			case MV_SM_WD_APP_START:
				//reset width:2pclk cycles;interrupt mode;WDT enable
				sm_wdt_set_rspmode(SM_WDT_1, INTERRUPT_MODE);
				sm_wdt_set_rpl(SM_WDT_1, PCLK_8);
				sm_wdt_settimeout(SM_WDT_1, WDT1TIMEOUT);
				sm_wdt_enable(SM_WDT_1);
				sm_wdt_restart(SM_WDT_1);
				is_appwdt_enable = TRUE;
				PRT_INFO("SM: wdt1 enable\n");
				break;
			case MV_SM_WD_APP_CONTINUE:
				is_appwdt_enable = TRUE;
				PRT_INFO("SM: wdt1 app continue\n");
				break;
			case MV_SM_WD_APP_EXIT:
				is_appwdt_enable = FALSE;
				PRT_INFO("SM: wdt1 app exit\n");
				break;
			case MV_SM_WD_Kickoff:
				sm_wdt_restart(SM_WDT_1);
				break;
			case MV_SM_WD_FASTBOOT:
				PRT_INFO("SM: received MV_SM_WD_FASTBOOT Msg\n");
				mv_sm_wdt_idle_task();
			default:
				PRT_WARN("SM: unknow WD Msg\n");
				break;
		}
	}

	return S_OK;
}

static int __attribute__((used)) mv_sm_process_internal_wdt_msg(void *data, INT32 len)
{
	UINT32 *msg = (UINT32 *)data;

	if(msg[0] == MV_SM_ID_POWER) {
		switch (msg[1]) {
		case FLOW_ACTIVE_2_LOWPOWERSTANDBY:
		case FLOW_ACTIVE_2_NORMALSTANDBY:
		case FLOW_ACTIVE_2_SUSPEND:
			if (msg[2] == STA_LEAVEFLOW)
				is_appwdt_enable = FALSE;
			break;
		case FLOW_SUSPEND_2_ACTIVE:
			if (msg[2] == STA_LEAVEFLOW)
				is_appwdt_enable = TRUE;
			break;
		default:
			break;
		}
	}
	return S_OK;
}

#define WDT_STACK_SIZE ((uint16_t) 128)

static void __attribute__((used)) create_wdt_task(void)
{
	xTaskCreate(mv_sm_wdt_task, "wdt", WDT_STACK_SIZE, NULL, TASK_PRIORITY_2, NULL);
}

DECLARE_RT_MODULE(
	wdt,
	MV_SM_ID_WDT,
	create_wdt_task,
	mv_sm_process_wdt_msg,
	mv_sm_process_internal_wdt_msg
);

static int __attribute__((used)) sm_watchdog_init(void)
{
	T32smSysCtl_SM_WDT_MASK reg;

	reg.u32 = MV_SM_READ_REG32(SM_MEMMAP_SYS_CTRL_BASE + RA_smSysCtl_SM_WDT_MASK);
	reg.uSM_WDT_MASK_SM_RST = 0x6;//wdt0: reset SM
	reg.uSM_WDT_MASK_SOC_RST = 0x4;//wdt 0,wdt 1:SoC reset enable
	MV_SM_WRITE_REG32(SM_MEMMAP_SYS_CTRL_BASE + RA_smSysCtl_SM_WDT_MASK, reg.u32);

	//interrupt mode, and reset signal pulse repeat 8 pclk cycles
	sm_wdt_set_rspmode(SM_WDT_0, INTERRUPT_MODE);
	sm_wdt_set_rpl(SM_WDT_0, PCLK_8);
	sm_wdt_settimeout(SM_WDT_0, WDT0TIMEOUT);

	return S_OK;
}

DECLARE_RT_INIT(wdtinit, INIT_DEV_P_25, sm_watchdog_init);

#endif
