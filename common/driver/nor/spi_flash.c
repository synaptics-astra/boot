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
#include "com_type.h"
//use c to make 32bit and 64bit system compatible
void SPIReadFlash(unsigned long src, unsigned int len, unsigned long dst)
{
	//the src address should be 4 bytes aligned
	//if((strDest != NULL) && (strSrc != NULL))
	{
		uint32_t *addressDest = (uint32_t *)dst;
		uint32_t *addressSrc = (uint32_t *) src;
		while (len--)
			*addressDest++ = *addressSrc++;
	}
}
/* drivers below comes from DIAG */
#include "io.h"
#include "debug.h"
#include "apbRegBase.h"
#include "timer.h"
#include "string.h"

#define SPI_BASE_ADDRESS            (APB_SSI1_BASE)
#define GET_VALUE(a)                (*(volatile unsigned int*)(SPI_BASE_ADDRESS+(a)))
#define SET_VALUE(a, v)             ((*(volatile unsigned int*)(SPI_BASE_ADDRESS+(a))) = (v))
#define SET_VALUE_MASK(a, v, m)     SET_VALUE(a, (GET_VALUE(a)&(~(m))) | ((v)&(m)))

#define SPI_MODE_RECV_SEND          0
#define SPI_MODE_TRANS_ONLY         1
#define SPI_MODE_RECV_ONLY          2
#define SPI_MODE_RECV_EEPROM        3

#define SPI_SET_MODE(mode)          SET_VALUE_MASK(0x00, ((mode)<<8), (3<<8))

#define SPI_DISABLE()               SET_VALUE(0x08, 0)
#define SPI_ENABLE()                SET_VALUE(0x08, 1)
#define SPI_CLEAR_RECV()            {while(GET_VALUE(0x24) > 0) GET_VALUE(0x60);}
#define SPI_TX_NOT_FULL()           (((GET_VALUE(0x28)&0x2))?1:0)
#define SPI_IS_BUSY()               (((GET_VALUE(0x28)&0x1))?1:0) // Status, bit0:busy

#define SPI_START()                 SET_VALUE(0x10, 1)
#define SPI_STOP()                  SET_VALUE(0x10, 0)

#define PROGRESS_POINT_SIZE         0x10000

#define SPI_VENDOR_UNKNOWN          0
#define SPI_VENDOR_WINBOND          1
#define SPI_VENDOR_ST               2

int spi_return_back_to_normal_mode()
{
    // make spi controlled by bus read controller. (direct read using 0xF0000000)
    SPI_DISABLE();
    SPI_SET_MODE(SPI_MODE_RECV_EEPROM);
    SET_VALUE(0x04, 3); // number of data frames
    SPI_ENABLE();
    SPI_START();

    return 0;
}

int spi_init()
{
    return 0;
}

int spi_wait_busy()
{
    int timeout = 1000000;

    while (timeout--)
    {
        int i;
        int ret;
        ret = SPI_IS_BUSY();
        for(i = 0; i < 1000; i++); // delay to let device complete
        if (!ret) // not busy
        {
            break;
        }
    }

    if (timeout <= 0)
    {
        return -1;
    }

    return 0;
}

int spi_get_status()
{
    unsigned char status;

    SPI_DISABLE();
    SPI_SET_MODE(SPI_MODE_RECV_EEPROM);
    SET_VALUE(0x04, 1); // number of data frames
    SPI_ENABLE();

    SPI_STOP(); // disable slave first
    SET_VALUE(0x60, 0x05); // cmd = 0x05 (read status 1)
    SPI_START(); // enable slave

    if (spi_wait_busy() < 0)
    {
        dbg_printf(PRN_RES, "get status timeout\n");
        return -1;
    }

    status = (unsigned char)(GET_VALUE(0x60) & 0xff);
    return status;
}


int spi_wait_write_complete()
{
    int timeout = 10*1000*1000; // 10s
    // wait erase complete
    while(timeout > 0)
    {
        int status;
        status = spi_get_status();
        if (!(status & 0x1))
        {
            break;
        }
        // delay
        udelay(100);
        timeout -= 100;
    }

    if (timeout <= 0)
    {
        return -1;
    }

    return 0;
}

int spi_read_jedec_id(int* p_vendor)
{
    unsigned char buff[3];
    int vendor;
    char* vendor_name;

    SPI_DISABLE();
    SPI_SET_MODE(SPI_MODE_RECV_EEPROM);
    SET_VALUE(0x04, 2); // number of data frames
    SPI_ENABLE();

    SPI_STOP(); // disable slave first

    SET_VALUE(0x60, 0x9F); // cmd = 0xAB (read)

    SPI_START(); // enable slave

    if (spi_wait_busy() < 0)
    {
        dbg_printf(PRN_RES, "get jedec id timeout\n");
        return -1;
    }

    buff[0] = GET_VALUE(0x60);
    buff[1] = GET_VALUE(0x60);
    buff[2] = GET_VALUE(0x60);

    if (buff[0] == 0xEF)
    {
        vendor = SPI_VENDOR_WINBOND;
        vendor_name = "WINBOND";
    }
    else if (buff[0] == 0x20)
    {
        vendor = SPI_VENDOR_ST;
        vendor_name = "ST";
    }
    else
    {
        vendor = SPI_VENDOR_UNKNOWN;
        vendor_name = "UNKNOWN";
    }

    if (p_vendor)
    {
        *p_vendor = vendor;
    }
    else
    {
        dbg_printf(PRN_RES, "JEDEC ID:0x%02x(%s) 0x%02x 0x%02x\n", buff[0], vendor_name, buff[1], buff[2]);
    }

    SPI_STOP();

    return 0;
}

