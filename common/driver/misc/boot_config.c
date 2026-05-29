/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright © 2013-2026 Synaptics Incorporated. All rights reserved.
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

///////////////////////////////////////////////////////////////////////////////
//! \file	       boot_config.c
//! \brief 	checking boot flag and PMIC settings.
///////////////////////////////////////////////////////////////////////////////

#include "io.h"
#include "string.h"
#include "debug.h"
#include "boot_devinfo.h"
#include "boot_config.h"
#include "flash_adaptor.h"
#ifdef CONFIG_GENX_ENABLE
#include "bm_generic.h"
#endif
#if defined(SPI_BOOT_LINUX)
#include "boot_mode.h"
#include "spi.h"
#endif

#define BOOT_CONFIG_SZ 0x100
/* EMMC: Position: 512KB - 3KB - BOOT_CONFIG_SZ */
#define EMMC_PREBOOT_PT_SZ (512 * 1024)
#define EMMC_BOOT_CONFIG_OFFSET (EMMC_PREBOOT_PT_SZ - 3072 - BOOT_CONFIG_SZ)
/* SPI: Position: 512KB - 3KB - BOOT_CONFIG_SZ + 1KB(header) */
#define SPI_PREBOOT_PT_SZ (512 * 1024)
#define SPI_BOOT_CONFIG_OFFSET (SPI_PREBOOT_PT_SZ - 2048 - BOOT_CONFIG_SZ)

struct boot_config g_boot_config;

static uint32_t calculate_checksum(const uint32_t *data, uint32_t size)
{
	uint32_t checksum = 0;
	int i;

	for (i = 0; i < (size/4); i++)
		checksum ^= data[i];

	return checksum;
}

int get_bootflag_skip_fw(void)
{
	if (g_boot_config.magic == BOOT_CONFIG_MAGIC)
		return g_boot_config.bootflag.skip_fw;

	return 0;
}

int get_bootflag_force_spi(void)
{
	if (g_boot_config.magic == BOOT_CONFIG_MAGIC)
		return g_boot_config.bootflag.force_spi;

	return 0;
}

int get_pmic_config(struct pmic_config **pmic)
{
	if (g_boot_config.magic == BOOT_CONFIG_MAGIC) {
		*pmic = &g_boot_config.pmic;
		return 0;
	}

	return -1;
}

int check_boot_config(struct boot_config *config)
{
	uint32_t expected_checksum;

	if (!config) {
		return -1;
	}

	if (config->magic != BOOT_CONFIG_MAGIC) {
		NOTICE("BOOT_CONFIG: Invalid magic number 0x%08x, expected 0x%08x\n",
				   config->magic, BOOT_CONFIG_MAGIC);
		return -1;
	}

	if (config->version != BOOT_CONFIG_VERSION) {
		NOTICE("BOOT_CONFIG: Unsupported version 0x%08x, expected 0x%08x\n",
				   config->version, BOOT_CONFIG_VERSION);
		return -1;
	}

	expected_checksum = calculate_checksum((const uint32_t *)(&config->magic), sizeof(struct boot_config) - sizeof(config->magic));
	if (config->checksum != expected_checksum) {
		NOTICE("BOOT_CONFIG: Checksum mismatch 0x%08x, expected 0x%08x\n",
				   config->checksum, expected_checksum);
		return -1;
	}

	return 0;
}

int load_boot_config(void)
{
#if defined(EMMC_SECURE_BOOT)
	const uint32_t boot_config_offset = EMMC_BOOT_CONFIG_OFFSET;
#ifndef CONFIG_GENX_ENABLE
	int enabled_bootpart = mmc_g_flash.get_current_enabled_bootpart();
#else
	int enabled_bootpart = GenX_BCM_Get_Boot_Slot() + 1;
#endif
	int block_size = 512;
	int lba = boot_config_offset + BOOT_CONFIG_SZ - 512;
	int offset_in_block = boot_config_offset % block_size;
	struct boot_config* found_config;
	unsigned char * buff = (unsigned char *)(TEMP_BUF_ADDR);
	int ret = -1;

	ret = init_flash(FLASH_DEV_EMMC);
	if(ret != 0) {
		ERR("Miniloader: init_flash error(%d)!\n", ret);
		return -1;
	}

	if (enabled_bootpart != 1 && enabled_bootpart != 2)
		enabled_bootpart = 1; // Default to partition 1

	switch_flash_part(enabled_bootpart);

	if (read_flash(lba, block_size, buff) != 0) {
		NOTICE("BOOT_CONFIG: Failed to read from eMMC at LBA %d\n", lba);
		ret = -1;
	} else {
		found_config = (struct boot_config*)(buff + offset_in_block);
		if (check_boot_config(found_config) == 0) {
			memcpy(&g_boot_config, found_config, sizeof(struct boot_config));
			ret = 0;
		} else {
			ret = -1;
		}
	}

	switch_flash_part(0);
	return ret;
#elif defined(SPI_BOOT_LINUX)
	int boot_slot = GenX_BCM_Get_Boot_Slot();
	int lba = SPI_BOOT_CONFIG_OFFSET;
	struct boot_config* found_config;
	unsigned char * buff = (unsigned char *)(TEMP_BUF_ADDR);
	int ret = -1;

	ret = init_flash(FLASH_DEV_SPI);
	if(ret != 0) {
		ERR("Miniloader: init_flash error(%d)!\n", ret);
		return -1;
	}

	if (boot_slot == BOOTSEL_B)
		lba += SPI_BOOT_PART_SIZE;

	if (read_flash(lba, BOOT_CONFIG_SZ, buff) != 0) {
		NOTICE("BOOT_CONFIG: Failed to read from SPI flash at LBA %d\n", lba);
		ret = -1;
	} else {
		found_config = (struct boot_config*)(buff);
		if (check_boot_config(found_config) == 0) {
			memcpy(&g_boot_config, found_config, sizeof(struct boot_config));
			ret = 0;
		} else {
			ret = -1;
		}
	}

	return ret;
#endif
	return -1;
}



