/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright (C) 2013-2021 Synaptics Incorporated. All rights reserved.
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
#include "init_emmc.h"
#include "mmc.h"
#include "dwcmshc_mmc.h"
#include "flash_adaptor.h"
#include "global.h"
#include "apbRegBase.h"
#include "apb_gpio.h"
#include "irq.h"
#include "soc.h"
#include "string.h"
#include "cache.h"

#define CONFIG_DWCMSHC_PHY_SUPPORT

#define SDMASA_R 0x0

#define BLOCKSIZE_R 0x4
#define XFER_BLOCK_SIZE (0xfff << 0)
#define SDMA_BUF_BDARY  (0x7 << 12)
#define SDMA_512k_BDRY   0x7

#define BLOCKCOUNT_R 0x6
#define BLOCK_CNT  (0xffff << 0)

#define ARGUMENT_R 0x8

#define XFER_MODE_R 0xc
#define DMA_ENABLE (1 << 0)
#define BLOCK_COUNT_ENABLE (1 << 1)
#define AUTO_CMD_ENABLE (3<<2)
#define DATA_XFER_DIR (1<<4)
#define MULTI_BLK_SEL (1<<5)

#define CMD_R 0xe
#define RESP_TYPE_SELECT   (3 << 16)
#define NO_RESP 0
#define RESP_LEN_136 1
#define RESP_LEN_48 2
#define RESP_LEN_48B 3
#define CMD_CRC_CHK_ENABLE (1 << 19)
#define CMD_IDX_CHK_ENABLE (1 << 20)
#define DATA_PRESENT_SEL   (1 << 21)
#define CMD_DATA            1
#define CMD_NODATA          0
#define CMD_TYPE           (3 << 22)
#define NORMAL_CMD_TYPE     0x0
#define SUSPEND_CMD_TYPE    0x1
#define RESUME_CMD_TYPE     0x2
#define ABORT_CMD_TYPE      0x3

#define CMD_INDEX          (0x3f << 24)

#define RESP01_R 0x10
#define RESP23_R 0x14
#define RESP45_R 0x18
#define RESP67_R 0x1c

#define PSTATE_REG             0x24
#define CMD_INHIBIT           (1 << 0)
#define CMD_INHIBIT_DAT       (1 << 1)
#define DAT_LINE_ACTIVE       (1 << 2)
#define CARD_DETECT_PIN_LEVEL (1 << 18)

#define HOST_CTRL1_R     0x28
#define DAT_XFER_WIDTH  (1 << 1)
#define CTRL_HISPD		(1 << 2)
#define DMA_SEL         (3 << 3)
#define EXT_DAT_XFER    (1 << 5)
#define SDMA_SELECT      0
#define ADMA2_SELECT     2
#define ADMA2_3_SELECT   3

#define PWR_CTRL_R       0x29
#define SD_BUS_PWR_VDD1  BIT(0)
#define SD_BUS_VOL_VDD1 (0x7 << 1)
#define V_1_2            0x5
#define V_1_8            0x6
#define V_3_3            0x7

#define CLK_CTRL_R           0x2c
#define INTERNAL_CLK_EN     (1 << 0)
#define INTERNAL_CLK_STABLE (1 << 1)
#define SD_CLK_EN           (1 << 2)
#define UPPER_FREQ_SEL      (3 << 6)
#define FREQ_SEL            (0xf << 8)

#define TOUT_CTRL_R  0x2e
#define TOUT_CNT    (0xf << 0)
#define MM4CLOCK187KHZRATE      0x80
#define MM4CLOCK48MHZRATE       0x0
#define CLOCK_27_MULT           0xE

#define SW_RST_R       0x2f
#define SW_RST_ALL     BIT(0)
#define SW_RST_CMD     BIT(1)
#define SW_RST_DAT     BIT(2)

#define NORMAL_INT_STAT_R 0x30
#define ERR_INTERRUPT BIT(15)
#define CQE_EVENT BIT(14)
#define FX_EVENT BIT(13)
#define RE_TUNE_EVENT BIT(12)
#define INT_C BIT(11)
#define INT_B BIT(10)
#define INT_A BIT(9)
#define CARD_INTERRUPT BIT(8)
#define CARD_REMOVAL BIT(7)
#define CARD_INSERTION BIT(6)
#define BUF_RD_READY BIT(5)
#define BUF_WR_READY BIT(4)
#define DMA_INTERRUPT BIT(3)
#define BGAP_EVENT BIT(2)
#define XFER_COMPLETE BIT(1)
#define CMD_COMPLETE BIT(0)

#define ERROR_INT_STAT_R 0x32
#define VENDOR_ERR3 BIT(15)
#define VENDOR_ERR2 BIT(14)
#define VENDOR_ERR1 BIT(13)
#define BOOT_ACK_ERR BIT(12)
#define RESP_ERR BIT(11)
#define TUNING_ERR BIT(10)
#define ADMA_ERR BIT(9)
#define AUTO_CMD_ERR BIT(8)
#define CUR_LMT_ERR BIT(7)
#define DATA_END_BIT_ERR BIT(6)
#define DATA_CRC_ERR BIT(5)
#define DATA_TOUT_ERR BIT(4)
#define CMD_IDX_ERR BIT(3)
#define CMD_END_BIT_ERR BIT(2)
#define CMD_CRC_ERR BIT(1)
#define CMD_TOUT_ERR BIT(0)

#define NORMAL_INT_STAT_EN_R 0x34
#define NORMAL_INT_SIGNAL_EN_R 0x38
#define CMD_COMPLETE_STAT	BIT(0)
#define XFER_COMPLETE_STAT	BIT(1)
#define DMA_INTERRUPT_STAT	BIT(3)
#define BUF_WR_READY_STAT	BIT(4)
#define BUF_RD_READY_STAT	BIT(5)
#define CARD_INSERTION_STAT	BIT(6)
#define CARD_REMOVAL_STAT	BIT(7)
#define CARD_INTERRUPT_STAT	BIT(8)
#define CMD_TOUT_ERR_STAT	BIT(16)
#define CMD_CRC_ERR_STAT		BIT(17)
#define CMD_END_BIT_ERR_STAT	BIT(18)
#define CMD_IDX_ERR_STAT	BIT(19)
#define DATA_TOUT_ERR_STAT	BIT(20)
#define DATA_CRC_ERR_STAT	BIT(21)
#define DATA_END_BIT_ERR_STAT	BIT(22)
#define AUTO_CMD_ERR_STAT	BIT(24)
#define ADMA_ERR_STAT	BIT(25)
#define BOOT_ACK_ERR_SIGNAL BIT(28)

#define AUTO_CMD_STAT_R 0x3c
#define CMD_NOT_ISSUED_AUTO_CMD12 BIT(7)
#define AUTO_CMD_RESP_ERR BIT(5)
#define AUTO_CMD_IDX_ERR BIT(4)
#define AUTO_CMD_EBIT_ERR BIT(3)
#define AUTO_CMD_CRC_ERR BIT(2)
#define AUTO_CMD_TOUT_ERR BIT(1)
#define AUTO_CMD12_NOT_EXEC BIT(0)

