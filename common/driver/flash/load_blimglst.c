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
#include "util.h"

#include "flash_adaptor.h"

#include "load_blimglst.h"

/*bootloader image list*/
#define MAX_HEADER_IMAGE_NUM 16
#define MAX_IMAGE_NAME 100

typedef struct {
	int num;
	struct block_param block_params[MAX_HEADER_IMAGE_NUM];
}header_image_list_t;
static header_image_list_t header_image_list;
/*bootloader image list end*/

void dump_block_param(struct block_param *block_param_entry)
{
	dbg_printf(PRN_RES,"%s:offset = %d,size = %d,flags = %d,reserved = %s\n",
			block_param_entry->block_name,block_param_entry->offset,block_param_entry->size,
			block_param_entry->flags,block_param_entry->reserved);
}

int get_image_list_info(int part, unsigned int start, unsigned int size, unsigned char* buff)
{
	long long ret = 0;
	int part_addr = 0;

	dbg_printf(PRN_DBG,"EMMC: Load image list info from boot partition %d.\n", part);
	//switch to boot area
	part_addr = switch_flash_part(part);
	if(part_addr < 0) {
		dbg_printf(PRN_ERR,"flash: error when switch to boot partion %d(err = %d).\n", part, part_addr);
		return part_addr;
	}

	ret = read_flash((part_addr + start), size, buff);
	if(ret){
		dbg_printf(PRN_ERR,"flash: read version table error\n");
		return FLASH_OP_ERR;
	}

	return switch_flash_part(0);
}


int parse_image_list_info(unsigned char* buff)
{
	int i = 0;
	bootloader_header_t *header=(bootloader_header_t*)buff;
	struct block_param *block_param_entry;

	header_image_list.num = 0;
	if(header->magicnum == BOOTLOADER_HEADER_MAGICNUM){
		for(i=0;i<header->number_of_blocks;i++){
			block_param_entry = &header->block_params[i];
			memcpy(&(header_image_list.block_params[i]),block_param_entry,sizeof(struct block_param));
			header_image_list.num ++;
			dump_block_param(&(header_image_list.block_params[i]));
		}
	}

	return 0;
}

int find_image(const void *name)
{
	int i = 0;
	for(i = 0; i < header_image_list.num; i++) {
		if(strnlen(name, MAX_IMAGE_NAME) != strnlen(
			header_image_list.block_params[i].block_name, MAX_IMAGE_NAME))
			continue;
		if(memcmp(name, header_image_list.block_params[i].block_name, strlen(name)) == 0) {
			return i;
		}
	}
	return IMAGE_NON_EXIST;
}

int fetch_image_info(int num, struct block_param *param)
{
	if((num >= 0) && (num < header_image_list.num)) {
		memcpy(param, &(header_image_list.block_params[num]), sizeof(struct block_param));
		return num;
	}
	return IMAGE_NON_EXIST;
}


int read_image_data(struct block_param *param, unsigned char *image_buff, int part)
{
	long long ret = 0;
	int part_addr = 0;
	unsigned int image_start = 0, size = 0;

	if( (!param) || (!image_buff) ){
		dbg_printf(PRN_RES,"param or image_buff NULL pointer error!\n");
		return -2;
	}

	//switch to boot area
	part_addr = switch_flash_part(part);
	if(part_addr < 0) {
		dbg_printf(PRN_ERR,"flash: error when switch to boot partion %d(err = %d).\n", part, part_addr);
		return part_addr;
	}

	image_start = part_addr + param->offset + IMGLST_INFO_ADDR;
	size = param->size;

	ret = read_flash(image_start, size, image_buff);
	if(ret) {
		dbg_printf(PRN_ERR,"flash: read %s fail.\n", param->block_name);
		return ret;
	}

	return switch_flash_part(0);
}
