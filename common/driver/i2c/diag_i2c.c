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
//#include "diag_common.h"
//#include "diag_ictl.h"
//#include "diag_misc.h"
#include "io.h"
#include "debug.h"
#include "apbRegBase.h"
#include "timer.h"

#undef dbg_printf
#define dbg_printf(dev, fmt, args...)

const unsigned int i2c_bases[] =
{
    APB_I2C0_BASE,
    APB_I2C1_BASE,
#ifdef CONFIG_SM
    SM_APB_I2C0_BASE,
    SM_APB_I2C1_BASE,
#endif
};

#if 0
const int i2c_irq_id[] =
{
    ICTL_IRQ_I2C0,
    ICTL_IRQ_I2C1,
    SMICTL_IRQ_SM_I2C0,
    SMICTL_IRQ_SM_I2C1,
};
#endif

#define CONFIG_CLOCK_KHZ	200000
#define SM_CLOCK_KHZ		25000

#define SOC_I2C_CLOCK               CONFIG_CLOCK_KHZ    // KHz
#define SM_I2C_CLOCK                SM_CLOCK_KHZ        // KHz
#define I2C_TIMEOUT                 (1000*1000)     // us
#define I2C_POLLING_INTERVAL        10              // us

#define I2C_REG_READ(base, reg)     (*(volatile unsigned int*)(uintptr_t)((base)+(reg)))
#define I2C_REG_WRITE(base, reg, v) (*(volatile unsigned int*)(uintptr_t)((base)+(reg)) = (v))
#define I2C_REG_WRITE_WITH_MASK(base, a, v, m)   I2C_REG_WRITE(base, a, (I2C_REG_READ(base, a)&(~(m))) | ((v)&(m)))

#define I2C_RAWINTSTATE(base)       I2C_REG_READ(base, 0x34)
#define I2C_STATE(base)             I2C_REG_READ(base, 0x70)
#define I2C_GETDATA(base)           I2C_REG_READ(base, 0x10)
#define I2C_SETDATA(base, v)        I2C_REG_WRITE(base, 0x10, v)

#define I2C_TX_FIFO_LEVEL(base)     I2C_REG_READ(base, 0x74)
#define I2C_RX_FIFO_LEVEL(base)     I2C_REG_READ(base, 0x78)

#define I2C_ENABLE(base)            I2C_REG_WRITE(base, 0x6C, 1)
#define I2C_DISABLE(base)           I2C_REG_WRITE(base, 0x6C, 0)

#define I2C_STATE_ACTIVE                (1<<0)
#define I2C_STATE_TX_FIFO_NOT_FULL      (1<<1)
#define I2C_STATE_TX_FIFO_EMPTY         (1<<2)
#define I2C_STATE_RX_FIFO_NOT_EMPTY     (1<<3)

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

#define I2C_FIFO_DEPTH          256
#define I2C_FIFO_DEPTH_TH       (I2C_FIFO_DEPTH/2)

#define ARRAY_NUM(a)            (sizeof(a)/sizeof(a[0]))

typedef struct
{
    unsigned int base_addr;

    unsigned char* send_buff;
    unsigned char* recv_buff;

    int send_pos;
    int send_length;

    int recv_req_pos;
    int recv_length;

    int recv_pos;

    int done;
    unsigned int status;
    unsigned int abort_source;

} i2c_context_t;

static unsigned int i_i2c_get_base_addr(int id)
{
    if (id < 0 || id >= ARRAY_NUM(i2c_bases))
    {
        return 0;
    }

    return i2c_bases[id];
}

static int i_i2c_is_sm(int id)
{
    if (id >= 2)
    {
        return 1;
    }

    return 0;
}

