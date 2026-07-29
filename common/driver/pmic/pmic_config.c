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
#include "pmic_config.h"
#include "boot_config.h"
#include "pmic_select.h"
#include "pmic_pinmux.h"
#include "printf.h"
#include "string.h"
#include "gpio.h"

struct pmic_config *g_pmic_config;

char *pmic_model_names[] = {
	PMIC_MODEL_NAMES_AUTOGEN
};

static UNSG32 calculate_checksum(const struct pmic_config *config)
{
	UNSG32 checksum = 0;
	const UNSG32 *data = (const UNSG32 *)config;
	int i;

	for (i = 0; i < (sizeof(struct pmic_config) / 4 - 1); i++) {
		checksum ^= data[i];
	}

	return checksum;
}

int pmic_config_validate(const struct pmic_config *config)
{
	UNSG32 expected_checksum;

	if (!config) {
		return -1;
	}

	if (config->magic != PMIC_CONFIG_MAGIC) {
		lgpl_printf("PMIC: Invalid magic number 0x%08x, expected 0x%08x\n",
				   config->magic, PMIC_CONFIG_MAGIC);
		return -1;
	}

	if (config->version != PMIC_CONFIG_VERSION) {
		lgpl_printf("PMIC: Unsupported version 0x%08x, expected 0x%08x\n",
				   config->version, PMIC_CONFIG_VERSION);
		return -1;
	}

	expected_checksum = calculate_checksum(config);
	if (config->checksum != expected_checksum) {
		lgpl_printf("PMIC: Checksum mismatch 0x%08x, expected 0x%08x\n",
				   config->checksum, expected_checksum);
		return -1;
	}

	return 0;
}

static int get_pmic_auto_detection_index(enum voltage_type voltage_type)
{
	int gpioh = 0;
	int gpiol = 0;
	int pmic_index = 0;
	int ret = -1;
	const struct pmic_pinmux_ops *ops;

	ops = pmic_pinmux_get_ops();
	if (!ops || !ops->set_pinmux)
		return -1;

	if (voltage_type == VCORE) {
		if (g_pmic_config->vcore_gpioh == 0xff || g_pmic_config->vcore_gpiol == 0xff)
			return -1;
		ret = ops->set_pinmux(g_pmic_config->vcore_gpioh, "gpio", NULL);
		if (ret)
			return -1;
		ret = ops->set_pinmux(g_pmic_config->vcore_gpiol, "gpio", NULL);
		if (ret)
			return -1;
		GPIO_PortRead(g_pmic_config->vcore_gpioh, &gpioh);
		GPIO_PortRead(g_pmic_config->vcore_gpiol, &gpiol);
	} else if (voltage_type == VCPU) {
		if (g_pmic_config->vcpu_gpioh == 0xff || g_pmic_config->vcpu_gpiol == 0xff)
			return -1;
		ret = ops->set_pinmux(g_pmic_config->vcpu_gpioh, "gpio", NULL);
		if (ret)
			return -1;
		ret = ops->set_pinmux(g_pmic_config->vcpu_gpiol, "gpio", NULL);
		if (ret)
			return -1;
		GPIO_PortRead(g_pmic_config->vcpu_gpioh, &gpioh);
		GPIO_PortRead(g_pmic_config->vcpu_gpiol, &gpiol);
	}

	gpioh &= 1;
	gpiol &= 1;
	pmic_index = (gpioh << 1) | gpiol;

	return pmic_index;
}

static UNSG32 pmic_get_valid_id(UNSG32 id)
{
	if (id == 0 || id == 255)
		return PMIC_UNKNOW;
	return id - 1;
}

