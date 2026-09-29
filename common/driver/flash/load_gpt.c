/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright (c) 2013-2020 Synaptics Incorporated. All rights reserved.
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

#include "uuid.h"
#include "gpt.h"

#include "load_gpt.h"


#define CRC32_SIZE      (4)
extern unsigned int crc32(unsigned int crc, unsigned char *buf, unsigned int len);

static gpt_table_t gptes;
/*version table end*/

/*function declaration*/
static void gpt_convert_efi_name_to_char(char *s, const uint16_t *ent_name, int n);
long long gpt_read_image_from_offset(struct gpt_ent *gpte, unsigned int pt_offset, unsigned int size, unsigned char *image_buf);
long long gpt_write_image_to_offset(struct gpt_ent *gpte, unsigned int pt_offset, unsigned int size, unsigned char *image_buf);


static void dump_gpt_entry(struct gpt_ent_part * gpte)
{
	char ent_name_char[72] = {0};
	gpt_convert_efi_name_to_char(ent_name_char, gpte->gpt.ent_name, sizeof(gpte->gpt.ent_name));

	INFO("%s: part=%d start=%d, end=%d \n", ent_name_char, gpte->part, gpte->gpt.ent_lba_start, gpte->gpt.ent_lba_end);
}

void dump_gpt_table(void)
{
	int i = 0;
	for(i = 0; i < gptes.num; i++) {
		INFO("[%02d,sd%02d] ", i, i);
		dump_gpt_entry(&(gptes.entry[i]));
	}
}

int parse_gpt_table(uint8_t part, struct gpt_hdr * hdr, struct gpt_ent * entry)
{
	unsigned int i;
	unsigned int num;
	struct uuid	unused;

	if(memcmp(hdr->hdr_sig, GPT_HDR_SIG, 8) || hdr->hdr_revision != GPT_HDR_REVISION) {
		ERR("gpt header data corrupted at part %d!\n", part);
		return -1;
	}

	if (hdr->hdr_size > MAX_HDR_SIZE) {
		ERR("gpt hdr size too large at part %d!\n", part);
		return -1;
	}

	num = (MAX_GPT_ENTRY - gptes.num) > hdr->hdr_entries ? hdr->hdr_entries : (MAX_GPT_ENTRY - gptes.num);

	memset(&unused, 0, sizeof(unused));
	for(i = 0;i < num; i++) {
		if (memcmp(&unused, &entry[i], sizeof(unused)) == 0)
			continue;

		gptes.entry[gptes.num].part = part;
		memcpy(&(gptes.entry[gptes.num].gpt), &entry[i], sizeof(struct gpt_ent));
		gptes.num ++;
	}

	return 0;
}

void uuid_to_str(char *s, size_t len, const struct uuid *u)
{
	if (len < UUID_STR_LEN)
		return;

	snprintf(s, len,
		"%08x-%04x-%04x-%04x-%04x%04x%04x",
		u->time_low,
		u->time_mid,
		u->time_hi_and_version,
		(u->clock_seq_hi_and_reserved << 8) | u->clock_seq_low,
		(u->node[0] << 8) | u->node[1],
		(u->node[2] << 8) | u->node[3],
		(u->node[4] << 8) | u->node[5]);
}

#ifndef GET_ALIGNED
static unsigned get_aligned(unsigned address, unsigned page_size) {
	return (address + page_size - 1) / page_size * page_size;
}
#endif

#define START_ADDR_GPT    0x0 //start from MBR
#define GPT_PRI_READ_SIZE 0x6800  //26k (MBR 512B; GPT HEADER 512B; GPT TABLE 16K; RESERVE 1K; SIGN 8K)
#define GPT_ALT_READ_SIZE 0x4200  // 16.5K
#define GPT_HEADER_OFFSET 0x200
#define GPT_TABLE_OFFSET  0x400
#define GPT_PART_SIZE     0x4000  //GTP TABLE MAXIMUM SIZE 16K

int get_gpt(void * tbuff)
{
	//we need a buff that is at least 64bytes aligned address because of the DMA of EMMC
	unsigned char * buff = (unsigned char *)(uintmax_t)get_aligned((unsigned)(uintmax_t)tbuff, 64);
	long long start = START_ADDR_GPT;
	struct gpt_hdr * hdr = NULL;
	struct gpt_ent * entry = NULL;
	int i = 0;
	int part = 0;

	for (i = 0; i < 5; i++) {
		part = i? 3+i: i;

		/* Primary GPT */
		start = START_ADDR_GPT;
		if(read_flash_from_part(part, start, GPT_PRI_READ_SIZE, buff) != 0) {
			INFO("read primary gpt error at part %d\n", part);
			break;
		}

		hdr = (struct gpt_hdr *)(buff + GPT_HEADER_OFFSET);
		entry = (struct gpt_ent *)(buff + GPT_TABLE_OFFSET);

		if (parse_gpt_table(part, hdr, entry) == 0)
			continue;

		if (part == 0) {
			/* Backup GPT */
			ERR("Trying Backup GPT\n");
			start = get_flash_capacity() - GPT_ALT_READ_SIZE;//at the last 16.5K of emmc user area
			if(read_flash_from_part(part, start, GPT_ALT_READ_SIZE, buff) != 0) {
				INFO("read alt gpt error at part %d\n", part);
				break;
			}

			hdr = (struct gpt_hdr *)(buff + GPT_PART_SIZE);
			entry = (struct gpt_ent *)(buff);
			parse_gpt_table(part, hdr, entry);
		}
	}

	if (gptes.num == 0)
			return -1;

	return 0;
}

