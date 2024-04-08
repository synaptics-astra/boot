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
#ifdef IRENABLE
#include "FreeRTOS.h"
#include "task.h"

#include "sm_type.h"

#include "sm_ir_key_def.h"

#include "platform_config.h"

#include "sm_common.h"

#include "sm_timer.h"
#include "sm_apb_timer.h"
#include "sm_comm.h"
#include "sm_state.h"
#include "sm_power.h"

#include "sm_printf.h"

#include "sm_gpio.h"
#include "sm_rt_module.h"

#ifdef XMP_PROTOCAL
#include "ir_xmp.h"
#include "ir_xmp_key_table.h"
#endif
#ifdef NEC_PROTOCAL
#include "ir_nec.h"
#include "ir_nec_key_table_elektra.h"
#include "ir_nec_key_table.h"
#include "ir_nec_key_table_sky.h"

#endif
#ifdef RC5_PROTOCAL
#include "ir_rc5.h"
#include "ir_rc5_key_table.h"
#endif
#ifdef TC9012_PROTOCAL
#include "ir_tc9012.h"
#include "ir_tc9012_key_table.h"
#endif

/*
 * Samsung IR TV signal:
 * ___---_--_--_--_-_-_-_-_-_--_--_--_-_-_-_-_-_~~
 * Pre    1  1  1  0 0 0 0 0 1  1  1  0 0 0 0 0 Effect_code
 */

static UINT32 cbuffer[IR_CBUFF_LENGTH];
static UINT32 cb_start=0, cb_end=0;
volatile INT32 PowerWaitElaps=0;
volatile INT32 PowerWaitFlag=0;
extern BOOL bLoaderOpen;
extern volatile struct wakeup_info wi;

static BOOL is_linuxready = FALSE;
#ifdef LWSTANDBY_ENABLE
static BOOL bSendPowerkeyOnly = FALSE;
#endif

#ifdef XMP_PROTOCAL
static ir_key_table_t *ir_key_table = vendor_ir_xmp_table;
#endif
#ifdef NEC_PROTOCAL
static ir_key_table_t *ir_key_table = vendor_ir_nec_table;
#endif
#ifdef RC5_PROTOCAL
static ir_key_table_t *ir_key_table = vendor_ir_rc5_table;
#endif
#ifdef TC9012_PROTOCAL
static ir_key_table_t *ir_key_table = vendor_ir_tc9012_table;
#endif

static INT32 bInputDisable = 0;
/*
 * Send out the infrared key to Berlin
 */
static UINT32 Translate_IRCode( UINT32 uiIRCode )
{
	INT32 i = 0;
	UINT32 tmp_code;
	UINT32 ir_key = MV_IR_KEY_NULL;

	//PRT_RES("key code:0x%08x\n",uiIRCode);
	while( ir_key_table[i].ir_key != MV_IR_KEY_NULL )
	{
		tmp_code = ( uiIRCode >> ir_key_table[i].key_bitpos) & ir_key_table[i].key_mask;
		if( ( ir_key_table[i].ir_encode & ir_key_table[i].key_mask ) == tmp_code )
		{
			ir_key = ir_key_table[i].ir_key;
			break;
		}
		i++;
	}

	if( ir_key_table[i].ir_key == MV_IR_KEY_NULL ) {
		PRT_DEBUG("havent found this key. key code is %x.\n", uiIRCode);
	}

	return ir_key;
}

/*
 * analyse the level length based on NEC protocol.
 * NEC ir transmitter timing:
 * Preamble: 8.993ms/4.642ms/0.561ms, Bit1: 1.75ms/0.561ms, Bit0: 0.598ms/0.561ms.
 * Hold: 8.993ms/2.25ms/0.56ms
 */

#define IR_HOLDKEY_DELAY
#ifdef IR_HOLDKEY_DELAY
static int iHoldKeyCount = 0;
#endif

/*
 * Called in interrupt handler to store the level length
 */
static void store_ircbuffer(unsigned int level_len, unsigned int high_low)
{
	cbuffer[cb_end] = (level_len & 0x7fffffff) | (high_low << 31);
	cb_end++;
	cb_end &= CIRC_BUFF_MASK;

	/*
	 * if cb_start is equal to cb_end, there's no new data in the cbuffer.
	 * Therefore, we have to avoid that, because we're writing a new data.
	 */
	if (cb_end == cb_start) {
		cb_start++;
		cb_start &= CIRC_BUFF_MASK;
	}
}

unsigned int isr_length_caculate(unsigned int ms, unsigned int ticks,
					unsigned int prev_ms, unsigned int prev_ticks)
{
	return ((ticks > prev_ticks) ? (ticks - prev_ticks) : (ticks - prev_ticks + (ms/INT_MS - prev_ms/INT_MS)*INT_MS*1000*SM_SYSTEM_HZ));
}

