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
#include "io.h"
#include "apb_perf_base.h"
#include "galois_speed.h"
#include "i2c_driver.h"
#include "debug.h"

//////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////I2C  API///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

// number of I2C masters in system
#define I2C_MASTER_NUM	I2C_MAX

// I2C master system clock
// minimum sysmte clock are required
// Standard Mode: 	2 MHz
// Fast Mode:		10 MHz
// High-Speed Mode:	100 MHz
#define I2C_SM_CLOCK	25

// I2C master TX/RX fifo size
#define I2C_RX_FIFO_SIZE        64
#define I2C_TX_FIFO_SIZE	64

// register definitions of SPI master
#define I2C_REG_CON		0x00
#define I2C_REG_TAR		0x04
#define I2C_REG_SAR		0x08
#define I2C_REG_HS_MADDR	0x0C
#define I2C_REG_DATA_CMD	0x10
#define I2C_REG_SS_SCL_HCNT	0x14
#define I2C_REG_SS_SCL_LCNT	0x18
#define I2C_REG_FS_SCL_HCNT	0x1C
#define I2C_REG_FS_SCL_LCNT	0x20
#define I2C_REG_HS_SCL_HCNT	0x24
#define I2C_REG_HS_SCL_LCNT	0x28
#define I2C_REG_INTR_STAT	0x2C
#define I2C_REG_INTR_MASK	0x30
#define I2C_REG_RINTR_STAT	0x34
#define I2C_REG_RX_TL		0x38
#define I2C_REG_TX_TL		0x3C
#define I2C_REG_CLR_INTR	0x40
#define I2C_REG_CLR_RX_UNDER	0x44
#define I2C_REG_CLR_RX_OVER	0x48
#define I2C_REG_CLR_TX_OVER	0x4C
#define I2C_REG_CLR_RD_REQ	0x50
#define I2C_REG_CLR_TX_ABRT	0x54
#define I2C_REG_CLR_RX_DONE	0x58
#define I2C_REG_CLR_ACTIVITY	0x5C
#define I2C_REG_CLR_STOP_DET	0x60
#define I2C_REG_CLR_START_DET	0x64
#define I2C_REG_CLR_GEN_CALL	0x68
#define I2C_REG_ENABLE		0x6C
#define I2C_REG_STATUS		0x70
#define I2C_REG_TXFLR		0x74
#define I2C_REG_RXFLR		0x78
#define I2C_REG_TX_ABRT_STAT	0x80
#define I2C_REG_DMA_CR		0x88
#define I2C_REG_DMA_TDLR	0x8C
#define I2C_REG_DMA_RDLR	0x90
#define I2C_REG_COMP_PARAM	0xF4
#define I2C_REG_COMP_VERSION	0xF8
#define I2C_REG_COMP_TYPE	0xFC // reset value: 0x44570140

// Macros of I2C_REG_CON bit offset
#define CON_SLAVE_DISABLE	6
#define CON_RESTART_EN		5
#define CON_10BITADDR		4
#define CON_SPEED		1
#define CON_MASTER_ENABLE	0

// Macros of I2C_REG_TAR bit offset
#define TAR_10BITADDR	12
#define TAR_SPECIAL	11
#define TAR_GC_OR_START	10

// Macros of I2C_REG_INTR_* bit definition
#define INTR_GEN_CALL	0x800
#define INTR_START_DET	0x400
#define INTR_STOP_DET	0x200
#define INTR_ACTIVITY	0x100
#define INTR_RX_DONE	0x080
#define INTR_TX_ABRT	0x040
#define INTR_RD_REQ		0x020
#define INTR_TX_EMPTY	0x010
#define INTR_TX_OVER	0x008
#define INTR_RX_FULL	0x004
#define INTR_RX_OVER	0x002
#define INTR_RX_UNDER	0x001

// Macros of I2C_REG_ENABLE bit definition
#define EN_ENABLE	0x01

