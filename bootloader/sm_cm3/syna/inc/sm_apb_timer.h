/* SPDX-License-Identifier: GPL-2.0+ */
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
#ifndef __SM_APB_TIMER_H__
#define __SM_APB_TIMER_H__

#define TIMER_MS(x)	(x * 1000 * SM_SYSTEM_HZ)
#define TIMER_US(x)	(x * SM_SYSTEM_HZ)
#define MAX_MS		(0xFFFFFFFF / (SM_SYSTEM_HZ*1000))
#define MAX_TIMER_MS (0xFFFFFFFF)
#define INT_MS		(1000)

#define TIMER_ID_MAX	8

#define TIMER_ID_IDX_END	3
#define TIMER_ID_IDX_START	1

#define SYS_TIMER_ID		1
//#define RESERVE_TIMER_ID	2

#define INT_MS				(1000)

void sm_apb_timer_init(INT32 min_id, INT32 max_id);
void sm_apb_set_sys_timer_id(INT32 sys_timer_id);
INT32 sm_apb_get_sys_timer_id(void);
INT32 sm_apb_timer_reserve(INT32 timerid);
INT32 sm_apb_timer_readclock(void);
INT32 sm_apb_timer_gettimems(void);
INT32 sm_apb_timer_getlongtimems(timer_long_t *ret);
INT32 sm_apb_timer_request(void);
INT32 sm_apb_timer_start(INT32 timerid, INT32 timeoutus, void (*cb)(void), BOOL is_freerun);
INT32 sm_apb_timer_stop(INT32 timerid);
void sm_apb_timer_isr_timer1(void);

#endif

