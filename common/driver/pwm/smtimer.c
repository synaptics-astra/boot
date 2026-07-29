/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright (C) 2013-2026 Synaptics Incorporated. All rights reserved.
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
#include "io.h"
#include "debug.h"
#include "chip_voltage_info.h"
#include "timer.h"
#include "pv_comp.h"
#include "memmap.h"
#include "pwm.h"
#include "pmic_select.h"
#include "pmic_config.h"
#include "apbRegBase.h"

extern unsigned int vout_core_smtimer_ch, vout_cpu_smtimer_ch;

static int get_pmic_params(unsigned int pwm_ch, UNSG32 *v_min, UNSG32 *v_max,
			    UNSG32 *d_min, UNSG32 *d_max, UNSG32 *tcount, UNSG32 *pol)
{
	if (pwm_ch == vout_core_smtimer_ch) {
		*v_min = g_pmic_config->vcore_pwm_volt_min_uv;
		*v_max = g_pmic_config->vcore_pwm_volt_max_uv;
		*d_min = g_pmic_config->vcore_pwm_duty_at_volt_min;
		*d_max = g_pmic_config->vcore_pwm_duty_at_volt_max;
		if (g_pmic_config->vcore_pwm_freq_hz == 0) {
			dbg_printf(PRN_ERR, "Vcore invalid PWM frequency\n");
			return -1;
		}
		*tcount = SM_TIMER_CLOCK / g_pmic_config->vcore_pwm_freq_hz;
		*pol = g_pmic_config->vcore_pwm_polarity;
	} else {
		*v_min = g_pmic_config->vcpu_pwm_volt_min_uv;
		*v_max = g_pmic_config->vcpu_pwm_volt_max_uv;
		*d_min = g_pmic_config->vcpu_pwm_duty_at_volt_min;
		*d_max = g_pmic_config->vcpu_pwm_duty_at_volt_max;
		if (g_pmic_config->vcpu_pwm_freq_hz == 0) {
			dbg_printf(PRN_ERR, "Vcpu invalid PWM frequency\n");
			return -1;
		}
		*tcount = SM_TIMER_CLOCK / g_pmic_config->vcpu_pwm_freq_hz;
		*pol = g_pmic_config->vcpu_pwm_polarity;
	}

	return 0;

}

static void sm_pwm_set_duty(unsigned int ch, unsigned int duty_l, unsigned int duty_h)
{
	REG_WRITE32((SM_APB_TIMER1_BASE + 0x14 * ch), duty_l - 1);
	REG_WRITE32((SM_APB_TIMER1_BASE + 0xb0 + 0x4 * ch), duty_h - 1);
	REG_WRITE32((SM_APB_TIMER1_BASE + 0x08 + 0x14 * ch), 0xb);
}

static int smtimer_set_voltage(unsigned int pwm_ch, int target_uv)
{
	int v_min;
	int v_max;
	int d_min;
	int d_max;
	int tcount;
	int pol;

	SIGN64 duty_calc;

	unsigned int duty_val;
	unsigned int duty_l;
	unsigned int duty_h;

	if (get_pmic_params(pwm_ch,
			    (UNSG32 *)&v_min,
			    (UNSG32 *)&v_max,
			    (UNSG32 *)&d_min,
			    (UNSG32 *)&d_max,
			    (UNSG32 *)&tcount,
			    (UNSG32 *)&pol))
		return -1;

	/* target voltage */
	if (target_uv < v_min)
		target_uv = v_min;

	if (target_uv > v_max)
		target_uv = v_max;

	if (v_max == v_min)
		return -1;

	/* Calculate PWM duty*/
	duty_calc = (SIGN64)d_min +
		(((SIGN64)target_uv - (SIGN64)v_min) *
		((SIGN64)d_max - (SIGN64)d_min)) /
		((SIGN64)v_max - (SIGN64)v_min);

	/* Limit duty range */
	if (duty_calc < 0)
		duty_calc = 0;

	if (duty_calc > tcount)
		duty_calc = tcount;

	duty_val = (unsigned int)duty_calc;

	/* PWM polarity invert */
	if (pol) {
		if (duty_val >= tcount)
			duty_val = 0;
		else
			duty_val = tcount - duty_val;
	}

	/* Convert duty to PWM register value */
	duty_h = duty_val + 1;


	if (duty_val >= tcount)
		duty_l = 1;
	else
		duty_l = tcount - duty_val + 1;

	sm_pwm_set_duty(pwm_ch, duty_l, duty_h);

	/* Wait voltage stable */
	mdelay(1);

	DBG("smtimer ch%d set %d uV, duty=%u/%d\n",
	    pwm_ch,
	    target_uv,
	    duty_val,
	    tcount);

	return 0;
}

static int smtimer_set_vcore_voltage(int from, int to)
{
	(void)from;
	return smtimer_set_voltage(vout_core_smtimer_ch, to);
}

static int smtimer_set_vcpu_voltage(int from, int to)
{
	(void)from;
	return smtimer_set_voltage(vout_cpu_smtimer_ch, to);
}

static int smtimer_get_vcore_voltage(void)
{
	return 0;
}

static int smtimer_get_vcpu_voltage(void)
{
	return 0;
}

const dvfs_ops_t smtimer_ops = {
	.get_vcpu_volt  = smtimer_get_vcpu_voltage,
	.get_vcore_volt = smtimer_get_vcore_voltage,
	.set_vcpu_volt  = smtimer_set_vcpu_voltage,
	.set_vcore_volt = smtimer_set_vcore_voltage,
	.pwm_type = 1,
};

