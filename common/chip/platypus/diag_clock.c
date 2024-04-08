/******************************************************************************
*                 2018 Synaptics Incorporated. All Rights Reserved            *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF Synaptics.                   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
* OF Synaptics OR ANY THIRD PARTY. Synaptics RESERVES THE RIGHT AT ITS SOLE   *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO Synaptics.  *
* THIS CODE IS PROVIDED "AS IS". Synaptics MAKES NO WARRANTIES, EXPRESSED,    *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
*                                                                             *
*                   WARNING FOR USING THIS DIAG CODE                          *
*   1. Please take your own responsibility if you refer DIAG source code for  *
*      any type software development.                                         *
*   2. DIAG code is mainly for internal validation purpose, and not intend to *
*      be used in production SW.                                              *
*   3. DIAG code is subject to change without notice.                         *
*******************************************************************************/
#include "diag_clock.h"
#include "Galois_memmap.h"
#include "global.h"
#include "cpu_wrp.h"
#include "diag_pll.h"
#include "mc_wrap.h"
#include "mc_defines.h"
#include "io.h"

#define ARRAY_NUM(a)            (sizeof(a)/sizeof(a[0]))
//#define ENABLE_REE_SET_SECURE_CLOCK

struct g_aClock_info_t
{
    const char*     clock_name;
    unsigned int    reg_offset;
    unsigned int    source;
    unsigned int    div;
};

#define SRC_IDX_SYSPLL0_PLLOUTF     SRC_IDX_MAX                         // default SYSPLL0 PLLOUTF for CLKDx and SIPLLOUTF for sissCLKDx
#define SRC_IDX_SIPLL_PLLOUTF        SRC_IDX_MAX                         // default SYSPLL0 PLLOUTF for CLKDx and SIPLLOUTF for sissCLKDx
#define SRC_IDX_SYSPLL0_PLLOUT      clkD4_ctrl_ClkPllSel_CLKSRC0
#define SRC_IDX_SIPLL_PLLOUT        clkD4_ctrl_ClkPllSel_CLKSRC1
#define SRC_IDX_SYSPLL1_PLLOUT      clkD4_ctrl_ClkPllSel_CLKSRC2
#define SRC_IDX_AVPLL0              clkD4_ctrl_ClkPllSel_CLKSRC3
#define SRC_IDX_SYSPLL1_PLLOUTF     clkD4_ctrl_ClkPllSel_CLKSRC4
#define SRC_IDX_SISS_AVPLL1         clkD4_ctrl_ClkPllSel_CLKSRC4        // SISS uses a slight different index with non-SISS clocks.
#define SRC_IDX_MAX                 (SRC_IDX_SYSPLL1_PLLOUTF + 1)

#define DEF_ACLOCK_INFO(name,source,div) {#name, RA_Gbl_##name, SRC_IDX_##source, div}

