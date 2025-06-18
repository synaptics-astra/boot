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
#ifdef TEMPENABLE
#include "sm_type.h"
#include "sm_io.h"

#include "sm_common.h"
#include "platform_config.h"

#include "SysMgr.h"

#include "sm_timer.h"
#include "sm_printf.h"

#include "sm_comm.h"

//#include "sm_adc.h"
#include "sm_state.h"
#include "sm_power.h"
//#include "sm_trace.h"
#include "sm_rt_module.h"

#define TEMP_BIAS_L		80
#define TEMP_BIAS_H		120
#define TEMP_BIAS_DEFAULT	100

static INT32 temp_bias; /* get from bootloader */

static INT32 last_raw_data, raw_data;
static INT32 last_calc_data, calc_data;
static INT32 last_calibrate_data, calibrate_data;
static int temp_offset = 0;

INT32 chip_ver;
INT32 chip_subver;

#define PVT_LATCH_DELAY	0						// make sure data is good, register bus clock (SM 25Mhz SOC 100Mhz) delay, PVT_test_sequence.docx pg2 4.	s/w should wait for data latency timer once it detects interrupt/data_rdy to sample the data
#define PVT_DELAY_US	10
#define PVT_TIMEOUT 	(300/PVT_DELAY_US)		// Min Clock Freq 1.15 MHz, use 25mhz crystal oscillator and 20 divider 1.25Mhz, Max Conversion Time 373/5 + PVT_LATCH_DELAY clock cycles for Temp or Volt/Process 324.35(1.15Mhz)/298.4(1.25MHz)+0.64(SM)/0.16(SOC) us max

#define PVT_MAX_VAL		(1<<10)


unsigned int pvt_read(unsigned int trimo,
					unsigned int trimg,
					unsigned int vsample,
					unsigned int psample)
{
	T32TSEN_CTRL reg;
	T32TSEN_STATUS reg1;
	unsigned int ret;
	int i = 0;

	// Delay Several Clocks When Data is Ready
	BFM_HOST_Bus_Read32((PVT_BASE + RA_TSEN_CTRL), &reg.u32);
	reg.uCTRL_TSEN_DAT_LT = PVT_LATCH_DELAY;
	BFM_HOST_Bus_Write32((PVT_BASE + RA_TSEN_CTRL), reg.u32);


	// 1. Set VSAMPLE and PSAMPLE
	BFM_HOST_Bus_Read32((PVT_BASE + RA_TSEN_CTRL), &reg.u32);
	reg.uCTRL_TRIMO=trimo;
	reg.uCTRL_TRIMG=trimg;
	reg.uCTRL_VSAMPLE=vsample;
	reg.uCTRL_PSAMPLE=psample;
	BFM_HOST_Bus_Write32((PVT_BASE + RA_TSEN_CTRL), reg.u32);

	// 2. Set ENA to 1'b1, start sample after enable clock make more sense
	BFM_HOST_Bus_Read32((PVT_BASE + RA_TSEN_CTRL), &reg.u32);
	reg.uCTRL_ENA=1;
	BFM_HOST_Bus_Write32((PVT_BASE + RA_TSEN_CTRL), reg.u32);

	// 3. Enabl pvt sensor clock
	BFM_HOST_Bus_Read32((PVT_BASE + RA_TSEN_CTRL), &reg.u32);
	reg.uCTRL_CLK_EN=1;
	BFM_HOST_Bus_Write32((PVT_BASE + RA_TSEN_CTRL), reg.u32);

	// 4. Sample data
	for(i = 0;i < PVT_TIMEOUT; i++)
	{
		vTaskDelay(1);
		BFM_HOST_Bus_Read32((PVT_BASE + RA_TSEN_STATUS), &reg1.u32);
		if(reg1.uSTATUS_DATA_RDY)
			break;
	}
	if(i >= PVT_TIMEOUT)
	{
		ret = PVT_MAX_VAL;
		PRT_ERROR("PVT read timeout\n");
	}
	else
	{
		T32TSEN_DATA reg2;
		BFM_HOST_Bus_Read32((PVT_BASE + RA_TSEN_DATA), &reg2.u32);
		ret=reg2.uDATA_DATA;
	}

	// 5. Clear status, is it fo resample again? ASIC said uSTATUS_DATA_RDY won't change DAT_VALID line
	BFM_HOST_Bus_Read32((PVT_BASE + RA_TSEN_STATUS), &reg1.u32);
	reg1.uSTATUS_DATA_RDY=0;
	BFM_HOST_Bus_Write32((PVT_BASE + RA_TSEN_STATUS), reg1.u32);
	BFM_HOST_Bus_Read32((PVT_BASE + RA_TSEN_STATUS), &reg1.u32);
	if(reg1.uSTATUS_DATA_RDY)
		PRT_ERROR("PVT Data Ready isn't cleaned\n");

	// 5-b. Set ENA to 0
	BFM_HOST_Bus_Read32((PVT_BASE + RA_TSEN_CTRL), &reg.u32);
	reg.uCTRL_ENA=0;
	BFM_HOST_Bus_Write32((PVT_BASE + RA_TSEN_CTRL), reg.u32);

	// 5-c. Gate pvt sensor clock
	BFM_HOST_Bus_Read32((PVT_BASE + RA_TSEN_CTRL), &reg.u32);
	reg.uCTRL_CLK_EN=0;
	BFM_HOST_Bus_Write32((PVT_BASE + RA_TSEN_CTRL), reg.u32);

	return ret;
}

unsigned int pvt_read_samples()
{
	unsigned int value = 0, sum = 0;
	int i = 0;

	for(i = 0; i < 4; i++) {
		value = pvt_read(0, 0xf, 0, 0);
		if(value >= PVT_MAX_VAL)
			break;
		sum += value;
	}

	if(i == 4)
		return sum / i;

	return 0;
}

