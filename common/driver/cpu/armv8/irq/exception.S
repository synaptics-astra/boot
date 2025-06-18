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
#include "asm_macro.h"

/*
 * Vector entry
 */
.macro ventry label
	.align 7
	b \label
.endm

/*
 * Enter Exception.
 * This will save the processor state that is ELR/X0~X30
 * to the stack frame.
 */
.macro	exception_entry
	stp	x29, x30, [sp, #-16]!
	stp	x27, x28, [sp, #-16]!
	stp	x25, x26, [sp, #-16]!
	stp	x23, x24, [sp, #-16]!
	stp	x21, x22, [sp, #-16]!
	stp	x19, x20, [sp, #-16]!
	stp	x17, x18, [sp, #-16]!
	stp	x15, x16, [sp, #-16]!
	stp	x13, x14, [sp, #-16]!
	stp	x11, x12, [sp, #-16]!
	stp	x9, x10, [sp, #-16]!
	stp	x7, x8, [sp, #-16]!
	stp	x5, x6, [sp, #-16]!
	stp	x3, x4, [sp, #-16]!
	stp	x1, x2, [sp, #-16]!

	/* Could be running at EL3/EL2/EL1 */
	jump_by_el x11, 3f, 2f, 1f
3:	mrs	x1, esr_el3
	mrs	x2, elr_el3
	b	0f
2:	mrs	x1, esr_el2
	mrs	x2, elr_el2
	b	0f
1:	mrs	x1, esr_el1
	mrs	x2, elr_el1
0:
	stp	x2, x0, [sp, #-16]!
	mov	x0, sp
.endm


.macro	exception_exit
	ldp	x2, x0, [sp], #16

	/* Could be running at EL3/EL2/EL1 */
	jump_by_el x11, 3f, 2f, 1f
3:	msr	elr_el3, x2
	b	0f
2:	msr	elr_el2, x2
	b	0f
1:	msr	elr_el1, x2
0:
	ldp     x1, x2, [sp], #16
	ldp     x3, x4, [sp], #16
	ldp     x5, x6, [sp], #16
	ldp     x7, x8, [sp], #16
	ldp     x9, x10, [sp], #16
	ldp     x11, x12, [sp], #16
	ldp     x13, x14, [sp], #16
	ldp     x15, x16, [sp], #16
	ldp     x17, x18, [sp], #16
	ldp     x19, x20, [sp], #16
	ldp     x21, x22, [sp], #16
	ldp     x23, x24, [sp], #16
	ldp     x25, x26, [sp], #16
	ldp     x27, x28, [sp], #16
	ldp     x29, x30, [sp], #16
	eret
.endm



.global	exception_handler
	.section	.vectors, "ax"; .align 11
exception_handler:
	ventry sp0_sync
	ventry sp0_irq
	ventry sp0_fiq
	ventry sp0_serror

	ventry spx_sync
	ventry spx_irq
	ventry spx_fiq
	ventry spx_serror

	ventry a64_sync
	ventry a64_irq
	ventry a64_fiq
	ventry a64_serror

	ventry a32_sync
	ventry a32_irq
	ventry a32_fiq
	ventry a32_serror

	/* -----------------------------------------------------
	 * Current EL with SP0 : 0x0 - 0x200
	 * -----------------------------------------------------
	 */
sp0_sync:
	b	panic

sp0_irq:
	b	panic

sp0_fiq:
	b	panic

sp0_serror:
	b	panic

	/* -----------------------------------------------------
	 * Current EL with SPx: 0x200 - 0x400
	 * -----------------------------------------------------
	 */
spx_sync:
	b	sync_handle

spx_irq:
	exception_entry
	bl	do_irq
	exception_exit

spx_fiq:
	b	panic

spx_serror:
	b	panic

	/* -----------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600
	 * -----------------------------------------------------
	 */
a64_sync:
	b	sync_handle

a64_irq:
	b	panic

a64_fiq:
	b	panic

a64_serror:
	b	panic

	/* -----------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * -----------------------------------------------------
	 */
a32_sync:
	b	panic

a32_irq:
	b	panic

a32_fiq:
	b	panic

a32_serror:
	b	panic

panic:
	b	panic

sync_handle:		// 0x400: Synchronous
	exception_entry
	jump_by_el x11, el3_sync, el2_sync, panic
	exception_exit

el3_sync:
	mrs	x1, esr_el3
	ubfx	x2, x1, 26, 6	// EC, bits [31:26]
	cmp	x2, #0x13
	beq	_smc_handle	// SMC from AArch32
	cmp	x2, #0x17
	beq	_smc_handle
	b	do_sync

_smc_handle:
	LDP	x2, x0, [sp]	// SMC parameter from x0

	// set NS
	mrs	x1, scr_el3
	bic	x1, x1, #1
	ubfx	x2, x0, #5, #1
	orr	x1, x1, x2
	msr	scr_el3, x1

	// set M[4:0]
	mrs	x1, spsr_el3
	bic	x1, x1, #0x1f
	and	x2, x0, #0x1f
	orr	x1, x1, x2
	msr	spsr_el3, x1

	exception_exit

el2_sync:
	mrs	x1, esr_el2
loop1:
	b	loop1
	//b	do_sync