int i2c_master_init(int id, int speed, int b_10bit_addr)
{
    unsigned int base_addr;
    int b_sm;
    int base_clock;
    int high_count;
    int low_count;
    int hold_count;
    int b_use_fast = 0;
    unsigned int control_value;

    base_addr = i_i2c_get_base_addr(id);
    if (base_addr == 0)
    {
        return -1;
    }

    control_value = (1<<6) | (1<<5) | (1<<0); // slave disabled, re-start en, master enbled

    I2C_REG_WRITE(base_addr, 0x6C, 0);         // disable
    I2C_REG_WRITE(base_addr, 0x30, 0);         // interrupt mask all

    // try to find the max fifo depth and set threshold to half
    I2C_REG_WRITE(base_addr, 0x38, I2C_FIFO_DEPTH-1);       // rx fifo threshold
    I2C_REG_WRITE(base_addr, 0x3C, I2C_FIFO_DEPTH-1);       // tx fifo threshold
    //DIAG_ASSERT(I2C_REG_READ(base_addr, 0x38) == I2C_FIFO_DEPTH-1);
    //DIAG_ASSERT(I2C_REG_READ(base_addr, 0x3C) == I2C_FIFO_DEPTH-1);

    I2C_REG_WRITE(base_addr, 0x3C, I2C_FIFO_DEPTH_TH-1);   // tx fifo threshold

    b_sm = i_i2c_is_sm(id);
    base_clock = b_sm?SM_I2C_CLOCK:SOC_I2C_CLOCK;

    if (speed > 100)
    {
        b_use_fast = 1;
    }

    if (b_use_fast)
    {
        // default clock
        high_count  = base_clock/speed*40/100;              // high:40%
        low_count   = base_clock/speed - high_count;

        I2C_REG_WRITE(base_addr, 0x1C, high_count);         // IC_FS_SCL_HCNT
        I2C_REG_WRITE(base_addr, 0x20, low_count);          // IC_FS_SCL_LCNT

        control_value |= 2<<1;
    }
    else
    {
        high_count  = base_clock/speed*40/100;              // high:40%
        low_count   = base_clock/speed - high_count;

        I2C_REG_WRITE(base_addr, 0x14, high_count);        // IC_SS_SCL_HCNT
        I2C_REG_WRITE(base_addr, 0x18, low_count);         // IC_SS_SCL_LCNT

        control_value |= 1<<1;
    }

    hold_count  = low_count*30/100;
    I2C_REG_WRITE(base_addr, 0x7C, hold_count);              // SDA hold time

    if (b_10bit_addr)
    {
        control_value |= (1<<4);  // enable 10 bit mode for master
    }

    // enable master mode
    I2C_REG_WRITE(base_addr, 0x00, control_value);

    dbg_printf(PRN_INFO, "I2C_%d, HCNT:%d, LCNT:%d, control:0x%02x\n", id, high_count, low_count, control_value);

    return 0;
}

int i2c_set_hold_time(int id, int value)
{
    unsigned int base_addr;

    base_addr = i_i2c_get_base_addr(id);
    if (base_addr == 0)
    {
        return -1;
    }

    I2C_REG_WRITE(base_addr, 0x7C, value);

    return 0;
}


int i2c_set_setup_time(int id, int value)
{
    unsigned int base_addr;

    base_addr = i_i2c_get_base_addr(id);
    if (base_addr == 0)
    {
        return -1;
    }

    I2C_REG_WRITE(base_addr, 0x94, value);

    return 0;
}


