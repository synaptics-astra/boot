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
#ifdef ADCBTNENABLE
#include "sm_common.h"
#include "sm_type.h"
#include "sm_io.h"
#include "sm_timer.h"
#include "SysMgr.h"

#include "sm_rt_module.h"
#include "sm_ir_key_def.h"

#include "platform_config.h"

#include "sm_printf.h"
#include "sm_apb_timer.h"
#include "sm_timer.h"

#include "sm_state.h"
#include "sm_power.h"
#include "sm_comm.h"

#include "sm_adcbtn.h"

#define HIGH_LEV	800
#define LOW_LEV		220

#define BUTTON_FILTER1	15
#define BUTTON_FILTER2	20

#define ADC_SAMPLE_TIME	4

#ifdef CONFIG_FUNCTION_BUTTON_HIGH_LEV_VALID
#define BUTTON_PRESSED(val)	(val >= VOLTAGE_STAGE1_THRESHOLD)
#define BUTTON_RELEASED(val)	(val < VOLTAGE_STAGE1_THRESHOLD)
#else
#define BUTTON_PRESSED(val)	(val <= VOLTAGE_STAGE2_THRESHOLD)
#define BUTTON_RELEASED(val)	(val > VOLTAGE_STAGE2_THRESHOLD)
#endif

#define ADCBTN_STACK_SIZE ((uint16_t) 256)

const unsigned int adc_high[ADC_12BIT+1] =
{
	0x3f,
	0xff,
	0x3ff,
	0xfff,
};

const unsigned int adc_shift[ADC_12BIT+1] =
{
	6,
	4,
	2,
	0,
};

/*
 * start_time/end_time: Used for the first_time we detect button action.
 * first_time/last_time: The real time used for button action, in case of shake.
 */
static UINT32 start_time[ADC_INPUT_MAX];
static UINT32 end_time[ADC_INPUT_MAX];
static UINT32 first_time[ADC_INPUT_MAX];
static UINT32 last_time[ADC_INPUT_MAX];

static void sm_adc_enable(ADC_MODE mode)
{
	T32smSysCtl_SM_CTRL reg;

	//enable/disable
	reg.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_CTRL);
	reg.uSM_CTRL_ADC_RESET = 0;
	reg.uSM_CTRL_ADC_EN = mode;
	reg.uSM_CTRL_ADC_CONT = 0;
	reg.uSM_CTRL_ADC_SEL_VREF=0;
	if (mode == ADC_POWER_On)
		reg.uSM_CTRL_ADC_SELBG=1;
	else
		reg.uSM_CTRL_ADC_SELBG=0;
	reg.uSM_CTRL_ADC_ENCTR=0;
	reg.uSM_CTRL_ADC_SELIN=ADC_INPUT_MAX;
	MV_SM_WRITE_REG32((SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_CTRL), reg.u32);

	if (mode == ADC_POWER_On)
	{
		T32smSysCtl_SM_ADC_CTRL   reg1;
		T32smSysCtl_SM_TEST_DATA0 reg2;
		T32smSysCtl_SM_TEST_DATA1 reg3;
		T32smSysCtl_SM_ADC_STATUS reg4;

		reg1.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_ADC_CTRL);
		reg1.uSM_ADC_CTRL_ADC_DAT_LT=ADC_LATCH_DELAY;
		MV_SM_WRITE_REG32((SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_ADC_CTRL), reg1.u32);

		reg2.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_TEST_DATA0);
		reg2.uSM_TEST_DATA0_ADC_DATA_HIGH=adc_high[ADC_12BIT];
		MV_SM_WRITE_REG32((SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_TEST_DATA0), reg2.u32);

		reg3.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_TEST_DATA1);
		reg3.uSM_TEST_DATA1_ADC_DATA_LOW=0;
		MV_SM_WRITE_REG32((SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_TEST_DATA1), reg3.u32);

		reg.uSM_CTRL_ADC_SELBG=1;
		reg.uSM_CTRL_ADC_SEL_VREF=1;
		reg.uSM_CTRL_ADC_EN=1;
		MV_SM_WRITE_REG32((SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_CTRL), reg.u32);
		vTaskDelay(1);
		reg.uSM_CTRL_ADC_RESET=1;
		MV_SM_WRITE_REG32((SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_CTRL), reg.u32);

		reg4.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_ADC_STATUS);
		reg4.uSM_ADC_STATUS_DATA_RDY=0;
		reg4.uSM_ADC_STATUS_INT_EN=0;
		MV_SM_WRITE_REG32((SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_ADC_STATUS), reg4.u32);
		vTaskDelay(1);

		reg.uSM_CTRL_ADC_RESET=0;
		MV_SM_WRITE_REG32((SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_CTRL), reg.u32);

		reg.uSM_CTRL_ADC_CONT=0;
		reg.uSM_CTRL_ADC_START=1;
		reg.uSM_CTRL_ADC_SELRES=ADC_6BIT;
		reg.uSM_CTRL_ADC_SELDIFF=0;
		reg.uSM_CTRL_ADC_SELIN=ADC_INPUT0;
		MV_SM_WRITE_REG32((SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_CTRL), reg.u32);

		vTaskDelay(ADC_TIMEOUT*ADC_DELAY_US);

		reg4.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_ADC_STATUS);
		if(reg4.uSM_ADC_STATUS_DATA_RDY)
		{
			T32smSysCtl_SM_TEST reg5;
			reg4.uSM_ADC_STATUS_DATA_RDY=0;
			MV_SM_WRITE_REG32((SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_ADC_STATUS), reg4.u32);
			reg5.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_TEST);
			if(reg5.uSM_TEST_ADC_TEST_FAIL)
				PRT_ERROR("ADC data is not in the valid window\n");
		}
		else
			PRT_ERROR("ADC dummy conversion is not finished\n");
	}
}

