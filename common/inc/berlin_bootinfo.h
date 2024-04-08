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

#ifndef __BERLIN_BOOTINFO_H__
#define __BERLIN_BOOTINFO_H__

#include <stdint.h>

#define BG2CDP_CHIP_VERSION_Z1      0x1
#define BG2CDP_CHIP_VERSION_Z2      0x2
#define BG2CDP_CHIP_VERSION_A0      0xA0
#ifdef BERLIN_SOC_MYNA2
#define OTP_DUMP_BASE_ADDR          0xF9200000
#else
#define OTP_DUMP_BASE_ADDR          0xF7A44000
#endif
#define BG4CDP_CHIP_VERSION_Z1      0x1
#define BG4CDP_CHIP_VERSION_A0      0xA0
#define BCM_BOOT_INFO_TSP_ADDRESS   (OTP_DUMP_BASE_ADDR + 0xB00)


typedef struct __berlin_bootinfo_t {
	uint32_t		otp_rkek_id[2]; /* rkek id from OTP */
	uint32_t		otp_version;    /* otp version */
	uint32_t		otp_market_id;  /* otp market id */
	uint32_t		otp_ult[2];     /* otp ULT info */
	uint32_t		chip_version;   /* bg2cdp chip version*/
	uint32_t		speed_tag;      /* speed tag info */
	uint32_t		leakage_current;/* leakage current info */
	uint32_t		temp_id;        /* Temp ID in deg C */
	uint32_t		erom_id;        /* erom id*/
	union
	{
		uint32_t feature_bits;
		struct
		{
			uint32_t rsa_key_index:4;
			uint32_t usb_boot_timeout:4;

			uint32_t romkey_disable:1;
			uint32_t disable_usb_boot:1;
			uint32_t usb_disable_bootstrap:1;
			uint32_t usb_disable_blank_media:1;
			uint32_t usb_force_full_speed:1;
			uint32_t usb_ignore_reset_tracker:1;
			uint32_t otp_locked:1;
		};
	};
	uint32_t		aesk0_crc;      /* crc for aesk0  */
	uint32_t		rkek_crc;       /* crc for rkek   */
	uint32_t		signk7_crc;     /* hash of signk7 crc */
	uint32_t		mrvl_sign_right;/* mrvl sign rights */
	uint32_t		cust_sign_right;/* cust sign rights */

	uint32_t nand_page_size;
	uint32_t nand_ecc;
	uint32_t nand_blk_size;
	union
	{
		uint32_t nand_src_blk_num[2];
		struct
		{
			uint8_t tsm_src_blk_num;
			uint8_t sysinit_src_blk_num;
			uint8_t erom_src_blk_num;
			uint8_t tz_loader_src_blk_num;
			uint8_t tz_kern_src_blk_num;
			uint8_t btld_src_blk_num;
			uint8_t padding[2];
		};
	};
	uint32_t		pvcomp_rev;     /* PVComp Revision */
	uint32_t		bgs;            /* ATE programmed FE BGS value */
	uint32_t		soc_temp_id;    /* SOC Temp ID in deg C */
	uint32_t		cpu_temp_id;    /* CPU Temp ID in deg C */
} BERLIN_BOOTINFO;

#endif /* __BERLIN_BOOTINFO_H__ */
