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
#ifdef FPSTBYENABLE
#include "FreeRTOS.h"
#include "task.h"

#include "sm_type.h"
#include "SysMgr.h"

#include "sm_state.h"

#include "sm_printf.h"

#include "sm_gpio.h"
#include "sm_power.h"

//FIXME: GPIO setting is defined in gpio_def.h
//#include "gpio_def.h"
#define SM_GPIO_PORT_FP_STBY 18

volatile BOOL fpbutton_wakeup = FALSE;

void fpbutton_wakeup_handler(void)
{
	if(fpbutton_wakeup) {
		mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_BUTTON);
		mv_sm_power_enterflow_bysmstate();
		fpbutton_wakeup = FALSE;
	}
}

void mv_sm_fpstby_isr(void)
{
        if(mv_sm_get_state() != MV_SM_STATE_ACTIVE){
                fpbutton_wakeup = TRUE;
        }

        sm_gpio_clearint(SM_GPIO_PORT_FP_STBY);
        PRT_INFO("FP STBY INT!\n");
}

DECLARE_GPIO_ISR(SM_GPIO_PORT_FP_STBY, EDGE_SENSITIVE, FALLING_EDGE, mv_sm_fpstby_isr);
#endif