static void sm_adc_start(UINT32 start, int sample)
{
	T32smSysCtl_SM_CTRL reg;

	//start/stop
	reg.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_CTRL);

	if (start){
		if(sample>1)
		{
			reg.uSM_CTRL_ADC_CONT = 1;
			reg.uSM_CTRL_ADC_START = 0;
		}
		else
		{
			reg.uSM_CTRL_ADC_CONT = 0;
			reg.uSM_CTRL_ADC_START = 1;
		}

	}
	else {
		reg.uSM_CTRL_ADC_CONT = 0;
		reg.uSM_CTRL_ADC_START = 0;
		reg.uSM_CTRL_ADC_SELIN = ADC_INPUT_MAX;
	}

	MV_SM_WRITE_REG32((SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_CTRL), reg.u32);

	vTaskDelay(10);
}

static INT32 sm_adc_getdata(unsigned int ch)
{
	INT32 cnt = ADC_TIMEOUT_COUNT;
	INT32 result = -1;
	T32smSysCtl_SM_ADC_STATUS reg1;
	T32smSysCtl_SM_ADC_DATA reg2;
	T32smSysCtl_SM_TEST reg3;
	ADC_RES res = ADC_12BIT;

	/* FIXME: the while maybe casue block issue */
	// wait for channel data ready
	while (--cnt) {
		reg1.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_ADC_STATUS);
		if(reg1.uSM_ADC_STATUS_DATA_RDY)
		{
			reg1.uSM_ADC_STATUS_DATA_RDY=0;
			MV_SM_WRITE_REG32((SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_ADC_STATUS), reg1.u32);
			break;
		}
	}

	if (cnt > 0) {
		reg3.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_TEST);
		if(reg3.uSM_TEST_ADC_TEST_FAIL)	{
			PRT_ERROR("ADC data is not in the valid window\n");
		}
		else {
			reg2.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_ADC_DATA);
			result = reg2.uSM_ADC_DATA_ADC_DATA>>adc_shift[res];
		}
	}

	return result;
}

static INT32 sm_adc_read(int ch, int sample)
{
	UINT32 adc_sum = 0;
	UINT32 i;
	INT32 data=0;
	T32smSysCtl_SM_CTRL reg;
	T32smSysCtl_SM_ADC_STATUS reg1;
	T32smSysCtl_SM_TEST_DATA0 reg2;

	if (ch >= ADC_INPUT_MAX)
	{
		PRT_ERROR("Valid input 0 - %d\n",ADC_INPUT_MAX-1);
		return -1;
	}

	reg.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_CTRL);
	if(reg.uSM_CTRL_ADC_EN==0)
		sm_adc_enable(ADC_POWER_On);

	reg1.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_ADC_STATUS);
	reg1.uSM_ADC_STATUS_DATA_RDY=0;
	MV_SM_WRITE_REG32((SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_ADC_STATUS), reg1.u32);
	reg2.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_TEST_DATA0);
	reg2.uSM_TEST_DATA0_ADC_DATA_HIGH=adc_high[ADC_12BIT];
	MV_SM_WRITE_REG32((SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_TEST_DATA0), reg2.u32);
	reg.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_CTRL);
	reg.uSM_CTRL_ADC_SELRES=ADC_12BIT;
	reg.uSM_CTRL_ADC_SELIN=ch;
	MV_SM_WRITE_REG32((SM_MEMMAP_SMREG_BASE+RA_smSysCtl_SM_CTRL), reg.u32);

	sm_adc_start(1, sample);

	// save sampled data to memory
	for (i = 0; i < sample; i++)
	{
		data = sm_adc_getdata(ch);
		if (data < 0) {
			sm_adc_start(0, sample);
			PRT_DEBUG("%s - failed, need another attempt.\n", __func__);
			return -1;
		}
		adc_sum += data;
	}

	sm_adc_start(0, sample);

	data = adc_sum / sample;

	//PRT_RES(" ADC read channel %d %d times avg 0x%03x\n",ch, i,adc_sum);

	return data;
}

