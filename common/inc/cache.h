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
#ifndef _CACHE_H_
#define _CACHE_H_

#ifndef __ASSEMBLY__

void enable_caches(void);
void disable_caches(void);

/** Invalidate the whole I-cache.
 */
void invalidate_icache_all(void);

/** Invalidate the whole D-cache.
 */
void invalidate_dcache_all(void);

/**  Clean the whole D-cache.
 */
void clean_dcache_all(void);

/** Clean & Invalidate the whole D-cache.
 */
void flush_dcache_all(void);

/** Flush the entire cache system.
 *
 *  The data cache flush is now achieved using atomic clean / invalidates
 *  working outwards from L1 cache. This is done using Set/Way based cache
 *  maintenance instructions.
 *  The instruction cache can still be invalidated back to the point of
 *  unification in a single instruction.
 */
void flush_cache_all(void);
void invalidate_cache_all(void);

/** invalidate dcache by range.
 *
 * @param start		virtual start address of region
 * @param end		virtual end address of region
 */
void invalidate_dcache_range(void *start, void *end);

/** clean dcache by range.
 *
 * @param start		virtual start address of region
 * @param end		virtual end address of region
 */
void clean_dcache_range(void *start, void *end);

/** clean & invalidate dcache by range.
 *
 * Ensure that the data held in the page addr is written back
 * to the page in question.
 * @param start		virtual start address of region
 * @param end		virtual end address of region
 */
void flush_dcache_range(void *start, void *end);
void invalidate_unified_tlb(void);
void enable_branch_prediction(void);
void disable_branch_prediction(void);
void flush_branch_target_cache(void);
void disable_mmu(void);

#endif /* __ASSEMBLY__ */

#endif /* _CACHE_H_ */
