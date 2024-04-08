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

//-----------------------------------------------------------------------------
//! \file       avpll_debug.c
//! \brief      The AVPLL channel/frequency dump framework source file
//! \author     Shanmugam Ramachandran
//-----------------------------------------------------------------------------

#include "com_type.h"
#include "Galois_memmap.h"
#include "galois_io.h"
#include "global.h"
#include "avpll.h"
#include "pllDiag.h"

#if (BERLIN_CHIP_VERSION >= BERLIN_BG2_Q)
#include "avioGbl.h"
#endif

#define AVPLL_DEBUG_PRN_PREFIX  "AVPLL_DBG--"
#if 0
#define dbg_printf1(RES, fmt, ...) printf("%d(%s:%d)--" fmt, RES, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define dbg_printf1(RES, fmt, ...) printf(AVPLL_DEBUG_PRN_PREFIX fmt, ##__VA_ARGS__)
#endif
#define dbg_printf2(RES, fmt, ...)

#include "MC6Ctrl.h"
#include "cpu_wrp.h"
extern unsigned int get_divider(unsigned int D3Switch, unsigned int Switch, unsigned int Select);

#define SYSPLL_CTRL_BASE    (MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_sysPll)
#define CPUPLL_CTRL_BASE    (MEMMAP_CA7_REG_BASE + RA_CPU_WRP_PLL_REG)
#define MEMPLL_CTRL_BASE    (MEMMAP_MCTRLSS_REG_BASE + RA_MC6Ctrl_memPll)

#define MAX_AVPLL_REGS  35
#ifdef CONFIG_HAL_AVPLL_B_ENABLE
#define AVPLL_MAX   2
#else
#define AVPLL_MAX   1
#endif
typedef struct _tag_avpll_reg_dump_table {
    UINT32 offset_addr;
    char *addr_name;
} AVPLL_REG_INFO;


