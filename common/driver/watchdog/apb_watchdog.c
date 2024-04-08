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
#include "com_type.h"

#include "io.h"

#if (BERLIN_CHIP_VERSION < BERLIN_BG4CD)
#include "apb_perf_base.h"
#else
#include "apbRegBase.h"
#endif

#ifdef CONFIG_SM
#include "SysMgr.h"
#include "system_manager.h"
#endif
#include "debug.h"
#include "apb_watchdog.h"

#define SYSTEM_RESET_MODE	0
#define PCLK_8				2

#define WDT_CR			(0x00)
#define WDT_TORR		(0x04)
#define WDT_CRR			(0x0C)
#ifdef CONFIG_SM
#define SM_WDT2_CR			(SM_APB_WDT2_BASE + WDT_CR)
#define SM_WDT2_TORR		(SM_APB_WDT2_BASE + WDT_TORR)
#define SM_WDT2_CRR			(SM_APB_WDT2_BASE + 0x0C)


static void mv_sm_wdt2_set_rspmode(int mode)
{
	UINT32 reg = readl(SM_WDT2_CR);
	if(mode)
		reg |= (1 << 1);
	else
		reg &= ~(1 << 1);
	REG_WRITE32(SM_WDT2_CR, reg);
}

static void mv_sm_wdt2_set_rpl(unsigned int rpl)
{
	UINT32 reg = readl(SM_WDT2_CR);
	reg &= ~(0x7 << 2);
	reg |= (rpl << 2);
	REG_WRITE32(SM_WDT2_CR, reg);
}

static void mv_sm_wdt2_settimeout(INT32 settings)
{
	if((settings < 0) || (settings > 15)) {
		dbg_printf(PRN_RES, "wrong timeout parameter\n");
		return;
	}

	REG_WRITE32(SM_WDT2_TORR, settings);
}

static void apb_sm_watchdog2_init(void)
{
	unsigned int wdt_mask;

	wdt_mask = readl(SOC_SM_SYS_CTRL_REG_BASE + RA_smSysCtl_SM_WDT_MASK);
	//clear bit2 for wdt2 sm reset and and bit 5 for wdt2 soc reset
	wdt_mask &= ~(1 << 2);
	wdt_mask &= ~(1 << 5);
	REG_WRITE32(SOC_SM_SYS_CTRL_REG_BASE + RA_smSysCtl_SM_WDT_MASK, wdt_mask);

	//interrupt mode, and reset signal pulse repeat 8 pclk cycles
	mv_sm_wdt2_set_rspmode(SYSTEM_RESET_MODE);
	mv_sm_wdt2_set_rpl(PCLK_8);
	mv_sm_wdt2_settimeout(0); // 2-3ms
}

static void mv_wdt2_enable()
{
	UINT32 reg = readl(SM_WDT2_CR);

	REG_WRITE32(SM_WDT2_CR, reg | 0x01);
}

static void mv_wdt2_restart()
{
	// 0x76 is the value that wdt spec recommended
	REG_WRITE32(SM_WDT2_CRR, 0x76);
}
#endif

#if defined(BERLIN_SOC_MYNA2)
#define RA_Gbl_WDTSysRstMask			0x0610
#define CONFIG_DW_WDT_BASE			0xF7E83400
#define SOC_CHIP_CTRL_REG_BASE			0xF7EA0000

#define CONFIG_DW_WDT_CLOCK_KHZ			25000

#define DW_WDT_CR	0x00
#define DW_WDT_TORR	0x04
#define DW_WDT_CRR	0x0C

#define DW_WDT_CR_EN_OFFSET	0x00
#define DW_WDT_CR_RMOD_OFFSET	0x01
#define DW_WDT_CR_RMOD_VAL	0
#define DW_WDT_CRR_RESTART_VAL	0x76
#define DW_WDT_CR_RPL_OFFSET	0x02
#define DW_WDT_CR_RPL_MASK	0x07
#define DW_WDT_CR_RPL_PCLK_8	0x02

