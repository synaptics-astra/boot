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
//! \file com_type.h
//! \brief type define for default type
//!
//! Purpose:
//!     Defines an alias name for default type
//!
//! Note:
//!     Version, Date and Author :
//!             04/15/2007,     Fang Bao
//!                             Add CYGPKG_KERNEL_THREADS_DESTRUCTORS macro to avoid error if no eCos lib supported
//!             06/24/2008, Fang Bao
//!                             Modify MV_CC_(X)SEC_PER_SEC_64 using singed value, add MV_CC_NSEC_PER_SEC_U64 using unsiged value
//!
////////////////////////////////////////////////////////////////////////////////

#ifndef __COMMON_TYPE_H__
#define __COMMON_TYPE_H__

#include <stddef.h>

/*---------------------------------------------------------------------------
    GNU C/C++
  ---------------------------------------------------------------------------*/

    typedef	unsigned char       UNSG8;
    typedef	signed char         SIGN8;
    typedef	unsigned short      UNSG16;
    typedef	signed short        SIGN16;
    typedef	unsigned int        UNSG32;
    typedef	signed int          SIGN32;
    typedef	unsigned long long  UNSG64;
    typedef	signed long long    SIGN64;
    typedef	float               REAL32;
    typedef	double              REAL64;

    #ifndef INLINE
        #define INLINE          static inline
    #endif


typedef unsigned char          UCHAR;
typedef char                   CHAR;

typedef UCHAR                  BOOL;
typedef UCHAR                  BOOLEAN;
typedef CHAR                   INT8;
typedef UCHAR                  UINT8;
typedef UCHAR                  BYTE;

typedef short                  INT16;
typedef unsigned short         UINT16;
typedef int                    INT32;
typedef unsigned int           UINT32;
typedef long long              INT64;
typedef unsigned long long     UINT64;
typedef unsigned int           UINT;
typedef int                    INT;

typedef signed int             HRESULT;
/*---------------------------------------------------------------------------
 *    Multiple-word types
 *---------------------------------------------------------------------------*/
#ifndef Txxb
#define Txxb
typedef UINT8                           T8b;
typedef UINT16                          T16b;
typedef UINT32                          T32b;
typedef UINT32                          T64b [2];
typedef UINT32                          T96b [3];
typedef UINT32                          T128b[4];
typedef UINT32                          T160b[5];
typedef UINT32                          T192b[6];
typedef UINT32                          T224b[7];
typedef UINT32                          T256b[8];
#endif

typedef volatile unsigned int  VUINT32;

typedef void                   VOID;
typedef void*                  PTR;
typedef void**                 PHANDLE;
typedef void*                  HANDLE;
typedef void*                  PVOID;

typedef long long              loff_t;
typedef signed int             hresult;

#ifndef TRUE
#define TRUE                   (1)
#endif
#ifndef FALSE
#define FALSE                  (0)
#endif

#ifndef true
#define true                   (1)
#endif
#ifndef false
#define false                  (0)
#endif

#ifndef True
#define True                   (1)
#endif
#ifndef False
#define False                  (0)
#endif

#ifndef NULL
#define NULL                   ((void *)0)
#endif

#define E_SUC           (0x00000000)
#define E_ERR           (0x80000000)

#define E_GENERIC_BASE  (0x0000 << 16)

#define E_GEN_SUC(code) (E_SUC | E_GENERIC_BASE | (code&0x0000FFFF))
#define E_GEN_ERR(code) (E_ERR | E_GENERIC_BASE | (code&0x0000FFFF))

#define S_OK            E_GEN_SUC(0x0000)
#define S_FALSE         E_GEN_SUC(0x0001)

#define MAGIC_NUMBER           (0xD2ADA3F1)
#define COMMAND_SIZE            5
#define _RESERVED_              0

#define IMAGE3_DDR_ADDR         IMAGE3_DDR_ADDRESS

#define reg(addr)               (*((volatile int*)(addr)))

#define UNUSED(var) do { (void)(var); } while(0)

#endif  // #ifndef __COMMON_TYPE__
