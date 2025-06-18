/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright © 2013-2020 Synaptics Incorporated. All rights reserved.
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
#include "sm_type.h"
#include "sm_io.h"
#include "sm_common.h"
#include "sm_memmap.h"
#include "sm_rt_module.h"

#include "sm_printf.h"
#include "sm_exception.h"
#include "sm_timer.h"
#include "sm_apb_timer.h"
#include "sm_apb_timer_defs.h"

#include "platform_config.h"


static UINT32 uptime_calibrator = 0;
#ifdef LTIMER_ENABLE
static UINT32 uptimeLong_calibrator = 0;
#endif /* LTIMER_ENABLE */
static INT32 g_sys_timer_id = 0;
static INT32 g_timer_min_id = 0;
static INT32 g_timer_max_id = 0;

typedef struct {
	BOOL is_freerun;
	BOOL is_used;
	void (*callback)(void);
}timer_resource_t;

static timer_resource_t timer_res[TIMER_ID_MAX];

INT32 sm_apb_timer_stop(INT32 timerid);

INT32 sm_apb_timer_gettimems(void);

void sm_apb_timer_init(INT32 min_id, INT32 max_id)
{
	INT32 i = 0;

	g_timer_min_id = min_id;
	g_timer_max_id = max_id;

	for(i = (min_id - 1); i < max_id; i++) {
		sm_apb_timer_stop(i + 1);
		timer_res[i].is_freerun = FALSE;
		timer_res[i].is_used = FALSE;
		timer_res[i].callback = NULL;
	}
}

void sm_apb_set_sys_timer_id(INT32 sys_timer_id)
{
	if (sys_timer_id < g_timer_min_id || sys_timer_id > g_timer_max_id) {
		PRT_ERROR("%s - arg check fail!", __func__);
	}
	g_sys_timer_id = sys_timer_id;
}

INT32 sm_apb_get_sys_timer_id(void)
{
	return g_sys_timer_id;
}

INT32 sm_apb_timer_reserve(INT32 timerid)
{
	if((timerid >= g_timer_min_id) && (timerid <= g_timer_max_id)
		&& (!timer_res[timerid - 1].is_used)) {
			timer_res[timerid - 1].is_used = TRUE;
		return timerid;
	}
	return -1;
}

INT32 sm_apb_timer_getintstatus(INT32 timerid)
{
	if (timerid < g_timer_min_id || timerid > g_timer_max_id) {
		PRT_ERROR("%s - arg check fail!", __func__);
		return -1;
	}
	return MV_SM_READ_REG32(TIMER_INTSTAT(timerid));
}

INT32 sm_apb_timer_clearint(INT32 timerid)
{
	if (timerid < g_timer_min_id || timerid > g_timer_max_id) {
		PRT_ERROR("%s - arg check fail!", __func__);
		return -1;
	}
	return MV_SM_READ_REG32(TIMER_EOI(timerid));
}

INT32 sm_apb_timer_readclock(void)
{
	return SM_SYSTEM_HZ*1000*1000-MV_SM_READ_REG32(TIMER_CURRENTVAL(g_sys_timer_id));
}

INT32 sm_apb_timer_gettimems(void)
{
	INT32 timems = 0;

	timems = sm_apb_timer_readclock() / (SM_SYSTEM_HZ * 1000);

	/*
	 * If we've just missed a timer overflow, then it is necessary to check
	 * the interrupt status to ensure the time we get is reliable.
	 */
	if (timems < 1)
		if (MV_SM_READ_REG32(TIMER_INTSTAT(g_sys_timer_id)) & 0x1)
			timems += INT_MS;

	timems += uptime_calibrator;

	return timems;
}

INT32 sm_apb_timer_getlongtimems(timer_long_t *ret)
{
#ifdef LTIMER_ENABLE
	INT32 timems = 0;
	UINT32 left = 0;

	if (ret == NULL)
		return S_FALSE;

	memset(ret, 0, sizeof(timer_long_t));

	timems = sm_apb_timer_readclock() / (SM_SYSTEM_HZ * 1000);

	/*
	 * If we've just missed a timer overflow, then it is necessary to check
	 * the interrupt status to ensure the time we get is reliable.
	 */
	if (timems < 1)
		if (MV_SM_READ_REG32(TIMER_INTSTAT(g_sys_timer_id)) & 0x1)
			timems += INT_MS;

	ret->low += timems;
	left = MAX_TIMER_MS - ret->low;
	if (left > uptime_calibrator)
	{
		ret->low += uptime_calibrator;
	}
	else
	{
		ret->low = uptime_calibrator - left - 1;
		ret->high++;
	}
	ret->high += uptimeLong_calibrator;

	return S_OK;
#else /* LTIMER_ENABLE */
	return S_FALSE;
#endif /* !LTIMER_ENABLE */
}

