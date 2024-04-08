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
#include "mmu_config_64.h"
#include "asm_macro.h"

.weak InitMultiLevelMMU

.macro  mov_32, reg, immediate_32
    MOV     \reg,   #(\immediate_32 & 0xffff)
    MOVK    \reg,   #((\immediate_32>>16) & 0xffff), lsl #16
.endm

/* L1 is first level for highest address bit( 38 <= n <= 30) for 4KB page
LEVEL1_TABLE_OFFSET:        L1.0            => 0GB-1GB
                 +8:        L1.1            => 1GB-2GB
                +16:        L1.2
                            .........
*/

.macro  build_level1_table, start_addr, end_addr
    // 1.0 prepare x0, x1
    MOV     x0, #(\start_addr/LEVEL1_BLOCK_SIZE*8 + LEVEL1_TABLE_OFFSET)
    ADD     x0, x0, x10

    MOV     x1, #(\end_addr/LEVEL1_BLOCK_SIZE*8 + LEVEL1_TABLE_OFFSET)
    ADD     x1, x1, x10

    // 1.1 prepare x2, x3
    mov_32  x3, (\start_addr/LEVEL1_BLOCK_SIZE*PAGE_SIZE + LEVEL2_TABLE_OFFSET)
    ADD     x3, x3, x10
    MOV     x2, #(PDM_AF | 0x3)
    ADD     x2, x2, x3
    ADD     x3, x2, #PAGE_SIZE

    // 1.2 loop to build level1 table
label\@.1:

    STP     x2, x3, [x0], #16
    ADD     x2, x2, #(PAGE_SIZE<<1)
    ADD     x3, x3, #(PAGE_SIZE<<1)
    CMP     x0, x1
    BLT     label\@.1
.endm

/*
LEVEL2_TABLE_OFFSET:        L2.P0.0   -  L2.P0.511      =>     0MB - 512*2MB
               +4KB:        L2.P1.0   -  L2.P1.511      => 512*2MB - 1024*2MB
               +8kB:        L2.P2.0   -  L2.P2.511
                            .........
*/

.macro  build_level2_table, start_addr, end_addr
    // 2.0 prepare x0, x1
    mov_32  x0, (\start_addr/LEVEL2_BLOCK_SIZE*8 + LEVEL2_TABLE_OFFSET)
    ADD     x0, x0, x10

    mov_32  x1, (\end_addr/LEVEL2_BLOCK_SIZE*8 + LEVEL2_TABLE_OFFSET)
    ADD     x1, x1, x10

    // 2.1 prepare x2, x3
    mov_32  x3, (\start_addr/LEVEL2_BLOCK_SIZE*PAGE_SIZE + LEVEL3_TABLE_OFFSET)
    ADD     x3, x3, x10
    MOV     x2, #(PDM_AF | 0x3)
    ADD     x2, x2, x3
    ADD     x3, x2, #PAGE_SIZE

    // 2.2 loop to build level2 table
label\@.2:
    STP     x2, x3, [x0], #16
    ADD     x2, x2, #(PAGE_SIZE<<1)
    ADD     x3, x3, #(PAGE_SIZE<<1)
    CMP     x0, x1
    BLT     label\@.2
.endm

.macro  build_level2_region, start_addr, end_addr, attr_index
    // 2.0 prepare x0, x1
    mov_32  x0, (\start_addr/LEVEL2_BLOCK_SIZE*8 + LEVEL2_TABLE_OFFSET)
    ADD     x0, x0, x10

    mov_32  x1, (\end_addr/LEVEL2_BLOCK_SIZE*8 + LEVEL2_TABLE_OFFSET)
    ADD     x1, x1, x10

    // 2.1 prepare x2, x3
    mov_32  x2, (\start_addr | PAGE_ATTRS | (\attr_index << 2) | 0x1)
    ADD     x3, x2, #LEVEL2_BLOCK_SIZE

    // 2.2 loop to build level2 table
label\@.3:
    STP     x2, x3, [x0], #16
    ADD     x2, x2, #(LEVEL2_BLOCK_SIZE<<1)
    ADD     x3, x3, #(LEVEL2_BLOCK_SIZE<<1)
    CMP     x0, x1
    BLT     label\@.3
.endm

/*
LEVEL3_TABLE_OFFSET:        L3.P0.0   -  L3.P0.511          =>     0KB -  512*4KB
               +4KB:        L3.P1.0   -  L3.P1.511          => 512*4KB - 1024*4KB
               +8kB:        L3.P2.0   -  L3.P2.511
                            .........
*/

.macro  build_level3_region, start_addr, end_addr, attr_index
    // 3.0 prepare x0, x1
    mov_32  x0, (\start_addr/PAGE_SIZE*8 + LEVEL3_TABLE_OFFSET)
    ADD     x0, x0, x10

    mov_32  x1, (\end_addr/PAGE_SIZE*8 + LEVEL3_TABLE_OFFSET)
    ADD     x1, x1, x10

    // 3.1 prepare x2, x3
    mov_32  x2, (\start_addr | PAGE_ATTRS | (\attr_index << 2) | 3)
    ADD     x3, x2, #PAGE_SIZE

    // 3.2 loop to build level3 table
label\@.3:
    STP     x2, x3, [x0], #16
    ADD     x2, x2, #(PAGE_SIZE<<1)
    ADD     x3, x3, #(PAGE_SIZE<<1)
    CMP     x0, x1
    BLT     label\@.3
.endm


/* void InitMultiLevelMMU(unsigned long long mmu_table_base) */
/* x0: p        */
/* x1: end      */
/* x2: value for entry_n        */
/* x3: value for entry_n+1      */
    .global InitMultiLevelMMU
InitMultiLevelMMU:
    MOV     x10, x0         // backup mmu_table_base

#ifndef NOCS
    jump_by_el x11, 3f, 2f, 1f
3:
#endif
    /* build level 1 */
    build_level1_table      0x00000000, 0x100000000 			// 0-4GB

    /* build level 2 */
    build_level2_region     0x00000000, 0x80000000, MT_NORMAL_C	// 0-2GB
    build_level2_region     0xF0000000, 0x100000000, MT_DEVICE_NGNRNE	// the highest 256MB in 4GB space
    RET
#ifndef NOCS
2:
    /* build level 1 */
    build_level1_table      0x00100000, 0x100000000 			// 0-4GB

    /* build level 2 */
    build_level2_table      0x00000000, 0x00200000				//0-2MB
#ifndef CONFIG_GENX_ENABLE
    build_level2_table      0x04000000, 0x04200000
    build_level2_region     0x04200000, 0x80000000, MT_NORMAL_C
#else
    build_level2_table      0x04000000, 0x04200000
    /* Jump over 0x04A00000-0x06A00000 NNA memory for dsp */
    build_level2_region     0x04200000, 0x04800000, MT_NORMAL_C
    build_level2_region     0x06A00000, 0x80000000, MT_NORMAL_C
#endif
    build_level2_region     0xF0000000, 0x100000000, MT_DEVICE_NGNRNE	// the highest 256MB in 4GB space

    /* build level 3 */
    /* FIXME: Jump over 0x1000 to work around loading dtbo abort */
    build_level3_region     0x00001000, 0x00100000, MT_NORMAL_C
    /* Jump over 0x00100000-0x04100000 NNA memory for tz */
    build_level3_region     0x04100000, 0x04200000, MT_NORMAL_C
    RET
1:
    //build_level3_region     0x00000000,  0x00800000, MT_NORMAL_C        // Cacheable buffer for test
    //build_level3_region     0x00800000,  0x03800000, MT_NORMAL_NC       //
    //build_level3_region     0x03800000,  0x04000000, MT_NORMAL_C        // MMU table, stack
    //build_level3_region     0x04000000,  0x0F000000, MT_NORMAL_NC       //
    //build_level3_region     0x0F000000,  0x0FE00000, MT_NORMAL_C        // code/data
    //build_level3_region     0x0FE00000,  0x22000000, MT_NORMAL_NC       //
    //build_level3_region     0x22000000,  0x34000000, MT_NORMAL_C        // heap
    //build_level3_region     0x34000000,  0x80000000, MT_NORMAL_NC        //
    RET
#endif

/* enable_mmu() */
	.global enable_mmu
enable_mmu:
	jump_by_el x11, 3f, 2f, 1f
3:
	LDR	x0, =__ttb_base_start
	MSR	TTBR0_EL3, x0
	LDR	x0, =TCR_VALUE
	MSR	TCR_EL3, x0
	LDR	x0, =MAIR_VALUE
	MSR	MAIR_EL3, x0

	IC	ialluis // invalidate ICACHE
	DSB	ish
	ISB

	MRS	x0, SCTLR_EL3
	ORR	x0, x0, #0x00000001	// enable MMU
	ORR	x0, x0, #0x00000004	// enable DCACHE
	ORR	x0, x0, #0x00001000	// enable ICACHE
	MSR	SCTLR_EL3, x0
	ISB
	RET
2:
	LDR 	x0, =__ttb_base_start
	MSR 	TTBR0_EL2, x0
	LDR 	x0, =TCR_VALUE
	MSR 	TCR_EL2, x0
	LDR 	x0, =MAIR_VALUE
	MSR 	MAIR_EL2, x0

	IC	ialluis // invalidate ICACHE
	DSB	ish
	ISB

	MRS 	x0, SCTLR_EL2				// reset value 0x00c52838
	ORR 	x0, x0, #0x00000001 		// enable MMU
	ORR 	x0, x0, #0x00000004 		// enable D cache
	ORR 	x0, x0, #0x00001000 		// enable I cache
	MSR 	SCTLR_EL2, x0
	ISB
	RET

1:
	RET


/* int set_mmu_ns_bit(void* mmu_table_base, void* address, unsigned int size, int ns) */
/* x0: mmu_table    */
/* x1: address      */
/* x2: size         */
/* x3: ns           */
    .global _set_mmu_ns_bit
_set_mmu_ns_bit:
    ADD     x2, x1, x2
    ADD     x1, x0, x1, LSR #(BITS_PAGE_SIZE-3)         // addr/page_size*8
    ADD     x2, x0, x2, LSR #(BITS_PAGE_SIZE-3)         // addr/page_size*8

    mov_32  x0, LEVEL3_TABLE_OFFSET
    ADD     x1, x1, x0
    ADD     x2, x2, x0

    CBNZ    x3, set_ns
clear_ns:
    LDR     x0, [x1]
    BIC     x0, x0, #(1<<5)
    STR     x0, [x1], #8
    CMP     x1, x2
    BLT     clear_ns
    B       set_mmu_ns_bit_exit
set_ns:
    LDR     x0, [x1]
    ORR     x0, x0, #(1<<5)
    STR     x0, [x1], #8
    CMP     x1, x2
    BLT     set_ns
set_mmu_ns_bit_exit:
    MOV     x0, #0
    RET

// ------------------------------------------------------------
// TLB
// ------------------------------------------------------------
// void invalidate_tlb_all(void)
  .global invalidate_tlb_all
  .global invalidate_unified_tlb_is
invalidate_unified_tlb_is:
invalidate_tlb_all:
  jump_by_el x11, 3f, 2f, 1f
3:
  TLBI	ALLE3
  DSB	sy
  ISB
  RET
2:
  TLBI	ALLE2
  DSB	sy
  ISB
  RET
1:
  RET
