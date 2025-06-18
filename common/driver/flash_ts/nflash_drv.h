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
#ifndef _NFLASH_DRV_H_
#define _NFLASH_DRV_H_

#ifdef __cplusplus__
extern "C" {
#endif

#include "priv_type.h"

int is_blank(const void *buf, size_t size);

/********************************************************
 * FUNCTION: initialize NFC with device configuration
 * PARAM: pconfig - device specific configuration
 * RETURN: none
 *******************************************************/
int NFlash_Init();

int init_nfc(int block_size, int page_size, int ecc_strength);

/********************************************************
 * FUNCTION: reset device
 * PARAM: none
 * RETURN: 0 - succeed
 *        -1 - fail
 *******************************************************/
int NFlash_ResetDevice(void);

/********************************************************
 * FUNCTION: return the block status, good or bad
 * PARAM: addr - flash page memory address
 * RETURN: 0 - good block
 *        1 - bad block
 *******************************************************/
int is_block_bad(loff_t addr);

/********************************************************
 * FUNCTION: read a page
 * PARAM: addr - flash page memory address
 *        pbuf - buffer to hold data
 * RETURN: 0 - succeed
 *         -1 - fail
 *******************************************************/
int NFlash_PageRead(loff_t addr, int *pbuf);

/********************************************************
 * FUNCTION: write a page
 * PARAM: off - flash page memory address
 *        buf - buffer to hold data
 * RETURN: 0 - succeed
 *         -1 - fail
 *******************************************************/
int NFlash_PageWrite(loff_t ofs, int *buf);

/********************************************************
 * FUNCTION: erase a block
 * PARAM: ofs - flash page memory address
 * RETURN: 0 - succeed
 *         -1 - fail
 *******************************************************/
int NFlash_Erase(loff_t ofs);

/********************************************************
 * FUNCTION: mark one block as bad
 * PARAM: ofs - flash page memory address
 * RETURN: 0 - succeed
 *         -1 - fail
 *******************************************************/
int NFlash_Markbad(loff_t ofs);

long long nand_read_generic(unsigned int start, unsigned int end, char* data_buff,
                                                unsigned int data_size);
#ifdef __cplusplus__
}
#endif

#endif
