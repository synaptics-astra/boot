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

/** SMC Calling Conversion - Argument passing
 *             Document number: ARM DEN 0028A
 *
 * http://infocenter.arm.com/help/topic/com.arm.doc.den0028a/index.html
 *
 * [SMC32 Argument passing]
 *
 * When the SMC32 convention is used, the SMC intructions take up to seven
 * 32-bit arguments in register and can return up to four 32-bit values in
 * registers.
 *
 * When an SMC32 call is made from AArch32:
 * o Arguments are passed in register R0-R6
 * o Results are returned in R0-R3
 * o Registers R4-R14 are callee-saved and must be preserved over the SMC call
 *
 * When an SMC32 call is made from AArch64:
 * o Arguments are passed in register X0-X6
 * o Results are returned in X0-X3
 * o Registers X18-X30 and stack pointers SP_EL0 and SP_ELx are callee-saved and
 *   must be preserved over the SMC call
 *
 * [SMC64 Argument passing]
 *
 * When the SMC64 convention is used, the SMC instructions take up to seven 64-bit
 * arguments in registers and can return up to four 64-bit values in register
 *
 * When an SMC64 call is made from AArch64:
 * o Arguments are passed in register X0-X6
 * o Results are returned in X0-X3
 * o Registers X18-X30 and stack pointers SP_EL0 and SP_ELx are callee-saved and
 *   must ber preserved over the SMC call
 *
 * Then calling convention cannot be used by code executing AArch32 state
 */

/** SMC call command with 2 parameters
 *
 * unsigned long __smc_cmd2(unsigned long func_id,
 *                          unsigned long param0,
 *                          unsigned long param1,
 *                          unsigned long result[2])
 *
 * @param func_id	function ID in x0
 * @param param0	parameters 0 in x1
 * @param param1	parameters 1 in x2
 * @param result	pointer to returned results (x1-x2) in x3 if not NULL
 *
 * @retval 0xffffffff   invalidate function ID
 * @retval others       status
 *
 */
	.globl __smc_cmd2
