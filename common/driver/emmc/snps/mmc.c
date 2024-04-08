/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Define a simple and generic interface to access eMMC and SD-card devices. */

#include "debug.h"
#include "mmc.h"
#include "string.h"
#include "dwcmshc_mmc.h"
#include "cache.h"

#define MMC_DEFAULT_MAX_RETRIES		5
#define SEND_OP_COND_MAX_RETRIES	100

#define MULT_BY_512K_SHIFT		19
#define MMC_CAP(mode)		(1 << mode)

static const struct mmc_ops *ops;
static unsigned int mmc_ocr_value;
static struct mmc_csd_emmc mmc_csd;
static unsigned int mmc_flags;
struct mmc_device_info *mmc_dev_info;
static unsigned int rca;
static long long c_size;
static unsigned int __attribute__((aligned(64))) scr[2]  = { 0 };
static unsigned int __attribute__((aligned(64))) switch_status[16]  = { 0 };

static bool is_cmd23_enabled(void)
{
	return ((mmc_flags & MMC_FLAG_CMD23) != 0U);
}

static int mmc_send_cmd(unsigned int idx, unsigned int arg,
			unsigned int r_type, unsigned int *r_data)
{
	struct mmc_cmd cmd;
	int ret;

	memset(&cmd, 0, sizeof(struct mmc_cmd));

	cmd.cmd_idx = idx;
	cmd.cmd_arg = arg;
	cmd.resp_type = r_type;


	ret = ops->send_cmd(&cmd);

	if ((ret == 0) && (r_data != NULL)) {
		int i;

		for (i = 0; i < 4; i++) {
			*r_data = cmd.resp_data[i];
			r_data++;
		}
	}

	if (ret != 0) {
		ERR("Send command %u error: %d\n", idx, ret);
	}

	return ret;
}

static int mmc_device_state(void)
{
	int retries = 100;
	unsigned int resp_data[4];

	do {
		int ret;

		if (retries == 0) {
			ERR("CMD13 failed after %d retries\n",
			      MMC_DEFAULT_MAX_RETRIES);
			return -EIO;
		}

		ret = mmc_send_cmd(MMC_CMD(13), rca << RCA_SHIFT_OFFSET,
				   MMC_RESPONSE_R1, &resp_data[0]);
		if (ret != 0) {
			retries--;
			continue;
		}

		if ((resp_data[0] & STATUS_SWITCH_ERROR) != 0U) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return -EIO;
		}

		retries--;
	} while ((resp_data[0] & STATUS_READY_FOR_DATA) == 0U);

	return MMC_GET_STATE(resp_data[0]);
}

int mmc_set_ext_csd(unsigned int ext_cmd, enum mmc_extcsd_access access_mode, unsigned int value)
{
	int ret;

	ret = mmc_send_cmd(MMC_CMD(6),
			   EXTCSD_ACCESS_MODE(access_mode) | EXTCSD_CMD(ext_cmd) |
			   EXTCSD_VALUE(value)/* | EXTCSD_CMD_SET_NORMAL*/,
			   MMC_RESPONSE_R1B, NULL);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	do {
		ret = mmc_device_state();
		if (ret < 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}
	} while (ret == MMC_STATE_PRG);

	return 0;
}

