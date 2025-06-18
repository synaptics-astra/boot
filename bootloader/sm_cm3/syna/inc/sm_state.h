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
#ifndef __SM_STATE_H__
#define __SM_STATE_H__

/* FIXME: refine the state */
//define System Manager state
typedef enum
{
	// basic state
	MV_SM_STATE_ACTIVE = 0x0,
	MV_SM_STATE_LOWPOWERSTANDBY,
	MV_SM_STATE_NORMALSTANDBY,
	MV_SM_STATE_SUSPEND,

	MV_SM_STATE_ACTIVE_2_LOWPOWERSTANDBY = 0x10,
	MV_SM_STATE_ACTIVE_2_NORMALSTANDBY,
	MV_SM_STATE_ACTIVE_2_SUSPEND,

	MV_SM_STATE_STANDBY_2_ACTIVE = 0x20,

	MV_SM_STATE_COLD = 0xF0,
}MV_SM_STATE;

//FIXME: it's a message type. don't put it here
#define MV_SM_POWER_BOOT_MODE 8 // doubles as a message type code from bootloader to SM, telling normal/recovery


//define soc state to notify bootflow
typedef enum
{
	MV_SOC_STATE_POWERON = 0,

	MV_SOC_STATE_ACTIVE_2_LOWPOWERSTANDBY = 0x10, // not defined in old
	MV_SOC_STATE_ACTIVE_2_NORMALSTANDBY = 4, //MV_SoC_STATE_WARMDOWN_1
	MV_SOC_STATE_ACTIVE_2_SUSPEND = 0xA0,  // MV_SoC_STATE_WARMDOWN_2
	MV_SOC_STATE_SUSPEND_REQUEST = 0xA1,

	MV_SOC_STATE_SYSRESET_2_ACTIVE = 0xFF,
	MV_SOC_STATE_SYSRESET_2_LOWPOWERSTANDBY = 0x21,
	MV_SOC_STATE_SYSRESET_2_NORMALSTANDBY,

	MV_SOC_STATE_COLD_2_ACTIVE = 2, //MV_SoC_STATE_WARMUP_0 (deprecated)
	MV_SOC_STATE_LOWPOWERSTANDBY_2_ACTIVE = 7, //MV_SoC_STATE_WARMUP_FROM_DDR_OFF_1
	MV_SOC_STATE_NORMALSTANDBY_2_ACTIVE = 0x30, // not defined in old
	MV_SOC_STATE_SUSPEND_2_ACTIVE = 6, //MV_SoC_STATE_WARMUP_1
}MV_SOC_STATE;

void mv_sm_set_state(INT32 state);
int mv_sm_get_state();
void mv_sm_set_boot_flag(UINT32 bootflag);
unsigned int mv_sm_get_boot_flag(void);

#endif /* __SM_STATE_H__ */
