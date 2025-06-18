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
#ifdef LEDENABLE
#include "sm_type.h"

#include "platform_config.h"

#include "sm_common.h"

#include "sm_power.h"
#include "sm_timer.h"
#include "sm_printf.h"
//#include "sm_debug.h"

#include "sm_gpio.h"
#include "sm_rt_module.h"

#ifdef SM_GPIO_PORT_LED
static int mv_sm_led_ctrl(BOOL on)
{
	if(on) {
		//turn on LED,GPIO 13=PORTB bit 5=0
		sm_gpio_set_output(SM_GPIO_PORT_LED, 0);
	} else {
		//turn on LED,GPIO 13=PORTB bit 5=0
		sm_gpio_set_output( SM_GPIO_PORT_LED, 1);
	}
	return S_OK;
}

static int __attribute__((used)) mv_sm_process_internal_led_msg(void * data, INT32 len)
{
	UINT32 *msg = (UINT32*)data;

	//PRT_INFO("led internal msg %x, %x, %x\n", msg[0], msg[1], msg[2]);
	if(msg[0] == MV_SM_ID_POWER) {
		switch(msg[1]) {
		case FLOW_COLD_2_LOWPOWERSTANDBY:
		case FLOW_ACTIVE_2_LOWPOWERSTANDBY:
		case FLOW_ACTIVE_2_NORMALSTANDBY:
		case FLOW_ACTIVE_2_SUSPEND:
			if(msg[2] == STA_LEAVEFLOW)
				mv_sm_led_ctrl(1);
			break;
		case FLOW_LOWPOWERSTANDBY_2_ACTIVE:
		case FLOW_NORMALSTANDBY_2_ACTIVE:
		case FLOW_SUSPEND_2_ACTIVE:
			if(msg[2] == STA_ENTERFLOW)
				mv_sm_led_ctrl(0);
			break;
		case FLOW_SYSRESET_2_LOWPOWERSTANDBY:
		case FLOW_SYSRESET_2_NORMALSTANDBY:
		case FLOW_SYSRESET_2_ACTIVE:
			if(msg[2] == STA_POWEROFF)
				mv_sm_led_ctrl(1);
			else if(msg[2] == STA_POWEROK)
				mv_sm_led_ctrl(0);
			break;
		default:
			break;
		}
	} else {
		if((msg[1] == 0) || (msg[1] == 1))
			mv_sm_led_ctrl(msg[1]);
	}
	return S_OK;
}

DECLARE_RT_MODULE(
	led,
	MV_SM_ID_LED,
	NULL,
	NULL,
	mv_sm_process_internal_led_msg
);

#endif
#endif /* LEDENABLE */
