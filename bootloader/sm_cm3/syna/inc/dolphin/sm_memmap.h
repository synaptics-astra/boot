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
#ifndef _SM_MEMMAP_
#define _SM_MEMMAP_

#include "SysMgr.h"

/******************************************************
*   Dragonite DTCM Memory Allocation Table              *
*                                                       *
*        SOC_SYNC + OUT_BUF          *
*        SHARED_BUF                  *
*        DATA + BSS                  *
*        HEAP                        *
*        IRQ_STACK                   *
*        STACK                       *
*                                    *
********************************************************/

#define RANGE_1K (0x400)
#define RANGE_2K (0x800)
#define RANGE_4K (0x1000)
#define RANGE_16K (0x4000)

// ITCM 64K, DTCM 16K
#define SM_SM_ITCM_BASE         (0x00000000)
#define SM_SM_ITCM_SIZE         (0x00010000)

#define SM_SM_DTCM_BASE         (0x00010000)
#define SM_SM_DTCM_SIZE         (0x00010000)

// Memory for SOC and SM to exchange cmd, params and status. (Keep this at start of DTCM)
#define SM_MEMMAP_SOC_SYNC_BASE     (SM_SM_DTCM_BASE)
#define SM_MEMMAP_SOC_SYNC_SIZE      RANGE_2K

//addr range that is used by program and can run memory test on
// FIXME: please make it align to 16 Bytes
#define SM_MEMMAP_SOC_SM_SHARED_BUF_BASE    (SM_MEMMAP_SOC_SYNC_BASE + SM_MEMMAP_SOC_SYNC_SIZE)
#define SM_MEMMAP_SOC_SM_SHARED_BUF_SIZE    RANGE_2K

#define SM_MEMMAP_DATA_BSS_BASE     (SM_MEMMAP_SOC_SM_SHARED_BUF_BASE + SM_MEMMAP_SOC_SM_SHARED_BUF_SIZE)
#define SM_MEMMAP_DATA_BSS_SIZE     RANGE_4K

// Memory for Heap
//#define MEMMAP_HEAP_BASE        0x04002000
//#define MEMMAP_HEAP_SIZE        (MEMMAP_SHARED_BUF_BASE - MEMMAP_HEAP_BASE)

// IRQ SM stack, 1K bytes
#define SM_MEMMAP_IRQ_STACK_BASE    (SM_MEMMAP_STACK_BASE - SM_MEMMAP_STACK_SIZE)
#define SM_MEMMAP_IRQ_STACK_SIZE    RANGE_1K

// SM stack, 2K bytes
//#define SM_MEMMAP_STACK_BASE        (SM_SM_DTCM_BASE + SM_SM_DTCM_SIZE - 4)
//#define SM_MEMMAP_STACK_SIZE        RANGE_2K

#define SM_MEMMAP_APB_BASE          SM_MEMMAP_SMAPB_BASE    // (0x40000000)
#define SM_MEMMAP_SYS_CTRL_BASE     SM_MEMMAP_SMREG_BASE    // (0x40011000)
#define SM_MEMMAP_SEC_RAM_BASE      SM_MEMMAP_SSRAM_BASE    // (0x10000000)

#define SM_SM_APB_ICTL0_BASE        (SM_MEMMAP_APB_BASE + SM_APB_ICTL_0)    // 0x0000
#define SM_SM_APB_ICTL1_BASE        (SM_MEMMAP_APB_BASE + SM_APB_ICTL_1)    // 0x1000     // interrupt output to soc
#define SM_SM_APB_ICTL2_BASE        (SM_MEMMAP_APB_BASE + SM_APB_ICTL_2)    // 0x2000     // interrupt output to soc
#define SM_SM_APB_WDT0_BASE         (SM_MEMMAP_APB_BASE + SM_APB_WDT_0)     // 0x3000
#define SM_SM_APB_WDT1_BASE         (SM_MEMMAP_APB_BASE + SM_APB_WDT_1)     // 0x4000
#define SM_SM_APB_WDT2_BASE         (SM_MEMMAP_APB_BASE + SM_APB_WDT_2)     // 0x5000
#define SM_SM_APB_TIMER0_BASE       (SM_MEMMAP_APB_BASE + SM_APB_TIMERS_0)  // 0x6000
#define SM_SM_APB_TIMER1_BASE       (SM_MEMMAP_APB_BASE + SM_APB_TIMERS_1)  // 0x7000
#define SM_SM_APB_GPIO0_BASE        (SM_MEMMAP_APB_BASE + SM_APB_GPIO_0)    // 0x8000
#define SM_SM_APB_SPI_BASE          (SM_MEMMAP_APB_BASE + SM_APB_SSI)       // 0xA000
#define SM_SM_APB_I2C0_BASE         (SM_MEMMAP_APB_BASE + SM_APB_I2C_0)     // 0xB000
#define SM_SM_APB_I2C1_BASE         (SM_MEMMAP_APB_BASE + SM_APB_I2C_1)     // 0xC000
#define SM_SM_APB_UART0_BASE        (SM_MEMMAP_APB_BASE + SM_APB_UART_0)    // 0xD000
#define SM_SM_APB_UART1_BASE        (SM_MEMMAP_APB_BASE + SM_APB_UART_1)    // 0xE000

#endif
