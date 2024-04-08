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


////////////////////////////////////////////////////////////////////////////////
//! \file sys_memmap.h
//! \brief system level memory map, define memory block address for different CPUs.
//!	
//!	
//!	
//! Purpose:
//!	   define memory address for CPUs
//!
//!	Version, Date and Author :
//!		V 1.00,	7/16/2007,	Fenglei Wang
//!
//! Note:
////////////////////////////////////////////////////////////////////////////////

#ifndef _SYS_MEMMAP_H_
#define _SYS_MEMMAP_H_

#include "memmap.h"

#define GALOIS_CPU1_CACHE_START		(MEMMAP_DRAM_CACHE_BASE)
#if defined(BERLIN)
#define GALOIS_CPU1_CACHE_SIZE		0x16000000
#else
#define GALOIS_CPU1_CACHE_SIZE		0x0e000000
#endif

#define GALOIS_CPU0_CACHE_START		( GALOIS_CPU1_CACHE_START + GALOIS_CPU1_CACHE_SIZE )
#if defined(BERLIN)
#define GALOIS_CPU0_CACHE_SIZE		0x06000000
#else
#define GALOIS_CPU0_CACHE_SIZE		0x0e000000
#endif

#define GALOIS_CACHE_ADDR_END		( GALOIS_CPU0_CACHE_START + GALOIS_CPU0_CACHE_SIZE )

#if GALOIS_CACHE_ADDR_END > ( MEMMAP_DRAM_CACHE_BASE + MEMMAP_DRAM_CACHE_SIZE )
#error Error: cached memory address out of range!!!
#endif

#define GALOIS_NON_CACHE_START		( MEMMAP_DRAM_UNCACHE_BASE | (GALOIS_CACHE_ADDR_END))
#define GALOIS_NON_CACHE_SIZE		0x04000000

#define CACHE_ADDR_MASK			(MEMMAP_DRAM_UNCACHE_BASE -1)
#define NON_CACHE_ADDR( x )		( (void*)( ( (unsigned int)(x) & CACHE_ADDR_MASK) | MEMMAP_DRAM_UNCACHE_BASE) )

#define CACHE_ADDR( x )			( (void*)( (unsigned int)(x) & CACHE_ADDR_MASK) )

#endif	// #ifndef _SYS_MEMMAP_H_
