/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright (C) 2013-2025 Synaptics Incorporated. All rights reserved.
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
#include "pmic_config.h"
#include "flash_adaptor.h"
#include "pmic_select.h"
#include "printf.h"
#include "string.h"
#ifdef CONFIG_GENX_ENABLE
#include "bm_generic.h"
#endif
#if defined(SPI_BOOT_LINUX)
#include "boot_mode.h"
#include "spi.h"
#endif

#define PMIC_CONFIG_SZ 0x20
/* EMMC: Position: 512KB - 3KB - 32 */
#define EMMC_PREBOOT_PT_SZ (512 * 1024)
#define EMMC_PMIC_CONFIG_OFFSET (EMMC_PREBOOT_PT_SZ - 3072 - PMIC_CONFIG_SZ)
/* SPI: Position: 512KB - 3KB - 32 + 1KB(header) */
#define SPI_PREBOOT_PT_SZ (512 * 1024)
#define SPI_PMIC_CONFIG_OFFSET (SPI_PREBOOT_PT_SZ - 2048 - PMIC_CONFIG_SZ)



extern int do_emmc_get_current_enabled_bootpart(void);

struct pmic_config g_pmic_config;
static int g_pmic_config_valid = 0;
__attribute__((aligned(64))) unsigned char pmic_buf[512] = {0};

static const char* pmic_model_names[] = {
	[PMIC_MODEL_UNKNOWN] = "unknown",
	[PMIC_MODEL_M88PG86X] = "m88pg86x",
	[PMIC_MODEL_SY8824B] = "sy8824b",
	[PMIC_MODEL_NCP6335D] = "ncp6335d",
	[PMIC_MODEL_MP886X] = "mp886x",
	[PMIC_MODEL_SY8827N] = "sy8827n",
	[PMIC_MODEL_HL7593WL06] = "hl7593wl06",
	[PMIC_MODEL_TPS62868] = "tps62868",
	[PMIC_MODEL_RT5739] = "rt5739",
	[PMIC_MODEL_FP6359] = "fp6359",
	[PMIC_MODEL_TPS6287X] = "tps6287x",
};

static UNSG32 calculate_checksum(const struct pmic_config *config)
{
	UNSG32 checksum = 0;
	const UNSG32 *data = (const UNSG32 *)config;
	int i;

	for (i = 0; i < (PMIC_CONFIG_SIZE / 4 - 1); i++) {
		checksum ^= data[i];
	}

	return checksum;
}

int pmic_config_validate(const struct pmic_config *config)
{
	UNSG32 expected_checksum;

	if (!config) {
		return -1;
	}

	if (config->magic != PMIC_CONFIG_MAGIC) {
		lgpl_printf("PMIC: Invalid magic number 0x%08x, expected 0x%08x\n",
				   config->magic, PMIC_CONFIG_MAGIC);
		return -1;
	}

	if (config->version != PMIC_CONFIG_VERSION) {
		lgpl_printf("PMIC: Unsupported version 0x%08x, expected 0x%08x\n",
				   config->version, PMIC_CONFIG_VERSION);
		return -1;
	}

	expected_checksum = calculate_checksum(config);
	if (config->checksum != expected_checksum) {
		lgpl_printf("PMIC: Checksum mismatch 0x%08x, expected 0x%08x\n",
				   config->checksum, expected_checksum);
		return -1;
	}

	if (config->vcore_id == 0 && config->vcpu_id == 0 && config->pwm_valid == 0) {
		lgpl_printf("PMIC: Invalid PMIC ID\n");
		return -1;
	}

	return 0;
}