int i2c_int_handle(i2c_context_t* p_context)
{
    unsigned int int_status;
    unsigned int base_addr = p_context->base_addr;
    unsigned int dump;
    int remain;

    int_status = I2C_REG_READ(base_addr, 0x2C);   // interrupt status

    if (int_status & I2C_INT_TX_ABRT)
    {
        p_context->abort_source = I2C_REG_READ(base_addr, 0x80);
    }

    if (int_status != 0)
    {
        //dump = I2C_REG_READ(base_addr, 0x40);  // clear combined interrupt

        // clear one by one
        //0  IC_CLR_RX_UNDER     0x44
        if (int_status & I2C_INT_RX_UNDER)  dump = I2C_REG_READ(base_addr, 0x44);
        //1  IC_CLR_RX_OVER      0x48
        if (int_status & I2C_INT_RX_OVER)   dump = I2C_REG_READ(base_addr, 0x48);
        //3  IC_CLR_TX_OVER      0x4C
        if (int_status & I2C_INT_TX_OVER)   dump = I2C_REG_READ(base_addr, 0x4C);
        //5  IC_CLR_RD_REQ       0x50
        if (int_status & I2C_INT_RD_REQ)    dump = I2C_REG_READ(base_addr, 0x50);
        //6  IC_CLR_TX_ABRT      0x54
        if (int_status & I2C_INT_TX_ABRT)   dump = I2C_REG_READ(base_addr, 0x54);
        //7  IC_CLR_RX_DONE      0x58
        if (int_status & I2C_INT_RX_DONE)   dump = I2C_REG_READ(base_addr, 0x58);
        //8  IC_CLR_ACTIVITY     0x5c
        if (int_status & I2C_INT_ACTIVITY)  dump = I2C_REG_READ(base_addr, 0x5C);
        //9  IC_CLR_STOP_DET     0x60
        if (int_status & I2C_INT_STOP_DET)  dump = I2C_REG_READ(base_addr, 0x60);
        //10 IC_CLR_START_DET    0x64
        if (int_status & I2C_INT_START_DET) dump = I2C_REG_READ(base_addr, 0x64);
        //11 IC_CLR_GEN_CALL     0x68
        if (int_status & I2C_INT_GEN_CALL)  dump = I2C_REG_READ(base_addr, 0x68);
    }
	dump = 0;
	if(dump) {
		dbg_printf(PRN_INFO, "dump");
	}

    p_context->status |= int_status;

    if (int_status & I2C_INT_RX_FULL)    // rx full
    {
        remain = I2C_RX_FIFO_LEVEL(base_addr);
        while(remain > 0)
        {
            p_context->recv_buff[p_context->recv_pos++] = I2C_GETDATA(base_addr);
            remain--;
        }

        if (p_context->recv_length - p_context->recv_pos < I2C_FIFO_DEPTH_TH)
        {
            I2C_REG_WRITE(base_addr, 0x38, p_context->recv_length - p_context->recv_pos - 1);   // rx fifo threshold
        }
    }

    if (int_status & I2C_INT_TX_EMPTY)    // tx empty
    {
        remain = I2C_FIFO_DEPTH - I2C_TX_FIFO_LEVEL(base_addr);
        while(remain > 0)
        {
            if (p_context->send_pos < p_context->send_length)
            {
                I2C_SETDATA(base_addr, p_context->send_buff[p_context->send_pos++]);
                remain--;
            }
            else if (p_context->recv_req_pos < p_context->recv_length)
            {
                // send read req
                I2C_SETDATA(base_addr, 0x100);
                remain--;
                p_context->recv_req_pos++;
            }
            else
            {
                // send done
                I2C_REG_WRITE_WITH_MASK(base_addr, 0x30, 0, I2C_INT_TX_EMPTY);   // 0: interrupt masked
                break;
            }
        }
    }

    if (int_status & I2C_INT_STOP_DET)    // stopped
    {
        dump = I2C_REG_READ(base_addr, 0x60);  // clear STOP

        // recv data in fifo
        while(I2C_STATE(base_addr) & I2C_STATE_RX_FIFO_NOT_EMPTY)
        {
            if (p_context->recv_pos < p_context->recv_length)
            {
                p_context->recv_buff[p_context->recv_pos++] = I2C_GETDATA(base_addr);
            }
            else
            {
                //unsigned int data;
                // dump
                //data = I2C_GETDATA(base_addr);
                I2C_GETDATA(base_addr);
            }
        }

        p_context->done = 1;
    }

    if ((int_status & I2C_INT_TX_ABRT) ||
        (int_status & I2C_INT_TX_OVER) ||
        (int_status & I2C_INT_RX_OVER) ||
        (int_status & I2C_INT_RX_UNDER))
    {
        p_context->done = -1;
        I2C_DISABLE(base_addr);  // disable
    }

    return 0;
}

void i2c_tx_abort_info(unsigned int abort_source)
{
    const char* tx_abort_infos[] =
    {
        "(7-bit addressing) address not ACKed by any slave",
        "(10-bit addressing) 1st address byte not ACKed by any slave",
        "(10-bit addressing) 2nd address byte not ACKed by any slave",
        "address ACKed, data not ACKed by slave",
        "general call not responded by any slave",
        "general call followed by read command from user",
        "master in high speed mode and high speed master code ACKed",
        "master sent a start byte and start byte ACKed",
        "restart disabled while user trying to use master to send data in high speed mode",
        "restart disabled while user trying to send a start byte",
        "restart disabled and master send a read command in 10-bit addressing mode",
        "user attempted to use disabled master",
        "lost arbitration",
        "slave received read command while data exists in Tx FIFO",
        "slave lost bus while trasmitting data to a remote master",
        "slave requesting data to Tx and user wrote read command into Tx FIFO"
    };

    int i;

    dbg_printf(PRN_ERR, " tx abort source:0x%x\n", abort_source);

    for (i = 0; i < ARRAY_NUM(tx_abort_infos); i++)
    {
        if (abort_source & (1<<i))
        {
            dbg_printf(PRN_ERR, "  bit%d:%s\n", i, tx_abort_infos[i]);
        }
    }
}