#define HOST_CTRL2_R    0x3e
#define SIGNALING_EN   (1 << 3)
#define UHS2_IF_ENABLE (1 << 8)
#define UHS_MODE_SEL   (7 << 0)
#define EMMC_HOSTMODE_HS400 (7 << 0)
#define EMMC_HOSTMODE_HS200 (3 << 0)
#define EMMC_HOSTMODE_HSDDR (4 << 0)
#define EMMC_HOSTMODE_HSSDR (1 << 0)
#define EMMC_HOSTMODE_LEGACY (0 << 0)

#define P_VENDOR_SPECIFIC_AREA 0xe8
#define REG_OFFSET_ADDR       (0xfff << 0)
#define EMMC_CTRL_R            0x2c
#define CARD_IS_EMMC          (1 << 0)
#define EMMC_RST_N            (1 << 2)
#define EMMC_RST_N_OE         (1 << 3)

#define PHY_CNFG     0x300
#define PHY_RSTN    (1 << 0)
#define PHY_PWRGOOD (1 << 1)

#define PHY_CMDPAD_CNFG 0x304
#define PHY_DATPAD_CNFG 0x306
#define PHY_CLKPAD_CNFG 0x308
#define PHY_STBPAD_CNFG 0x30a
#define PHY_RSTNPAD_CNFG 0x30c

#define PHY_COMMDL_CNFG 0x31c
#define DLSTEP_SEL     (1 << 0)
#define DLOUT_EN       (1 << 1)

#define PHY_SDCLKDL_CNFG 0x31d
#define EXTDLY_EN       (1 << 0 )
#define BYPASS_EN       (1 << 1)
#define INPSEL_CNFG     (0x3 << 2)
#define UPDATE_DC       (1 << 4)

#define PHY_SDCLKDL_DC  0x31e
#define CCKDL_DC       (0x7f << 0)

#define PHY_SMPLDL_CNFG    0x320
#define S_EXTDLY_EN       (1 << 0)
#define S_BYPASS_EN       (1 << 1)
#define S_INPSEL_CNFG     (3 << 2)
#define S_INPSEL_OVERRIDE (1 << 4)

#define PHY_ATDL_CNFG  0x321
#define A_EXTDLY_EN   (1<<0)
#define A_BYPASS_EN   (1<<1)
#define A_INPSEL_CNFG (3<<2)

#define PHY_AT_CTRL_R     0x540
#define TUNE_CLK_STOP_EN (1<<16)
#define PRE_CHANGE_DLY   (3<<17)
#define POST_CHANGE_DLY  (3<<19)

#define EMMC_DMA_BOUNDARY 0x8000000

static void dwcmshc_init(struct mmc_device_info *dev_info);
static int dwcmshc_send_cmd(struct mmc_cmd *cmd);
static int dwcmshc_set_ios(unsigned int clk, unsigned int width);
static int dwcmshc_prepare(int lba, uintptr_t buf, size_t size);
static int dwcmshc_read(int lba, uintptr_t buf, size_t size);
static int dwcmshc_write(int lba, uintptr_t buf, size_t size);

static unsigned int current_part = 0xF;
static volatile mmc_response mmc_resp;
static const struct mmc_ops dwcmshc_mmc_ops = {
	.init		= dwcmshc_init,
	.send_cmd	= dwcmshc_send_cmd,
	.set_ios	= dwcmshc_set_ios,
	.prepare	= dwcmshc_prepare,
	.read		= dwcmshc_read,
	.write		= dwcmshc_write,
};

static dwcmshc_mmc_params_t dwcmshc_params;

#if defined(CONFIG_DWCMSHC_PHY_SUPPORT)
#define PHY_GEN_SETTING(pad_sp, pad_sn) \
	 ((((pad_sp) & 0x0f) << 16) | \
	 (((pad_sn) & 0x0f) << 20))
#define PHY_GEN_MASK PHY_GEN_SETTING(0xf, 0xf)

struct phy_gen_setting {
	unsigned int reg;
	unsigned int mask;
	unsigned int setting;
};

#define PHY_PAD_SETTING(rxsel, weakpull_en, txslew_ctrl_p, txslew_ctrl_n) \
	(((rxsel) & 0x3) | \
	(((weakpull_en) & 0x3) << 3) | \
	(((txslew_ctrl_p) & 0xf) << 5) | \
	((txslew_ctrl_n) & 0xf) << 9)
#define PHY_PAD_MASK PHY_PAD_SETTING(0x3, 0x3, 0xf, 0xf)
#define PHY_PAD_SETTING_NUM 5

struct phy_pad_setting {
	unsigned int reg;
	unsigned short mask;
	unsigned short setting;
};

struct phy_gen_setting gen_setting_1v8 = {PHY_CNFG, PHY_GEN_MASK, PHY_GEN_SETTING(PAD_SP_8, PAD_SN_8)};
struct phy_pad_setting pad_setting_1v8[PHY_PAD_SETTING_NUM] = {
	{PHY_CMDPAD_CNFG, PHY_PAD_MASK, PHY_PAD_SETTING(SCHMITT1P8, WPE_PULLUP, TX_SLEW_P_0, TX_SLEW_N_3)},
	{PHY_DATPAD_CNFG, PHY_PAD_MASK, PHY_PAD_SETTING(SCHMITT1P8, WPE_PULLUP, TX_SLEW_P_0, TX_SLEW_N_3)},
	{PHY_CLKPAD_CNFG, PHY_PAD_MASK, PHY_PAD_SETTING(RXSELOFF, WPE_DISABLE, TX_SLEW_P_0, TX_SLEW_N_3)},
	{PHY_STBPAD_CNFG, PHY_PAD_MASK, PHY_PAD_SETTING(SCHMITT1P8, WPE_PULLDOWN, TX_SLEW_P_0, TX_SLEW_N_3)},
	{PHY_RSTNPAD_CNFG, PHY_PAD_MASK, PHY_PAD_SETTING(SCHMITT1P8, WPE_PULLUP, TX_SLEW_P_0, TX_SLEW_N_3) }
};

