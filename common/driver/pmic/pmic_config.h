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

enum voltage_type {
	VCORE,
	VCPU
};
enum pmic_i2c_channel {
	I2C_TW0 = 0,
	I2C_TW1,
	I2C_TW1A,
	I2C_TW1B,
	I2C_TW2,
	I2C_TW2A,
	I2C_TW2B,
	I2C_TW3,
	I2C_TW3A,
	I2C_TW3B
};

typedef enum {
	PMIC_TYPE_I2C = 1,
	PMIC_TYPE_PWM,
	PMIC_TYPE_SM_TIMER,
	PMIC_TYPE_AUTO
} PMIC_TYPE_E;

/* PMIC configuration structure */
struct pmic_config {
	UNSG32 magic;            /* Magic number: 0x504D4943 ("PMIC") */
	UNSG32 version;          /* Version: 0x00000001 */
	UNSG32 vcore_id[4];      /* VCORE PMIC model ID */
	UNSG32 vcpu_id[4];       /* VCPU PMIC model ID */
	UNSG32 vcore_i2c_ch;     /* VCORE I2C channel */
	UNSG32 vcpu_i2c_ch;      /* VCPU I2C channel 0xff-no such ch */
	UNSG32 vcore_gpioh;      /* VCORE GPIOH pin */
	UNSG32 vcore_gpiol;      /* VCORE GPIOL pin */
	UNSG32 vcpu_gpioh;       /* VCPU GPIOH pin */
	UNSG32 vcpu_gpiol;       /* VCPU GPIOL pin */
	UNSG32 pmic_type;        /* 1-I2C, 2-PWM, 3-SM Timer, 4-auto detection */
	UNSG32 vcore_pwm_gpio;   /* VCORE PWM GPIO index */
	UNSG32 vcpu_pwm_gpio;    /* VCPU PWM GPIO index */
	UNSG32 vcore_smtmr_gpio; /* VCORE PWM GPIO index */
	UNSG32 vcpu_smtmr_gpio;  /* VCPU PWM GPIO index */

	/* PWM regulator configuration */
	UNSG32 vcore_pwm_freq_hz;
	UNSG32 vcore_pwm_polarity;
	UNSG32 vcore_pwm_volt_min_uv;
	UNSG32 vcore_pwm_volt_max_uv;
	UNSG32 vcore_pwm_duty_at_volt_min;
	UNSG32 vcore_pwm_duty_at_volt_max;

	UNSG32 vcpu_pwm_freq_hz;
	UNSG32 vcpu_pwm_polarity;
	UNSG32 vcpu_pwm_volt_min_uv;
	UNSG32 vcpu_pwm_volt_max_uv;
	UNSG32 vcpu_pwm_duty_at_volt_min;
	UNSG32 vcpu_pwm_duty_at_volt_max;

	UNSG32 checksum; /* Simple checksum */
};

/*
 * Register configuration for a single GPIO pin's mux control.
 * Maps a GPIO number to its register bitfield location.
 */
struct gpio_reg_map {
	unsigned int gpio;
	unsigned char byte_offs; /* register byte offset within the domain block */
	unsigned char bit_offs;  /* LSB bit position within the register */
	unsigned char bit_wid;   /* bit field width */
};

/*
 * Function-to-value assignment for a specific GPIO pin.
 * Each entry maps (gpio, function name) to the register value and auxiliary channel.
 */
struct gpio_fun_map {
	unsigned int gpio;
	const char *funame;
	unsigned char val;       /* value to write into the mux bitfield */
	unsigned char aux;       /* auxiliary channel (pwm/timer), 0 if unused */
};

#define REG_MAP_ENTRY(_gpio, _byte_offs, _bit_offs, _bit_wid) \
	{ .gpio = _gpio, .byte_offs = _byte_offs, .bit_offs = _bit_offs, .bit_wid = _bit_wid }

#define FUN_MAP_ENTRY(_gpio, _funame, _val, _aux) \
	{ .gpio = _gpio, .funame = _funame, .val = _val, .aux = _aux }

#define PMIC_CONFIG_MAGIC        0x504D4943  /* "PMIC" */
#define PMIC_CONFIG_VERSION   0x00000001

extern struct flash_adaptor mmc_g_flash;
extern struct pmic_config *g_pmic_config;
/* Function prototypes */
int pmic_config_init(void);
UNSG32 pmic_get_vcore_id(void);
UNSG32 pmic_get_vcpu_id(void);
const char* pmic_get_model_name(UNSG32 model_id);
int pmic_config_validate(const struct pmic_config *config);
void pmic_select_driver(UNSG32 vcore_id, UNSG32 vcpu_id);
int pmic_is_pwm_enabled(void);
int pmic_is_vcore_pwm_configured(void);
int pmic_is_vcpu_pwm_configured(void);

#endif /* __PMIC_CONFIG_H__ */