static int mmc_sd_switch(unsigned int bus_width)
{
	int ret;
	unsigned int bus_width_arg = 2;

#ifdef CONFIG_DCACHE
	invalidate_dcache_range((void*)(uintptr_t)scr, (void*)(scr + sizeof(scr)/sizeof(scr[0])));
#endif

	ret = ops->prepare(0, (uintptr_t)scr, sizeof(scr));
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	int retries = MMC_DEFAULT_MAX_RETRIES;
	/* CMD55: Application Specific Command */
	ret = mmc_send_cmd(MMC_CMD(55), rca << RCA_SHIFT_OFFSET,
			   MMC_RESPONSE_R1, NULL);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	/* ACMD51: SEND_SCR */
	do {

		ret = mmc_send_cmd(MMC_ACMD(51), 0, MMC_RESPONSE_R1, NULL); //block size 64
		if ((ret != 0) && (retries == 0)) {
			ERR("%s, %d, ACMD51 failed after %d retries (ret=%d)\n",
			      __func__, __LINE__, MMC_DEFAULT_MAX_RETRIES, ret);
			return ret;
		}

		retries--;
	} while (ret != 0);

#ifdef CONFIG_DCACHE
	invalidate_dcache_range((void*)(uintptr_t)scr, (void*)(scr + sizeof(scr)/sizeof(scr[0])));
#endif

	scr[0] = __be32_to_cpu(scr[0]);

	//parse SCR for SD version v1.0 or don't support 4bit,then return
	if (((scr[0] >> 16) & SD_SCR_BUS_WIDTH_4) == 0U) {
		ERR("card don't support 4bit\n");
		return -1;
	}

	if ((((scr[0] >> 16) & SD_SCR_BUS_WIDTH_4) != 0U) &&
	    (bus_width == MMC_BUS_WIDTH_4)) {
		bus_width_arg = 2;
	}

	/* CMD55: Application Specific Command */
	ret = mmc_send_cmd(MMC_CMD(55), rca << RCA_SHIFT_OFFSET,
			   MMC_RESPONSE_R1, NULL);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	/* ACMD6: SET_BUS_WIDTH */
	ret = mmc_send_cmd(MMC_ACMD(6), bus_width_arg, MMC_RESPONSE_R1, NULL);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	do {
		ret = mmc_device_state();
		if (ret < 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}
	} while (ret == MMC_STATE_PRG);

	return 0;
}



#define SD_SWITCH_CHECK		0
#define SD_SWITCH_SWITCH	1


static int sd_switch(int mode, int group, unsigned char value, void *resp, unsigned int bufsize)
{
	struct mmc_cmd cmd;

	/* Switch the frequency */
	cmd.cmd_idx = MMC_CMD(6);//SD_CMD_SWITCH_FUNC
	cmd.resp_type = MMC_RESPONSE_R1;
	cmd.cmd_arg = (mode << 31) | 0xffffff;
	cmd.cmd_arg &= ~(0xf << (group * 4));
	cmd.cmd_arg |= value << (group * 4);

#ifdef CONFIG_DCACHE
	invalidate_dcache_range((void*)(uintptr_t)resp, (void*)((char *)resp + bufsize));
#endif

	ops->prepare(0, (uintptr_t)resp, bufsize);
	mmc_send_cmd(cmd.cmd_idx, cmd.cmd_arg, cmd.resp_type, NULL);

#ifdef CONFIG_DCACHE
	invalidate_dcache_range((void*)(uintptr_t)resp, (void*)((char *)resp + bufsize));
#endif

	return 0;
}