struct phy_gen_setting gen_setting_3v3 = {PHY_CNFG, PHY_GEN_MASK, PHY_GEN_SETTING(PAD_SP_9, PAD_SN_8)};
struct phy_pad_setting pad_setting_3v3[PHY_PAD_SETTING_NUM] = {
	{PHY_CMDPAD_CNFG, PHY_PAD_MASK, PHY_PAD_SETTING(SCHMITT3P3, WPE_PULLUP, TX_SLEW_P_3, TX_SLEW_N_2)},
	{PHY_DATPAD_CNFG, PHY_PAD_MASK, PHY_PAD_SETTING(SCHMITT3P3, WPE_PULLUP, TX_SLEW_P_3, TX_SLEW_N_2)},
	{PHY_CLKPAD_CNFG, PHY_PAD_MASK, PHY_PAD_SETTING(RXSELOFF, WPE_DISABLE, TX_SLEW_P_3, TX_SLEW_N_2)},
	{PHY_STBPAD_CNFG, PHY_PAD_MASK, PHY_PAD_SETTING(SCHMITT3P3, WPE_PULLDOWN, TX_SLEW_P_3, TX_SLEW_N_2)},
	{PHY_RSTNPAD_CNFG, PHY_PAD_MASK, PHY_PAD_SETTING(SCHMITT3P3, WPE_PULLUP, TX_SLEW_P_3, TX_SLEW_N_2) }
};
#endif

#ifdef MMC_DEBUG
void emmc_print_regs(unsigned int start, unsigned int end)
{
	int i;
	end = (end + 0xf) & ~0xf;
	for(i = 0;i <= (end-start+4)/16;i += 1)
		dbg_printf(MMC_PRN_DEBUG_LEVEL, "%08x:  %08x %08x %08x %08x\n",
					MEMMAP_EMMC_REG_BASE + start + i*0x10,
					readl((MEMMAP_EMMC_REG_BASE + start + i*0x10)),
					readl((MEMMAP_EMMC_REG_BASE + start + 4 + i*0x10)),
					readl((MEMMAP_EMMC_REG_BASE + start + 8 + i*0x10)),
					readl((MEMMAP_EMMC_REG_BASE + start + 0xc + i*0x10)));
}
#endif

void emmc_reset(struct mmc_device_info *dev_info)
{
	unsigned int val, offset;

	/* clock enable for modules using perifSysClk as clock source */
	REG_READ32(MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_clkEnable, &val);
	if (dev_info->mmc_dev_type == MMC_IS_EMMC)
		val |= (0x1 << LSb32Gbl_clkEnable_emmcSysClkEn);
	else
		val |= (0x1 << LSb32Gbl_clkEnable_sdioSysClkEn);
	REG_WRITE32(MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_clkEnable, val);

	/* eMMC reset host */
	REG_READ32(MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_perifReset, &val);
	if (dev_info->mmc_dev_type == MMC_IS_EMMC) {
		val &= ~MSK32Gbl_perifReset_emmcSyncReset;
		val |= 0x1<<LSb32Gbl_perifReset_emmcSyncReset;
	} else {
		val &= ~MSK32Gbl_perifReset_sdioSyncReset;
		val |= 0x1<<LSb32Gbl_perifReset_sdioSyncReset;
	}
	REG_WRITE32(MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_perifReset, val);

	sd_delay(1);

	if (dev_info->mmc_dev_type == MMC_IS_EMMC)
		val &= ~MSK32Gbl_perifReset_emmcSyncReset;
	else
		val &= ~MSK32Gbl_perifReset_sdioSyncReset;
	REG_WRITE32(MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_perifReset, val);

	/* Card connected to MSHC is an eMMC card */
	offset = mmc_readl(P_VENDOR_SPECIFIC_AREA);
	offset = (offset & REG_OFFSET_ADDR) + EMMC_CTRL_R;

	if (dev_info->mmc_dev_type == MMC_IS_EMMC) {
		val = mmc_readl(offset);
		val |= CARD_IS_EMMC;
		mmc_writel(val, offset);
		sd_delay(1);
	}

	/* Reset to eMMC device asserted (active low)  */
	val = mmc_readl(offset);
	val |= EMMC_RST_N_OE;
	val &= ~EMMC_RST_N;
	mmc_writel(val, offset);
	sd_delay(1000);

	/* Reset to eMMC device is deasserted  */
	val = mmc_readl(offset);
	val |= EMMC_RST_N;
	mmc_writel(val, offset);
}

void set_controller_voltage(struct mmc_device_info *dev_info)
{
	unsigned int val;

	if (dev_info->mmc_dev_type == MMC_IS_EMMC) {
		val = mmc_readl(AUTO_CMD_STAT_R);
		val |= (SIGNALING_EN<<16);
		mmc_writel(val, AUTO_CMD_STAT_R);

		/* Set the voltage 1.8V to controller, Enable Bus Power */
		val = mmc_readl(HOST_CTRL1_R);
		val &= ~(SD_BUS_VOL_VDD1<<8);
		val |= (V_1_8<<9);
		val |= (SD_BUS_PWR_VDD1<<8);
		mmc_writel(val, HOST_CTRL1_R);
	} else {
		val = mmc_readl(AUTO_CMD_STAT_R);
		val &= ~(SIGNALING_EN<<16);
		mmc_writel(val, AUTO_CMD_STAT_R);

		/* Set the voltage 3.3V to controller, Enable Bus Power */
		val = mmc_readl(HOST_CTRL1_R);
		val &= ~(SD_BUS_VOL_VDD1<<8);
		val |= (V_3_3<<9);
		val |= (SD_BUS_PWR_VDD1<<8);
		mmc_writel(val, HOST_CTRL1_R);
	}
}

#if defined(CONFIG_DWCMSHC_PHY_SUPPORT)
/* tx_delay_code is from DIAG team */
void phy_delayline_setup(unsigned int tx_delay_code, unsigned int extdly_en)
{
	unsigned int val1;
	unsigned char val2;

	/* disable SD/eMMC clock */
	val1 = mmc_readl(CLK_CTRL_R);
	val1 &= ~SD_CLK_EN;
	mmc_writel(val1, CLK_CTRL_R);
	udelay(1);

	val2 = mmc_readb(PHY_SDCLKDL_CNFG);
	val2 |= UPDATE_DC;
	if(extdly_en)
		val2 |= EXTDLY_EN;
	else
		val2 &= ~EXTDLY_EN;
	mmc_writeb(val2, PHY_SDCLKDL_CNFG);

	val2 = mmc_readb(PHY_SDCLKDL_DC);
	val2 &= ~CCKDL_DC;
	val2 |= tx_delay_code;
	mmc_writeb(val2, PHY_SDCLKDL_DC);

	val2 = mmc_readb(PHY_SDCLKDL_CNFG);
	val2 &= ~UPDATE_DC;
	mmc_writeb(val2, PHY_SDCLKDL_CNFG);

	/* enable SD/eMMC clock */
	val1 = mmc_readl(CLK_CTRL_R);
	val1 |= SD_CLK_EN;
	mmc_writel(val1, CLK_CTRL_R);
	return;
}

void phy_tuning_setup(void)
{
	unsigned int val;
	val = mmc_readl(PHY_AT_CTRL_R);
	val = val & ~TUNE_CLK_STOP_EN & ~POST_CHANGE_DLY & ~PRE_CHANGE_DLY;
	val |= (1<<16) | (3<<17) | (3<<19);
	mmc_writel(val, PHY_AT_CTRL_R);
}