void AVPLL_DumpSpeed(void)
{
    TclkD1_ctrl clkDx;
    unsigned int divider;
    unsigned int D3Switch, Switch, Select;
    unsigned int PllSel;
    unsigned int sysPll_diff_p=800;
    unsigned int sysPll=800;        //need to read it from register
    static const char *AVPLL[] =
    {
        "AVPllA2",
        "AVPllA5",
        "AVPllA6",
    };

#if (BERLIN_CHIP_VERSION == BERLIN_BG4_CD || \
        BERLIN_CHIP_VERSION == BERLIN_BG4_CDP || \
        BERLIN_CHIP_VERSION == BERLIN_BG4_CDP_A0)
    enum Clocks
    {
        /*  0 */  CPUFASTREFCLK = 0,
        /*  1 */  MEMFASTREFCLK,
        /*  2 */  CFGCLK,
        /*  3 */  SYSCLK,
        /*  4 */  V4GCLK,
        /*  5 */  GFX3DCORECLK,
        /*  6 */  GFX3DSYSCLK,
        /*  7 */  AVIOSYSCLK,
        /*  8 */  VPPSYSCLK,
        /*  9 */  ARCREFCLK,
#if (BERLIN_CHIP_VERSION != BERLIN_BG4_CDP_A0)
        /*    */  HDMIRXMCLK,
#endif
        /* 10 */  PERIFCLK,
        /* 11 */  TSPCLK,
        /* 12 */  TSPREFCLK,
        /* 13 */  ATBCLK,
        /* 14 */  BCMCLK,
        /* 15 */  EMMCCLK,
        /* 16 */  NFCECCCLK,
        /* 17 */  SD0CLK,
#if (BERLIN_CHIP_VERSION == BERLIN_BG4_CDP_A0)
        /* 18 */  SD1CLK,
#endif
        /* 19 */  USB2TESTCLK,
        /* 20 */  SDIO3DLLMSTREFCLK,
        /* 21 */  EDDCCLK,
        /* 22 */  AVIOBIUCLK,
        /* 23 */  V4GM3CLK,
        /* 24 */  APBCORECLK,
        /* 25 */  ZSPCLK,
        /* 26 */  PCIETESTCLK,
        /* 27 */  VIPPIPECLK,
        /* 28 */  AVIOFPLL400_CLK,
        /* 29 */  IHBPHYOCCCLK,
#if (BERLIN_CHIP_VERSION == BERLIN_BG4_CDP_A0)
        /* 30 */  IHBPHYTXRXOCCCLK,
        /* 31 */  SDIOEXTRADIV2CLKCSEL,
#endif
        /* 32 */  NONCLK,
    };

    static const char* g_aClocks[] =
    {
        /*  0 */  "cpufastRefClk",
        /*  1 */  "memfastRefClk",
        /*  2 */  "cfgClk",
        /*  3 */  "sysClk",
        /*  4 */  "v4gClk",
        /*  5 */  "gfx3DCoreClk",
        /*  6 */  "gfx3DSysClk",
        /*  7 */  "avioSysClk",
        /*  8 */  "vppSysClk",
        /*  9 */  "arcRefClk",
#if (BERLIN_CHIP_VERSION != BERLIN_BG4_CDP_A0)
        /*    */  "hdmirxMClk",
#endif
        /* 10 */  "perifClk",
        /* 11 */  "tspClk",
        /* 12 */  "tspRefClk",
        /* 13 */  "atbClk",
        /* 14 */  "bcmClk",
        /* 15 */  "emmcClk",
        /* 16 */  "nfcEccClk",
        /* 17 */  "sd0Clk",
#if (BERLIN_CHIP_VERSION == BERLIN_BG4_CDP_A0)
        /* 18 */  "sd1Clk",
#endif
        /* 19 */  "usb2TestClk",
        /* 20 */  "sdio3DllMstRefClk",
        /* 21 */  "eddcClk",
        /* 22 */  "avioBiuClk",
        /* 23 */  "v4gM3Clk",
        /* 24 */  "apbCoreClk",
        /* 25 */  "zspClk",
        /* 26 */  "pcieTestClk",
        /* 27 */  "vipPipeClk",
        /* 28 */  "avioFpll400_clk",
        /* 29 */  "ihbPhyOccClk",
#if (BERLIN_CHIP_VERSION == BERLIN_BG4_CDP_A0)
        /* 30 */  "ihbPhyTxRxOccClk",
        /* 31 */  "sdioExtraDiv2ClkSel",
#endif
    };

    static const unsigned int g_aClkOffset[]=
    {
        /*  0 */  RA_Gbl_cpufastRefClk,
        /*  1 */  RA_Gbl_memfastRefClk,
        /*  2 */  RA_Gbl_cfgClk,
        /*  3 */  RA_Gbl_sysClk,
        /*  4 */  RA_Gbl_v4gClk,
        /*  5 */  RA_Gbl_gfx3DCoreClk,
        /*  6 */  RA_Gbl_gfx3DSysClk,
        /*  7 */  RA_Gbl_avioSysClk,
        /*  8 */  RA_Gbl_vppSysClk,
        /*  9 */  RA_Gbl_arcRefClk,
#if (BERLIN_CHIP_VERSION != BERLIN_BG4_CDP_A0)
        /*    */  RA_Gbl_hdmirxMClk,
#endif
        /* 10 */  RA_Gbl_perifClk,
        /* 11 */  RA_Gbl_tspClk,
        /* 12 */  RA_Gbl_tspRefClk,
        /* 13 */  RA_Gbl_atbClk,
        /* 14 */  RA_Gbl_bcmClk,
        /* 15 */  RA_Gbl_emmcClk,
        /* 16 */  RA_Gbl_nfcEccClk,
        /* 17 */  RA_Gbl_sd0Clk,
#if (BERLIN_CHIP_VERSION == BERLIN_BG4_CDP_A0)
        /* 18 */  RA_Gbl_sd1Clk,
#endif
        /* 19 */  RA_Gbl_usb2TestClk,
        /* 20 */  RA_Gbl_sdio3DllMstRefClk,
        /* 21 */  RA_Gbl_eddcClk,
        /* 22 */  RA_Gbl_avioBiuClk,
        /* 23 */  RA_Gbl_v4gM3Clk,
        /* 24 */  RA_Gbl_apbCoreClk,
        /* 25 */  RA_Gbl_zspClk,
        /* 26 */  RA_Gbl_pcieTestClk,
        /* 27 */  RA_Gbl_vipPipeClk,
        /* 28 */  RA_Gbl_avioFpll400_clk,
        /* 29 */  RA_Gbl_ihbPhyOccClk,
#if (BERLIN_CHIP_VERSION == BERLIN_BG4_CDP_A0)
        /* 30 */  RA_Gbl_ihbPhyTxRxOccClk,
        /* 31 */  RA_Gbl_sdioExtraDiv2ClkSel,
#endif
    };

