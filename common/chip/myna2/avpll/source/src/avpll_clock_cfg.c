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
#include "Galois_memmap.h"
#include "galois_io.h"
#include "global.h"
#include "avpll.h"
#include "pllDiag.h"
#include "avpll_debug.h"

#if (BERLIN_CHIP_VERSION >= BERLIN_BG2_Q)
#include "avioGbl.h"
#endif

#if (BERLIN_CHIP_VERSION == BERLIN_BG4_CDP_A0)
#define  AVPLL2_CLK_FREQ  600000000
#define  AVPLL5_CLK_FREQ  650000000
#define  AVPLL6_CLK_FREQ  600000000
#define  SYPLL_CLK_FREQ   800000000

static AVPLL_CLOCK_CONFIG clk_config[] = {
 {RA_Gbl_vppSysClk,     600000000, 0, PLL_CLK_AVPLL2, AVPLL2_CLK_FREQ, PLL_CLK_D1, 0},
 {RA_Gbl_gfx3DSysClk,   650000000, 0, PLL_CLK_AVPLL5, AVPLL5_CLK_FREQ, PLL_CLK_D1, 0},
 {RA_Gbl_v4gClk,        650000000, 0, PLL_CLK_AVPLL5, AVPLL5_CLK_FREQ, PLL_CLK_D1, 0},
 {RA_Gbl_v4gM3Clk,      600000000, 0, PLL_CLK_AVPLL6, AVPLL6_CLK_FREQ, PLL_CLK_D1, 0},
 {RA_Gbl_avioSysClk,    600000000, 0, PLL_CLK_AVPLL6, AVPLL6_CLK_FREQ, PLL_CLK_D1, 0},
 {RA_Gbl_avioBiuClk,    200000000, 0, PLL_CLK_AVPLL6, AVPLL6_CLK_FREQ, PLL_CLK_D3, 0},
 {RA_Gbl_vipPipeClk,    600000000, 0, PLL_CLK_AVPLL6, AVPLL6_CLK_FREQ, PLL_CLK_D1, 0}
};

static AVPLL_CLOCK_CONFIG clk_config_default[] = {
 {RA_Gbl_vppSysClk,     600000000, 0, PLL_CLK_AVPLL2, AVPLL2_CLK_FREQ, PLL_CLK_D1, 0},
 {RA_Gbl_gfx3DSysClk,   650000000, 0, PLL_CLK_AVPLL5, AVPLL5_CLK_FREQ, PLL_CLK_D1, 0},
 {RA_Gbl_v4gClk,        650000000, 0, PLL_CLK_AVPLL5, AVPLL5_CLK_FREQ, PLL_CLK_D1, 0},
 {RA_Gbl_v4gM3Clk,      600000000, 0, PLL_CLK_AVPLL6, AVPLL6_CLK_FREQ, PLL_CLK_D1, 0},
 {RA_Gbl_avioSysClk,    600000000, 0, PLL_CLK_AVPLL6, AVPLL6_CLK_FREQ, PLL_CLK_D1, 0},
 {RA_Gbl_avioBiuClk,    200000000, 0, PLL_CLK_AVPLL6, AVPLL6_CLK_FREQ, PLL_CLK_D3, 0},
 {RA_Gbl_vipPipeClk,    600000000, 0, PLL_CLK_AVPLL6, AVPLL6_CLK_FREQ, PLL_CLK_D1, 0}
};

static AVPLL_CLOCK_CONFIG clk_config_div1[] = {
 {RA_Gbl_vppSysClk,     600000000, 0, PLL_CLK_AVPLL2, AVPLL2_CLK_FREQ, PLL_CLK_D2, 0},
 {RA_Gbl_gfx3DSysClk,   650000000, 0, PLL_CLK_AVPLL5, AVPLL5_CLK_FREQ, PLL_CLK_D1, 0},
 {RA_Gbl_v4gClk,        650000000, 0, PLL_CLK_AVPLL5, AVPLL5_CLK_FREQ, PLL_CLK_D1, 0},
 {RA_Gbl_v4gM3Clk,      600000000, 0, PLL_CLK_AVPLL6, AVPLL6_CLK_FREQ, PLL_CLK_D1, 0},
 {RA_Gbl_avioSysClk,    600000000, 0, PLL_CLK_AVPLL6, AVPLL6_CLK_FREQ, PLL_CLK_D1, 0},
 {RA_Gbl_avioBiuClk,    200000000, 0, PLL_CLK_AVPLL6, AVPLL6_CLK_FREQ, PLL_CLK_D3, 0},
 {RA_Gbl_vipPipeClk,    600000000, 0, PLL_CLK_AVPLL6, AVPLL6_CLK_FREQ, PLL_CLK_D1, 0}
};

#else
static AVPLL_CLOCK_CONFIG clk_config[] = {};
#endif

