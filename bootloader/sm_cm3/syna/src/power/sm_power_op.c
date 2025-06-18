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
#include "sm_io.h"
#include "SysMgr.h"
#include "sm_common.h"
#include "sm_timer.h"

#include "platform_config.h"
#include "sm_printf.h"
#include "sm_power.h"

#include "sm_gpio.h"

#include "sm_func.h"

int soc_assert(void)
{
	T32smSysCtl_SM_RST_CTRL reg;
	T32smSysCtl_SM_CTRL reg_ctrl;

	//enable ISO_EN before power off soc
	reg_ctrl.u32=MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_CTRL);
	reg_ctrl.uSM_CTRL_ISO_EN = 1;
	MV_SM_WRITE_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_CTRL, reg_ctrl.u32);

	// assert SoC reset
	reg.u32=MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_RST_CTRL);
	reg.uSM_RST_CTRL_SOC_RST_GO = 0;
	MV_SM_WRITE_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_RST_CTRL, reg.u32);
	PRT_INFO("%s done\n", __func__);

	return S_OK;
}

int soc_deassert(void)
{
	T32smSysCtl_SM_RST_CTRL reg;
	T32smSysCtl_SM_CTRL reg_ctrl;

	// deassert SoC reset
	reg.u32=MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_RST_CTRL);
	reg.uSM_RST_CTRL_SOC_RST_GO = 1;
	MV_SM_WRITE_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_RST_CTRL, reg.u32);

	//enable ISO_EN before power off soc
	reg_ctrl.u32=MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_CTRL);
	reg_ctrl.uSM_CTRL_ISO_EN = 0;
	MV_SM_WRITE_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_CTRL, reg_ctrl.u32);
	PRT_INFO("%s done\n", __func__);

	return S_OK;
}

int peripheral_assert(void)
{
	board_peripheral_assert();
	PRT_INFO("%s done\n", __func__);
	return S_OK;
}

int peripheral_deassert(void)
{
	board_peripheral_deassert();
	PRT_INFO("%s done\n", __func__);
	return S_OK;
}


int mv_sm_power_socpoweron(void)
{
	board_soc_poweron();
	PRT_INFO("%s done\n", __func__);
	return S_OK;
}

int mv_sm_power_socpoweroff(void)
{
	soc_assert();
	peripheral_assert();
	board_soc_poweroff();

	// Add IR int enable as it is disabled during warmup flow, only enabled when IR ready
	sm_gpio_int_enable(SM_GPIO_PORT_IR);

	PRT_INFO("%s done\n", __func__);

	return S_OK;
}

int mv_sm_power_soc_power_status(void)
{
	// if there is any hardware/register through which we can
	// check the power status of soc
	//return board_soc_power_status();
	return S_OK;
}
