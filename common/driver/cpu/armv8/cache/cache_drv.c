/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright (c) 2013-2020 Synaptics Incorporated. All rights reserved.
 * Copyright (c) 2016, Linaro Limited
 * Copyright (c) 2014, STMicroelectronics International N.V.
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
#include "cache_helpers.h"
#include "cache.h"
#include "arch_sys_reg_op.h"

extern int __ttb_base_start;
extern void enable_mmu(void);
extern void invalidate_tlb_all(void);
extern void InitMultiLevelMMU(unsigned long long mmu_table_base);

/** Invalidate the whole D-cache.
 */
void invalidate_dcache_all(void)
{
	dcache_op_all(DCACHE_OP_INV);
}

/** Clean & Invalidate the whole D-cache.
 */
void flush_dcache_all(void)
{
	dcache_op_all(DCACHE_OP_CLEAN_INV);
}

void flush_dcache_range(void *start, void *end)
{
	size_t len;
	len = end - start;
	dcache_cleaninv_range(start, len);
}

void invalidate_dcache_range(void *start, void *end)
{
	size_t len;
	len = end - start;
	dcache_inv_range(start, len);
}

void __attribute__((optimize("O0"))) enable_caches(void)
{
	/* enable I, C and M together */
	if (!(get_sctlr_reg() & SCTLR_M_BIT)) {
		dcache_op_all(DCACHE_OP_INV);
		invalidate_tlb_all();
		InitMultiLevelMMU((uintptr_t)(&__ttb_base_start));
		enable_mmu(); // enables mmu, icache and dcache
	}
}

void __attribute__((optimize("O0"))) disable_caches(void)
{
	uint64_t sctlr;

	sctlr = get_sctlr_reg();

	/* if cache isn't enabled no need to disable */
	if (!(sctlr & SCTLR_M_BIT))
		return;

	set_sctlr_reg(sctlr & ~(SCTLR_C_BIT|SCTLR_M_BIT|SCTLR_I_BIT));

	dcache_op_all(DCACHE_OP_CLEAN_INV);

	invalidate_tlb_all();
}

void __attribute__((optimize("O0"))) enable_icache(void)
{
	icache_inv_all();
	set_sctlr_reg(get_sctlr_reg() | SCTLR_I_BIT);
}