int i2c_master_write_and_read(int id, int target_addr, unsigned char* send_buff, int send_len, unsigned char* recv_buff, int recv_len)
{
    unsigned int base_addr;
    i2c_context_t i2c_context;

    base_addr = i_i2c_get_base_addr(id);
    if (base_addr == 0)
    {
        return -1;
    }

    I2C_REG_WRITE(base_addr, 0x04, target_addr);  // target address

    // prepare context and enable int
    i2c_context.base_addr = base_addr;
    i2c_context.send_buff = send_buff;
    i2c_context.recv_buff = recv_buff;

    i2c_context.send_pos = 0;
    i2c_context.recv_pos = 0;

    i2c_context.send_length  = send_len;
    i2c_context.recv_req_pos = 0;
    i2c_context.recv_length  = recv_len;

    i2c_context.done = 0;
    i2c_context.status = 0;
    i2c_context.abort_source = 0;

    I2C_REG_WRITE(base_addr, 0x30, 0xFFF);         // interrupt unmask all

    if (i2c_context.recv_length < I2C_FIFO_DEPTH_TH)
    {
        I2C_REG_WRITE(base_addr, 0x38, i2c_context.recv_length - 1);   // rx fifo threshold
    }
    else
    {
        I2C_REG_WRITE(base_addr, 0x38, I2C_FIFO_DEPTH_TH-1);
    }

    // kick off
    I2C_ENABLE(base_addr);

    // wait done
    {
        int timeout = I2C_TIMEOUT;
        int recv_pos = 0;
        int send_pos = 0;
        while(timeout > 0)
        {
            i2c_int_handle(&i2c_context);

            if (i2c_context.recv_pos > recv_pos || i2c_context.send_pos > send_pos)    // received
            {
                recv_pos = i2c_context.recv_pos;
                send_pos = i2c_context.send_pos;
                timeout = I2C_TIMEOUT;
            }

            if (i2c_context.done != 0)
            {
                break;
            }

            udelay(I2C_POLLING_INTERVAL);
            timeout -= I2C_POLLING_INTERVAL;
        }

        if (i2c_context.done != 1)
        {
            dbg_printf(PRN_ERR, " i2c status:0x%08x, send_num:%d (expect:%d), recv_num:%d (expect:%d)\n",
                        i2c_context.status, i2c_context.send_pos, i2c_context.send_length, i2c_context.recv_pos, i2c_context.recv_length);

            if (i2c_context.abort_source != 0)
            {
                i2c_tx_abort_info(i2c_context.abort_source);
            }
        }
    }

    // disable
    I2C_DISABLE(base_addr);

    return i2c_context.done==1?0:-1;
}

#if 0
static int g_i2c_slave_simulation_service_running = 0;

int i_i2c_change_to_slave_mode(unsigned int base_addr)
{
    I2C_REG_WRITE_WITH_MASK(base_addr, 0, 0, (1<<6)|(1<<0));
}

int i2c_slave_simulation_service_stop()
{
    g_i2c_slave_simulation_service_running = 0;
}