static inline unsigned int log_2_n_round_up(unsigned int n)
{
        int log2n = -1;
        unsigned int temp = n;

        while (temp) {
                log2n++;
                temp >>= 1;
        }

        if (n & (n - 1))
                return log2n + 1; /* not power of 2 - round up */
        else
                return log2n; /* power of 2 */
}

static void berlin_dw_wdt_set_rspmode(int mode)
{
	unsigned int reg = readl(CONFIG_DW_WDT_BASE + DW_WDT_CR);
	if (mode)
		reg |= (1 << DW_WDT_CR_RMOD_OFFSET);
	else
		reg &= ~(1 << DW_WDT_CR_RMOD_OFFSET);
	writel(reg, CONFIG_DW_WDT_BASE + DW_WDT_CR);
}

static void berlin_dw_wdt_set_rpl(unsigned int rpl)
{
	unsigned int reg = readl(CONFIG_DW_WDT_BASE + DW_WDT_CR);
	reg &= ~(DW_WDT_CR_RPL_MASK << DW_WDT_CR_RPL_OFFSET);
	reg |= (rpl << DW_WDT_CR_RPL_OFFSET);
	writel(reg, CONFIG_DW_WDT_BASE + DW_WDT_CR);
}

/*
 * Set the watchdog time interval.
 * Counter is 32 bit.
 */
static int berlin_dw_wdt_settimeout(unsigned int timeout)
{
	signed int i;

	/* calculate the timeout range value */
	i = (log_2_n_round_up(timeout * CONFIG_DW_WDT_CLOCK_KHZ)) - 16;
	if (i > 15)
		i = 15;
	if (i < 0)
		i = 0;

	writel((i | (i << 4)), (CONFIG_DW_WDT_BASE + DW_WDT_TORR));
	return 0;
}

static void berlin_dw_wdt_init(void)
{
	unsigned int wdt_mask;

	wdt_mask = readl(SOC_CHIP_CTRL_REG_BASE + RA_Gbl_WDTSysRstMask);

	//Clear bit0 for wdt0 mask, bit1 for wdt1, bit2 for wdt2
	wdt_mask &= ~(1 << 0);
	writel(wdt_mask, SOC_CHIP_CTRL_REG_BASE + RA_Gbl_WDTSysRstMask);

	//interrupt mode, and reset signal pulse repeat 8 pclk cycles
	berlin_dw_wdt_set_rspmode(DW_WDT_CR_RMOD_VAL);
	berlin_dw_wdt_set_rpl(DW_WDT_CR_RPL_PCLK_8);
	berlin_dw_wdt_settimeout(0); // 2-3ms
}

static void berlin_dw_wdt_enable(void)
{
	unsigned int reg = readl(CONFIG_DW_WDT_BASE + DW_WDT_CR);
	reg |= 0x1 << DW_WDT_CR_EN_OFFSET;
	writel(reg, CONFIG_DW_WDT_BASE + DW_WDT_CR);
}

static unsigned int berlin_dw_wdt_is_enabled(void)
{
	unsigned long val;
	val = readl((CONFIG_DW_WDT_BASE + DW_WDT_CR));
	return val & 0x1;
}

void berlin_dw_wdt_reset(void)
{
	if (berlin_dw_wdt_is_enabled())
		/* restart the watchdog counter */
		writel(DW_WDT_CRR_RESTART_VAL,
		       (CONFIG_DW_WDT_BASE + DW_WDT_CRR));
}
#endif

void reset_soc(void)
{
#ifdef CONFIG_SM
	apb_sm_watchdog2_init();
	mv_wdt2_enable();
	mv_wdt2_restart();
#elif defined(BERLIN_SOC_MYNA2)
	berlin_dw_wdt_init();
	berlin_dw_wdt_enable();
	berlin_dw_wdt_reset();
#endif
	while(1);
}

