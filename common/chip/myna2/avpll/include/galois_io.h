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

#ifndef    __GALOIS_IO_H__
#define    __GALOIS_IO_H__

#include "galois_type.h"
#ifndef VPP_IN_TRUST_ZONE
#include "galois_dev_mem_map.h"
#endif
#include "Galois_memmap.h"
#include "galois_common.h"


#ifdef __MEMIO_PCIE
extern MV_U32                       gu32_galois_pcie_ioremap_base;
#endif

#define    CPU_PHY_MEM(x)                       ((MV_U32)(x))
#define    CPU_MEMIO_CACHED_ADDR(x)             ((void*)(x))
#define    CPU_MEMIO_UNCACHED_ADDR(x)           ((void*)(x))
/*!
 * CPU endian check
 */
#if defined( CPU_BIG_ENDIAN ) && defined( __LITTLE_ENDIAN )
#error CPU endian conflict!!!
#elif defined(__BYTE_ORDER)
#if __BYTE_ORDER == __BIG_ENDIAN
#error CPU endian conflict!!!
#endif
#elif defined( __BIG_ENDIAN )
#error CPU endian conflict!!!
#endif

/*!
 * CPU architecture dependent 32, 16, 8 bit read/write IO addresses
 */
#define    MV_MEMIO32_WRITE(addr, data)         ((*((volatile unsigned int *)devmem_phy_to_virt(addr))) = ((unsigned int)(data)))
#define    MV_MEMIO32_READ(addr)                ((*((volatile unsigned int *)devmem_phy_to_virt(addr))))
#define    MV_MEMIO16_WRITE(addr, data)         ((*((volatile unsigned short *)devmem_phy_to_virt(addr))) = ((unsigned short)(data)))
#define    MV_MEMIO16_READ(addr)                ((*((volatile unsigned short *)devmem_phy_to_virt(addr))))
#define    MV_MEMIO08_WRITE(addr, data)         ((*((volatile unsigned char *)devmem_phy_to_virt(addr))) = ((unsigned char)(data)))
#define    MV_MEMIO08_READ(addr)                ((*((volatile unsigned char *)devmem_phy_to_virt(addr))))


/*!
 * No Fast Swap implementation (in assembler) for ARM
 */
#define    MV_32BIT_LE_FAST(val)                MV_32BIT_LE(val)
#define    MV_16BIT_LE_FAST(val)                MV_16BIT_LE(val)
#define    MV_32BIT_BE_FAST(val)                MV_32BIT_BE(val)
#define    MV_16BIT_BE_FAST(val)                MV_16BIT_BE(val)

/*!
 * 32 and 16 bit read/write in big/little endian mode
 */

/*!
 * 16bit write in little endian mode
 */
#define    MV_MEMIO_LE16_WRITE(addr, data)      MV_MEMIO16_WRITE(addr, MV_16BIT_LE_FAST(data))

/*!
 * 16bit read in little endian mode
 */
#define    MV_MEMIO_LE16_READ(addr)             MV_16BIT_LE_FAST((MV_U16)(MV_MEMIO16_READ((MV_U32)(addr))))

/*!
 * 32bit write in little endian mode
 */
#define    MV_MEMIO_LE32_WRITE(addr, data)      MV_MEMIO32_WRITE(addr, MV_32BIT_LE_FAST(data))

/*!
 * 32bit read in little endian mode
 */
#define    MV_MEMIO_LE32_READ(addr)             MV_32BIT_LE_FAST((MV_U32)(MV_MEMIO32_READ((MV_U32)(addr))))

/*!
 * Generate 32bit mask
 */
#define    GA_REG_MASK(bits, l_shift)           ((bits) ? (((bits) < 32) ? (((1uL << (bits)) - 1) << (l_shift)) : (0xFFFFFFFFuL << (l_shift))) : 0)

/*!
 * Galois's register address translate
 */
#if defined(VPP_IN_TRUST_ZONE)
#if	(defined(__MEMIO_DIRECT))
#ifndef INTER_REGS_BASE
#define INTER_REGS_BASE             0
#endif

#define REG_ADDR(offset)            ((MV_U32)(INTER_REGS_BASE | (offset)))

#elif (defined(__MEMIO_PCIE))
/*!
 * After ioremap the value in BAR1, got IO base address. value in BAR0 is for PCI-e core, BAR1 is for galois
 */
#define REG_ADDR(offset)            ((MV_U32)(gu32_galois_pcie_ioremap_base + (offset)))

#else
#define REG_ADDR(offset)            ((MV_U32)(offset))

#endif
#else
#    ifndef    INTER_REGS_BASE
#    define    INTER_REGS_BASE                  0
#    endif
#endif

#    define    REG_ADDR(offset)                 ((MV_U32)(INTER_REGS_BASE | (offset)))

