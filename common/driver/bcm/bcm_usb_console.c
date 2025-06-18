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

#include "bcm_usb_console.h"
#include "bcm_mailbox.h"
#include "bcm_status.h"

#define	BCM_PI_ENABLE_USB_CONSOLE	(0x000a)
#define	BCM_PI_DISABLE_USB_CONSOLE	(0x000b)

#ifdef _ENABLE_USB_CONSOLE

int bcm_enable_usb_console(void)
{
	BCM_MAILBOX_COMMAND cmd;
	BCM_MAILBOX_STATUS status;

	bcm_clear_command(&cmd);
	cmd.command = BCM_PI_ENABLE_USB_CONSOLE;

	bcm_clear_status(&status);
	execute_bcm_command(&cmd, &status);
	if(status.return_status != STATUS_SUCCESS)
		return -1;

	return 0;
}

int bcm_disable_usb_console(void)
{
	BCM_MAILBOX_COMMAND cmd;
	BCM_MAILBOX_STATUS status;

	bcm_clear_command(&cmd);
	cmd.command = BCM_PI_DISABLE_USB_CONSOLE;

	bcm_clear_status(&status);
	execute_bcm_command(&cmd, &status);
	if(status.return_status != STATUS_SUCCESS)
		return -1;

	return 0;
}

#endif /* _ENABLE_USB_CONSOLE */