__smc_cmd2:
	/* ----------------------------------------------------------
	 * x30 = link address, x29 = frame pointer
	 *
	 * Save in the stack, stack pointer MUST be quadword aligned
	 * that is it must be aligned to 16 bytes. Misalignment
	 * generates a Stack Alighment fault
	 * ----------------------------------------------------------
	 */
	stp	x29, x30, [sp, #-32]!
	mov	x29, sp

	/* ----------------------------------------------------------
	 * x0 = function ID, x1 = param0, x2 = param1
	 *
	 * preserve x3 and x19 on the stack
	 *   x3 contains the pointer to returned value
	 *   x19 as a temporary register
	 * ----------------------------------------------------------
	 */
	stp	x3, x19, [sp, #16]

	/* ----------------------------------------------------------
	 * make smc call now
	 * ----------------------------------------------------------
	 */
	smc	#0

	/* ---------------------------------------------------------
	 * retrieve the pointer of returned value previously saved
	 * on the stack
	 * ---------------------------------------------------------
	 */
	ldr	x19, [sp, #16]

	/* ---------------------------------------------------------
	 * x0-x3 contains the returned value
	 *
	 * If the pointer is not NULL saved x1-x2 to result[2]
	 * ---------------------------------------------------------
	 */
	cmp	x19, #0
	b.eq	1f
	stp	x1, x2, [x19]
1:
	/* ----------------------------------------------------------
	 * restore x19 from stack
	 * ----------------------------------------------------------
	 */
	ldr	x19, [sp, #24]

	/* ----------------------------------------------------------
	 * x30 = link address, x29 = frame pointer
	 *
	 * restore link address register & frame pointer register
	 * ----------------------------------------------------------
	 */
	ldp	x29, x30, [sp], #32

	/* ---------------------------------------------------------
	 * back to caller
	 * ---------------------------------------------------------
	 */
	ret

/** SMC call command with 3 parameters.
 *
 *  unsigned long __smc_cmd3(unsigned long func_id,
 *                           unsigned long param0,
 *                           unsigned long param1,
 *                           unsigned long param2,
 *                           unsigned long result[3]);
 *
 *  unsigned long __smc3(unsigned long func_id,
 *                       unsigned long param0,
 *                       unsigned long param1,
 *                       unsigned long param2,
 *                       unsigned long result[3]);
 *
 * @param func_id	function ID in x0
 * @param param0	parameter 0 in x1
 * @param param1	parameter 1 in x2
 * @param param2	parameter 2 in x3
 * @param result	pointer to return results (x1-x3) in x4 if not NULL
 *
 * @retval 0xffffffff	invalidate Function ID.
 * @retval others	status
 */
	.globl __smc_cmd3
__smc_cmd3:
	/* ----------------------------------------------------------
	 * x30 = link address, x29 = frame pointer
	 *
	 * Save in the stack, stack pointer MUST be quadword aligned
	 * that is it must be aligned to 16 bytes. Misalignment
	 * generates a Stack Alighment fault
	 * ----------------------------------------------------------
	 */
	stp	x29, x30, [sp, #-32]!
	mov	x29, sp

	/* ----------------------------------------------------------
	 * x0 = function ID, x1 = param0, x2 = param1, x3 = param2
	 *
	 * preserve x4 and x19 on the stack
	 *   x4 contains the pointer to returned value
	 *   x19 as a temporary register
	 * ----------------------------------------------------------
	 */
	stp	x4, x19, [sp, #16]

	/* ----------------------------------------------------------
	 * make smc call now
	 * ----------------------------------------------------------
	 */
	smc	#0

	/* ---------------------------------------------------------
	 * retrieve the pointer of returned value previously saved
	 * on the stack
	 * ---------------------------------------------------------
	 */
	ldr	x19, [sp, #16]

	/* ---------------------------------------------------------
	 * x0-x3 contains the returned value
	 *
	 * If the pointer is not NULL saved x1-x3 to result[3]
	 * ---------------------------------------------------------
	 */
	cmp	x19, #0
	b.eq	1f
	stp	x1, x2, [x19]
	str	x3, [x19, #16]
1:
	/* ----------------------------------------------------------
	 * restore x19 from stack
	 * ----------------------------------------------------------
	 */
	ldr	x19, [sp, #24]

	/* ----------------------------------------------------------
	 * x30 = link address, x29 = frame pointer
	 *
	 * restore link address register & frame pointer register
	 * ----------------------------------------------------------
	 */
	ldp	x29, x30, [sp], #32

	/* ---------------------------------------------------------
	 * back to caller
	 * ---------------------------------------------------------
	 */
	ret

/** SMC call without results.
 *
 * unsigned long __smc(unsigned long func_id, ...);
 *
 * @note it can be used as bellow:
 *       status = smc_nores(func_id);
 *       status = smc_nores(func_id, param0);
 *       status = smc_nores(func_id, param0, param1);
 *       status = smc_nores(func_id, param0, param1, param2);
 *
 * @param func_id	function ID in x0
 * @param param0	parameter 0 in x1
 * @param param1	parameter 1 in x2
 * @param param2	parameter 2 in x3
 *
 * @retval 0xffffffff	invalidate Function ID.
 * @retval others	status
 */
	.globl __smc
__smc:
	/* ----------------------------------------------------------
	 * x30 = link address, x29 = frame pointer
	 *
	 * Save in the stack, stack pointer MUST be quadword aligned
	 * that is it must be aligned to 16 bytes. Misalignment
	 * generates a Stack Alighment fault
	 * ----------------------------------------------------------
	 */
	stp	x29, x30, [sp, #-16]!
	mov	x29, sp

	/* ----------------------------------------------------------
	 * make smc call now
	 * ----------------------------------------------------------
	 */
	smc	#0

	/* ----------------------------------------------------------
	 * x30 = link address, x29 = frame pointer
	 *
	 * restore link address register & frame pointer register
	 * ----------------------------------------------------------
	 */
	ldp	x29, x30, [sp], #16

	/* ---------------------------------------------------------
	 * back to caller
	 * ---------------------------------------------------------
	 */
	ret

/** SMC call command with 3 parameters.
 *
 *  unsigned long __smc3(unsigned long func_id,
 *                       unsigned long param0,
 *                       unsigned long param1,
 *                       unsigned long param2,
 *                       unsigned long result[4]);
 *
 * @param func_id	function ID in x0
 * @param param0	parameter 0 in x1
 * @param param1	parameter 1 in x2
 * @param param2	parameter 2 in x3
 * @param result	pointer to return results (x0-x3) in x4 if not NULL
 *
 * @retval 0xffffffff	invalidate Function ID.
 * @retval others	status
 */
	.globl __smc3
__smc3:
	/* ----------------------------------------------------------
	 * x30 = link address, x29 = frame pointer
	 *
	 * Save in the stack, stack pointer MUST be quadword aligned
	 * that is it must be aligned to 16 bytes. Misalignment
	 * generates a Stack Alighment fault
	 * ----------------------------------------------------------
	 */
	stp	x29, x30, [sp, #-32]!
	mov	x29, sp

	/* ----------------------------------------------------------
	 * x0 = function ID, x1 = param0, x2 = param1, x3 = param2
	 *
	 * preserve x4 and x19 on the stack
	 *   x4 contains the pointer to returned value
	 *   x19 as a temporary register
	 * ----------------------------------------------------------
	 */
	stp	x4, x19, [sp, #16]

	/* ----------------------------------------------------------
	 * make smc call now
	 * ----------------------------------------------------------
	 */
	smc	#0

	/* ---------------------------------------------------------
	 * retrieve the pointer of returned value previously saved
	 * on the stack
	 * ---------------------------------------------------------
	 */
	ldr	x19, [sp, #16]

	/* ---------------------------------------------------------
	 * x0-x3 contains the returned value
	 *
	 * If the pointer is not NULL saved x0-x3 to result[4]
	 * ---------------------------------------------------------
	 */
	cmp	x19, #0
	b.eq	1f
	stp	x0, x1, [x19]
	stp	x2, x3, [x19, #16]
1:
	/* ----------------------------------------------------------
	 * restore x19 from stack
	 * ----------------------------------------------------------
	 */
	ldr	x19, [sp, #24]

	/* ----------------------------------------------------------
	 * x30 = link address, x29 = frame pointer
	 *
	 * restore link address register & frame pointer register
	 * ----------------------------------------------------------
	 */
	ldp	x29, x30, [sp], #32

	/* ---------------------------------------------------------
	 * back to caller
	 * ---------------------------------------------------------
	 */
	ret

/** SMC call with 6 parameters.
 *
 * unsigned long __smc6(unsigned long func_id,
 *                      const unsigned long param[6],
 *                      unsigned long result[4]);
 *
 * @param func_id	function ID in x0
 * @param param		buffer to store 6 parmeters in r1
 * @param result	buffer to return results (x0-x3) in r2 if not NULL
 *			result[0] is returned by function too
 *
 * @retval 0xffffffff	invalidate Function ID.
 * @retval others	status
 */
	.globl __smc6
__smc6:
	/* ----------------------------------------------------------
	 * x30 = link address, x29 = frame pointer
	 *
	 * Save in the stack, stack pointer MUST be quadword aligned
	 * that is it must be aligned to 16 bytes. Misalignment
	 * generates a Stack Alighment fault
	 * ----------------------------------------------------------
	 */
	stp	x29, x30, [sp, #-32]!
	mov	x29, sp

	/* ----------------------------------------------------------
	 * x0 = function ID
	 * x1 = pointer to 6 parameters (x1-x6)
	 * x2 = pointer to return results (x0-x3)
	 *
	 * preserve x2 and x19 on the stack
	 *   x2 contains the pointer to the returned value
	 *   x19 as a temporary register
	 * ----------------------------------------------------------
	 */
	stp	x2, x19, [sp, #16]

	/* ----------------------------------------------------------
	 * Load 6 parameters into r1-r6, will panic if r1=NULL
	 *
	 * x19 = ip0, first intra-procedure-call scratch register
	 * ----------------------------------------------------------
	 */
	mov	x19, x1
	ldp	x1, x2, [x19]
	ldp	x3, x4, [x19, #16]
	ldp	x5, x6, [x19, #32]

	/* ----------------------------------------------------------
	 * make smc call now
	 * ----------------------------------------------------------
	 */
	smc	#0

	/* ---------------------------------------------------------
	 * retrieve the pointer of returned value previously saved
	 * on the stack
	 * ---------------------------------------------------------
	 */
	ldr	x19, [sp, #16]

	/* ---------------------------------------------------------
	 * x0-x3 contains the returned value
	 *
	 * If the pointer is not NULL saved x1-x3 to result[4]
	 * ---------------------------------------------------------
	 */
	cmp	x19, #0
	b.eq	1f
	stp	x0, x1, [x19]
	stp	x2, x3, [x19, #16]
1:
	/* ----------------------------------------------------------
	 * restore x19 from stack
	 * ----------------------------------------------------------
	 */
	ldr	x19, [sp, #24]

	/* ----------------------------------------------------------
	 * x30 = link address, x29 = frame pointer
	 *
	 * restore link address register & frame pointer register
	 * ----------------------------------------------------------
	 */
	ldp	x29, x30, [sp], #32

	/* ---------------------------------------------------------
	 * back to caller
	 * ---------------------------------------------------------
	 */
	ret
