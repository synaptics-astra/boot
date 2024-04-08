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
#ifndef	_IMAGE_2_IO_H
#define	_IMAGE_2_IO_H

#include "com_type.h"
#define BIT(x)              (0x1 << x)

#define	__chk_io_ptr(a)	    (void)0

#define UNUSED(var) do { (void)(var); } while(0)

/* Macro to correctly convert 32-bit address to pointer */
#define ADDR32_TO_PTR(x) ((void *)(uintptr_t)(x))
#define PTR_TO_ADDR32(x) ((uint32_t)(uintptr_t)(x))

#if defined(ARMV8_64BIT)
#define dsb() __asm__ __volatile__ ("dsb sy" : : : "memory")
#else
#define dsb() __asm__ __volatile__ ("dsb" : : : "memory")
#endif
#define mb() dsb()

uint8_t read8(void *addr);
void write8(void *addr, uint8_t value);
uint16_t read16(void *addr);
void write16(void *addr, uint16_t value);
uint32_t read32(void *addr);
void write32(void *addr, uint32_t value);
uint64_t read64(void *addr);
void write64(void *addr, uint64_t value);
#define __raw_writeb(v,a)   (__chk_io_ptr(a), *(volatile unsigned char *)(a) = (v))
#define __raw_writew(v,a)   (__chk_io_ptr(a), *(volatile unsigned short *)(a) = (v))
#define __raw_writel(v,a)   (__chk_io_ptr(a), *(volatile unsigned int *)(a) = (v))

#define __raw_readb(a)      (__chk_io_ptr(a), *(volatile unsigned char *)(a))
#define __raw_readw(a)      (__chk_io_ptr(a), *(volatile unsigned short *)(a))
#define __raw_readl(a)      (*(volatile unsigned int *)(a))


#define writeb(v, a)	write8((void *)((uintmax_t)a), v)
#define writew(v, a)	write16((void *)((uintmax_t)a), v)
#define writel(v, a)	write32((void *)((uintmax_t)a), v)
#define writex(v, a)	write64((void *)((uintmax_t)a), v)

#define readb(a)	read8((void *)((uintmax_t)a))
#define	readw(a)	read16((void *)((uintmax_t)a))
#define	readl(a)	read32((void *)((uintmax_t)a))
#define	readx(a)	read64((void *)((uintmax_t)a))

#ifndef REG_WRITE64
#define REG_WRITE64(a, v) (write64((void *)(uintptr_t)(a), v))
#endif
#ifndef REG_READ64
#define REG_READ64(a, h) (*h = read64((void *)(uintptr_t)(a)))
#endif
#ifndef REG_WRITE32
#define REG_WRITE32(a, v) (write32((void *)(uintptr_t)(a), v))
#endif

#ifndef REG_READ32
#define REG_READ32(a, h) (*h = read32((void *)(uintptr_t)(a)))
#endif

#ifndef REG_WRITE16
#define REG_WRITE16(a, v) (write16((void *)(uintptr_t)(a), v))
#endif

#ifndef REG_READ16
#define REG_READ16(a, h) (*h = read16((void *)(uintptr_t)(a)))
#endif

#ifndef REG_WRITE8
#define REG_WRITE8(a, v) (write8((void *)(uintptr_t)(a), v))
#endif

#ifndef REG_READ8
#define REG_READ8(a, h) (*h = read8((void *)(uintptr_t)(a)))
#endif

#ifndef BFM_HOST_Bus_Write32
#define BFM_HOST_Bus_Write32 REG_WRITE32
#endif

#ifndef BFM_HOST_Bus_Read32
#define BFM_HOST_Bus_Read32 REG_READ32
#endif

#ifndef GA_REG_WORD32_WRITE
#define GA_REG_WORD32_WRITE REG_WRITE32
#endif

#ifndef GA_REG_WORD32_READ
#define GA_REG_WORD32_READ REG_READ32
#endif

#define	io32_set_bits(a, v, lsb, bits) \
do { \
	uint32_t reg_val, mask; \
	mask = ((bits<32) ? ((1<<(bits)) - 1) : ~0) << (lsb); \
	reg_val = readl(a); \
	reg_val &= ~mask; \
	reg_val |= ((v) << (lsb)) & mask; \
	writel(reg_val, a); \
} while(0);


#endif