// Macros of read-command
#define READ_CMD	0x100

// Macros to interrupt mask flag
#define MASK	0
#define UNMASK	1

// Macros of read/write I2C master registers
#define I2C_RegRead(a, pv)	REG_READ32(base_addr+(a), pv) /*(*(volatile int *)(base_addr+(a)))*/
//#define I2C_RegRead(a, pv)	(*(pv) = readl(base_addr+(unsigned)(a)))
#define I2C_RegWrite(a, v)	REG_WRITE32(base_addr+(a), v) /**(volatile int *)(base_addr+(a)) = (v)*/
//#define I2C_RegWrite(a, v)	(writel((v), base_addr+(unsigned)(a)))

// I2C master's transfer buffer, shared by TX and RX
static int i2c_master_buffer[I2C_MASTER_NUM][256+4];
// number of command which have been written to TX fifo so far,
// this will equal bytes_of_write_cmd + bytes_of_read_cmd at the end of transaction
static int bytes_written_to_txfifo[I2C_MASTER_NUM];
// number of bytes read out from RX fifo so far,
static int bytes_read_from_rxfifo[I2C_MASTER_NUM];
// number of write-command which need to be written to TX fifo
static int bytes_of_write_cmd[I2C_MASTER_NUM];
// number of read-command which need to be written to TX fifo
static int bytes_of_read_cmd[I2C_MASTER_NUM];
// variable indicating transaction finishing or not
static volatile int transaction_done[I2C_MASTER_NUM];

// I2C master's base-address vector
static const unsigned int I2C_MASTER_BASEADDR[I2C_MASTER_NUM] = {APB_I2C_INST0_BASE, APB_I2C_INST1_BASE, SM_APB_I2C0_BASE, SM_APB_I2C1_BASE};

const char i2c_master_name[I2C_MASTER_NUM][8]=
{
	"SOC TW0",
	"SOC TW1",
	"SM TW0",
	"SM TW1"
};

/*****************************************************
 * set I2C master TX FIFO underflow threshold
 * thrld: 0 - 255
 *
 *****************************************************/
static void set_txfifo_threshold(int master_id, int thrld)
{
	unsigned int base_addr;

	if (thrld<0 || thrld>255) return;

	base_addr = I2C_MASTER_BASEADDR[master_id];

	I2C_RegWrite(I2C_REG_TX_TL, thrld);

	return;
}

/*****************************************************
 * set I2C master RX FIFO overflow threshold
 * thrld: 1 - 256
 *
 *****************************************************/
static void set_rxfifo_threshold(int master_id, int thrld)
{
	unsigned int base_addr;

	if (thrld<1 || thrld>256) return;

	base_addr = I2C_MASTER_BASEADDR[master_id];

	I2C_RegWrite(I2C_REG_RX_TL, thrld-1);

	return;
}

/*********************************************************
 * FUNCTION: init I2C master: set default bus speed
 * PARAMS: master_id - id of I2C master to init
 * RETURN: I2C_OK - succeed
 *         I2C_ERROR - invalid parameter
 ********************************************************/
int diag_i2c_master_init(int master_id)
{
	unsigned int base_addr;
	int i;

	if ((master_id < 0) || (master_id > I2C_MASTER_NUM-1))
	{
		dbg_printf(PRN_DBG,"I2C wrong master id\n");
		return (I2C_ERROR);
	}

	base_addr = I2C_MASTER_BASEADDR[master_id];

	// disable I2C module first
	I2C_RegWrite(I2C_REG_ENABLE, ~EN_ENABLE);

	// set TX/RX fifo default threshold
	set_txfifo_threshold(master_id, 0);
	set_rxfifo_threshold(master_id, I2C_RX_FIFO_SIZE/2);

        // disable all I2C interrupts.
	I2C_RegWrite(I2C_REG_INTR_MASK, 0);

	// disable slave mode, enable restart function, enable master mode
	I2C_RegRead(I2C_REG_CON, &i);
	i |= (1<<CON_SLAVE_DISABLE)|(1<<CON_RESTART_EN)|(1<<CON_MASTER_ENABLE);
	I2C_RegWrite(I2C_REG_CON, i);

	// set default I2C master speed: 100KHz fast-speed mode
	diag_i2c_master_set_speed(master_id, I2C_STANDARD_SPEED_MODE, I2C_STANDARD_SPEED_100K);
	//diag_i2c_master_set_speed(master_id, I2C_FAST_SPEED_MODE, I2C_STANDARD_SPEED_400K);

	I2C_RegRead(I2C_REG_INTR_MASK, &i);

	return (I2C_OK);
}

