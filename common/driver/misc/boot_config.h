/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright © 2013-2026 Synaptics Incorporated. All rights reserved.
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
///////////////////////////////////////////////////////////////////////////////
//! \file	       boot_config.h
//! \brief 	checking boot flag and PMIC settings.
///////////////////////////////////////////////////////////////////////////////

#ifndef _BOOT_CONFIG_H
#define _BOOT_CONFIG_H

#include "pmic_config.h"

#define BOOT_CONFIG_MAGIC	 0x42544346  /* "BTCF" */
#define BOOT_CONFIG_VERSION   0x00000001

struct bootflag_config {
	uint32_t skip_fw   : 1; /* Don't load firmware in Miniloader */
	uint32_t force_spi : 1; /* Don't check SD in SPI boot mode */
	uint32_t rsvd_bits : 30;
	uint32_t reserved[12];
};

struct boot_config {
	uint32_t checksum;  /* Simple checksum */
	uint32_t magic;     /* Magic number: 0x42544346 ("BTCF") */
	uint32_t version;   /* Version: 0x00000001 */

	struct bootflag_config bootflag; /* 52 Bytes*/

	struct pmic_config pmic;
};

extern struct boot_config g_boot_config;

int get_bootflag_skip_fw(void);
int get_bootflag_force_spi(void);
int get_pmic_config(struct pmic_config **pmic);
int load_boot_config(void);

#endif
