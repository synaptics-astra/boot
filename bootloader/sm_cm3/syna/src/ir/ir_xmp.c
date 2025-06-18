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
#ifdef XMP_PROTOCAL
#include "sm_type.h"
#include "platform_config.h"
#include "ir_xmp.h"

static UINT32 iCode, iCode_nibble, iPreamble_Found, iVendorid, iChecksum, iPrevCode;

UINT32 read_xmp(UINT32 *cbuffer, UINT32 *cb_start, UINT32 *cb_end, BOOL *is_hold)
{
	INT32 iTwin_Level = 0;
	INT32 iData = 0;
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

		//Filter1: min & max length of a nibble
		if((iTwin_Level > XMP_HEADER_MIN_HIGH_TIME) && (iTwin_Level < XMP_HEADER_MAX_HIGH_TIME))  {
			/*
			* It's a active low signal, hence deal data with high level
			* According to XR11 protocol: where T = 1.085 micro sec
			*  		 ------------------------	   ----------------------
			*	194T |	699T + iData*126T	| 194T |  699T + iData*126T
			*  ------						--------
			* 699T is followed by 194 ticks to represent data of 126 tics where  0 <= iData <= 15.
			*/

			iData = ((high_level - XMP_HEADER_699_TICKS_IN_US) / XMP_HEADER_126_TICKS_IN_US) + 1;
			iVendorid = (iVendorid << 4) | iData;

			//Filter2: venderid
			if (iVendorid == VENDOR_ID_XFINITY) {
				//PRT_INFO("vendor_id = 0x%08X  \n", iVendorid);

				/*Set Preamble flag to collect next 8 data nibbles*/
				iPreamble_Found = 1;

				/*Reset all flags to receive code*/
				iCode = 0;
				iCode_nibble = 0;
				iVendorid = 0;
				iChecksum = 0;
			} else if (iPreamble_Found) {
				if (iCode_nibble <= 8) {
					/*Collect 8 nibbles into iCode*/
					iCode = (iCode << 4) | iData;
					iCode_nibble++;
					iChecksum = + iCode;

					if (iCode_nibble == 8) {
						/* Reset Preamble flag to scan venderid*/
						iPreamble_Found = 0;

						/* Checksum is that number which causes the mod-16 addition
						 * of the eight nibbles to equal the appropriate
						 * Checksum Result. For Registration Packets and Data
						 * Packets the Checksum Result is zero (0).
						 */
						if (!(iChecksum % 16)) {
							 iRcvCode = iCode;

							/*Key Repeat Flag (repeat of last key = 1)*/
							if (iRcvCode & 0x00800000)
								*is_hold = TRUE;

							/*Copy previous keycode*/
							iPrevCode = iRcvCode;

							//PRT_INFO("key_code = 0x%02X  key: %s\n", iRcvCode, *is_hold?"pressed":"released");
							//break;
						}
					}
				}
			}
		}
	}

	return iRcvCode;
}
#endif
#endif