static pthread_mutex_t lock_avpll_clock_switch = PTHREAD_MUTEX_INITIALIZER;
void avpll_clock_init(void)
{
    int index, target_clks;
    unsigned int addr;

    target_clks = sizeof(clk_config)/sizeof(AVPLL_CLOCK_CONFIG);

    //save default clock settings for each target
    for(index = 0; index < target_clks; index++)
    {
        addr = (MEMMAP_CHIP_CTRL_REG_BASE + clk_config[index].target_select);
        BFM_HOST_Bus_Read32(addr,&clk_config[index].target_default_sel);
    }
    return;
}

#if defined(AVPLL6_CLK_FREQ) || defined(AVPLL5_CLK_FREQ) || defined(AVPLL2_CLK_FREQ)
static unsigned int avpll_getClkFreq(unsigned int chanNdx) {
    int index, target_clks;
    unsigned int srcClk = 0;

    target_clks = sizeof(clk_config)/sizeof(AVPLL_CLOCK_CONFIG);
    for(index = 0; index < target_clks; index++) {
        //Update the entries related to the requested avpll channel
        if( clk_config[index].pll_select == chanNdx ) {
            srcClk = clk_config[index].source_clk_freq;
            break;
        }
    }
    return srcClk;
}
#endif

#if (BERLIN_CHIP_VERSION == BERLIN_BG4_CDP_A0)
static void avpll_chanClk_SetChanFreq(unsigned int chanNdx, unsigned int chanFreq, AVPLL_CLOCK_CONFIG *clkCfg) {
    int index, target_clks;

    target_clks = sizeof(clk_config)/sizeof(AVPLL_CLOCK_CONFIG);

    for(index = 0; index < target_clks; index++, clkCfg++) {
        //Update the entries related to the requested avpll channel
        if( clkCfg->pll_select == chanNdx ) {
            printf("AVPLL_DBG:ChangeFreq: TargetClk:%d chanNdx:%u, Freq:%d->%u\n",index, chanNdx, clkCfg->source_clk_freq, chanFreq);
            clkCfg->source_clk_freq = chanFreq;
            clkCfg->target_clk_freq = chanFreq;
        }
    }
}

static unsigned int avpll_chanClk_getChanNdx(unsigned int userChanNum, unsigned int *pChanNdx) {
    unsigned int retVal = 1;
    switch(userChanNum) {
        case 2: *pChanNdx = PLL_CLK_AVPLL2; break;
        case 5: *pChanNdx = PLL_CLK_AVPLL5; break;
        case 6: *pChanNdx = PLL_CLK_AVPLL6; break;
        case 7: *pChanNdx = PLL_CLK_AVPLL7; break;
        default: retVal = 0; break;
    }

    return retVal;
}
#endif

static void avpll_clock_syspllswitch(void)
{
    int index, target_clks;
    unsigned int addr;

    target_clks = sizeof(clk_config)/sizeof(AVPLL_CLOCK_CONFIG);

    //save default clock settings for each target
    for(index = 0; index < target_clks; index++)
    {
        addr = (MEMMAP_CHIP_CTRL_REG_BASE + clk_config[index].target_select);
        if(clk_config[index].pll_select != PLL_CLK_SYSPLL) //Do not change SYSPLL settings
            BFM_HOST_Bus_Write32(addr,clk_config[index].target_default_sel);
    }
    return;
}

static void avpll_clock_avpllswitch(PLL_CLK_CFG_OPT_FLAG opt_flag)
{
    TclkD1_ctrl     clkDx;
    int index, target_clks;
    unsigned int addr;

    target_clks = sizeof(clk_config)/sizeof(AVPLL_CLOCK_CONFIG);

    //save default clock settings for each target
    for(index = 0; index < target_clks; index++)
    {
        //During optimized operation, If entry is not modified then no need to configure it
        if( (PLL_CLK_CFG_OPT_FLAG_ENABLED == opt_flag) && (!clk_config[index].clk_cfg_flags) ) {
            continue;
        }

        addr = MEMMAP_CHIP_CTRL_REG_BASE + clk_config[index].target_select;
        BFM_HOST_Bus_Read32(addr, &clkDx.u32[0]);
        if(clk_config[index].divider == PLL_CLK_D3)
        {
            clkDx.uctrl_ClkD3Switch = 1;
        }
        else if(clk_config[index].divider == PLL_CLK_D1)
        {
            clkDx.uctrl_ClkSwitch = 0;
            clkDx.uctrl_ClkD3Switch = 0;
        }
        else
        {
            clkDx.uctrl_ClkD3Switch = 0;
            clkDx.uctrl_ClkSwitch = 1;
            clkDx.uctrl_ClkSel = clk_config[index].divider;
        }
        clkDx.uctrl_ClkEn = 1;//enable clock
        BFM_HOST_Bus_Write32(addr, clkDx.u32[0]);
        // change Pll Select
        clkDx.uctrl_ClkPllSel = clk_config[index].pll_select;
        BFM_HOST_Bus_Write32(addr, clkDx.u32[0]);

        // change Pll Switch
        if(clk_config[index].pll_select == PLL_CLK_SYSPLL)
            clkDx.uctrl_ClkPllSwitch = 0;
        else
            clkDx.uctrl_ClkPllSwitch = 1;
        BFM_HOST_Bus_Write32(addr, clkDx.u32[0]);
    }
    return;
}