unsigned int length_caculate(unsigned int ticks,unsigned int prev_ticks)
{
	return ((ticks > prev_ticks) ? (ticks - prev_ticks) : (ticks - prev_ticks + INT_MS*1000*SM_SYSTEM_HZ));
}

void mv_sm_ir_isr(void)
{
	static unsigned int iPrev_ticks = 0;
	static int ir_current_set_rise = 0;
	unsigned int iCurr_ticks = 0;
	int iCurr_length = 0;

	while(1) {
		iCurr_ticks = sm_apb_timer_readclock();
		if(iCurr_ticks<=(INT_MS*1000*SM_SYSTEM_HZ) && iCurr_ticks>=0)
			break;
		//PRT_ERROR("!!!Abnormal tick value(0x%x)...Try again",iCurr_ticks);
	}

	ir_current_set_rise ^= 1;

	if(ir_current_set_rise) {
		sm_gpio_set_polarity(SM_GPIO_PORT_IR, RISING_EDGE);
	} else {
		sm_gpio_set_polarity(SM_GPIO_PORT_IR, FALLING_EDGE);
	}

	// Store the current level length to circular buffer
	iCurr_length = length_caculate(iCurr_ticks, iPrev_ticks);

	store_ircbuffer(iCurr_length, ir_current_set_rise );

	iPrev_ticks = iCurr_ticks;

	// Clear interrupt
	sm_gpio_clearint(SM_GPIO_PORT_IR);

	return;
}

static UINT32 ir_preprocess_key(void)
{
	UINT32 ir_code = 0;
	BOOL is_hold = FALSE;
	UINT32 uiIRKey = MV_IR_KEY_NULL;
	static unsigned int keydown_ticks = 0;
	static unsigned int lastkeyhold_ticks = 0;
	unsigned int curr_ticks = 0;
	static BOOL keydown=FALSE,keyhold=FALSE;
	static UINT32 currkey=MV_IR_KEY_NULL;

#ifdef XMP_PROTOCAL
	ir_code = read_xmp(cbuffer, &cb_start, &cb_end, &is_hold);//get downkey,hold key,null keys
#endif
#ifdef NEC_PROTOCAL
	ir_code = read_nec(cbuffer, &cb_start, &cb_end, &is_hold);//get downkey,hold key,null keys
#endif
#ifdef RC5_PROTOCAL
	ir_code = read_rc5(cbuffer, &cb_start, &cb_end, &is_hold);
#endif
#ifdef TC9012_PROTOCAL
	ir_code = read_tc9012(cbuffer, &cb_start, &cb_end, &is_hold);
#endif
	if( ir_code == 0xffffffff ) {
		uiIRKey = MV_IR_KEY_NULL;
	} else {
		uiIRKey = Translate_IRCode( ir_code );

		if(is_hold) {
			uiIRKey = MV_IR_HOLDKEY_KEY(uiIRKey);
		}
	}

	if(IR_HOLDKEY_SEND && (!IR_UPKEY_SEND))
	{
		return uiIRKey;
	}

	if((!IR_HOLDKEY_SEND)&&(!IR_UPKEY_SEND))//filter holdkey
	{
		if(uiIRKey&MV_IR_HOLDKEY_FLAG)
			uiIRKey=MV_IR_KEY_NULL;
		return uiIRKey;
	}

	if( uiIRKey != MV_IR_KEY_NULL )//hold keys or new key down
	{
		//SMLOGPRINTF(("raw IRkey = 0x%8x\n",uiIRKey));
		if((uiIRKey&MV_IR_HOLDKEY_FLAG)!=0)//hold key
		{
			if((uiIRKey==MV_IR_HOLDKEY_KEY(currkey))&&keydown)
			{
				//SMLOGPRINTF(("[%d]hold key\n",mv_sm_timer_gettimems()));
				keyhold=TRUE;
				lastkeyhold_ticks = sm_apb_timer_readclock();
#ifdef IR_HOLDKEY_DELAY
				iHoldKeyCount++;
				if(iHoldKeyCount<HoldKeySkipTime)
					uiIRKey = MV_IR_KEY_NULL;
#endif
			}
			else
				uiIRKey = MV_IR_KEY_NULL;
		}
		else//new key
		{
			if(keydown)//The fist key hasn't up, so throw this key
				uiIRKey=MV_IR_KEY_NULL;
			else
			{
				//SMLOGPRINTF(("[%d]new key\n",mv_sm_timer_gettimems()));
				currkey=uiIRKey;
				keydown_ticks = sm_apb_timer_readclock();
#ifdef IR_HOLDKEY_DELAY
				iHoldKeyCount=0;
#endif
				keydown=TRUE;
			}
		}


	}
	else
	{
		if(keydown)
		{
			curr_ticks = sm_apb_timer_readclock();
			if(keyhold)
			{
				//key up after some hold keys
				if (length_caculate(curr_ticks, lastkeyhold_ticks) > FRAME_CYCLE_NORMAL_MAX)
				{
					//SMLOGPRINTF(("key hold lead to keyup\n"));
					uiIRKey = MV_IR_KEY2UPKEY(currkey);
					keydown = FALSE;
					keyhold = FALSE;
					currkey = MV_IR_KEY_NULL;
				}
			}
			else
			{
				//no hold key,key up directly
				if (length_caculate(curr_ticks, keydown_ticks) > FRAME_CYCLE_FIRSTHOLD_MAX)
				{
					//SMLOGPRINTF(("key down lead to keyup\n"));
					uiIRKey=MV_IR_KEY2UPKEY(currkey);
					keydown=FALSE;
					currkey=MV_IR_KEY_NULL;
				}
			}

		}
	}
	if(IR_HOLDKEY_SEND&&IR_UPKEY_SEND)
	{
		return uiIRKey;
	}

	if((!IR_HOLDKEY_SEND)&&(IR_UPKEY_SEND))//filter holdkey
	{
		if(uiIRKey&MV_IR_HOLDKEY_FLAG)
			uiIRKey=MV_IR_KEY_NULL;
		return uiIRKey;
	}
}