const struct g_aClock_info_t g_aClocks_VH[] =
{   // VH clock (based on v3p0, , updated for decoderClk=900/gfx3DCoreClk=900/dspClk=900/npuClk=800/tspClk=800)
    DEF_ACLOCK_INFO(    cpufastRefClk         ,   /* 600 */ SYSPLL0_PLLOUT  ,   1),  // 0
    DEF_ACLOCK_INFO(    memfastRefClk         ,   /* 600 */ SYSPLL0_PLLOUT  ,   1),  // 1
    DEF_ACLOCK_INFO(    cfgClk                ,   /* 100 */ SYSPLL0_PLLOUT  ,   6),  // 2
    DEF_ACLOCK_INFO(    perifSysClk           ,   /* 300 */ SYSPLL0_PLLOUT  ,   2),  // 3
    DEF_ACLOCK_INFO(    atbClk                ,   /* 100 */ SYSPLL0_PLLOUT  ,   6),  // 4
    DEF_ACLOCK_INFO(    decoderClk            ,   /* 900 */ SYSPLL0_PLLOUTF ,   1),  // 5
    DEF_ACLOCK_INFO(    encoderClk            ,   /* 300 */ SYSPLL0_PLLOUT  ,   2),  // 6
    DEF_ACLOCK_INFO(    ovpCoreClk            ,   /* 400 */ SYSPLL1_PLLOUTF ,   2),  // 7
    DEF_ACLOCK_INFO(    gfx3DCoreClk          ,   /* 900 */ SYSPLL0_PLLOUTF ,   1),  // 8
    DEF_ACLOCK_INFO(    gfx3DSysClk           ,   /* 700 */ SIPLL_PLLOUT    ,   1),  // 9
    DEF_ACLOCK_INFO(    avioSysClk            ,   /* 400 */ SYSPLL1_PLLOUTF ,   2),  // 10
    DEF_ACLOCK_INFO(    vppSysClk             ,   /* 600 */ SYSPLL0_PLLOUT  ,   1),  // 11
    DEF_ACLOCK_INFO(    esmClk                ,   /* 100 */ SYSPLL0_PLLOUT  ,   6),  // 12
    DEF_ACLOCK_INFO(    avioBiuClk            ,   /* 250 */ SYSPLL1_PLLOUT  ,   2),  // 13
    DEF_ACLOCK_INFO(    vipPipeClk            ,   /* 600 */ SYSPLL0_PLLOUT  ,   1),  // 14
    DEF_ACLOCK_INFO(    avioFpll400_clk       ,   /* 400 */ SYSPLL1_PLLOUTF ,   2),  // 15
    DEF_ACLOCK_INFO(    apbCoreClk            ,   /* 200 */ SYSPLL0_PLLOUT  ,   3),  // 19
    DEF_ACLOCK_INFO(    emmcClk               ,   /* 200 */ SYSPLL0_PLLOUT  ,   3),  // 20
    DEF_ACLOCK_INFO(    sd0Clk                ,   /* 200 */ SYSPLL0_PLLOUT  ,   3),  // 21
    DEF_ACLOCK_INFO(    usb3CoreClk           ,   /* 400 */ SYSPLL1_PLLOUTF ,   2),  // 22
    DEF_ACLOCK_INFO(    npuClk                ,   /* 800 */ SYSPLL1_PLLOUTF ,   1),  // 24
    DEF_ACLOCK_INFO(    hdmiRxrefClk          ,   /* 250 */ SYSPLL1_PLLOUT  ,   2),  // 27
    DEF_ACLOCK_INFO(    aioSysClk             ,   /* 200 */ SYSPLL0_PLLOUT  ,   3),  // 28
    DEF_ACLOCK_INFO(    usimClk               ,   /* 350 */ SIPLL_PLLOUT    ,   2),  // 29
    DEF_ACLOCK_INFO(    dspSysClk             ,   /* 400 */ SYSPLL1_PLLOUTF ,   2),  // 31
    DEF_ACLOCK_INFO(    dsp0Clk               ,   /* 900 */ SYSPLL0_PLLOUTF ,   1),  // 32
    DEF_ACLOCK_INFO(    dsp1Clk               ,   /* 900 */ SYSPLL0_PLLOUTF ,   1),  // 33
    DEF_ACLOCK_INFO(    vnSysClk              ,   /* 700 */ SIPLL_PLLOUT    ,   1),  // 34
#ifdef ENABLE_REE_SET_SECURE_CLOCK
    DEF_ACLOCK_INFO(    tspClk                ,   /* 800 */ SIPLL_PLLOUTF   ,   1),  // 16
    DEF_ACLOCK_INFO(    tspRefClk             ,   /* 200 */ SIPLL_PLLOUTF   ,   4),  // 17
    DEF_ACLOCK_INFO(    nocsClk               ,   /* 200 */ SIPLL_PLLOUTF   ,   4),  // 18
    DEF_ACLOCK_INFO(    bcmClk                ,   /* 200 */ SIPLL_PLLOUTF   ,   4),  // 23
    DEF_ACLOCK_INFO(    sissSysClk            ,   /* 350 */ SIPLL_PLLOUT    ,   2),  // 25
    DEF_ACLOCK_INFO(    ifcpClk               ,   /* 200 */ SIPLL_PLLOUTF   ,   4),  // 26
    DEF_ACLOCK_INFO(    nskClk                ,   /* 200 */ SIPLL_PLLOUTF   ,   4),  // 30
#endif
};

