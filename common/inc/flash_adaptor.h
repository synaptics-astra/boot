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
#ifndef _FLASH_OP_H_
#define _FLASH_OP_H_

#include "priv_type.h"

#define FLASH_SWITCH_PART_NOEXIST	(-1)
#define FLASH_OP_ERR	 	(-2)
struct flash_adaptor {
	size_t boot_part_size;
	size_t block_size;
	size_t page_size;
	int addr_cycle;
	int oob_size;

	int (*init)(int dev);
	long long (*get_capacity)(int dev);
	unsigned int (*dev_id_inc)(unsigned int dev_id);

	int (*get_boot_partition_number)(void);
	loff_t (*switch_part)(unsigned int);
	void (*set_parameters)(int blocksize, int pagesize, int addrcycle);
	void (*stop_alt_boot_mode)(void);


	long long (*read)(long long start, unsigned int size, unsigned char *buff);
	long long (*write)(long long start, unsigned int size, unsigned char *buff);
};
void set_flash_parameters(int blocksize, int pagesize, int addrcycle);
size_t get_block_size();
size_t get_page_size();
size_t get_boot_part_size();
int get_addr_cycle();
int get_boot_partition_number(void);
int init_flash();
int switch_flash_part(unsigned int part);
unsigned int flash_dev_id_inc(unsigned int dev_id);
long long read_flash(long long start, unsigned int size, unsigned char *buff);
long long read_flash_from_part(unsigned int part, long long start, unsigned int size, unsigned char *buff);
long long get_flash_capacity(void);
long long write_flash(long long start, unsigned int size, unsigned char *buff);
void stop_alt_boot_mode(void);
#endif
