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
    HPICLK                ,  // 12
    AVIOBIUCLK            ,  // 13
    VIPPIPECLK            ,  // 14
    AVIOFPLL400_CLK       ,  // 15
    TSPCLK                ,  // 16
    TSPREFCLK             ,  // 17
    NOCSCLK               ,  // 18
    APBCORECLK            ,  // 19
    EMMCCLK               ,  // 20
    SD0CLK                ,  // 21
    GETHRGMIICLK          ,  // 22
    AVIOOVPCLK            ,  // 23
    PERIFTESTCLK125MGROUP0,  // 24
    USB2TESTCLK           ,  // 25
    PERIFTESTCLK250MGROUP0,  // 26
    USB3CORECLK           ,  // 27
    BCMCLK                ,  // 28
    VXSYSCLK              ,  // 29
    NPUCLK                ,  // 30
    SISSSYSCLK            ,  // 31
    IFCPCLK               ,  // 32
    ISSSYSCLK             ,  // 33
    ISPCLK                ,  // 34
    ISPBECLK              ,  // 35
    ISPDSCCLK             ,  // 36
    ISPCSI0CLK            ,  // 37
    ISPCSI1CLK            ,  // 38
    HDMIRXREFCLK          ,  // 39
    MIPIRXSCANBYTECLK     ,  // 40
    USB2TESTCLK480MGROUP0 ,  // 41
    USB2TESTCLK480MGROUP1 ,  // 42
    USB2TESTCLK480MGROUP2 ,  // 43
    USB2TESTCLK100MGROUP0 ,  // 44
    USB2TESTCLK100MGROUP1 ,  // 45
    USB2TESTCLK100MGROUP2 ,  // 46
    USB2TESTCLK100MGROUP3 ,  // 47
    USB2TESTCLK100MGROUP4 ,  // 48
    PERIFTESTCLK200MGROUP0,  // 49
    PERIFTESTCLK200MGROUP1,  // 50
    PERIFTESTCLK500MGROUP0,  // 51
    TXCLKESC              ,  // 52
    AIOSYSCLK             ,  // 53
    NONCLK                ,  // 54
};

enum
{
	LIST_CPU_DDR_SPEEDS,
	LIST_ALL_SPEEDS
};

#define LIST_ALL_SPEEDS 1

extern int diag_clock_change_cpuPll(unsigned int frequency);
extern int diag_clock_change_sysPll(unsigned int frequency, unsigned int frequency_clkoutp);
extern int diag_clock_change_otherClk(unsigned int index, unsigned int
		pllSwitch, unsigned int pllSel, unsigned int divider, int en_print);
extern unsigned int get_divider(unsigned int D3Switch, unsigned int Switch,
		unsigned int Select);
extern void list_speed(int level);
extern void diag_clock_list();

int diag_clock_get_mempll();
int diag_clock_get_syspll();
int diag_clock_get_otherClk(int index);

#if PLATFORM != ASIC
int diag_clock_set_mempll_fpga_veloce(int mempll);
int diag_clock_get_mempll_fpga_veloce();
#endif // PLATFORM != ASIC

#define CLOCK_SET_T2        0
#define CLOCK_SET_VL        1
#define CLOCK_SET_VH        2

#endif // _DIAG_CLOCK_H_