//#define IR_DBG_POWER
hresult mv_sm_process_key(unsigned int module_id, unsigned int key)
{
	INT32 iSysState;

	iSysState = mv_sm_get_state();

	switch( iSysState )
	{
	case MV_SM_STATE_STANDBY_2_ACTIVE:
		//discard up and hold key
		if(((key & MV_IR_HOLDKEY_FLAG) == 0) &&
			((key & MV_IR_UPKEY_FLAG) == 0)) {
			if(key == MV_IR_KEY_POWER) {
				//FIXME: how should we do
				//MV_SM_Power_Enterflow(mv_sm_timer_readclock(),FLOW_Warmup2WaitWarmdown);
			}
		}
		break;
	case MV_SM_STATE_ACTIVE:
/* FIXME */
		if(is_linuxready) {
			if (bInputDisable)
				return S_OK;
#ifdef LWSTANDBY_ENABLE
			if(!bSendPowerkeyOnly
				|| (bSendPowerkeyOnly && ((key & 0xFFFF) == MV_IR_KEY_POWER)))
#endif
			sm_send_msg(module_id, (unsigned char*)(&key), sizeof(key));
		} else {
			if(((key & MV_IR_HOLDKEY_FLAG) == 0) &&
				((key&MV_IR_UPKEY_FLAG)==0)) {
				if(key == MV_IR_KEY_POWER) {
					//FIXME: which state we should go
					//linux is booting now, discard any key request
					//mv_sm_power_enterflow(FLOW_ACTIVE_2_NORMALSTANDBY);
				}
			}
		}
		break;
	case MV_SM_STATE_LOWPOWERSTANDBY:
	case MV_SM_STATE_NORMALSTANDBY:
	case MV_SM_STATE_SUSPEND:
		//PRT_INFO("--FIXME: IR KEY Wakeup Recevived !\n"); //only debug message
		//discard hold key and enter wakeup flow in up key enent
		if(((key & MV_IR_HOLDKEY_FLAG) == 0) && ((key & MV_IR_UPKEY_FLAG) != 0)) {
			/*FIXME*/
			mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_IR);
			mv_sm_power_setwakeuppayload(key);
			mv_sm_power_enterflow_bysmstate();
		}
		break;
	default:
		break;
	}

	return S_OK;
}

static void mv_sm_ir_broadcast_key_state(UINT32 is_down, UINT32 key)
{
	UINT32 msg[3];

	msg[0] = MV_SM_ID_IR;
	msg[1] = is_down;
	msg[2] = key;

	sm_send_internal_msg(BOARDCASTMSG, (unsigned char *)msg, sizeof(msg));
}

static int mv_sm_ir_task(void * data)
{
	UINT32 uiIRKey = MV_IR_KEY_NULL;

	uiIRKey = ir_preprocess_key();
	if( uiIRKey != MV_IR_KEY_NULL )
	{
		PRT_INFO("-----IRkey = 0x%08x -----\n",uiIRKey);

		if(uiIRKey & MV_IR_UPKEY_FLAG)//upkey
		{
			/*FIXME*/
			//if(!bFastboot_Standby)
			//	MV_SM_LED_Ctrl(0);
			mv_sm_ir_broadcast_key_state(0, uiIRKey);
		}
		else if(!(uiIRKey & MV_IR_HOLDKEY_FLAG))//down key
		{
			/*FIXME*/
			//MV_SM_LED_Ctrl(1);
			mv_sm_ir_broadcast_key_state(1, uiIRKey);
		}

		mv_sm_process_key(MV_SM_ID_IR,uiIRKey);
	}

	return S_OK;
}


