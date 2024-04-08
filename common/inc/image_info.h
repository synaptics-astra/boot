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
#ifndef __IMAGE_INFO_H__
#define __IMAGE_INFO_H__

#define	IMAGE_INFO_VERSION_1_0_0_0	0x01000000
#define	IMAGE_INFO_MAGIC		0xE11D

#ifdef CONFIG_GENX_ENABLE
/* GenX Image Type Define */
#define IMAGE_TYPE_EROM                 0x10
#define IMAGE_TYPE_SCS_DATA_PARAM       0X11
#define IMAGE_TYPE_BOOTMONITOR          0x12
#define IMAGE_TYPE_SYS_INIT             0x13
#define IMAGE_TYPE_MINILOADER           0x14
#define IMAGE_TYPE_BCM_KERNEL           0x15
#define IMAGE_TYPE_TZ_KERNEL            0x16
#define IMAGE_TYPE_ATF                  0x17
#define IMAGE_TYPE_TZK_BOOT_PARAMETER   0x18
#define IMAGE_TYPE_TA_ROOT_CERT         0x19
#define IMAGE_TYPE_TA_CERT              0x1a
#define IMAGE_TYPE_TA                   0x1b
#define IMAGE_TYPE_TZK_CONTAINER_HEADER 0x1c

#define IMAGE_TYPE_BOOTLOADER           0x20
#define IMAGE_TYPE_LINUX_KERNEL         0x21
#define IMAGE_TYPE_TZK_OEM_SETTINGS     0x22
#define IMAGE_TYPE_AVB_KEYS             0x23
#define IMAGE_TYPE_LINUX                0x24
#define IMAGE_TYPE_UBOOT                0x25
#define IMAGE_TYPE_FASTBOOT             0x26
#define IMAGE_TYPE_FASTLOGO             0x27

#define IMAGE_TYPE_TSP_FW               0x30
#define IMAGE_TYPE_DSP_FW               0x31
#define IMAGE_TYPE_GPU_FW               0x32
#define IMAGE_TYPE_IFCP_FW              0x33
#define IMAGE_TYPE_SM                   0x34

#define GENX_IMAGE_HEADER_BOOTLOADER_SIZE   336
#define GENX_IMAGE_HEADER_BOOT_SIZE         336
#define GENX_IMAGE_HEADER_SM_SIZE           336
#define GENX_IMAGE_HEADER_AVB_KEYS_SIZE     336
#define GENX_IMAGE_HEADER_FASTLOGO_SIZE     336
#define GENX_IMAGE_HEADER_LINUX_SIZE        336

struct genx_image_info
{
	uint32_t	image_type;
	uint32_t	image_format_version;
	uint32_t	image_size; //header + body
};
#endif

struct image_info
{
	uint32_t	version;
	uint16_t	magic;
	uint8_t		reserved1[2];
	uint32_t	image_size;
	uint32_t	image_offset;
	uint8_t		reserved2[48];
};

#define PREPEND_IMAGE_INFO_SIZE sizeof(struct image_info)
#endif