static int sd_set_card_speed(enum bus_mode mode)
{
	int err;

	int speed;

	switch (mode) {
	case SD_LEGACY:
		speed = UHS_SDR12_BUS_SPEED;
		break;
	case SD_HS:
		speed = HIGH_SPEED_BUS_SPEED;
		break;
	case UHS_SDR12:
		speed = UHS_SDR12_BUS_SPEED;
		break;
	case UHS_SDR25:
		speed = UHS_SDR25_BUS_SPEED;
		break;
	case UHS_SDR50:
		speed = UHS_SDR50_BUS_SPEED;
		break;
	case UHS_DDR50:
		speed = UHS_DDR50_BUS_SPEED;
		break;
	case UHS_SDR104:
		speed = UHS_SDR104_BUS_SPEED;
		break;
	default:
		return -1;
	}

	err = sd_switch(SD_SWITCH_SWITCH, 0, speed, (unsigned char *)switch_status, sizeof(switch_status));
	if (err)
		return err;

	if (((__be32_to_cpu(switch_status[4]) >> 24) & 0xF) != speed)
		return -1;

	return 0;
}
static int mmc_set_ios(unsigned int clk, unsigned int bus_width)
{
	int ret = 0;
	unsigned int width = bus_width;
	int retry = 0;
	unsigned int sd3_bus_mode;
	unsigned int card_caps = 0;

	if (mmc_dev_info->mmc_dev_type != MMC_IS_EMMC) {
		if (width == MMC_BUS_WIDTH_8) {
			ERR("Wrong bus config for SD-card, force to 4\n");
			width = MMC_BUS_WIDTH_4;
		}

		ret = mmc_sd_switch(width);
		if (ret != 0) {
			ERR("Switch SD width error\n");
			return ret;
		}

		//set host bus width
		ops->set_ios(SDCLK_SEL_DIV_1024, MMC_BUS_WIDTH_4);

		retry = 4;
		while (retry--) {
			memset(switch_status, 0, sizeof(switch_status));

			ret = sd_switch(SD_SWITCH_CHECK, 0, 1, switch_status, sizeof(switch_status));

			if (ret)
				return ret;

			/* The high-speed function is busy.  Try again */
			if (!(__be32_to_cpu(switch_status[7]) & SD_HIGHSPEED_BUSY))
				break;
		}

		if (__be32_to_cpu(switch_status[3]) & SD_HIGHSPEED_SUPPORTED)
			ERR("high speed support\n");

		sd3_bus_mode = __be32_to_cpu(switch_status[3]) >> 16 & 0x1f;

		if (sd3_bus_mode & SD_MODE_UHS_SDR104)
			card_caps |= MMC_CAP(UHS_SDR104);
		if (sd3_bus_mode & SD_MODE_UHS_SDR50)
			card_caps |= MMC_CAP(UHS_SDR50);
		if (sd3_bus_mode & SD_MODE_UHS_SDR25)
			card_caps |= MMC_CAP(UHS_SDR25);
		if (sd3_bus_mode & SD_MODE_UHS_SDR12)
			card_caps |= MMC_CAP(UHS_SDR12);
		if (sd3_bus_mode & SD_MODE_UHS_DDR50)
			card_caps |= MMC_CAP(UHS_DDR50);

		//CMD6 set card speed ,block size 64
		ret = sd_set_card_speed(SD_HS);

		//set host bus mode and clock
		ops->set_ios(SDCLK_SEL_DIV_4, MMC_BUS_WIDTH_4);

		//TODO if set host error,then need return devices to legacy mode,and go other mode

		return 0;
	} else {
		ret = mmc_set_ext_csd(CMD_EXTCSD_BUS_WIDTH, WRITE_BYTES,
				      (unsigned int)width);
		if (ret != 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}
		ret = mmc_set_ext_csd(CMD_EXTCSD_HS_TIMING, WRITE_BYTES,
				      1);//high speed
		if (ret != 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}
	}

	return ops->set_ios(clk, width);
}