const struct g_aClock_info_t g_aClocks_VL[] =
{   // VL clock (based on v3p0, , updated for decoderClk=800/gfx3DCoreClk=800/dspClk=800/npuClk=700/tspClk=800)
    DEF_ACLOCK_INFO(    cpufastRefClk         ,   /* 600 */ SYSPLL0_PLLOUT  ,   1),  // 0
    DEF_ACLOCK_INFO(    memfastRefClk         ,   /* 600 */ SYSPLL0_PLLOUT  ,   1),  // 1
    DEF_ACLOCK_INFO(    cfgClk                ,   /* 100 */ SYSPLL0_PLLOUT  ,   6),  // 2
    DEF_ACLOCK_INFO(    perifSysClk           ,   /* 300 */ SYSPLL0_PLLOUT  ,   2),  // 3
    DEF_ACLOCK_INFO(    atbClk                ,   /* 100 */ SYSPLL0_PLLOUT  ,   6),  // 4
    DEF_ACLOCK_INFO(    decoderClk            ,   /* 800 */ SYSPLL1_PLLOUTF ,   1),  // 5
    DEF_ACLOCK_INFO(    encoderClk            ,   /* 300 */ SYSPLL0_PLLOUT  ,   2),  // 6
    DEF_ACLOCK_INFO(    ovpCoreClk            ,   /* 400 */ SYSPLL1_PLLOUTF ,   2),  // 7
    DEF_ACLOCK_INFO(    gfx3DCoreClk          ,   /* 800 */ SYSPLL1_PLLOUTF ,   1),  // 8
    DEF_ACLOCK_INFO(    gfx3DSysClk           ,   /* 700 */ SIPLL_PLLOUT    ,   1),  // 9
    DEF_ACLOCK_INFO(    avioSysClk            ,   /* 400 */ SYSPLL1_PLLOUTF ,   2),  // 10
    DEF_ACLOCK_INFO(    vppSysClk             ,   /* 600 */ SYSPLL0_PLLOUT  ,   1),  // 11
    DEF_ACLOCK_INFO(    esmClk                ,   /* 100 */ SYSPLL0_PLLOUT  ,   6),  // 12
    DEF_ACLOCK_INFO(    avioBiuClk            ,   /* 250 */ SYSPLL1_PLLOUT  ,   2),  // 13
    DEF_ACLOCK_INFO(    vipPipeClk            ,   /* 600 */ SYSPLL0_PLLOUT  ,   1),  // 14
    DEF_ACLOCK_INFO(    avioFpll400_clk       ,   /* 400 */ SYSPLL1_PLLOUTF ,   2),  // 15
    DEF_ACLOCK_INFO(    apbCoreClk            ,   /* 200 */ SYSPLL0_PLLOUT  ,   3),  // 19
    DEF_ACLOCK_INFO(    emmcClk               ,   /* 200 */ SYSPLL0_PLLOUT  ,   3),  // 20
    DEF_ACLOCK_INFO(    sd0Clk                ,   /* 200 */ SYSPLL0_PLLOUT  ,   3),  // 21
    DEF_ACLOCK_INFO(    usb3CoreClk           ,   /* 400 */ SYSPLL1_PLLOUTF ,   2),  // 22
    DEF_ACLOCK_INFO(    npuClk                ,   /* 700 */ SIPLL_PLLOUT    ,   1),  // 24
    DEF_ACLOCK_INFO(    hdmiRxrefClk          ,   /* 250 */ SYSPLL1_PLLOUT  ,   2),  // 27
    DEF_ACLOCK_INFO(    aioSysClk             ,   /* 200 */ SYSPLL0_PLLOUT  ,   3),  // 28
    DEF_ACLOCK_INFO(    usimClk               ,   /* 350 */ SIPLL_PLLOUT    ,   2),  // 29
    DEF_ACLOCK_INFO(    dspSysClk             ,   /* 400 */ SYSPLL1_PLLOUTF ,   2),  // 31
    DEF_ACLOCK_INFO(    dsp0Clk               ,   /* 800 */ SYSPLL1_PLLOUTF ,   1),  // 32
    DEF_ACLOCK_INFO(    dsp1Clk               ,   /* 800 */ SYSPLL1_PLLOUTF ,   1),  // 33
    DEF_ACLOCK_INFO(    vnSysClk              ,   /* 700 */ SIPLL_PLLOUT    ,   1),  // 34
#ifdef ENABLE_REE_SET_SECURE_CLOCK
    DEF_ACLOCK_INFO(    tspClk                ,   /* 700 */ SIPLL_PLLOUT    ,   1),  // 16
    DEF_ACLOCK_INFO(    tspRefClk             ,   /* 200 */ SIPLL_PLLOUTF   ,   4),  // 17
    DEF_ACLOCK_INFO(    nocsClk               ,   /* 200 */ SIPLL_PLLOUTF   ,   4),  // 18
    DEF_ACLOCK_INFO(    bcmClk                ,   /* 200 */ SIPLL_PLLOUTF   ,   4),  // 23
    DEF_ACLOCK_INFO(    sissSysClk            ,   /* 350 */ SIPLL_PLLOUT    ,   2),  // 25
    DEF_ACLOCK_INFO(    ifcpClk               ,   /* 200 */ SIPLL_PLLOUTF   ,   4),  // 26
    DEF_ACLOCK_INFO(    nskClk                ,   /* 200 */ SIPLL_PLLOUTF   ,   4),  // 30
#endif
};

