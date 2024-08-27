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
#include "sm_uart.h"

#define UART_STACK_SIZE  ((uint16_t) 128)

static void uart_wake_task(void *para)
{
	MV_SM_STATE state;

	while (1) {
		state = mv_sm_get_state();

		if((MV_SM_STATE_LOWPOWERSTANDBY == state) ||
			(MV_SM_STATE_NORMALSTANDBY == state) ||
			(MV_SM_STATE_SUSPEND == state)) {
			if (UART_receive_wake_up_signal()) {
				if(mv_sm_power_enterflow_bysmstate() == S_OK) {
					PRT_INFO("Wake up from UART\n");
					mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_WOL);
				}
			}
		}
		vTaskDelay(1000);		
	}
}

static void __attribute__((used)) create_uart_wake_task(void)
{
	/* Create timer trigger task */
	xTaskCreate(uart_wake_task, "uart_wake", UART_STACK_SIZE, NULL,
		TASK_PRIORITY_2, NULL);
}

DECLARE_RT_MODULE(
	power_uart,
	MV_SM_ID_UART,
	create_uart_wake_task,
	NULL,
	NULL
);