#elif (((BERLIN_CHIP_VERSION == BERLIN_BG4_CT)||(BERLIN_CHIP_VERSION == BERLIN_BG4_CT_A0)))
    enum Clocks
    {
        /* 0 */  NONCLK = 0,
    };

    static const char* g_aClocks[] =
    {
    };

    static const unsigned int g_aClkOffset[] =
    {
    };
#else
    enum Clocks
    {
        /* 0 */  NONCLK = 0,
    };

    static const char* g_aClocks[] =
    {
    };

    static const unsigned int g_aClkOffset[]=
    {
    };
#endif


    dbg_printf1(PRN_INFO, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

    {
        unsigned int FbDiv;
        unsigned int vcoDiv_sel_setting;
        Tpll_ctrl pllCtl;
        unsigned int memPll;
        unsigned int sysPll;
        unsigned int cpuPll;
        unsigned int RefClkIn = 25; //25Mhz
        unsigned int RefDiv = 2;

        // memPll
        BFM_HOST_Bus_Read32((MEMPLL_CTRL_BASE + RA_pll_ctrl), &pllCtl.u32[0]);
        BFM_HOST_Bus_Read32((MEMPLL_CTRL_BASE + RA_pll_ctrl1), &pllCtl.u32[1]);
        FbDiv = pllCtl.uctrl_FBDIV;
        RefDiv = pllCtl.uctrl_REFDIV;
        vcoDiv_sel_setting = pllCtl.uctrl_CLKOUT_DIFF_DIV_SEL;
        memPll = FbDiv * 4 * RefClkIn / RefDiv;
        memPll = memPll / vcoDiv_sel_setting;
        dbg_printf1(PRN_RES, "\tmemPll frequency %u\n", memPll);

        // sysPll
        BFM_HOST_Bus_Read32((SYSPLL_CTRL_BASE + RA_pll_ctrl), &pllCtl.u32[0]);
        BFM_HOST_Bus_Read32((SYSPLL_CTRL_BASE + RA_pll_ctrl1), &pllCtl.u32[1]);
        FbDiv = pllCtl.uctrl_FBDIV;
        RefDiv = pllCtl.uctrl_REFDIV;
        vcoDiv_sel_setting = pllCtl.uctrl_CLKOUT_SE_DIV_SEL;
        sysPll = FbDiv * 4 * RefClkIn / RefDiv;
        sysPll = sysPll / vcoDiv_sel_setting;
        dbg_printf1(PRN_RES,  "\tsysPll frequency %u\n", sysPll);

        // cpuPll
        BFM_HOST_Bus_Read32((CPUPLL_CTRL_BASE + RA_pll_ctrl), &pllCtl.u32[0]);
        BFM_HOST_Bus_Read32((CPUPLL_CTRL_BASE + RA_pll_ctrl1), &pllCtl.u32[1]);
        FbDiv = pllCtl.uctrl_FBDIV;
        RefDiv = pllCtl.uctrl_REFDIV;
        vcoDiv_sel_setting = pllCtl.uctrl_CLKOUT_SE_DIV_SEL;
        cpuPll = FbDiv * 4 * RefClkIn / RefDiv;
        cpuPll = cpuPll / vcoDiv_sel_setting;
        dbg_printf1(PRN_RES, "\tcpuPll frequency %u\n", cpuPll);
    }

    {
        int i;
        for (i = 0; i < NONCLK; i++)
        {
            BFM_HOST_Bus_Read32((MEMMAP_CHIP_CTRL_REG_BASE + g_aClkOffset[i]), &clkDx.u32[0]);
            D3Switch = clkDx.uctrl_ClkD3Switch;
            Switch = clkDx.uctrl_ClkSwitch;
            Select = clkDx.uctrl_ClkSel;
            divider = get_divider(D3Switch, Switch, Select);
            PllSel = clkDx.uctrl_ClkPllSel;
            dbg_printf2(PRN_INFO, " 0x%X: 0x%X: D3Switch %d, Switch %d, Select %d, divider %d, PllSel %d\n",
                                    MEMMAP_CHIP_CTRL_REG_BASE + g_aClkOffset[i], clkDx.u32[0], D3Switch, Switch, Select, divider, PllSel);
            if (clkDx.uctrl_ClkPllSwitch)
                if (PllSel == 0)        // AVPLL
                    dbg_printf1(PRN_RES, " %-32sfrequency %.0f [%s]\n",
                            g_aClocks[i], current_freq[0][PllSel + 2] / divider, AVPLL[PllSel]);
                else if ((PllSel > 0) && (PllSel < 3))      // AVPLL
                    dbg_printf1(PRN_RES, " %-32sfrequency %.0f [%s]\n",
                            g_aClocks[i], current_freq[0][PllSel + 4] / divider, AVPLL[PllSel]);
                else                // sysPll P (differential output)
                    dbg_printf1(PRN_RES, " %-32sfrequency %d\n", g_aClocks[i], sysPll_diff_p / divider);
            else    // sysPll (single ended output)
                dbg_printf1(PRN_RES, " %-32sfrequency %d\n", g_aClocks[i], sysPll / divider);
        }
    }
    dbg_printf1(PRN_INFO, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
}


#include "Galois_memmap.h"
#include "avioGbl.h"

void AVPLL_DumpRegs(void) {
    AVPLL_REG_INFO avpll_reg_table[] = {
        {RA_avPll_ctrlPLL,                  "ctrlPLL"},
        {RA_avPll_ctrlPLL1,                 "ctrlPLL1"},
        {RA_avPll_ctrlCAL,                  "ctrlCAL"},
        {RA_avPll_ctrlCAL1,                 "ctrlCAL1"},
        {RA_avPll_ctrlSlowLoop,             "ctrlSlowLoop"},
        {RA_avPll_ctrlINTP,                 "ctrlINTP"},
        {RA_avPll_ctrlC8AddOn,              "ctrlC8AddOn"},
        {RA_avPll_C1 + RA_avpllCh_ctrl,     "C1_CTRL"},
        {RA_avPll_C1 + RA_avpllCh_ctrl1,    "C1_CTRL1"},
        {RA_avPll_C1 + RA_avpllCh_ctrl1,    "C1_CTRL2"},
        {RA_avPll_C1 + RA_avpllCh_ctrl1,    "C1_CTRL3"},
        {RA_avPll_C2 + RA_avpllCh_ctrl,     "C2_CTRL"},
        {RA_avPll_C2 + RA_avpllCh_ctrl1,    "C2_CTRL1"},
        {RA_avPll_C2 + RA_avpllCh_ctrl2,    "C2_CTRL2"},
        {RA_avPll_C2 + RA_avpllCh_ctrl2,    "C2_CTRL3"},
        {RA_avPll_C3 + RA_avpllCh_ctrl,     "C3_CTRL"},
        {RA_avPll_C3 + RA_avpllCh_ctrl1,    "C3_CTRL1"},
        {RA_avPll_C3 + RA_avpllCh_ctrl2,    "C3_CTRL2"},
        {RA_avPll_C3 + RA_avpllCh_ctrl3,    "C3_CTRL3"},
        {RA_avPll_C4 + RA_avpllCh_ctrl,     "C4_CTRL"},
        {RA_avPll_C4 + RA_avpllCh_ctrl1,    "C4_CTRL1"},
        {RA_avPll_C4 + RA_avpllCh_ctrl2,    "C4_CTRL2"},
        {RA_avPll_C4 + RA_avpllCh_ctrl3,    "C4_CTRL3"},
        {RA_avPll_C5 + RA_avpllCh_ctrl,     "C5_CTRL"},
        {RA_avPll_C5 + RA_avpllCh_ctrl1,    "C5_CTRL1"},
        {RA_avPll_C5 + RA_avpllCh_ctrl2,    "C5_CTRL2"},
        {RA_avPll_C5 + RA_avpllCh_ctrl3,    "C5_CTRL3"},
        {RA_avPll_C6 + RA_avpllCh_ctrl,     "C6_CTRL"},
        {RA_avPll_C6 + RA_avpllCh_ctrl1,    "C6_CTRL1"},
        {RA_avPll_C6 + RA_avpllCh_ctrl2,    "C6_CTRL2"},
        {RA_avPll_C6 + RA_avpllCh_ctrl3,    "C6_CTRL3"},
        {RA_avPll_C7 + RA_avpllCh_ctrl,     "C7_CTRL"},
        {RA_avPll_C7 + RA_avpllCh_ctrl1,    "C7_CTRL1"},
        {RA_avPll_C7 + RA_avpllCh_ctrl2,    "C7_CTRL2"},
        {RA_avPll_C7 + RA_avpllCh_ctrl3,    "C7_CTRL3"},
        {RA_avPll_C8 + RA_avpllCh8_ctrl,    "C8_CTRL"},
        {RA_avPll_C8 + RA_avpllCh8_ctrl1,   "C8_CTRL1"},
        {RA_avPll_C8 + RA_avpllCh8_ctrl2,   "C8_CTRL2"},
        {RA_avPll_C8 + RA_avpllCh8_ctrl3,   "C8_CTRL3"},
        {RA_avPll_ctrlTest,                 "ctrlTest"},
        {RA_avPll_status,                   "status"},
        {RA_avPll_status1,                  "status1"}
            };
            char g_avpll_names[2] = {'A','B'};

            UINT32 size = 0;
            UINT32 idx = 0;
            UINT32 regVal = 0;
            UINT32 i;
            UINT32 base_addr;

    unsigned int g_avpllBaseAddrPhy[2];

    //STORE phy address
    #if (BERLIN_CHIP_VERSION >= BERLIN_BG4_CD)
        g_avpllBaseAddrPhy[0] = (MEMMAP_AVIO_REG_BASE + AVIO_MEMMAP_AVIO_GBL_BASE + RA_avioGbl_AVPLLA);
    #else
        g_avpllBaseAddrPhy[0] = MEMMAP_AVIO_GBL_BASE + RA_avioGbl_AVPLLA;
    #endif
    #ifdef CONFIG_HAL_AVPLL_B_ENABLE
    g_avpllBaseAddrPhy[1] = (MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_AVPLLB);
    #endif

    size = sizeof(avpll_reg_table)/sizeof(AVPLL_REG_INFO);
    dbg_printf1(PRN_RES, "@@@@@@@ AVPLL REG DUMP START @@@@@@@\n");
    for(i=0;i<AVPLL_MAX;i++) {
        base_addr = g_avpllBaseAddrPhy[i];
        for(idx=0; idx<size; idx++){
            BFM_HOST_Bus_Read32(base_addr + avpll_reg_table[idx].offset_addr, &regVal);
            dbg_printf1(PRN_RES, "%c_%s\t 0x%X 0x%X\n", g_avpll_names[i],avpll_reg_table[idx].addr_name, base_addr + avpll_reg_table[idx].offset_addr, regVal);
        }
    }
    dbg_printf1(PRN_RES, "@@@@@@@ AVPLL REG DUMP END @@@@@@@\n");
}

void AVPLL_DumpChannelFrequency(void)
{
    double vco_freq;
    double freq_from_dpll, freq_from_offset;
    int offset, sync1, sync2, post_div, post_0p5_div;
    int enDPLL;
    SIE_avPll *avPllBase; int chId;
    const int MAX_CHANNELS = 8;
    int currVcoSetting;
    unsigned int g_avpllBaseAddr[AVPLL_MAX];
    unsigned int g_avpllBaseAddrPhy[AVPLL_MAX];
    int i;
    //STORE phy address
    #if (BERLIN_CHIP_VERSION >= BERLIN_BG4_CD)
        g_avpllBaseAddrPhy[0] = (MEMMAP_AVIO_REG_BASE + AVIO_MEMMAP_AVIO_GBL_BASE + RA_avioGbl_AVPLLA);
    #else
        g_avpllBaseAddrPhy[0] = MEMMAP_AVIO_GBL_BASE + RA_avioGbl_AVPLLA;
    #endif
    #ifdef CONFIG_HAL_AVPLL_B_ENABLE
    g_avpllBaseAddrPhy[1] = (MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_AVPLLB);
    #endif

    //STORE virt address
    g_avpllBaseAddr[0] = (unsigned int)AVPLL_A_ADDR;
    #ifdef CONFIG_HAL_AVPLL_B_ENABLE
    g_avpllBaseAddr[1] = (unsigned int)AVPLL_B_ADDR;
    #endif


    dbg_printf1(PRN_RES, "@@@@@@@ AVPLL REG Details DUMP START @@@@@@@\n");
    for(i=0;i<AVPLL_MAX;i++) {

        avPllBase = (SIE_avPll *)g_avpllBaseAddr[i];
        currVcoSetting = (avPllBase == (SIE_avPll *)AVPLL_A_ADDR) ? diag_pll_A_VCO_Setting : diag_pll_B_VCO_Setting;
        vco_freq=diag_vcoFreqs[currVcoSetting];

        dbg_printf1(PRN_RES, "AVPLL%s, avPllBase=0x%p/0x%x, diag_pll_VCO_Setting=%i, diag_vcoFreqs[diag_pll_VCO_Setting]=%f\n", \
                (avPllBase == (SIE_avPll *)AVPLL_A_ADDR)?"A":"B", avPllBase, g_avpllBaseAddrPhy[i], currVcoSetting, vco_freq);
        dbg_printf1(PRN_RES, "Channel\tMhz-Freq(Offset)\tMhz-Freq(DPLL)\t ----- offset\tsync1\tsync2\tpost_div\tpost_0p5_div\tenDPLL\n");

        for(chId=1; chId<=MAX_CHANNELS; chId++) {
            //find offset, sync1, sync2, divHDMI, divAV1, divAV2, divAV3 for this channel
            PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl1, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl1);
            offset = ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_FREQ_OFFSET;

            PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl2, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl2);
            sync1 = ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_P_SYNC1;

            PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl3, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl3);
            sync2 = ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_P_SYNC2;

            PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl);
            post_div = ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_POSTDIV;

            PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl);
            post_0p5_div = ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_POSTDIV_0P5;

            PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl);
            enDPLL = ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_EN_DPLL;

            if(enDPLL != 0)
                freq_from_dpll = vco_freq*sync2/sync1;
            else
                freq_from_dpll = vco_freq;

            if(offset & (1<<18))
            {
                offset = (offset & ((1<<18)-1));
            }
            else
            {
                offset = -offset;
            }
            freq_from_offset = 1/(1-(double)offset/4194304)*vco_freq;

            if(post_div)
            {
                freq_from_offset = freq_from_offset/(post_div+post_0p5_div*.5);
                freq_from_dpll = freq_from_dpll/(post_div+post_0p5_div*.5);
            }

            //dbg_printf(PRN_RES, "PLL%s:C%d Freq is: %fMHz(Offset) %fMHz(DPLL)\n", (avPllBase == (SIE_avPll *)AVPLL_A_ADDR)?"A":"B", chId, freq_from_offset, freq_from_dpll);
            dbg_printf1(PRN_RES, "AVPLL%s:C%d\t%4.5f\t\t%4.5f\t ----- %d\t%d\t%d\t%d\t\t%d\t\t%d\n",              \
                    (avPllBase == (SIE_avPll *)AVPLL_A_ADDR)?"A":"B", chId, freq_from_offset, freq_from_dpll,    \
                    offset, sync1, sync2, post_div, post_0p5_div, enDPLL);
        }
    }
    dbg_printf1(PRN_RES, "@@@@@@@ AVPLL REG Details DUMP END @@@@@@@\n");
}