int pmic_config_init(void)
{
	int ret = -1;
	UNSG32 vcore_id;
	UNSG32 vcpu_id;
	UNSG32 vcore_auto_index;
	UNSG32 vcpu_auto_index;
	UNSG32 vcore_model_id, vcpu_model_id;

	ret = get_pmic_config(&g_pmic_config);
	if (ret == 0 && pmic_config_validate(g_pmic_config) == 0) {
		switch (g_pmic_config->pmic_type) {
		case PMIC_TYPE_I2C:
			lgpl_printf("I2C PMIC detected: VCORE %s (ID: 0x%02x), VCPU %s (ID: 0x%02x) \n",
				    pmic_get_model_name(g_pmic_config->vcore_id[0]), g_pmic_config->vcore_id[0],
				    pmic_get_model_name(g_pmic_config->vcpu_id[0]), g_pmic_config->vcpu_id[0]);

			vcpu_id = pmic_get_valid_id(g_pmic_config->vcpu_id[0]);
			vcore_id = pmic_get_valid_id(g_pmic_config->vcore_id[0]);

			set_pmic_id(vcpu_id, vcore_id);
			ret = 1;
			break;

		case PMIC_TYPE_PWM:
			lgpl_printf("PWM PMIC detected: VCORE gpio: 0x%02x, VCPU gpio: 0x%02x \n",
				    g_pmic_config->vcore_pwm_gpio,
				    g_pmic_config->vcpu_pwm_gpio);
			vcore_id = (g_pmic_config->vcore_pwm_gpio == 0xff) ? PMIC_UNKNOW : PWM_PMIC;
			vcpu_id = (g_pmic_config->vcpu_pwm_gpio == 0xff) ? PMIC_UNKNOW : PWM_PMIC;
			set_pmic_id(vcpu_id, vcore_id);
			ret = 2;
			break;

		case PMIC_TYPE_SM_TIMER:
			vcore_id = (g_pmic_config->vcore_smtmr_gpio == 0xff) ? PMIC_UNKNOW : SMTIMER;
			vcpu_id = (g_pmic_config->vcpu_smtmr_gpio == 0xff) ? PMIC_UNKNOW : SMTIMER;
			set_pmic_id(vcpu_id, vcore_id);
			ret = 3;
			break;

		case PMIC_TYPE_AUTO:
			// Here we assume the index maps to I2C models.
			vcore_auto_index = get_pmic_auto_detection_index(VCORE);
			if (vcore_auto_index != -1)
				vcore_id = pmic_get_valid_id(g_pmic_config->vcore_id[vcore_auto_index]);
			else
				vcore_id = PMIC_UNKNOW;

			vcpu_auto_index = get_pmic_auto_detection_index(VCPU);
			if (vcpu_auto_index != -1)
				vcpu_id = pmic_get_valid_id(g_pmic_config->vcpu_id[vcpu_auto_index]);
			else
				vcpu_id = PMIC_UNKNOW;

			lgpl_printf("PMIC Auto Detect index : VCORE:%d, VCPU:%d\n", vcore_auto_index, vcpu_auto_index);

			if (vcore_auto_index != -1 || vcpu_auto_index != -1) {
				vcore_model_id = (vcore_auto_index == -1) ? 0 : g_pmic_config->vcore_id[vcore_auto_index];
				vcpu_model_id = (vcpu_auto_index == -1) ? 0 : g_pmic_config->vcpu_id[vcpu_auto_index];
				lgpl_printf("I2C PMIC detected: VCORE %s (ID: 0x%02x), VCPU %s (ID: 0x%02x) \n",
					pmic_get_model_name(vcore_model_id), vcore_model_id,
					pmic_get_model_name(vcpu_model_id), vcpu_model_id);

				set_pmic_id(vcpu_id, vcore_id);
				ret = 4;
			} else
				ret = -1;
			break;

		default:
			vcore_id = PMIC_UNKNOW;
			vcpu_id = PMIC_UNKNOW;
			lgpl_printf("PMIC: unknown PMIC type (%d)\n", g_pmic_config->pmic_type);
			set_pmic_id(vcpu_id, vcore_id);
			break;
		}
	} else {
		vcore_id = PMIC_UNKNOW;
		vcpu_id = PMIC_UNKNOW;
		lgpl_printf("PMIC: unknown PMIC model\n");
		set_pmic_id(vcpu_id, vcore_id);
	}

	return ret;
}

const char *pmic_get_model_name(UNSG32 model_id)
{
	if (model_id < sizeof(pmic_model_names) / sizeof(pmic_model_names[0]) &&
		pmic_model_names[model_id] != NULL) {
		return pmic_model_names[model_id];
	}

	return "unknown";
}