static int addr_type = I2C_7BIT_SLAVE_ADDR;

int diag_i2c_master_set_addr_type(int type)
{
	addr_type = type;
	return 0;
}

/*********************************************************************
 * FUNCTION: set I2C master bus speed
 * PARAMS: master_id - id of I2C master to config
 *         mode - STANDARD(1) or FAST(2)
 *         speed - in unit of KHz
 * RETURN: I2C_OK - succeed
 * 	   	   I2C_ERROR - I2C module is enabled or speed not support
 ********************************************************************/
int diag_i2c_master_set_speed(int master_id, int mode, int speed)
{
	unsigned int base_addr;
	unsigned int input_clock;
	int i;

	if ((master_id < 0) || (master_id > I2C_MASTER_NUM-1))
	{
		dbg_printf(PRN_DBG,"I2C wrong master id\n");
		return (I2C_ERROR);
	}

	base_addr = I2C_MASTER_BASEADDR[master_id];

	// check whether I2C module is disabled
	I2C_RegRead(I2C_REG_ENABLE, &i);
	if (i & EN_ENABLE)
	{
		dbg_printf(PRN_DBG,"I2C is busy\n");
		return (I2C_ERROR);
	}

	if (master_id == 0 || master_id == 1)
#ifdef BCM_EN
		//FIX ME: should get the frequency of perifSysClk
		// the clock setting still has some problem
		input_clock = 200;
#else
		input_clock = GaloisGetFrequency(SOC_FREQ_CFG);
#endif
	else
		input_clock = I2C_SM_CLOCK;

	switch (mode)
	{
		case I2C_STANDARD_SPEED_MODE: // standard-speed
			I2C_RegRead(I2C_REG_CON, &i);
			i &= ~(3<<CON_SPEED);
			i |= (1<<CON_SPEED);
			I2C_RegWrite(I2C_REG_CON, i); /* set master to operate in standard-speed mode */
			if(speed==I2C_STANDARD_SPEED_100K)
			{
				//use the default standard 100KHz speed
				I2C_RegWrite(I2C_REG_SS_SCL_HCNT, 4000*input_clock/1000); /* 4000-ns minimum HIGH-period*/
				I2C_RegWrite(I2C_REG_SS_SCL_LCNT, 4700*input_clock/1000); /* 4700-ns minimum LOW-period*/
			}
			else
			{
				// calculate high/low period of SCL in 1/3(high)-2/3(low) criteria.
				i = 1000000/(speed*3);
				I2C_RegWrite(I2C_REG_SS_SCL_HCNT, i*input_clock/1000);
				i <<= 1;
				I2C_RegWrite(I2C_REG_SS_SCL_LCNT, i*input_clock/1000);
			}
			break;
		case I2C_FAST_SPEED_MODE: // fast-speed
			I2C_RegRead(I2C_REG_CON, &i);
			i &= ~(3<<CON_SPEED);
			i |= (2<<CON_SPEED);
			I2C_RegWrite(I2C_REG_CON, i); /* set master to operate in fast-speed mode */
			if(speed==I2C_STANDARD_SPEED_400K)
			{
				//use the default fast 400KHz speed
				I2C_RegWrite(I2C_REG_FS_SCL_HCNT, 600*input_clock/1000); /* 600-ns minimum HIGH-period*/
				I2C_RegWrite(I2C_REG_FS_SCL_LCNT, 1300*input_clock/1000); /* 1300-ns minimum LOW-period*/
			}
			else
			{
								// calculate high/low period of SCL in 1/3(high)-2/3(low) criteria.
				i = 1000000/(speed*3);
				I2C_RegWrite(I2C_REG_FS_SCL_HCNT, i*input_clock/1000);
				i <<= 1;
				I2C_RegWrite(I2C_REG_FS_SCL_LCNT, i*input_clock/1000);
			}
			break;
		default:
			dbg_printf(PRN_DBG,"I2C unsupported speed mode\n");
			return (I2C_ERROR);
	}

	return (I2C_OK);
}

