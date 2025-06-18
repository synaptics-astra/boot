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

extern struct flash_adaptor g_flash;
static int global_flash_id = 0;

void set_flash_parameters(int blocksize, int pagesize, int addrcycle)
{
	if(g_flash.set_parameters)
		return g_flash.set_parameters(blocksize, pagesize, addrcycle);
}

size_t get_block_size()
{
	return g_flash.block_size;
}
size_t get_boot_part_size()
{
	return g_flash.boot_part_size;
}

size_t get_page_size()
{
	return g_flash.page_size;
}

int get_addr_cycle()
{
	return g_flash.addr_cycle;
}

int get_boot_partition_number(void)
{
	if(g_flash.get_boot_partition_number)
		return g_flash.get_boot_partition_number();
	else
		return FLASH_OP_ERR;
}

int init_flash(int dev)
{
	global_flash_id = dev;

	if(g_flash.init)
		return g_flash.init(dev);
	else
		return FLASH_OP_ERR;
}

//return value is the start address in part
//this function is only for reading bootloader.subimg
int switch_flash_part(unsigned int part)
{
	if(g_flash.switch_part)
		return g_flash.switch_part(part);
	else
		return FLASH_OP_ERR;
}

void stop_alt_boot_mode(void)
{
	if(g_flash.stop_alt_boot_mode)
		g_flash.stop_alt_boot_mode();
}

long long read_flash(long long start, unsigned int size, unsigned char *buff)
{
	if(g_flash.read)
		return g_flash.read(start, size, buff);
	else
		return FLASH_OP_ERR;

}

long long get_flash_capacity(void)
{
	if(g_flash.get_capacity)
		return g_flash.get_capacity(global_flash_id);
	else
		return FLASH_OP_ERR;
}

long long read_flash_from_part(unsigned int part, long long start, unsigned int size, unsigned char *buff)
{
	int part_addr = 0;

	//switch to specific part
	part_addr = switch_flash_part(part);
	if(part_addr < 0) {
		ERR("flash: error when switch to boot partion %d(err = %d).\n", part, part_addr);
		return FLASH_OP_ERR;
	}
	return read_flash((part_addr + start), size, buff);
}


long long write_flash(long long start, unsigned int size, unsigned char *buff)
{
	if(g_flash.write)
		return g_flash.write(start, size, buff);
	else
		return FLASH_OP_ERR;

}


unsigned int flash_dev_id_inc(unsigned int dev_id)
{
	if(g_flash.dev_id_inc)
		return g_flash.dev_id_inc(dev_id);
	else
		return ++dev_id;
}
