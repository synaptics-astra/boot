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
#include "io.h"
#include "debug.h"
#include "chip_voltage_info.h"
#include "timer.h"
#include "pv_comp.h"
#include "memmap.h"
#include "pwm.h"

#ifndef VOUT_CPU_CH
#define VOUT_CPU_CH                     (3)
#endif

#ifndef VOUT_CORE_CH
#define VOUT_CORE_CH                    (2)
#endif

// all defined under uv
#define FP6359_STEPuV 304
#define MIN_VOLTAGE 700000
#define MAX_VOLTAGE 1000000
#define MAX_PWM_CH 4

#define RSVD		0xFFFFFFFF

static int dutyCyclePwmRead(unsigned int ch)
{
	unsigned int duty;

	REG_READ32((MEMMAP_PWM_REG_BASE + RA_PWM_pwmCh0Duty + (0x10 * ch)), &duty);
	dbg_printf(PRN_INFO, "pwm read val = 0x%x\n", duty);

	return duty;
}

void dutyCyclePwm(unsigned int ch, unsigned int duty)
{
	REG_WRITE32((MEMMAP_PWM_REG_BASE + RA_PWM_pwmCh0Duty + (0x10 * ch)), duty);
	dbg_printf(PRN_INFO, "pwm write val = 0x%x\n", duty);
}

void enablePwm(unsigned int ch, unsigned int en)
{
	REG_WRITE32((MEMMAP_PWM_REG_BASE + RA_PWM_pwmCh0En + (0x10 * ch)), en);
	dbg_printf(PRN_INFO, "set PWM%d Enable REG (0x%08x) to 0x%x\n",ch,(MEMMAP_PWM_REG_BASE +RA_PWM_pwmCh0En+(0x10*ch)),(en << 31));
}

void terminalCountPwm(unsigned int ch, unsigned int tcnt)
{
	REG_WRITE32((MEMMAP_PWM_REG_BASE + BA_PWM_pwmCh0TCnt_pwmCh0TCnt + (0x10 * ch)), tcnt);
	dbg_printf(PRN_INFO, "set PWM%d terminal count register (0x%08x) to 0x%08x\n",ch, (MEMMAP_PWM_REG_BASE +RA_PWM_pwmCh0TCnt+(0x10*ch)), tcnt);
}

void preScalePwm(unsigned int ch, unsigned int scale, unsigned pol)
{
	REG_WRITE32((MEMMAP_PWM_REG_BASE + RA_PWM_pwmCh0Ctrl + (0x10 * ch)), (scale | pol));
	dbg_printf(PRN_INFO, "set PWM%d control register (0x%08x) to 0x%08x\n",  ch, (MEMMAP_PWM_REG_BASE +RA_PWM_pwmCh0Ctrl+(0x10*ch)), (scale | pol));
}

void fp6359_init(unsigned int pwm_ch)
{
	unsigned int TCount = 0x3E8;
	unsigned int preScale = 0x0;

	enablePwm(pwm_ch, 1);
	terminalCountPwm(pwm_ch, TCount);
	preScalePwm(pwm_ch, preScale, 0);
}

static int fp6359_pmic_volt2data(int volt, int *p_data)
{
	unsigned int data;

	if ((volt >= MIN_VOLTAGE) && (volt <= MAX_VOLTAGE))
		data = (MAX_VOLTAGE - volt) / FP6359_STEPuV;
	else
		data = RSVD;

	if (data == RSVD)
		return -1;
	else {
		*p_data = data;
		return 0;
	}
}

static int fp6359_pmic_data2volt(int *p_volt, int vdata)
{
	const unsigned int max_data = 0x3E8;
	unsigned int volt;
	unsigned int data = vdata;

	if (data <= max_data)
		volt = MAX_VOLTAGE - (data * FP6359_STEPuV);
	else
		volt = RSVD;

	if (volt == RSVD)
		return -1;
	else {
		*p_volt = volt;
		return 0;
	}
}

static int fp6359_set_vol(int pwm_ch, int vol)
{
	int data, ret;
	unsigned int target_uv = vol;

	if (pwm_ch > MAX_PWM_CH) {
		dbg_printf(PRN_ERR, "MAX_PWM_CH is %d\n", MAX_PWM_CH);
		return -1;
	}

	fp6359_init(pwm_ch);

	ret = fp6359_pmic_volt2data(target_uv, &data);
	if (ret != 0)
	{
		dbg_printf(PRN_ERR, "Don't support %d uV\n", target_uv);
		return ret;
	}

	dutyCyclePwm(pwm_ch, data);

	return ret;
}


static int fp6359_get_vol(int pwm_ch)
{
	int volt, data, ret;

	if (pwm_ch > MAX_PWM_CH) {
		dbg_printf(PRN_ERR, "MAX_PWM_CH is %d\n", MAX_PWM_CH);
		return -1;
	}

	fp6359_init(pwm_ch);

	data = dutyCyclePwmRead(pwm_ch);

	ret = fp6359_pmic_data2volt(&volt, data);
	if(ret) {
		dbg_printf(PRN_ERR, "pwm register read fail\n");
		return -1;
	}
	return volt;
}

static int i2c_get_cpu_volt(void)
{
	int volt = fp6359_get_vol(VOUT_CPU_CH);

	if(volt < 0)
		return 0;

	return volt;
}

static int i2c_get_core_volt(void)
{
	int volt = fp6359_get_vol(VOUT_CORE_CH);

	if(volt < 0)
		return 0;

	return volt;
}

static int i2c_set_volt(int pwm_ch, int from, int to)
{
	int volt = from;

	while (volt != to){
		if (volt > to){
			volt -= FP6359_STEPuV;
			if (volt < to)
				volt = to;
		}else{
			volt += FP6359_STEPuV;
			if (volt > to)
				volt = to;
		}

		dbg_printf(PRN_INFO, "setting voltage: %d\n", volt);
		if(0 != fp6359_set_vol(pwm_ch, volt))
			return 1;
	}
dutyCyclePwmRead(pwm_ch);
	return 0;
}

static int i2c_set_vcpu_volt(int from, int to)
{
	return i2c_set_volt(VOUT_CPU_CH, from ,to);
}

static int i2c_set_vcore_volt(int from, int to)
{
	return i2c_set_volt(VOUT_CORE_CH, from ,to);
}

const dvfs_ops_t fp6359_ops = {
	.get_vcpu_volt = i2c_get_cpu_volt,
	.get_vcore_volt = i2c_get_core_volt,
	.set_vcpu_volt = i2c_set_vcpu_volt,
	.set_vcore_volt =i2c_set_vcore_volt
};