/***********************************************************************
 * FUNCTION: read bytes from slave device
 * PARAMS: master_id - id of I2C master to operate
 *         slave_addr - address of slave device
 *         pwbuf - pointer of write buffer
 *         wnum - number of bytes to write
 *         prbuf - pointer of read buffer
 *         rnum - number of bytes to read
 * RETURN: I2C_OK - succeed
 * 	   	   I2C_ERROR - I2C module is enabled, or read failed
 * NOTE: maximum write bytes is 260, maximum read bytes is 256 in a single
 *       transaction
 ***********************************************************************/
int diag_i2c_master_writeread_bytes(int master_id, int slave_addr, unsigned char *pwbuf, int wnum, unsigned char *prbuf, int rnum)
{
	unsigned int base_addr;
	int i, j;
	int timeout=1000000;

	if ((master_id < 0) || (master_id > I2C_MASTER_NUM-1))
	{
		dbg_printf(PRN_DBG,"I2C wrong master id\n");
		return (I2C_ERROR);
	}

	if (wnum>260 || rnum>256)
	{
		dbg_printf(PRN_DBG,"I2C number of bytes too big\n");
		return (I2C_ERROR);
	}

	if (addr_type!=I2C_7BIT_SLAVE_ADDR && addr_type!=I2C_10BIT_SLAVE_ADDR)
	{
		dbg_printf(PRN_DBG,"I2C wrong address type\n");
		return (I2C_ERROR);
	}

	base_addr = I2C_MASTER_BASEADDR[master_id];

	// check whether I2C module is disabled
	I2C_RegRead(I2C_REG_ENABLE, &i);
	if (i & EN_ENABLE)
	{
		dbg_printf(PRN_DBG,"I2C is busy\n");
		return (I2C_ERROR);
	}

	I2C_RegRead(I2C_REG_CON, &i);

	if (addr_type==I2C_7BIT_SLAVE_ADDR){ // 7-bit slave address

		// set 7-bit target address, normal transfer mode
		I2C_RegRead(I2C_REG_TAR, &i);
		i &= ~((1<<TAR_10BITADDR) | (1<<TAR_SPECIAL) | 0x3ff);
		i |= slave_addr;
		I2C_RegWrite(I2C_REG_TAR, i);

		// for SM_TW1, the dynamic address change is 0,
		// so we have to set 7-bit address type here
		I2C_RegRead(I2C_REG_CON, &i);
		i &= ~(1<<CON_10BITADDR);
		I2C_RegWrite(I2C_REG_CON, i);

	} else { // 10-bit slave address

		// set 10-bit target address, normal transfer mode
		I2C_RegRead(I2C_REG_TAR, &i);
		i &= ~((1<<TAR_SPECIAL) | 0x3ff);
		i |= ((1<<TAR_10BITADDR) | slave_addr);
		I2C_RegWrite(I2C_REG_TAR, i);

		// for SM_TW1, the dynamic address change is 0,
		// so we have to set 7-bit address type here
		I2C_RegRead(I2C_REG_CON, &i);
		i |= (1<<CON_10BITADDR);
		I2C_RegWrite(I2C_REG_CON, i);
	}

	// initiate transaction status flag
	transaction_done[master_id] = 0;

	// set TX fifo threshold to make sure TXE interrupt will be triggered
	// as soon as we unmask the TXE interupt, we will start transaction by writing CMD_DATA register then.
	set_txfifo_threshold(master_id, I2C_TX_FIFO_SIZE/2);

	// clear TX_ABRT_SOURCE by reading
	I2C_RegRead(I2C_REG_CLR_INTR, &i);

	if ((pwbuf && wnum>0) && (prbuf && rnum>0)){ // write & read transaction
		// set RX fifo threshold
		if (rnum<=I2C_RX_FIFO_SIZE)
			set_rxfifo_threshold(master_id, rnum);
		else
			set_rxfifo_threshold(master_id, I2C_RX_FIFO_SIZE/2);

		// initiate number of write and read commands which are going to be written to TX fifo
		bytes_written_to_txfifo[master_id] = 0;
		bytes_read_from_rxfifo[master_id] = 0;
		bytes_of_write_cmd[master_id] = wnum;
		bytes_of_read_cmd[master_id] = rnum;

		// write-command need to be copy to buffer first,
		// read-command don't need because it can be generated dynamically
		for (i=0; i<wnum; i++)
			i2c_master_buffer[master_id][i] = (int)pwbuf[i];

		// enable I2C master
		I2C_RegWrite(I2C_REG_ENABLE, EN_ENABLE);

		// issue write commands to TX fifo
		for (i=0; i<wnum; i++)
		{
			do{
				I2C_RegRead(I2C_REG_STATUS, &j);
			}while (!(j&0x02));	/* TX fifo is full */

			I2C_RegWrite(I2C_REG_DATA_CMD, i2c_master_buffer[master_id][i]);
		}
		// issue read commands to TX fifo and receive data from RX fifo
		i = 0;
		while (bytes_read_from_rxfifo[master_id]<rnum)
		{
			// if there are received data in RX fifo, read out to buffer
			I2C_RegRead(I2C_REG_STATUS, &j);
			if (j&0x08)	/* RX fifo is not empty */
				I2C_RegRead(I2C_REG_DATA_CMD, &(i2c_master_buffer[master_id][bytes_read_from_rxfifo[master_id]++]));
			// if TX fifo not full, issue remaining read commands to TX fifo
			if (i<rnum){
				I2C_RegRead(I2C_REG_STATUS, &j);
				if (j&0x02){	/* TX fifo is not full */
					I2C_RegWrite(I2C_REG_DATA_CMD, READ_CMD);
					i ++;
				}
			}
			if(!(--timeout)) break;
		}
		transaction_done[master_id] = 1;

	} else if (pwbuf && wnum>0) { // write-only transaction
		// initiate number of write commands which are going to be written to TX fifo
		bytes_written_to_txfifo[master_id] = 0;
		bytes_read_from_rxfifo[master_id] = 0;
		bytes_of_write_cmd[master_id] = wnum;
		bytes_of_read_cmd[master_id] = 0;

		// write-command need to be copy to buffer first,
		for (i=0; i<wnum; i++)
			i2c_master_buffer[master_id][i] = (int)pwbuf[i];

		// enable I2C master
		I2C_RegWrite(I2C_REG_ENABLE, EN_ENABLE);

		// issue write commands to TX fifo
		for (i=0; i<wnum; i++)
		{

			timeout=10000;
			I2C_RegRead(I2C_REG_STATUS, &j);
			while (!(j&0x02)) /* TX fifo is full, so wait, but if it take too long, timeout*/
			{
				if(!(timeout--))
				{
					// disable I2C master
					I2C_RegWrite(I2C_REG_ENABLE, ~EN_ENABLE);
					/* somehow error happen !!! */
					dbg_printf(PRN_DBG,"I2C write failed\n");
					return (I2C_ERROR);
				}
				I2C_RegRead(I2C_REG_STATUS, &j);
			}

			I2C_RegWrite(I2C_REG_DATA_CMD, i2c_master_buffer[master_id][i]);

		}

		//add some delay, otherwise, the next error condition will get hit on fast cpu
		//fifo not empty and master is idle
		{
			volatile int delay=50000;
			while(delay--);
		}

		I2C_RegRead(I2C_REG_STATUS, &j);
		while (!(j&0x04)){ /* TX fifo is not empty */
			if(!(j&0x01)) /* but I2C master is idle */
			{
				// disable I2C master
				I2C_RegWrite(I2C_REG_ENABLE, ~EN_ENABLE);
				/* somehow error happen !!! */
				dbg_printf(PRN_DBG,"I2C write failed\n");
				return (I2C_ERROR);
			}
			I2C_RegRead(I2C_REG_STATUS, &j);
		}

		do{
			I2C_RegRead(I2C_REG_STATUS, &i);
		}while (i&0x01); /* wait until transfer finish */

		//for write only transaction, need to check TX_ABRT_SOURCE to know if tx failed
		I2C_RegRead(I2C_REG_TX_ABRT_STAT, &i);
		if(i)
		{
			// disable I2C master
			I2C_RegWrite(I2C_REG_ENABLE, ~EN_ENABLE);
			/* somehow error happen !!! */
			dbg_printf(PRN_DBG,"I2C tx aborted! I2C_REG_TX_ABRT_STAT = 0x%x\n", i);
			return (I2C_ERROR);
		}


		transaction_done[master_id] = 1;

	} else if (prbuf && rnum>0) { // read-only transaction
		// set RX fifo threshold
		if (rnum<=I2C_RX_FIFO_SIZE)
			set_rxfifo_threshold(master_id, rnum);
		else
			set_rxfifo_threshold(master_id, I2C_RX_FIFO_SIZE/2);
		// initiate number of read commands which are goding to be written to TX fifo
		bytes_written_to_txfifo[master_id] = 0;
		bytes_read_from_rxfifo[master_id] = 0;
		bytes_of_write_cmd[master_id] = 0;
		bytes_of_read_cmd[master_id] = rnum;

		// enable I2C master
		I2C_RegWrite(I2C_REG_ENABLE, EN_ENABLE);

		// issue read commands to TX fifo and receive data from RX fifo
		i = 0;
		while (bytes_read_from_rxfifo[master_id]<rnum)
		{
			// if there are received data in RX fifo, read out to buffer
			I2C_RegRead(I2C_REG_STATUS, &j);
			if (j&0x08)	/* RX fifo is not empty */
				I2C_RegRead(I2C_REG_DATA_CMD, &(i2c_master_buffer[master_id][bytes_read_from_rxfifo[master_id]++]));
			// if TX fifo not full, issue remaining read commands to TX fifo
			if (i<rnum){
				I2C_RegRead(I2C_REG_STATUS, &j);
				if (j&0x02){	/* TX fifo is not full */
					I2C_RegWrite(I2C_REG_DATA_CMD, READ_CMD);
					i ++;
				}
			}
			if(!(--timeout)) break;
		}
		transaction_done[master_id] = 1;
	}

	// if it is a write only transaction, wait until transfer finish
	do{
		I2C_RegRead(I2C_REG_STATUS, &i);
	}while (i&0x01); /* wait until transfer finish */

	// copy received data from buffer to prbuf if there is
	if (bytes_of_read_cmd[master_id] > 0){
		for (i=0; i<bytes_of_read_cmd[master_id]; i++)
			prbuf[i] = (unsigned char)i2c_master_buffer[master_id][i];
	}

	// disable I2C master
	I2C_RegWrite(I2C_REG_ENABLE, ~EN_ENABLE);

	if(!timeout)
	{
		dbg_printf(PRN_DBG,"I2C Timed out\n");
		return (I2C_ERROR);
	}

	return (I2C_OK);
}