void phy_config(struct mmc_device_info *dev_info)
{
	struct phy_gen_setting *gen_setting = &gen_setting_1v8;
	struct phy_pad_setting *pad_setting = pad_setting_1v8;

	int i;
	volatile unsigned short valw;
	volatile unsigned int val;
	int timeout = 100;

	if (dev_info->mmc_dev_type == MMC_IS_SD) {
		gen_setting = &gen_setting_3v3;
		pad_setting = pad_setting_3v3;
	}

	val = mmc_readl(gen_setting->reg);
	val &= ~(gen_setting->mask);
	val |= gen_setting->setting;
	mmc_writel(val, gen_setting->reg);

	for (i = 0; i < PHY_PAD_SETTING_NUM; i++) {
		valw = mmc_readw(pad_setting[i].reg);
		valw &= ~(pad_setting[i].mask);
		valw |= pad_setting[i].setting;
		mmc_writew(valw, pad_setting[i].reg);
	}

	do {
		sd_delay(1000);
		val = mmc_readl(PHY_CNFG);

		if (!timeout) {
			ERR("%s, PHY config FAIL!\n", __func__);
			reset_soc();
		}
	} while (!(val & PHY_PWRGOOD) && timeout--);
}


void phy_deassert_reset(struct mmc_device_info *dev_info)
{
	unsigned int val = 0;

	/* deassert global PHY sticky reset */
	phy_deassert_global_reset(dev_info);

	/* deassert PHY reset */
	val = mmc_readl(PHY_CNFG);
	val |= PHY_RSTN;
	mmc_writel(val, PHY_CNFG);
}
#endif

void dwcmshc_host_clock_setup(unsigned int rate)
{
	unsigned int val, timeout;
	/* disable SD/eMMC Clock and internal clock */
	val = mmc_readl(CLK_CTRL_R);
	val &= ~SD_CLK_EN & ~INTERNAL_CLK_EN;
	mmc_writel(val, CLK_CTRL_R);

	mmc_writel(val, CLK_CTRL_R);
	/* Update the rate and start the clock. */
	val &= (~UPPER_FREQ_SEL & ~FREQ_SEL);
	val |= (rate & 0xff) << 8;
	val |= ((rate >> 8) & 0x3) << 6;
	val |= INTERNAL_CLK_EN;
	mmc_writel(val, CLK_CTRL_R);

	/* Wait for clock to become stable */
	/* max 150ms */
	timeout = 150;
	while (timeout) {
		val = mmc_readl(CLK_CTRL_R);
		if (val & INTERNAL_CLK_STABLE)
			break;
		sd_delay(1);
		timeout--;
	}

	if (!timeout) {
		ERR("%s: Internal clock never stabilised.\n", __func__);
		reset_soc();
	}

	val = mmc_readl(HOST_CTRL1_R);

	switch (rate) {
		//SD Card Frequency division macro defines parameters
		case SDCLK_SEL_DIV_4:
		case SDCLK_SEL_DIV_2:
		case SDCLK_SEL_DIV_1:

		//EMMC Frequency division Decimal value
		case 4:
		//case 2: same as SDCLK_SEL_DIV_4
		//case 1: same as SDCLK_SEL_DIV_2
			val |= CTRL_HISPD;
			break;
		default:
			val &= ~CTRL_HISPD;
			break;
	}

	mmc_writel(val, HOST_CTRL1_R);
	/* enable SD/eMMC Clock */
	val = mmc_readl(CLK_CTRL_R);
	val |= SD_CLK_EN;
	mmc_writel(val, CLK_CTRL_R);
}

void dwcmshc_set_int_enable(INT_SOURCE_INDEX loc, unsigned int enable)
{
	unsigned int val;
	val = mmc_readl(NORMAL_INT_STAT_EN_R);
	val &= ~(1 << loc);
	val |= ((enable & 1) << loc);
	mmc_writel(val, NORMAL_INT_STAT_EN_R);
	mmc_writel(val, NORMAL_INT_SIGNAL_EN_R);
}

void emmc_init(struct mmc_device_info *dev_info)
{
	unsigned int val;
	/* need to do swreset to host controller per SPEC */
	val = mmc_readl(CLK_CTRL_R);
	val |= (SW_RST_ALL<<24);
	mmc_writel(val, CLK_CTRL_R);
	sd_delay(5000);

	/* emmc mode enable */
	/* emmc high speed mode */
	val = mmc_readl(AUTO_CMD_STAT_R);
	val &= ~(UHS2_IF_ENABLE<<16);
	val &= ~(UHS_MODE_SEL << 16);
	if (dev_info->mmc_dev_type == MMC_IS_EMMC)
		val |= (EMMC_HOSTMODE << 16);
	mmc_writel(val, AUTO_CMD_STAT_R);

#if defined(CONFIG_DWCMSHC_PHY_SUPPORT)
	phy_config(dev_info);
	phy_deassert_reset(dev_info);
	/* From DIAG */
	phy_delayline_setup(EMMC_TX_DELAY_CODE, 0);
	phy_tuning_setup();
#endif
	set_controller_voltage(dev_info);
	dwcmshc_host_clock_setup(SDCLK_SEL_DIV_1024);

	/* Set Read Response Timeout,0xE - TMCLK x 2^27 */
	val = mmc_readl(CLK_CTRL_R);
	val &= ~(TOUT_CNT<<16);
	val |= (CLOCK_27_MULT << 16);
	mmc_writel(val, CLK_CTRL_R);

	/* enable emmc interrupts */
	val = mmc_readl(NORMAL_INT_SIGNAL_EN_R);
	val |= CMD_COMPLETE_STAT | XFER_COMPLETE_STAT | DMA_INTERRUPT_STAT |
		BUF_WR_READY_STAT | BUF_RD_READY_STAT | CARD_INSERTION_STAT |
		CARD_REMOVAL_STAT | CMD_TOUT_ERR_STAT |
		CMD_CRC_ERR_STAT | CMD_END_BIT_ERR_STAT | CMD_IDX_ERR_STAT |
		DATA_TOUT_ERR_STAT | DATA_CRC_ERR_STAT | DATA_END_BIT_ERR_STAT |
		AUTO_CMD_ERR_STAT | ADMA_ERR_STAT | BOOT_ACK_ERR_SIGNAL;
	mmc_writel(val, NORMAL_INT_SIGNAL_EN_R);

	/* unmask interrupt */
	val = mmc_readl(NORMAL_INT_STAT_EN_R);
	val |= CMD_COMPLETE_STAT | DMA_INTERRUPT_STAT |
		BUF_WR_READY_STAT | BUF_RD_READY_STAT | CARD_INSERTION_STAT |
		CARD_REMOVAL_STAT | CMD_TOUT_ERR_STAT |
		CMD_CRC_ERR_STAT | CMD_END_BIT_ERR_STAT | CMD_IDX_ERR_STAT |
		DATA_TOUT_ERR_STAT | DATA_CRC_ERR_STAT | DATA_END_BIT_ERR_STAT |
		AUTO_CMD_ERR_STAT | ADMA_ERR_STAT | BOOT_ACK_ERR_SIGNAL;
	mmc_writel(val, NORMAL_INT_STAT_EN_R);

}

