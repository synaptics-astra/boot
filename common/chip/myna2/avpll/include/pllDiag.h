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
#ifndef __PLLDIAG__H_
#define __PLLDIAG__H_
#include "Galois_memmap.h"
#include "avioGbl.h"
#include "aio_def.h"

#define BERLIN_BG5_CT BERLIN_BG5CT
#define BERLIN_BG4_CD BERLIN_BG4CD
#define BERLIN_BG4_CDP BERLIN_BG4CDP
#define MEMMAP_AVIO_GBL_BASE MEMMAP_AVIO_REG_BASE
#define pthread_mutex_t int
#define PTHREAD_MUTEX_INITIALIZER 0
#define pthread_mutex_lock(mutex)
#define pthread_mutex_unlock(mutex)



#undef DIAG_ENV

//#define DIAG_PRT
#undef DIAG_PRT

#if (BERLIN_CHIP_VERSION >= BERLIN_BG4_CD)
#define AVPLL_A_ADDR ((volatile unsigned int *)devmem_phy_to_virt(MEMMAP_AVIO_REG_BASE + AVIO_MEMMAP_AVIO_GBL_BASE + RA_avioGbl_AVPLLA))
#else
#define AVPLL_A_ADDR ((volatile unsigned int *)devmem_phy_to_virt(MEMMAP_AVIO_GBL_BASE + RA_avioGbl_AVPLLA))
#ifdef CONFIG_HAL_AVPLL_B_ENABLE
#define AVPLL_B_ADDR ((volatile unsigned int *)devmem_phy_to_virt(MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_AVPLLB))
#endif
#endif

extern int diag_pll_A_VCO_Setting;
extern int diag_pll_B_VCO_Setting;
enum
{
	VCO_FREQ_1_512G=0,
	VCO_FREQ_1_620G,
	VCO_FREQ_1_856G,
	VCO_FREQ_2_160G,
	VCO_FREQ_2_227G,
	VCO_FREQ_2_520G,
	VCO_FREQ_2_700G,
	VCO_FREQ_2_970G,
};

enum PRINT_LEVEL
{
	PRN_NONE=0,
	PRN_ERR,
	PRN_RES,
	PRN_INFO,
	PRN_DBG
};

enum
{
	HDMI_8BIT_MODE=0,
	HDMI_10BIT_MODE,
	HDMI_12BIT_MODE
};

enum
{
	FRAME_RATE_59P94=0,
	FRAME_RATE_60
};

typedef enum PLL_CLK_DIV_T
{
    PLL_CLK_D1  = 0,
    PLL_CLK_D2  = 1,
    PLL_CLK_D4  = 2,
    PLL_CLK_D6  = 3,
    PLL_CLK_D8  = 4,
    PLL_CLK_D12 = 5,
    PLL_CLK_D3  = 6
}PLL_CLK_DIV;

typedef enum PLL_CLK_SEL_T
{
    PLL_CLK_AVPLL2 = 0,
    PLL_CLK_AVPLL5 = 1,
    PLL_CLK_AVPLL6 = 2,
    PLL_CLK_AVPLL7 = 3,
    PLL_CLK_AVPLL_MAX = 4,
    PLL_CLK_SYSPLL = PLL_CLK_AVPLL_MAX,
}PLL_CLK_SEL;

typedef enum PLL_SWITCH_TYPE_T
{
    PLL_SWITCH_TYPE_SYSPLL = 0,
    PLL_SWITCH_TYPE_AVPLL,
}PLL_SWITCH_TYPE;

//Flags to indicate optimized/non-optimized clock configuration operation
typedef enum PLL_CLK_CFG_OPT_FLAG_T
{
    PLL_CLK_CFG_OPT_FLAG_DISABLED = 0,
    PLL_CLK_CFG_OPT_FLAG_ENABLED,
}PLL_CLK_CFG_OPT_FLAG;

//Bit position for various flag indicating change in clock configuration
typedef enum PLL_CLK_CFG_FLAGS_T
{
    PLL_CLK_CFG_F_TGT_SEL = 0,
    PLL_CLK_CFG_F_TGT_FREQ,
    PLL_CLK_CFG_F_TGT_DSEL,
    PLL_CLK_CFG_F_PLL_SEL,
    PLL_CLK_CFG_F_PLL_SRC_FREQ,
    PLL_CLK_CFG_F_PLL_DIV,
}PLL_CLK_CFG_FLAGS;

typedef struct AVPLL_CLOCK_CONFIG_T
{
    unsigned int target_select;
    unsigned int target_clk_freq;
    unsigned int target_default_sel;
    PLL_CLK_SEL pll_select;
    int source_clk_freq;
    PLL_CLK_DIV divider;
    //PLL CLK CFG changes (accumulation of above fields)
    int clk_cfg_flags;
}AVPLL_CLOCK_CONFIG;

#ifdef DIAG_PRT
#define dbg_printf(RES, fmt, ...) printf("%d(%s:%d)--" fmt, RES, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define dbg_printf(RES, fmt, ...)
#endif

extern void diag_setChanOffset(SIE_avPll *avPllBase, int offset, int chId);
void diag_set_Post_Div(SIE_avPll *avPllBase, int div, int chId);
void diag_set_Post_0P5_Div(SIE_avPll *avPllBase, int enable, int chId);
extern void diag_set1KPPM(SIE_avPll *avPllBase, int inc, int chId);
extern void diag_changeRefFreq(int vco_freq_index, int grp);
extern void diag_avpllFreq(SIE_avPll *avPllBase, int chId);
extern void diag_vcoFreq(SIE_avPll *avPllBase);
extern void diag_videoFreq(SIE_avPll *avPllBase, int freqIndex, int hdmiMode, int frameRate, float overSampleRate, int chId);
extern int diag_clockFreq(SIE_avPll *avPllBase, int vco_freq_index, unsigned int target_freq, int chId);
extern void diag_assertReset(SIE_avPll *avPllBase);
extern void diag_deassertReset(SIE_avPll *avPllBase);
void diag_powerDown(SIE_avPll *avPllBase);
void diag_powerUp(SIE_avPll *avPllBase);
void diag_calibrate(SIE_avPll *avPllBase, double fvco);
extern void diag_initAvPll(void);

extern int diag_pll_A_VCO_Setting;
extern int diag_pll_B_VCO_Setting;

int diag_powerStatus(SIE_avPll *avPllBase);
void dump_AVPLL_regs(int nRegs);

#define BFM_HOST_Bus_Write32    GA_REG_WORD32_WRITE
#define BFM_HOST_Bus_Read32     GA_REG_WORD32_READ

#endif
