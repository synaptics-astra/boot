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
#include "com_type.h"
#include "flash_adaptor.h"
#include "debug.h"

extern int do_emmcread(unsigned long long start, unsigned int blks, unsigned char * buffer);
extern int do_emmcwrite(unsigned long long offset, unsigned int blks, unsigned char * emmc_buffer);
extern int do_emmcerase(unsigned long long offset, unsigned long long size);


int is_blank(const void *buf, size_t size)
{
	size_t i;
	const unsigned int *data = (const unsigned int *)buf;
	size = size / sizeof(*data);

	if (data[0] != 0x00000000 && data[0] != 0xffffffff)
		return 0;

	for (i = 0; i < size; i++)
		if (data[i] != data[0])
			return 0;
	return 1;
}


int NFlash_Init()
{
	dbg_printf(PRN_DBG,"NFlash_Init() -> init_nand_data()\n");
	// nand_data.szofpg = 512;
	// nand_data.szofblk = 524288;

	return 0;
}

/********************************************************
 * FUNCTION: reset device
 * PARAM: none
 * RETURN: 0 - succeed
 *        -2 - fail
 *******************************************************/
int NFlash_ResetDevice(void)
{
	dbg_printf(PRN_DBG,"NFlash_ResetDevice() -> mv_nand_reset_chip(0)\n");
	// mv_nand_reset_chip(0);
	return 0;
}

/********************************************************
 * FUNCTION: return the block status, good or bad
 * PARAM: addr - flash page memory address
 * RETURN: 0 - good block
 *        1 - bad block
 *******************************************************/
int is_block_bad(loff_t addr)
{
	dbg_printf(PRN_DBG,"is_block_bad(addr=0x%08X)\n",(int)addr);
	return 0;
}

/********************************************************
 * FUNCTION: read a page
 * PARAM: addr - flash page memory address
 *        pbuf - buffer to hold data
 * RETURN: 0 - succeed
 *         -1 - fail
 *******************************************************/
int NFlash_PageRead(loff_t addr, int *pbuf)
{
	unsigned int blk = get_page_size() / 512;
	dbg_printf(PRN_DBG,"NFlash_PageRead(addr=0x%08X, pbuf=0x%08X)\n", (int)addr, (uintptr_t)pbuf);

	return do_emmcread(addr, blk, (unsigned char *)pbuf);
}

/********************************************************
 * FUNCTION: write a page
 * PARAM: off - flash page memory address
 *        buf - buffer to hold data
 * RETURN: 0 - succeed
 *         -1 - fail
 *******************************************************/
int NFlash_PageWrite(loff_t ofs, int *buf)
{
	unsigned int blk = get_page_size() / 512;
	dbg_printf(PRN_DBG,"NFlash_PageWrite(addr=0x%08X, pbuf=0x%08X)\n", (int)ofs, (uintptr_t)buf);
	return do_emmcwrite(ofs, blk, (unsigned char *)buf);
}

/********************************************************
 * FUNCTION: erase a block
 * PARAM: ofs - flash page memory address
 * RETURN: 0 - succeed
 *         -1 - fail
 *******************************************************/
int NFlash_Erase(loff_t ofs)
{
	unsigned int blk_size = get_block_size();

	return do_emmcerase(ofs, (unsigned long long)blk_size);

	return 0;
}

/********************************************************
 * FUNCTION: mark one block as bad
 * PARAM: ofs - flash page memory address
 * RETURN: 0 - succeed
 *         -1 - fail
 *******************************************************/
int NFlash_Markbad(loff_t ofs)
{
	dbg_printf(PRN_DBG,"NFlash_Markbad(addr=0x%08X)\n", (int)ofs);
	return 0;
}
