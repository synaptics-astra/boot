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
#include "i2c_driver.h"
#include "global.h"
#include "io.h"
#include "debug.h"
#include "chip_voltage_info.h"
#include "timer.h"
#include "pv_comp.h"
#include "pinmux.h"

#define TPS6287X_VSET		0x00
#define TPS6287X_CTRL1		0x01
#define TPS6287X_CTRL1_VRAMP	0x03
#define TPS6287X_CTRL1_FPWMEN	(1 << 4)
#define TPS6287X_CTRL1_SWEN	(1 << 5)
#define TPS6287X_CTRL2		0x02
#define TPS6287X_CTRL2_VRANGE	0x0C
#define TPS6287X_CTRL3		0x03
#define TPS6287X_STATUS		0x04
#define TPS6287X_DISCHARGE_MASK	(1 << 3)

#define TPS6287X_STEP_VRANGE0_uV 1250  // 1.25mV
#define TPS6287X_STEP_VRANGE1_uV 2500  // 2.5mV
#define TPS6287X_STEP_VRANGE2_uV 5000  // 5mV
#define TPS6287X_STEP_VRANGE3_uV 10000 // 10mV
#define MIN_VOLTAGE 400000
#define MAX_VOLTAGE 1675000

#define SLAVE_ADDR 	0x40
#define RSVD		0xFFFFFFFF

extern int i2c_master_init(int id, int speed, int b_10bit_addr);
extern int i2c_master_write_and_read(int id, int target_addr, unsigned char* send_buff, int send_len, unsigned char* recv_buff, int recv_len);

static int tps6287x_get_step_uv(int master_id, int slaveAddr)
{
	unsigned char ctrl2_reg = TPS6287X_CTRL2;
	unsigned char ctrl2_val = 0;
	int vrange = 2; //default 5mV step
	int ret;

	ret = i2c_master_init(master_id, 100, 0);
	if (ret) {
		return TPS6287X_STEP_VRANGE2_uV;
	}

	ret = i2c_master_write_and_read(master_id, slaveAddr, &ctrl2_reg, 1, &ctrl2_val, 1);
	if (ret == 0) {
		vrange = (ctrl2_val & TPS6287X_CTRL2_VRANGE) >> 2;
	}

	switch (vrange) {
		case 0: return TPS6287X_STEP_VRANGE0_uV; // 1.25mV
		case 1: return TPS6287X_STEP_VRANGE1_uV; // 2.5mV
		case 2: return TPS6287X_STEP_VRANGE2_uV; // 5mV
		case 3: return TPS6287X_STEP_VRANGE3_uV; // 10mV
		default: return TPS6287X_STEP_VRANGE2_uV; // 5mV
	}
}

static int tps6287x_pmic_volt2data_with_vrange(int volt, int *p_data, int vrange)
{
	unsigned int data;
	int base_voltage;
	int step_uv;

	switch (vrange) {
		case 0: // 00b = 0.4V to 0.71875V in 1.25-mV steps
			base_voltage = 400000;
			step_uv = TPS6287X_STEP_VRANGE0_uV;
			break;
		case 1: // 01b = 0.4V to 1.0375V in 2.5-mV steps
			base_voltage = 400000;
			step_uv = TPS6287X_STEP_VRANGE1_uV;
			break;
		case 2: // 10b = 0.4V to 1.675V in 5-mV steps
			base_voltage = 400000;
			step_uv = TPS6287X_STEP_VRANGE2_uV;
			break;
		case 3: // 11b = 0.8V to 3.35V in 10-mV steps
			base_voltage = 800000;
			step_uv = TPS6287X_STEP_VRANGE3_uV;
			break;
		default:
			base_voltage = 400000;
			step_uv = TPS6287X_STEP_VRANGE2_uV;
			break;
	}

	if (volt >= base_voltage) {
		data = (volt - base_voltage) / step_uv;
		if (data <= 255) { //8bit
			*p_data = data;
			return 0;
		}
	}

	return -1;
}

