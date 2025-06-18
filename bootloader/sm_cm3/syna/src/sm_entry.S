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

	.syntax unified
	.cpu cortex-m3
	.thumb

.global _asm_start
.global	_vector_base
.global	Default_Handler

.extern __stack_start
.extern __stack_end
.extern __initstack_start

_asm_start:
_vector_base:
	.word	__stack_start
	.word	Reset_Handler
	.word	NMI_Handler
	.word	HardFault_Handler
	.word	MemManage_Handler
	.word	BusFault_Handler
	.word	UsageFault_Handler
	.word	0
	.word	0
	.word	0
	.word	0
	.word	vPortSVCHandler
	.word	DebugMon_Handler
	.word	0
	.word	xPortPendSVHandler
	.word	xPortSysTickHandler
	.word	IRQ_0_Handler
	.word	IRQ_1_Handler

    .section	.text.Reset_Handler
	.weak	Reset_Handler
	.type	Reset_Handler, %function
Reset_Handler:
	ldr r0, =__initstack_start
	mov sp, r0

/* Zero fill the bss segment. */
zerobss:
	.extern  __bss_start
	.extern  __bss_end
	ldr	r0, =__bss_start
	ldr r1, =__bss_end
	mov	r4, #0
	mov r5, #0
	mov r6, #0
	mov r7, #0
bss_quadclear:
	stmia	r0!, {r4-r7}
	cmp r0, r1
	blt	bss_quadclear

	ldr r0, =_vector_base
	bl set_vector_base_and_clear_exception

/* Call the application's entry point.*/
	bl	_start
	bx	lr
.size	Reset_Handler, .-Reset_Handler

/**
 * @brief  This is the code that gets called when the processor receives an
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 *
 * @param  None
 * @retval : None
*/
    .section	.text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
	b	Infinite_Loop
	.size	Default_Handler, .-Default_Handler

/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler.
* As they are weak aliases, any function with the same name will override
* this definition.
*
*******************************************************************************/
	.weak  NMI_Handler
	.thumb_set NMI_Handler,Default_Handler

	.weak  HardFault_Handler
	.thumb_set HardFault_Handler,Default_Handler

	.weak  MemManage_Handler
	.thumb_set MemManage_Handler,Default_Handler

	.weak  BusFault_Handler
	.thumb_set BusFault_Handler,Default_Handler

	.weak  UsageFault_Handler
	.thumb_set UsageFault_Handler,Default_Handler

	.weak  DebugMon_Handler
	.thumb_set DebugMon_Handler,Default_Handler
