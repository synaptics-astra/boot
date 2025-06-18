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
#include "i2c_driver.h"
#include "global.h"
#include "io.h"
#include "debug.h"
#include "memmap.h"
#include "chip_voltage_info.h"
#include "SysMgr.h"
#include "apbRegBase.h"
#include "pv_comp.h"
#include "pinmux.h"

#ifndef VOUT_CPU_ID
#define VOUT_CPU_ID                             (1)
#endif

#ifndef VOUT_CORE_ID
#define VOUT_CORE_ID                    (2)
#endif

#define VCORE_SLAVE_ADDR 0x60
#define VCPU_SLAVE_ADDR 0x62

#define MAXmV	1000
#define MinmV	600
#define DELTAmV	50
#define MP8869STEPmV   10

#define MAXuV	(MAXmV * 1000)
#define MinuV	(MinmV * 1000)
#define MP8869STEPuV   (MP8869STEPmV * 1000)

//#define MP886X_DBG
#ifdef MP886X_DBG
static void mp886x_vcpu_set_verify(void);
static void mp886x_vcore_set_verify(void);
#endif


extern int i2c_master_init(int id, int speed, int b_10bit_addr);
extern int i2c_master_write_and_read(int id, int target_addr, unsigned char* send_buff, int send_len, unsigned char* recv_buff, int recv_len);

static int pmic_reg_wt(int master_id, int slave_addr, unsigned int reg, unsigned int val)
{
	int ret = 0;
	unsigned char w_buf[2];

	w_buf[0] = (unsigned char)reg;
	w_buf[1] = (unsigned char)val;

	i2c_master_init(master_id, 100, 0);  //int id, int speed, int b_10bit_addr
	ret = i2c_master_write_and_read(master_id, slave_addr, w_buf, 2, 0, 0);

	if (ret){
		dbg_printf(PRN_ERR,"%s FAIL\n", __func__);
	}

	return ret;
}

static int pmic_reg_rd(int master_id, int slave_addr, unsigned int reg, unsigned int * p_val)
{
	int ret = 0;
	unsigned int read = 0;

	unsigned char w_buf[2];

	w_buf[0] = (unsigned char)reg;
	read = 0xff;

	i2c_master_init(master_id, 100, 0);  //int id, int speed, int b_10bit_addr

	// read
	ret = i2c_master_write_and_read(master_id, slave_addr, w_buf, 1, (unsigned char*)&read, 1);

	if (ret){
		dbg_printf(PRN_ERR,"%s FAIL\n", __func__);
	}else {
		*p_val = read;
	}

	return ret;
}

static int mp886x_data2vol(unsigned int *p_data, unsigned int *p_vol)
{
	unsigned int vol;
	unsigned int data = *p_data;
	vol = data * 10 + 600;
	*p_vol = vol;
	return 0;
}

static int mp886x_vol2data(int *p_vol, int *p_data)
{
	unsigned int data;
	unsigned int vref = 800;

	if ((*p_vol >= MinmV) && (*p_vol <= (MAXmV+DELTAmV)))
		vref = *p_vol;
	data = (vref-600)/10;
	*p_data = data;
	return 0;
}

static int mp886x_get_volt(int master_id, int slave_addr)
{
	unsigned int volt;
	unsigned int read = 0;
	unsigned int bulk_reg;
	int ret;

	bulk_reg = 0x00;
	ret = pmic_reg_rd(master_id, slave_addr, bulk_reg, &read);
	if (ret != 0) {
		lgpl_printf(" i2c read fail %d %s\n", __LINE__, __func__);
		return -ret;
	}
	if(read & 0x80)
		volt = 800;
	else
		ret = mp886x_data2vol(&read, &volt);

	return volt;
}

static int mp8869_change_vsel(int master_id, int slave_addr, int vsel)
{
	unsigned int data;
	int ret;

	ret = pmic_reg_rd(master_id, slave_addr, 01, &data);
	if (ret != 0) {
		lgpl_printf(" i2c read fail %d %s\n", __LINE__, __func__);
		return -ret;
	}
	data |= 0x40;
	ret = pmic_reg_wt(master_id, slave_addr, 01, data);
	if (ret != 0) {
		lgpl_printf(" i2c write fail %d %s\n", __LINE__, __func__);
		return -ret;
	}
	ret = pmic_reg_wt(master_id, slave_addr, 00, vsel & 0x7F);
	if (ret != 0) {
		lgpl_printf(" i2c write fail %d %s\n", __LINE__, __func__);
		return -ret;
	}
	do{
		ret = pmic_reg_rd(master_id, slave_addr, 01, &data);
		if (ret != 0) {
			lgpl_printf(" i2c read fail %d %s\n", __LINE__, __func__);
			return -ret;
		}
	}while(data&0x40);

	return ret;
}