static INT32 sm_apb_timer_setctrl(INT32 timerid, UINT32 mask, UINT32 mode, UINT32 enable)
{
	UINT32 val = 0;

	if (timerid < g_timer_min_id || timerid > g_timer_max_id
			|| mask > 1
			|| mode > 1
			|| enable > 1) {
		PRT_ERROR("%s - arg check fail!", __func__);
	}

	val = mask << 2 | mode << 1;
	MV_SM_WRITE_REG32(TIMER_CONTROLREG(timerid), val);
	val |= enable;
	MV_SM_WRITE_REG32(TIMER_CONTROLREG(timerid), val);

	return S_OK;
}

static INT32 sm_apb_timer_setloadcount(INT32 timerid, INT32 loadCount)
{
	if (timerid < g_timer_min_id || timerid > g_timer_max_id) {
		PRT_ERROR("%s - arg check fail!", __func__);
		return S_FALSE;
	}
	MV_SM_WRITE_REG32(TIMER_LOADCOUNT(timerid), loadCount);
	return S_OK;
}

// this function is used to get an unused timer
// if you can confirm a timer is not used, you can ignore this function
INT32 sm_apb_timer_request(void)
{
	INT32 i = 0;
	for(i = 0; i < g_timer_max_id; i++) {
		if(!timer_res[i].is_used)
			break;
	}
	return (i == g_timer_max_id) ? -1 : (i + 1);
}

INT32 sm_apb_timer_start(INT32 timerid, INT32 timeoutus, void (*cb)(void), BOOL is_freerun)
{
	INT32 mode = 0;

	if (timerid < g_timer_min_id || timerid > g_timer_max_id || cb == NULL) {
		PRT_ERROR("%s - arg check fail!", __func__);
		return S_FALSE;
	}

	// TODO: need to check timeout value

	if(timer_res[timerid - 1].is_used == FALSE) {

		if(is_freerun)
			mode = 0;
		else
			mode = 1;

		timer_res[timerid - 1].callback = cb;
		timer_res[timerid - 1].is_freerun = is_freerun;

		sm_apb_timer_setctrl(timerid, 0, mode, 0);
		if(timeoutus >= (0xFFFFFFFF / SM_SYSTEM_HZ))
			sm_apb_timer_setloadcount(timerid, 0xFFFFFFFF);
		else
			sm_apb_timer_setloadcount(timerid, TIMER_US(timeoutus));

		timer_res[timerid - 1].is_used = TRUE;

		sm_apb_timer_setctrl(timerid, 0, mode, 1);
	} else {
		PRT_WARN("timer %d has been started and not stopped yet", timerid);
		return S_FALSE;
	}
	return S_OK;
}

INT32 sm_apb_timer_stop(INT32 timerid)
{
	sm_apb_timer_setctrl(timerid, 0, 1, 0);
	timer_res[timerid - 1].is_used = FALSE;
	timer_res[timerid - 1].is_freerun = FALSE;
	/* cannot clear callback since it may be stopped before callback */
	/*timer_res[timerid - 1].callback = FALSE;*/
	return S_OK;
}

static void __attribute__((used)) sm_apb_timer_isr(void)
{
	INT32 id = 0;

	for (id = g_timer_min_id; id <= g_timer_max_id; id++)
	{
		if(sm_apb_timer_getintstatus(id) & 0x1) {
			/* make sure the timer was registered. otherwise, clear int directly */
			if(timer_res[id - 1].is_used == TRUE) {
				if (timer_res[id - 1].callback != NULL) {
					timer_res[id - 1].callback();
				} else {
					PRT_WARN("timer %d interrupt ignored\n", id);
				}

				sm_apb_timer_clearint(id);
				return;
			}
		}
	}

	return;
}
DECLARE_ISR(SMICTL_IRQ_TIMER0_0, sm_apb_timer_isr);

// timer 1 interrupt, used for generic system uptime counter
void sm_apb_timer_isr_timer1(void)
{
	//MV_SM_READ_REG32(TIMER_EOI(SYS_TIMER_ID));

#ifdef LTIMER_ENABLE
	if ((MAX_TIMER_MS - uptime_calibrator) < INT_MS)
	{
		uptimeLong_calibrator++;
		uptime_calibrator = 0;
		return;
	}
#endif /* LTIMER_ENABLE */

	uptime_calibrator += INT_MS;
}

//static int __attribute__((used)) apb_timer_init(void)
static int __attribute__((used)) apb_timer_init(void)
{
	sm_apb_timer_init(TIMER_ID_IDX_START, TIMER_ID_IDX_END);
#ifdef RESERVE_TIMER_ID
	sm_apb_timer_reserve(RESERVE_TIMER_ID);
#endif

	//we'd better specify a timer for such important usage
	sm_apb_set_sys_timer_id(SYS_TIMER_ID);
	sm_apb_timer_start(SYS_TIMER_ID, 1000*1000, sm_apb_timer_isr_timer1, 0);

	return S_OK;
}

DECLARE_RT_INIT(apbinit, INIT_DEV_P_25, apb_timer_init);