void emmc_wait_card_detect()
{
	unsigned int count, val;

	count = 8000000;
	while (count) {
		val = mmc_readl(PSTATE_REG);
		if (val & CARD_DETECT_PIN_LEVEL)
			break;
		count--;
	}

	if (!count) {
		ERR("%s: No card present\n", __func__);
		reset_soc();
	}
	else
		INFO("card preset. switch power on\n");
}

void emmc_isr()
{
	unsigned int int_stat, int_err_stat, val1, i, complete;
	int_stat = mmc_readl(NORMAL_INT_STAT_R);

	/* Clear the interrupts */
	mmc_writel(int_stat, NORMAL_INT_STAT_R); //clear INT
	val1 = mmc_readl(NORMAL_INT_STAT_R);//Must make a dummy read to allow interrupts to clear.

	if (int_stat & CARD_INTERRUPT) {
		dwcmshc_set_int_enable(CD_INT, DISABLE);
		val1 = mmc_readl(NORMAL_INT_STAT_EN_R);
	}

	if (int_stat & (CARD_INSERTION | CARD_REMOVAL)) {
		mmc_writel(0xFFFF7EFF, NORMAL_INT_STAT_R); //clear INT
		val1 = mmc_readl(NORMAL_INT_STAT_R);//Must make a dummy read to allow interrupts to clear.
		return;
	}

	/* Check for any error */
	if (int_stat & ERR_INTERRUPT) {
		/* Reset MM4CMD pin in case it's hung waiting on a response */
		val1 = mmc_readl(CLK_CTRL_R);
		val1 |= (SW_RST_CMD << 24);
		mmc_writel(val1, CLK_CTRL_R);

		val1 = mmc_readl(CLK_CTRL_R);
		val1 |= (SW_RST_DAT << 24);
		mmc_writel(val1, CLK_CTRL_R);

		int_err_stat = int_stat >> 8;
		if (int_err_stat & AUTO_CMD_ERR) {
			val1 = mmc_readl(AUTO_CMD_STAT_R);
			/* clear the auto command error bits */
			mmc_writel(val1, AUTO_CMD_STAT_R);
		}
	}

	if (int_stat & DMA_INTERRUPT) {
		/* the transfer halted because the boundary specified in ... was reached.
		 * rewriting the sysaddr with the next address allows the transfer to resume.
		 * fortunately the sysaddr register itself contains the next address.
		 * so, just re-write the sysaddr register with its own current contents.
		 */
		val1 = mmc_readl(SDMASA_R);
		mmc_writel(val1, SDMASA_R);
		return;
	}

	for (i = 0; i < 4; i++)
		mmc_resp.pBuffer[i] = 0;

	if ((int_stat & CMD_COMPLETE) || (int_stat & XFER_COMPLETE))
	{
		complete = 1;
		mmc_resp.pBuffer[0] = mmc_readl(RESP01_R);
		mmc_resp.pBuffer[1] = mmc_readl(RESP23_R);
		mmc_resp.pBuffer[2] = mmc_readl(RESP45_R);
		mmc_resp.pBuffer[3] = mmc_readl(RESP67_R);
	}

	/* Indicate that the response has been read */
	mmc_resp.CommandComplete = complete;
	return;
}

void emmc_intr_on(struct mmc_device_info *dev_info)
{
	int irq = dev_info->mmc_dev_type == MMC_IS_EMMC ? IRQ_emmc_int : IRQ_sdio_interrupt;

	if(register_isr(emmc_isr, irq))
		ERR("mmc isr register failed\n");
	set_irq_enable(irq);
}

static void dwcmshc_init(struct mmc_device_info *dev_info)
{
	emmc_reset(dev_info);
	emmc_init(dev_info);
	emmc_intr_on(dev_info);
	emmc_wait_card_detect();
}

void dwcmshc_emmc_send_setup_command(struct mmc_cmd *cmd)
{
	unsigned int val;

	/*
	 * only need command interrupt
	 * disable xfer complete command
	 */
	dwcmshc_set_int_enable(CMD_COMP, ENABLE);
	dwcmshc_set_int_enable(XFR_COMP, DISABLE);

	// Set the Argument Field
	mmc_writel(cmd->cmd_arg, ARGUMENT_R);

	// Set the Data Transfer Command fields.
	val = mmc_readl(XFER_MODE_R);
	val &= ~CMD_INDEX;
	val |= (cmd->cmd_idx << 24);
	val &= ~CMD_TYPE;
	val |= (NORMAL_CMD_TYPE << 22);
	val &= ~DATA_PRESENT_SEL;
	if(cmd->resp_type == MMC_RESPONSE_R3) {
		val &= ~CMD_IDX_CHK_ENABLE;
		val &= ~CMD_CRC_CHK_ENABLE;
	} else if(cmd->resp_type == MMC_RESPONSE_R2) {
		val &= ~CMD_IDX_CHK_ENABLE;
		val |= CMD_CRC_CHK_ENABLE;
	} else {
		val |= CMD_IDX_CHK_ENABLE;
		val |= CMD_CRC_CHK_ENABLE;
	}

	/* Response Length 48 */
	if (cmd->resp_type & MMC_RSP_48) {
		val &= ~RESP_TYPE_SELECT;
		val |= (RESP_LEN_48 << 16);
	}

	if (cmd->resp_type & MMC_RSP_136) {
		val &= ~RESP_TYPE_SELECT;
		val |= (RESP_LEN_136<<16);
	}

	if (cmd->resp_type & MMC_RSP_BUSY) {
		val &= ~RESP_TYPE_SELECT;
		val |= (RESP_LEN_48B<<16);
	}

	val &= ~BLOCK_COUNT_ENABLE;
	val &= ~DMA_ENABLE;
	val &= ~MULTI_BLK_SEL;
	val &= ~DATA_XFER_DIR;

#ifdef MMC_DEBUG
	NOTICE("\nCMD %d:\n", cmd->cmd_idx);
	NOTICE("CMD_R: 0x%x\n", val);
	NOTICE("argument: 0x%x\n", cmd->cmd_arg);
	NOTICE("resp_type: 0x%x\n", cmd->resp_type);
#endif

	/* Kick off the command */
	mmc_writel(val, XFER_MODE_R);
	return;
}

/*
 * multi_blk_sel:0-single,1-multiple
 * data_xfer_dir:0-write,1-read
 */