static int sd_send_op_cond(void)
{
	int n,ret;
	unsigned int resp_data[4];
	unsigned int allowed_ocr,card_ocr = 0;

	/* CMD55: Application Specific Command */
	ret = mmc_send_cmd(MMC_CMD(55), 0, MMC_RESPONSE_R1, NULL);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	/* ACMD41: SD_SEND_OP_COND */
	ret = mmc_send_cmd(MMC_ACMD(41), 0, MMC_RESPONSE_R3, &resp_data[0]);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	card_ocr = resp_data[0];

	allowed_ocr = card_ocr & mmc_dev_info->ocr_voltage;

	if(0 == allowed_ocr) {
		ERR("allowed ocr error\n");
		return -1;
	}

	if(allowed_ocr & OCR_3_5_3_6) {
		allowed_ocr = OCR_3_5_3_6;
	} else if(allowed_ocr & OCR_3_4_3_5) {
		allowed_ocr = OCR_3_4_3_5;
	} else if(allowed_ocr & OCR_3_3_3_4) {
		allowed_ocr = OCR_3_3_3_4;
	} else if(allowed_ocr & OCR_3_2_3_3) {
		allowed_ocr = OCR_3_2_3_3;
	} else if(allowed_ocr & OCR_3_1_3_2) {
		allowed_ocr = OCR_3_1_3_2;
	} else if(allowed_ocr & OCR_3_0_3_1) {
		allowed_ocr = OCR_3_0_3_1;
	} else if(allowed_ocr & OCR_2_9_3_0) {
		allowed_ocr = OCR_2_9_3_0;
	} else if(allowed_ocr & OCR_2_8_2_9) {
		allowed_ocr = OCR_2_8_2_9;
	} else if(allowed_ocr & OCR_2_7_2_8) {
		allowed_ocr = OCR_2_7_2_8;
	} else{
		ERR("ocr not matching\n");
		return -1;
	}
	for (n = 0; n < SEND_OP_COND_MAX_RETRIES; n++) {
		/* CMD55: Application Specific Command */
		ret = mmc_send_cmd(MMC_CMD(55), 0, MMC_RESPONSE_R1, NULL);
		if (ret != 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}

		/* ACMD41: SD_SEND_OP_COND */
		ret = mmc_send_cmd(MMC_ACMD(41), OCR_HCS |
			(card_ocr & ~0xff8000) | allowed_ocr, MMC_RESPONSE_R3,
			&resp_data[0]);
		if (ret != 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}

		if ((resp_data[0] & OCR_BUSY) != 0U) {
			mmc_ocr_value = resp_data[0];

			if ((mmc_ocr_value & OCR_HCS) != 0U) {
				mmc_dev_info->mmc_dev_type = MMC_IS_SD_HC;
			} else {
				mmc_dev_info->mmc_dev_type = MMC_IS_SD;
			}

			return 0;
		}

		sd_delay(10000);
	}

	ERR("%s, %d, ACMD41 failed after %d retries\n", __func__, __LINE__, SEND_OP_COND_MAX_RETRIES);

	return -EIO;
}

int mmc_reset_to_idle(void)
{
	int ret;

	sd_delay(500); //wait at least 74 clocks

	/* CMD0: reset to IDLE */
	ret = mmc_send_cmd(MMC_CMD(0), 0, 0, NULL);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	sd_delay(2000);

	return 0;
}

static int mmc_send_op_cond(void)
{
	int ret, n;
	unsigned int resp_data[4];

	ret = mmc_reset_to_idle();
	if (ret != 0) {
		return ret;
	}

	for (n = 0; n < SEND_OP_COND_MAX_RETRIES; n++) {
		ret = mmc_send_cmd(MMC_CMD(1), OCR_SECTOR_MODE |
				   OCR_VDD_MIN_2V7 | OCR_VDD_MIN_1V7,
				   MMC_RESPONSE_R3, &resp_data[0]);
		if (ret != 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}

		if ((resp_data[0] & OCR_BUSY) != 0U) {
			mmc_ocr_value = resp_data[0];
			return 0;
		}

		sd_delay(10000);
	}

	ERR("CMD1 failed after %d retries\n", SEND_OP_COND_MAX_RETRIES);

	return -EIO;
}

