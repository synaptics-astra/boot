/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright (c) 2013-2025 Synaptics Incorporated. All rights reserved.
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
#include "load_gpt.h"
#include "load_vt.h"
#include "flash_partition.h"
#include "flash_adaptor.h"

struct partition_table {
	enum partition_type type;
	union {
		gpt_table_t *gpt;
		version_table_t *vt;
	};
};

static struct partition_table flash_pt;

int parse_pt(void * tbuff)
{
	if (flash_pt.type == FLASH_PARTITION_GPT)
		return gpt_get_partition_info(tbuff);
	else if (flash_pt.type == FLASH_PARTITION_VT)
		return vt_get_partition_info(tbuff);
	else
		return -1;
}

enum partition_type get_pt_type(void)
{
	return flash_pt.type;
}

int set_pt_type(enum flash_dev_type dev)
{
	if (dev == FLASH_DEV_EMMC || dev ==  FLASH_DEV_SD)
		flash_pt.type = FLASH_PARTITION_GPT;

	if (dev == FLASH_DEV_SPI)
		flash_pt.type = FLASH_PARTITION_VT;

	return 0;
}

int get_pt_index(const char * name){
	if (flash_pt.type == FLASH_PARTITION_GPT)
		return gpt_find_partition(name);
	else if (flash_pt.type == FLASH_PARTITION_VT)
		return vt_find_partition(name);
	else
		return -1;
}

int get_pt_info(int index, union partition_entry *entry)
{
	if (flash_pt.type == FLASH_PARTITION_GPT)
		return gpt_fetch_partition_info(index, &entry->gpt_entry.gpt);
	else if (flash_pt.type == FLASH_PARTITION_VT)
		return vt_fetch_partition_info(index, &entry->vt_entry);
	else
		return -1;
}

long long read_pt_offset(union partition_entry *entry, unsigned int pt_offset, unsigned int size, unsigned char *buff)
{
	if (flash_pt.type == FLASH_PARTITION_GPT)
		return gpt_read_image_from_offset(&entry->gpt_entry.gpt, pt_offset, size, buff);
	else if (flash_pt.type == FLASH_PARTITION_VT)
		return vt_read_image_from_offset(&entry->vt_entry, pt_offset, size, buff);
	else
		return -1;
}

long long read_pt(union partition_entry *entry, unsigned int size, unsigned char *buff)
{
	return read_pt_offset(entry, 0, size, buff);
}

long long write_pt(union partition_entry *entry, unsigned int size, unsigned char *buff)
{
	if (flash_pt.type == FLASH_PARTITION_GPT)
		return gpt_write_image(&entry->gpt_entry.gpt, size, buff);
	else if (flash_pt.type == FLASH_PARTITION_VT)
		return vt_write_image(&entry->vt_entry, size, buff);
	else
		return -1;
}

long long read_pt_by_name(const char *pt_name, unsigned int size, unsigned char *buff)
{
	if (flash_pt.type == FLASH_PARTITION_GPT)
		return gpt_read_image_by_ptname(pt_name, size, buff);
	else if (flash_pt.type == FLASH_PARTITION_VT)
		return vt_read_image_by_ptname(pt_name, size, buff);
	else
		return -1;
}