static void handle_cbtn_press(ADC_INPUT input, INT32 value, unsigned int dur)
{
	UINT32 iMsg = MV_IR_KEY_NULL;
	UINT32 key_id = ADC_BTN_ID_INVALID;
	UINT32 short_press_action_time[ADC_BTN_ID_MAX] = {
		ADC_BTN_SW1_SHORT_PRESS_ACTION_TIME,
		ADC_BTN_SW2_SHORT_PRESS_ACTION_TIME,
		ADC_BTN_SW3_SHORT_PRESS_ACTION_TIME,
		ADC_BTN_SW4_SHORT_PRESS_ACTION_TIME,
		};
	UINT32 long_press_action_time[ADC_BTN_ID_MAX] = {
		ADC_BTN_SW1_LONG_PRESS_ACTION_TIME,
		ADC_BTN_SW2_LONG_PRESS_ACTION_TIME,
		ADC_BTN_SW3_LONG_PRESS_ACTION_TIME,
		ADC_BTN_SW4_LONG_PRESS_ACTION_TIME,
		};
	UINT32 short_press_ir_key[ADC_BTN_ID_MAX] = {
		ADC_BTN_SW1_SHORT_PRESS_KEY,
		ADC_BTN_SW2_SHORT_PRESS_KEY,
		ADC_BTN_SW3_SHORT_PRESS_KEY,
		ADC_BTN_SW4_SHORT_PRESS_KEY,
		};
	UINT32 long_press_ir_key[ADC_BTN_ID_MAX] = {
		ADC_BTN_SW1_LONG_PRESS_KEY,
		ADC_BTN_SW2_LONG_PRESS_KEY,
		ADC_BTN_SW3_LONG_PRESS_KEY,
		ADC_BTN_SW4_LONG_PRESS_KEY,
		};

	PRT_DEBUG("input = %d, Key value = 0x%0x, dur = %d\n", input, value, dur);
	/* Get Key ID */
	if ((input == ADC_INPUT0) && (value >= VOLTAGE_STAGE2_THRESHOLD))
		key_id = ADC_BTN_ID_SW2;
	else if ((input == ADC_INPUT0) && (value >= VOLTAGE_STAGE1_THRESHOLD))
		key_id = ADC_BTN_ID_SW1;
	else if ((input == ADC_INPUT1) && (value >= VOLTAGE_STAGE2_THRESHOLD))
		key_id = ADC_BTN_ID_SW4;
	else if ((input == ADC_INPUT1) && (value >= VOLTAGE_STAGE1_THRESHOLD))
		key_id = ADC_BTN_ID_SW3;

	if (key_id != ADC_BTN_ID_INVALID){
		if(dur > short_press_action_time[key_id] &&
			dur < long_press_action_time[key_id]) {
			iMsg = short_press_ir_key[key_id];
		}
		else if(dur > long_press_action_time[key_id] && dur < LONG_PRESS_TIMEOUT) {
			iMsg = long_press_ir_key[key_id];
		}
		else {
			PRT_INFO("Button pressed too short or too long, ignore...\n");
		}
	}
	else {
		PRT_INFO("ADC key press: Not in key window, igore...\n");
	}

	if (iMsg != MV_IR_KEY_NULL) {
		if (mv_sm_get_linux_state() == TRUE) {
			sm_send_msg( MV_SM_ID_IR, (unsigned char*)(&iMsg), sizeof(iMsg) );
			iMsg |= 0x8000000;
			sm_send_msg( MV_SM_ID_IR, (unsigned char*)(&iMsg), sizeof(iMsg) );
			PRT_INFO("SM sent IR key (0x%x).\n", iMsg);
		}
		else if (iMsg == MV_IR_KEY_POWER) {
			INT32 sys_state = mv_sm_get_state();

			if (MV_SM_STATE_LOWPOWERSTANDBY == sys_state ||
				MV_SM_STATE_NORMALSTANDBY == sys_state ||
				MV_SM_STATE_SUSPEND == sys_state) {
				PRT_INFO("Turn on from standby or suspend.\n");
				mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_BUTTON);
				mv_sm_power_enterflow_bysmstate();
			}
		}
		else {
			PRT_INFO("Linux is not ready, do nothing.\n");
		}
	}

	first_time[input] = 0;
	last_time[input] = 0;
	start_time[input] = 0;
	end_time[input] = 0;
}