static int mmc_enumerate(unsigned int clk, unsigned int bus_width)
{
	int ret;
	unsigned int resp_data[4];
	unsigned int csd_version = 0;

	ops->init(mmc_dev_info);

	ret = mmc_reset_to_idle();
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	};

	if (mmc_dev_info->mmc_dev_type == MMC_IS_EMMC) {
		ret = mmc_send_op_cond();
	} else {
		/* CMD8: Send Interface Condition Command */
		ret = mmc_send_cmd(MMC_CMD(8), VHS_2_7_3_6_V | CMD8_CHECK_PATTERN,
				   MMC_RESPONSE_R7, &resp_data[0]);
		if ((ret == 0) && ((resp_data[0] & 0xffU) == CMD8_CHECK_PATTERN)) {
			ret = sd_send_op_cond();
		}
	}

	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	/* CMD2: Card Identification */
	ret = mmc_send_cmd(MMC_CMD(2), 0, MMC_RESPONSE_R2, NULL);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	/* CMD3: Set Relative Address */
	if (mmc_dev_info->mmc_dev_type == MMC_IS_EMMC) {
		rca = MMC_FIX_RCA;
		ret = mmc_send_cmd(MMC_CMD(3), rca << RCA_SHIFT_OFFSET,
				   MMC_RESPONSE_R1, NULL);
		if (ret != 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}
	} else {
		ret = mmc_send_cmd(MMC_CMD(3), 0,
				   MMC_RESPONSE_R6, &resp_data[0]);
		if (ret != 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}

		rca = (resp_data[0] & 0xFFFF0000U) >> 16;
	}

	/* CMD9: CSD Register */
	ret = mmc_send_cmd(MMC_CMD(9), rca << RCA_SHIFT_OFFSET,
			   MMC_RESPONSE_R2, &resp_data[0]);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	if (mmc_dev_info->mmc_dev_type == MMC_IS_EMMC) {
		memcpy(&mmc_csd, &resp_data, sizeof(resp_data));
	}else{
		csd_version = (resp_data[3] >> 22) & 0x3;
		if(CSD_STRUCTURE_VER_2_0 == csd_version) {
			c_size = (resp_data[1] >> 8) & 0x3FFFFF;
			INFO("SD size is %u MB\n",(c_size+1)*512/1024);
		}
	}

	/* CMD7: Select Card */
	ret = mmc_send_cmd(MMC_CMD(7), rca << RCA_SHIFT_OFFSET,
			   MMC_RESPONSE_R1, NULL);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	do {
		ret = mmc_device_state();
		if (ret < 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}
	} while (ret != MMC_STATE_TRAN);

	ret = mmc_set_ios(clk, bus_width);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	do {
		ret = mmc_device_state();
		if (ret < 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}
	} while (ret != MMC_STATE_TRAN);

	return 0;
}

long long get_mmc_size(void)
{
	return c_size;
}

size_t mmc_read_extcsd(int lba, uintptr_t buf)
{
	int ret;

	ret = ops->prepare(lba, buf, MMC_BLOCK_SIZE);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	/* MMC CMD8: SEND_EXT_CSD */
	ret = mmc_send_cmd(MMC_CMD(8), 0, MMC_RESPONSE_R1, NULL);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	do {
		ret = mmc_device_state();
		if (ret < 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}
	} while (ret != MMC_STATE_TRAN);

	return 0;
}

size_t mmc_read_blocks(int lba, uintptr_t buf, size_t size)
{
	int ret;
	unsigned int cmd_idx, cmd_arg;

	assert((ops != NULL) &&
	       (ops->read != NULL) &&
	       (size != 0U) &&
	       ((size & MMC_BLOCK_MASK) == 0U));

	ret = ops->prepare(lba, buf, size);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	if (is_cmd23_enabled()) {
		/* Set block count */
		ret = mmc_send_cmd(MMC_CMD(23), size / MMC_BLOCK_SIZE,
				   MMC_RESPONSE_R1, NULL);
		if (ret != 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}

		cmd_idx = MMC_CMD(18);
	} else {
		if (size > MMC_BLOCK_SIZE) {
			cmd_idx = MMC_CMD(18);
		} else {
			cmd_idx = MMC_CMD(17);
		}
	}

	if (((mmc_ocr_value & OCR_ACCESS_MODE_MASK) == OCR_BYTE_MODE) &&
	    (mmc_dev_info->mmc_dev_type != MMC_IS_SD_HC)) {
		cmd_arg = lba * MMC_BLOCK_SIZE;
	} else {
		cmd_arg = lba;
	}

	ret = mmc_send_cmd(cmd_idx, cmd_arg, MMC_RESPONSE_R1, NULL);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	/* Wait buffer empty */
	do {
		ret = mmc_device_state();
		if (ret < 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}
	} while ((ret != MMC_STATE_TRAN) && (ret != MMC_STATE_DATA));

	if (!is_cmd23_enabled() && (size > MMC_BLOCK_SIZE)) {
		ret = mmc_send_cmd(MMC_CMD(12), 0, MMC_RESPONSE_R1B, NULL);
		if (ret != 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}
	}

	return 0;
}

