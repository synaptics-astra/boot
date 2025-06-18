/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright © 2013-2018 Synaptics Incorporated. All rights reserved.
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
#include "com_type.h"

#include "io.h"
#include "apbRegBase.h"
#include "debug.h"

#include "timer.h"

/* this function can only work after cfgclk is inited/stable */
/*note: the max value we can delay is (0xffffffff/DEFAULT_CFG_CLK) us
  or (0xffffffff/(DEFAULT_CFG_CLK * 1000)) ms
  */
#if defined(APB_TIMER_DISABLE)

void delay_cycles(unsigned long cycles)
{
	cycles /= 2;
	__asm__ __volatile__ (
		"1:	subs	%0, %0, #1	\n\t"
		"	bhi	1b		\n\t"
		: "+r" (cycles)
		:
		: "cc"
		);
}
void ndelay(unsigned int ns)
{
#ifndef CPUPLL
#define CPUPLL	1000
#endif
//#define VELOCE_PLAT
#ifndef VELOCE_PLAT
	unsigned long cycles = (unsigned long)((uint64_t)ns * CPUPLL / 1000);
#else
	unsigned long cycles = ns / 1000;
#endif
	delay_cycles(cycles);
}
void udelay(unsigned int us)
{
	const unsigned int step_us = 1000000;
	const unsigned int step_ns = 1000 * step_us;
	while (us > step_us) {
		ndelay(step_ns);
		us -= step_us;
	}
	if (us)
		ndelay(1000 * us);
}
void init_timer(void)
{
	//null
}
void delay_us(unsigned int us)
{
	udelay(us);
}

void mdelay(unsigned int ms)
{
	const unsigned int step_ms = 1000000;
	const unsigned int step_us = 1000 * step_ms;
	while (ms > step_ms) {
		udelay(step_us);
		ms -= step_ms;
	}
	if (ms)
		udelay(1000 * ms);
}
void delay_ms(unsigned int ms)
{
	mdelay(ms);
}

unsigned int get_time_ms(void)
{
	return 0;
}

void stop_timer(void)
{
}

#else

#define APB_TIMER_INST_BASE APB_TIMER0_BASE
#ifndef DEFAULT_CFG_CLK
#define DEFAULT_CFG_CLK (100)
#endif

#define TIMER_MS(x)	(x * 1000 * DEFAULT_CFG_CLK)
#define TIMER_US(x)	(x * DEFAULT_CFG_CLK)
#define TIMER_10NS(x)	((x * DEFAULT_CFG_CLK) / 100)
#define MAX_MS		(0xFFFFFFFF / (DEFAULT_CFG_CLK * 1000))
#define MAX_US          (MAX_MS * 1000)
#define MAX_10NS        (MAX_US * 100)

static INT32 timer_setctrl(UINT32 mask, UINT32 mode, UINT32 enable)
{
	UINT32 val = 0;

	if (mask > 1 || mode > 1 || enable > 1) {
		dbg_printf(PRN_ERR, "%s - arg check fail!", __func__);
        return FALSE;
	}

	val = mask << 2 | mode << 1;
	writel(val, (APB_TIMER_INST_BASE + 0x0008));
	val |= enable;
	writel(val, (APB_TIMER_INST_BASE + 0x0008));

	return TRUE;
}

static INT32 timer_setloadcount(INT32 loadCount)
{
	writel(loadCount, (APB_TIMER_INST_BASE + 0x0000));
	return TRUE;
}

static UINT32 timer_getloadcount()
{
	return readl((APB_TIMER_INST_BASE + 0x0004));
}

void init_timer(void)
{
    timer_setloadcount(0xFFFFFFFF);
    timer_setctrl(0, 0, 1); // set to free-running mode
}

void delay_10ns(unsigned int ns)
{
    if(ns < MAX_10NS) {
        unsigned int loadcount = timer_getloadcount();

        while((loadcount - timer_getloadcount()) < TIMER_10NS(ns))
        {};
    }
}

void udelay(unsigned int us)
{
    if(us < MAX_US) {
        unsigned int loadcount = timer_getloadcount();

        while((loadcount - timer_getloadcount()) < TIMER_US(us))
        {};
    }
}

void delay_us(unsigned int us)
{
	udelay(us);
}

void mdelay(unsigned int ms)
{
    if(ms < MAX_MS)
        delay_us(ms * 1000);
}

void delay_ms(unsigned int ms)
{
	mdelay(ms);
}

/* the max ms is 42949 */
unsigned int get_time_ms(void)
{
    return ((0xFFFFFFFF - timer_getloadcount()) / TIMER_MS(1));
}

void stop_timer(void)
{
    timer_setctrl(0, 0, 0);
}
#endif

