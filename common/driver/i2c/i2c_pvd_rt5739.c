/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright (C) 2013-2020 Synaptics Incorporated. All rights reserved.
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
#include "i2c_driver.h"
#include "io.h"
#include "debug.h"
#include "chip_voltage_info.h"
#include "timer.h"
#include "pv_comp.h"

// all defined under uv
#define RT5739_STEPuV 5000
#define RT5739_MIN_VOLTAGE 300000
#define RT5739_MAX_VOLTAGE 1300000

#define RT5733_STEPuV 6250
#define RT5733_MIN_VOLTAGE 270000
#define RT5733_MAX_VOLTAGE 1401250

#define SLAVE_ADDR 	0x50
#define RSVD		0xFFFFFFFF
#define REG_VSEL1	0x1
#define REG_CTRL	0x2
#define REG_ID1		0x3
#define DID_MASK	0x0F
#define RT5733_DID	0x01

static int g_step_uv = RT5739_STEPuV;
static int g_min_volt = RT5739_MIN_VOLTAGE;
static int g_max_volt = RT5739_MAX_VOLTAGE;

extern int i2c_master_init(int id, int speed, int b_10bit_addr);
extern int i2c_master_write_and_read(int id, int target_addr, unsigned char* send_buff, int send_len, unsigned char* recv_buff, int recv_len);

static int rt5739_chip_init(int master_id, int slaveAddr)
{
	unsigned char vid = 0;
	int ret;

	i2c_master_init(master_id, 100, 0);
	ret = i2c_master_write_and_read(master_id, slaveAddr, (unsigned char*)REG_ID1, 1, &vid, 1);
	if (ret) {
		dbg_printf(PRN_ERR, "PMIC rt573x: failed to read VID\n");
		return ret;
	}

	if ((vid & DID_MASK) == RT5733_DID) {
		g_step_uv = RT5733_STEPuV;
		g_min_volt = RT5733_MIN_VOLTAGE;
		g_max_volt = RT5733_MAX_VOLTAGE;
	} else {
		g_step_uv = RT5739_STEPuV;
		g_min_volt = RT5739_MIN_VOLTAGE;
		g_max_volt = RT5739_MAX_VOLTAGE;
	}

	return 0;
}

static int rt5739_pmic_volt2data(int volt, int *p_data)
{
	unsigned int data;

	if ((volt >= g_min_volt) && (volt <= g_max_volt))
		data = (volt - g_min_volt) / g_step_uv;
	else
		data = RSVD;

	if (data == RSVD)
		return -1;
	else {
		*p_data = data;
		return 0;
	}
}

static int rt5739_pmic_data2volt(int *p_volt, int vdata)
{
	unsigned int volt;
	unsigned int data = vdata;
	int max_val = (g_max_volt - g_min_volt) / g_step_uv;

	if (data <= max_val)
		volt = g_min_volt + (data * g_step_uv);
	else
		volt = RSVD;

	if (volt == RSVD)
		return -1;
	else {
		*p_volt = volt;
		return 0;
	}
}

static int rt5739_set_vol(int master_id, int slaveAddr, int volt)
{
	int target = volt;
	unsigned char buff[2];
	unsigned char data;
	int vdata = 0;
	int ret = 1;

	rt5739_chip_init(master_id, slaveAddr);

	ret = rt5739_pmic_volt2data(target, &vdata);
	if (ret == 0) {
		i2c_master_init(master_id, 100, 0);  //int id, int speed, int b_10bit_addr

		buff[0] = (unsigned char)REG_VSEL1;
		buff[1] = (unsigned char)vdata;
		ret = i2c_master_write_and_read(master_id, slaveAddr, buff, 2, (unsigned char*)0, 0);
		if (ret) {
			dbg_printf(PRN_ERR,"PMIC i2c write fail !\n");
			return ret;
		}

		buff[0] = (unsigned char)REG_VSEL1;
		ret = i2c_master_write_and_read(master_id, slaveAddr, buff, 1, &data, 1);
		if (ret) {
			dbg_printf(PRN_ERR,"PMIC i2c read fail !\n");
			return ret;
		}

		if (vdata != data) {
			dbg_printf(PRN_ERR, "PMIC rt573x: read:0x%02x != write:0x%02x\n", data, vdata);
			return -1;
		}
		mdelay(1);
	}
	else {
		dbg_printf(PRN_ERR, "Illegal target voltage for PMIC rt573x !\n");
		return -1;
	}
	return ret;
}

static int rt5739_get_vol(int master_id, int slaveAddr)
{
	unsigned char read = 0xff;
	unsigned char buff[2];
	int volt = -1;
	unsigned int bulk_reg = REG_VSEL1;
	unsigned int bulk_ctrl = REG_CTRL;
	int ret = 1;

	rt5739_chip_init(master_id, slaveAddr);

	i2c_master_init(master_id, 100, 0);  //int id, int speed, int b_10bit_addr

	ret = i2c_master_write_and_read(master_id, slaveAddr, (unsigned char*)&bulk_ctrl, 1, (unsigned char*)&read, 1);
	if (ret) {
		dbg_printf(PRN_ERR,"PMIC i2c read fail !\n");
		return ret;
	}

	buff[0] = (unsigned char)REG_CTRL;
	buff[1] = (unsigned char)(read & 0x7F); //disable fast discharge
	ret = i2c_master_write_and_read(master_id, slaveAddr, buff, 2, (unsigned char*)0, 0);
	if (ret) {
		dbg_printf(PRN_ERR,"PMIC i2c write fail !\n");
		return ret;
	}

	ret = i2c_master_write_and_read(master_id, slaveAddr, (unsigned char*)&bulk_reg, 1, (unsigned char*)&read, 1);

	ret |= rt5739_pmic_data2volt(&volt, (int)read);
	if(ret) {
		dbg_printf(PRN_ERR," i2c read fail\n");
		return -1;
	}
	return volt;
}

static int i2c_get_cpu_volt(void)
{
	int volt = rt5739_get_vol(vout_cpu_id, SLAVE_ADDR);

	if(volt < 0)
		return 0;

	return volt;
}

static int i2c_get_core_volt(void)
{
	int volt = rt5739_get_vol(vout_core_id, SLAVE_ADDR);

	if(volt < 0)
		return 0;

	return volt;
}

static int i2c_set_volt(int master_id, int from, int to)
{
	int volt = from;

	while (volt != to){
		if (volt > to){
			volt -= g_step_uv;
			if (volt < to)
				volt = to;
		}else{
			volt += g_step_uv;
			if (volt > to)
				volt = to;
		}

		if(0 != rt5739_set_vol(master_id, SLAVE_ADDR, volt))
			return 1;
	}

	return 0;
}

static int i2c_set_vcpu_volt(int from, int to)
{
	return i2c_set_volt(vout_cpu_id, from ,to);
}

static int i2c_set_vcore_volt(int from, int to)
{
	return i2c_set_volt(vout_core_id, from ,to);
}

const dvfs_ops_t rt5739_ops = {
	.get_vcpu_volt = i2c_get_cpu_volt,
	.get_vcore_volt = i2c_get_core_volt,
	.set_vcpu_volt = i2c_set_vcpu_volt,
	.set_vcore_volt =i2c_set_vcore_volt
};