static int tps6287x_pmic_data2volt_with_vrange(int *p_volt, int vdata, int vrange)
{
	int base_voltage;
	int step_uv;

	switch (vrange) {
		case 0: // 00b = 0.4V to 0.71875V in 1.25-mV steps
			base_voltage = 400000;
			step_uv = TPS6287X_STEP_VRANGE0_uV;
			break;
		case 1: // 01b = 0.4V to 1.0375V in 2.5-mV steps
			base_voltage = 400000;
			step_uv = TPS6287X_STEP_VRANGE1_uV;
			break;
		case 2: // 10b = 0.4V to 1.675V in 5-mV steps
			base_voltage = 400000;
			step_uv = TPS6287X_STEP_VRANGE2_uV;
			break;
		case 3: // 11b = 0.8V to 3.35V in 10-mV steps
			base_voltage = 800000;
			step_uv = TPS6287X_STEP_VRANGE3_uV;
			break;
		default:
			base_voltage = 400000;
			step_uv = TPS6287X_STEP_VRANGE2_uV;
			break;
	}

	*p_volt = base_voltage + (vdata * step_uv);

	return 0;
}

static int tps6287x_set_vol(int master_id, int slaveAddr, int volt)
{
	int target = volt;
	unsigned char buff[2];
	unsigned char data_reg0, data;
	int vdata = 0;
	int ret = 0;
	int vrange = 0;

	unsigned char ctrl2_reg = TPS6287X_CTRL2;
	unsigned char ctrl2_val = 0;
	ret = i2c_master_init(master_id, 100, 0);
	if (ret) {
		dbg_printf(PRN_ERR,"TPS6287X: I2C master init failed in set_vol, ret=%d\n", ret);
		return ret;
	}

	ret = i2c_master_write_and_read(master_id, slaveAddr, &ctrl2_reg, 1, &ctrl2_val, 1);
	if (ret == 0) {
		vrange = (ctrl2_val & TPS6287X_CTRL2_VRANGE) >> 2;
		dbg_printf(PRN_DBG,"TPS6287X: CTRL2 register = 0x%02x, mask=0x%02x, VRANGE bits = 0x%02x, VRANGE = %d for voltage setting\n",
			ctrl2_val, TPS6287X_CTRL2_VRANGE, (ctrl2_val & TPS6287X_CTRL2_VRANGE), vrange);
	} else {
		dbg_printf(PRN_ERR,"TPS6287X: Failed to read CTRL2 register for VRANGE, ret=%d, using default VRANGE=2\n", ret);
		vrange = 2;
	}

	if (tps6287x_pmic_volt2data_with_vrange(target, &vdata, vrange)) {
		dbg_printf(PRN_ERR,"TPS6287X: volt2data conversion failed for %duV with VRANGE=%d\n", target, vrange);
		return -1;
	}

	data_reg0 = (unsigned char)vdata;

	buff[0] = (unsigned char)TPS6287X_VSET;
	buff[1] = (unsigned char)data_reg0;
	ret = i2c_master_write_and_read(master_id, slaveAddr, buff, 2, (unsigned char*)0, 0);
	if (ret) {
		dbg_printf(PRN_ERR,"PMIC i2c write fail! ret=%d\n", ret);
		return ret;
	}

	mdelay(2);

	buff[0] = (unsigned char)TPS6287X_VSET;
	ret = i2c_master_write_and_read(master_id, slaveAddr, buff, 1, &data, 1);
	if (ret) {
		dbg_printf(PRN_ERR,"PMIC i2c read fail !\n");
		return ret;
	}

	if (data_reg0 != data) {
		dbg_printf(PRN_DBG, "TPS6287x: First verification failed, read:0x%02x != write:0x%02x, retrying...\n", data, data_reg0);
		mdelay(5);
		ret = i2c_master_write_and_read(master_id, slaveAddr, buff, 1, &data, 1);
		if (ret) {
			dbg_printf(PRN_ERR,"PMIC i2c read fail on retry!\n");
			return ret;
		}

		if (data_reg0 != data) {
			dbg_printf(PRN_ERR, "PMIC TPS6287x: Final verification failed: read:0x%02x != write:0x%02x\n", data, data_reg0);
			return -1;
		} else {
			dbg_printf(PRN_DBG, "TPS6287x: Verification successful on retry: 0x%02x\n", data);
		}
	}

	return ret;
}