void AVPLL_DumpVideoFrequency(void)
{
    int i;
    static const char *videoFreqStr[]  = {
        "VCO_FREQ_1_512G",
        "VCO_FREQ_1_620G",
        "VCO_FREQ_1_856G",
        "VCO_FREQ_2_160G",
        "VCO_FREQ_2_227G",
        "VCO_FREQ_2_520G",
        "VCO_FREQ_2_700G",
        "VCO_FREQ_2_970G",
    };
    dbg_printf1(PRN_RES, "@@@@@@@ AVPLL Video frequency DUMP START @@@@@@@\n");
    for(i=0;i<=VCO_FREQ_2_970G;i++) {
        if(clk_freqs_computed[i].invalid)
        {
            dbg_printf1(PRN_RES, "%s:%d:%s VALID: %4.2fGHz\t%4.3fMHz\tN\n", \
                    videoFreqStr[i], i, clk_freqs_computed[i].invalid ? "IN": "  ", \
                    diag_vcoFreqs[i], \
                    clk_freqs_computed[i].clkout_freq);
        }
        else
        {
            dbg_printf1(PRN_RES, "%s:%d:%s VALID: %4.2fGHz\t%4.3fMHz\t%6d\t%6d\t%6d\t%d\t%d\t%1.3f\n", videoFreqStr[i], i, clk_freqs_computed[i].invalid ? "IN": "  ",
                    diag_vcoFreqs[i],
                    clk_freqs_computed[i].clkout_freq,
                    clk_freqs_computed[i].freq_offset,
                    clk_freqs_computed[i].p_sync2,
                    clk_freqs_computed[i].p_sync1,
                    clk_freqs_computed[i].post_div,
                    clk_freqs_computed[i].post_div_0p5,
                    (((double)clk_freqs_computed[i].p_sync2-(double)clk_freqs_computed[i].p_sync1))/clk_freqs_computed[i].p_sync1);
        }
    }
    dbg_printf1(PRN_RES, "@@@@@@@ AVPLL Video frequency DUMP END @@@@@@@\n");
}