const unsigned int clock_divider[] =
{
    1,
    2,
    4,
    6,
    8,
    12,
    1,
    1
};

int speed_cpu;
unsigned int RefClkIn = 25;
unsigned int RefDiv = 2;
float current_freq[8];
float cpupll_freqs[2];

int diag_clock_change_otherClk(unsigned int index, const struct g_aClock_info_t * g_aClocks)
{
    unsigned int pllSwitch = g_aClocks[index].source<SRC_IDX_MAX;
    unsigned int pllSelect = g_aClocks[index].source<SRC_IDX_MAX ? g_aClocks[index].source : 0x4;
    unsigned int divider = g_aClocks[index].div;
    TclkD1_ctrl  clkDx;

#if (PLATFORM == FPGA)
    dbg_printf(PRN_RES, "Warning: real clock is not changed for FPGA\n");
#endif

    if ((pllSwitch > 1) || (pllSelect > 4))
    {
        dbg_printf(PRN_ERR, " invalid parameter!\n");
        return -1;
    }
    else if ((divider != 1) && (divider != 2) && (divider != 3) && (divider != 4) &&
            (divider != 6) && (divider != 8) && (divider != 12))
    {
        dbg_printf(PRN_ERR, " invalid divider!\n");
        return -1;
    }

//     1) program divider to 12
    BFM_HOST_Bus_Read32((MEMMAP_CHIP_CTRL_REG_BASE + g_aClocks[index].reg_offset), &clkDx.u32[0]);
    clkDx.uctrl_ClkD3Switch = 0;
    clkDx.uctrl_ClkSwitch = 1;
    clkDx.uctrl_ClkSel = DIVIDED_BY_12;
    BFM_HOST_Bus_Write32((MEMMAP_CHIP_CTRL_REG_BASE + g_aClocks[index].reg_offset), clkDx.u32[0]);

//    2) switch to syspll by setting clkPllSwitch
    clkDx.uctrl_ClkPllSwitch = 0;
    BFM_HOST_Bus_Write32((MEMMAP_CHIP_CTRL_REG_BASE + g_aClocks[index].reg_offset), clkDx.u32[0]);

//    3) program clkPllSel (if target clock source is avpll or syspll_clkoutp)
    clkDx.uctrl_ClkPllSel = pllSelect;
    BFM_HOST_Bus_Write32((MEMMAP_CHIP_CTRL_REG_BASE + g_aClocks[index].reg_offset), clkDx.u32[0]);

//    4) switch to avpll/syspll_clkoutp by setting clkPllSwitch (if target clock source is avpll or syspll_clkoutp)
    clkDx.uctrl_ClkPllSwitch = pllSwitch;
    BFM_HOST_Bus_Write32((MEMMAP_CHIP_CTRL_REG_BASE + g_aClocks[index].reg_offset), clkDx.u32[0]);

//    5) program proper divider
//    BFM_HOST_Bus_Read32((MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_ClkSwitch), &ClkSwitch.u32[0]);
    BFM_HOST_Bus_Read32((MEMMAP_CHIP_CTRL_REG_BASE + g_aClocks[index].reg_offset), &clkDx.u32[0]);

    // sysPll bypass ON
//    ClkSwitch.uClkSwitch_sysPLLSWBypass = 1;
//    BFM_HOST_Bus_Write32((MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_ClkSwitch), ClkSwitch.u32[0]);

    // use default sysPll
//    clkDx.uctrl_ClkPllSwitch = 0;
//    BFM_HOST_Bus_Write32((MEMMAP_CHIP_CTRL_REG_BASE + g_aClkOffset[index]), clkDx.u32[0]);

    // change divider to divided-by-3 first
//    clkDx.uctrl_ClkD3Switch = 1;
//    BFM_HOST_Bus_Write32((MEMMAP_CHIP_CTRL_REG_BASE + g_aClkOffset[index]), clkDx.u32[0]);

    // change divider to target
    switch (divider)
    {
        case 1:
            clkDx.uctrl_ClkD3Switch = 0;
            clkDx.uctrl_ClkSwitch = 0;
            break;
        case 2:
            clkDx.uctrl_ClkD3Switch = 0;
            clkDx.uctrl_ClkSwitch = 1;
            clkDx.uctrl_ClkSel = DIVIDED_BY_2;
            break;
        case 3:
            clkDx.uctrl_ClkD3Switch = 1;
            break;
        case 4:
            clkDx.uctrl_ClkD3Switch = 0;
            clkDx.uctrl_ClkSwitch = 1;
            clkDx.uctrl_ClkSel = DIVIDED_BY_4;
            break;
        case 6:
            clkDx.uctrl_ClkD3Switch = 0;
            clkDx.uctrl_ClkSwitch = 1;
            clkDx.uctrl_ClkSel = DIVIDED_BY_6;
            break;
        case 8:
            clkDx.uctrl_ClkD3Switch = 0;
            clkDx.uctrl_ClkSwitch = 1;
            clkDx.uctrl_ClkSel = DIVIDED_BY_8;
            break;
        case 12:
            clkDx.uctrl_ClkD3Switch = 0;
            clkDx.uctrl_ClkSwitch = 1;
            clkDx.uctrl_ClkSel = DIVIDED_BY_12;
            break;
        default:
            dbg_printf(PRN_ERR, " this is impossible\n");
            break;
    }
    BFM_HOST_Bus_Write32((MEMMAP_CHIP_CTRL_REG_BASE + g_aClocks[index].reg_offset), clkDx.u32[0]);

    dbg_printf(PRN_RES, "  Changed %s, now\n", g_aClocks[index].clock_name);

    return 0;
}

