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
#ifndef __SM_IR_XMP_KEY_TABLE_H__
#define __SM_IR_XMP_KEY_TABLE_H__

#include "ir_key_table_def.h"

#ifdef XMP_PROTOCAL
#ifdef XMP_SYNA_RC
/* define vendor ir key table */
static ir_key_table_t vendor_ir_xmp_table[] = {
	{8, 0xff, 0x0F, MV_IR_KEY_POWER},
	{8, 0xff, 0x90, MV_IR_KEY_TVPOWER},

	{8, 0xff, 0x0C, MV_IR_KEY_VOL_MUTE},
	{8, 0xff, 0x0A, MV_IR_KEY_VOL_PLUS},
	{8, 0xff, 0x0B, MV_IR_KEY_VOL_MINUS},
	{8, 0xff, 0x0D, MV_IR_KEY_CHANNEL_PLUS},
	{8, 0xff, 0x0E, MV_IR_KEY_CHANNEL_MINUS},
	{8, 0xff, 0x53, MV_IR_KEY_BACK},

	{8, 0xff, 0x30, MV_IR_KEY_PLAY},
	{8, 0xff, 0x34, MV_IR_KEY_FAST_FORWARD},
	{8, 0xff, 0x33, MV_IR_KEY_FAST_BACKWARD},

	{8, 0xff, 0x2A, MV_IR_KEY_EXIT},
	{8, 0xff, 0xCF, MV_IR_KEY_SEARCH},
	{8, 0xff, 0x35, MV_IR_KEY_REC},

	{8, 0xff, 0x27, MV_IR_KEY_GUIDE},
	{8, 0xff, 0x92, MV_IR_KEY_MENU}, //RECHECK FOR XFINITY
	{8, 0xff, 0x28, MV_IR_KB_PAGEUP},

	{8, 0xff, 0x21, MV_IR_KEY_UP},
	{8, 0xff, 0x22, MV_IR_KEY_DOWN},
	{8, 0xff, 0x23, MV_IR_KEY_LEFT},
	{8, 0xff, 0x24, MV_IR_KEY_RIGHT},
	{8, 0xff, 0x25, MV_IR_KEY_ENTER},

	{8, 0xff, 0x51, MV_IR_KEY_LAST},
	{8, 0xff, 0x26, MV_IR_KEY_INFO},
	{8, 0xff, 0x29, MV_IR_KB_PAGEDOWN},

	{8, 0xff, 0x60, MV_IR_KEY_A},
	{8, 0xff, 0x61, MV_IR_KEY_B},
	{8, 0xff, 0x62, MV_IR_KEY_C},
	{8, 0xff, 0x63, MV_IR_KEY_D},

	{8, 0xff, 0x01, MV_IR_KEY_DIGIT_1},
	{8, 0xff, 0x02, MV_IR_KEY_DIGIT_2},
	{8, 0xff, 0x03, MV_IR_KEY_DIGIT_3},
	{8, 0xff, 0x04, MV_IR_KEY_DIGIT_4},
	{8, 0xff, 0x05, MV_IR_KEY_DIGIT_5},
	{8, 0xff, 0x06, MV_IR_KEY_DIGIT_6},
	{8, 0xff, 0x07, MV_IR_KEY_DIGIT_7},
	{8, 0xff, 0x08, MV_IR_KEY_DIGIT_8},
	{8, 0xff, 0x09, MV_IR_KEY_DIGIT_9},
	{8, 0xff, 0x00, MV_IR_KEY_DIGIT_0},

	{8, 0xff, 0x92, MV_IR_KEY_SETUPMENU},
	{8, 0xff, 0x57, MV_IR_KEY_TV_INPUT},

	{0, 0xff, 0xff, MV_IR_KEY_NULL},
};
#endif //CONFIG_XMP_SYNA_RC
#endif //XMP_PROTOCAL

#endif //__SM_IR_XMP_KEY_TABLE_H__
#endif //IRENABLE