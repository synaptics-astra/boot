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
#include "com_type.h"
#include "debug.h"
#include "pmic_select.h"

const static char * pmic_name[] = {
	"M88PG86X",
	"SY8824B",
	"NCP6335D",
	"MP886X",
	"SY8827N",
	"HL7593WL06",
	"TPS62868",
	"RT5739",
	"FP6359",
	"UNKNOWN"
};

static unsigned int default_pmic_vcpu  = PMIC_UNKNOW;
static unsigned int default_pmic_vcore = PMIC_UNKNOW;
static unsigned int pmic_vcpu  = PMIC_UNKNOW;
static unsigned int pmic_vcore = PMIC_UNKNOW;

void set_default_pmic_id(unsigned int vcpu_pmic_sel, unsigned int vcore_pmic_sel)
{
	default_pmic_vcpu = vcpu_pmic_sel;
	default_pmic_vcore = vcore_pmic_sel;
}

void set_pmic_id(unsigned int vcpu_pmic_sel, unsigned int vcore_pmic_sel)
{
	pmic_vcpu = vcpu_pmic_sel;
	pmic_vcore = vcore_pmic_sel;
}

unsigned int get_pmic_id(unsigned int device_id)
{
	if (device_id == PMIC_SOC_VCPU)
		return pmic_vcpu;
	else if (device_id == PMIC_SOC_VCORE)
		return pmic_vcore;
	else
	{
		ERR("Unknown Device ID (%d)\n", device_id);
		return PMIC_UNKNOW;
	}
}

unsigned int get_default_pmic_id(unsigned int device_id)
{
	if (device_id == PMIC_SOC_VCPU)
		return default_pmic_vcpu;
	else if (device_id == PMIC_SOC_VCORE)
		return default_pmic_vcore;
	else
	{
		ERR("Unknown Device ID (%d)\n", device_id);
		return PMIC_UNKNOW;
	}
}

const char * get_pmic_name(unsigned int pmic_id)
{
	if (pmic_id >= PMIC_UNKNOW)
		return pmic_name[PMIC_UNKNOW];
	else
		return pmic_name[pmic_id];
}