unsigned int get_divider(unsigned int D3Switch, unsigned int Switch, unsigned int Select)
{
    unsigned int divider;
    if (D3Switch)
        divider = 3;
    else
    {
        if (!Switch)
            divider = 1;
        else
            divider = clock_divider[Select];
    }
    return divider;
}

const char* cpu_clks_name[] =
{
    "cpu0",
    "cpu1",
    "cpu2",
    "cpu3",
    "dsu",
};

void list_cpu_speed()
{
    int index;

    unsigned int    reg_cpuClkSel;
    unsigned int    reg_cpuClkSwitch;
    unsigned int    reg_cpuClkD3Switch;
    unsigned int    reg_cpuClkDivSel;

    BFM_HOST_Bus_Read32(MEMMAP_CA7_REG_BASE + RA_CPU_WRP_CPU_REG + RA_CPU_REG_cpuClkSel,         &reg_cpuClkSel);
    BFM_HOST_Bus_Read32(MEMMAP_CA7_REG_BASE + RA_CPU_WRP_CPU_REG + RA_CPU_REG_cpuClkSwitch,      &reg_cpuClkSwitch);
    BFM_HOST_Bus_Read32(MEMMAP_CA7_REG_BASE + RA_CPU_WRP_CPU_REG + RA_CPU_REG_cpuClkD3Switch,    &reg_cpuClkD3Switch);
    BFM_HOST_Bus_Read32(MEMMAP_CA7_REG_BASE + RA_CPU_WRP_CPU_REG + RA_CPU_REG_cpuClkDivSel,      &reg_cpuClkDivSel);

    for (index = 0; index < ARRAY_NUM(cpu_clks_name); index++)
    {
        int divider;
        int D3Switch = (reg_cpuClkD3Switch>>index) & 1;
        int Switch = (reg_cpuClkSwitch>>index) & 1;
        int Select = (reg_cpuClkDivSel>>(4*index)) & 0xf;
        int ClkSel = (reg_cpuClkSel>>index) & 1;

        divider = get_divider(D3Switch, Switch, Select);

        if (index != 4)  ClkSel = !ClkSel;

        dbg_printf(PRN_RES, " %-32sfrequency %4.0f [%s/%d]\n",
                cpu_clks_name[index], cpupll_freqs[ClkSel]/divider, ClkSel?"CPUPLL_OUTF":"CPUPLL_OUT", divider);

        if (index == 0)
        {
            speed_cpu = cpupll_freqs[ClkSel]/divider;   // use core0 freq for global cpu speed
        }
    }
}

