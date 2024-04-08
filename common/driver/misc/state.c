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
#include "io.h"
#include "memmap.h"

#include "SysMgr.h"
#include "system_manager.h"
#include "debug.h"

#include "misc.h"

static unsigned int bootState_uiFlag = 0;

void init_bootstate(void)
{
#ifdef CONFIG_SM
	bootState_uiFlag = readl(SM_BOOT_FLAG_REG);
#else
	bootState_uiFlag = 0x0;
#endif
	return ;
}

unsigned int get_bootstate(void)
{
	//bootState_uiFlag = readl(BOOT_STATE_REG);
	//dbg_printf(PRN_DBG, "get_bootstate: RegAddr= 0x%X, Value= 0x%x !\n", SM_BOOT_FLAG_REG, bootState_uiFlag);
	return bootState_uiFlag;
}

void set_bootstate(unsigned int state)
{
	//dbg_printf(PRN_DBG, "Set_bootstate to 0x%x !\n", state);
#ifdef CONFIG_SM
	writel(state, SM_BOOT_FLAG_REG);
#endif
	state = state;
}

bool is_resume(void)
{
	if(get_bootstate() == MV_SoC_STATE_WARMUP_1)
		return TRUE;

	return FALSE;
}

bool is_reset(void)
{
	if(get_bootstate() == MV_SoC_STATE_SYSRESET)
			return TRUE;

	return FALSE;
}

bool is_poweron(void)
{
	if(get_bootstate() == MV_SoC_STATE_SYSPOWERON)
		return TRUE;

	return FALSE;
}

bool is_coldbootup(void)
{
	if(get_bootstate() == MV_SoC_STATE_COLDBOOTUP)
		return TRUE;

	return FALSE;
}

bool is_standby_2_active(void)
{
	if(get_bootstate() == MV_SoC_STATE_WARMUP_0)
		return TRUE;

	return FALSE;
}

bool is_lowpowerstandby_2_active(void)
{
	if(get_bootstate() == MV_SoC_STATE_WARMUP_FROM_DDR_OFF_1)
		return TRUE;

	return FALSE;
}