static void AVPLL_DumpClkConfig(void) {
    int index, target_clks;
    AVPLL_CLOCK_CONFIG *clkCfg = clk_config;
    target_clks = sizeof(clk_config)/sizeof(AVPLL_CLOCK_CONFIG);

    dbg_printf1(PRN_RES, "@@@@@@@ AVPLL CLK Configuration DUMP START @@@@@@@\n");
    dbg_printf1(PRN_RES, "ClkCfg: TargetClk |\ttarget_select | target_clk_freq | target_default_sel |\tpll_select |\tsource_clk_freq |\tdivider\n");
    for(index = 0; index < target_clks; index++, clkCfg++) {
        dbg_printf1(PRN_RES, "ClkCfg: %9d |\t%13x | %15d | %15x |\t%9d |\t%d |\t%d \n",index,
            clkCfg->target_select, clkCfg->target_clk_freq, clkCfg->target_default_sel, clkCfg->pll_select, clkCfg->source_clk_freq, clkCfg->divider);
    }
    dbg_printf1(PRN_RES, "@@@@@@@ AVPLL CLK Configuration DUMP END @@@@@@@\n");

}

void AVPLL_DumpAll(void) {
    AVPLL_DumpSpeed();
    AVPLL_DumpRegs();
    AVPLL_DumpChannelFrequency();
    AVPLL_DumpVideoFrequency();
}

void AVPLL_DBG_processPrintCmd(unsigned int printCmd) {
    int printMainSubCmd = printCmd&0x0F;

    switch(printMainSubCmd) {
        case 0:
            AVPLL_DumpAll();
        break;
        case AVPLL_CMD_DUMP_SPEED:
            AVPLL_DumpSpeed();
        break;
        case AVPLL_CMD_DUMP_REGS:
            AVPLL_DumpRegs();
        break;
        case AVPLL_CMD_DUMP_CHAN_FREQ:
            AVPLL_DumpChannelFrequency();
        break;
        case AVPLL_CMD_DUMP_VIDEO_FREQ:
            AVPLL_DumpVideoFrequency();
        break;
        case AVPLL_CMD_DUMP_CLK_CFG:
            AVPLL_DumpClkConfig();
        break;
    }
}