void list_speed(int level)
{
#if PLATFORM == ASIC
    CLOCKO_t sysclock0 = diag_get_syspll0();
    CLOCKO_t sysclock1 = diag_get_syspll1();
#ifdef ENABLE_REE_SET_SECURE_CLOCK
    CLOCKO_t sipll = diag_get_sipll();
#endif
    CLOCKO_t cpuclock = diag_get_cpupll();
    CLOCKO_t memclock = diag_get_mempll();
    CLOCKO_t avpll0clock = diag_get_avpll(0);
    CLOCKO_t avpll1clock = diag_get_avpll(0);

    // update global pll freq table here
    current_freq[0] = sysclock0.clocko;
    current_freq[1] = sysclock0.clocko1;
    current_freq[2] = sysclock1.clocko;
    current_freq[3] = sysclock1.clocko1;
#ifdef ENABLE_REE_SET_SECURE_CLOCK
    current_freq[4] = sipll.clocko;
    current_freq[5] = sipll.clocko1;
#endif
    current_freq[6] = avpll0clock.clocko;
    current_freq[7] = avpll1clock.clocko;

    cpupll_freqs[0] = cpuclock.clocko;
    cpupll_freqs[1] = cpuclock.clocko1;

    // memPll
	dbg_printf(PRN_RES, " %-32sfrequency %d\n", "memPLL", memclock.clocko);

    // sysPll
	dbg_printf(PRN_RES, " %-32sfrequency %d\n", "sysPLL0", sysclock0.clocko);

    // sysPll clkoutp
	dbg_printf(PRN_RES, " %-32sfrequency %d\n", "sysPLL0_CLKO1", sysclock0.clocko1);

    // sysPll
	dbg_printf(PRN_RES, " %-32sfrequency %d\n", "sysPLL1", sysclock1.clocko);

    // sysPll clkoutp
	dbg_printf(PRN_RES, " %-32sfrequency %d\n", "sysPLL1_CLKO1", sysclock1.clocko1);

#ifdef ENABLE_REE_SET_SECURE_CLOCK
    // siPll
	dbg_printf(PRN_RES, " %-32sfrequency %d\n", "siPLL", sipll.clocko);

    // siPll clkoutp
	dbg_printf(PRN_RES, " %-32sfrequency %d\n", "siPLL_CLKO1", sipll.clocko1);
#endif

    // cpuPll
	dbg_printf(PRN_RES, " %-32sfrequency %d\n", "cpuPLL", cpuclock.clocko);
	dbg_printf(PRN_RES, " %-32sfrequency %d\n", "cpuPLLO1", cpuclock.clocko1);

    // avPll0
	dbg_printf(PRN_RES, " %-32sfrequency %d\n", "AVPLL0", avpll0clock.clocko);

    // avPll1
	dbg_printf(PRN_RES, " %-32sfrequency %d\n", "AVPLL1", avpll1clock.clocko);

    // DDR
	dbg_printf(PRN_RES, " %-32sfrequency %d\n", "dclk", memclock.clocko);
#endif

    // CPU
    list_cpu_speed();

}

