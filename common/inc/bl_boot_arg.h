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
#ifndef _BL_BOOT_ARG_H_
#define _BL_BOOT_ARG_H_

#define BL_ARG_MAGIC 0xdeadbeaf

enum fastboot_command {
	CMD_NULL = 0,
	CMD_BOOT,
	CMD_CONTINUE,
	CMD_REBOOT,
	CMD_REBOOT_FB,
	CMD_END
};

struct fastboot_cmd {
	unsigned int command;
	unsigned int param[3];
};

struct bl_boot_arg {
	unsigned long param[8];		// the first param is set to the address of bl_boot_arg.
	unsigned int flash_param[3];	// the original param
	unsigned int leakage;		// leakage id
	unsigned int chip_id[2];	// unique chip id
	unsigned int soc_tsen_id;	// soc tsen_id
	unsigned int cpu_tsen_id;	// cpu tsen_id
	unsigned int chip_revision;	// chip revision: Z1/A0/A1/A2
};

typedef struct {
	unsigned int magic;
	unsigned int flag;
#ifdef CONFIG_FAST_BOOT
	struct fastboot_cmd cmd;
	unsigned long mm_addr;
	unsigned int mf_addr;
	unsigned int mf_size;
	char serial_no[64];
	char product[32];
#endif
}bootflag_t;

typedef struct {
	volatile unsigned int magic;
	volatile unsigned int flag;
}rebootflag_t;

unsigned int get_chip_revision(void);

#endif
