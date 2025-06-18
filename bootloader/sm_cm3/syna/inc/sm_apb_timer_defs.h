/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright © 2013-2020 Synaptics Incorporated. All rights reserved.
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
#ifndef __SM_APB_TIMER_DEFS_H__
#define __SM_APB_TIMER_DEFS_H__

//#define APB_TIMER_DISABLE 1

#define TIMERS_APB_TIMER0_BASE	SM_SM_APB_TIMER0_BASE
#define TIMERS_APB_TIMER1_BASE	SM_SM_APB_TIMER1_BASE

#define TIMER_LOADCOUNTOFF      0x00
#define TIMER_CURRENTVALOFF     0x04
#define TIMER_CONTROLREGOFF     0x08
#define TIMER_EOIOFF            0x0c
#define TIMER_INTSTATOFF        0x10

/* there are 8 timers from timer 1 to timer 8 */
#define APB_TIMER0_BASE(x)      (TIMERS_APB_TIMER0_BASE + ((x - 1) * 0x14))

#define TIMER0_INTSTAT          (TIMERS_APB_TIMER0_BASE + 0xa0)
#define TIMER0_EOI              (TIMERS_APB_TIMER0_BASE + 0xa4)
#define TIMER0_RAWINTSTAT       (TIMERS_APB_TIMER0_BASE + 0xa8)
#define TIMER0_COMPVERSION      (TIMERS_APB_TIMER0_BASE + 0xac)

#define TIMER_LOADCOUNT(x)      (APB_TIMER0_BASE(x) + TIMER_LOADCOUNTOFF)
#define TIMER_CURRENTVAL(x)     (APB_TIMER0_BASE(x) + TIMER_CURRENTVALOFF)
#define TIMER_CONTROLREG(x)     (APB_TIMER0_BASE(x) + TIMER_CONTROLREGOFF)
#define TIMER_EOI(x)            (APB_TIMER0_BASE(x) + TIMER_EOIOFF)
#define TIMER_INTSTAT(x)        (APB_TIMER0_BASE(x) + TIMER_INTSTATOFF)

#endif	//#define __SM_APB_TIMER_DEFS_H__