const char* g_aApbI2C_AbortSrc[20] = {0};

int diag_i2c_check_error(int master_id, unsigned int *tx_abort_source)
{
	unsigned int base_addr;
	unsigned int read, i;

	if ((master_id < 0) || (master_id > I2C_MASTER_NUM-1))
	{
		dbg_printf(PRN_DBG,"I2C wrong master id\n");
		return (I2C_ERROR);
	}

	base_addr = I2C_MASTER_BASEADDR[master_id];

	I2C_RegRead(I2C_REG_TX_ABRT_STAT, &read);;	// check Tx Aborad Source register
	//dbg_printf(  "Tx Abort Source 0x%04X\n", read);
	*tx_abort_source = read;
	for (i = 0; i < 16; i++)
	{
		if (read & 1)
			dbg_printf(PRN_DBG,"%s\n", g_aApbI2C_AbortSrc[i]);
		read /= 2;
		if ((read & 0xFFFF) == 0)
			break;
	}

	return (I2C_OK);
}

#if defined(BERLIN_SOC_BG2CD)
#define I2C_MAX_BUF_SIZE 128

static unsigned char rbuf[I2C_MAX_BUF_SIZE];

static const unsigned char* wbuf= (unsigned char*)0xF8182F00;

void diag_i2c_gpio_master_init_wrapper(unsigned int data1)
{
	unsigned int iResult;

	if (data1 >= I2C_MAX) //GPIO simulated I2C
	{
		dbg_printf(PRN_DBG,"I2C GPIO master is not enabled\n");
		/*iResult = diag_i2c_gpio_master_init(data1 - I2C_MAX);
		if (iResult)
			dbg_printf(PRN_RES, " I2C GPIO master %d INIT Failed\n", (data1 - I2C_MAX));
		else
			dbg_printf(PRN_RES, " I2C GPIO master %d INIT OK\n", (data1 - I2C_MAX));*/
	}
	else
	{
		iResult = diag_i2c_master_init(data1);
		if (iResult)
			dbg_printf(PRN_DBG," I2C master %s INIT Failed\n", i2c_master_name[data1]);
		else
			dbg_printf(PRN_DBG," I2C master %s INIT OK\n",i2c_master_name[data1]);
	}
}

