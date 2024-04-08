/*
*
*     Copyright ARM Ltd 2007. All rights reserved.
*
* Public header file for the MPCore Interrupt Distributor.
* This file contains definitions of the CT11MPCore primary GIC registers.
*/

#ifndef __MP_GIC_H__
#define __MP_GIC_H__


// MPCore GIC interrupt IDs
#define MP_BERLIN_INTR_ID(id)   (id + 32)   // berlin interrupts starts from ID 32

#define INT_ENABLE		1
#define INT_DISABLE		0

void initMPGIC(void);
void GIC_IRQ_Handler(void);
void initMPGIC(void);
void reinitMPGIC(void);
void set_irq_enable(int irq_id);

#endif // __MP_GIC_H__

