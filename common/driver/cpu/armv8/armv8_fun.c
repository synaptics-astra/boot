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
#include "debug.h"
#include "timer.h"
#include "apb_watchdog.h"

struct pt_regs{
	unsigned long elr;
	unsigned long regs[31];
};

void EnableIRQ(void)
{
	__asm__ __volatile__(
		"MSR     DAIFCLR, #2"   "\n\t"
		:
		:
		:"cc"
	);
}

uint32_t getMPid(void)
{
	unsigned int val;
	asm volatile("mrs %0, MPIDR_EL1" : "=r" (val) : : "cc");
	val &= 0xf;
	return val;
}

static void show_regs(struct pt_regs *regs)
{
	int i;

	dbg_printf(PRN_ERR, "ELR:     %lx\n", regs->elr);
	dbg_printf(PRN_ERR, "LR:      %lx\n", regs->regs[30]);
	for (i = 0; i < 29; i += 2)
		dbg_printf(PRN_ERR, "x%-2d: %016lx x%-2d: %016lx\n", i, regs->regs[i], i+1, regs->regs[i+1]);
	dbg_printf(PRN_ERR, "\n");
}

void do_sync(struct pt_regs *pt_regs, unsigned int esr)
{
	dbg_printf(PRN_ERR, "\"Synchronous Abort\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);

	dbg_printf(PRN_ERR, "Resetting CPU ...\n");
#ifndef CONFIG_PANIC_HANG
	udelay(100000);	/* allow messages to go out */
	reset_soc();
#endif

	while(1);
}
