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
#include "timers.h"
#include "semphr.h"

#include "platform_config.h"

#include "sm_type.h"
#include "SysMgr.h"
#include "sm_common.h"

#include "sm_printf.h"
#include "sm_gpio.h"
#include "sm_state.h"
#include "sm_timer.h"

#include "sm_rt_module.h"
#include "sm_power.h"
#include "sm_func.h"

#ifndef WIFIBT_GAP_MIN
#define WIFIBT_GAP_MIN	10
#define WIFIBT_GAP_MAX	125
#define ETH_GAP_MAX	512
#endif

volatile BOOL is_wakeup = FALSE;

static TimerHandle_t wifireset_timer = NULL;

void sm_gpio_int_pre(int port)
{
	sm_gpio_int_disable(port);
	sm_gpio_clearint(port);
	sm_gpio_set_input(port);
}

BOOL state_to_wait_wakeup(void)
{
	MV_SM_STATE cur_state = mv_sm_get_state();

	if((MV_SM_STATE_LOWPOWERSTANDBY == cur_state) ||
		(MV_SM_STATE_NORMALSTANDBY == cur_state) ||
		(MV_SM_STATE_SUSPEND == cur_state)) {
		return TRUE;
	}

	return FALSE;
}

static void sm_wifireset_timer_isr(TimerHandle_t xtimer)
{
	board_wifi_poweron();

	if(xTimerStop(wifireset_timer, 0) == pdFAIL)
		PRT_ERROR("wifireset timer can't be stopped\n");
}

void sm_wifi_reset(void)
{
	board_wifi_poweroff();

	if (wifireset_timer != NULL) {
		if ( xTimerIsTimerActive(wifireset_timer) == pdFALSE ) {
			if(xTimerStart(wifireset_timer, pdMS_TO_TICKS(1)) == pdFAIL)
				PRT_ERROR("wifireset timer can't be started\n");
		}
	}
}

#ifdef SM_GPIO_PORT_WIFIBT
#define WIFIBT_STACK_SIZE  ((uint16_t) 128)

int wifibt_falling_time = 0;
int wifibt_gap = 0;

static TimerHandle_t wifibt_timer = NULL;
static SemaphoreHandle_t wifibt_sem = NULL;

static void sm_wifibt_timer_isr(TimerHandle_t xtimer)
{
	wifibt_gap = mv_sm_timer_gettimems_formISR() - wifibt_falling_time;

	if (sm_gpio_read(SM_GPIO_PORT_WIFIBT) == RISING_EDGE || wifibt_gap > WIFIBT_GAP_MAX) {
		is_wakeup = TRUE;

		if(wifibt_gap > (WIFIBT_GAP_MIN * 2))
			mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_BT);
		else
			mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_WIFI);
	}

	if (is_wakeup) {
		if(xTimerStop(wifibt_timer, 0) == pdFAIL)
			PRT_ERROR("wifibt timer can't be stopped\n");
		wifibt_gap = 0;
	}
}

static void __attribute__((used)) sm_wifibt_isr(void)
{
	sm_gpio_clearint(SM_GPIO_PORT_WIFIBT);

	if(wifibt_gap == 0)
		wifibt_falling_time = mv_sm_timer_gettimems_formISR();

	if ( wifibt_sem != NULL ) {
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(wifibt_sem, &xHigherPriorityTaskWoken);
	}
}

DECLARE_GPIO_ISR(SM_GPIO_PORT_WIFIBT, EDGE_SENSITIVE, FALLING_EDGE, sm_wifibt_isr);