void diag_i2c_master_read_bytes(unsigned int data1, unsigned int data2, unsigned int data3)
{
	unsigned int iResult,data4;

	data2 = 0;			// slave address
	data3 = 1;			// number of bytes
	/* Parse command arguments. */

	if (data1 >= I2C_MAX) //GPIO simulated I2C
		//iResult = diag_i2c_gpio_master_writeread_bytes(data1-I2C_MAX, data2, I2C_7BIT_SLAVE_ADDR, 0, 0, rbuf, data3);
		dbg_printf(PRN_DBG,"I2C GPIO master is not enabled\n");
	else
		iResult = diag_i2c_master_writeread_bytes(data1, data2, 0, 0, rbuf, data3);
	if (iResult)
	{
		if (data1 >= I2C_MAX)
		//	iResult |= diag_i2c_gpio_check_error(data1 - I2C_MAX);
			dbg_printf(PRN_DBG,"I2C GPIO master is not enabled\n");
		else
		{
			iResult |= diag_i2c_check_error(data1, &data4);
			dbg_printf(PRN_DBG," error code 0x%08x\n", data4);
		}
		dbg_printf(PRN_DBG," Failed\n");
	}
	else
	{
		int i;
		dbg_printf(PRN_DBG,"Read Data: ");
		for (i = 0; i < data3; i++)
			dbg_printf(PRN_DBG, "0x%X ", rbuf[i]);
		dbg_printf(PRN_DBG,"\n Passed\n");
	}
}

