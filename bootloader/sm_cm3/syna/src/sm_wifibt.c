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
#include "sm_type.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "platform_config.h"

#include "sm_common.h"

#include "sm_power.h"
#include "sm_timer.h"
#include "sm_printf.h"
#include "sm_func.h"

#include "sm_gpio.h"
#include "sm_rt_module.h"

static TimerHandle_t wifireset_timer = NULL;

#ifdef SM_GPIO_WIFIBT_POWER

static hresult mv_sm_wifibt_power_ctrl(BOOL on)
{
	if(on) {
		sm_gpio_set_output(SM_GPIO_WIFIBT_POWER, 1);
	} else {
		sm_gpio_set_output(SM_GPIO_WIFIBT_POWER, 0);
	}
	PRT_INFO("WIFI/BT power %s, gpio: %d\n", on?"on":"off", SM_GPIO_WIFIBT_POWER);
	return S_OK;
}

#else

static hresult mv_sm_wifibt_power_ctrl(BOOL on)
{
	return S_OK;
}

#endif

static void sm_wifireset_timer_isr(TimerHandle_t xtimer)
{
	board_wifi_poweron();

	if(xTimerStop(wifireset_timer, 0) == pdFAIL)
		PRT_ERROR("wifireset timer can't be stopped\n");
}

static void sm_wifi_reset(void)
{
	board_wifi_poweroff();

	if (wifireset_timer != NULL) {
		if ( xTimerIsTimerActive(wifireset_timer) == pdFALSE ) {
			if(xTimerStart(wifireset_timer, pdMS_TO_TICKS(1)) == pdFAIL)
				PRT_ERROR("wifireset timer can't be started\n");
		}
	}
}

hresult mv_sm_process_internal_wifibt_msg(void * data, INT32 len)
{
	UINT32 *msg = (UINT32*)data;

	if(msg[0] == MV_SM_ID_POWER) {
		switch(msg[1]) {
		case FLOW_COLD_2_LOWPOWERSTANDBY:
		case FLOW_ACTIVE_2_LOWPOWERSTANDBY:
			if(msg[2] == STA_LEAVEFLOW)
				mv_sm_wifibt_power_ctrl(0);
			break;
		case FLOW_LOWPOWERSTANDBY_2_ACTIVE:
			if(msg[2] == STA_ENTERFLOW)
				mv_sm_wifibt_power_ctrl(1);
			break;
		case FLOW_SYSRESET_2_ACTIVE:
			if(msg[2] == STA_ENTERFLOW)
				sm_wifi_reset();
			break;
		default:
			break;
		}
	}
	return S_OK;
}

DECLARE_RT_MODULE(
	pwifibt,
	MV_SM_ID_WIFIBT,
	NULL,
	NULL,
	mv_sm_process_internal_wifibt_msg
);

static int __attribute__((used)) power_wifibt_init(void)
{
	/* Crate Wifireset timer */
	if (wifireset_timer == NULL) {
		// Create timer during system init
		wifireset_timer = xTimerCreate("wifireset",
				pdMS_TO_TICKS(200), // 200ms
				pdFAIL,
				0,
				sm_wifireset_timer_isr);
	}
	if (wifireset_timer == NULL)
		PRT_ERROR("wifireset timer request failed\n");

	return S_OK;
}
DECLARE_RT_INIT(power_wifibt, INIT_DEV_P_25, power_wifibt_init);