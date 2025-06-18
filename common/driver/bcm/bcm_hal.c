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
////////////////////////////////////////////////////////////////////////////////
//! \file  : bcm_hal.c
//! \brief : BCM (Base Crypto Module) driver
//! \author: Yong Shen
//! \date  : Jan, 2014
//!  Note:
////////////////////////////////////////////////////////////////////////////////
#include "io.h"
#include "bcm_mailbox.h"
#include "bcm_primitive.h"
#include "bcm_status.h"
#include "timer.h"

#ifdef CONFIG_DCACHE
#include "cache.h"
#endif

#define bcm_display(...)

#define devmem_phy_to_virt(x) (x)

void bcm_memclr(void *p, int numInt)
{
        volatile unsigned int *pStart = (unsigned int *)p;
        volatile unsigned int *pEnd = pStart + numInt;
        while (pStart < pEnd)
                *pStart++ = 0;
}

void bcm_clear_command(BCM_MAILBOX_COMMAND * pCmd)
{
        bcm_memclr(pCmd, sizeof(BCM_MAILBOX_COMMAND)/sizeof(int));
}

void bcm_clear_status(BCM_MAILBOX_STATUS * pStatus)
{
        bcm_memclr(pStatus, sizeof(BCM_MAILBOX_STATUS)/sizeof(int));
}

void bcm_clear_complete_status()
{
	volatile _BCM_MAILBOX * biu = (_BCM_MAILBOX *)devmem_phy_to_virt(BCM_MAILBOX_BASE);
	unsigned int int_status = biu->host_interrupt_register;

	if(int_status & 0x1)
		biu->host_interrupt_register = 0x1 << 0;
}

static void bcm_send_command(const BCM_MAILBOX_COMMAND *pCmd)
{
        volatile _BCM_MAILBOX * biu = (_BCM_MAILBOX *)devmem_phy_to_virt(BCM_MAILBOX_BASE);
#ifdef CONFIG_DCACHE
	flush_dcache_all();
#endif
        biu->command_parameter0  = pCmd->arg0;
        biu->command_parameter1  = pCmd->arg1;
        biu->command_parameter2  = pCmd->arg2;
        biu->command_parameter3  = pCmd->arg3;
        biu->command_parameter4  = pCmd->arg4;
        biu->command_parameter5  = pCmd->arg5;
        biu->command_parameter6  = pCmd->arg6;
        biu->command_parameter7  = pCmd->arg7;
        biu->command_parameter8  = pCmd->arg8;
        biu->command_parameter9  = pCmd->arg9;
        biu->command_parameter10 = pCmd->arg10;
        biu->command_parameter11 = pCmd->arg11;
        biu->command_parameter12 = pCmd->arg12;
        biu->command_parameter13 = pCmd->arg13;
        biu->command_parameter14 = pCmd->arg14;
        biu->command_parameter15 = pCmd->arg15;
        biu->secure_processor_command = pCmd->command;
        bcm_display("Sending BCM command, command=0x%x\n", pCmd->command);
}


static int bcm_wait_for_complete(int wait_count)
{
        volatile _BCM_MAILBOX * biu = (_BCM_MAILBOX *)devmem_phy_to_virt(BCM_MAILBOX_BASE);
        volatile unsigned int intr_stat;
		UNUSED(wait_count);
        while(1) {
                intr_stat = biu->host_interrupt_register; // 0xc8 Write-to-Clear
                if(intr_stat & 1)
                        break;
		udelay(1000);
        }

        return 0;
}