void diag_i2c_master_write_bytes(unsigned int data1, unsigned int data2, unsigned int data3)
{
	unsigned int iResult,data4;
	//clear the wbuf first, if number of bytes is
	//less than the cmd line supplied bytes, the extra bytes will still
	//be copied int wbuf but not sent to I2C. If number of bytes is
	//larger than the cmd line supplied bytes, extra zeros will be sent
/*	memset(wbuf, 0, I2C_MAX_BUF_SIZE);
	data4 = 0;	//counts how many dwords are collected
	while (1)
	{
		skipSpace( &pCmd );
		if ( *pCmd != '\0' )
			pCmd = getHexToken( pCmd, (unsigned int *)(wbuf+data4*4));
		else
			break;
		data4++;
	}*/
	if (data1 >= I2C_MAX)
		//iResult = diag_i2c_gpio_master_writeread_bytes(data1-I2C_MAX, data2, I2C_7BIT_SLAVE_ADDR, wbuf, data3, 0, 0);
		dbg_printf(PRN_DBG,"I2C GPIO master is not enabled\n");
	else
		iResult = diag_i2c_master_writeread_bytes(data1, data2, (unsigned char *)wbuf, data3, 0, 0);
	if (iResult)
	{
		if (data1 >= I2C_MAX)
		{
			//iResult |= diag_i2c_gpio_check_error(data1 - I2C_MAX);
			dbg_printf(PRN_DBG,"I2C GPIO master is not enabled\n");
		}
		else
		{
			iResult |= diag_i2c_check_error(data1, &data4);
			dbg_printf(PRN_DBG," error code 0x%08x\n", data4);
		}
		dbg_printf(PRN_DBG," Failed\n");
	}
	else
	{
		int i;
		dbg_printf(PRN_DBG,"Write Data: ");
		for (i = 0; i < data3; i++)
		{
			dbg_printf(PRN_DBG,"0x%x ", wbuf[i]);
		}
		dbg_printf(PRN_DBG,"\n Passed\n");
	}
}

