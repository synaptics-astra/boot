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
#include "SysMgr.h"

#include "sm_memmap.h"
#include "sm_printf.h"
#include "sm_timer.h"

const unsigned int i2c_bases[] =
{
    SM_MEMMAP_SMAPB_BASE + SM_APB_I2C_0,
    SM_MEMMAP_SMAPB_BASE + SM_APB_I2C_1,
};

const int i2c_irq_id[] =
{
    SM_IRQ_I2C_0,
    SM_IRQ_I2C_1,
};

#define SM_I2C_CLOCK                (25 * 1000)        // KHz
#define I2C_TIMEOUT                 (1000*1000)     // us
#define I2C_POLLING_INTERVAL        10              // us

#define I2C_REG_READ(base, reg)     (*(volatile unsigned int*)((base)+(reg)))
#define I2C_REG_WRITE(base, reg, v) (*(volatile unsigned int*)((base)+(reg)) = (v))

// interrupt status
#define I2C_INT_GEN_CALL    (1<<11)
#define I2C_INT_START_DET   (1<<10)
#define I2C_INT_STOP_DET    (1<<9)
#define I2C_INT_ACTIVITY    (1<<8)
#define I2C_INT_RX_DONE     (1<<7)
#define I2C_INT_TX_ABRT     (1<<6)
#define I2C_INT_RD_REQ      (1<<5)
#define I2C_INT_TX_EMPTY    (1<<4)
#define I2C_INT_TX_OVER     (1<<3)
#define I2C_INT_RX_FULL     (1<<2)
#define I2C_INT_RX_OVER     (1<<1)
#define I2C_INT_RX_UNDER    (1<<0)

#define ARRAY_NUM(a)            (sizeof(a)/sizeof(a[0]))
static unsigned int i_i2c_get_base_addr(int id)
{
    if (id < 0 || id >= ARRAY_NUM(i2c_bases))
    {
        return 0;
    }

    return i2c_bases[id];
}

int i2c_master_init(int id, int speed)
{
    unsigned int base_addr;

    base_addr = i_i2c_get_base_addr(id);
    if (base_addr == 0)
    {
        return -1;
    }

    if (speed != 100) {
        dbg_printf(PRN_RES, "only support 100KHz\n");
        return -1;
    }

    I2C_REG_WRITE(base_addr, 0x6c, 0x0000); // disable
    I2C_REG_WRITE(base_addr, 0x30, 0x0000); // interrupt mask all
    I2C_REG_WRITE(base_addr, 0x38, 0x00ff);  // rx fifo threshold
    I2C_REG_WRITE(base_addr, 0x3c, 0x007f);  // tx fifo threshold
    I2C_REG_WRITE(base_addr, 0x14, 0x0064);  // IC_SS_SCL_HCNT
    I2C_REG_WRITE(base_addr, 0x18, 0x0096);  // IC_SS_SCL_LCNT
    I2C_REG_WRITE(base_addr, 0x7c, 0x0004);  // SDA hold time
    I2C_REG_WRITE(base_addr, 0x00, 0x0063);  // enable master mode

    return 0;
}

static int wait_to_idle(unsigned int base_addr)
{
    unsigned int data;
	unsigned int timeout = 1000;

    while (1)
    {
        data = I2C_REG_READ(base_addr, 0x34);
        if (data & (I2C_INT_TX_ABRT || I2C_INT_TX_OVER || I2C_INT_RX_OVER || I2C_INT_RX_OVER || I2C_INT_RX_UNDER))
        {
            dbg_printf(PRN_ERR, "error, status:0x%x\n", data);
            return -1;
        }
        if (data & I2C_INT_STOP_DET)  // done
        {
            return 0;
        }
        vTaskDelay(1);
		timeout--;
		if(timeout <= 0) {
			dbg_printf(PRN_ERR, "i2c timeout\n");
			return -1;
		}
    }
}

int i2c_master_write_and_read(int id, int target_addr, unsigned char* wbuf, int num_write, unsigned int* rbuf, int num_read)
{
    int ret = 0;
    unsigned int base_addr;
    unsigned int i;
    //unsigned int data;

    base_addr = i_i2c_get_base_addr(id);
    if (base_addr == 0) {
        return -1;
    }

    I2C_REG_WRITE(base_addr, 0x04, target_addr);    //target address
    I2C_REG_WRITE(base_addr, 0x6c, 0x0001);         // enable i2c

    for (i = 0; i < num_write; i++) {
        I2C_REG_WRITE(base_addr, 0x10, wbuf[i]);  // write data to tx fifo
    }

    for (i = 0; i < num_read; i++) {
        I2C_REG_WRITE(base_addr, 0x10, 0x100);  // write 0x100 to tx fifo
    }

    ret = wait_to_idle(base_addr);

    if (ret == 0)
    {
        for (i = 0; i < num_read; i++) {
            rbuf[i] = I2C_REG_READ(base_addr, 0x10);  // read n data
        }
    }

    I2C_REG_WRITE(base_addr, 0x6c, 0x0000);  // disable i2c

    return ret;
}
