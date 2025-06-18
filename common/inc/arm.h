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
#ifndef _ARM_H_
#define _ARM_H_

/* ARM exception number define. should be identical with hook type */
#define ARM_EXCEPTION_UDF	0x0
#define ARM_EXCEPTION_SWI	0x1
#define ARM_EXCEPTION_PRF	0x2
#define ARM_EXCEPTION_ABT	0x3
#define ARM_EXCEPTION_IRQ	0x4
#define ARM_EXCEPTION_FIQ	0x5
#define ARM_EXCEPTION_NUM	6

/*
 * PSR bit definition
 */
#define PSR_USR_MODE		0x00000010
#define PSR_FIQ_MODE		0x00000011
#define PSR_IRQ_MODE		0x00000012
#define PSR_SVC_MODE		0x00000013
#define PSR_MON_MODE		0x00000016
#define PSR_ABT_MODE		0x00000017
#define PSR_UND_MODE		0x0000001b
#define PSR_SYS_MODE		0x0000001f
#define PSR_MODE_MASK		0x0000001f

#define PSR_T_BIT		0x00000020
#define PSR_F_BIT		0x00000040
#define PSR_I_BIT		0x00000080
#define PSR_A_BIT		0x00000100
#define PSR_E_BIT		0x00000200
#define PSR_J_BIT		0x01000000
#define PSR_Q_BIT		0x08000000
#define PSR_V_BIT		0x10000000
#define PSR_C_BIT		0x20000000
#define PSR_Z_BIT		0x40000000
#define PSR_N_BIT		0x80000000

#define PSR_IRQ_DISABLE		PSR_I_BIT
#define PSR_FIQ_DISABLE		PSR_F_BIT
#define PSR_THUMB_ENABLE	PSR_T_BIT

/* CP15 System Control Register (SCTLR) bit definition */
#define SCTLR_M_BIT		(1 << 0)
#define SCTLR_A_BIT		(1 << 1)
#define SCTLR_C_BIT		(1 << 2)
#define SCTLR_W_BIT		(1 << 3)
#define SCTLR_SW_BIT		(1 << 10)
#define SCTLR_BPU_BIT		(1 << 11)
#define SCTLR_I_BIT		(1 << 12)
#define SCTLR_V_BIT		(1 << 13)

#define SCTLR_MMU_ENABLE		SCTLR_M_BIT
#define SCTLR_ALIGN_CHECK_ENABLE	SCTLR_A_BIT
#define SCTLR_DCACHE_ENABLE		SCTLR_C_BIT
#define SCTLR_BPU_ENABLE		SCTLR_BPU_BIT
#define SCTLR_ICACHE_ENABLE		SCTLR_I_BIT
#define SCTLR_VECTOR_HIGH		SCTLR_V_BIT

/* Secure Control Register (SCR) bit definition */
#define SCR_NS_BIT		0x1	/* 0: secure; 1: non-secure */
#define SCR_IRQ_BIT		0x2	/* 1: IRQ trap to Monitor */
#define SCR_FIQ_BIT		0x4	/* 1: FIQ trap to Monitor */
#define SCR_EA_BIT		0x8	/* 1: Abort trap to Monitor */
#define SCR_FW_BIT		0x10	/* 1: F bit writable in non-secure */
#define SCR_AW_BIT		0x20	/* 1: A bit writable in non-secure*/

#ifndef ENTRY
#define ENTRY(name) \
.globl name; \
name:
#endif

#ifndef ENDPROC
#define ENDPROC(name) \
.type name, %function; \
.size name, .-name
#endif

#ifndef __KERNEL__
#define isb()			__asm__ __volatile__ ("isb" : : : "memory")
#define dsb()			__asm__ __volatile__ ("dsb" : : : "memory")
#define dmb()			__asm__ __volatile__ ("dmb" : : : "memory")
#define wfi()			__asm__ __volatile__ ("wfi")
#define wfe()			__asm__ __volatile__ ("wfe")

#define barrier()		__asm__ __volatile__ ("": : :"memory")
#define cmb()			barrier()

/**
 * NOP must be encoded as 'MOV r0,r0' in ARM code and 'MOV r8,r8'
 * in Thumb code, see ARMv7-A/R ARM C.2
 *
 */
#define nop()			__asm__ __volatile__("mov\tr0,r0\t@ nop\n\t")

#endif /* _KERNEL__ */

#ifndef __ASSEMBLY__
/*unsigned int __cpsr(void);
unsigned int __spsr(void);
void mmu_set_ttb(void *ttb);
void mmu_enable(void);
void mmu_disable(void);

void arch_local_irq_enable(void);
unsigned long arch_local_irq_disable(void);
void arch_local_fiq_enable(void);
unsigned long arch_local_fiq_disable(void);
void arch_local_int_enable(void);
unsigned long arch_local_int_disable(void);
void arch_local_restore_flags(unsigned long flags);
*/
#endif /* __ASSEMBLY__ */

#endif /* _ARM_H_ */
