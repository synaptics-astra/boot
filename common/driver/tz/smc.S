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

	.text

#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
	.arch_extension sec
#endif

/*
 * Note: a DSB is required before call smc. It's to avoid any "pending"
 *       imprecise abort to bother the other world.
 *       It's not to ensure the memory access completed, which is already
 *       guanteed by hardware.
 */

	.globl __smc_cmd2
__smc_cmd2:
	push	{r3}

	dsb
	smc	#0

	pop	{ip}
	cmp	ip, #0
	stmne	ip!, {r1-r2}

	bx	lr

	.globl __smc_cmd3
__smc_cmd3:
	dsb
	smc	#0

	ldr	ip, [sp]
	cmp	ip, #0
	stmne	ip!, {r1-r3}

	bx	lr

	.globl __smc3
__smc3:
	dsb
	smc	#0

	ldr	ip, [sp]
	cmp	ip, #0
	stmne	ip!, {r0-r3}

	bx	lr

	.globl __smc6
__smc6:
	mov	ip, r2
	push	{r4-r6, ip}

	/* load parameters to registers */
	mov	ip, r1
	ldm	ip!, {r1-r6}

	dsb
	smc	#0

	pop	{r4-r6, ip}
	cmp	ip, #0
	stmne	ip!, {r0-r3}

	bx	lr

	.globl __smc
__smc:
	dsb
	smc	#0
	bx	lr
