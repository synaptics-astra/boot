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
#ifndef IMAGE_CHUNK_H
#define IMAGE_CHUNK_H

#define IMAGE_HEADER_VERSION		(0)

#ifndef MAKE_FOURCC
#define MAKE_FOURCC(ch0, ch1, ch2, ch3) \
	((unsigned int)(char)(ch0) | ((unsigned int)(char)(ch1) << 8) | \
	((unsigned int)(char)(ch2) << 16) | ((unsigned int)(char)(ch3) << 24))
#endif /* MAKE_FOURCC */

#define IMAGE_HEADER_MAGIC_NUM		MAKE_FOURCC('I', 'M', '*', 'H')	/* 'IM*H' */

#define IMAGE_CHUNK_ID_CUSTKEY		MAKE_FOURCC('C', 'U', 'T', 'K')
#define IMAGE_CHUNK_ID_CUSTFW		MAKE_FOURCC('C', 'T', 'F', 'W')
#define IMAGE_CHUNK_ID_DDRSCRAM		MAKE_FOURCC('D', 'D', 'R', 'S')
#define IMAGE_CHUNK_ID_PTINFO		MAKE_FOURCC('P', 'T', 'I', 'N')
#define IMAGE_CHUNK_ID_BOOTLOADER		MAKE_FOURCC('B', 'T', 'L', 'R')
#define IMAGE_CHUNK_ID_JTAGEN_FW		MAKE_FOURCC('J', 'T', 'A', 'G')
#define IMAGE_CHUNK_ID_UNITID_FW		MAKE_FOURCC('U', 'N', 'I', 'T')
#define IMAGE_CHUNK_ID_SYSTEM_MANAGER		MAKE_FOURCC('S', 'M', '*', '*')
#define IMAGE_CHUNK_ID_UBOOT			MAKE_FOURCC('U', 'B', 'T', '*')

#define IMAGE_CHUNK_ID_LINUX_BOOTIMG		MAKE_FOURCC('L', 'N', 'X', 'B')
#define IMAGE_CHUNK_ID_LINUX_DTB		MAKE_FOURCC('L', 'D', 'T', 'B')

#define IMAGE_CHUNK_ID_TZ_KERNEL		MAKE_FOURCC('T', 'Z', 'K', '*')
#define IMAGE_CHUNK_ID_TZ_BOOT_PARAM		MAKE_FOURCC('T', 'Z', 'B', 'P')

#define IMAGE_CHUNK_ID_TZK_PAKING		MAKE_FOURCC('T', 'Z', 'K', 'P')
#define IMAGE_CHUNK_ID_TZ_OEM_PARAM		MAKE_FOURCC('T', 'Z', 'O', 'P')

#define IMAGE_CHUNK_ID_TZK1			MAKE_FOURCC('T', 'Z', 'K', '1')
#define IMAGE_CHUNK_ID_TZK2			MAKE_FOURCC('T', 'Z', 'K', '2')

#define IMAGE_CHUNK_ID_TZ_LOADABLE_TA		MAKE_FOURCC('T', 'Z', 'T', 'A')

#define IMAGE_CHUNK_ID_FASTBOOT		MAKE_FOURCC('F', 'S', 'B', 'T')

#define IMAGE_CHUNK_ID_SSTZ		MAKE_FOURCC('S', 'S', 'T', 'Z')

#define IMAGE_CHUNK_ID_OEMK		MAKE_FOURCC('O', 'E', 'M', 'K')
#define IMAGE_CHUNK_ID_USRK		MAKE_FOURCC('U', 'S', 'R', 'K')

#define IMAGE_CHUNK_ID_FASTLOGO		MAKE_FOURCC('L', 'O', 'G', 'O')
#define IMAGE_CHUNK_ID_KEYMASTER		MAKE_FOURCC('C', 'Y', 'P', 'T')

#define IMAGE_CHUNK_ID_SM_FW	MAKE_FOURCC('S', 'M', 'F', 'W')
#define IMAGE_CHUNK_ID_TSP_FW	MAKE_FOURCC('T', 'S', 'P', 'F')
#define IMAGE_CHUNK_ID_DSP_FW	MAKE_FOURCC('D', 'S', 'P', 'F')
#define IMAGE_CHUNK_ID_GPU_FW	MAKE_FOURCC('G', 'P', 'U', 'F')
#define IMAGE_CHUNK_ID_IFCP_FW	MAKE_FOURCC('I', 'F', 'C', 'P')

#define MAX_IMAGE_CHUNK_ENTRY 128

#endif	/* IMAGE_CHUNK_H */
