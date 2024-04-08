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
#define _EXPANDER_GPIO_C_

#include "debug.h"
#include "i2c_fxl6408.h"
#include "i2c_driver.h"


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

const int I2C_SLAVE_ADDR[] =
{
    I2C_SLAVE0_ADDR,
    I2C_SLAVE1_ADDR,
};


extern int i2c_master_init(int id, int speed, int b_10bit_addr);
extern int i2c_master_write_and_read(int id, int target_addr, unsigned char* send_buff, int send_len, unsigned char* recv_buff, int recv_len);

static void i2c_init(int id)
{
	if((is_init > 0) && (id == master_id))
		return;

	i2c_master_init(id, 100, 0);
	master_id = id;
	is_init = 1;
}

static int fxl6408_set_reg(int id, int slave_addr, unsigned char reg, unsigned int mask, unsigned int value)
{
	int ret = -1;
	unsigned char regvalue = 0;
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

static int fxl6408_set_iodir(int id, int slave_addr, unsigned int mask, unsigned int value)
{
	// 03H: io direction, 0: input, 1:output. default input.
	return fxl6408_set_reg(id, slave_addr, IO_DIR, mask, value);
}

static int fxl6408_set_output_state(int id, int slave_addr, unsigned int mask, unsigned int value)
{
	// 05H: output state, 0: low, 1: high. default low.
	return fxl6408_set_reg(id, slave_addr, OUTPUT_STATE, mask, value);
}

static int fxl6408_set_output_enable(int id, int slave_addr, unsigned int mask, unsigned int value)
{
	// 07H: output high Z. default high Z.
	return fxl6408_set_reg(id, slave_addr, OUTPUT_HIGHZ, mask, value);
}

int i2c_fxl6408_gpio_write(int id, int expander, unsigned int port, unsigned int value)
{
	int ret = 0;
	int slave_addr = I2C_SLAVE_ADDR[expander];
	unsigned int mask = 1 << port;
	unsigned int port_value = value << port;

	ret = fxl6408_set_iodir(id, slave_addr, mask, mask);
	if(ret)
		return -1;

	ret = fxl6408_set_output_state(id, slave_addr, mask, port_value);
	if(ret)
		return -1;

	ret = fxl6408_set_output_enable(id, slave_addr, mask, ~(mask));
	return ret;
}