int spi_info()
{
    int ret;

    spi_init();

    ret = spi_read_jedec_id(NULL);
    if (ret < 0)
    {
        return ret;
    }
    spi_return_back_to_normal_mode();
    return ret;
}

int spi_write_enable()
{
    SPI_DISABLE();
    SPI_SET_MODE(SPI_MODE_TRANS_ONLY);
    SPI_ENABLE();

    SPI_STOP(); // disable slave first
    SET_VALUE(0x60, 0x06); // cmd = 0x06 (write enable)
    SPI_START(); // enable slave

    if (spi_wait_busy() < 0)
    {
        dbg_printf(PRN_RES, "write enable timeout\n");
        return -1;
    }

    return 0;
}

int spi_earse_a_block(unsigned int offset)
{
    if (spi_write_enable() < 0)
    {
        return -1;
    }

    SPI_DISABLE();
    SPI_SET_MODE(SPI_MODE_TRANS_ONLY);
    SPI_ENABLE();

    SPI_STOP(); // disable slave first

    SET_VALUE(0x60, 0xD8); // cmd = 0xD8 (block erase, 64KB)

    // offset
    SET_VALUE(0x60, (offset >> 16) & 0xff);
    SET_VALUE(0x60, (offset >> 8) & 0xff);
    SET_VALUE(0x60, (offset >> 0) & 0xff);

    SPI_START(); // enable slave

    if (spi_wait_busy() < 0)
    {
        dbg_printf(PRN_RES, "erase block timeout\n");
        return -1;
    }

    // wait erase complete
    if (spi_wait_write_complete() < 0)
    {
        dbg_printf(PRN_RES, "erase timeout\n");
        return -1;
    }

    return 0;
}

int spi_erase(unsigned int offset, int size)
{
    unsigned int start = offset;
    unsigned int end = offset+size;
    int block_size = (64*1024);
    int ret;

    start &= ~(block_size-1);

    while(start < end)
    {
        ret = spi_earse_a_block(start);
        if (ret < 0)
        {
            return -1;
        }
        start += block_size;

        if ((start % PROGRESS_POINT_SIZE) == 0)
        {
            int i;
            for (i = 0; i < block_size; i += PROGRESS_POINT_SIZE)
            {
                dbg_printf(PRN_RES, ".");
            }
        }
    }
    dbg_printf(PRN_RES, "\n");
    spi_return_back_to_normal_mode();

    return 0;
}

int spi_write_a_page(unsigned int memory, unsigned int offset, int size)
{
    unsigned int start = offset;
    unsigned int end;

    if (size > 256)
    {
        size = 256;
    }
    end = start + size;

    while(start < end)
    {
        if (spi_write_enable() < 0)
        {
            return -1;
        }

        SPI_DISABLE();
        SPI_SET_MODE(SPI_MODE_TRANS_ONLY);
        SPI_ENABLE();

        SPI_STOP(); // disable slave first

        SET_VALUE(0x60, 2); // cmd = 2 (page program)

        // offset
        SET_VALUE(0x60, (offset >> 16) & 0xff);
        SET_VALUE(0x60, (offset >> 8) & 0xff);
        SET_VALUE(0x60, (offset >> 0) & 0xff);

        // write out to fifo
        while(SPI_TX_NOT_FULL() && (start < end))
        {
            SET_VALUE(0x60, *(unsigned char*)(uintptr_t)memory);
            memory++;
            start++;
            offset++;
        }

        SPI_START(); // enable slave

        // if fifo is full, continue
        while (start < end)
        {
            unsigned int timeout = 0xffffffff;
            // wait fifo empty
            while(!SPI_TX_NOT_FULL() && --timeout);
            if (timeout == 0) break;
            SET_VALUE(0x60, *(unsigned char*)(uintptr_t)memory);
            memory++;
            start++;
            offset++;
        }

        if (spi_wait_busy() < 0)
        {
            dbg_printf(PRN_RES, "write busy timeout\n");
            return -1;
        }

        // wait write complete
        if (spi_wait_write_complete() < 0)
        {
            dbg_printf(PRN_RES, "write timeout\n");
            return -1;
        }

        SPI_STOP();
    }

    return size;
}

int spi_write(unsigned int memory, unsigned int offset, int size)
{
    unsigned int mem = memory;
    unsigned int addr = offset;
    int write;

    while(size > 0)
    {
        write = spi_write_a_page(mem, addr, size);
        if (write < 0)
        {
            return -1;
        }
        mem += write;
        addr += write;
        size -= write;
        if ((addr % PROGRESS_POINT_SIZE) == 0)
        {
            dbg_printf(PRN_RES, ".");
        }
    }
    dbg_printf(PRN_RES, "\n");
    spi_return_back_to_normal_mode();

    return 0;
}

