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
#ifdef RC5_PROTOCAL
#include "sm_type.h"
#include "sm_timer.h"
#include "ir_rc5.h"
#include "sm_printf.h"
#include "sm_ir_key_def.h"
#include "platform_config.h"

static UINT32 iCode_Bit, iCode, prev_code, iPrev_Data;
static UINT32 ERR_time;
static UINT32 last_bit;

UINT32 read_rc5(UINT32 *cbuffer, UINT32 *cb_start, UINT32 *cb_end, BOOL *is_hold)
{
	UINT32 iData = 0, iTime = 0;
	UINT32 rcv_code = 0xffffffff;

	if(ERR_time != 0) {
		if( mv_sm_timer_gettimems() - ERR_time > 90 ){
			ERR_time = 0;
			iCode_Bit = 0;
			iCode = 0;
			prev_code = 0xffffffff;
			*cb_start = 0;
			*cb_end = 0;
		}
		return 0xffffffff;
	}

	while((*cb_start != *cb_end) && (rcv_code == 0xffffffff) )
	{
		if(last_bit) {
			last_bit = 0;
			(*cb_start)++;
				*cb_start &= CIRC_BUFF_MASK;
			continue;
		}

		iData = ((cbuffer[*cb_start] & 0x80000000)? 1:0);
		iTime = cbuffer[*cb_start] & 0x7fffffff;

		if(iCode_Bit == 0)
		{
			if(iData == 0) {
				PRT_DEBUG("it's not the start bit 0\n");
				(*cb_start)++;
				*cb_start &= CIRC_BUFF_MASK;
				iCode_Bit = 0;
				continue;
			}
			(*cb_start)++;
				*cb_start &= CIRC_BUFF_MASK;
		} else if(iCode_Bit == 1) {
			if(iTime > MAX_BIT_TIME || iTime < MIN_BIT_TIME) {
				PRT_DEBUG("bit1 time = %d\n", iTime);
				ERR_time = mv_sm_timer_gettimems();
				return 0xffffffff;
			}

			if(iData != iPrev_Data) {
				(*cb_start)++;
				*cb_start &= CIRC_BUFF_MASK;
				continue;
			}
			if(iData == 0) {
				PRT_DEBUG("it's not the start bit 1\n");
				(*cb_start)++;
				*cb_start &= CIRC_BUFF_MASK;
				iCode_Bit = 0;
				continue;
			}
			(*cb_start)++;
				*cb_start &= CIRC_BUFF_MASK;
		} else {
			if(iTime < MAX_BIT_TIME && iTime > MIN_BIT_TIME && iData != iPrev_Data) {
				(*cb_start)++;
				*cb_start &= CIRC_BUFF_MASK;
				continue;
			} else if( iTime < MAX_BIT_TIME && iTime > MIN_BIT_TIME && iData == iPrev_Data) {
				(*cb_start)++;
				*cb_start &= CIRC_BUFF_MASK;
			} else if( iTime < 2*MAX_BIT_TIME && iTime > 2*MIN_BIT_TIME && iData != iPrev_Data) {
				(*cb_start)++;
				*cb_start &= CIRC_BUFF_MASK;
			} else {
				PRT_DEBUG("bitX time = %d\n", iTime);
				ERR_time = mv_sm_timer_gettimems();
				return 0xffffffff;
			}
		}
			iCode = ((iCode<<1) + iData);
			iPrev_Data = iData;
			iCode_Bit++;

		if(iCode_Bit == 14)
		{
			if(prev_code == iCode) {
				*is_hold = TRUE;
			}
			rcv_code = prev_code = iCode;
			last_bit = iData;
		}
	}



	if(rcv_code!=0xffffffff) {
		iCode_Bit = 0;
		iCode = 0;
	//	*cb_start = 0;
	//	*cb_end = 0;
		return rcv_code;
	}

	return 0xffffffff;
}
#endif
