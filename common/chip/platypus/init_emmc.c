/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright (C) 2013-2021 Synaptics Incorporated. All rights reserved.
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
#include "io.h"
#include "init_emmc.h"
#include "global.h"
#include "apbRegBase.h"
#include "apb_gpio.h"

#define SDIO0_WP_GPIO_BASE	APB_GPIO1_BASE
#define SDIO0_WP_GPIO_INDEX	16

#define I2S1_DO1_GPIO_BASE	APB_GPIO0_BASE
#define I2S1_DO1_GPIO_INDEX	17

static void soc_set_pinmux(unsigned int cntl_bus_addr, unsigned int mask, unsigned int lsb, unsigned int mode)
{
	unsigned int pinmux;

	REG_READ32(MEMMAP_CHIP_CTRL_REG_BASE + cntl_bus_addr, &pinmux);
	pinmux &= ~mask;
	pinmux |= mode << lsb;

	REG_WRITE32(MEMMAP_CHIP_CTRL_REG_BASE + cntl_bus_addr, pinmux);
}

int sdio_setup(void)
{
	soc_set_pinmux(RA_Gbl_pinMuxCntlBus2, MSK32Gbl_pinMuxCntlBus_SDIO0_WP, LSb32Gbl_pinMuxCntlBus_SDIO0_WP,
	        Gbl_pinMuxCntlBus_SDIO0_WP_MODE_1);

	/* set output */
	REG_WRITE32(SDIO0_WP_GPIO_BASE + APB_GPIO_SWPORTA_DDR, 1 << SDIO0_WP_GPIO_INDEX);
	REG_WRITE32(SDIO0_WP_GPIO_BASE + APB_GPIO_SWPORTA_DR, 1 << SDIO0_WP_GPIO_INDEX);

	/* set output */
	REG_WRITE32(I2S1_DO1_GPIO_BASE + APB_GPIO_SWPORTA_DDR, 1 << I2S1_DO1_GPIO_INDEX);
	REG_WRITE32(I2S1_DO1_GPIO_BASE + APB_GPIO_SWPORTA_DR, 1 << I2S1_DO1_GPIO_INDEX);
	return 0;
}

void phy_deassert_global_reset(struct mmc_device_info *dev_info)
{
	return;
}