int find_pmic_config(struct pmic_config *config)
{
#if defined(EMMC_SECURE_BOOT)
	const UNSG32 pmic_config_offset = EMMC_PMIC_CONFIG_OFFSET;
#ifndef CONFIG_GENX_ENABLE
	int enabled_bootpart = mmc_g_flash.get_current_enabled_bootpart();
#else
	int enabled_bootpart = GenX_BCM_Get_Boot_Slot() + 1;
#endif
	int block_size = 512;
	int lba = pmic_config_offset + PMIC_CONFIG_SZ - 512;
	int offset_in_block = pmic_config_offset % block_size;
	struct pmic_config* found_config;
	int ret = -1;

	if (enabled_bootpart != 1 && enabled_bootpart != 2)
		enabled_bootpart = 1; // Default to partition 1

	switch_flash_part(enabled_bootpart);

	if (read_flash(lba, block_size, pmic_buf) != 0) {
		lgpl_printf("PMIC: Failed to read from eMMC at LBA %d\n", lba);
		ret = -1;
	} else {
		found_config = (struct pmic_config*)(pmic_buf + offset_in_block);
		if (pmic_config_validate(found_config) == 0) {
			memcpy(config, found_config, sizeof(struct pmic_config));
			ret = 0;
		} else {
			ret = -1;
		}
	}

	switch_flash_part(0);
	return ret;
#elif defined(SPI_BOOT_LINUX)
	int boot_slot = GenX_BCM_Get_Boot_Slot();
	int lba = SPI_PMIC_CONFIG_OFFSET;
	struct pmic_config* found_config;
	int ret = -1;

	if (boot_slot == BOOTSEL_B)
		lba += SPI_BOOT_PART_SIZE;

	if (read_flash(lba, PMIC_CONFIG_SZ, pmic_buf) != 0) {
		lgpl_printf("PMIC: Failed to read from SPI flash at LBA %d\n", lba);
		ret = -1;
	} else {
		found_config = (struct pmic_config*)(pmic_buf);
		if (pmic_config_validate(found_config) == 0) {
			memcpy(config, found_config, sizeof(struct pmic_config));
			ret = 0;
		} else {
			ret = -1;
		}
	}

	return ret;
#endif
	return -1;
}

int pmic_config_init(void)
{
	int ret = -1;
	UNSG32 vcore_id;
	UNSG32 vcpu_id;

	ret = find_pmic_config(&g_pmic_config);
	if (ret == 0) {
		g_pmic_config_valid = 1;
		if (g_pmic_config.pwm_valid == 0) {//I2C
			lgpl_printf("PMIC: VCORE %s (ID: 0x%02x), VCPU %s (ID: 0x%02x) I2C PMIC detected\n",
					pmic_get_model_name(g_pmic_config.vcore_id), g_pmic_config.vcore_id,
					pmic_get_model_name(g_pmic_config.vcpu_id), g_pmic_config.vcpu_id);
			if (g_pmic_config.vcore_id == 0)
				vcore_id = PMIC_UNKNOW;
			else
				vcore_id = g_pmic_config.vcore_id - 1;

			if (g_pmic_config.vcpu_id == 0)
				vcpu_id = PMIC_UNKNOW;
			else
				vcpu_id = g_pmic_config.vcpu_id - 1;

			set_pmic_id(vcpu_id, vcore_id);
			ret = 1;
		} else if (g_pmic_config.pwm_valid == 1) {//PWM
			lgpl_printf("PMIC: VCORE ch: 0x%02x, VCPU ch: 0x%02x PWM PMIC detected\n",
					g_pmic_config.vcore_pwm_ch,
					g_pmic_config.vcpu_pwm_ch);
			if (g_pmic_config.vcore_pwm_ch == 0xff)
				vcore_id = PMIC_UNKNOW;
			else
				vcore_id = FP6359;

			if (g_pmic_config.vcpu_pwm_ch == 0xff)
				vcpu_id = PMIC_UNKNOW;
			else
				vcpu_id = FP6359;

			set_pmic_id(vcpu_id, vcore_id);
			ret = 2;
		}
	} else {
		g_pmic_config_valid = 0;
		vcore_id = PMIC_UNKNOW;
		vcpu_id = PMIC_UNKNOW;
		lgpl_printf("PMIC: unknown PMIC model\n");
		set_pmic_id(vcpu_id, vcore_id);
	}

	return ret;
}

const char* pmic_get_model_name(UNSG32 model_id)
{
	if (model_id < sizeof(pmic_model_names) / sizeof(pmic_model_names[0]) &&
		pmic_model_names[model_id] != NULL) {
		return pmic_model_names[model_id];
	}

	return "unknown";
}