static int __attribute__((used)) mv_sm_process_ir_msg(void * data, INT32 len)
{
	INT32 *pMsgBody = (INT32*)data;
	UINT32 uiIRKey = MV_IR_KEY_NULL;
	//This msg actaully means IR device is opened by application
	switch (*pMsgBody)
	{
		case MV_SM_IR_Linuxready:
			is_linuxready = TRUE;
			PRT_INFO("*******IR ready********\n");
			sm_gpio_int_enable(SM_GPIO_PORT_IR);
			break;
		case MV_SM_INPUT_DISABLE:
			bInputDisable = 1;
			PRT_INFO("*******IR disable********\n");
			break;
		case MV_SM_INPUT_ENABLE:
			bInputDisable = 0;
			PRT_INFO("*******IR enable********\n");
			break;
		case MV_SM_INPUT_SEND_KEY:
			uiIRKey = pMsgBody[1];
			PRT_INFO("*******IRkey = 0x%8x -----\n", uiIRKey);
			sm_send_msg(MV_SM_ID_IR, (unsigned char*)(&uiIRKey), sizeof(uiIRKey));
			break;
#ifdef LWSTANDBY_ENABLE
		case MV_SM_INPUT_POWERKEY_ONLY:
			bSendPowerkeyOnly = pMsgBody[1];
			PRT_INFO("*******IR SendPowerKeyOnly %d********\n", bSendPowerkeyOnly);
			break;
#endif
		default:
			break;
	}
	return S_OK;
}

/*
 * send IR POWER KEY event to system on woken up
 */
static void sm_send_power_key()
{
	UINT32 iMsg = MV_IR_KEY_POWER;
	switch (wi.wakeup_source) {
		case MV_SM_WAKEUP_SOURCE_IR:
		case MV_SM_WAKEUP_SOURCE_WIFI:
		case MV_SM_WAKEUP_SOURCE_BT:
		case MV_SM_WAKEUP_SOURCE_WOL:
		case MV_SM_WAKEUP_SOURCE_BUTTON:
			sm_send_msg(MV_SM_ID_IR, (unsigned char*)(&iMsg), sizeof(iMsg));
			iMsg |= 0x8000000;
			sm_send_msg(MV_SM_ID_IR, (unsigned char*)(&iMsg), sizeof(iMsg));
			PRT_DEBUG("send power key");
			break;
		default:
			PRT_DEBUG("do not send power key");
			break;
	}
}

static int __attribute__((used)) mv_sm_process_internal_ir_msg(void * data, INT32 len)
{
	UINT32 *msg = (UINT32*)data;

	if(msg[0] == MV_SM_ID_POWER) {
		switch (msg[1]) {
		case FLOW_COLD_2_LOWPOWERSTANDBY:
		case FLOW_ACTIVE_2_LOWPOWERSTANDBY:
		case FLOW_ACTIVE_2_NORMALSTANDBY:
		case FLOW_ACTIVE_2_SUSPEND:
		case FLOW_SYSRESET_2_LOWPOWERSTANDBY:
		case FLOW_SYSRESET_2_NORMALSTANDBY:
		case FLOW_SYSRESET_2_ACTIVE:
			is_linuxready = 0;
			break;
		case FLOW_SUSPEND_2_ACTIVE:
			if(msg[2] == STA_LEAVEFLOW)
				sm_send_power_key();

		case FLOW_LOWPOWERSTANDBY_2_ACTIVE:
		case FLOW_NORMALSTANDBY_2_ACTIVE:
			if(msg[2] == STA_ENTERFLOW) {
				if(!is_linuxready) {
					PRT_INFO("----IR interrupt disabled----\n");
					sm_gpio_int_disable(SM_GPIO_PORT_IR);
				}
			}
			break;
		default:
			break;
		}
	}
	return S_OK;
}

DECLARE_GPIO_ISR(SM_GPIO_PORT_IR, EDGE_SENSITIVE, FALLING_EDGE, mv_sm_ir_isr);

#define IR_STACK_SIZE ((uint16_t) 256)

static void ir_task(void * para)
{
	for ( ;; ) {
		mv_sm_ir_task(para);
		vTaskDelay(1);
	}
}

static void __attribute__((used)) create_ir_task(void)
{
	xTaskCreate(ir_task, "ir", IR_STACK_SIZE, NULL, TASK_PRIORITY_1, NULL);
}

DECLARE_RT_MODULE(
	ir,
	MV_SM_ID_IR,
	create_ir_task,
	mv_sm_process_ir_msg,
	mv_sm_process_internal_ir_msg
);

#endif
