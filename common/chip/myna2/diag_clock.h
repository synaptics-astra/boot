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
#ifndef _DIAG_CLOCK_H_
#define _DIAG_CLOCK_H_

enum Clocks
{
    CPUFASTREFCLK         ,  // 0
    MEMFASTREFCLK         ,  // 1
    CFGCLK                ,  // 2
    PERIFSYSCLK           ,  // 3
    ATBCLK                ,  // 4
    DECODERCLK            ,  // 5
    ENCODERCLK            ,  // 6
    OVPCORECLK            ,  // 7
    GFX3DCORECLK          ,  // 8
    GFX3DSYSCLK           ,  // 9
    AVIOSYSCLK            ,  // 10
    VPPSYSCLK             ,  // 11
    ESMCLK                ,  // 12
    AVIOBIUCLK            ,  // 13
    VIPPIPECLK            ,  // 14
    AVIOFPLL400_CLK       ,  // 15
    TSPCLK                ,  // 16
    TSPREFCLK             ,  // 17
    NOCSCLK               ,  // 18
    APBCORECLK            ,  // 19
    EMMCCLK               ,  // 20
    SD0CLK                ,  // 21
    USB3CORECLK           ,  // 22
    BCMCLK                ,  // 23
    NPUCLK                ,  // 24
    SISSSYSCLK            ,  // 25
    IFCPCLK               ,  // 26
    HDMIRXREFCLK          ,  // 27
    AIOSYSCLK             ,  // 28
    USIMCLK               ,  // 29
    NSKCLK                ,  // 30
    DSPSYSCLK             ,  // 31
    DSP0CLK               ,  // 32
    DSP1CLK               ,  // 33
    VNSYSCLK              ,  // 34
    USB2TESTCLK           ,  // 35
    USB2TESTCLK480MGROUP0 ,  // 36
    USB2TESTCLK480MGROUP1 ,  // 37
    USB2TESTCLK480MGROUP2 ,  // 38
    USB2TESTCLK100MGROUP0 ,  // 39
    USB2TESTCLK100MGROUP1 ,  // 40
    USB2TESTCLK100MGROUP2 ,  // 41
    USB2TESTCLK100MGROUP3 ,  // 42
    USB2TESTCLK100MGROUP4 ,  // 43
    PERIFTESTCLK50MGROUP0 ,  // 44
    PERIFTESTCLK125MGROUP0,  // 45
    PERIFTESTCLK200MGROUP0,  // 46
    PERIFTESTCLK200MGROUP1,  // 47
    PERIFTESTCLK250MGROUP0,  // 48
    PERIFTESTCLK500MGROUP0,  // 49
};

enum
{
	LIST_CPU_DDR_SPEEDS,
	LIST_ALL_SPEEDS
};

#define LIST_ALL_SPEEDS 1


extern unsigned int get_divider(unsigned int D3Switch, unsigned int Switch,
		unsigned int Select);
extern void list_speed(int level);
extern void diag_clock_list();

int diag_clock_get_otherClk(int index);

#if PLATFORM != ASIC
int diag_clock_set_mempll_fpga_veloce(int mempll);
int diag_clock_get_mempll_fpga_veloce();
#endif // PLATFORM != ASIC

#define CLOCK_SET_T2        0
#define CLOCK_SET_VL        1
#define CLOCK_SET_VH        2


// ClkSel,0:cpupll_out, 1:cpupll_outF

int diag_clock_change_cpuClk(unsigned int index, unsigned int ClkSel, unsigned int divider, int en_print);

// USBOTG_REFCLK (20MHz) is sourced from SYSPLL2_F on DOLPHIN, it is from SYSPLL1_F on PLATYPUS and AS470.
// for 50% duty, low_count = divval/2. divval should be even.
int diag_clock_USBOTG_REFCLK_div(int divval, int low_count);

#endif // _DIAG_CLOCK_H_




