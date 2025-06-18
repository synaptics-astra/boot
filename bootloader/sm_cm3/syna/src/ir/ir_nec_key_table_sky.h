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
#ifndef __SM_IR_NEC_KEY_TABLE_SKY_H__
#define __SM_IR_NEC_KEY_TABLE_SKY_H__

#include "ir_key_table_def.h"

#ifdef NEC_PROTOCAL
#ifdef NEC_SKY_RC
/* define vendor ir key table */
static ir_key_table_t vendor_ir_nec_table[] = {
//skyworth ir
/*
	{0, 0xffffffff, 0x23dcfd01, MV_IR_KEY_POWER},
	{0, 0xffffffff, 0xfa05fd01, MV_IR_KEY_SUBTITLE},
	{0, 0xffffffff, 0x6d92fd01, MV_IR_KEY_DIGIT_1},
	{0, 0xffffffff, 0x6c93fd01, MV_IR_KEY_DIGIT_2},
	{0, 0xffffffff, 0x33ccfd01, MV_IR_KEY_DIGIT_3},
	{0, 0xffffffff, 0x718efd01, MV_IR_KEY_DIGIT_4},
	{0, 0xffffffff, 0x708ffd01, MV_IR_KEY_DIGIT_5},
	{0, 0xffffffff, 0xe01fff00, MV_IR_KEY_DIGIT_6},
	{0, 0xffffffff, 0x758afd01, MV_IR_KEY_DIGIT_7},
	{0, 0xffffffff, 0x748bfd01, MV_IR_KEY_DIGIT_8},
	{0, 0xffffffff, 0x3bc4fd01, MV_IR_KEY_DIGIT_9},
	{0, 0xffffffff, 0x7887fd01, MV_IR_KEY_DIGIT_0},
	{0, 0xffffffff, 0x2fd0fd01, MV_IR_KEY_INFO},

	{0, 0xffffffff, 0xb649ff00, MV_IR_KEY_SEARCH},
	{0, 0xffffffff, 0xfe01fd01, MV_IR_KEY_REPEAT_AB},

	{0, 0xffffffff, 0x6798fd01, MV_IR_KEY_HOME},

	{0, 0xffffffff, 0x35cafd01, MV_IR_KEY_UP},
	{0, 0xffffffff, 0x2dd2fd01, MV_IR_KEY_DOWN},
	{0, 0xffffffff, 0x6699fd01, MV_IR_KEY_LEFT},
	{0, 0xffffffff, 0x3ec1fd01, MV_IR_KEY_RIGHT},
	{0, 0xffffffff, 0x31cefd01, MV_IR_KEY_ENTER},

	{0, 0xffffffff, 0x3ac5fd01, MV_IR_KEY_BACK},


	{0, 0xffffffff, 0xfc03fd01, MV_IR_KEY_PAUSE},
	{0, 0xffffffff, 0xf50afd01, MV_IR_KEY_PLAY_PAUSE},
	{0, 0xffffffff, 0xf40bfd01, MV_IR_KEY_STOP},
	{0, 0xffffffff, 0xf20dfd01, MV_IR_KEY_FAST_BACKWARD},
	{0, 0xffffffff, 0xf10efd01, MV_IR_KEY_FAST_FORWARD},

	{0, 0xffffffff, 0x25dafd01, MV_IR_KEY_F1},
	{0, 0xffffffff, 0x7c83fd01, MV_IR_KEY_F2},
	{0, 0xffffffff, 0xf10efd01, MV_IR_KEY_F3},
	{0, 0xffffffff, 0xf20dfd01, MV_IR_KEY_SEARCH},
	{0, 0xffffffff, 0x6e91fd01, MV_IR_KEY_VOL_MUTE},
	{0, 0xffffffff, 0x2fd0fd01, MV_IR_KEY_VOL_MINUSQ},
	{0, 0xffffffff, 0x36c9fd01, MV_IR_KEY_VOL_PLUS},
	{0, 0xffffffff, 0x1ae5fd01, MV_IR_KEY_F8},


	{0, 0xffffffff, 0x7a85fd01, MV_IR_KEY_VOL_PLUS},
	{0, 0xffffffff, 0x7f80fd01, MV_IR_KEY_CHANNEL_PLUS},
	{0, 0xffffffff, 0x7986fd01, MV_IR_KEY_VOL_MINUS},
	{0, 0xffffffff, 0x639cfd01, MV_IR_KEY_VOL_MUTE},
	{0, 0xffffffff, 0x7e81fd01, MV_IR_KEY_CHANNEL_MINUS},
*/
//directv ir
	{0, 0xffffffff, 0x51ae2f86, MV_IR_KEY_POWER},
	{0, 0xffffffff, 0x916e2f86, MV_IR_KEY_POWER},
	{0, 0xffffffff, 0xfd022f86, MV_IR_KEY_DIGIT_1},
	{0, 0xffffffff, 0xec132f86, MV_IR_KEY_DIGIT_2},
	{0, 0xffffffff, 0xf30c2f86, MV_IR_KEY_DIGIT_3},
	{0, 0xffffffff, 0xe9162f86, MV_IR_KEY_DIGIT_4},
	{0, 0xffffffff, 0xeb142f86, MV_IR_KEY_DIGIT_5},
	{0, 0xffffffff, 0xe8172f86, MV_IR_KEY_DIGIT_6},
	{0, 0xffffffff, 0xe7182f86, MV_IR_KEY_DIGIT_7},
	{0, 0xffffffff, 0xf20d2f86, MV_IR_KEY_DIGIT_8},
	{0, 0xffffffff, 0xe6192f86, MV_IR_KEY_DIGIT_9},
	{0, 0xffffffff, 0xf10e2f86, MV_IR_KEY_DIGIT_0},
	{0, 0xffffffff, 0x708f2f86, MV_IR_KEY_INFO},


	{0, 0xffffffff, 0x7f802f86, MV_IR_KEY_HOME},

	{0, 0xffffffff, 0x748b2f86, MV_IR_KEY_UP},
	{0, 0xffffffff, 0x6e912f86, MV_IR_KEY_DOWN},
	{0, 0xffffffff, 0x77882f86, MV_IR_KEY_LEFT},
	{0, 0xffffffff, 0x629d2f86, MV_IR_KEY_RIGHT},
	{0, 0xffffffff, 0xef102f86, MV_IR_KEY_ENTER},

	{0, 0xffffffff, 0x9a652f86, MV_IR_KEY_BACK},

	{0, 0xffffffff, 0x5aa52f86, MV_IR_KEY_PLAY_PAUSE},
	{0, 0xffffffff, 0x6a952f86, MV_IR_KEY_FAST_BACKWARD},
	{0, 0xffffffff, 0xaa552f86, MV_IR_KEY_FAST_FORWARD},

//voice search
	{0, 0xffffffff, 0xa55a2f86, MV_IR_KEY_F4},
//mute
	{0, 0xffffffff, 0xf50a2f86, MV_IR_KEY_F5},
//vol+-
	{0, 0xffffffff, 0xe41b2f86, MV_IR_KEY_F6},
	{0, 0xffffffff, 0xed122f86, MV_IR_KEY_F7},
//Netflix
	{0, 0xffffffff, 0xba452f86, MV_IR_KEY_C},
//Sky
	{0, 0xffffffff, 0x7a852f86, MV_IR_KEY_D},
//FIXME: the following 4 keys don't have IR key map.
#if 0
//Guia
	{0, 0xffffffff, 0x639c2f86, MV_IR_KEY_F2},
//Lista
	{0, 0xffffffff, 0xea152f86, MV_IR_KEY_F2},
//Sair
	{0, 0xffffffff, 0xda252f86, MV_IR_KEY_F2},
//Dash
	{0, 0xffffffff, 0x3ac52f86, MV_IR_KEY_F2},
#endif
	{0, 0xffffffff, 0xf8072f86, MV_IR_KEY_CHANNEL_PLUS},
	{0, 0xffffffff, 0xfe012f86, MV_IR_KEY_CHANNEL_MINUS},

	{0, 0xffffffff, 0x2ad52f86, MV_IR_KEY_REC},

	{0, 0xffffffff, 0xffffffff, MV_IR_KEY_NULL},
};
#endif /* CONFIG_NEC_SKY_RC */
#endif /* NEC_PROTOCAL */

#endif /* __SM_IR_NEC_KEY_TABLE_SKY_H__ */
#endif /* IRENABLE */
