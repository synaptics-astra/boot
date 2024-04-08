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

#include "load_gih.h"

#include "apb_watchdog.h"

#include "img_layout.h"
#include "image_chunk.h"

#define BOOT_PARTITION_SIZE (512 * 1024) // 512KB

void dump_chunk_param(struct chunk_param chunk)
{
	INFO("%c%c%c%c: offset = 0x%08x, size = 0x%08x, dest_start = 0x%016llx, \
		dest_size = 0x%08x, attr0 = 0x%08x, attr1 = 0x%08x\n", (char)chunk.id,
		(char)(chunk.id >> 8), (char)(chunk.id >> 16), (char)(chunk.id >> 24), chunk.offset,
		chunk.size, chunk.dest_start, chunk.dest_size, chunk.attr0,	chunk.attr1);
}

//the whole GIH image size = the offset of the last chunk + the size of the last chunk
unsigned int get_image_size_from_GIH(struct image_header * buff)
{
	int num = buff->chunk_num;
	unsigned int size = 0;

	size = buff->chunk[num - 1].offset + buff->chunk[num - 1].size;

	return size;
}

// only for the images in bootloader.subimg which the GIH header and images list are separated.
int read_image_from_GIH(int part, struct image_header * gih, unsigned int id, unsigned char* buff)
{
	long long ret = 0;
	int chunk_id = 0;
	int part_addr = 0;
	unsigned int offset = 0, size = 0;

	DBG("flash: read image from boot partition %d.\n", part);

	chunk_id = find_chunk(id, gih);
	if(chunk_id >= 0) {
		offset = gih->chunk[chunk_id].offset + IMG_HEADER_ADDR;
		size = gih->chunk[chunk_id].size;
	} else {
		ERR("Didn't find %c%c%c%c.\n", (char)id,
			(char)(id >> 8), (char)(id >> 16), (char)(id >> 24));
		//FIXME:
		//reset_soc();
		while(1);;
	}

	//switch to specific part
	part_addr = switch_flash_part(part);
	if(part_addr < 0) {
		ERR("flash: error when switch to boot partion %d(err = %d).\n", part, part_addr);
		return FLASH_OP_ERR;
	}

	// the image should be in the range of boot partition
	// for u-boot, there should no this limitation
#ifndef CONFIG_UBOOT
	if((offset + size) > BOOT_PARTITION_SIZE) {
		ERR("flash: the image is exceed the boot partition\n");
		//FIXME:
		//reset_soc();
		while(1);
	}
#endif

	ret = read_flash((part_addr + offset), size, buff);
	if(ret){
		ERR("flash: read image %c%c%c%c error.\n",
			(char)id, (char)(id >> 8), (char)(id >> 16), (char)(id >> 24));
		dump_chunk_param(gih->chunk[chunk_id]);
		return FLASH_OP_ERR;
	}

	//switch back to user area
	part_addr = switch_flash_part(0);
	if(part_addr < 0) {
		ERR("flash: error when switch to boot partion %d(err = %d).\n", part, part_addr);
		return FLASH_OP_ERR;
	}

	return 0;
}

int find_chunk(unsigned int id, struct image_header * buff)
{
	if (buff->chunk_num < MAX_IMAGE_CHUNK_ENTRY) {
		for(int i = 0; i < buff->chunk_num; i++) {
			if(id == buff->chunk[i].id) {
				//dump_chunk_param((buff->chunk[i]));
				return i;
			}
		}
	}

	return CHUNK_NON_EXIST;
}
