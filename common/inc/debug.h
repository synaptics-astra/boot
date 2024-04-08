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
#ifndef __DEBUG_H
#define __DEBUG_H

#include "com_type.h"
#include "printf.h"

enum PRINT_LEVEL
{
	PRN_NONE=0,
	PRN_FATAL,
	PRN_ERR,
	PRN_NOTICE,
	PRN_RES,
	PRN_INFO,
	PRN_DBG

};

void set_dbg_level(int lvl);
int get_dbg_level();

void dumpmem(uint8_t * mem, uint32_t size);

#ifdef CONFIG_LESSPRINT
#define	dbg_printf(lel,format,args...)	\
do { \
} while(0)
#define DBG_SYSINIT(format, args...) \
do { \
	printf(format, ## args); \
} while(0)
#else
#define	dbg_printf(lel,format,args...)	\
do { \
	if (lel <= get_dbg_level()) { \
		printf(format, ## args); \
		} \
} while(0)
#endif

#define FATAL(format, args...) dbg_printf(PRN_ERR, format, ## args)
//#define ERR(format, args...) dbg_printf(PRN_ERR, format, ## args)
#define ERR(format, args...) lgpl_printf(format, ## args)

//#define NOTICE(format, args...) dbg_printf(PRN_NOTICE, format, ## args)
#define NOTICE(format, args...) lgpl_printf(format, ## args)
#define INFO(format, args...) dbg_printf(PRN_INFO, format, ## args)

#define DBG(format, args...) dbg_printf(PRN_DBG, format, ## args)

#endif
