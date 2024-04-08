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

#include "sm_type.h"
#include "sm_printf.h"
#include "sm_func.h"
#include "sm_fxl6408.h"
#include "sm_gpio.h"

#include "platform_config.h"

void board_sdram_poweroff(void)
{
	if(sm_fxl16408_gpios_write(I2C_MASTER_ID, I2C_SLAVE0_ADDR, \
			(1 << GPIO_VDDM), \
			(VDDM_OFF << GPIO_VDDM)) != 0)
		PRT_ERROR("%s: failed\n", __func__);
}

void board_sdram_poweron(void)
{
	if(sm_fxl16408_gpios_write(I2C_MASTER_ID, I2C_SLAVE0_ADDR, \
			(1 << GPIO_VDDM), \
			(VDDM_ON << GPIO_VDDM)) != 0)
		PRT_ERROR("%s: failed\n", __func__);
}

void board_sdram_refresh(void)
{
	// No need to power off VDDM_LPQ on DDR4

	return;
}

void board_peripheral_assert(void)
{
	if(sm_fxl16408_gpios_write(I2C_MASTER_ID, I2C_SLAVE1_ADDR, \
			(1 << GPIO_PERI), \
			(PERI_OFF << GPIO_PERI)) != 0)
		PRT_ERROR("%s: failed\n", __func__);
}

void board_peripheral_deassert(void)
{
	if(sm_fxl16408_gpios_write(I2C_MASTER_ID, I2C_SLAVE1_ADDR, \
			(1 << GPIO_PERI), \
			(PERI_ON << GPIO_PERI)) != 0)
		PRT_ERROR("%s: failed\n", __func__);
}

void board_soc_poweron(void)
{
	if(sm_fxl16408_gpios_write(I2C_MASTER_ID, I2C_SLAVE0_ADDR, \
			((1 << GPIO_VCPU) | (1 << GPIO_VCORE)), \
			((VCPU_ON << GPIO_VCPU) | (VCORE_ON << GPIO_VCORE))) != 0)
		PRT_ERROR("%s: failed\n", __func__);
}

void board_soc_poweroff(void)
{
	if(sm_fxl16408_gpios_write(I2C_MASTER_ID, I2C_SLAVE0_ADDR, \
			((1 << GPIO_VCPU) | (1 << GPIO_VCORE)), \
			((VCPU_OFF << GPIO_VCPU) | (VCORE_OFF << GPIO_VCORE))) != 0)
		PRT_ERROR("%s: failed\n", __func__);
}

void board_wifi_poweron(void)
{
	if(sm_fxl16408_gpios_write(I2C_MASTER_ID, I2C_SLAVE1_ADDR,
		(1 << GPIO_WIFI), (WIFI_ON << GPIO_WIFI)) != 0)
		PRT_ERROR("%s: failed\n", __func__);
}

void board_wifi_poweroff(void)
{
	if(sm_fxl16408_gpios_write(I2C_MASTER_ID, I2C_SLAVE1_ADDR,
		(1 << GPIO_WIFI), (WIFI_OFF << GPIO_WIFI)) != 0)
		PRT_ERROR("%s: failed\n", __func__);
}
