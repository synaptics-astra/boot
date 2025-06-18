/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright � 2013-2018 Synaptics Incorporated. All rights reserved.
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
#ifndef __SM_TIMER_H__
#define __SM_TIMER_H__
#include "FreeRTOS.h"
#include "task.h"

/* we already defined the width of tick count to 64bit.
 * so there is no long time again
 */

// define long time structure
typedef union{
	unsigned long ltime;
	struct {
		unsigned int low;
		unsigned int high;
	};
} timer_long_t;

static inline unsigned long mv_sm_timer_gettimems(void) {	return (xTaskGetTickCount() / pdMS_TO_TICKS(1)); };

/* there are different APIs in FreeRTOS to get tick from nonISR/ISR function
 */
static inline unsigned long mv_sm_timer_gettimems_formISR(void) {	return ((unsigned long)(xTaskGetTickCountFromISR() / pdMS_TO_TICKS(1))); };

/* this function can't be used in ISR
 * even that we use freertos, I disable the preemption in order to simplify the loop
 * I write this function here just in case. I don't hope to use this function
*/
static inline void mv_sm_delay_ms(int ms) {
	unsigned long t1 = 0, t2 = 0;
	t1 = mv_sm_timer_gettimems();
	do {
		t2 = mv_sm_timer_gettimems();
	} while((t2 - t1) >= ms);
};

/* get tick from SYST_CVR, don't know if it work */
static inline unsigned int mv_sm_timer_getticks(void) {
	return (((configCPU_CLOCK_HZ / configTICK_RATE_HZ) - 1) - (*((volatile uint32_t *) 0xe000e018)));
};

#endif //__SM_TIMER_H__

