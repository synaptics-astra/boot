/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright (C) 2013-2025 Synaptics Incorporated. All rights reserved.
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
#ifndef __PMIC_CONFIG_H__
#define __PMIC_CONFIG_H__

#include "com_type.h"

/* PMIC model IDs - must match gen_pmic_config.py */
#define PMIC_MODEL_UNKNOWN	 0x00
#define PMIC_MODEL_M88PG86X	0x01
#define PMIC_MODEL_SY8824B	 0x02
#define PMIC_MODEL_NCP6335D	0x03
#define PMIC_MODEL_MP886X	  0x04
#define PMIC_MODEL_SY8827N	 0x05
#define PMIC_MODEL_HL7593WL06  0x06
#define PMIC_MODEL_TPS62868	0x07
#define PMIC_MODEL_RT5739	  0x08
#define PMIC_MODEL_FP6359	  0x09
#define PMIC_MODEL_TPS6287X	0x0a

/* PMIC configuration structure */
struct pmic_config {
	UNSG32 magic;		/* Magic number: 0x504D4943 ("PMIC") */
	UNSG32 version;	  /* Version: 0x00000001 */
	UNSG32 vcore_id;	 /* VCORE PMIC model ID */
	UNSG32 vcpu_id;	  /* VCPU PMIC model ID */
	UNSG32 pwm_valid;  /* PWM valid flag */
	UNSG32 vcore_pwm_ch;	 /* VCORE PWM channel */
	UNSG32 vcpu_pwm_ch;	 /* VCPU PWM channel */
	UNSG32 checksum;	 /* Simple checksum */
};

#define PMIC_CONFIG_MAGIC	 0x504D4943  /* "PMIC" */
#define PMIC_CONFIG_VERSION   0x00000001
#define PMIC_CONFIG_SIZE	  32

extern struct flash_adaptor mmc_g_flash;
extern struct pmic_config g_pmic_config;
/* Function prototypes */
int pmic_config_init(void);
UNSG32 pmic_get_vcore_id(void);
UNSG32 pmic_get_vcpu_id(void);
const char* pmic_get_model_name(UNSG32 model_id);
int pmic_config_validate(const struct pmic_config *config);
void pmic_select_driver(UNSG32 vcore_id, UNSG32 vcpu_id);

/* PWM-related function prototypes */
int pmic_is_pwm_enabled(void);
UNSG32 pmic_get_vcore_pwm_channel(void);
UNSG32 pmic_get_vcpu_pwm_channel(void);
int pmic_is_vcore_pwm_configured(void);
int pmic_is_vcpu_pwm_configured(void);

#endif /* __PMIC_CONFIG_H__ */
