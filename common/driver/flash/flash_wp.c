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
#include "debug.h"
#include "load_vt.h"
#include "flash_wp.h"
#include "bootloader.h"
#include "apb_watchdog.h"

#ifdef EMMC_BOOT
#include "sdmmc_api.h"
static int emmc_write_protect(int bootmode)
{
	ver_table_entry_t vt_entry;
	unsigned long long wp_start_addr;
	unsigned long long wp_size;
	unsigned long long emmc_chip_size;
	int i, ret, tmp;
	int ver_table_entry_num;

	//amend emmc erase unit size, write protect size and chip size.
	ret = emmc_amend_sundry_size();
	if (ret){
		dbg_printf(PRN_ERR,"emmc amend sundry_size fail.\n");
	}
	emmc_chip_size = emmc_get_chip_size();
	ret = emmc_enable_userarea_poweron_write_protect();
	if (ret != 0){
		dbg_printf(PRN_ERR,"emmc user area write protect enable fail.\n");
		reset_soc();
	}
	//1. partition tables at header.
	wp_start_addr = 0ULL;
	wp_size = (16ULL << 20);
	ret = emmc_set_userarea_write_protect(wp_start_addr, wp_size);
	if (ret){
		dbg_printf(PRN_ERR,"emmc partition table write protect fail.\n");
		reset_soc();
	}

	//2. partition tables at tail.
	tmp = emmc_chip_size % (16ULL << 20);
	if (tmp)
		wp_start_addr = emmc_chip_size - tmp;
	else
		wp_start_addr = emmc_chip_size - (16ULL << 20);
	wp_size = (16ULL << 20);
	ret = emmc_set_userarea_write_protect(wp_start_addr, wp_size);
	if (ret){
		dbg_printf(PRN_ERR,"emmc partition table write protect fail.\n");
		reset_soc();
	}
	//3. partitions according boot mode.
	ver_table_entry_num = get_version_table_entry_number();

	if(bootmode == BOOTMODE_RECOVERY){
#if 0
		dbg_printf(PRN_RES,"Clear write protect of boot partition.\n");
		ret = emmc_disable_bootarea_poweron_write_protect(pSDMMCP);
		if (ret != 0){
			dbg_printf(PRN_ERR,"emmc boot partition write protect enable fail.\n");
			while(1);
		}
#endif
		for (i = 0; i < ver_table_entry_num; i++)
		{
			fetch_partition_info(i, &vt_entry);
			if (vt_entry.write_protect_flag & RECOVERY_WP_MASK)
			{
				wp_start_addr = vt_entry.part1_start_blkind * ((512ULL) << 10);
				wp_size = vt_entry.part1_blks * ((512ULL) << 10);
				ret = emmc_set_userarea_write_protect(wp_start_addr, wp_size);
				if (ret){
					dbg_printf(PRN_ERR,"set user area partition %s to write protect fail.\n", vt_entry.name);
					reset_soc();
				}
				dbg_printf(PRN_RES,"Set user area partition %s to write protect.\n", vt_entry.name);
			}
		}
	}else if (bootmode == BOOTMODE_NORMAL){
		dbg_printf(PRN_RES,"Set boot area to write protect. \n");
		ret = emmc_enable_bootarea_poweron_write_protect();
		if (ret != 0){
			dbg_printf(PRN_ERR,"emmc boot area write protect enable fail.\n");
			reset_soc();
		}
		for (i = 0; i < ver_table_entry_num; i++)
		{
			fetch_partition_info(i, &vt_entry);
			//dbg_printf(PRN_RES,"vt_entry.name = %s.\t", vt_entry.name);
			//dbg_printf(PRN_RES,"vt_entry.write_protect_flag = %d.\n", vt_entry.write_protect_flag);
			if (vt_entry.write_protect_flag & NORMAL_WP_MASK)
			{
				wp_start_addr = vt_entry.part1_start_blkind * ((512ULL) << 10);
				wp_size = vt_entry.part1_blks * ((512ULL) << 10);
				ret = emmc_set_userarea_write_protect(wp_start_addr, wp_size);
				if (ret){
					dbg_printf(PRN_ERR,"set user area partition %s to write protect fail.\n", vt_entry.name);
					reset_soc();
				}
				dbg_printf(PRN_RES,"set user area partition %s to write protect.\n", vt_entry.name);
			}
		}
	}
	return 0;
}
#endif

int flash_write_protect(int bootmode)
{
#ifdef EMMC_BOOT
	return emmc_write_protect(bootmode);
#endif
	dbg_printf(PRN_DBG,"write protection is not supported.\n");
	return 0;
}