void dwcmshc_emmc_send_data_command(struct mmc_cmd *cmd, unsigned int multi_blk_sel, unsigned int data_xfer_dir)
{
	unsigned int val;

	/*
	 * only need xfer interrupt
	 * disable command complete command
	 */
	dwcmshc_set_int_enable(CMD_COMP, DISABLE);
	dwcmshc_set_int_enable(XFR_COMP, ENABLE);

	// Set the Argument Field
	mmc_writel(cmd->cmd_arg, ARGUMENT_R);

	// Set the Data Transfer Command fields.
	val = mmc_readl(XFER_MODE_R);
	val &= ~CMD_INDEX;
	val |= cmd->cmd_idx << 24;
	val &= ~CMD_TYPE;
	val |= (NORMAL_CMD_TYPE << 22);
	val &= ~DATA_PRESENT_SEL;
	val |= (CMD_DATA << 21);
	val &= ~CMD_IDX_CHK_ENABLE;
	val |= CMD_CRC_CHK_ENABLE;

	if (cmd->resp_type & MMC_RSP_48) {
		val &= ~RESP_TYPE_SELECT;
		val |= (RESP_LEN_48 << 16);
	}

	if (cmd->resp_type & MMC_RSP_136) {
		val &= ~RESP_TYPE_SELECT;
		val |= (RESP_LEN_136<<16);
	}

	if (cmd->resp_type & MMC_RSP_BUSY) {
		val &= ~RESP_TYPE_SELECT;
		val |= (RESP_LEN_48B<<16);
	}

	val &= ~MULTI_BLK_SEL;
	val |= multi_blk_sel << 5;
	val &= ~DATA_XFER_DIR;
	val |= data_xfer_dir << 4;
	val &= ~AUTO_CMD_ENABLE;
	val |= BLOCK_COUNT_ENABLE;
	val |= DMA_ENABLE;

#ifdef MMC_DEBUG
	NOTICE("\nCMD %d:\n", cmd->cmd_idx);
	NOTICE("CMD_R: 0x%x\n", val);
	NOTICE("argument: 0x%x\n", cmd->cmd_arg);
	NOTICE("resp_type: 0x%x\n", cmd->resp_type);
#endif

	/* Kick off the command */
	mmc_writel(val, XFER_MODE_R);
	return;
}

static int dwcmshc_send_cmd(struct mmc_cmd *cmd)
{
	unsigned int multi_blk_sel, data_xfer_dir;
	unsigned int i = 0, count;
	assert(cmd);
	mmc_resp.R1_RESP = 0;
	mmc_resp.CommandComplete = 0;

	static unsigned int last_cmd = 0;

	count = 100;
	while(mmc_readl(PSTATE_REG) & (CMD_INHIBIT | CMD_INHIBIT_DAT | DAT_LINE_ACTIVE)) {
		count--;
		sd_delay(1000);
		if (!count) {
			ERR("%s, Host Controller is not ready, status 0x%x\n", __func__, mmc_readl(PSTATE_REG));
			break;
		}
	}

	if (!count) {
		ERR("%s, Host Controller is not ready to issue a command!\n", __func__);
		reset_soc();
	}

	switch (cmd->cmd_idx) {
	//setup command
	case 0://GO_IDLE_STATE
	case 1://SEND_OP_COND
	case 2://ALL_SEND_CID
	case 3://SET_RELATIVE_ADDR
	case 7://SELECT/DESELECT_CARD
	case 9://SEND_CSD
	case 12://STOP_TRANSMISSION
	case 13://SEND_STATUS
	case 23://SET_BLOCK_COUNT
	case 35://ERASE_GROUP_START
	case 36://ERASE_GROUP_END
	case 38://ERASE
	case 41://SD_SEND_OP_COND
	case 55://APP_CMD
		dwcmshc_emmc_send_setup_command(cmd);
		break;

	case 6://EMMC is only MMC_RESPONSE_R1B,SD is only MMC_RESPONSE_R1
		if(MMC_RESPONSE_R1B == cmd->resp_type || (MMC_RESPONSE_R1 == cmd->resp_type && 2 == cmd->cmd_arg)) {//For EMMC switch mode and for SD SET_BUS_WIDTH 4
			dwcmshc_emmc_send_setup_command(cmd);
		}else{//For SD card SWITCH_FUNC
			multi_blk_sel = 0;
			data_xfer_dir = 1;//0-write,1-read
			dwcmshc_emmc_send_data_command(cmd, multi_blk_sel, data_xfer_dir);
		}
		break;
	//data command
	case 8:
		if(CMD8_CHECK_PATTERN == (CMD8_CHECK_PATTERN & cmd->cmd_arg)) {//SD card - SEND_IF_COND
			dwcmshc_emmc_send_setup_command(cmd);
		} else {//eMMC - SEND_EXT_CSD
			multi_blk_sel = 0;
			data_xfer_dir = 1;//0-write,1-read
			dwcmshc_emmc_send_data_command(cmd, multi_blk_sel, data_xfer_dir);
		}
		break;
	case 17://READ_BLOCK
	case 51://SEND_SCR
		multi_blk_sel = 0;
		data_xfer_dir = 1;//0-write,1-read
		dwcmshc_emmc_send_data_command(cmd, multi_blk_sel, data_xfer_dir);
		break;
	case 18://READ_MULTIPLE_BLOCK
		multi_blk_sel = 1;
		data_xfer_dir = 1;//0-write,1-read
		dwcmshc_emmc_send_data_command(cmd, multi_blk_sel, data_xfer_dir);
		break;
	case 24://WRITE_BLOCK
		multi_blk_sel = 0;
		data_xfer_dir = 0;//0-write,1-read
		dwcmshc_emmc_send_data_command(cmd, multi_blk_sel, data_xfer_dir);
		break;
	case 25://WRITE_MULTIPLE_BLOCK
		multi_blk_sel = 1;
		data_xfer_dir = 0;//0-write,1-read
		dwcmshc_emmc_send_data_command(cmd, multi_blk_sel, data_xfer_dir);
		break;
	default:
		ERR("%s, The command is not supported and needs to be improved\n", __func__);
		break;
	}

	count = 200;
	while (!mmc_resp.CommandComplete) {
		count--;
		sd_delay(5000);
		if (!count)
			break;
	}

	if (!count) {
		ERR("%s, wait command %d complete time out!\n", __func__, cmd->cmd_idx);
		ERR("last success command is %d \n",last_cmd);
		return -1;
	} else {
		last_cmd = cmd->cmd_idx;
	}

	mmc_resp.CommandComplete = 0;

	switch (cmd->resp_type) {
	case MMC_RESPONSE_R1:
	case MMC_RESPONSE_R1B:
		if (cmd->cmd_idx == 13) {//SEND_STATUS
			cmd->resp_data[0] = mmc_resp.pBuffer[0];
#ifdef MMC_DEBUG
			NOTICE("resp_data[0]: 0x%x\n", cmd->resp_data[0]);
#endif
		} else {//MMC_RESPONSE_R7, the macro MMC_RESPONSE_R7 value is same as MMC_RESPONSE_R1,
				//so, case MMC_RESPONSE_R7
				memcpy((void *)cmd->resp_data, (void *)mmc_resp.pBuffer, sizeof(mmc_resp.pBuffer));
		}

		break;
	case MMC_RESPONSE_R2: //CSD & CID
		if (cmd->cmd_idx == 9 || cmd->cmd_idx == 2)
		{
			// Copy the CSD values from the buffer
			for (i=0; i<4; i++) {
				cmd->resp_data[i] = mmc_resp.pBuffer[i];
#ifdef MMC_DEBUG
				NOTICE("resp_data[%d]: 0x%x\n", i, cmd->resp_data[i]);
#endif
			}
		}
#ifdef MMC_DEBUG
		else
			NOTICE("no need resp_data\n");
#endif
 		break;
	case MMC_RESPONSE_R3: //OCR
		cmd->resp_data[0] = mmc_resp.pBuffer[0];
#ifdef MMC_DEBUG
		NOTICE("resp_data[0]: 0x%x\n", cmd->resp_data[0]);
#endif
		break;
	default:
#ifdef MMC_DEBUG
		NOTICE("no need resp_data\n");
#endif
		break;
	}

	return 0;
}

