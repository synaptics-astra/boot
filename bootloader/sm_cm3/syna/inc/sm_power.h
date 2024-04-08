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
#ifndef __SM_POWER_H__
#define __SM_POWER_H__

typedef enum
{
	/* warmdown */
	FLOW_ACTIVE_2_LOWPOWERSTANDBY = 0,   /*  */
	FLOW_ACTIVE_2_NORMALSTANDBY,
	FLOW_ACTIVE_2_SUSPEND,

	/* warmup */
	FLOW_LOWPOWERSTANDBY_2_ACTIVE,
	FLOW_NORMALSTANDBY_2_ACTIVE,
	FLOW_SUSPEND_2_ACTIVE,

	/* reset */
	FLOW_SYSRESET_2_LOWPOWERSTANDBY,
	FLOW_SYSRESET_2_NORMALSTANDBY,
	FLOW_SYSRESET_2_ACTIVE,

	/* cold boot */
	FLOW_COLD_2_LOWPOWERSTANDBY,

	FLOW_IDLE,
}SM_POWER_FLOW;

typedef enum
{
	STA_ENTERFLOW = 0,         /* enter flow */
	STA_POWERON,               /* power on soc */
	STA_POWEROFF,              /* power on soc */
	STA_POWEROK,               /* wait soc power ok */
	STA_ASSERT,                /* assert cpu */
	STA_DEASSERT,              /* deassert cpu */
	STA_WAITRSP,               /* wait for response from bootloader */
	STA_LEAVEFLOW,             /* leave flow */
	STA_INVALID,
}POWER_STATUS;

/*SDRAM power mode*/
typedef enum
{
	SDRAM_PWR_OFF = 0,
	SDRAM_PWR_ON,
	SDRAM_PWR_REFRESH,
}SDRAM_PWR_MODE;

typedef enum {
	UNKNOW = 0x0,
	REBOOT = 0x2,
	WATCHDOG = 0x4,
	KERNEL_PANIC = 0x8,
	NORMAL_FLOW = 0xFF,
}REBOOT_REASON_STATE;

struct wakeup_info {
	int wakeup_source;
	unsigned int payload[2];
};

/* power op */
int soc_assert(void);
int soc_deassert(void);
int peripheral_assert(void);
int peripheral_deassert(void);
int mv_sm_power_socpoweron(void);
int mv_sm_power_socpoweroff(void);
int mv_sm_power_soc_power_status(void);

/* wakeup / power down by timer*/
int sm_setup_power_up_timer(int msg);
int sm_get_warmup_remain_time(void);
int sm_setup_power_down_timer(int msg);

void sm_set_reboot_reason(UINT32 reboot_reason);

void sm_wifi_reset(void);
void eth_wifibt_wakeup_handler(void);
void fpbutton_wakeup_handler(void);

/* misc thing related with power */
#ifdef FPSTBYENABLE
void mv_sm_fpstby_isr(void);
#endif

BOOL mv_sm_get_linux_state(void);

hresult mv_sm_power_enterflow(SM_POWER_FLOW flow);
hresult mv_sm_power_enterflow_bysmstate(void);

hresult mv_sm_power_setwakeupsource(INT32 ws);
void mv_sm_power_setwakeuppayload(unsigned int value);

extern volatile BOOL is_wakeup;
#ifdef GETHENABLE
extern volatile BOOL geth_wakeup;
#endif
#ifdef FPSTBYENABLE
extern volatile BOOL fpbutton_wakeup;
#endif
#endif
