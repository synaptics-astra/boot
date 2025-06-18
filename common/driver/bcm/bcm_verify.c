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
#include "com_type.h"

#include "bcmREG.h"
#include "Galois_memmap.h"
#include "debug.h"
#include "timer.h"
#include "string.h"

#include "bcm_mailbox.h"
#include "bcm_status.h"

#include "bcm_verify.h"

#ifdef CONFIG_DCACHE
#include "cache.h"
#endif

#ifdef CONFIG_TRUSTZONE
#include "tz_nw_boot.h"
#endif

#include "codetype.h"

#define BCM_PI_IMAGE_VERIFY  0x0005

int bcm_image_verify(unsigned int type, unsigned int src, unsigned int dst)
{
	BCM_MAILBOX_COMMAND cmd;

	BCM_MAILBOX_STATUS status;

	bcm_clear_command(&cmd);
	cmd.arg0 = type;
	cmd.arg1 = src;
	cmd.arg2 = dst;
	cmd.command = BCM_PI_IMAGE_VERIFY;

	bcm_clear_status(&status);
	execute_bcm_command(&cmd, &status);
	if(status.return_status != STATUS_SUCCESS)

				return -1;

		return 0;
}

#if !defined(CLEAR_BOOTFLOW)
#define BCM_MAILBOX          MEMMAP_BCM_REG_BASE //0xF7930000
//#define BCM_PI_IMAGE_VERIFY  0x004E
#define BCM_STATUS_BCM_FAULT               (1<<10)
#define BCM_STATUS_BOOTSTRAP_IN_PROGRESS   (1<<9)
#define BCM_STATUS_BCM_READY               (1<<8)
#define BCM_STATUS_BCM_CMD_FLIP            (1<<7)

int bcm_verify(unsigned int type, unsigned int src, unsigned int dst)
{
	unsigned status;
	unsigned int waitCount;
#ifndef CONFIG_LESSPRINT
	unsigned int i, *p32;
#endif
	volatile NOT_MAILBOX *mb = (NOT_MAILBOX *) BCM_MAILBOX;

#ifndef CONFIG_LESSPRINT
	p32 = (unsigned int *)(uintptr_t) src;
	dbg_printf(PRN_DBG,"\tverify 0:0x%x, 1k:0x%x, 2k:0x%x\n", p32[1], p32[1+1024/4], p32[1+2048/4]);
#endif

	status = mb->command_fifo_status;
	// older erom will have BCM_STATUS_BOOTSTRAP_IN_PROGRESS set
	if (0 == (status & BCM_STATUS_BOOTSTRAP_IN_PROGRESS))
	{
		// wait for bcm ready
		//lgpl_printf("wait for bcm ready...\n");
		//for (waitCount=~255; waitCount; waitCount--)
		while (0 == (status & BCM_STATUS_BCM_READY))
		{
			if (status != mb->command_fifo_status)
			{
				status = mb->command_fifo_status;
				//lgpl_printf("bcm fifo staus = 0x%x\n", status);
			}
		}
	}
	mb->primitive_command_parameter0 = type;
	mb->primitive_command_parameter1 = src;
	mb->primitive_command_parameter2 = dst;
	mb->secure_processor_command = BCM_PI_IMAGE_VERIFY;

	//for (waitCount=0; waitCount<~255; waitCount++) // Wait_For_WTM_Complete( 0x10000, pCtrl );
	for (waitCount=0; ; waitCount++) // Wait_For_WTM_Complete( 0x10000, pCtrl );
	{
		//if ((mb->command_fifo_status & BCM_STATUS_BCM_CMD_FLIP) != status)
			//break;
		// wait for "command complete" or timeout
		if( mb->host_interrupt_register & 0x1 )
			break;
		mdelay(1);
	}

	/*
	  * Do a full reset to BCM HOST interrupt
	  * ----------------------------------------------------------------------------------
	  * Bit      Field Name              Access  Reset  Description
	  *                                  Mode    Value	 * 31:19    RSVD                      R/W   0h     Reserved
	  * 18       HOST_Q_FULL_RST           R/W   0h     Host Queue Full Reset
	  * 17       HOST_Q_FULL_ACC_RST       R/W   0h     Host Queue Full Access Reset
	  * 16       HOST_ADRS_PNG_EXCPTN_RST  R/W   0h     Host Address Range Exception Reset
	  * 15:1     RSVD                      R/W   0h     Reserved
	  * 0        SP_CMD_CMPLT_RST          R/W   0h     SP Command Complete Reset
	  * ----------------------------------------------------------------------------------
	  */

	mb->host_interrupt_register = 0x70001; // Clear_WTM_Interrupts( 0xffffffff, pCtrl );
	status = mb->command_return_status;
#ifndef CONFIG_LESSPRINT
	i = mb->command_status_0;
	dbg_printf(PRN_DBG, "\tverify image %x, size=%d, waitcount=%d\n", status, i, waitCount);
#endif
	return status;
}
#endif

#ifdef CONFIG_GENX_ENABLE
int VerifyImage(const void *src, unsigned int size, void *dst, unsigned int image_type)
{
#if defined(CLEAR_BOOTFLOW)
	const char * buffer_src = src;
	if(buffer_src != dst) {
		if(size == 0)
			return -1;
		memmove(dst, buffer_src, size);
	}
	return 0;
#else
	int ret = 0;
	dbg_printf(PRN_DBG, "tz_nw_verify_image: src 0x%08x, dst: 0x%08x, size: 0x%x, image_type 0x%x\n", src, dst, size, image_type);
	ret = tz_nw_verify_image(5, src, size, dst, size, image_type);

#ifdef CONFIG_DCACHE
	invalidate_dcache_range(dst, (void *)(((char *)dst) + size));
#endif

	if (ret > 0)
		ret = 0;
	return ret;
#endif
}
#else
int VerifyImage(const void *src, unsigned int size, void *dst, unsigned int codetype)
{
#if defined(CLEAR_BOOTFLOW)
	const char * buffer_src = src;
	if(CODETYPE_KERNEL == codetype)
		buffer_src = (char *)((uintptr_t)src + CUSTKEY_SIZE);

	if(buffer_src != dst) {
		if(size == 0)
			return -1;
		memmove(dst, buffer_src, size);
	}
	return 0;
#else
//In SH code base, we use 0x4 as enc_type for TZ, bootloader and kernel.
//It should match with building TZ and kernel.
//And for sysinit, we use 0x1.
	int ret = 0;
#ifdef CONFIG_DCACHE
	//FIXME: so we'd better know the size of buffer
	flush_dcache_range(dst, (void *)(((char *)dst) + size));
#endif

#ifdef CONFIG_TRUSTZONE
	ret = tz_nw_verify_image(5, src, size, dst, size, codetype);
#else
#if (BERLIN_CHIP_VERSION >= BERLIN_BG4CD)
	//FIXME:
	ret = bcm_verify(codetype, (unsigned int)(uintptr_t)src, (unsigned int)(uintptr_t)dst);
#else
	ret = bcm_verify(4, (unsigned int)src, (unsigned int)dst);
#endif
#endif

#ifdef CONFIG_DCACHE
	invalidate_dcache_range(dst, (void *)(((char *)dst) + size));
#endif

	if (ret > 0)
		ret = 0;
	return ret;
#endif
}
#endif
