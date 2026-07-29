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
#include "io.h"
#include "debug.h"
#include "chip_voltage_info.h"
#include "timer.h"
#include "pv_comp.h"
#include "memmap.h"
#include "pwm.h"
#include "pmic_config.h"

#define MAX_PWM_CH 3

extern unsigned int vout_core_ch, vout_cpu_ch;

static void dutyCyclePwm(unsigned int ch, unsigned int duty)
{
	REG_WRITE32((MEMMAP_PWM_REG_BASE + RA_PWM_pwmCh0Duty + (0x10 * ch)), duty);
	dbg_printf(PRN_INFO, "pwm write val = 0x%x\n", duty);
}

static void enablePwm(unsigned int ch, unsigned int en)
{
	REG_WRITE32((MEMMAP_PWM_REG_BASE + RA_PWM_pwmCh0En + (0x10 * ch)), en);
	dbg_printf(PRN_INFO, "set PWM%d Enable REG (0x%08x) to 0x%x\n",ch,(MEMMAP_PWM_REG_BASE +RA_PWM_pwmCh0En+(0x10*ch)), en);
}

static void terminalCountPwm(unsigned int ch, unsigned int tcnt)
{
	REG_WRITE32((MEMMAP_PWM_REG_BASE + RA_PWM_pwmCh0TCnt + (0x10 * ch)), tcnt);
		dbg_printf(PRN_INFO, "set PWM%d terminal count register (0x%08x) to 0x%08x\n", ch, (MEMMAP_PWM_REG_BASE +RA_PWM_pwmCh0TCnt + (0x10*ch)), tcnt);

}

static void preScalePwm(unsigned int ch, unsigned int scale, unsigned int pol)
{
	REG_WRITE32((MEMMAP_PWM_REG_BASE + RA_PWM_pwmCh0Ctrl + (0x10 * ch)), (scale | (pol << 3)));
	dbg_printf(PRN_INFO, "set PWM%d control register (0x%08x) to 0x%08x\n",  ch, (MEMMAP_PWM_REG_BASE +RA_PWM_pwmCh0Ctrl+(0x10*ch)), (scale | pol << 3));
}

static int pwm_pmic_init(unsigned int pwm_ch)
{
	unsigned int TCount = 0;
	unsigned int preScale = 0x0;
	unsigned int pol = 0;

	if (pwm_ch == vout_core_ch) {
		if (g_pmic_config->vcore_pwm_freq_hz == 0) {
			dbg_printf(PRN_ERR, "Vcore invalid PWM frequency\n");
			return -1;
		}
		TCount = PWM_CLOCK / g_pmic_config->vcore_pwm_freq_hz;
		pol = g_pmic_config->vcore_pwm_polarity;
	} else if (pwm_ch == vout_cpu_ch) {
		if (g_pmic_config->vcpu_pwm_freq_hz == 0) {
			dbg_printf(PRN_ERR, "Vcpu invalid PWM frequency\n");
			return -1;
		}
		TCount = PWM_CLOCK / g_pmic_config->vcpu_pwm_freq_hz;
		pol = g_pmic_config->vcpu_pwm_polarity;
	}

	enablePwm(pwm_ch, 1);
	terminalCountPwm(pwm_ch, TCount);
	preScalePwm(pwm_ch, preScale, pol);

	return 0;
}

static void get_pmic_range(int pwm_ch, UNSG32 *v_min, UNSG32 *v_max,
			    UNSG32 *d_min, UNSG32 *d_max)
{
	if (pwm_ch == (int)vout_core_ch) {
		*v_min = g_pmic_config->vcore_pwm_volt_min_uv;
		*v_max = g_pmic_config->vcore_pwm_volt_max_uv;
		*d_min = g_pmic_config->vcore_pwm_duty_at_volt_min;
		*d_max = g_pmic_config->vcore_pwm_duty_at_volt_max;
	} else {
		*v_min = g_pmic_config->vcpu_pwm_volt_min_uv;
		*v_max = g_pmic_config->vcpu_pwm_volt_max_uv;
		*d_min = g_pmic_config->vcpu_pwm_duty_at_volt_min;
		*d_max = g_pmic_config->vcpu_pwm_duty_at_volt_max;
	}
}

static int pwm_pmic_volt2duty(int pwm_ch, int volt_uv)
{
	UNSG32 v_min, v_max, d_min, d_max;
	SIGN64 duty;

	get_pmic_range(pwm_ch, &v_min, &v_max, &d_min, &d_max);

	if (volt_uv < (int)v_min)
		volt_uv = (int)v_min;

	if (volt_uv > (int)v_max)
		volt_uv = (int)v_max;

	if (v_max == v_min)
		return -1;

	duty = (SIGN64)d_min +
		((SIGN64)(volt_uv - (SIGN64)v_min) *
		((SIGN64)d_max - (SIGN64)d_min)) /
		((SIGN64)v_max - (SIGN64)v_min);
	
	return (int)duty;
}

static int pwm_pmic_set_vol(int pwm_ch, int volt_uv)
{
	int duty;

	if (pwm_ch > MAX_PWM_CH) {
		dbg_printf(PRN_ERR, "MAX_PWM_CH is %d\n", MAX_PWM_CH);
		return -1;
	}

	if (pwm_pmic_init(pwm_ch))
		return -1;

	duty = pwm_pmic_volt2duty(pwm_ch, volt_uv);
	if (duty < 0) {
		dbg_printf(PRN_ERR, "Unsupported voltage: %d uV\n", volt_uv);
		return -1;
	}

	dutyCyclePwm(pwm_ch, (unsigned int)duty);
	dbg_printf(PRN_INFO, "set voltage: %d uV, duty: %d\n", volt_uv, duty);
	mdelay(1); // Wait for voltage settle
	return 0;
}

static int pwm_set_vcpu_volt(int from, int to)
{
	(void)from;
	return pwm_pmic_set_vol(vout_cpu_ch, to);
}

static int pwm_set_vcore_volt(int from, int to)
{
	(void)from;
	return pwm_pmic_set_vol(vout_core_ch, to);
}

static int pwm_get_cpu_volt(void)
{
	// Readback from PWM register is unreliable at boot.
	return 0;
}

static int pwm_get_core_volt(void)
{
	return 0;
}

const dvfs_ops_t pwm_pmic_ops = {
	.get_vcpu_volt  = pwm_get_cpu_volt,
	.get_vcore_volt = pwm_get_core_volt,
	.set_vcpu_volt  = pwm_set_vcpu_volt,
	.set_vcore_volt = pwm_set_vcore_volt,
	.pwm_type = 1,
};