static int tps6287x_get_vol(int master_id, int slaveAddr)
{
	unsigned char read = 0xff;
	unsigned char buff[2];
	int volt;
	unsigned int bulk_reg = TPS6287X_VSET;
	unsigned int bulk_ctrl = TPS6287X_CTRL3;
	int ret = 1;

	ret = i2c_master_init(master_id, 100, 0);  //int id, int speed, int b_10bit_addr
	if (ret) {
		ERR("TPS6287X: I2C master init failed in get_vol, ret=%d\n", ret);
		return ret;
	}

	ret = i2c_master_write_and_read(master_id, slaveAddr, (unsigned char*)&bulk_ctrl, 1, (unsigned char*)&read, 1);
	if (ret) {
		ERR("TPS6287X: I2C read CTRL3 fail! master_id=%d, slaveAddr=0x%02x, reg=0x%02x, ret=%d\n",
			master_id, slaveAddr, bulk_ctrl, ret);
		return ret;
	}

	buff[0] = (unsigned char)TPS6287X_CTRL3;
	buff[1] = (unsigned char)(read & ~TPS6287X_DISCHARGE_MASK); //disable fast discharge
	ret = i2c_master_write_and_read(master_id, slaveAddr, buff, 2, (unsigned char*)0, 0);
	if (ret) {
		dbg_printf(PRN_ERR,"PMIC i2c write fail !\n");
		return ret;
	}

	unsigned char ctrl2_reg = TPS6287X_CTRL2;
	unsigned char ctrl2_val = 0;
	int vrange = 0;
	ret = i2c_master_write_and_read(master_id, slaveAddr, &ctrl2_reg, 1, &ctrl2_val, 1);
	if (ret == 0) {
		vrange = (ctrl2_val & TPS6287X_CTRL2_VRANGE) >> 2;
		dbg_printf(PRN_DBG,"TPS6287X: [GET_VOL] CTRL2 register = 0x%02x, mask=0x%02x, VRANGE bits = 0x%02x, VRANGE = %d\n",
			ctrl2_val, TPS6287X_CTRL2_VRANGE, (ctrl2_val & TPS6287X_CTRL2_VRANGE), vrange);
	} else {
		dbg_printf(PRN_ERR,"TPS6287X: [GET_VOL] Failed to read CTRL2 register for VRANGE, ret=%d, using default VRANGE=2\n", ret);
		vrange = 2;
	}

	ret = i2c_master_write_and_read(master_id, slaveAddr, (unsigned char*)&bulk_reg, 1, (unsigned char*)&read, 1);

	dbg_printf(PRN_DBG,"TPS6287X: REG_VSET = 0x%02x (%d)\n", read, read);

	ret |= tps6287x_pmic_data2volt_with_vrange(&volt, (int)read, vrange);
	if(ret) {
		ERR(" i2c read fail\n");
		return -1;
	}

	return volt;
}

static int i2c_get_cpu_volt(void)
{
	int volt = tps6287x_get_vol(vout_cpu_id, SLAVE_ADDR);

	if(volt < 0)
		return 0;

	return volt;
}

static int i2c_get_core_volt(void)
{
	int volt = tps6287x_get_vol(vout_core_id, SLAVE_ADDR);

	if(volt < 0)
		return 0;

	return volt;
}

static int i2c_set_volt(int master_id, int from, int to)
{
	int volt = from;
	int step_uv = tps6287x_get_step_uv(master_id, SLAVE_ADDR);

	while (volt != to) {
		if (volt > to) {
			volt -= step_uv;
			if (volt < to)
				volt = to;
		} else {
			volt += step_uv;
			if (volt > to)
				volt = to;
		}

		if(0 != tps6287x_set_vol(master_id, SLAVE_ADDR, volt))
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

const dvfs_ops_t tps6287x_ops = {
	.get_vcpu_volt = i2c_get_cpu_volt,
	.get_vcore_volt = i2c_get_core_volt,
	.set_vcpu_volt = i2c_set_vcpu_volt,
	.set_vcore_volt = i2c_set_vcore_volt
};
