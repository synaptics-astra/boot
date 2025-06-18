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
#ifndef __SM_APB_WDT_DEFS_H__
#define __SM_APB_WDT_DEFS_H__

/* WDT 0, WDT 1, WDT2 */
static unsigned int apb_wdt_base[] =
	{
		SM_SM_APB_WDT0_BASE,
		SM_SM_APB_WDT1_BASE,
		SM_SM_APB_WDT2_BASE
	};
#define APB_WDT_BASE(x) (apb_wdt_base[x])

#define WDT_WDTControlReg_Offset               0x00
#define WDT_WDTTimeoutRangeReg_Offset          0x04
#define WDT_WDTCurrentCounterValueReg_Offset   0x08
#define WDT_WDTCounterResetReg_Offset          0x0C
#define WDT_WDTStatReg_Offset                  0x10
#define WDT_WDTEOIReg_Offset                   0x14
#define WDT_WDTVIDReg_Offset                   0x18

#define WDT_WDTCOMP_PARAMS_5_Offset            0xe4
#define WDT_WDTCOMP_PARAMS_4_Offset            0xe8
#define WDT_WDTCOMP_PARAMS_3_Offset            0xec
#define WDT_WDTCOMP_PARAMS_2_Offset            0xf0
#define WDT_WDTCOMP_PARAMS_1_Offset            0xf4
#define WDT_WDTCOMP_VERSION_Offset             0xf8
#define WDT_WDTCOMP_TYPE_Offset                0xfc

#ifdef QUICKSTART_WDT
#define WDT_WDTControlReg(x)             (APB_WDT_BASE(x) + WDT_WDTControlReg_Offset            )
#define WDT_WDTTimeoutRangeReg(x)        (APB_WDT_BASE(x) + WDT_WDTTimeoutRangeReg_Offset       )
#define WDT_WDTCurrentCounterValueReg(x) (APB_WDT_BASE(x) + WDT_WDTCurrentCounterValueReg_Offset)
#define WDT_WDTCounterResetReg(x)        (APB_WDT_BASE(x) + WDT_WDTCounterResetReg_Offset       )
#define WDT_WDTStatReg(x)                (APB_WDT_BASE(x) + WDT_WDTStatReg_Offset               )
#define WDT_WDTEOIReg(x)                 (APB_WDT_BASE(x) + WDT_WDTEOIReg_Offset                )
#define WDT_WDTVIDReg(x)                 (APB_WDT_BASE(x) + WDT_WDTVIDReg_Offset                )
#endif

#define WDT_WDT_CR(x)                    (APB_WDT_BASE(x) + WDT_WDTControlReg_Offset            )
#define WDT_WDT_TORR(x)                  (APB_WDT_BASE(x) + WDT_WDTTimeoutRangeReg_Offset       )
#define WDT_WDT_CCVR(x)                  (APB_WDT_BASE(x) + WDT_WDTCurrentCounterValueReg_Offset)
#define WDT_WDT_CRR(x)                   (APB_WDT_BASE(x) + WDT_WDTCounterResetReg_Offset       )
#define WDT_WDT_STAT(x)                  (APB_WDT_BASE(x) + WDT_WDTStatReg_Offset               )
#define WDT_WDT_EOI(x)                   (APB_WDT_BASE(x) + WDT_WDTEOIReg_Offset                )
#define WDT_WDT_VID(x)                   (APB_WDT_BASE(x) + WDT_WDTVIDReg_Offset                )
#define WDT_WDT_COMP_PARAMS_5(x)         (APB_WDT_BASE(x) + WDT_WDTCOMP_PARAMS_5_Offset         )
#define WDT_WDT_COMP_PARAMS_4(x)         (APB_WDT_BASE(x) + WDT_WDTCOMP_PARAMS_4_Offset         )
#define WDT_WDT_COMP_PARAMS_3(x)         (APB_WDT_BASE(x) + WDT_WDTCOMP_PARAMS_3_Offset         )
#define WDT_WDT_COMP_PARAMS_2(x)         (APB_WDT_BASE(x) + WDT_WDTCOMP_PARAMS_2_Offset         )
#define WDT_WDT_COMP_PARAMS_1(x)         (APB_WDT_BASE(x) + WDT_WDTCOMP_PARAMS_1_Offset         )
#define WDT_WDT_COMP_VERSION(x)          (APB_WDT_BASE(x) + WDT_WDTCOMP_VERSION_Offset          )
#define WDT_WDT_COMP_TYPE(x)             (APB_WDT_BASE(x) + WDT_WDTCOMP_TYPE_Offset             )

#endif	//#define __SM_APB_WDT_DEFS_H__
