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
#ifdef WOLENABLE
#include "sm_type.h"
#include "sm_io.h"
#include "sm_common.h"
#include "sm_memmap.h"
#include "sm_ictl.h"

#include "sm_printf.h"

#include "sm_power.h"

#include "sm_apb_ictl_defs.h"

#include "sm_rt_module.h"
#include "sm_exception.h"
#include "sm_state.h"

#define WOL_INT_MAGIC_PATTERN 0x03
#define WOL_INT_INVALID 0xFF

#define WOL_MAC_LEN 6

static UINT8 int_type;

static UINT8 mv_sm_wol_get_int_type(void)
{
	return int_type;
}


/**********************************************************************
 * FUNCTION	: Enable/Disalbe magic pattern interrupt
 * PARAMETERS:
 *    en: 1: enable; 0: disable
 * RETURN:
 *    S_OK: success; S_FALSE: fail
 *********************************************************************/
static UINT32 mv_sm_wol_mp_intenable(BOOL en)
{
	UINT32 val;
	UINT32 mask;

	val = MV_SM_READ_REG32(APB_ICTL_IRQ_INTEN);
	mask = MV_SM_READ_REG32(APB_ICTL_IRQ_INTMASK);
	PRT_DEBUG("APB_ICTL_IRQ_INT(EN,MASK): 0x%x 0x%x-> ", val, mask);

	if (en) {
		// Enable WOL interrupt.
		val |= (1 << SMICTL_IRQ_SMI_MGP_INT_N);
		mask &= ~(1 << SMICTL_IRQ_SMI_MGP_INT_N);
		int_type = WOL_INT_MAGIC_PATTERN;
	} else {
		// Disable WOL interrupt.
		val &= ~(1 << SMICTL_IRQ_SMI_MGP_INT_N);
		mask |= (1 << SMICTL_IRQ_SMI_MGP_INT_N);
		int_type = WOL_INT_INVALID;
	}

	MV_SM_WRITE_REG32(APB_ICTL_IRQ_INTEN, val);
	MV_SM_WRITE_REG32(APB_ICTL_IRQ_INTMASK, mask);

	PRT_DEBUG("0x%x 0x%x\n", val, mask);

	return S_OK;
}

static void __attribute__((used)) mv_sm_wol_isr(void)
{
	UINT8 int_type = mv_sm_wol_get_int_type();
	INT32 state = mv_sm_get_state();

	/* FIXME: no  MV_SM_STATE_LOWPOWERSTANDBY because eth phy is closed */
	if(((MV_SM_STATE_LOWPOWERSTANDBY == state) ||
		(MV_SM_STATE_NORMALSTANDBY == state) ||
		(MV_SM_STATE_SUSPEND == state)) &&
		(int_type != WOL_INT_INVALID)) {

		if(mv_sm_power_enterflow_bysmstate() == S_OK) {
			PRT_INFO("Wake up from LAN\n");
			mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_WOL);
			mv_sm_wol_mp_intenable(FALSE); // Disable wake up
		}

		//FIXME: Clear interrupt
	}

	return;
}

DECLARE_ISR(SMICTL_IRQ_SMI_MGP_INT_N, mv_sm_wol_isr);

static int __attribute__((used)) sm_process_internal_wol_msg(void * data, INT32 len)
{
	UINT32 *msg = (UINT32*)data;

	//PRT_INFO("led internal msg %x, %x, %x\n", msg[0], msg[1], msg[2]);
	if(msg[0] == MV_SM_ID_POWER) {
		switch(msg[1]) {
		//case FLOW_COLD_2_LOWPOWERSTANDBY:
		case FLOW_ACTIVE_2_LOWPOWERSTANDBY:
		case FLOW_ACTIVE_2_NORMALSTANDBY:
		case FLOW_ACTIVE_2_SUSPEND:
			if(msg[2] == STA_LEAVEFLOW) {
				mv_sm_wol_mp_intenable(TRUE);
			}
			break;
		default:
			break;
		}
	}
	return S_OK;
}

static int __attribute__((used)) sm_process_wol_msg(void *data, INT32 len)
{
	char *msg = (char *)data;
	UINT32 mac_low = 0, mac_high = 0;

	if (len != WOL_MAC_LEN) {
			PRT_ERROR("wol: MAC address with invalid length, ignored.\n");
			return S_OK;
	}

	mac_low |= msg[0] & 0xff;
	mac_low |= (msg[1] << 8) & 0xff00;
	mac_low |= (msg[2] << 16) & 0xff0000;
	mac_low |= (msg[3] << 24) & 0xff000000;
	mac_high |= msg[4] & 0xff;
	mac_high |= (msg[5] << 8) & 0xff00;

	PRT_INFO("Set wol MAC address %02x:%02x:%02x:%02x:%02x:%02x\n",
			msg[0]&0xff, msg[1]&0xff, msg[2]&0xff,
			msg[3]&0xff, msg[4]&0xff, msg[5]&0xff);

	return S_OK;
}

DECLARE_RT_MODULE(
	wol,
	MV_SM_ID_WOL,
	NULL,
	sm_process_wol_msg,
	sm_process_internal_wol_msg
);

static int __attribute__((used)) sm_wol_init(void)
{
	int_type = WOL_INT_INVALID;

	return S_OK;
}

DECLARE_RT_INIT(wolinit, INIT_DEV_P_50, sm_wol_init);


#endif /* WOLENABLE */
