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
#include <io.h>
#include "util.h"
#include "debug.h"
#include "chip_voltage_info.h"
#include "pmic_select.h"
#include "pv_comp.h"

static int find_dvfs_group(struct freq_leakage_table *flt, int pll, int leakage)
{
	struct leakage_table *p = NULL;

	while(flt->freq != 0) {
		if(flt->freq >= pll) {
			p = flt->lt;
			if(p == NULL) {
				break;
			}
			while(1) {
				if(leakage <= p->info)
					break;
				p++;
			}
			return p->volt;
		}
		flt++;
	}

	return -1;
}


static int get_vcpu_volt_from_dvfs(int cpu_pll, int leakage)
{
	struct freq_leakage_table *flt = get_vcpu_leakage_table();
	int ret = 0;

	if(flt == NULL) {
		ERR("No vcpu PV table found!!!!\n");
		return -1;
	}

	ret = find_dvfs_group(flt, cpu_pll, leakage);

	if(ret == -1)
		ERR("No leakage group in vcpu PV table found!!!!\n");

	return ret;
}


void get_vcpu_vh_vl(unsigned int *opp)
{
	int leakage = 0;
	int volt = 0;
	unsigned int count = 0;
	struct freq_leakage_table *flt = NULL;

	leakage = get_leakage_info();
	flt = get_vcpu_leakage_table();

	if(flt == NULL) {
		ERR("No vcpu PV table found!!!!\n");
		goto error;
	}

	//vl
	if(flt->freq != 0) {
		volt = get_vcpu_volt_from_dvfs(flt->freq, leakage);
		if(volt > 0) {
			opp[count++] = volt;
		}
		flt++;

		//vh
		if(flt->freq != 0) {
			volt = get_vcpu_volt_from_dvfs(flt->freq, leakage);
			if(volt > 0) {
				opp[count++] = volt;
			}
		}
	}

	if(count > 1) {
		INFO("Got %d value for vcpu vl(%d) and vh(%d)\n", count, opp[0], opp[1]);
		return;
	}

error:
	ERR("###### wrong value for vcpu vl and vh!!!!!!!!\n");
	return;
}

int get_vcore_volt_from_dvfs(int gpu_pll, int leakage)
{
	struct freq_leakage_table *flt = get_vcore_leakage_table();
	int ret = 0;

	if(flt == NULL) {
		ERR("No vcore PV table found!!!!\n");
		return -1;
	}

	ret = find_dvfs_group(flt, gpu_pll, leakage);

	if(ret == -1)
		ERR("No leakage group in vcore PV table found!!!!\n");

	return ret;
}

void get_vcore_vh_vl(unsigned int *opp)
{
	int leakage = 0;
	int volt = 0;
	unsigned int count = 0;
	struct freq_leakage_table *flt = NULL;

	leakage = get_leakage_info();
	flt = get_vcore_leakage_table();

	if(NULL == flt) {
		ERR("No vcore PV table found!!!!\n");
		goto error;
	}

	//vl
	if(0 != flt->freq) {
		volt = get_vcore_volt_from_dvfs(flt->freq, leakage);
		if(volt > 0) {
			opp[count++] = volt;
		}
		flt++;

		//vh
		if(0 != flt->freq) {
			volt = get_vcore_volt_from_dvfs(flt->freq, leakage);
			if(volt > 0) {
				opp[count++] = volt;
			}
		}
	}

	if(count > 1) {
		INFO("Got %d value for vcore vl(%d) and vh(%d)\n", count, opp[0], opp[1]);
		return;
	}

error:
	ERR("###### wrong value for vcore vl and vh!!!!!!!!\n");
	return;
}