void avpll_clock_switch(PLL_SWITCH_TYPE toavpll, PLL_CLK_CFG_OPT_FLAG opt_flag)
{
    pthread_mutex_lock(&lock_avpll_clock_switch);
    if(toavpll)
        avpll_clock_avpllswitch(opt_flag);
    else
        avpll_clock_syspllswitch();
    pthread_mutex_unlock(&lock_avpll_clock_switch);
    return;
}

void avpll_clock_reconfig(void)
{
   //reconfig AVPLL channel clocks on change in VCO
#ifdef AVPLL6_CLK_FREQ
    AVPLL_Set(0, 6, avpll_getClkFreq(PLL_CLK_AVPLL6));
#endif

#ifdef AVPLL5_CLK_FREQ
    AVPLL_Set(0, 5, avpll_getClkFreq(PLL_CLK_AVPLL5));
#endif

#ifdef AVPLL2_CLK_FREQ
    AVPLL_Set(0, 2, avpll_getClkFreq(PLL_CLK_AVPLL2));
#endif
    avpll_clock_switch(PLL_SWITCH_TYPE_AVPLL, PLL_CLK_CFG_OPT_FLAG_DISABLED); //switch clk source to avpll

    //AVPLL_DumpAll();

    return;
}

#if (BERLIN_CHIP_VERSION == BERLIN_BG4_CDP_A0)
static int avpll_get_clock_configuration_flags(const AVPLL_CLOCK_CONFIG *clkCfg1, const AVPLL_CLOCK_CONFIG *clkCfg2) {
    int modificationFlag = 0;

    //This field is not used : PLL_CLK_CFG_F_TGT_DSEL,

    if((clkCfg1->target_select     != clkCfg2->target_select)) {
        modificationFlag |= (1<<PLL_CLK_CFG_F_TGT_SEL);
    }
    if((clkCfg1->target_clk_freq   != clkCfg2->target_clk_freq)) {
        modificationFlag |= (1<<PLL_CLK_CFG_F_TGT_FREQ);
    }
    if((clkCfg1->pll_select        != clkCfg2->pll_select)) {
        modificationFlag |= (1<<PLL_CLK_CFG_F_PLL_SEL);
    }
    if((clkCfg1->source_clk_freq   != clkCfg2->source_clk_freq)) {
        modificationFlag |= (1<<PLL_CLK_CFG_F_PLL_SRC_FREQ);
    }
    if((clkCfg1->divider           != clkCfg2->divider)) {
        modificationFlag |= (1<<PLL_CLK_CFG_F_PLL_DIV);
    }

    return modificationFlag;
}

void avpll_set_clock_configure(PLL_CLK_CONFIG_TYPE clkConfigType) {
    int index, target_clks;

    target_clks = sizeof(clk_config)/sizeof(AVPLL_CLOCK_CONFIG);

    //save default/div1 clock settings for each target
    for(index = 0; index < target_clks; index++)
    {
        const AVPLL_CLOCK_CONFIG *tmpClkCfg;
        if(PLL_CLK_CONFIG_TYPE_DEFAULT == clkConfigType) {
            tmpClkCfg = &clk_config_default[index];
        } else {
            tmpClkCfg = &clk_config_div1[index];
        }

        clk_config[index].clk_cfg_flags     = avpll_get_clock_configuration_flags(&clk_config[index], tmpClkCfg);
        clk_config[index].target_select     = tmpClkCfg->target_select;
        clk_config[index].target_clk_freq   = tmpClkCfg->target_clk_freq;
        clk_config[index].pll_select        = tmpClkCfg->pll_select;
        clk_config[index].source_clk_freq   = tmpClkCfg->source_clk_freq;
        clk_config[index].divider           = tmpClkCfg->divider;
    }
}

void avpll_set_channel_configure(AVPLL_CHANNEL_CLK_CONFIG *pChanConfig, int nConfig) {
    int index;
    unsigned int chanNdx;

    for(index = 0; index < nConfig; index++) {
        if(avpll_chanClk_getChanNdx(pChanConfig[index].avpllChannel, &chanNdx)) {
            printf("AVPLL_DBG:ChangeFreq: userConfigNdx:%d chanNdx:%u, Freq:%d\n",index, chanNdx, pChanConfig[index].clkFreq);
            //Update the channel configuration in all the clk configuration structures
            avpll_chanClk_SetChanFreq(chanNdx, pChanConfig[index].clkFreq, clk_config_default);
            avpll_chanClk_SetChanFreq(chanNdx, pChanConfig[index].clkFreq, clk_config_div1);
            avpll_chanClk_SetChanFreq(chanNdx, pChanConfig[index].clkFreq, clk_config);
        }
    }
}
#endif
