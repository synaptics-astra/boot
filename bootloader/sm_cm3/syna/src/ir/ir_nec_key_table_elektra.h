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

#ifdef IRENABLE
#ifndef __SM_IR_NEC_KEY_TABLE_ELEKTRA_H__
#define __SM_IR_NEC_KEY_TABLE_ELEKTRA_H__

#include "ir_key_table_def.h"

#ifdef NEC_PROTOCAL
#ifdef NEC_ARCADYAN_RC
/* define vendor ir key table */
static ir_key_table_t vendor_ir_nec_table[] = {
	//Omni White
	{0, 0xffffffff, 0xf50af708, MV_IR_KEY_POWER},
	{0, 0xffffffff, 0xfa05f708, MV_IR_KEY_RED},
	{0, 0xffffffff, 0xf609f708, MV_IR_KEY_GREEN},
	{0, 0xffffffff, 0xbd42f708, MV_IR_KEY_YELLOW},
	{0, 0xffffffff, 0xbc43f708, MV_IR_KEY_BLUE},
	{0, 0xffffffff, 0xae51f708, MV_IR_KEY_FAST_BACKWARD},
	{0, 0xffffffff, 0xad52f708, MV_IR_KEY_PLAY_PAUSE},
	{0, 0xffffffff, 0xac53f708, MV_IR_KEY_FAST_FORWARD},
	{0, 0xffffffff, 0xb14ef708, MV_IR_KEY_YELLOW},
	{0, 0xffffffff, 0xb24df708, MV_IR_KEY_BLUE},
	{0, 0xffffffff, 0xff00f708, MV_IR_KEY_UP},
	{0, 0xffffffff, 0xfc03f708, MV_IR_KEY_LEFT},
	{0, 0xffffffff, 0xe01ff708, MV_IR_KEY_ENTER},
	{0, 0xffffffff, 0xfd02f708, MV_IR_KEY_RIGHT},
	{0, 0xffffffff, 0xfe01f708, MV_IR_KEY_DOWN},
	{0, 0xffffffff, 0xe31cf708, MV_IR_KEY_RETURN},
	{0, 0xffffffff, 0xf30cf708, MV_IR_KEY_HOMEPAGE},
	{0, 0xffffffff, 0x9966f708, MV_IR_KEY_TITLEMENU},
	{0, 0xffffffff, 0xa758f708, MV_IR_KEY_VOL_PLUS},
	{0, 0xffffffff, 0x8b74f708, MV_IR_KEY_F4},
	{0, 0xffffffff, 0xa55af708, MV_IR_KEY_CHANNEL_PLUS},
	{0, 0xffffffff, 0xa25df708, MV_IR_KEY_VOL_MINUS},
	{0, 0xffffffff, 0xf20df708, MV_IR_KEY_VOL_MUTE},
	{0, 0xffffffff, 0xa05ff708, MV_IR_KEY_CHANNEL_MINUS},
	{0, 0xffffffff, 0xee11f708, MV_IR_KEY_DIGIT_1},
	{0, 0xffffffff, 0xed12f708, MV_IR_KEY_DIGIT_2},
	{0, 0xffffffff, 0xec13f708, MV_IR_KEY_DIGIT_3},
	{0, 0xffffffff, 0xeb14f708, MV_IR_KEY_DIGIT_4},
	{0, 0xffffffff, 0xea15f708, MV_IR_KEY_DIGIT_5},
	{0, 0xffffffff, 0xe916f708, MV_IR_KEY_DIGIT_6},
	{0, 0xffffffff, 0xe817f708, MV_IR_KEY_DIGIT_7},
	{0, 0xffffffff, 0xe718f708, MV_IR_KEY_DIGIT_8},
	{0, 0xffffffff, 0xe619f708, MV_IR_KEY_DIGIT_9},
	//{0, 0xffffffff, 0x8877f708, },
	{0, 0xffffffff, 0xef10f708, MV_IR_KEY_DIGIT_0},
	//{0, 0xffffffff, 0xcf30f708, },
	//Omni White

	//syna nec ir keys
	{0, 0xffffffff, 0xfd02ff00, MV_IR_KEY_POWER},
	{0, 0xffffffff, 0xe11eff00, MV_IR_KEY_OPENCLOSE},

	{0, 0xffffffff, 0xfc03ff00, MV_IR_KEY_DISCMENU},
	{0, 0xffffffff, 0xf807ff00, MV_IR_KEY_TITLEMENU},
	{0, 0xffffffff, 0xbf40ff00, MV_IR_KEY_SUBTITLE},
	{0, 0xffffffff, 0xbb44ff00, MV_IR_KEY_ANGLE},

	{0, 0xffffffff, 0xf40bff00, MV_IR_KEY_DIGIT_1},
	{0, 0xffffffff, 0xec13ff00, MV_IR_KEY_DIGIT_2},
	{0, 0xffffffff, 0xe41bff00, MV_IR_KEY_DIGIT_3},
	{0, 0xffffffff, 0xf00fff00, MV_IR_KEY_DIGIT_4},
	{0, 0xffffffff, 0xe817ff00, MV_IR_KEY_DIGIT_5},
	{0, 0xffffffff, 0xe01fff00, MV_IR_KEY_DIGIT_6},
	{0, 0xffffffff, 0xb748ff00, MV_IR_KEY_DIGIT_7},
	{0, 0xffffffff, 0xaf50ff00, MV_IR_KEY_DIGIT_8},
	{0, 0xffffffff, 0xa758ff00, MV_IR_KEY_DIGIT_9},
	{0, 0xffffffff, 0xab54ff00, MV_IR_KEY_DIGIT_0},
	{0, 0xffffffff, 0xb34cff00, MV_IR_KEY_INFO},
	{0, 0xffffffff, 0xa35cff00, MV_IR_KEY_SETUPMENU},

	{0, 0xffffffff, 0xbe41ff00, MV_IR_KEY_AUDIO},
	{0, 0xffffffff, 0xb649ff00, MV_IR_KEY_F4},
	{0, 0xffffffff, 0xae51ff00, MV_IR_KEY_ZOOM},
	{0, 0xffffffff, 0xa659ff00, MV_IR_KEY_DISPLAY},
	{0, 0xffffffff, 0xba45ff00, MV_IR_KEY_REPEAT},
	{0, 0xffffffff, 0xb24dff00, MV_IR_KEY_REPEAT_AB},
	{0, 0xffffffff, 0xaa55ff00, MV_IR_KEY_PIP},
	{0, 0xffffffff, 0xa25dff00, MV_IR_KEY_EXIT},

	{0, 0xffffffff, 0xbd42ff00, MV_IR_KEY_A},
	{0, 0xffffffff, 0xb54aff00, MV_IR_KEY_B},
	{0, 0xffffffff, 0xa956ff00, MV_IR_KEY_C},
	{0, 0xffffffff, 0xa15eff00, MV_IR_KEY_D},

	{0, 0xffffffff, 0xbc43ff00, MV_IR_KEY_HOME},
	{0, 0xffffffff, 0xa55aff00, MV_IR_KEY_TITLE},

	{0, 0xffffffff, 0xb04fff00, MV_IR_KEY_UP},
	{0, 0xffffffff, 0xa857ff00, MV_IR_KEY_DOWN},
	{0, 0xffffffff, 0xb44bff00, MV_IR_KEY_LEFT},
	{0, 0xffffffff, 0xa45bff00, MV_IR_KEY_RIGHT},
	{0, 0xffffffff, 0xac53ff00, MV_IR_KEY_ENTER},

	{0, 0xffffffff, 0xb847ff00, MV_IR_KEY_STEP},
	{0, 0xffffffff, 0xa05fff00, MV_IR_KEY_BACK},

	{0, 0xffffffff, 0xfe01ff00, MV_IR_KEY_SLOW},
	{0, 0xffffffff, 0xf609ff00, MV_IR_KEY_PAUSE},
	{0, 0xffffffff, 0xee11ff00, MV_IR_KEY_PLAY_PAUSE},
	{0, 0xffffffff, 0xe619ff00, MV_IR_KEY_STOP},
	{0, 0xffffffff, 0xfa05ff00, MV_IR_KEY_FAST_BACKWARD},
	{0, 0xffffffff, 0xf20dff00, MV_IR_KEY_FAST_FORWARD},
	{0, 0xffffffff, 0xea15ff00, MV_IR_KEY_SKIP_BACKWARD},
	{0, 0xffffffff, 0xe21dff00, MV_IR_KEY_SKIP_FORWARD},

	{0, 0xffffffff, 0xff00ff00, MV_IR_KEY_F1},
	{0, 0xffffffff, 0xf708ff00, MV_IR_KEY_F2},
	{0, 0xffffffff, 0xef10ff00, MV_IR_KEY_F3},
	{0, 0xffffffff, 0xe718ff00, MV_IR_KEY_SEARCH},
	{0, 0xffffffff, 0xfb04ff00, MV_IR_KEY_VOL_MUTE},
	{0, 0xffffffff, 0xf30cff00, MV_IR_KEY_VOL_MINUS},
	{0, 0xffffffff, 0xeb14ff00, MV_IR_KEY_VOL_PLUS},
	{0, 0xffffffff, 0xe31cff00, MV_IR_KEY_F8},

	/* Google Large RCU */
	{0, 0xffffffff, 0xde217788, MV_IR_KEY_POWER},
	{0, 0xffffffff, 0x9f607788, MV_IR_KEY_INPUT},
	{0, 0xffffffff, 0xfe017788, MV_IR_KEY_DIGIT_1},
	{0, 0xffffffff, 0xfd027788, MV_IR_KEY_DIGIT_2},
	{0, 0xffffffff, 0xfc037788, MV_IR_KEY_DIGIT_3},
	{0, 0xffffffff, 0xfb047788, MV_IR_KEY_DIGIT_4},
	{0, 0xffffffff, 0xfa057788, MV_IR_KEY_DIGIT_5},
	{0, 0xffffffff, 0xf9067788, MV_IR_KEY_DIGIT_6},
	{0, 0xffffffff, 0xf8077788, MV_IR_KEY_DIGIT_7},
	{0, 0xffffffff, 0xf7087788, MV_IR_KEY_DIGIT_8},
	{0, 0xffffffff, 0xf6097788, MV_IR_KEY_DIGIT_9},
	{0, 0xffffffff, 0xf50a7788, MV_IR_KEY_DIGIT_0},

	{0, 0xffffffff, 0xa7587788, MV_IR_KEY_SUBTITLE},
	{0, 0xffffffff, 0xd6297788, MV_IR_KEY_INFO},
	{0, 0xffffffff, 0xb44b7788, MV_IR_KEY_RED},
	{0, 0xffffffff, 0xb54a7788, MV_IR_KEY_GREEN},
	{0, 0xffffffff, 0xb6497788, MV_IR_KEY_YELLOW},
	{0, 0xffffffff, 0xb34c7788, MV_IR_KEY_BLUE},
	{0, 0xffffffff, 0x8b747788, MV_IR_KEY_MARKER},
	{0, 0xffffffff, 0xb9467788, MV_IR_KEY_SEARCH},
	{0, 0xffffffff, 0xf00f7788, MV_IR_KEY_F3},

	{0, 0xffffffff, 0xea157788, MV_IR_KEY_UP},
	{0, 0xffffffff, 0xe9167788, MV_IR_KEY_DOWN},
	{0, 0xffffffff, 0xe8177788, MV_IR_KEY_LEFT},
	{0, 0xffffffff, 0xe7187788, MV_IR_KEY_RIGHT},
	{0, 0xffffffff, 0xe6197788, MV_IR_KEY_ENTER},

	{0, 0xffffffff, 0xb7487788, MV_IR_KEY_BACK},
	{0, 0xffffffff, 0xb8477788, MV_IR_KEY_HOME},
	{0, 0xffffffff, 0xcd327788, MV_IR_KEY_GUIDE},
	{0, 0xffffffff, 0xdc237788, MV_IR_KEY_VOL_PLUS},
	{0, 0xffffffff, 0xdb247788, MV_IR_KEY_VOL_MINUS},
	{0, 0xffffffff, 0xda257788, MV_IR_KEY_VOL_MUTE},
	{0, 0xffffffff, 0xcc337788, MV_IR_KEY_CHANNEL_PLUS},
	{0, 0xffffffff, 0xcb347788, MV_IR_KEY_CHANNEL_MINUS},

	{0, 0xffffffff, 0x9b647788, MV_IR_KEY_F1},
	{0, 0xffffffff, 0x9c637788, MV_IR_KEY_F2},
	{0, 0xffffffff, 0x98677788, MV_IR_KEY_TITLEMENU},
	{0, 0xffffffff, 0x97687788, MV_IR_KEY_TITLE},
	/* end of Google Large RCU */

	{0, 0xffffffff, 0xffffffff, MV_IR_KEY_NULL},
	//syna ir nec keys
};
#endif /* CONFIG_NEC_ARCADYAN_RC */
#endif /* NEC_PROTOCAL */

#endif /* __SM_IR_NEC_KEY_TABLE_ELEKTRA_H__ */
#endif /* IRENABLE */
// vim: set noexpandtab:
