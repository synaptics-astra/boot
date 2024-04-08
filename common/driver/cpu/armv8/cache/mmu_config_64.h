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
#ifndef _MMU_CONFIG_H_
#define _MMU_CONFIG_H_

// MMU table layout (AArch64)
// 0x000000 - 0x001000          Level 1
// 0x001000 - 0x011000          Level 2
// 0x011000 - +size_level3      Level 3           size_level3 = MAX_DDR_SIZE/4KB*8
// For BG4CT, DDR is 2GB by defualt, so the MMU table is 0x11000+4MB = 0x411000
// For BG4CD, DDR is 1GB by defualt, so the MMU table is 0x11000+2MB = 0x211000


// For BG4CT, the max address space is 8GB. 4-8GB is alias to 0-4GB DDR. 3.5G-4GB is register space. 
#define PAGE_SIZE           0x1000
#define BITS_PAGE_SIZE      12


#define LEVEL1_BLOCK_SIZE           (1024*1024*1024)            // 1GB
#define LEVEL2_BLOCK_SIZE              (2*1024*1024)            // 2MB
#define LEVEL3_PAGE_SIZE                    (4*1024)            // 4KB

// note, following space allocation is not tightened. Use aligned address only for easy caculation
#define LEVEL1_TABLE_OFFSET                     0           // mmu start at level 1 for 38bit address. only one table (4KB) for level1
#define LEVEL2_TABLE_OFFSET                0x1000           // 64KB, (max 16 L2 tables, size:512MB*16=8GB)
#define LEVEL3_TABLE_OFFSET               0x11000           // each table is 4KB for 2MB memory, etc, 4MB for 2GB.

#define MAX_DDR_SIZE                         0x20000000
//#define MAX_DDR_SIZE                         0x80000000     // 2GB, diag will create fixed linear mapping for basic ddr region on initialization
                                                            // for others region, it can create on the fly.
#define MAX_MAPPING_SIZE_LEVEL1_LEVEL2      0x200000000     // 8GB


/*
 * Memory types
 */
#define MT_DEVICE_NGNRNE    0               // Device non-Gathering, non-Reordering, No Early write acknowledgement. (strongly-ordered)
#define MT_DEVICE_NGNRE     1               // Device non-Gathering, non-Reordering, Early Write Acknowledgement. (Device type)
#define MT_NORMAL_NC        2               // Normal Memory, un-cacheable
#define MT_NORMAL_C         3               // Normal Memory, cacheable

#define MT_DEVICE_NGNRNE_ATTR    0x00
#define MT_DEVICE_NGNRE_ATTR     0x04
#define MT_NORMAL_NC_ATTR        0x44
#define MT_NORMAL_C_ATTR         0xff


#define MAIR_VALUE         ((MT_DEVICE_NGNRNE_ATTR << (MT_DEVICE_NGNRNE*8)) |\
                            (MT_DEVICE_NGNRE_ATTR << (MT_DEVICE_NGNRE*8))   |\
                            (MT_NORMAL_NC_ATTR << (MT_NORMAL_NC*8))         |\
                            (MT_NORMAL_C_ATTR << (MT_NORMAL_C*8)))
                            

/*
 * TCR flags.
 */
#define VA_BITS            (36)        /* 34 bits virtual address */
#define TCR_T0SZ(x)         ((64 - (x)) << 0)
#define TCR_IRGN_NC         (0 << 8)
#define TCR_IRGN_WBWA       (1 << 8)
#define TCR_IRGN_WT         (2 << 8)
#define TCR_IRGN_WBNWA      (3 << 8)
#define TCR_IRGN_MASK       (3 << 8)
#define TCR_ORGN_NC         (0 << 10)
#define TCR_ORGN_WBWA       (1 << 10)
#define TCR_ORGN_WT         (2 << 10)
#define TCR_ORGN_WBNWA      (3 << 10)
#define TCR_ORGN_MASK       (3 << 10)
#define TCR_SHARED_NON      (0 << 12)
#define TCR_SHARED_OUTER    (1 << 12)
#define TCR_SHARED_INNER    (2 << 12)
#define TCR_TG0_4K          (0 << 14)
#define TCR_TG0_64K         (1 << 14)
#define TCR_TG0_16K         (2 << 14)
#define TCR_EL3_IPS_BITS    (1 << 16)    /* 36 bits physical address */
#define TCR_TBI_IGNORED     (1 << 20)    /* Top Byte ignored */

/* PTWs cacheable, inner/outer WBWA and non-shareable */
#define TCR_VALUE          (TCR_TG0_4K          |\
                            TCR_SHARED_NON      |\
                            TCR_ORGN_WBWA       |\
                            TCR_IRGN_WBWA       |\
                            TCR_T0SZ(VA_BITS)   |\
                            TCR_TBI_IGNORED     |\
                            TCR_EL3_IPS_BITS)

// TTBCR for AArch32, EAE = 1
#define TTBCR_VALUE        ((1<<31)             |\
                            TCR_SHARED_NON      |\
                            TCR_ORGN_WBWA       |\
                            TCR_IRGN_WBWA)

/*
 * PAGE description attribute
 */
#define PDM_NS              (0 << 5) 
#define PDM_AP              (1 << 6)        // read/write for EL0 and EL1
#define PDM_S               (3 << 8)
#define PDM_AF              (1 << 10)
#define PDM_NG              (0 << 11) // (1 << 11)
#define PDM_PXN             ((unsigned long long)(0) << 53)
#define PDM_UXN             ((unsigned long long)(0) << 54)
//#define PDM_ATTRS           (PDM_UXN|PDM_PXN|PDM_NG|PDM_AF|PDM_S|PDM_AP|PDM_NS)
#define PDM_ATTRS           (PDM_NS|PDM_S|PDM_AF)

/*
 * PAGE attributes
 */
#define PAGE_NS             (0 << 5) 
#define PAGE_AP             (1 << 6)        // read/write for EL0 and EL1
#define PAGE_S              (3 << 8)
#define PAGE_AF             (1 << 10)
//#define PAGE_ATTRS          (PAGE_AF|PAGE_S|PAGE_AP|PAGE_NS)
#define PAGE_ATTRS          (PAGE_NS|PAGE_S|PDM_AF)

#endif // _MMU_CONFIG_H_