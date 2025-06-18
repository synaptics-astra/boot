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
#include "mmc.h"

static unsigned int current_part = 0xF;

extern int do_emmcinit(int dev);
extern int do_emmcread(unsigned long long start, unsigned int blks, unsigned char * buffer);
extern int do_emmcwrite(unsigned long long start, unsigned int blks, unsigned char * buffer);
extern int do_emmc_switch_part(UINT32 PartitionNumber);
extern long long do_emmc_capacity(void);
extern void do_emmc_stop_alt_boot_mode(void);
static void emmc_set_parameters(int blocksize, int pagesize, int addrcycle);

/* dev: 0 for emmc, 1 for SD */
static int init_emmc(int dev)
{
	int ret = 0;
	ret = do_emmcinit(dev) ;
	if (ret) {
		ERR("mmc init fail. dev=%d\n", dev);
		//FIXME
		//reset_soc();
	}
	return ret;
}
// part 0(user area), 1(b1), 2(b2)
static loff_t switch_emmc_part(unsigned int part)
{
	if(part > 2)
		return FLASH_SWITCH_PART_NOEXIST;

	if(current_part != part)
		current_part = part;
	else
		return 0; // if we have switched to this part, just return

	if(do_emmc_switch_part(part)){
		ERR("EMMC: switch to boot partion %d error.\n", part);
		return FLASH_OP_ERR;
	}
	return 0;
}

static long long read_emmc(long long start, unsigned int size, unsigned char *buff)
{
	long long ret;
	int blks = (size + MMC_BLOCK_SIZE - 1) / MMC_BLOCK_SIZE;

	if(start % MMC_BLOCK_SIZE) {
		ERR("The start address should be %d bytes aligned for EMMC.\n", MMC_BLOCK_SIZE);
		return -1;
	}

	ret = do_emmcread(start, blks, buff);

	return ret;
}

static long long write_emmc(long long start, unsigned int size, unsigned char *buff)
{
	long long ret;
	unsigned int blks = (size + MMC_BLOCK_SIZE - 1) / MMC_BLOCK_SIZE;
	ret = do_emmcwrite(start, blks, buff);
	return ret;
}

static unsigned int emmc_dev_id_inc(unsigned int dev_id)
{
	dev_id++;
#ifndef CONFIG_GPT
	/* if we reach EBR area, need add 1 to skip it */
	if (dev_id == 4)
		dev_id++;
#endif
	return dev_id;
}

static int emmc_get_boot_partition_number(void)
{
	return 2;
}

static long long adaptor_get_capacity(int dev)
{
	if(dev == 0){
		return do_emmc_capacity();
	}else{
		return get_mmc_size();
	}
}

struct flash_adaptor g_flash= {
	.block_size = 0x80000,
	.page_size = 8192,
	.addr_cycle = 0,
	.init = init_emmc,
	.get_capacity = adaptor_get_capacity,
	.dev_id_inc = emmc_dev_id_inc,
	.get_boot_partition_number = emmc_get_boot_partition_number,
	.switch_part = switch_emmc_part,
	.read = read_emmc,
	.write = write_emmc,
	.set_parameters = emmc_set_parameters,
	.stop_alt_boot_mode = do_emmc_stop_alt_boot_mode,
};

static void emmc_set_parameters(int blocksize, int pagesize, int addrcycle)
{
	g_flash.block_size = blocksize;
	g_flash.page_size = pagesize;
	g_flash.addr_cycle = addrcycle;
}