/* function button main task */
hresult sm_adcbtn_task_entry(void *data)
{
	UINT32 i;
	UINT32  dur = 0;
	UINT32	curr_time = mv_sm_timer_gettimems();
	INT32 value;
	static INT32 key = 0;

	for (i = ADC_INPUT0; i < ADC_INPUT_MAX; i++) {
		value = sm_adc_read(i, ADC_SAMPLE_TIME);

		if (value < 0)
			continue;

		if (first_time[i] == 0 && BUTTON_PRESSED(value)) {
			key = value;
			if (start_time[i] == 0)
				start_time[i] = curr_time;
			else {
				if (curr_time - start_time[i] > BUTTON_FILTER1)
					first_time[i] = curr_time;
			}
			PRT_DEBUG("input = %d, Key value = 0x%0x\n", i, value);
		} else {
			if (first_time[i] == 0 && BUTTON_RELEASED(value)) {
				if (start_time[i] != 0 && (curr_time - start_time[i] > BUTTON_FILTER2))
					start_time[i] = 0;
				continue;
			}
		}
		if (first_time[i] != 0 && BUTTON_RELEASED(value)) {
			if (end_time[i] == 0)
				end_time[i] = curr_time;
			else {
				if (curr_time - end_time[i] > BUTTON_FILTER2)
					last_time[i] = curr_time;
			}
		} else {
			if (first_time[i] != 0 && BUTTON_PRESSED(value)) {
				key = value;
				if (end_time[i] != 0 && (curr_time - end_time[i] > BUTTON_FILTER1))
					end_time[i] = 0;
				dur = curr_time - first_time[i];
				PRT_DEBUG("input = %d, Key value = 0x%0x, dur = %d\n", i, value, dur);
			}
		}

		if (last_time[i] != 0) {
			dur = last_time[i] - first_time[i];
			if (dur > SHORT_PRESS_THRESHOLD) {
				PRT_DEBUG("Button pressed for %ds.\n", dur/1000);
				handle_cbtn_press(i, key, dur);
			} else {
				start_time[i] = 0;
				end_time[i] = 0;
				first_time[i] = 0;
				last_time[i] = 0;
			}
		}
	}

	return S_OK;
}

static void sm_adcbtn_task(void* data)
{
	while(1) {
		sm_adcbtn_task_entry(data);
		vTaskDelay(1);
	}
}

static void __attribute__((used)) create_adcbtn_task(void)
{
	xTaskCreate(sm_adcbtn_task, "adcbtn", ADCBTN_STACK_SIZE, NULL, TASK_PRIORITY_2, NULL);
}

DECLARE_RT_MODULE(
	adcbtn,
	MV_SM_ID_ADCBTN,
	create_adcbtn_task,
	NULL,
	NULL
);

static void sm_adc_init(void)
{
	UINT32 iStarttime,iCurrtime;
	UINT32 i;
	T32smSysCtl_SM_CTRL reg;

	for (i = 0; i < ADC_INPUT_MAX; i++) {
		start_time[i] = 0;
		end_time[i] = 0;
		first_time[i] = 0;
		last_time[i] = 0;
	}

	reg.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_CTRL);
	reg.uSM_CTRL_ADC_EN = 1;
	reg.uSM_CTRL_ADC_RESET = 1;

	MV_SM_WRITE_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_CTRL, reg.u32);


	iStarttime = sm_apb_timer_readclock();
	while(1) {
		iCurrtime = sm_apb_timer_readclock();
		if (((iCurrtime > iStarttime) ? (iCurrtime - iStarttime) : (iCurrtime - iStarttime + INT_MS*1000*SM_SYSTEM_HZ))
				> (SM_SYSTEM_HZ * 1000 * 1))
			break;
	}

	reg.u32 = MV_SM_READ_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_CTRL);
	reg.uSM_CTRL_ADC_RESET = 0;
	MV_SM_WRITE_REG32(SM_MEMMAP_SMREG_BASE + RA_smSysCtl_SM_CTRL,reg.u32);
}

static int __attribute__((used)) sm_adcbtn_init(void)
{
	sm_adc_init();

	return S_OK;
}

DECLARE_RT_INIT(adcbtninit, INIT_DEV_P_25, sm_adcbtn_init);

#endif /* ADCBTNENABLE */