void diag_i2c_writeread_bytes(data1, data2, data3, data4)
{
	unsigned int iResult,data5 = 0;	//counts how many dwords are collected
	/*while (1)
	{
		skipSpace( &pCmd );
		if ( *pCmd != '\0' )
			pCmd = getHexToken( pCmd, (unsigned int *)(wbuf+data5*4));
		else
			break;
		data5++;
	}*/
	if (data1 >= I2C_MAX)
	//iResult = diag_i2c_gpio_master_writeread_bytes(data1-I2C_MAX, data2, I2C_7BIT_SLAVE_ADDR, wbuf, data3, rbuf, data4);
		dbg_printf(PRN_DBG,"I2C GPIO master is not enabled\n");
	else
		iResult = diag_i2c_master_writeread_bytes(data1, data2, (unsigned char *)wbuf, data3, (unsigned char *)rbuf, data4);
	if (iResult)
	{
		if (data1 >= I2C_MAX)
			//iResult |= diag_i2c_gpio_check_error(data1 - I2C_MAX);
			dbg_printf(PRN_DBG,"I2C GPIO master is not enabled\n");
		else
		{
			iResult |= diag_i2c_check_error(data1, &data5);
			dbg_printf(PRN_DBG," error code 0x%08x\n", data5);
		}
		dbg_printf(PRN_DBG," Failed\n");
	}
	else
	{
		int i;
		dbg_printf(PRN_DBG,"Write Data: ");
		for (i = 0; i < data3; i++)
			dbg_printf( PRN_DBG,"0x%X ", wbuf[i]);
		dbg_printf(PRN_DBG,"\nRead Data: ");
		for(i = 0; i < data4; i++)
			dbg_printf(PRN_DBG, "0x%X ", rbuf[i]);
		dbg_printf(PRN_DBG,"\n Passed\n");
	}
}
#endif
