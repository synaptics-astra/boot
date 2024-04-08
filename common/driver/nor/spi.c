
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
#include "io.h"
#include "string.h"
#include "debug.h"

#include "flash_adaptor.h"

extern void SPIReadFlash(unsigned int, unsigned int, unsigned int);

static void nor_init()
{
	//NULL
}

static int nor_switch_part(unsigned int part)
{
	if(part > 1)
		return FLASH_SWITCH_PART_NOEXIST;

	//if part = 0, return 0
	return 0;
}

static long long nor_read(unsigned int start, unsigned int size, unsigned char *buff)
{
	//we read spi 4bytes by 4bytes
	int mod = size % 4;
	size = size/4;
	if(mod) {
		size++;
	}
	SPIReadFlash((start + 0xF0000000), size,(uintptr_t)buff);
	return 0;
}

static int nor_get_boot_partition_number(void)
{
	return 1;
}

static void nor_set_parameters(int blocksize, int pagesize, int addrcycle);

struct flash_adaptor g_flash = {
	.block_size = 0x10000,
	.page_size = 512,
	.addr_cycle = 0,
	.init = nor_init,
	.get_boot_partition_number = nor_get_boot_partition_number,
	.switch_part = nor_switch_part,
	.set_parameters = nor_set_parameters,
	.read = nor_read,
};

static void nor_set_parameters(int blocksize, int pagesize, int addrcycle)
{
	g_flash.block_size = 0x10000;
	g_flash.page_size = 512;
	g_flash.addr_cycle = 0;
}