static int dwcmshc_set_ios(unsigned int clk, unsigned int width)
{
	unsigned int val;

	val = mmc_readl(HOST_CTRL1_R);
	switch (width) {
	case MMC_BUS_WIDTH_1:
		val &= ~EXT_DAT_XFER;
		val &= ~DAT_XFER_WIDTH;
		mmc_writel(val, HOST_CTRL1_R);
		break;
	case MMC_BUS_WIDTH_4:
	case MMC_BUS_WIDTH_DDR_4:
		val &= ~EXT_DAT_XFER;
		val |= DAT_XFER_WIDTH;
		mmc_writel(val, HOST_CTRL1_R);
		break;
	case MMC_BUS_WIDTH_8:
	case MMC_BUS_WIDTH_DDR_8:
		val |= EXT_DAT_XFER;
		mmc_writel(val, HOST_CTRL1_R);
		break;
	default:
		assert(0);
		break;
	}

	switch (clk)
	{
		case SDCLK_SEL_DIV_1024:
		case SDCLK_SEL_DIV_512:
		case SDCLK_SEL_DIV_256:
		case SDCLK_SEL_DIV_128:
		case SDCLK_SEL_DIV_64:
		case SDCLK_SEL_DIV_32:
		case SDCLK_SEL_DIV_16:
		case SDCLK_SEL_DIV_8:
		case SDCLK_SEL_DIV_4:
		case SDCLK_SEL_DIV_2:
		case SDCLK_SEL_DIV_1:
			dwcmshc_host_clock_setup(clk);
		break;

		default:
			dwcmshc_host_clock_setup(MMC_SRC_CLK_RATE_IN_MHZ / (2 * clk));
		break;

	}
#ifdef MMC_DEBUG
	emmc_print_regs(0, 0xfc);
	lgpl_printf("\n");
	emmc_print_regs(0x300, 0x320);
	lgpl_printf("\n");
	emmc_print_regs(0x500, 0x540);
#endif

	return 0;
}

static int dwcmshc_prepare(int lba, uintptr_t buf, size_t size)
{
	unsigned int blk_count, val;

	if(buf % 64 != 0) {
		ERR("WARNING: %s: target buffer (0x%08x) is not 64 byte alignment!\n", __func__, buf);
		//return 0;
	}

	blk_count = CALC_ALIGN(size, MMC_BLOCK_SIZE) / MMC_BLOCK_SIZE;

	/* set xfr_blksz & blk_cnt for current transfer */
	val = mmc_readw(BLOCKSIZE_R);
	val &= ~XFER_BLOCK_SIZE;
	if(size < MMC_BLOCK_SIZE) {
		val |= size;
	} else {
		val |= (MMC_BLOCK_SIZE << 0);
	}

	mmc_writew(val, BLOCKSIZE_R);

	val = mmc_readw(BLOCKCOUNT_R);
	val &= ~BLOCK_CNT;
	val |= (blk_count << 0);
	mmc_writew(val, BLOCKCOUNT_R);

	/* set sdma*/
	val = mmc_readw(BLOCKSIZE_R);
	val &= ~SDMA_BUF_BDARY;
	val |= (SDMA_512k_BDRY << 12);
	mmc_writew(val, BLOCKSIZE_R);

	mmc_writel(buf, SDMASA_R);

	/*select sdma*/
	val = mmc_readl(HOST_CTRL1_R);
	val &= ~DMA_SEL;
	val |= (SDMA_SELECT << 3);
	mmc_writel(val, HOST_CTRL1_R);

	return 0;
}

static int dwcmshc_read(int lba, uintptr_t buf, size_t size)
{
	return 0;
}

static int dwcmshc_write(int lba, uintptr_t buf, size_t size)
{
	return 0;
}

static int dwcmshc_emmcread_extcsd(unsigned char* buffer)
{
	int ret;

#ifdef CONFIG_DCACHE
	size_t size = MMC_BLOCK_SIZE;
	invalidate_dcache_range((void*)(uintptr_t)buffer, (void*)(buffer + size));
#endif
	ret = mmc_read_extcsd(0, (uintptr_t)buffer);
#ifdef CONFIG_DCACHE
	invalidate_dcache_range((void*)(uintptr_t)buffer, (void*)(buffer + size));
#endif

#ifdef MMC_DEBUG
	unsigned int i;
	dbg_printf(MMC_PRN_DEBUG_LEVEL,"@@@@@dump ext_csd@@@@@\n");
	for(i=0;i<128;i++)
	{
		dbg_printf(MMC_PRN_DEBUG_LEVEL,"Bytes %03d ~ %03d: ", i*4, i*4 + 3);
		dbg_printf(MMC_PRN_DEBUG_LEVEL,"0x%02x 0x%02x 0x%02x 0x%02x\n",
			buffer[i*4],
			buffer[i*4+1],
			buffer[i*4+2],
			buffer[i*4+3]);
	}
#endif

	return ret;
}

int do_emmcinit(int dev)
{
	dwcmshc_mmc_params_t params, *ptr_params;
	struct mmc_device_info info, *ptr_info;
	ptr_params = &params;
	ptr_info = &info;
	int ret;

	if (dev == 1) {
		ptr_params->reg_base = MEMMAP_SDIO3_REG_BASE;
		ptr_params->clk_rate = 50; // 50MHz: high speed
		ptr_params->bus_width = MMC_BUS_WIDTH_4;
		ptr_info->mmc_dev_type = MMC_IS_SD;
		ptr_info->ocr_voltage = OCR_2_9_3_0 | OCR_3_0_3_1| OCR_3_2_3_3 | OCR_3_3_3_4;

		sdio_setup();
	} else {
		ptr_params->reg_base = MEMMAP_EMMC_REG_BASE;
		ptr_params->clk_rate = 50; // 50MHz: high speed
		ptr_params->bus_width = EMMC_BUS_WIDTH;
		ptr_params->flags = MMC_FLAG_CMD23;
		ptr_info->mmc_dev_type = MMC_IS_EMMC;
	}

	memcpy(&dwcmshc_params, ptr_params, sizeof(dwcmshc_mmc_params_t));
	dwcmshc_params.mmc_dev_type = ptr_info->mmc_dev_type;
	ret = mmc_init(&dwcmshc_mmc_ops, ptr_params->clk_rate, ptr_params->bus_width,
		ptr_params->flags, ptr_info);

#if defined(CONFIG_DWCMSHC_PHY_SUPPORT)
	if (EMMC_TX_INIT_DELAY_CODE != 0 && MMC_IS_EMMC == ptr_info->mmc_dev_type)
		phy_delayline_setup(EMMC_TX_INIT_DELAY_CODE, 0);
#endif

#ifdef MMC_DEBUG
	unsigned char __attribute__((aligned(64))) ext_csd[512];
	if(dwcmshc_emmcread_extcsd(ext_csd) != 0) {
		ERR("get EXTCSD Error !\n");
		return 0;
	}
#endif

	return ret;
}