/*!
 * Galois controller register read/write macros
 *
 * offset -- address offset (32bits)
 * holder -- pointer to the variable that will be used to store the data being read in.
 * val -- variable contains the data that will be written out.
 * bitMask -- variable contains the data (32bits) that will be written out.
 * clearMask -- variable contains the mask (32bits) that will be used to clear the corresponding bits.
 *
 * GA_REG_WORD32_READ(offset, holder) -- Read a Double-Word (32bits) from 'offset' to 'holder'
 * GA_REG_WORD16_READ(offset, holder) -- Read a Word (16bits) from 'offset' to 'holder'
 * GA_REG_BYTE_READ(offset, holder) -- Read a Byte (8bits) from 'offset' to 'holder'
 *
 * GA_REG_WORD32_WRITE(offset, val) -- Write a Double-Word (32bits) to 'offset'
 * GA_REG_WORD16_WRITE(offset, val) -- Write a Word (16bits) to 'offset'
 * GA_REG_BYTE_WIRTE(offset, val) -- Write a Byte (8bits) to 'offset'
 *
 * GA_REG_WORD32_BIT_SET(offset, bitMask) -- Set bits to '1b' at 'offset', 'bitMask' should only be used to set '1b' for corresponding bits.
 * GA_REG_WORD32_BITS_SET(offset, clearMask, val) -- Clear the bits to zero for the bits in clearMask are '1b' and write 'val' to 'offset'.
 * GA_REG_WORD32_BIT_CLEAR(offset, clearMask) -- Clear the bits to zero for the bits in bitMask are '1b'
 *
 */
#if defined(VPP_IN_TRUST_ZONE)
#if	(defined(__MEMIO_DIRECT))

#define GA_REG_WORD32_READ(offset, holder) (*(holder) = (*((volatile unsigned int *)devmem_phy_to_virt(offset))))
#define GA_REG_WORD16_READ(offset, holder) (*(holder) = MV_MEMIO_LE16_READ(REG_ADDR(offset)))
#define GA_REG_BYTE_READ(offset, holder)   (*(holder) = MV_MEMIO08_READ(REG_ADDR(offset)))

#define GA_REG_WORD32_WRITE(addr, data)    ((*((volatile unsigned int *)devmem_phy_to_virt(addr))) = ((unsigned int)(data)))
#define GA_REG_WORD16_WRITE(offset, val)   (MV_MEMIO_LE16_WRITE(REG_ADDR(offset), (MV_U16)(val)))
#define GA_REG_BYTE_WRITE(offset, val)     (MV_MEMIO08_WRITE(REG_ADDR(offset), (MV_U8)(val)))

#define GA_REG_WORD32_BIT_SET(offset, bitMask) (MV_MEMIO32_WRITE(REG_ADDR(offset),         \
                            (MV_MEMIO_LE32_READ(REG_ADDR(offset)) | MV_32BIT_LE_FAST(bitMask))))
# define GA_REG_WORD32_BITS_SET(offset, clearMask, val)                                  \
                            (MV_MEMIO32_WRITE(REG_ADDR(offset),                             \
                            ((MV_MEMIO_LE32_READ(REG_ADDR(offset)) & MV_32BIT_LE_FAST(~(clearMask)))\
                            | MV_32BIT_LE_FAST(val))))
#define GA_REG_WORD32_BIT_CLEAR(offset, clearMask)                              \
                            (MV_MEMIO32_WRITE(REG_ADDR(offset),                 \
                            (MV_MEMIO_LE32_READ(REG_ADDR(offset)) & MV_32BIT_LE_FAST(~(clearMask)))))

#elif (defined(__MEMIO_PCIE))
/*!
 * System dependent little endian from / to CPU conversions
 */
#define MV_CPU_TO_LE16(x)           cpu_to_le16(x)
#define MV_CPU_TO_LE32(x)           cpu_to_le32(x)

#define MV_LE16_TO_CPU(x)           le16_to_cpu(x)
#define MV_LE32_TO_CPU(x)           le32_to_cpu(x)

/*!
 * System dependent register read / write in byte/word16/word32 variants
 */
#define GA_REG_WORD32_READ(offset, holder) (*(holder) = MV_LE32_TO_CPU(readl(REG_ADDR(offset))))
#define GA_REG_WORD16_READ(offset, holder) (*(holder) = MV_LE16_TO_CPU(readw(REG_ADDR(offset))))
#define GA_REG_BYTE_READ(offset, holder)   (*(holder) = readb(REG_ADDR(offset)))

#define GA_REG_WORD32_WRITE(offset, val)   writel(MV_CPU_TO_LE32((MV_U32)(val)), REG_ADDR(offset))
#define GA_REG_WORD16_WRITE(offset, val)   writew(MV_CPU_TO_LE16((MV_U16)(val)), REG_ADDR(offset))
#define GA_REG_BYTE_WRITE(offset, val)     writeb((MV_U8)(val), REG_ADDR(offset))

