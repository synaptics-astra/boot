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
#include "i2c_driver.h"
#include "global.h"
#include "io.h"
#include "debug.h"
#include "memmap.h"
#include "chip_voltage_info.h"
#include "SysMgr.h"
#include "apbRegBase.h"
#include "timer.h"
#include "pv_comp.h"

#ifndef VOUT_CPU_ID
#define VOUT_CPU_ID                             (2)
#endif

#ifndef VOUT_CORE_ID
#define VOUT_CORE_ID                    (0)
#endif

#ifndef VOUT_CPU_CHANNEL
#define VOUT_CPU_CHANNEL                (0)
#endif

#ifndef VOUT_CORE_CHANNEL
#define VOUT_CORE_CHANNEL               (1)
#endif

// all defined under uv
#ifndef MINIMAL_VOL_STEP_M88PG86X
#define MINIMAL_VOL_STEP_M88PG86X 25000
#endif
#define MIN_VOLTAGE 900000
#define MAX_VOLTAGE 1600000

extern int i2c_master_init(int id, int speed, int b_10bit_addr);
extern int i2c_master_write_and_read(int id, int target_addr, unsigned char* send_buff, int send_len, unsigned char* recv_buff, int recv_len);

//assume the lowest voltage is 900mv
//all voltage is *10 here to handle 12.5mv case
static int pmic_volt2data(int volt, int *p_data)
{
	if((volt < MIN_VOLTAGE) || (volt > MAX_VOLTAGE))
		return -1;
	if((volt - MIN_VOLTAGE) % 25000)
		*p_data = (volt - MIN_VOLTAGE) / 25000 + 1 + 0x7;
	else
		*p_data = (volt - MIN_VOLTAGE) / 25000 + 0x7;

	return 0;
}

static int pmic_data2volt(int *p_volt, int data)
{
	*p_volt = (MIN_VOLTAGE + (data - 0x7) * 25000);

	return 0;
}

static int i2c_get_volt(int master_id)
{
	int ret, volt;
	unsigned char buff;
	unsigned char read = 0xff;
	unsigned char slaveAddr = 0x19; // PG867 slave address is 0x19
	buff = 0x24;

	i2c_master_init(master_id, 100, 0);
	ret = i2c_master_write_and_read(master_id, (int)slaveAddr, (unsigned char*)&buff, 1, (unsigned char*)&read, 1);
	if (ret != 0) {
		dbg_printf(PRN_ERR," i2c read fail\n");
		return -ret;
	}

	pmic_data2volt(&volt, (int)read);

	return volt;
}

static int diag_i2c_volt_control(int master_id, int volt_index)
{
	unsigned char buff[2];
	unsigned char read = 0xff;
	int ret;
	// VIndex: [0-0.90,1-0.925,2-0.95(default),3-0.975,4-1.00,5-1.025,6-1.050]
	unsigned char slaveAddr = 0x19; // PG867 slave address is 0x19
	buff[0] = 0x24;  // PG867 buck1 target voltage 2 (active) register
	buff[1] = 0x7 + volt_index; // 0x7 is 0.90v. each step is 0.025

	i2c_master_init(master_id, 100, 0);
	ret = i2c_master_write_and_read(master_id, (int)slaveAddr, buff, 2, (unsigned char*)0, 0);
	if (ret != 0) {
		dbg_printf(PRN_ERR," i2c write fail\n");
		return ret;
	}

	ret = i2c_master_write_and_read(master_id, (int)slaveAddr, buff, 1, (unsigned char*)&read, 1);
	if (ret != 0) {
		dbg_printf(PRN_ERR," i2c read fail\n");
		return ret;
	}

	if (buff[1] != read) {
		dbg_printf(PRN_ERR," i2c vcore control fail, read:0x%02x != write:0x%02x\n", read, buff[1]);
		return 1;
	}
	//dbg_printf(PRN_DBG," vcore is %4d.%03dv\n", (900+25*volt_index) / 1000, (900+25*volt_index) % 1000);

	return 0;
}

static int i2c_get_cpu_volt(void)
{
	int volt = i2c_get_volt(VOUT_CPU_ID);

	if(volt < 0)
		return 0;

	return volt;
}

static int i2c_get_core_volt(void)
{
	int volt = i2c_get_volt(VOUT_CORE_ID);

	if(volt < 0)
		return 0;

	return volt;
}

static int i2c_set_volt(int master_id, int from, int to)
{
	int volt_index = 0, volt = from;

	while (volt != to){
		if (volt > to){
			volt -= MINIMAL_VOL_STEP_M88PG86X;
			if (volt < to)
				volt = to;
		}else{
			volt += MINIMAL_VOL_STEP_M88PG86X;
			if (volt > to)
				volt = to;
		}
		if(0 != pmic_volt2data(volt, &volt_index)) {
			dbg_printf(PRN_ERR,"error convert volt %d to index %d.\n", volt, volt_index);
			return 1;
		}

		if(0 != diag_i2c_volt_control(master_id, volt_index))
			return 1;
		udelay(50); 	// delay 50us to wait it stable
	}

	return 0;
}

static int i2c_set_vcpu_volt(int from, int to)
{
	return i2c_set_volt(VOUT_CPU_ID, from ,to);
}

static int i2c_set_vcore_volt(int from, int to)
{
	return i2c_set_volt(VOUT_CORE_ID, from ,to);
}

const dvfs_ops_t m88pg86x_ops = {
	.get_vcpu_volt = i2c_get_cpu_volt,
	.get_vcore_volt = i2c_get_core_volt,
	.set_vcpu_volt = i2c_set_vcpu_volt,
	.set_vcore_volt = i2c_set_vcore_volt
};
