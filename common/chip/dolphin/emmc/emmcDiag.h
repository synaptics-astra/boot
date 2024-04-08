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

#define EMMC_PHY_OFFSET (0x300)
#define EMMC_PHY_CNFG   (0x0)
#define EMMC_PHY_CMDPAD_CNFG   (0x4)
#define EMMC_PHY_DATPAD_CNFG   (0x6)
#define EMMC_PHY_CLKPAD_CNFG   (0x8)
#define EMMC_PHY_STBPAD_CNFG   (0xA)
#define EMMC_PHY_RSTNPAD_CNFG   (0xC)
#define EMMC_PHY_COMMDL_CNFG   (0x1C)
#define EMMC_PHY_SDCLKDL_CNFG   (0x1D)
#define EMMC_PHY_SMPLDL_CNFG   (0x20)
#define EMMC_PHY_DLL_CTRL   (0x24)
#define EMMC_PHY_DLLDL_CNFG   (0x28)
#define EMMC_PHY_DLLLBT_CNFG   (0x2C)

#define EMMC_VENDOR1_OFFSET (0x500)
#define EMMC_CTRL_R	(0x2C)
#define EMMC_BOOT_CTRL_R	(0x2E)
#define EMMC_AT_CTRL_R   (0x40)
#define EMMC_AT_STAT_R   (0x44)

// EXT_CSD
#define ERASE_GROUP_DEF						175
#define BOOT_BUS_WIDTH_MMC_EXT_CSD_OFFSET	177
#define PARTITION_CONFIG_MMC_EXT_CSD_OFFSET	179
#define BUS_WIDTH_MMC_EXT_CSD_OFFSET		183
#define HS_TIMING_MMC_EXT_CSD_OFFSET		185
#define EXT_CSD_ACCESS_CMD_SET				0
#define EXT_CSD_ACCESS_SET_BITS				1
#define EXT_CSD_ACCESS_CLEAR_BITS			2
#define EXT_CSD_ACCESS_WRITE_BYTE			3
#define PARTITION_ACCESS_BITS				0x7


// MMC CMD 6 Related
typedef struct
{
	unsigned int	CmdSet 				:3;
	unsigned int	Reserved0 			:5;
	unsigned int   	Value				:8;
	unsigned int   	Index				:8;
	unsigned int   	Access				:2;
	unsigned int	Reserved1 			:6;
} MMC_CMD6_LAYOUT;

typedef union
{
	UINT_T				MMC_CMD6_Bits;
	MMC_CMD6_LAYOUT		MMC_CMD6_Layout;
} MMC_CMD6_OVERLAY;



void EMMC_ISR();