#define GA_REG_WORD32_BIT_SET(offset, bitMask) writel((readl(REG_ADDR(offset)) | MV_CPU_TO_LE32(bitMask)), REG_ADDR(offset))
#define GA_REG_WORD32_BITS_SET(offset, clearMask, val)                                          \
                            writel(((readl(REG_ADDR(offset)) & MV_CPU_TO_LE32(~(clearMask)))    \
                            | MV_CPU_TO_LE32(val)), REG_ADDR(offset))
#define	GA_REG_WORD32_BIT_CLEAR(offset, clearMask)                                  \
                            writel((readl(REG_ADDR(offset)) & MV_CPU_TO_LE32(~(clearMask))), REG_ADDR(offset))

#elif   (defined(__MMIO_SOCKET))
#define GA_REG_WORD32_READ(offset, holder)  (assert(BFM_HOST_Bus_Read32(REG_ADDR(offset), (MV_U32 *)(holder)) >= 0))
#define GA_REG_WORD16_READ(offset, holder)  (assert(BFM_HOST_Bus_Read16(REG_ADDR(offset), (MV_U16 *)(holder)) >= 0))
#define GA_REG_BYTE_READ(offset, holder)    (assert(BFM_HOST_Bus_Read8(REG_ADDR(offset), (MV_U8 *)(holder)) >= 0))

#define GA_REG_WORD32_WRITE(offset, val)    (assert(BFM_Host_Bus_Write32(REG_ADDR(offset), (MV_U32)(val)) >= 0))
#define GA_REG_WORD16_WRITE(offset, val)    (assert(BFM_Host_Bus_Write16(REG_ADDR(offset), (MV_U16)(val)) >= 0))
#define GA_REG_BYTE_WRITE(offset, val)      (assert(BFM_Host_Bus_Write8(REG_ADDR(offset), (MV_U8)(val)) >= 0))

#define GA_REG_WORD32_BIT_SET(offset, bitMask)                  \
        do {                                                    \
            MV_U32      temp;                                   \
                                                                \
            GA_REG_WORD32_READ(offset, &temp);                  \
            temp |= (bitMask);                                  \
            GA_REG_WORD32_WRITE(offset, temp);                  \
        } while (0)

#define	GA_REG_WORD32_BITS_SET(offset, clearMask, val)          \
        do {                                                    \
            MV_U32      temp;                                   \
                                                                \
            GA_REG_WORD32_READ(offset, &temp);                  \
            temp &= ~(clearMask);                               \
            temp |= val;                                        \
            GA_REG_WORD32_WRITE(offset, temp);                  \
        } while (0)

#define	GA_REG_WORD32_BIT_CLEAR(offset, clearMask)              \
        do {                                                    \
            MV_U32      temp;                                   \
                                                                \
            GA_REG_WORD32_READ(offset, &temp);                  \
            temp &= ~(clearMask);                               \
            GA_REG_WORD32_WRITE(offset, temp);                  \
        } while (0)

#else
#error "MEMI/O way not selected"

#endif	/* __MEMIO_SOCKET || __MEMIO_PCIE || __MEMIO_DIRECT	*/

#else
//temp use this, 'cause the cpu endian definition has confliction
#    define    GA_REG_WORD32_READ(offset, holder)   (*(holder) = (*((volatile unsigned int *)devmem_phy_to_virt(offset))))

#    define    GA_REG_WORD16_READ(offset, holder)   (*(holder) = MV_MEMIO_LE16_READ(REG_ADDR(offset)))
#    define    GA_REG_BYTE_READ(offset, holder)     (*(holder) = MV_MEMIO08_READ(REG_ADDR(offset)))

#    define    GA_REG_WORD32_WRITE(addr, data)      ((*((volatile unsigned int *)devmem_phy_to_virt(addr))) = ((unsigned int)(data)))

#    define    GA_REG_WORD16_WRITE(offset, val)     (MV_MEMIO_LE16_WRITE(REG_ADDR(offset), (MV_U16)(val)))
#    define    GA_REG_BYTE_WRITE(offset, val)       (MV_MEMIO08_WRITE(REG_ADDR(offset), (MV_U8)(val)))

#    define    GA_REG_WORD32_BIT_SET(offset, bitMask)    (MV_MEMIO32_WRITE(REG_ADDR(offset),                    \
                            (MV_MEMIO_LE32_READ(REG_ADDR(offset)) | MV_32BIT_LE_FAST(bitMask))))
#    define    GA_REG_WORD32_BITS_SET(offset, clearMask, val)                                    \
                            (MV_MEMIO32_WRITE(REG_ADDR(offset),                    \
                            ((MV_MEMIO_LE32_READ(REG_ADDR(offset)) & MV_32BIT_LE_FAST(~(clearMask)))\
                            | MV_32BIT_LE_FAST(val))))
#    define    GA_REG_WORD32_BIT_CLEAR(offset, clearMask)                                    \
                            (MV_MEMIO32_WRITE(REG_ADDR(offset),                    \
                            (MV_MEMIO_LE32_READ(REG_ADDR(offset)) & MV_32BIT_LE_FAST(~(clearMask)))))

#endif

#endif    /* __GALOIS_IO_H__ */
