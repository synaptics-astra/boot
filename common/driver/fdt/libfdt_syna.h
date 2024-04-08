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
#ifndef __LIBFDT_SYNA_H__
#define __LIBFDT_SYNA_H__

#include "com_type.h"

#define NR_BANKS                8

typedef struct system_info_t {
	unsigned int system_serial_low;
	unsigned int system_serial_high;
	unsigned int chip_ver;
	unsigned int leakage_current;
	unsigned int pvcomp_rev;
} system_info_t;

struct reserve_entry {
	unsigned int reserved_mem_start;
	unsigned int reserved_mem_size;
};

int set_fdt(void *fdt, int total_space,unsigned int initrd_start,unsigned int initrd_size,char * cmdline);

int fdt_set_reserved_mem(void *fdt, void * mem, unsigned int reserved_num);


void set_fdt_addr(unsigned int fdt, int total_space);
int get_fdt_addr(unsigned int *fdt, int * total_space);

int fdt_add_system_info(void *fdt,
		int total_space,
		const char *pnode,
		const system_info_t *system_info);

int fdt_update_i2c_slave_addr(void *fdt, int total_space, unsigned int masterid, unsigned int slave_addr);
int fdt_add_opp(void *fdt, int total_space, unsigned int * opp, int count);
void fdt_set_vcpu_opp(void * fdt, unsigned int vh, unsigned int vl);
void fdt_set_vcore_opp(void * fdt, unsigned int vcoreh, unsigned int vcorel);
int fdt_set_leakage(void *fdt, int total_space, unsigned int leakage_num);
int fdt_set_chiprev(void *fdt, int total_space, unsigned int chip_rev);
void fdt_select_pmic(void *fdt);
int fdt_set_mmc_param(void *fdt, int total_space);
#endif /* __LIBFDT_SYNA_H__ */