int value2degC(unsigned int value)				// temperature accuracy post trim +- 1 degC, long long have overflow issue
{
	// temp = f(code)
	// f(x) = a4 x^4 + a3 x^3 + a2 x^2 + a1 x + a0
	// a4 = -1.8439E-11; a3 = 8.0705E-08; a2 = -1.8501E-04;
	// a1 = 3.2843E-01; a0 = -4.8690E+01;
	//((((80705000-18439*value)*value-185010000000)*value+328430000000000)*value-48690000000000000)/1000000000000000;
	int result = 0;
	int code = (int)value;

	result = 18439 * code;
	result = result / 1000;
	result = 80705 - result;
	result *= code;
	result = result / 1000;
	result = 185010 - result;
	result *= code;
	result = result / 1000;
	result = 328430 - result;
	result *= code;
	result = result / 10000;
	result -= 4869;
	result = result / 100;

	// Remove TSEN_ID adjustment according to diag team's note.
	//result += temp_offset;

	return (int)result;
}

void mv_sm_temp_sample(void)
{
	last_raw_data = raw_data;
	last_calc_data = calc_data;
	last_calibrate_data = calibrate_data;

	raw_data = pvt_read_samples();
	calc_data = value2degC(raw_data);
	calibrate_data = calc_data;
	//calibrate_data = calc_data + 100 - temp_bias;
	//FIXME: add
	//rom_temp_trace(calc_data);
}

hresult mv_sm_process_temp_msg(void * data, INT32 len)
{
	INT32 *pMsgBody = (INT32*)data;
	INT32 msg_send[4];

	switch (pMsgBody[0]) {
	case MV_SM_TEMP_SAMPLE:
		msg_send[0] = temp_bias;
		msg_send[1] = raw_data;
		msg_send[2] = calc_data;
		msg_send[3] = calibrate_data;
		sm_send_msg(MV_SM_ID_TEMP, (unsigned char*)msg_send, sizeof(msg_send));
		break;

	case MV_SM_TEMP_CPU_TYPE:
		chip_ver = pMsgBody[1];
		chip_subver = pMsgBody[2];
		temp_bias = pMsgBody[3];
		if (temp_bias < TEMP_BIAS_L || temp_bias > TEMP_BIAS_H)
			temp_bias = TEMP_BIAS_DEFAULT;
		break;
	case MV_SM_SOC_TSEN_ID:
		/********************************************************************
		Need to do temperature adjust according to soc_tsen_id in OTP
		Note: The real time TSEN reading may have 5~10 degree variation, and
		it can be found from the OTP programmed TSEN_ID filed ( which is the
		TSEN reading at tester 105C condition).
		For example, you may read 98C to 114C from TSEN_ID OTP field from
		different parts. Therefor, the TSEN offset should be 105 â€“ TSEN_ID
		********************************************************************/
		temp_offset = 105 - pMsgBody[1];
		if (temp_offset > 15 || temp_offset < -15)
			temp_offset = 0;
		PRT_INFO("SM: soc_tsen_id=%d, \n", pMsgBody[1]);
	default:
		break;
	}

	return S_OK;
}

static int mv_sm_temp_task(void * data)
{
	// refresh data according to the interval of task
		// default is 5s.
		mv_sm_temp_sample();

		if((calc_data > 125) || (calc_data < (-45))) {
			PRT_RES("Chip Temperature exceeds measurement range: raw data = %d,\
				calc_data = %d, calibrate_data = %d\n", raw_data,
				calc_data, calibrate_data);
		}

#ifdef OVER_HEAT_TEMPERATURE
		if(calibrate_data > OVERHEATHRESHOLD) {
			PRT_INFO("Warning: Chip is over heat, and forcibly enter standby\n");
			PRT_INFO("Chip Temperature: %d degC(raw_data = %d)\n",
							calibrate_data, raw_data);
			if(mv_sm_get_state() == MV_SM_STATE_ACTIVE) {
				//FIXME: go to which state?
				mv_sm_power_enterflow(FLOW_ACTIVE_2_LOWPOWERSTANDBY);
			}
		}
#endif

		return S_OK;

}

/* print temperature every 30s(defined as task interval) */
static int mv_sm_temp_print_task(void * data)
{
	PRT_RES("Chip Temperature: %d degC(raw_data = %d)\n",
		calibrate_data, raw_data);
	return S_OK;
}

/* print temperature if the change is biger than 1 */
static int __attribute__((used)) mv_sm_temp_print_change_task(void * data)
{
	if(((calc_data - last_calc_data) > 1) || ((last_calc_data - calc_data) > 1)) {
		PRT_INFO("Chip Temperature: %d degC(raw_data = %d)\n",
			calibrate_data, raw_data);
	}
	return S_OK;
}

#define TEMP_STACK_SIZE ((uint16_t) 128)

static void mv_sm_temp_main_task(void * para)
{
	static int count = 0;
	for(;;) {
		mv_sm_temp_task(para);
		//mv_sm_temp_print_change_task(para);
		if(count == 60) { // 5 minutes
			mv_sm_temp_print_task(para);
			count = 0;
		}
		vTaskDelay(5000);
		count++;
	}
}

static void __attribute__((used)) create_temp_task(void)
{
	xTaskCreate(mv_sm_temp_main_task, "tsen", TEMP_STACK_SIZE, NULL, TASK_PRIORITY_2, NULL);
}

DECLARE_RT_MODULE(
	tsen,
	MV_SM_ID_TEMP,
	create_temp_task,
	mv_sm_process_temp_msg,
	NULL
);

#endif /* TEMPENABLE */