static int emmc_check_cross_dma_boundary(uintptr_t buf_addr, size_t buf_size)
{
	if((uintptr_t)buf_addr % EMMC_DMA_BOUNDARY + buf_size > EMMC_DMA_BOUNDARY)
		return 1;
	else return 0;
}

int do_emmcread(unsigned long long start, unsigned int blks, unsigned char * buffer)
{
	int lba_start = (int)(start / MMC_BLOCK_SIZE);
	int cur, offset = 0;
	int ret;
	uintptr_t cur_buf;

	dbg_printf(MMC_PRN_DEBUG_LEVEL,"\n%s, mmc read %x %x %x \n", __func__, buffer, lba_start, blks);
#ifdef CONFIG_DCACHE
	size_t size = blks * MMC_BLOCK_SIZE;
	if (((uintptr_t)buffer) % 64 != 0)
		dbg_printf(PRN_ERR, "warning, %s: target buffer (0x%08x) is not 64 byte alignment!!!\n", __func__, buffer);
	invalidate_dcache_range((void*)(uintptr_t)buffer, (void*)(buffer + size));
#endif

	do {
		/*
		 * The 65535 constraint comes from some hardware has
		 * only 16 bit width block number counter
		 */
		cur = (blks > 65535) ? 65535 : blks;
		cur_buf = (uintptr_t)buffer + offset * MMC_BLOCK_SIZE;
		if(emmc_check_cross_dma_boundary(cur_buf, cur * MMC_BLOCK_SIZE)) {
			cur = (EMMC_DMA_BOUNDARY - ((uintptr_t)cur_buf % EMMC_DMA_BOUNDARY)) / MMC_BLOCK_SIZE;
		}

		ret =  mmc_read_blocks(lba_start + offset, (uintptr_t)buffer + offset * MMC_BLOCK_SIZE, cur * MMC_BLOCK_SIZE);
		if (ret != 0)
		{
			dbg_printf(PRN_ERR,"EMMC: read emmc image fail.\n");
			return ret;
		}

		blks -= cur;
		offset += cur;
	} while (blks > 0);

#ifdef CONFIG_DCACHE
	invalidate_dcache_range((void*)(uintptr_t)buffer, (void*)(buffer + size));
#endif
	return ret;
}

int do_emmcwrite(unsigned long long start, unsigned int blks, unsigned char * buffer)
{
	int lba = (int)(start / MMC_BLOCK_SIZE);
	size_t size = blks * MMC_BLOCK_SIZE;
	int ret;

	dbg_printf(MMC_PRN_DEBUG_LEVEL,"\n%s, mmc write %x %x %x \n", __func__, buffer, lba, blks);
#ifdef CONFIG_DCACHE
	flush_dcache_range((void*)(uintptr_t)buffer, (void*)(buffer + size));
#endif
	ret = mmc_write_blocks(lba, (uintptr_t)buffer, size);
	return ret;
}

int do_emmcerase(unsigned long long start, unsigned long long size)
{
	int lba = (int)(start / MMC_BLOCK_SIZE);

	dbg_printf(MMC_PRN_DEBUG_LEVEL,"\n%s, mmc erase %x %x\n", __func__, lba, size);
	return mmc_erase_blocks(lba, (size_t)size);
}

//0-user area, 1-boot1, 2-boot2
int do_emmc_switch_part(unsigned int PartNum)
{
	static unsigned int power_on=0;

	if(current_part != PartNum)
		current_part = PartNum;
	else {
		dbg_printf(MMC_PRN_DEBUG_LEVEL,"EMMC: NO need switch partition.\n");
		return 0; // if we have switched to this part, just return
	}

	if(power_on == 0){
		if(mmc_set_ext_csd(CMD_ERASE_GROUP_DEF, SET_BITS, 1)){
			ERR("EMMC: Clear out Partition Access bits error.\n");
			return FLASH_OP_ERR;
		}
		power_on=1;
	}

	if(mmc_set_ext_csd(CMD_EXTCSD_PARTITION_CONFIG, CLR_BITS, PARTITION_ACCESS_BITS)){
		ERR("EMMC: Clear out Partition Access bits error.\n");
		return FLASH_OP_ERR;
	}

	if(mmc_set_ext_csd(CMD_EXTCSD_PARTITION_CONFIG, SET_BITS, (PartNum & 0x3))){
		ERR("EMMC: switch to boot partion %d error.\n", PartNum);
		return FLASH_OP_ERR;
	}

	return 0;
}

int do_emmc_get_current_part(void)
{
	return current_part;
}

long long do_emmc_capacity(void)
{
	unsigned char __attribute__((aligned(64))) ext_csd[512];
	unsigned char *buffer = ext_csd;
	unsigned long long sec_count;

	if(dwcmshc_emmcread_extcsd(ext_csd) != 0) {
		ERR("get EXTCSD Error !\n");
		return 0;
	}
	sec_count = ((unsigned int *)buffer)[CMD_EXTCSD_SEC_CNT/4];

	return (sec_count * 512);
}

void do_emmc_stop_alt_boot_mode(void)
{
	/* reset the Device to idle state from alternative boot */
	int ret;

	ret = mmc_reset_to_idle();
	if (ret != 0) {
		ERR("%s, reset to idle from alternative boot error!\n", __func__);
	}
}

unsigned long do_emmc_rpmbCapacity(void)
{
	unsigned char __attribute__((aligned(64))) ext_csd[512];

	if(dwcmshc_emmcread_extcsd(ext_csd) != 0) {
		ERR("get EXTCSD Error !\n");
		return 0;
	}

	if(ext_csd[CMD_RPMB_SIZE_MULT] > 0x80) {
		ERR("Invalid rpmb partition Size index !\n");
		return 0;
	}

	dbg_printf(MMC_PRN_DEBUG_LEVEL,"RPMB Partition Size: 0x%x Bytes\n", ext_csd[CMD_RPMB_SIZE_MULT] * 0x20000);
	return (ext_csd[CMD_RPMB_SIZE_MULT]);
}

