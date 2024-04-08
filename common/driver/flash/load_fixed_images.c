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
#include "string.h"
#include "debug.h"

#include "flash_adaptor.h"
#include "load_fixed_images.h"

int read_fixed_image_from_bootarea(int part, unsigned int start, unsigned int size, unsigned char * buff)
{
	long long ret = 0;
	int part_addr = 0;

	dbg_printf(PRN_DBG,"flash: Load image header from boot partition %d.\n", part);
	//switch to boot area
	part_addr = switch_flash_part(part);
	if(part_addr < 0) {
		dbg_printf(PRN_ERR,"flash: error when switch to boot partion %d(err = %d).\n", part, part_addr);
		return part_addr;
	}

	ret = read_flash((part_addr + start), size, buff);
	if(ret){
		return FLASH_OP_ERR;
	}

	return switch_flash_part(0);
}

int read_image_header(int part, unsigned char* buff)
{
	int ret = 0;

	ret = read_fixed_image_from_bootarea(part, IMG_HEADER_ADDR, IMG_HEADER_SIZE, buff);

	if(ret){
		dbg_printf(PRN_ERR,"flash: read image header error\n");
		return FLASH_OP_ERR;
	}

	return ret;
}

int get_miniloader(int part, unsigned char *buff)
{
	int ret = 0;

	ret = read_fixed_image_from_bootarea(part, MINILOADER_ADDR, MINILOADER_SIZE, buff);

	if(ret){
		dbg_printf(PRN_ERR,"flash: read miniloader fail.\n");
		return FLASH_OP_ERR;
	}

	return ret;
}

long long get_uboot_param(unsigned char *env_buff)
{
	int part_addr = 0;

	//switch to boot area
	part_addr = switch_flash_part(0);

	//FIXME: define MACRO for address and size
	return read_flash((part_addr + UBOOT_ENV_ADDR), UBOOT_ENV_SIZE , env_buff);
}
