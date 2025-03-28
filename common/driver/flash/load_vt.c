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

#include "load_vt.h"

#ifdef CONFIG_DTB
#include "libfdt.h"
#include "boot_mode.h"
#endif

#define CRC32_SIZE      (4)
extern unsigned int crc32(unsigned int crc, unsigned char *buf, unsigned int len);

/*version table*/
#define MAX_VERSION_TABLE 32

#define CLEAR_VT_SIZE 2048

typedef struct {
	int num;
	ver_table_entry_t vt[MAX_VERSION_TABLE];
	unsigned int dev_ids[MAX_VERSION_TABLE];
}ver_table_t;
static ver_table_t vts;
/*version table end*/
static inline unsigned get_aligned(unsigned address, unsigned page_size) {
	return (address + page_size - 1) / page_size * page_size;
}

static void dump_version_entry(ver_table_entry_t * vte)
{
	DBG("%s: part1(start=%d, blks=%d, version=%08u%04u), part2(start=%d, blks=%d, version=%08u%04u)\n", vte->name,
			vte->part1_start_blkind, vte->part1_blks, vte->part1_version.major_version, vte->part1_version.minor_version,
#ifdef CONFIG_EMMC_WRITE_PROTECT
			vte->part1_start_blkind, vte->part1_blks, vte->part1_version.major_version, vte->part1_version.minor_version);
#else
			vte->part2_start_blkind, vte->part2_blks, vte->part2_version.major_version, vte->part2_version.minor_version);
#endif
}

void dump_version_table(void)
{
	int i = 0;
	for(i = 0; i < vts.num; i++) {
		DBG("[%02d,sd%02d] ", i, vts.dev_ids[i]);
		dump_version_entry(&(vts.vt[i]));
	}
}

// because vt is encrypted, we need decrypt it then parse it
int parse_version_table(unsigned char* buff)
{
	unsigned int i;
	unsigned int dev_id = 0;

	unsigned vt_size = 0;

	ver_table_entry_t * vt_entry;
	version_table_t *vt = (version_table_t *)(buff);

	vt_size = sizeof(version_table_t) + vt->num_entries * sizeof(ver_table_entry_t);

	/* check calculated version table size*/
	if(vt_size >= 2048) {
		DBG("ERROR: vt_size is too big %d!!!\n", vt_size);
		return FLASH_OP_ERR;
	}

	if(vt->magic != MAGIC_NUMBER || 0xffffffff != crc32(0, (unsigned char *)buff, vt_size + CRC32_SIZE)) {
		ERR("EMMC: MAGIC NUMBER or CRC error. %d\n", vt_size);
		return FLASH_OP_ERR;
	}

	dev_id = 0;
	vts.num = 0;
	for(i = 0;i < vt->num_entries;i++) {
		vt_entry = &vt->table[i];

		memcpy(&(vts.vt[i]), vt_entry, sizeof(ver_table_entry_t));
		vts.dev_ids[i] = dev_id;
		vts.num ++;

		dev_id = flash_dev_id_inc(dev_id);
#ifdef CONFIG_EMMC_WRITE_PROTECT

#else
		if (vt_entry->part1_start_blkind != vt_entry->part2_start_blkind) {
			/* double copy of the partition */
			dev_id = flash_dev_id_inc(dev_id);
		}
#endif
	}

	return 0;
}

#if 0
inline static unsigned get_aligned(unsigned address, unsigned page_size) {
	return (address + page_size - 1) / page_size * page_size;
}
#endif

int get_partition_info(void * tbuff)
{
	//clear pt is located at the last 1K of boot partition(block)
	//encrypted pt is located at last 14K of boot partition(block)

	//we need a buff that is at least 64bytes aligned address because of the DMA of EMMC
	unsigned char * buff = (unsigned char *)(uintmax_t)get_aligned((unsigned)(uintmax_t)tbuff, 64);
	int num = 0, i = 0;
	int ret = 0;
	long long start = 0;

	num = get_boot_partition_number();
	for(i = 1; i <= num; i++) {
		//read the clear pt only
		start = get_boot_part_size() - CLEAR_VT_SIZE;
		if(read_flash_from_part(i, start, CLEAR_VT_SIZE, buff) != 0) {
			DBG("read partiton info error\n");
			continue;
		}

		ret = parse_version_table(buff);

		if(ret != 0) {
			DBG("parse partiton info error\n");
			continue;
		}
		return 0;
	}

	return FLASH_SWITCH_PART_NOEXIST;
}