// simulate a i2c slave, which will receive data into buffer and send back data from buffer
int i2c_slave_simulation_service_start(int id, int slave_addr, unsigned char* buff)
{
    unsigned int base_addr;

    base_addr = i_i2c_get_base_addr(id);
    if (base_addr == 0)
    {
        return -1;
    }
    if (g_i2c_slave_simulation_service_running)
    {
        dbg_printf(PRN_RES, " I2C %d slave service is already started\n");
        return 0;
    }

    dbg_printf(PRN_RES, " I2C %d slave simulation is running, addr:0x%x buffer address:0x%x\n", id, slave_addr, buff);
    g_i2c_slave_simulation_service_running = 1;

    i_i2c_change_to_slave_mode(base_addr);

    while(g_i2c_slave_simulation_service_running)
    {
        int done = 0;
        int recv_pos = 0;
        int send_pos = 0;
        unsigned int status = 0;

        I2C_REG_WRITE(base_addr, 0x08, slave_addr);  // slave address
        I2C_REG_WRITE(base_addr, 0x30, 0xFFF&(~I2C_INT_TX_EMPTY)&(~I2C_INT_ACTIVITY));       // interrupt unmask all
        I2C_ENABLE(base_addr);                       // kick off

        while(g_i2c_slave_simulation_service_running)
        {
            unsigned int int_status;
            unsigned int abort_source;

            int_status = I2C_REG_READ(base_addr, 0x2C);   // interrupt status

            if (int_status & I2C_INT_TX_ABRT)
            {
                abort_source = I2C_REG_READ(base_addr, 0x80);
            }

            if (int_status != 0)
            {
                unsigned int dump;
                dump = I2C_REG_READ(base_addr, 0x40);  // clear combined interrupt

                dbg_printf(PRN_INFO, " int_status:0x%x\n", int_status);
            }

            status |= int_status;

            if ((int_status & I2C_INT_RX_FULL) ||       // rx full or stop
                (int_status & I2C_INT_STOP_DET) ||      // stop
                (int_status & I2C_INT_START_DET))       // re-start
            {
                int remain = I2C_RX_FIFO_LEVEL(base_addr);
                while(remain > 0)
                {
                    buff[recv_pos++] = I2C_GETDATA(base_addr);
                    remain--;
                }
                dbg_printf(PRN_INFO, " recv_pos:%d\n", recv_pos);
            }

            if (int_status & I2C_INT_STOP_DET)    // stopped
            {
                done = 1;            // done
                break;
            }

            if (int_status & I2C_INT_RD_REQ)    // master read request
            {
                // send out data
                I2C_SETDATA(base_addr, buff[send_pos++]);
            }

            if ((int_status & I2C_INT_TX_EMPTY)||
                (int_status & I2C_INT_TX_ABRT) ||
                (int_status & I2C_INT_TX_OVER) ||
                (int_status & I2C_INT_RX_OVER) ||
                (int_status & I2C_INT_RX_UNDER))
            {
                dbg_printf(PRN_ERR, " i2c get unexpected interrupt\n");
                done = -1;
                break;
            }
        }
        I2C_DISABLE(base_addr);

        dbg_printf(PRN_INFO, " i2c done:%d recv_pos:%d, send_pos:%d status:0x%x\n",
                done, recv_pos, send_pos, status);
    }

    g_i2c_slave_simulation_service_running = 0;
    dbg_printf(PRN_RES, " I2C %d slave simulation is stopped\n", id);

    return 0;
}