static void gpt_convert_efi_name_to_char(char *s, const uint16_t *ent_name, int n)
{
	const char *ent_name_tmp = (const char *)ent_name;
	unsigned char index = 0x00;

	memset((void *)s, 0x00, n);

	for(index = 0; (index<<1) < n; index++) {
		s[index] = ent_name_tmp[index<<1];

		if(!s[index])
			break;
	}

	return;
}

//GPT_BUFF_SIZE must larger than GPT_PRI_READ_SIZE(0x6800) as the gpt buff address will be 64byte align for DMA transfer

int gpt_get_partition_info(void * tbuff)
{
	int ret;

	ret = get_gpt(tbuff);
	if(ret != 0) {
		return -1;
	}

	return 0;
}


int gpt_find_partition(const char * name)
{
	int i = 0;
	char ent_name_char[72] = {0};

	for(i = 0; i < gptes.num; i++) {
		gpt_convert_efi_name_to_char(ent_name_char, gptes.entry[i].gpt.ent_name, sizeof(gptes.entry[i].gpt.ent_name));
		if(strnlen(name, MAX_ENT_NAME) != strnlen(ent_name_char, MAX_ENT_NAME))
			continue;
		if(memcmp(name, ent_name_char, strnlen(name, MAX_ENT_NAME)) == 0) {
			switch_flash_part(gptes.entry[i].part);
			return i;
		}
	}

	return PARTITION_NOT_EXIST;
}

int gpt_fetch_partition_info(int num, struct gpt_ent *gpte)
{
	if((num >= 0) && (num < gptes.num)) {
		memcpy(gpte, &(gptes.entry[num].gpt), sizeof(struct gpt_ent));
		return num;
	}
	return PARTITION_NOT_EXIST;
}

unsigned int get_gpt_entry_number()
{
	return gptes.num;
}

long long gpt_read_image(struct gpt_ent *gpte, unsigned int image_size, unsigned char *image_buff)
{
	return gpt_read_image_from_offset(gpte, 0, image_size, image_buff);
}

long long gpt_read_image_by_ptname(const char *module_name, unsigned int image_size, unsigned char *image_buff)
{
	struct gpt_ent gpte;
	int pt_index = gpt_find_partition(module_name);
	if(pt_index < 0) {
		return -1;
	}
	gpt_fetch_partition_info(pt_index, &gpte);

	return gpt_read_image(&gpte, image_size, image_buff);
}

long long gpt_read_image_from_offset(struct gpt_ent *gpte, unsigned int pt_offset, unsigned int size, unsigned char *image_buf)
{
	long long start = 0, end = 0;

	start = (long long)gpte->ent_lba_start * SDIO_BLK_SIZE + pt_offset;
	end   = (long long)gpte->ent_lba_end * SDIO_BLK_SIZE;

	//image_size must not exceed the current partition, size aligned to SDIO_BLK_SIZE
	if( end < (size + start - SDIO_BLK_SIZE)) {
		ERR("the image is exceed the partition(%llx > %llx). Possible hacker attack detected!\n",
			(start + size), end);
		return -1;
	}

	return read_flash(start, size, image_buf);
}

long long gpt_write_image(struct gpt_ent *gpte, unsigned int image_size, unsigned char *image_buff)
{
	return gpt_write_image_to_offset(gpte, 0, image_size, image_buff);
}

long long gpt_write_image_to_offset(struct gpt_ent *gpte, unsigned int pt_offset, unsigned int size, unsigned char *image_buf)
{
	long long start = 0, end = 0;

	start = (long long)gpte->ent_lba_start * SDIO_BLK_SIZE + pt_offset;
	end   = (long long)gpte->ent_lba_end * SDIO_BLK_SIZE;

	//image_size must not exceed the current partition, size aligned to SDIO_BLK_SIZE
	if( end < (size + start - SDIO_BLK_SIZE)) {
		ERR("the image is exceed the partition(%llx > %llx). Possible hacker attack detected!\n",
			(start + size), end);
		return -1;
	}

	//For EMMC:switch to user area
	//For nand and nor: nothing
	switch_flash_part(0);

	return write_flash(start, size, image_buf);
}
