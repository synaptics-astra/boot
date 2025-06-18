/******************************************************************************
*                 2018 Synaptics Incorporated. All Rights Reserved            *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF Synaptics.                   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
* OF Synaptics OR ANY THIRD PARTY. Synaptics RESERVES THE RIGHT AT ITS SOLE   *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO Synaptics.  *
* THIS CODE IS PROVIDED "AS IS". Synaptics MAKES NO WARRANTIES, EXPRESSED,    *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
*                                                                             *
*                   WARNING FOR USING THIS DIAG CODE                          *
*   1. Please take your own responsibility if you refer DIAG source code for  *
*      any type software development.                                         *
*   2. DIAG code is mainly for internal validation purpose, and not intend to *
*      be used in production SW.                                              *
*   3. DIAG code is subject to change without notice.                         *
*******************************************************************************/
#include "sm_type.h"

#include "sm_i2c_raw.h"
#include "sm_printf.h"


#define DEVICE_ID_CTRL 0x1
#define IO_DIR 0x3
#define OUTPUT_STATE 0x5
#define OUTPUT_HIGHZ 0x7
#define INPUT_DF_STATE 0x9
#define PULL_ENABLE 0xB
#define PULL_DOWN_UP 0xD
#define INPUT_STATUS 0xF
#define INT_MASK 0x11
#define INT_STATUS 0x13

static int master_id = -1;
static int is_init = -1;

static void i2c_init(int id)
{
	if((is_init > 0) && (id == master_id))
		return;

	i2c_master_init(id, 100);
	master_id = id;
	is_init = 1;
}

void sm_fxl6408_read_deviceID(int id, int slave_addr)
{
	int ret = -1;
	unsigned int deviceID = 0;
	unsigned char wr = DEVICE_ID_CTRL;

	i2c_init(id);

	ret = i2c_master_write_and_read(id, slave_addr, &wr, 1, &deviceID, 1);

	PRT_RES("### device ID = %x, ret = %d\n", deviceID, ret);
}

static int sm_fxl6408_set_reg(int id, int slave_addr, unsigned char reg, unsigned int mask, unsigned int value)
{
	int ret = -1;
	unsigned int regvalue = 0;
	unsigned char wr[2];

	i2c_init(id);

	wr[0] = reg;

	//read regvalue first
	ret = i2c_master_write_and_read(id, slave_addr, wr, 1, &regvalue, 1);
	if(ret != 0)
		return ret;

	wr[0] = reg;
	wr[1] = regvalue & (~(mask));
	wr[1] |= (value & mask);

	ret = i2c_master_write_and_read(id, slave_addr, wr,  2, &regvalue, 0);

	return ret;
}

int sm_fxl6408_set_iodir(int id, int slave_addr, unsigned int mask, unsigned int value)
{
	// 03H: io direction, 0: input, 1:output. default input.
	return sm_fxl6408_set_reg(id, slave_addr, IO_DIR, mask, value);
}

int sm_fxl16408_set_output_state(int id, int slave_addr, unsigned int mask, unsigned int value)
{
	// 05H: output state, 0: low, 1: high. default low.
	return sm_fxl6408_set_reg(id, slave_addr, OUTPUT_STATE, mask, value);
}

int sm_fxl16408_set_output_enable(int id, int slave_addr, unsigned int mask, unsigned int value)
{
	// 07H: output high Z. default high Z.
	return sm_fxl6408_set_reg(id, slave_addr, OUTPUT_HIGHZ, mask, value);
}

int sm_fxl16408_gpios_write(int id, int slave_addr, unsigned int mask, unsigned int value)
{
	int ret = 0;
	ret = sm_fxl6408_set_iodir(id, slave_addr, mask, mask);
	if(ret)
		return -1;
	ret = sm_fxl16408_set_output_state(id, slave_addr, mask, value);
	if(ret)
		return -1;
	ret = sm_fxl16408_set_output_enable(id, slave_addr, mask, ~(mask));
	return ret;
}
