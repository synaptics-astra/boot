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
#ifndef _DIAG_GIC_
#define _DIAG_GIC_

#include "soc.h"        // include for interrupt id

#define GENERIC_TIMER_IRQ_ID_S          0x1D
#define GENERIC_TIMER_IRQ_ID_NS         0x1E

#define INT_ENABLE      1
#define INT_DISABLE     0

// MPCore GIC interrupt IDs
#define MP_BERLIN_INTR_ID(id)   (id + 32)   // berlin interrupts starts from ID 32

typedef int (*irq_handle_func)(void* param);

void EnableIRQ(void);
unsigned int DisableIRQ(void);
void RestoreIRQ(unsigned int cpsr_save);

void EnableFIQ(void);
unsigned int DisableFIQ(void);
void RestoreFIQ(unsigned int cpsr_save);

void initMPGIC(void);

int diag_GICSetInt(int MPid, int intId, int enable);

void GIC_IRQ_Handler(void);
void GIC_FIQ_Handler(void);

int diag_GICSetHandle(int intId, irq_handle_func handle, void* param);

int diag_GicSGI(int cpuList, int sgiId);

int diag_GICSetPriority(int intId, int bFIQ, int priority);

// print interrupt status
int diag_gic_print_status();

#endif // _DIAG_GIC_