static void bcm_get_status(BCM_MAILBOX_STATUS *pStatus)
{
        volatile _BCM_MAILBOX * biu = (_BCM_MAILBOX *)devmem_phy_to_virt(BCM_MAILBOX_BASE);

        pStatus->return_status = biu->command_return_status;
        pStatus->arg0  = biu->command_status0;
        pStatus->arg1  = biu->command_status1;
        pStatus->arg2  = biu->command_status2;
        pStatus->arg3  = biu->command_status3;
        pStatus->arg4  = biu->command_status4;
        pStatus->arg5  = biu->command_status5;
        pStatus->arg6  = biu->command_status6;
        pStatus->arg7  = biu->command_status7;
        pStatus->arg8  = biu->command_status8;
        pStatus->arg9  = biu->command_status9;
        pStatus->arg10 = biu->command_status10;
        pStatus->arg11 = biu->command_status11;
        pStatus->arg12 = biu->command_status12;
        pStatus->arg13 = biu->command_status13;
        pStatus->arg14 = biu->command_status14;
        pStatus->arg15 = biu->command_status15;

		//FIXME
        //if(pStatus->return_status!=0)
        //        bcm_display("!!!!!!BCM Return ERROR, return_status=0x%x!!!!!\n", pStatus->return_status);
}

void bcm_clear_interrupts(unsigned int mask)
{
        volatile _BCM_MAILBOX * biu = (_BCM_MAILBOX *)devmem_phy_to_virt(BCM_MAILBOX_BASE);
        biu->host_interrupt_register = 0x70001;

		UNUSED(mask);
}

void execute_bcm_command(const BCM_MAILBOX_COMMAND *pCmd, BCM_MAILBOX_STATUS *pStatus)
{
        bcm_send_command(pCmd);
        bcm_wait_for_complete(0);
        bcm_get_status(pStatus);
        bcm_clear_interrupts(~0);
}

int bcm_wait_for_boot()
{
        volatile _BCM_MAILBOX * biu = (_BCM_MAILBOX *)devmem_phy_to_virt(BCM_MAILBOX_BASE);
        unsigned int intr_stat;
        unsigned int fifo_stat;
        unsigned int cmd_stat;
	(void)cmd_stat;

        while(1) {
                intr_stat = biu->host_interrupt_register; // 0xc8 Write-to-Clear
                fifo_stat = biu->command_fifo_status; // 0xc4 Read Only

                if( (intr_stat & 0x00010001) != 0) {
                        cmd_stat = biu->command_return_status; // 0x80 Read Only
                        bcm_display("Error: BCM boot fail. intr=%0x status=%0x\n", intr_stat, cmd_stat);
                        return -1;
                }

                if (fifo_stat & 0x100) {
                        bcm_display("\n=========================================");
                        bcm_display("BCM booted successfully. status=%0x", fifo_stat);
                        bcm_display("=========================================\n");
                        break;
                }
        }

        return 0;
}

int bcm_usb_console_func(const unsigned int primitive_id)
{
#ifndef USB_BOOT
	BCM_MAILBOX_COMMAND cmd;
	BCM_MAILBOX_STATUS status;

	bcm_wait_for_boot();
	bcm_clear_command(&cmd);
	bcm_clear_status(&status);
	cmd.command = primitive_id;
	execute_bcm_command(&cmd, &status);
	if(status.return_status != STATUS_SUCCESS)
		return -1;
#endif

	return 0;
}
//clear fields not used in cmd before call.
int bcm_usb_boot_func(const BCM_MAILBOX_COMMAND* cmd)
{
	BCM_MAILBOX_STATUS status;

	bcm_clear_status(&status);
	bcm_clear_complete_status();
	execute_bcm_command(cmd, &status);

	return status.return_status;
}

/*
 *  Get random data
 *
 *  Arguments:
 * - buf   (IN/OUT) -  DDR buf to hold the output of random data
 * - size  (IN) -  size of random data in bytes.
 * Return:
 *         0 if success,  or one of the error code
 */
int bcm_generate_random_number(unsigned char *buf, const unsigned int size)
{
       BCM_MAILBOX_COMMAND cmd;
       BCM_MAILBOX_STATUS status;

       bcm_wait_for_boot();
       bcm_clear_command(&cmd);
       bcm_clear_status(&status);
       cmd.command = 0x4003; //DRM_GET_RANDOM; Cash Hacking
       cmd.arg0 = (unsigned int)((uintmax_t)buf);
       cmd.arg1 = size;
       execute_bcm_command(&cmd, &status);
       if(status.return_status != STATUS_SUCCESS)
               return -1;

       return 0;
}
