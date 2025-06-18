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
#ifndef __SM_ADCBTN_AD_H__
#define __SM_ADCBTN_AD_H__

#include "platform_config.h"

#define ADC_BTN_ID_SW1 		0
#define ADC_BTN_ID_SW2 		1
#define ADC_BTN_ID_SW3 		2
#define ADC_BTN_ID_SW4 		3
#define ADC_BTN_ID_MAX 		4
#define ADC_BTN_ID_INVALID	-1


#ifndef LONG_PRESS_TIMEOUT
/* ADC button press time threshold*/
#define ADC_BTN_SW1_SHORT_PRESS_ACTION_TIME	500
#define ADC_BTN_SW1_LONG_PRESS_ACTION_TIME	5*1000
#define ADC_BTN_SW2_SHORT_PRESS_ACTION_TIME	100
#define ADC_BTN_SW2_LONG_PRESS_ACTION_TIME	5*1000
#define ADC_BTN_SW3_SHORT_PRESS_ACTION_TIME	100
#define ADC_BTN_SW3_LONG_PRESS_ACTION_TIME	5*1000
#define ADC_BTN_SW4_SHORT_PRESS_ACTION_TIME	100
#define ADC_BTN_SW4_LONG_PRESS_ACTION_TIME	5*1000
#define LONG_PRESS_TIMEOUT			10*1000

/* ADC button IR Key */
#define ADC_BTN_SW1_SHORT_PRESS_KEY	MV_IR_KEY_BEGIN_PAIRING
#define ADC_BTN_SW1_LONG_PRESS_KEY 	MV_IR_KEY_NULL
#define ADC_BTN_SW2_SHORT_PRESS_KEY	MV_IR_KEY_POWER
#define ADC_BTN_SW2_LONG_PRESS_KEY 	MV_IR_KEY_RECOVERY
#define ADC_BTN_SW3_SHORT_PRESS_KEY	MV_IR_KEY_VOL_MINUS
#define ADC_BTN_SW3_LONG_PRESS_KEY 	MV_IR_KEY_NULL
#define ADC_BTN_SW4_SHORT_PRESS_KEY	MV_IR_KEY_VOL_PLUS
#define ADC_BTN_SW4_LONG_PRESS_KEY 	MV_IR_KEY_NULL
#endif

#define SHORT_PRESS_THRESHOLD	100

#define BOOTMODE_NORMAL		0
#define BOOTMODE_RECOVERY	1

#define ADC_LATCH_DELAY	0
#define ADC_DELAY_US	1
#define ADC_TIMEOUT 	(2/ADC_DELAY_US)
#define ADC_TIMEOUT_VAL	0xFFFF
#define ADC_MAX_VAL		(1<<12)

#define ADC_TIMEOUT_COUNT	100

#ifndef VOLTAGE_STAGE1_THRESHOLD
/*Voltage stage 1 threshold: 0.50V of 0.6V set to the threshold*/
#define VOLTAGE_STAGE1_THRESHOLD 0x6aa
/*Voltage stage 2 threshold: 0.94V of 1.2V set to the threshold*/
#define VOLTAGE_STAGE2_THRESHOLD 0xc99
/*Using ADC 12bit*/
#define VOLTAGE_THRESHOLD_MAX 0x1000
#endif

typedef enum _ADC_MODE
{
	ADC_POWER_Down = 0,
	ADC_POWER_On,
	ADC_STANDBY,
} ADC_MODE;

typedef enum _ADC_RES
{
	ADC_6BIT = 0,
	ADC_8BIT,
	ADC_10BIT,
	ADC_12BIT,
} ADC_RES;

typedef enum _ADC_INPUT
{
	ADC_INPUT0 = 0,
	ADC_INPUT1,
	ADC_INPUT_MAX,
} ADC_INPUT;

hresult mv_sm_fnbtn_task(void *data);

#endif /* __SM_ADCBTN_AD_H__ */
#endif /* ADCBTNENABLE */
