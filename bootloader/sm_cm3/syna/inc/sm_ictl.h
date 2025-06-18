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
#ifndef _SM_DIAG_ICTL_H_
#define _SM_DIAG_ICTL_H_

#define SMICTL_IRQ_SM_WDT0              0
#define SMICTL_IRQ_SM_WDT1              1
#define SMICTL_IRQ_SM_WDT2              2
#define SMICTL_IRQ_RSVD0                3
#define SMICTL_IRQ_SM_GPIO1             4
#define SMICTL_IRQ_SM_SPI               5
#define SMICTL_IRQ_SM_I2C0              6
#define SMICTL_IRQ_SM_I2C1              7
#define SMICTL_IRQ_SM_UART0             8
#define SMICTL_IRQ_SM_UART1             9
#define SMICTL_IRQ_RSVD1                10
#define SMICTL_IRQ_SM_GPIO0             11
#define SMICTL_IRQ_ADC                  12
#define SMICTL_IRQ_SOC2SM               13
#define SMICTL_IRQ_TSEN                 14
#define SMICTL_IRQ_RSVD2                15
#define SMICTL_IRQ_CEC                  16
#define SMICTL_IRQ_FIFO_CEC             17
#define SMICTL_IRQ_ETH1_INT_FEPHY       18
#define SMICTL_IRQ_HPD                  19
#define SMICTL_IRQ_HPD_INV              20
#define SMICTL_IRQ_TIMER0_0             21
#define SMICTL_IRQ_TIMER1_0             22
#define SMICTL_IRQ_TIMER1_1             23
#define SMICTL_IRQ_TIMER1_2             24
#define SMICTL_IRQ_TIMER1_3             25
#define SMICTL_IRQ_TIMER1_4             26
#define SMICTL_IRQ_TIMER1_5             27
#define SMICTL_IRQ_TIMER1_6             28
#define SMICTL_IRQ_TIMER1_7             29
#define SMICTL_IRQ_HDMIRXPWR5V          30
#define SMICTL_IRQ_SMI_MGP_INT_N	31
#define SMICTL_SMI_EXMGP_INT_N          32
#define SMICTL_IRQ_MAX                  33

void sm_ictl_enable(int irq, int enable);

#endif // _SM_DIAG_ICTL_H_
