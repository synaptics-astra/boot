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
#include "FreeRTOS.h"
#include "task.h"

#include "sm_type.h"
#include "SysMgr.h"
#include "sm_common.h"

#include "sm_comm.h"

#include "sm_state.h"
#include "sm_timer.h"

#include "sm_printf.h"
#include "sm_power.h"

#include "sm_rt_module.h"

#include "sm_ir_key_def.h"
#include "timers.h"

static int wmdn_timer_enable = 0, wmup_timer_enable = 0;

static timer_long_t wmdn_timer_begin, wmup_timer_begin;
static int wmdn_timer_interval, wmup_timer_interval;

static TimerHandle_t wmup_Timer = NULL;
static TimerHandle_t wmdn_Timer = NULL;

static void wakeup_timer( TimerHandle_t xTimer )
{
	int state = mv_sm_get_state();
	timer_long_t cur;

	cur.ltime = mv_sm_timer_gettimems();

	PRT_INFO("power up time out. time begin is %d, interval is %d current is %d\n",
			wmup_timer_begin.low, wmup_timer_interval, cur.low);

	// delete the timer
	if(xTimerDelete(wmup_Timer, 0) == pdFAIL) {
		PRT_ERROR("can't delete the wake up timer\n");
	} else {
		wmup_Timer = NULL;
	}

	// clear the flags
	wmup_timer_enable = 0;
	wmup_timer_begin.ltime = 0;
	wmup_timer_interval = 0;

	// if system is already up, ignore it
	if(state == MV_SM_STATE_ACTIVE) {
		int tmp = 0;
		PRT_INFO("system is in ACTIVE state so send out msg!!");
		sm_send_msg(MV_SM_ID_POWER, (unsigned char*)(&tmp), sizeof(tmp));
		return;
	}

	// set wake up source and go to wake up flow
	mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_TIMER);
	mv_sm_power_enterflow_bysmstate();
}

int sm_setup_power_up_timer(int msg)
{
	if((msg == 0) && wmup_timer_enable) { // cancel the original timer
		if(wmup_Timer) {
			if(xTimerDelete(wmup_Timer, 0) == pdFAIL) {
				PRT_ERROR("wake up timer can't be stopped!\n");
				return S_FALSE;
			}
			wmup_Timer = NULL;
			wmup_timer_enable = 0;
			wmup_timer_begin.ltime = 0;
			wmup_timer_interval = 0;
		}
	} else if (0 != msg) {
		if(wmup_timer_enable == 0) {
			wmup_timer_enable = 1;
			wmup_timer_begin.ltime = mv_sm_timer_gettimems();
			wmup_timer_interval = msg * 1000; // s -> ms
			// create and start the timer
			wmup_Timer = xTimerCreate("wmupTimer",
										wmup_timer_interval,
										pdFALSE,
										0,
										wakeup_timer);
			if(xTimerStart(wmup_Timer, 0) == pdFAIL) {
				PRT_ERROR("wake up timer can't be start!\n");
				wmup_timer_enable = 0;
				wmup_timer_begin.ltime = 0;
				wmup_timer_interval = 0;
				return S_FALSE;
			}
		} else {
			// FIXME: if a timer was already setup, what's the strategy
		}
	}
	return S_OK;
}

int sm_get_warmup_remain_time(void)
{
	int remain_time = 0;
	remain_time = mv_sm_timer_gettimems() - wmup_timer_begin.ltime;
	remain_time = wmup_timer_interval - remain_time;
	remain_time /= 1000;
	return remain_time;
}

static void powerdown_timer( TimerHandle_t xTimer )
{
	int state = mv_sm_get_state();
	timer_long_t cur;
	unsigned int uiKey;

	cur.ltime = mv_sm_timer_gettimems();
	// clear the flags
	wmdn_timer_enable = 0;
	wmdn_timer_begin.ltime = 0;
	wmdn_timer_interval = 0;

	PRT_INFO("power down time out. time begin is %d, interval is %d current is %d\n",
			wmdn_timer_begin.low, wmdn_timer_interval, cur.low);

	// delete the timer
	if(xTimerDelete(wmdn_Timer, 0) == pdFAIL) {
		PRT_ERROR("can't delete the wake up timer\n");
	} else {
		wmdn_Timer = NULL;
	}

	// if system is up, send power key to down
	if(state == MV_SM_STATE_ACTIVE)	{
		uiKey = MV_IR_KEY_POWER;
		sm_send_msg(MV_SM_ID_IR, (unsigned char*)(&uiKey), sizeof(uiKey));
		uiKey = MV_IR_KEY2UPKEY(uiKey);
		sm_send_msg(MV_SM_ID_IR, (unsigned char*)(&uiKey), sizeof(uiKey));
	} else {
		PRT_INFO("system is not in ACTIVE state so ignore powerdown!!");
	}
}

int sm_setup_power_down_timer(int msg)
{
	if((msg == 0) && wmdn_timer_enable) { // cancel the original timer
		if(wmdn_Timer) {
			if(xTimerDelete(wmdn_Timer, 0) == pdFAIL) {
				PRT_ERROR("wake up timer can't be stopped!\n");
				return S_FALSE;
			}
			wmdn_Timer = NULL;
			wmdn_timer_enable = 0;
			wmdn_timer_begin.ltime = 0;
			wmdn_timer_interval = 0;
		}
	} else if (0 != msg) {
		if(wmdn_timer_enable == 0) {
			wmdn_timer_enable = 1;
			wmdn_timer_begin.ltime = mv_sm_timer_gettimems();
			wmdn_timer_interval = msg * 1000; // s -> ms
			// create and start the timer
			wmdn_Timer = xTimerCreate("wmdnTimer",
										wmdn_timer_interval,
										pdFALSE,
										0,
										powerdown_timer
				);
			if(xTimerStart(wmdn_Timer, 0) == pdFAIL) {
				PRT_ERROR("power down timer can't be start!\n");
				wmdn_timer_enable = 0;
				wmdn_timer_begin.ltime = 0;
				wmdn_timer_interval = 0;
				return S_FALSE;
			}
		} else {
			// FIXME: if a timer was already setup, what's the strategy
		}
	}
	return S_OK;
}
