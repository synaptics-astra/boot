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
#include "sm_type.h"
#include "sm_io.h"
#include "SysMgr.h"
#include "sm_memmap.h"
#include "sm_common.h"

#include "FreeRTOSConfig.h"

#include "sm_rt_module.h"
#include "sm_apb_ictl_defs.h"
#include "sm_ictl.h"

void sm_ictl_enable(int irq, int enable)
{
    unsigned int value;

    if (irq < 32)
    {
        BFM_HOST_Bus_Read32(APB_ICTL_IRQ_INTEN_L, &value);    // 0x00, IRQ interrupt source enable register (low)
    }
    else
    {
        BFM_HOST_Bus_Read32(APB_ICTL_IRQ_INTEN_H, &value);    // 0x04, IRQ interrupt source enable register (high)
    }

    if (enable)
    {
        value |= 1 << ((irq<32)?irq:irq-32);
    }
    else
    {
        value &= ~(1 << ((irq<32)?irq:irq-32));
    }

    if (irq < 32)
    {
        BFM_HOST_Bus_Write32(APB_ICTL_IRQ_INTEN_L, value);
    }
    else
    {
        BFM_HOST_Bus_Write32(APB_ICTL_IRQ_INTEN_H, value);
    }
}

static int __attribute__((used)) sm_ictl_disable(void)
{
	// disable all interrupts
	BFM_HOST_Bus_Write32(APB_ICTL_IRQ_INTEN_L, 0);	  // 0x00, IRQ interrupt source enable register (low)
	BFM_HOST_Bus_Write32(APB_ICTL_IRQ_INTEN_H, 0);	  // 0x04, IRQ interrupt source enable register (high)

	return S_OK;
}

DECLARE_RT_INIT(disint, INIT_DEV_P_0, sm_ictl_disable);

static int __attribute__((used)) sm_ictl_init(void)
{
	T32smSysCtl_SM_CTRL reg;
#ifdef WDTENABLE
	unsigned int tmp = 0;
#endif

	// Enable Timer_0 interrupt
	sm_ictl_enable(SMICTL_IRQ_TIMER0_0, 1);

#ifdef USE_IR_BLASTER
	sm_ictl_enable(SMICTL_IRQ_SM_UART1, 1);
#endif

	// enable the interrupt of gpio group A
	sm_ictl_enable(SMICTL_IRQ_SM_GPIO0, 1);

#ifdef WDTENABLE
	// enable watchdog 0 and 1
	sm_ictl_enable(SMICTL_IRQ_SM_WDT0, 1);
	sm_ictl_enable(SMICTL_IRQ_SM_WDT1, 1);

	tmp = MV_SM_READ_REG32(APB_ICTL_IRQ_INTFORCE_L);
	// active low,wdt 0,high active
	MV_SM_WRITE_REG32(APB_ICTL_IRQ_INTFORCE_L, tmp | 0x7);

	//clear wdt0 interrupt

#endif

#ifdef TWSI0ENABLE
	// enable twsi0
	sm_ictl_enable(SMICTL_IRQ_SM_I2C0, 1);
#endif

#ifdef HWCEC
	// done in cec module itself
	sm_ictl_enable(SMICTL_IRQ_CEC, 1);
#endif

	// disable FIQ
	MV_SM_WRITE_REG32(APB_ICTL_FIQ_INTEN, 0);

	// clear SM to SoC interrupt
	reg.u32 = MV_SM_READ_REG32(SM_MEMMAP_SYS_CTRL_BASE + RA_smSysCtl_SM_CTRL);
	reg.uSM_CTRL_SM2SOC_SW_INTR = 0;
	MV_SM_WRITE_REG32(SM_MEMMAP_SYS_CTRL_BASE + RA_smSysCtl_SM_CTRL,reg.u32);

	//set IRQ1 priority
	*(volatile unsigned char*)(0xE000E401) = configMAX_SYSCALL_INTERRUPT_PRIORITY;
	//CM3 enable IRQ1
	*(volatile unsigned int*)(0xE000E100) = (1<<1);

	return S_OK;
}

DECLARE_RT_INIT(initint, INIT_DEV_P_50, sm_ictl_init);
