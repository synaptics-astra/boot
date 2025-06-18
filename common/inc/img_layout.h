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
#ifndef _IMG_LAYOUT_H_
#define _IMG_LAYOUT_H_

#if defined(CONFIG_BCM) || defined(BCM_EN)

/*******************************************************
 *   layout of bootloader.subimg in BCM case
 *   _____________________________
 *  |          |  OEM_ROOT_RSA(4k, p) + MRVL_ROOT_RSA (4k) (not for bg3cd)
 *  |page   |  BCM_CUSTK0(1k) + layout(512B, bg3cd only) + BCM_EXT_RSA0(1k, 3cd)(4k, 4ct) + header(1k:2k) + EROM
 *  |aligned|  APU_CUSTK0(1k) + APU_EXT_RSA0(1k:4k) + header(1k:2k) + sys_init
 *  |          |  page aligned (only for bg3cd?)
 *  |          |  BCM_CUSTK1(1k) + BCM_EXT_RSA1(1k, 4k) + header(1k:2k) + TSMK
 *  |          |  page aligned
 *  |______|_______________________
 *  | XXk   |  APU_CUSTK0(1k) + APU_EXT_RSA0(1k:4k) + header(1k:2k) + miniloader(encrypted) 
 *  |          |  page aligned
 *  |_____ |_______________________
 *  |		  |
 *  |  12k   |  OEM_CUSTK1 (1k) + OEM_EXT_RSA0 (1k:4k) + boot layout info (2k + 2k) + page aligned
 *  |_____ |_______________________
 *  |          |
 *  |  8k     |  partition table infor (2k + 2k) + page aligned
 *  |______|_______________________
 *  |          | OEM Key store header (2K)
 *  |          | OEM Key store (2K, clear)
 *  | 6k      | User Key store header (2K)           (android only)
 *  |          | User Key store(2K, clear)
 *  |______|_______________________
 *  |       stuffing
 *  |_____________________________
 *
 *****************************************************/
/* use different layout_info because they have different prebuild images */
#ifndef SYS_INIT_SIZE
#define SYS_INIT_SIZE	(180 << 10)
#endif

#ifndef MINILOADER_SIZE
#define MINILOADER_SIZE	(60 << 10)
#endif

#ifndef CONFIG_UBOOT
#define MINILOADER_ADDR		SYS_INIT_SIZE

#define IMG_HEADER_ADDR		(MINILOADER_ADDR + MINILOADER_SIZE)
#define IMG_HEADER_SIZE		(12 << 10)
#else
//we use the same layout with drm and put image header before miniloader
#define IMG_HEADER_ADDR		SYS_INIT_SIZE
#define IMG_HEADER_SIZE		(12 << 10)

#define MINILOADER_ADDR		(SYS_INIT_SIZE + IMG_HEADER_SIZE)
#endif
//#define PT_INFO_ADDR		(IMG_HEADER_ADDR + IMG_HEADER_SIZE)
#define PT_INFO_SIZE		(8 << 10)

//#define ANDROID_KEY_ADDR		(PT_INFO_ADDR + PT_INFO_SIZE)
#define ANDROID_OEMKEY_SIZE		(12 << 10)
#define ANDROID_USRKEY_SIZE		(12 << 10)


#else
/*******************************************************
 *   layout of bootloader.subimg
 *   _____________________________
 *  | 52K  |  sys_init binary(fixed)
 *  |_____|_______________________
 *  | 4K    |  Encrypted image header(fixed)
 *  |_____|_______________________
 *  | 4K    |  Custkey(fixed)
 *  |_____|_______________________
 *  | 20K  |  DRM Custkey loading FW(fixed)
 *  |_____|_______________________
 *  | 20K  |  DRM DDR scrambler FW(fixed)
 *  |_____|_______________________
 *  | 36K  |  miniloader(fixed)
 *  |_____|_______________________
 *  | 4K    |  Partition version table
 *  |_____|_______________________
 *  |         |  encrypted header
 *  |         |_______________________
 *  | xxxK |   generic image header
 *  |         |_______________________
 *  |         |   data chunk 1: bootloader
 *  |         |_______________________
 *  |         |   data chunk 2: others
 *  |_____|_______________________
 *  | xxxK |  jtag debug enable fw
 *  |_____|_______________________
 *
 *****************************************************/
#define SYS_INIT_SIZE		(52 << 10)

#define IMG_HEADER_ADDR		SYS_INIT_SIZE
#define IMG_HEADER_SIZE		(4 << 10)

#ifndef CONFIG_UBOOT
#define CUST_KEY_ADDR		(IMG_HEADER_ADDR + IMG_HEADER_SIZE)
#define CUST_KEY_SIZE		(4 << 10)

#define DRM_FIGO_ADDR		(CUST_KEY_ADDR + CUST_KEY_SIZE)
#define DRM_FIGO_SIZE		(20 << 10)

#define DDR_SCRAMBLER_ADDR	(DRM_FIGO_ADDR + DRM_FIGO_SIZE)
#define DDR_SCRAMBLER_SIZE	(20 << 10)

#define MINILOADER_ADDR		(DDR_SCRAMBLER_ADDR + DDR_SCRAMBLER_SIZE)
#define MINILOADER_SIZE		(36 << 10)
#else
#define MINILOADER_ADDR		(IMG_HEADER_ADDR + IMG_HEADER_SIZE)
#define MINILOADER_SIZE		(36 << 10)
#endif

#define PT_INFO_SIZE		(4 << 10)

#endif
#define UBOOT_ENV_ADDR		(12 << 20)
#endif