static int mp8869_set_vol(int master_id, int slave_addr, unsigned int vol)
{
	int target = vol/1000;
	int cur;
	int data;
	int direct =0;
	int ret = 0;

	if(target%10 >=5)
		target = target/10 *10 +10;
	else
		target = target/10 *10;
	dbg_printf(PRN_RES, "set vol to %dmV\n", target);

	do{
		cur = mp886x_get_volt(master_id, slave_addr);
		dbg_printf(PRN_DBG, "cur = %d, target = %d\n", cur, target);
		if ((target> cur)&&((target -cur) > (MP8869STEPmV/2) ))
		{
			dbg_printf(PRN_DBG, "cur%d, +%d\n", cur, MP8869STEPmV);
			cur = cur + MP8869STEPmV;
			direct = 1;
		}
		else if((target< cur)&&((cur- target) > (MP8869STEPmV/2) ))
		{
			dbg_printf(PRN_DBG, "cur%d, -%d\n", cur, MP8869STEPmV);
			cur = cur - MP8869STEPmV;
			direct = -1;
		}
		else
		{
			direct  = 0;
			return 0;
		}

		ret = mp886x_vol2data(&cur, &data);
		ret = mp8869_change_vsel(master_id, slave_addr, data);
	}while(direct);

	return ret;
}

static int mp8869_init(int master_id, int slave_addr)
{
	unsigned int data;
	int ret;

	/* Adjust switching frequency*/
	ret = pmic_reg_rd(master_id, slave_addr, 02, &data);
	if (ret != 0) {
		lgpl_printf(" i2c read fail %d %s\n", __LINE__, __func__);
		return -ret;
	}
	data |= (0x03 << 4);
	ret = pmic_reg_wt(master_id, slave_addr, 02, data);
	if (ret != 0) {
		lgpl_printf(" i2c write fail %d %s\n", __LINE__, __func__);
		return -ret;
	}

	return ret;
}

/**
if the reference  adjustment is within 50mV, GO_BIT will not reset to 0 automatically.
*/
static int mp8867_change_vsel(int master_id, int slave_addr, unsigned int vsel)
{
	unsigned int data;
	int ret;

	ret = pmic_reg_rd(master_id, slave_addr, 01, &data);
	if (ret != 0) {
		lgpl_printf(" i2c read fail %d %s\n", __LINE__, __func__);
		return -ret;
	}

	data |= 0x40;
	ret = pmic_reg_wt(master_id, slave_addr, 01, data);
	if (ret != 0) {
		lgpl_printf(" i2c write fail %d %s\n", __LINE__, __func__);
		return -ret;
	}
	ret = pmic_reg_wt(master_id, slave_addr, 00, vsel & 0x7F);
	if (ret != 0) {
		lgpl_printf(" i2c write fail %d %s\n", __LINE__, __func__);
		return -ret;
	}
	ret = pmic_reg_rd(master_id, slave_addr, 01, &data);
	if (ret != 0) {
		lgpl_printf(" i2c read fail %d %s\n", __LINE__, __func__);
		return -ret;
	}

	data &= ~0x40;

	ret = pmic_reg_wt(master_id, slave_addr, 01, data);
	if (ret != 0) {
		lgpl_printf(" i2c write fail %d %s\n", __LINE__, __func__);
		return -ret;
	}
	do{
		ret = pmic_reg_rd(master_id, slave_addr, 01, &data);
		if (ret != 0) {
			lgpl_printf(" i2c read fail %d %s\n", __LINE__, __func__);
			return -ret;
		}
	}while(data&0x40);

	return ret;
}

static int mp8867_set_vol(int master_id, int slave_addr, unsigned int vol)
{
	int target = vol/1000;//change to mv
	int cur;
	int data;
	int direct =0;
	int ret = 0;

	if(target%10 >=5)
		target = target/10 *10 +10;
	else
		target = target/10 *10;
	dbg_printf(PRN_RES, "set vol to %dmV\n", target);

	do{
		cur = mp886x_get_volt(master_id, slave_addr);
		dbg_printf(PRN_DBG, "cur = %d, target = %d\n", cur, target);
		if ((target> cur)&&((target -cur) > (MP8869STEPmV/2) ))
		{
			dbg_printf(PRN_DBG, "cur%d, +%d\n", cur, MP8869STEPmV);
			cur = cur + MP8869STEPmV;
			direct = 1;
		}
		else if((target< cur)&&((cur- target) > (MP8869STEPmV/2) ))
		{
			dbg_printf(PRN_DBG, "cur%d, -%d\n", cur, MP8869STEPmV);
			cur = cur - MP8869STEPmV;
			direct = -1;
		}
		else
		{
			direct  = 0;
			return 0;
		}

		ret = mp886x_vol2data(&cur, &data);
		ret = mp8867_change_vsel(master_id, slave_addr, data);
	}while(direct);

	return ret;
}

