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

#ifdef IRENABLE
#ifndef __SM_IR_NEC_KEY_TABLE_H__
#define __SM_IR_NEC_KEY_TABLE_H__

#include "ir_key_table_def.h"

#ifdef TC9012_PROTOCAL
// just a fake definition
#ifdef TC9012_SYNA_RC
static ir_key_table_t vendor_ir_tc9012_table[] = {
	{0, 0x7ff, 0x00c, MV_IR_KEY_POWER},

	{0, 0x7ff, 0x001, MV_IR_KEY_DIGIT_1},
	{0, 0x7ff, 0x002, MV_IR_KEY_DIGIT_2},
	{0, 0x7ff, 0x003, MV_IR_KEY_DIGIT_3},
	{0, 0x7ff, 0x004, MV_IR_KEY_DIGIT_4},
	{0, 0x7ff, 0x005, MV_IR_KEY_DIGIT_5},
	{0, 0x7ff, 0x006, MV_IR_KEY_DIGIT_6},
	{0, 0x7ff, 0x007, MV_IR_KEY_DIGIT_7},
};
#endif
#endif

#endif
#endif
