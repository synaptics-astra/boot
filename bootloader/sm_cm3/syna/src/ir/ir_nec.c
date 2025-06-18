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
#ifdef NEC_PROTOCAL
#include "sm_type.h"
#include "sm_timer.h"
#include "sm_apb_timer.h"
#include "sm_common.h"
#include "ir_nec.h"
#include "sm_ir_key_def.h"
#include "platform_config.h"

static UINT32 iPrevCode, iCode, iCode_Bit, iPreamble_Found, iPreamble_Maybe_HOLD;
static UINT32 iPreamble_Time;

extern unsigned int length_caculate(unsigned int ticks,unsigned int prev_ticks);

UINT32 read_nec(UINT32 *cbuffer, UINT32 *cb_start, UINT32 *cb_end, BOOL *is_hold)
{
	INT32 iTwin_Level = 0;
	INT32 iData = 0;
	UINT32 iCurr_Time;
	UINT32 level_len = 0;
	UINT32 high_low = 0;
	UINT32 low_level = 0;
	UINT32 high_level = 0;
	UINT32 iRcvCode = 0xffffffff;

	// This condition can be considered as data length >= 2
	while( ( ((*cb_start +1 ) < *cb_end) ||( (*cb_start > *cb_end) && (*cb_start - *cb_end < IR_CBUFF_LENGTH-1))) && (iRcvCode == 0xffffffff))
	{
		high_low = cbuffer[*cb_start] & 0x80000000;
		level_len = cbuffer[*cb_start] & 0x7fffffff;
		(*cb_start) ++;
		(*cb_start) &= CIRC_BUFF_MASK;

		if( high_low )
		{
			// The first data is high pulse, must be something wrong, just remove it
			continue;
		}

		low_level = level_len;

		high_level = cbuffer[*cb_start] & 0x7fffffff;
		(*cb_start) ++;
		(*cb_start) &= CIRC_BUFF_MASK;

		iTwin_Level = low_level + high_level;

		if( iPreamble_Found )
		{
			if( iTwin_Level > DATA_1_MIN_LEN && iTwin_Level < DATA_1_MAX_LEN )
			{
				iData = 1;
			}
			else if( iTwin_Level > DATA_0_MIN_LEN && iTwin_Level < DATA_0_MAX_LEN )
			{
				iData = 0;
			}
			else if( iTwin_Level > PREAMB_MIN_LEN && iTwin_Level < PREAMB_MAX_LEN && low_level > PREAMB_MIN_LOW && low_level < PREAMB_MAX_LOW )
			{
				//SMLOGPRINTF(("[%d]preamble found in found\n",rom_sm_timer_gettimems()));
				iPreamble_Found = 1;
				if( iTwin_Level < PREAMB_HOLD_MAX )
				{
					iPreamble_Maybe_HOLD = 1;
				}
				else
				{
					iPreamble_Maybe_HOLD = 0;
				}

				iPreamble_Time = sm_apb_timer_readclock();
				iCode = 0;
				iCode_Bit = 0;
			}
			else if( iTwin_Level > PREAMB_MAX_LEN )
			{
				iPreamble_Found = 0;
				iPreamble_Maybe_HOLD = 0;
				iCode = 0;
				iCode_Bit = 0;
			}
			else
			{
				continue;
			}

			if( iCode_Bit < 32 )
			{
				iCode += iData << iCode_Bit;
				iCode_Bit ++;
			}
		}
		else	// preamble not found, try to get the next preamble
		{
			if( iTwin_Level > PREAMB_MIN_LEN && iTwin_Level < PREAMB_MAX_LEN && low_level > PREAMB_MIN_LOW && low_level < PREAMB_MAX_LOW )
			{
				//SMLOGPRINTF(("[%d]preamble found in notfound\n",rom_sm_timer_gettimems()));
				iPreamble_Found = 1;
				if( iTwin_Level < PREAMB_HOLD_MAX )
				{
					iPreamble_Maybe_HOLD = 1;
				}
				else
				{
					iPreamble_Maybe_HOLD = 0;
				}
				iPreamble_Time = sm_apb_timer_readclock();
				iCode = 0;
				iCode_Bit = 0;
			} else {
			}
		}
		if( iCode_Bit == 32 )
		{
			iPrevCode = iRcvCode = iCode;
			iPreamble_Found = 0;
			iCode = 0;
			iCode_Bit = 0;
		}
	}

	if( iPreamble_Maybe_HOLD && iCode_Bit == 0 && iPrevCode != 0xffffffff )
	{
		high_low = cbuffer[*cb_start] & 0x80000000;
		level_len = cbuffer[*cb_start] & 0x7fffffff;

		if( !high_low )
		{
			low_level = level_len;

			if(low_level>BURST_MIN_LEN && low_level<BURST_MAX_LEN)
			{
				//SMLOGPRINTF(("[%d]hold found\n",rom_sm_timer_gettimems()));
				iRcvCode = iPrevCode;
				*is_hold = TRUE;
				iPreamble_Found = 0;
				iPreamble_Maybe_HOLD = 0;
				iCode = 0;
				iCode_Bit = 0;
			}
		}
	}

	if( iPreamble_Found )
	{
		iCurr_Time = sm_apb_timer_readclock();
		if (length_caculate(iCurr_Time, iPreamble_Time) > FRAME_MAX_LEN)
		{
			iPreamble_Found = 0;
			iPreamble_Maybe_HOLD = 0;
			iCode = 0;
			iCode_Bit = 0;
		}
	}

	if( iPrevCode != 0xffffffff )
	{
		iCurr_Time = sm_apb_timer_readclock();
		if (length_caculate(iCurr_Time, iPreamble_Time) > FRAME_CYCLE_MAX)
		{
			iPrevCode = 0xffffffff;
		}
	}

	return iRcvCode;
}
#endif