static int mp8867_init(int master_id, int slave_addr)
{
	unsigned int data;
	int ret;

	ret = pmic_reg_rd(master_id, slave_addr, 01, &data);
	if (ret != 0) {
		lgpl_printf(" i2c read fail %d %s\n", __LINE__, __func__);
		return -ret;
	}

	/* Adjust switching frequency*/
	data |= (0x03 << 1);

	ret = pmic_reg_wt(master_id, slave_addr, 01, data);
	if (ret != 0) {
		lgpl_printf(" i2c write fail %d %s\n", __LINE__, __func__);
		return -ret;
	}

	return ret;
}

static int i2c_set_vcpu_volt(int from, int to)
{
	UNUSED(from);
	return mp8867_set_vol(VOUT_CPU_ID, VCPU_SLAVE_ADDR, to);
}

static int i2c_get_cpu_volt(void)
{
	//TW1B Selected
	PINMUX_TW1_SELECT(1);
	int volt = mp886x_get_volt(VOUT_CPU_ID, VCPU_SLAVE_ADDR);

	if(volt < 0)
		return 0;

	volt *= 1000;
	dbg_printf(PRN_NONE, "vcpu:%duV\n", volt);
#ifdef MP886X_DBG
	mp886x_vcpu_set_verify();
#endif
	return volt;
}

static int i2c_get_core_volt(void)
{
	//TW2B Selected
	PINMUX_TW2_SELECT(1);
	int volt = mp886x_get_volt(VOUT_CORE_ID, VCORE_SLAVE_ADDR);
	if(volt < 0)
		return 0;

	volt *= 1000;
	dbg_printf(PRN_RES, "vcore:%dmV\n", volt);

#ifdef MP886X_DBG
	mp886x_vcore_set_verify();
#endif
	return volt;
}

static int i2c_set_vcore_volt(int from, int to)
{
	UNUSED(from);

	return mp8869_set_vol(VOUT_CORE_ID, VCORE_SLAVE_ADDR, to);
}

static int i2c_ops_init(void)
{
	int ret = S_OK;

	ret = mp8867_init(VOUT_CPU_ID, VCPU_SLAVE_ADDR);
	if (ret != S_OK)
		dbg_printf(PRN_ERR, "OPS init fial: %d %s\n", __LINE__, __func__);

	ret = mp8869_init(VOUT_CORE_ID, VCORE_SLAVE_ADDR);
	if (ret != S_OK)
		dbg_printf(PRN_ERR, "OPS init fial: %d %s\n", __LINE__, __func__);

	return ret;
}

#ifdef MP886X_DBG
static void mp886x_vcpu_set_verify(void)
{
	int volt = mp886x_get_volt(VOUT_CPU_ID, VCPU_SLAVE_ADDR);
	dbg_printf(PRN_NONE, "vcpu:%dmV\n", volt * 1000);
	dbg_printf(PRN_NONE, "set voltage to 0.82V\n");
	i2c_set_vcpu_volt(800000, 820000);
	volt = mp886x_get_volt(VOUT_CPU_ID, VCPU_SLAVE_ADDR);
	dbg_printf(PRN_NONE, "TO 0.82V:vcpu:%d\n", volt * 1000);
	i2c_set_vcpu_volt(820000, 800000);
	volt = mp886x_get_volt(VOUT_CPU_ID, VCPU_SLAVE_ADDR);
	dbg_printf(PRN_NONE, "BACK TO 0.8V,vcpu:%d\n", volt * 1000);
}
static void mp886x_vcore_set_verify(void)
{
	int volt = mp886x_get_volt(VOUT_CORE_ID, VCORE_SLAVE_ADDR);
	dbg_printf(PRN_NONE, "vcore:%dmV\n", volt * 1000);
	dbg_printf(PRN_NONE, "set voltage to 0.85V\n");
	i2c_set_vcore_volt(800000, 850000);
	volt = mp886x_get_volt(VOUT_CORE_ID, VCORE_SLAVE_ADDR);
	dbg_printf(PRN_NONE, "TO 0.85V:vcore:%d\n", volt * 1000);
	i2c_set_vcore_volt(850000, 800000);
	volt = mp886x_get_volt(VOUT_CORE_ID, VCORE_SLAVE_ADDR);
	dbg_printf(PRN_NONE, "BACK TO 0.8V,vcore:%d\n", volt * 1000);
}
#endif

const dvfs_ops_t mp886x_ops = {
	.get_vcpu_volt = i2c_get_cpu_volt,
	.get_vcore_volt = i2c_get_core_volt,
	.set_vcpu_volt = i2c_set_vcpu_volt,
	.set_vcore_volt = i2c_set_vcore_volt,
	.init = i2c_ops_init
};