int find_partition(const void *name)
{
	int i = 0;
	for(i = 0; i < vts.num; i++) {
		//FIXME: ugly condition
		//avoid boot = bootloader, boot = bootlogo
		if(strlen(name) != strlen(vts.vt[i].name))
			continue;
		if(memcmp(name, vts.vt[i].name, strlen(name)) == 0) {
			return i;
		}
	}

	return PARTITION_NOT_EXIST;
}

int fetch_partition_info(int num, ver_table_entry_t *vt)
{
	if((num >= 0) && (num < vts.num)) {
		memcpy(vt, &(vts.vt[num]), sizeof(ver_table_entry_t));
		return (int)(vts.dev_ids[num]);
	}
	return PARTITION_NOT_EXIST;
}

unsigned int get_version_table_entry_number()
{
	return vts.num;
}

long long read_image(ver_table_entry_t *vt,unsigned int image_size,unsigned char *image_buff)
{
	return read_image_from_offset(vt, 0, image_size, image_buff);
}

long long read_image_by_ptname(const char *module_name,unsigned int image_size,unsigned char *image_buff)
{
	ver_table_entry_t vt;
	int ret = find_partition(module_name);
	if(ret < 0) {
		return ret;
	}
	fetch_partition_info(ret, &vt);

	return read_image_from_offset(&vt, 0, image_size, image_buff);
}

long long read_image_from_offset(ver_table_entry_t *vt, unsigned int pt_offset, unsigned int size, unsigned char *image_buf)
{
	long long start = 0, end = 0;

	start = vt->part1_start_blkind;
	start *= get_block_size();
	start += pt_offset;
	end = vt->part1_start_blkind + vt->part1_blks;
	end *= get_block_size();

	//image_size must not exceed the current partition
	if((start + size) > end) {
		ERR("the image is exceed the partition(%x > %x). Possible hacker attack detected!\n",
			(start + size), end);
		return -1;
	}

	//For EMMC:switch to user area
	//For nand and nor: nothing
	switch_flash_part(0);

	return read_flash(start, size, image_buf);
}


long long write_image(ver_table_entry_t *vt, unsigned int image_size, unsigned char *image_buff)
{
	return write_image_to_offset(vt, 0, image_size, image_buff);
}

long long write_image_to_offset(ver_table_entry_t *vt, unsigned int pt_offset, unsigned int size, unsigned char *image_buf)
{
	long long start = 0, end = 0;

	start = vt->part1_start_blkind;
	start *= get_block_size();
	start += pt_offset;
	end = vt->part1_start_blkind + vt->part1_blks;
	end *= get_block_size();

	//image_size must not exceed the current partition
	if((start + size) > end) {
		ERR("the image is exceed the partition(%x > %x). Possible hacker attack detected!\n",
			(start + size), end);
		return -1;
	}

	//For EMMC:switch to user area
	//For nand and nor: nothing
	switch_flash_part(0);

	return write_flash(start, size, image_buf);
}

//FIXME: how this function can be unified
#ifdef EMMC_BOOT
void set_flash_ts_param(char *param_buf)
{
	ver_table_entry_t vt_fts;
	unsigned int fts_dev_id = 0;
	int num = find_partition(FTS_NAME);
	*param_buf = '\0';
	if(num >= 0) {
		fts_dev_id = (unsigned int)fetch_partition_info(num, &vt_fts);
		sprintf(param_buf, " emmc_ts.dev_id=%d emmc_ts.size=%d emmc_ts.erasesize=%d emmc_ts.writesize=%d",
				fts_dev_id, vt_fts.part1_blks * get_block_size(), get_block_size(), 512);
	}
}
#endif