int diag_clock_USBOTG_REFCLK_div(int divval, int low_count)
{
    BFM_HOST_Bus_Write32((MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_USBOTG_REFCLK_CTRL0), divval);
    BFM_HOST_Bus_Write32((MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_USBOTG_REFCLK_CTRL1), low_count);

    dbg_printf(PRN_INFO, " USBOTG refclk div (0x%08x):%d, low_count (0x%08x):%d\n",
        (MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_USBOTG_REFCLK_CTRL0), divval,
        (MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_USBOTG_REFCLK_CTRL1), low_count);

    return 0;
}

int diag_clock_set_clocks(unsigned int set, unsigned int mask)
{
    unsigned int i;

#if PLATFORM == ASIC
    if (mask & 1)   //cpupll
    {
        diag_change_cpupll(1350, 1800, 0, 0, 0, 1);
        speed_cpu = diag_get_cpupll().clocko;
    }

    if (mask & 2)   //syspll0
    {
        diag_change_syspll0(600, 900, 0, 0, 0, 1);
    }

    if (mask & 4)   //syspll1
    {
        diag_change_syspll1(500, 800, 0, 0, 0, 1);
        diag_clock_USBOTG_REFCLK_div(800/20, 800/20/2);
    }

#ifdef ENABLE_REE_SET_SECURE_CLOCK
    if (mask & 8)   //sipll
    {
        diag_change_sipll(700, 800, 0, 0, 0, 1);
    }
#endif //ENABLE_ACPU_SET_SECURE_CLOCK
#endif

    switch (set)
    {
        case CLOCK_SET_VL:
            for (i=0; i<ARRAY_NUM(g_aClocks_VL); i++)
            {
                diag_clock_change_otherClk(i, g_aClocks_VL);
            }
            break;

        case CLOCK_SET_VH:
            for (i=0; i<ARRAY_NUM(g_aClocks_VH); i++)
            {
                diag_clock_change_otherClk(i, g_aClocks_VH);
            }
            break;

        default:
            dbg_printf(PRN_ERR, " unknown clock set(%d)\n", set);
            break;
    }
    return 0;
}