size_t rpmb_response(uintptr_t buf, size_t size)
{
	int ret;
	unsigned int cmd_idx, cmd_arg;

	assert((ops != NULL) &&
	       (ops->read != NULL) &&
	       (size != 0U) &&
	       ((size & MMC_BLOCK_MASK) == 0U));

	ret = ops->prepare(0, buf, size);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	if (is_cmd23_enabled()) {
		/* Set block count */
		ret = mmc_send_cmd(MMC_CMD(23), size / MMC_BLOCK_SIZE,
				   MMC_RESPONSE_R1, NULL);
		if (ret != 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}

		cmd_idx = MMC_CMD(18);
	} else {
		if (size > MMC_BLOCK_SIZE) {
			cmd_idx = MMC_CMD(18);
		} else {
			cmd_idx = MMC_CMD(17);
		}
	}

	cmd_arg = 0;

	ret = mmc_send_cmd(cmd_idx, cmd_arg, MMC_RESPONSE_R1, NULL);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	/* Wait buffer empty */
	do {
		ret = mmc_device_state();
		if (ret < 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}
	} while ((ret != MMC_STATE_TRAN) && (ret != MMC_STATE_DATA));

	if (!is_cmd23_enabled() && (size > MMC_BLOCK_SIZE)) {
		ret = mmc_send_cmd(MMC_CMD(12), 0, MMC_RESPONSE_R1B, NULL);
		if (ret != 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}
	}

	return 0;
}


size_t mmc_write_blocks(int lba, const uintptr_t buf, size_t size)
{
	int ret;
	unsigned int cmd_idx, cmd_arg;

	assert((ops != NULL) &&
	       (ops->write != NULL) &&
	       (size != 0U) &&
	       /*((buf & MMC_BLOCK_MASK) == 0U) &&*/
	       ((size & MMC_BLOCK_MASK) == 0U));

	ret = ops->prepare(lba, buf, size);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	if (is_cmd23_enabled()) {
		/* Set block count */
		ret = mmc_send_cmd(MMC_CMD(23), size / MMC_BLOCK_SIZE,
				   MMC_RESPONSE_R1, NULL);
		if (ret != 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}

		cmd_idx = MMC_CMD(25);
	} else {
		if (size > MMC_BLOCK_SIZE) {
			cmd_idx = MMC_CMD(25);
		} else {
			cmd_idx = MMC_CMD(24);
		}
	}

	if ((mmc_ocr_value & OCR_ACCESS_MODE_MASK) == OCR_BYTE_MODE) {
		cmd_arg = lba * MMC_BLOCK_SIZE;
	} else {
		cmd_arg = lba;
	}

	ret = mmc_send_cmd(cmd_idx, cmd_arg, MMC_RESPONSE_R1, NULL);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	/* Wait buffer empty */
	do {
		ret = mmc_device_state();
		if (ret < 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}
	} while ((ret != MMC_STATE_TRAN) && (ret != MMC_STATE_RCV));

	if (!is_cmd23_enabled() && (size > MMC_BLOCK_SIZE)) {
		ret = mmc_send_cmd(MMC_CMD(12), 0, MMC_RESPONSE_R1B, NULL);
		if (ret != 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}
	}

	return 0;
}