static const dvfs_ops_t *get_dvfs_ops(unsigned int pmic_id)
{
	const dvfs_ops_t * dvfs_ops = NULL;

	INFO("PMIC: %s selected for board %s !\n", get_pmic_name(pmic_id), CONFIG_PLATFORM);
	switch (pmic_id) {
		case M88PG86X:
			dvfs_ops = &m88pg86x_ops;
			break;

		case SY8824B:
			dvfs_ops = &sy8824b_ops;
			break;

		case NCP6335D:
			dvfs_ops = &ncp6335d_ops;
			break;

		case MP886X:
			dvfs_ops = &mp886x_ops;
			break;

		case SY8827N:
			dvfs_ops = &sy8827n_ops;
			break;

		case HL7593WL06:
			dvfs_ops = &hl7593_ops;
			break;

		case TPS62868:
			dvfs_ops = &TPS62868_ops;
			break;

		case RT5739:
			dvfs_ops = &rt5739_ops;
			break;

		case FP6359:
			dvfs_ops = &fp6359_ops;
			break;

		default:
			ERR("Error! Unsupported PMIC for pv_comp!\n");
			break;
	}

	return dvfs_ops;
}


void pv_comp(int cpu_pll, int gpu_pll)
{
	const dvfs_ops_t * vcpu_dvfs_ops = NULL;
	const dvfs_ops_t * vcore_dvfs_ops = NULL;
	unsigned int vcpu_pmic_id, vcore_pmic_id;
	int leakage_info = 0;
	int volt = 0, final_volt = 0, default_volt = 0;;
	int ret = 0;

	vcpu_pmic_id = get_pmic_id(PMIC_SOC_VCPU);
	vcore_pmic_id = get_pmic_id(PMIC_SOC_VCORE);
	vcpu_dvfs_ops = get_dvfs_ops(vcpu_pmic_id);
	vcore_dvfs_ops = get_dvfs_ops(vcore_pmic_id);

	leakage_info = get_leakage_info();
	if(!leakage_info) {
		ERR("leakage is not present, skip pv_comp !!!!\n");
		return;
	}

	if (vcpu_pmic_id != PMIC_UNKNOW)
	{
		if (vcpu_dvfs_ops->init != NULL){
			ret = vcpu_dvfs_ops->init();
			if (ret != S_OK)
				ERR("WARN: pv_comp PMIC init fail!\n");
		}

		volt = get_vcpu_volt_from_dvfs(cpu_pll, leakage_info);
		if(volt != -1)
			final_volt = volt;
		else {
			ERR("Failed to parse valid vcpu from PVComp table !\n");
			goto set_vcore;
		}

		default_volt = vcpu_dvfs_ops->get_vcpu_volt();
		if(default_volt < 0) {
			ERR("Failed to get current Vcpu! \n");
			return ;
		}

		volt = default_volt;
		if(volt == final_volt) {
			NOTICE("Vcpu is %duV, default setting by hardware. \n",volt);
		} else {
			ret = vcpu_dvfs_ops->set_vcpu_volt(default_volt, final_volt);
			if(ret)
				ERR("Failed to set Vcpu from %duv to %duv\n", default_volt, final_volt);
			else
				NOTICE("set Vcpu from %duv to %duv\n", default_volt, final_volt);
		}
	}

set_vcore:
	if (vcore_pmic_id != PMIC_UNKNOW)
	{
		if (vcore_dvfs_ops->init != NULL){
			ret = vcore_dvfs_ops->init();
			if (ret != S_OK)
				ERR("WARN: pv_comp PMIC init fail!\n");
		}

		volt = get_vcore_volt_from_dvfs(gpu_pll, leakage_info);
		if(volt != -1)
			final_volt = volt;
		else {
			ERR("Failed to parse valid vcore from PVComp table !\n");
			return;
		}

		default_volt = vcore_dvfs_ops->get_vcore_volt();
		if(default_volt < 0) {
			ERR("Failed to get current Vcore! \n");
			return;
		}

		volt = default_volt;
		if(volt == final_volt) {
			NOTICE("Vcore is %duV, default setting by hardware. \n", volt);
		} else {
			ret = vcore_dvfs_ops->set_vcore_volt(default_volt, final_volt);
			if(ret) {
				ERR("Failed to set Vcore from %duv to %duv\n", default_volt, final_volt);
			} else {
				NOTICE("set Vcore from %duv to %duv\n", default_volt, final_volt);
			}
		}
	}

	return ;
}
