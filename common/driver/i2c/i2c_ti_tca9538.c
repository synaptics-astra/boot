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

//#include "memmap.h"
//#include "SysMgr.h"
//#include "apbRegBase.h"
//#include "pinmux.h"

#define REG_INPUT_PORT 		0x00
#define REG_OUTPUT_PORT 	0x01
#define REG_POLARITY		0x02
#define REG_CONFIG			0x03

static int master_id = -1;
static int is_init = -1;

extern int i2c_master_init(int id, int speed, int b_10bit_addr);
extern int i2c_master_write_and_read(int id, int target_addr, unsigned char* send_buff, int send_len, unsigned char* recv_buff, int recv_len);

static void i2c_tca9538_init(int id)
{
	if((is_init > 0) && (id == master_id))
		return;

	i2c_master_init(id, 100, 0);
	master_id = id;
	is_init = 1;
}

static int i2c_tca9538_set_reg(int id, int slave_addr, unsigned char reg, unsigned int mask, unsigned char value)
{
	int ret = -1;
	unsigned char regvalue = 0;
	unsigned char wr[2];

	i2c_tca9538_init(id);

	wr[0] = reg;

	//read reg value first
	ret = i2c_master_write_and_read(id, slave_addr, wr, 1, &regvalue, 1);
	if(ret != 0)
		return ret;

	wr[0] = reg;
	wr[1] = regvalue & (~(mask));

	wr[1] |= value;

	ret = i2c_master_write_and_read(id, slave_addr, wr,  2, &regvalue, 0);

	return ret;
}

static int i2c_tca9538_get_reg(int id, int slave_addr, unsigned char reg, unsigned int mask, unsigned char *value)
{
	int ret = -1;
	unsigned char wr[2];

	i2c_tca9538_init(id);

	wr[0] = reg;

	//read reg value
	ret = i2c_master_write_and_read(id, slave_addr, wr, 1, value, 1);

	return ret;
}

int i2c_tca9538_set_iodir(int id, int slave_addr, unsigned int mask, unsigned char value)
{
	// 03H: io direction, 0: input, 1:output. default input.
	return i2c_tca9538_set_reg(id, slave_addr, REG_CONFIG, mask, value);
}

int i2c_tca9538_set_output_state(int id, int slave_addr, unsigned int mask, unsigned char value)
{
	// 05H: output state, 0: low, 1: high. default low.
	return i2c_tca9538_set_reg(id, slave_addr, REG_OUTPUT_PORT, mask, value);
}

int i2c_tca9538_get_input_state(int id, int slave_addr, unsigned int mask, unsigned char *value)
{
	// 05H: output state, 0: low, 1: high. default low.
	return i2c_tca9538_get_reg(id, slave_addr, REG_INPUT_PORT, mask, value);
}

int i2c_tca9538_set_input_polarity(int id, int slave_addr, unsigned int mask, unsigned char value)
{
	// 07H: output high Z. default high Z.
	return i2c_tca9538_set_reg(id, slave_addr, REG_POLARITY, mask, value);
}

int i2c_tca9538_gpios_write(int id, int slave_addr, unsigned char bit, unsigned char value)
{
	int ret = 0;
	ret = i2c_tca9538_set_iodir(id, slave_addr, (0x01 << bit), ~(0x01 << bit));
	if(ret)
		return -1;

	if (value == 0)
		ret = i2c_tca9538_set_output_state(id, slave_addr, (0x01 << bit), ~(0x01 << bit));
	else
		ret = i2c_tca9538_set_output_state(id, slave_addr, (0x01 << bit), (0x01 << bit));

	return ret;
}

int i2c_tca9538_gpios_read(int id, int slave_addr, unsigned char bit, unsigned char *value)
{
	int ret = 0;
	unsigned char regValue;

	ret = i2c_tca9538_set_iodir(id, slave_addr, (0x01 << bit), (0x01 << bit));
	if(ret)
		return -1;

	ret = i2c_tca9538_get_input_state(id, slave_addr, (0x01 << bit), &regValue);
	*value = (regValue >> bit);

	return ret;
}

