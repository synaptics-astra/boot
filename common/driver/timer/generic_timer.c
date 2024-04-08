/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright © 2013-2021 Synaptics Incorporated. All rights reserved.
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

unsigned long long get_ticks(void)
{
	unsigned long cntpct;

	asm volatile("isb");
	asm volatile("mrs %0, cntpct_el0" : "=r" (cntpct));

	return cntpct;
}

unsigned long get_syscount(void)
{
	unsigned long sys_count;

	asm volatile("mrs %0, cntfrq_el0" : "=r" (sys_count));

	return sys_count;
}

unsigned long long us_to_tick(unsigned long us)
{
	unsigned long long tick = us;

	tick *= get_syscount();
	tick /= 1000000;

	return tick;
}

void stop_timer(void)
{
}

unsigned int get_time_ms(void)
{
	unsigned long long tick = get_ticks();

	tick *= 1000;
	tick /= get_syscount();

	return tick;
}

void udelay(unsigned int usec)
{
	unsigned long long tmp;

	tmp = get_ticks() + us_to_tick(usec);
	while (get_ticks() < tmp+1);
}

void mdelay(unsigned int msec)
{
	udelay(1000 * msec);
}

void ndelay(unsigned int nsec)
{
	unsigned int div_rup = (nsec + 1000 -1)/1000;
	udelay(div_rup);
}
