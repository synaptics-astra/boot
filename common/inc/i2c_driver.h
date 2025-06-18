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
#ifndef _I2C_API_H_
#define _I2C_API_H_

// error code definitions
#ifndef I2C_OK
#define I2C_OK			0
#endif
#ifndef I2C_ERROR
#define I2C_ERROR		1
#endif
#ifndef I2C_EBADPARAM
#define I2C_EBADPARAM		2
#endif
#ifndef I2C_EBUSRW
#define I2C_EBUSRW		3
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)>(b))?(b):(a))
#endif

enum I2C_INSTANCES
{
	SOC_I2C0,
	SOC_I2C1,
	SM_I2C0,
	SM_I2C1,
	I2C_MAX
};

// I2C standard bus speed
enum
{
	I2C_STANDARD_SPEED_100K = 100,
	I2C_STANDARD_SPEED_400K = 400,
};

// I2C speed mode registers, note our IP doesn't support high speed mode
enum
{
	I2C_STANDARD_SPEED_MODE = 1, // 100K is the default
	I2C_FAST_SPEED_MODE   = 2    // 400K is the default

};

// I2C slave address type
enum
{
	I2C_7BIT_SLAVE_ADDR	= 0,
	I2C_10BIT_SLAVE_ADDR = 1
};

// structure for I2C SDA/SCL timing information, used with GPIO I2C operation
typedef struct i2cdev_bus_timing_T{
	int start_su;	// START setup time, in unit of nano-second
	int start_hd;	// START hold time
	int stop_su;	// STOP setup time
	int stop_hd;	// STOP hold time
	int data_su;	// data setup time
	int data_hd;	// data hold time
	int clk_low;	// clock low period
	int clk_high;	// clock high period
	int timeout;	// timeout value in unit of nano-second
}i2cdev_bus_timing_t;

typedef struct i2cdev_bus_op1_T{
	int (*init)(int);		// I2C bus initialization
	void (*sda_set)(int);		// I2C data line pull high
	void (*sda_clr)(int);		// I2C data line pull low
	int (*sda_rd)(int);		// I2C data line latch
	void (*sda_tristate)(int);	// make I2C data line tri-state
	void (*scl_set)(int);		// I2C clock line pull high
	void (*scl_clr)(int);		// I2C clock line pull low
	int (*scl_rd)(int);		// I2C clock line latch
}i2cdev_bus_op1_t;

typedef struct i2cdev_gpio_pin_T
{
	int sda; 	//which gpio pin to use as sda
	int scl;	//which gpio pin to use as scl
} i2cdev_gpio_pin_t;

#ifdef __cplusplus__
extern "C" {
#endif

/******************************************
 *  global function declarations
 *****************************************/
/*********************************************************
 * FUNCTION: init I2C master: set default bus speed
 * PARAMS: master_id - id of I2C master to init
 * RETURN: I2C_OK - succeed
 *         I2C_EBADPARAM - invalid parameter
 ********************************************************/
int diag_i2c_master_init(int master_id);

/********************************************************************
 * FUNCTION: set I2C master bus speed
 * PARAMS: master_id - id of I2C master to config
 *         mode - STANDARD(1) or FAST(2)
 *         speed - in unit of KHz
 * RETURN: I2C_OK - succeed
 *         I2C_EBUSY - I2C module is enabled
 *         I2C_EUNSUPPORT - not support
 ********************************************************************/
int diag_i2c_master_set_speed(int master_id, int mode, int speed);

/***********************************************************************
 * FUNCTION: read bytes from slave device
 * PARAMS: master_id - id of I2C master to operate
 *         slave_addr - address of slave device
 *         addr_type - address type: I2C_7BIT_SLAVE_ADDR or I2C_10BIT_SLAVE_ADDR
 *         pwbuf - pointer of write buffer
 *         wnum - number of bytes to write
 *         prbuf - pointer of read buffer
 *         rnum - number of bytes to read
 * RETURN: I2C_OK - succeed  *         I2C_EBUSY - I2C module is enabled
 *         I2C_EBUSRW - read fail
 * NOTE: maximum write bytes is 260, maximum read bytes is 256 in a single
 *       transaction
 ***********************************************************************/
int diag_i2c_master_writeread_bytes(int master_id, int slave_addr, unsigned char *pwbuf, int wnum, unsigned char *prbuf, int rnum);


int diag_i2c_gpio_master_init(int master_id);

/*********************************************************************
 * FUNCTION: set I2C GPIO master bus speed
 * PARAMS: master_id - id of I2C master to config
 *         speed - in unit of KHz
 * RETURN: I2C_OK - succeed
 * 	   	   I2C_ERROR - I2C module is enabled or speed not support
 * NOTE: in STANDARD type, only 100 and 400 KHz speed are supported
 ********************************************************************/
int diag_i2c_gpio_master_set_speed(int master_id, int speed);

int diag_i2c_master_set_addr_type(int type);

int diag_i2c_gpio_master_writeread_bytes(int master_id, int slave_addr, int addr_type, unsigned char *pwbuf, int wnum, unsigned char *prbuf, int rnum);

void diag_i2c_gpio_set_pins(int master_id, int scl_pin, int sda_pin);

// poweron usb by ethernet
int diag_i2c_poweron_usb(int master_id);

#ifdef __cplusplus__
}
#endif

#endif