int i2c_loopback(int id, unsigned char* send_buff, unsigned char* recv_buff, int length, int ictl_id)
{
    unsigned int base_addr;
    unsigned int control_value;
	int b_sm;
    i2c_context_t i2c_context;
    int ret;

    base_addr = i_i2c_get_base_addr(id);
    if (base_addr == 0)
    {
        return -1;
    }

    control_value = I2C_REG_READ(base_addr, 0x00);

    b_sm = i_i2c_is_sm(id);
    if (b_sm)
    {
        I2C_REG_WRITE(base_addr, 0x04, 0x1055);  // target address
        I2C_REG_WRITE(base_addr, 0x08, 0x55);  // slave address
    }
    else
    {
        I2C_REG_WRITE(base_addr, 0x04, 0x11EA);  // target address
        I2C_REG_WRITE(base_addr, 0x08, 0x1EA);  // slave address
    }

    control_value &= ~(1<<6);   // clear slave disable, clear 10 bit mode for slave
    control_value |= (1<<4) | (1<<3);    // enable 10 bit mode for slave and master

    I2C_REG_WRITE(base_addr, 0x00, control_value);


    // prepare context and enable int
    i2c_context.base_addr = base_addr;
    i2c_context.send_buff = send_buff;
    i2c_context.recv_buff = recv_buff;

    i2c_context.send_pos = 0;
    i2c_context.recv_pos = 0;

    i2c_context.send_length  = length;
    i2c_context.recv_req_pos = length;  // no recv reqest for loopback
    i2c_context.recv_length  = length;

    i2c_context.done = 0;
    i2c_context.status = 0;
    i2c_context.abort_source = 0;

    I2C_REG_WRITE(base_addr, 0x30, 0xFFF);         // interrupt unmask all

    if (i2c_context.recv_length < I2C_FIFO_DEPTH_TH)
    {
        I2C_REG_WRITE(base_addr, 0x38, i2c_context.recv_length - 1);   // rx fifo threshold
    }
    else
    {
        I2C_REG_WRITE(base_addr, 0x38, I2C_FIFO_DEPTH_TH-1);
    }

    if (ictl_id >= 0)
    {
        // setup interrupt
        diag_ictl_set_handle(b_sm, ictl_id, i2c_irq_id[id], i2c_int_handle, &i2c_context);
        ret = diag_ictl_enable(b_sm, ictl_id, i2c_irq_id[id], 1);
        if (ret != 0)
        {
            return ret;
        }
    }

    // kick off
    I2C_ENABLE(base_addr);

    // wait done
    {
        int timeout = I2C_TIMEOUT;
        int recv_pos = 0;
        int send_pos = 0;
        while(timeout > 0)
        {
            if (ictl_id < 0)
            {
                i2c_int_handle(&i2c_context);
            }

            if (i2c_context.recv_pos > recv_pos || i2c_context.send_pos > send_pos)    // received
            {
                recv_pos = i2c_context.recv_pos;
                send_pos = i2c_context.send_pos;
                timeout = I2C_TIMEOUT;
            }

            if (i2c_context.done != 0)
            {
                break;
            }

            udelay(I2C_POLLING_INTERVAL);
            timeout -= I2C_POLLING_INTERVAL;
        }

        if (i2c_context.done != 1)
        {
            dbg_printf(PRN_ERR, " i2c status:0x%08x, send_num:%d (expect:%d), recv_num:%d (expect:%d)\n",
                        i2c_context.status, i2c_context.send_pos, i2c_context.send_length, i2c_context.recv_pos, i2c_context.recv_length);

            dbg_printf(PRN_ERR, " remain byte, tx_fifo:%d, rx_fifo:%d\n", I2C_TX_FIFO_LEVEL(base_addr), I2C_RX_FIFO_LEVEL(base_addr));

            if (i2c_context.abort_source != 0)
            {
                i2c_tx_abort_info(i2c_context.abort_source);
            }
       }
    }

    // disable
    I2C_DISABLE(base_addr);

    if (ictl_id >= 0)
    {
        // disable interrupt
        diag_ictl_enable(b_sm, ictl_id, i2c_irq_id[id], 0); // disable interrupt
    }

    return i2c_context.done==1?0:-1;
}

int i2c_check_slave(int id, int target_addr)
{
    unsigned int base_addr;
    i2c_context_t i2c_context;
    unsigned char send_buff[1] = {0};

    base_addr = i_i2c_get_base_addr(id);
    if (base_addr == 0)
    {
        return -1;
    }

    I2C_REG_WRITE(base_addr, 0x04, target_addr);  // target address

    // prepare context and enable int
    i2c_context.base_addr = base_addr;
    i2c_context.send_buff = send_buff;
    i2c_context.recv_buff = NULL;

    i2c_context.send_pos = 0;
    i2c_context.recv_pos = 0;

    i2c_context.send_length  = 1;
    i2c_context.recv_req_pos = 0;
    i2c_context.recv_length  = 0;

    i2c_context.done = 0;
    i2c_context.status = 0;
    i2c_context.abort_source = 0;

    I2C_REG_WRITE(base_addr, 0x30, 0xFFF);         // interrupt unmask all

    // kick off
    I2C_ENABLE(base_addr);

    // wait done
    {
        int timeout = I2C_TIMEOUT;
        while(timeout > 0)
        {
            i2c_int_handle(&i2c_context);

            if (i2c_context.done != 0)
            {
                break;
            }

            udelay(I2C_POLLING_INTERVAL);
            timeout -= I2C_POLLING_INTERVAL;
        }

        if (i2c_context.done != 1)
        {
            dbg_printf(PRN_INFO, " i2c status:0x%08x, abort_source:0x%x\n", i2c_context.status, i2c_context.abort_source);
        }
    }

    // disable
    I2C_DISABLE(base_addr);

    return i2c_context.done==1?0:-1;
}
#endif