#ifdef NAND_BOOT
void set_flash_ts_param(char *param_buf)
{
	ver_table_entry_t vt_fts;
	unsigned int fts_dev_id = 0;
	int num = find_partition(FTS_NAME);	
	*param_buf = '\0';
	if(num >= 0) {
		fts_dev_id = (unsigned int)fetch_partition_info(num, &vt_fts);
		sprintf(param_buf, " flash_ts.dev_id=%d flash_ts.size=%d flash_ts.erasesize=%d flash_ts.writesize=%d",
				fts_dev_id, vt_fts.part1_blks * get_block_size(), get_block_size(), get_page_size());
	}
}
#endif

#ifdef CONFIG_DTB
static const char *nand_compatible_name = NULL;
static const char *nand_pt_compatible_name = NULL;

void set_nand_compatible_name(const char *nand_compatible, const char* pt_compatible)
{
	nand_compatible_name = nand_compatible;
	nand_pt_compatible_name = pt_compatible;
}

int upate_pt(void *fdt, int total_space)
{
	int node;
	int offset = 0, ret = 0;
	unsigned int start;
	unsigned int size;
	unsigned int reg[2];
	int i;

	ret = fdt_open_into(fdt, fdt, total_space);
	if (ret < 0)
		return ret;

	if(!nand_pt_compatible_name && !nand_compatible_name) {
		ERR("nand and pt node is unknown!\n");
		return -1;
	}

	offset = fdt_node_offset_by_compatible(fdt, -1, nand_pt_compatible_name);
	if (offset == -FDT_ERR_NOTFOUND) {
		NOTICE("Create partions node in dts.\n");

		offset = fdt_node_offset_by_compatible(fdt, -1, nand_compatible_name);
		if (offset < 0) {
			ERR("fail to find node of compatible %s\n", nand_compatible_name);
			return -1;
		}
		offset = fdt_add_subnode(fdt, offset, "partitions");
		if (offset < 0) {
			ERR("fail to add subnode partitions");
			return -1;
		}

		ret = fdt_setprop_string(fdt, offset, "compatible", nand_pt_compatible_name);
		if (ret < 0) {
			ERR("fail to setprop_string compatible : %s.\n", nand_pt_compatible_name);
			return -1;
		}
	}else {
		INFO("partitions is exist in dts, skip to update the partition\n");
		return 0;
	}

	for (i = vts.num - 1; i >= 0; i--) {
		node = fdt_add_subnode(fdt, offset, vts.vt[i].name);
		if (node < 0) {
			ERR("fail to set subnode %s\n", vts.vt[i].name);
			return -1;
		}
		ret = fdt_setprop_string(fdt, node, "lable", vts.vt[i].name);
		if (ret < 0) {
			ERR("fail to setprop_string %s\n", vts.vt[i].name);
			return -1;
		}
		start = vts.vt[i].part1_start_blkind * get_block_size();
		size = vts.vt[i].part1_blks * get_block_size();
		reg[0] = cpu_to_fdt32(start);
		reg[1] = cpu_to_fdt32(size);

		INFO("[%s]: start = 0x%x, size = 0x%x.\n", vts.vt[i].name, start, size);
		fdt_setprop(fdt, node, "reg", reg, sizeof(reg));
		switch (get_bootmode()) {
		case BOOTMODE_RECOVERY:
			if (vts.vt[i].write_protect_flag & RECOVERY_WP_MASK) {
				INFO("[%s]: set read-only in recovery.\n", vts.vt[i].name);
				ret = fdt_setprop(fdt, node, "read-only", NULL, 0);
				if (ret < 0) {
					ERR("fail to setprop_string read-only for %s\n", vts.vt[i].name);
					return -1;
				}
			}
			break;
		case BOOTMODE_NORMAL:
			if (vts.vt[i].write_protect_flag & NORMAL_WP_MASK) {
				INFO("[%s]: set read-only in normal.\n", vts.vt[i].name);
				ret = fdt_setprop(fdt, node, "read-only", NULL, 0);
				if (ret < 0) {
					ERR("fail to setprop_string read-only for %s\n", vts.vt[i].name);
					return -1;
				}
			}
			break;
		default:
			break;
		}

	}

	return fdt_pack(fdt);

}
#endif
