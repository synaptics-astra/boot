/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright © 2013-2020 Synaptics Incorporated. All rights reserved.
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
#ifdef GPIOBTNENABLE
#include "sm_common.h"
#include "sm_type.h"
#include "sm_rt_module.h"

#include "sm_ir_key_def.h"

#include "platform_config.h"

#include "sm_printf.h"
#include "sm_gpio.h"
#include "sm_apb_gpio_defs.h"
#include "sm_apb_ictl_defs.h"

#include "sm_timer.h"
#include "sm_state.h"
#include "sm_power.h"
#include "sm_comm.h"

#include "sm_gpiobtn.h"

#define SM_GPIO_PORT_GPIOBTN 9

#define GPIOBTN_STACK_SIZE ((uint16_t) 256)

static INT32 press_time = 0;		// when was the connect button pressed? For catching press-and-hold condition.
static INT32 long_press_time = 0;	// if it has been pressed for a long time which indicate the connect or reset.

static void handle_gpiobtn_press(unsigned int dur)
{
	UINT32 iMsg;
	UINT32 sys_state = mv_sm_get_state();
	// when we're warmed down (standby, suspend), any press of CONNECT means turn on.
	if (MV_SM_STATE_LOWPOWERSTANDBY == sys_state ||
	    MV_SM_STATE_NORMALSTANDBY == sys_state ||
	    MV_SM_STATE_SUSPEND == sys_state) {
		PRT_DEBUG("turn on from standby or suspend.\n");
		mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_BUTTON);
		mv_sm_power_enterflow_bysmstate();
	} else {
		if (mv_sm_get_linux_state() == TRUE) {// should send key after linux ready
			if(dur > SHORT_PRESS_ACTION_TIME && dur < LONG_PRESS_ACTION_TIME) {
				PRT_INFO("SM sent MV_IR_KEY_BEGIN_PAIRING to ir.c\n");
				iMsg = MV_IR_KEY_BEGIN_PAIRING;
				sm_send_msg( MV_SM_ID_IR, (unsigned char*)(&iMsg), sizeof(iMsg));
				iMsg |= 0x8000000;
				sm_send_msg( MV_SM_ID_IR, (unsigned char*)(&iMsg), sizeof(iMsg) );
			} else if (dur > LONG_PRESS_ACTION_TIME ) {	//long press, when box is on, means reboot into recovery mode.
				if (dur < LONG_PRESS_TIMEOUT) {
					PRT_DEBUG("SM sent MV_IR_KEY_RECOVERY to ir.c\n");
					iMsg = MV_IR_KEY_RECOVERY;
					sm_send_msg( MV_SM_ID_IR, (unsigned char*)(&iMsg), sizeof(iMsg) );
					iMsg |= 0x8000000;
					sm_send_msg( MV_SM_ID_IR, (unsigned char*)(&iMsg), sizeof(iMsg) );
				} else {
					//PRT_DEBUG("too long keypress, ignored.\n");
				}
			}
		} else {
			PRT_INFO("linux is not ready, do nothing.\n");
		}
	}
	long_press_time = 0;		// we've handled this press.
}

static int sm_gpiobtn_task_entry(void* data)
{
	INT32  dur = 0;
	INT32 curr_time = mv_sm_timer_gettimems();

	if (long_press_time) {
		handle_gpiobtn_press(long_press_time);
	} else {
		if (press_time != 0) {
			PRT_DEBUG("last press time = %d\n", press_time);
			dur = curr_time - press_time;
			if (press_time == 0)	//had been handle in isr, return
				return S_OK;
			PRT_DEBUG("the duration is %d\n", dur);
			if (dur > LONG_PRESS_ACTION_TIME) {
				sm_gpio_set_polarity(SM_GPIO_PORT_GPIOBTN, FALLING_EDGE);
				handle_gpiobtn_press(dur);
				press_time = 0;
			}
		}
	}

	return S_OK;
}

static void sm_gpiobtn_task(void* data)
{
	for ( ;; ) {
		sm_gpiobtn_task_entry(data);
		vTaskDelay(1);
	}
}

static void __attribute__((used)) create_gpiobtn_task(void)
{
	xTaskCreate(sm_gpiobtn_task, "gpiobtn", GPIOBTN_STACK_SIZE, NULL, TASK_PRIORITY_2, NULL);
}

DECLARE_RT_MODULE(
	gpiobtn,
	MV_SM_ID_GPIOBTN,
	create_gpiobtn_task,
	NULL,
	NULL
);

static int __attribute__((used)) sm_gpiobtn_init(void)
{
	//sm_gpio_set_int(SM_GPIO_PORT_GPIOBTN, EDGE_SENSITIVE, FALLING_EDGE);

	return S_OK;
}

DECLARE_RT_INIT(gpiobtninit, INIT_DEV_P_25, sm_gpiobtn_init);

static void __attribute__((used)) sm_gpiobtn_isr(void)
{
	UINT32 pol = 0;
	INT32 dur = 0;
	INT32 release_time = 0;

	pol = sm_gpio_get_polarity(SM_GPIO_PORT_GPIOBTN);
	if (FALLING_EDGE == pol) {
		if (press_time == 0) {
			press_time = mv_sm_timer_gettimems();
			PRT_DEBUG("key pressed.press time = %d\n", press_time);
			long_press_time = 0;

			sm_gpio_set_polarity(SM_GPIO_PORT_GPIOBTN, RISING_EDGE);
		} else {
			PRT_DEBUG("we missed a rising edge interrupt.\n");
			press_time = 0;
		}
	} else {
		release_time = mv_sm_timer_gettimems();
		PRT_DEBUG("key released.release time = %d\n", release_time);
			dur = release_time - press_time;
			PRT_DEBUG("duration time = %d\n", dur);
		if (dur > SHORT_PRESS_THRESHOLD) {
			long_press_time = dur;
		}

		sm_gpio_set_polarity(SM_GPIO_PORT_GPIOBTN, FALLING_EDGE);

		press_time = 0;
	}
	sm_gpio_clearint(SM_GPIO_PORT_GPIOBTN);
}

DECLARE_GPIO_ISR(SM_GPIO_PORT_GPIOBTN, EDGE_SENSITIVE, FALLING_EDGE, sm_gpiobtn_isr);

#endif