static int __attribute__((used)) power_eth_init(void)
{
	/* Clean up interrupt & set WiFiBT wake pin direction */
	sm_gpio_int_pre(SM_GPIO_PORT_WIFIBT);

	/* Create notify semaphore */
	wifibt_sem = xSemaphoreCreateBinary();
	if (wifibt_sem == NULL)
		PRT_ERROR("wifibt semaphore create failed\n");

	/* Crate Wifibt timer */
	if (wifibt_timer == NULL) {
		// Create timer during system init
		wifibt_timer = xTimerCreate("wifibt",
				pdMS_TO_TICKS(5), // 5ms
				pdTRUE,
				0,
				sm_wifibt_timer_isr);
	}
	if (wifibt_timer == NULL)
		PRT_ERROR("wifibt timer request failed\n");

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

	sm_gpio_int_enable(SM_GPIO_PORT_WIFIBT);

	return S_OK;
}

static void wifibt_wake_task(void *para)
{
	while (1) {
		xSemaphoreTake(wifibt_sem, portMAX_DELAY);

		if(state_to_wait_wakeup()) {
			if (wifibt_timer != NULL) {
				if ( xTimerIsTimerActive(wifibt_timer) == pdFALSE ) {
					if(xTimerStart(wifibt_timer, pdMS_TO_TICKS(1)) == pdFAIL)
						PRT_ERROR("wifibt timer can't be started\n");
				}
			}
		}
	}
}

static void __attribute__((used)) create_wifibt_wake_task(void)
{
	/* Create timer trigger task */
	xTaskCreate(wifibt_wake_task, "wifibt_wake", WIFIBT_STACK_SIZE, NULL,
		TASK_PRIORITY_2, NULL);
}

DECLARE_RT_MODULE(
	power_eth,
	MV_SM_ID_ETH,
	create_wifibt_wake_task,
	NULL,
	NULL
);

#else
static void __attribute__((used)) sm_wifi_isr(void)
{
	sm_gpio_clearint(SM_GPIO_PORT_WIFI);

	if(state_to_wait_wakeup()) {
		is_wakeup = TRUE;
		mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_WIFI);
	}
}

DECLARE_GPIO_ISR(SM_GPIO_PORT_WIFI, EDGE_SENSITIVE, FALLING_EDGE, sm_wifi_isr);

static void __attribute__((used)) sm_bt_isr(void)
{
	sm_gpio_clearint(SM_GPIO_PORT_BT);

	if(state_to_wait_wakeup()) {
		is_wakeup = TRUE;
		mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_BT);
	}
}

DECLARE_GPIO_ISR(SM_GPIO_PORT_BT, EDGE_SENSITIVE, FALLING_EDGE, sm_bt_isr);

static int __attribute__((used)) power_eth_init(void)
{
	/* Clean up interrupt & set WiFiBT wake pin direction */
	sm_gpio_int_pre(SM_GPIO_PORT_WIFI);
	sm_gpio_int_pre(SM_GPIO_PORT_BT);

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

	sm_gpio_int_enable(SM_GPIO_PORT_WIFI);
	sm_gpio_int_enable(SM_GPIO_PORT_BT);

	return S_OK;
}
#endif

DECLARE_RT_INIT(power_eth, INIT_DEV_P_25, power_eth_init);

#ifdef GETHENABLE
volatile BOOL geth_wakeup = FALSE;

void sm_geth_isr(void)
{
	if(mv_sm_get_state() != MV_SM_STATE_ACTIVE){
		geth_wakeup = TRUE;
	}

	// Clear GPIO interrupt
	sm_gpio_clearint(SM_GPIO_PORT_GEPHY);
	PRT_INFO("@@@GEPHY GPIO INT!\n");
}

DECLARE_GPIO_ISR(SM_GPIO_PORT_GEPHY, EDGE_SENSITIVE, FALLING_EDGE, sm_geth_isr);
#endif

void eth_wifibt_wakeup_handler(void)
{
	if(is_wakeup) {
		mv_sm_power_enterflow_bysmstate();
		is_wakeup = FALSE;
	}

#ifdef GETHENABLE
	//FIXME: Keep original wifibt_gap for Ethernet wakeup temporarily until hw solution are fixed.
	if(geth_wakeup) {
		mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_WOL);
		mv_sm_power_enterflow_bysmstate();
		geth_wakeup = FALSE;
	}
#endif
}
