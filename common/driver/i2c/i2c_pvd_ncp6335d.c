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
#ifndef MINIMAL_VOL_STEP_NCP6335D
#define MINIMAL_VOL_STEP_NCP6335D 25000
#endif
#define MIN_VOLTAGE 600000
#define MAX_VOLTAGE 1375000

#define SLAVE_ADDR 0x0

extern int i2c_master_init(int id, int speed, int b_10bit_addr);
extern int i2c_master_write_and_read(int id, int target_addr, unsigned char* send_buff, int send_len, unsigned char* recv_buff, int recv_len);

static int pmic_volt2data(int volt, unsigned char *p_data)
{
	unsigned char data;

	if ((volt >= MIN_VOLTAGE) && (volt <= MAX_VOLTAGE))
		data = ((volt - MIN_VOLTAGE)/25000)*4;
	else
		return -1;

	data = data & 0x7f;
	*p_data = data;
	return 0;
}

static int pmic_data2volt(int *p_volt, unsigned char vdata)
{
	*p_volt = ((vdata / 4) * 25000 + MIN_VOLTAGE);

	return 0;
}

static int ncp6335d_set_vol(int master_id, bool volt_sel, int vol)
{
    int target = vol;
    unsigned char data,data_read = 0;
    unsigned int buck_reg0;
    const char slaveAddr = 0x1C;
    unsigned char buff[2];
    int ret = 0;

    if (volt_sel == 0){
        buck_reg0 = 0x11;
    } else {
        dbg_printf(PRN_ERR, "Don't support BUCK2\n");
        return 0;
    }

    ret = pmic_volt2data(target, &data);

    if (ret == 0) {
        data = 0x80 | data;

        buff[0] = (unsigned char)buck_reg0;
        buff[1] = (unsigned char)data;
        i2c_master_init(master_id, 100, 0);
        // write
        ret = i2c_master_write_and_read(master_id, slaveAddr, buff, 2, (unsigned char*)0, 0);
        if (ret) {
                dbg_printf(PRN_ERR,"PMIC i2c write fail !\n");
                return ret;
        }

        // read
        buff[0] = (unsigned char)buck_reg0;
        ret = i2c_master_write_and_read(master_id, slaveAddr, buff, 1, &data_read, 1);
        if (ret) {
                dbg_printf(PRN_ERR,"PMIC i2c read fail !\n");
                return ret;
        }

        if (data_read != data) {
                dbg_printf(PRN_ERR, "PMIC ncp6335: read:0x%02x != write:0x%02x\n", data, data_read);
                return -1;
        }

        dbg_printf(PRN_INFO, "set to %d mv\n", target);
        mdelay(1);
    }

    return ret;
}

static int ncp6335d_get_vol(int master_id, bool volt_sel)
{
    unsigned int read = 0xff;
    unsigned int buck_reg;
    const char slaveAddr = 0x1C;
    int ret;
	int volt = 0;

    if (volt_sel == 0){
        buck_reg = 0x11;
    } else {
        dbg_printf(PRN_ERR, "Don't support BUCK2\n");
        return -1;
    }

    i2c_master_init(master_id, 100, 0);  //int id, int speed, int b_10bit_addr
    ret = i2c_master_write_and_read(master_id, slaveAddr, (unsigned char*)&buck_reg, 1, (unsigned char*)&read, 1);
    if (ret != 0)
    {
        dbg_printf(PRN_ERR," i2c read fail\n");
        return -ret;
    }
    dbg_printf(PRN_INFO, "\nREG_VSEL0 = 0x%x \n", read);

    read = read & 0x7F;

	if(0 != pmic_data2volt(&volt , read)) {
		return -ret;
	}

    return volt;
}

static int i2c_get_cpu_volt(void)
{
	int volt = ncp6335d_get_vol(VOUT_CPU_ID, SLAVE_ADDR);

	if(volt < 0)
		return 0;

	return volt;
}

static int i2c_get_core_volt(void)
{
	int volt = ncp6335d_get_vol(VOUT_CORE_ID, SLAVE_ADDR);

	if(volt < 0)
		return 0;

	return volt;
}

static int i2c_set_volt(int master_id, int from, int to)
{
	int volt = from;

	while (volt != to){
		if (volt > to){
			volt -= MINIMAL_VOL_STEP_NCP6335D;
			if (volt < to)
				volt = to;
		}else{
			volt += MINIMAL_VOL_STEP_NCP6335D;
			if (volt > to)
				volt = to;
		}

		if(0 != ncp6335d_set_vol(master_id, SLAVE_ADDR, volt))
			return 1;
		//udelay(50); 	//FIXME: 1ms delayed in above function
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


const dvfs_ops_t ncp6335d_ops = {
	.get_vcpu_volt = i2c_get_cpu_volt,
	.get_vcore_volt = i2c_get_core_volt,
	.set_vcpu_volt = i2c_set_vcpu_volt,
	.set_vcore_volt =i2c_set_vcore_volt
};
