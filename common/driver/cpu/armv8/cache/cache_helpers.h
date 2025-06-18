/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2017, Linaro Limited
 */

#ifndef __CACHE_HELPERS_H
#define __CACHE_HELPERS_H

#include "stddef.h"

/* Cache maintenance operation type (deprecated with core_tlb_maintenance()) */
enum cache_op {
	DCACHE_CLEAN,
	DCACHE_AREA_CLEAN,
	DCACHE_INVALIDATE,
	DCACHE_AREA_INVALIDATE,
	ICACHE_INVALIDATE,
	ICACHE_AREA_INVALIDATE,
	DCACHE_CLEAN_INV,
	DCACHE_AREA_CLEAN_INV,
};

/* Data Cache set/way op type defines */
#define DCACHE_OP_INV		0x0
#define DCACHE_OP_CLEAN_INV	0x1
#define DCACHE_OP_CLEAN		0x2

#ifndef __ASSEMBLY__
void dcache_cleaninv_range(void *addr, size_t size);
void dcache_clean_range(void *addr, size_t size);
void dcache_inv_range(void *addr, size_t size);
void dcache_clean_range_pou(void *addr, size_t size);

void icache_inv_all(void);
void icache_inv_range(void *addr, size_t size);
void icache_inv_user_range(void *addr, size_t size);

void dcache_op_all(unsigned long op_type);
#endif /*!__ASSEMBLER__*/

#endif /*__CACHE_HELPERS_H*/