size_t rpmb_request(const uintptr_t buf, size_t size, int is_write)
{
	int ret;
	unsigned int cmd_idx, cmd_arg;

	assert((ops != NULL) &&
	       (ops->write != NULL) &&
	       (size != 0U) &&
	       /*((buf & MMC_BLOCK_MASK) == 0U) &&*/
	       ((size & MMC_BLOCK_MASK) == 0U));

	ret = ops->prepare(0, buf, size);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	if (is_cmd23_enabled()) {
		/* Set block count */
		if (is_write)
			ret = mmc_send_cmd(MMC_CMD(23), (size / MMC_BLOCK_SIZE) | 1 << 31,
				   MMC_RESPONSE_R1, NULL);
		else
			ret = mmc_send_cmd(MMC_CMD(23), size / MMC_BLOCK_SIZE,
				   MMC_RESPONSE_R1, NULL);

		if (ret != 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}

		cmd_idx = MMC_CMD(25);
	} else {
		if (size > MMC_BLOCK_SIZE) {
			cmd_idx = MMC_CMD(25);
		} else {
			cmd_idx = MMC_CMD(24);
		}
	}

	cmd_arg = 0;

	ret = mmc_send_cmd(cmd_idx, cmd_arg, MMC_RESPONSE_R1, NULL);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	/* Wait buffer empty */
	do {
		ret = mmc_device_state();
		if (ret < 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}
	} while ((ret != MMC_STATE_TRAN) && (ret != MMC_STATE_RCV));

	if (!is_cmd23_enabled() && (size > MMC_BLOCK_SIZE)) {
		ret = mmc_send_cmd(MMC_CMD(12), 0, MMC_RESPONSE_R1B, NULL);
		if (ret != 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}
	}

	return 0;
}


size_t mmc_erase_blocks(int lba, size_t size)
{
	int ret;

	assert(ops != NULL);
	assert((size != 0U) && ((size & MMC_BLOCK_MASK) == 0U));

	//add by clu
	if ((mmc_ocr_value & OCR_ACCESS_MODE_MASK) == OCR_BYTE_MODE)
		lba *= MMC_BLOCK_SIZE;

	ret = mmc_send_cmd(MMC_CMD(35), lba, MMC_RESPONSE_R1, NULL);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	ret = mmc_send_cmd(MMC_CMD(36), lba + (size / MMC_BLOCK_SIZE) - 1U,
			   MMC_RESPONSE_R1, NULL);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	ret = mmc_send_cmd(MMC_CMD(38), CMD38_TRIM_ENABLE, MMC_RESPONSE_R1B, NULL);
	if (ret != 0) {
		ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
		return ret;
	}

	do {
		ret = mmc_device_state();
		if (ret < 0) {
			ERR("%s, %d, ret = 0x%x\n",__func__, __LINE__, ret);
			return ret;
		}
	} while (ret != MMC_STATE_TRAN);

	return 0;
}

static inline void mmc_rpmb_enable(void)
{
	mmc_set_ext_csd(CMD_EXTCSD_PARTITION_CONFIG, WRITE_BYTES,
			PART_CFG_BOOT_PARTITION1_ENABLE |
			PART_CFG_PARTITION1_ACCESS);
}

static inline void mmc_rpmb_disable(void)
{
	mmc_set_ext_csd(CMD_EXTCSD_PARTITION_CONFIG, WRITE_BYTES,
			PART_CFG_BOOT_PARTITION1_ENABLE);
}

size_t mmc_rpmb_read_blocks(int lba, uintptr_t buf, size_t size)
{
	size_t size_read;

	mmc_rpmb_enable();
	size_read = mmc_read_blocks(lba, buf, size);
	mmc_rpmb_disable();

	return size_read;
}

size_t mmc_rpmb_write_blocks(int lba, const uintptr_t buf, size_t size)
{
	size_t size_written;

	mmc_rpmb_enable();
	size_written = mmc_write_blocks(lba, buf, size);
	mmc_rpmb_disable();

	return size_written;
}

size_t mmc_rpmb_erase_blocks(int lba, size_t size)
{
	size_t size_erased;

	mmc_rpmb_enable();
	size_erased = mmc_erase_blocks(lba, size);
	mmc_rpmb_disable();

	return size_erased;
}

int mmc_init(const struct mmc_ops *ops_ptr, unsigned int clk,
	     unsigned int width, unsigned int flags,
	     struct mmc_device_info *device_info)
{
	ops = ops_ptr;
	mmc_flags = flags;
	mmc_dev_info = device_info;

	return mmc_enumerate(clk, width);
}
