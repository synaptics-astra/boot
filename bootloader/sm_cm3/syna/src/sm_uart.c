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
#include "sm_io.h"
#include "sm_memmap.h"

#include "sm_uart.h"

#define UART_REG_RBR	0x00	// Receive Buffer Register
#define UART_REG_IIR	0x08	// Interrupt Identification Register
#define UART_REG_USR	0x7C	// UART Status Register

#ifdef FPGA_V
#define A_F_V(a, f, v)      (v)
#else
#define A_F_V(a, f, v)      (a)
#endif

#define UART0_CLOCK_HZ              (25*1000*1000)
#define UART0_CLOCK_HZ_REAL         A_F_V((25*1000*1000), (25*1000*1000),  (9600*16))

static void delay_us(int x)
{
	volatile int i = 0;

	for (i = x; i > 0; i--);
}

#if 0
void uart_interrupt_handler(void * param)
{
    unsigned int uart_id = *(unsigned int *)param;
    unsigned int uart_base;
    unsigned int read;
    unsigned int interruptId;

    switch (uart_id)
    {
    case 0: uart_base = SM_SM_APB_UART0_BASE; break;
    case 1: uart_base = SM_SM_APB_UART1_BASE; break;
    default: return;
    }

    BFM_HOST_Bus_Read32( (uart_base+UART_REG_IIR), &interruptId);
    interruptId &= 0x0f;
    if (interruptId == 7) // Busy
    {
        BFM_HOST_Bus_Read32( (uart_base+UART_REG_USR), &read);
        //dbg_printf(PRN_RES, " UART get busy interrupt\n");
    }
    else if (interruptId == 4)
    {
        BFM_HOST_Bus_Read32( (uart_base+UART_REG_RBR), &read);
        //dbg_printf(PRN_RES, " UART get '%c', 0x%x\n", read, read);
        p_sm_soc_sync_data->sm_apb.SmSharedUartCount++;
    }
    else
    {
        dbg_printf(PRN_RES, " UART interrupt:%d should not generated\n", interruptId);
    }

}

int UART_receive_wake_up_signal(void)
{
	unsigned int read;
	unsigned int bingo = 0;
	char input;
	while (dbg_port_rx_ready())
	{
		BFM_HOST_Bus_Read32((SM_SM_APB_UART0_BASE + 0x00), &read);
		input = read & 0xFF;
		if ((input == 'W') || (input == 'w') || (input == ' '))
			bingo = 1;
	}
	return bingo;
}
#endif

void dbg_port_tx_ready(void)
{
	UNSG32 status=0;
	do
	{
		BFM_HOST_Bus_Read32((SM_SM_APB_UART0_BASE + 0x14), &status);
	} while (!(status&0x20)); //not check the busy bit, just check TX FIFO not full
}

unsigned int dbg_port_rx_ready(void)
{
	UNSG32 read;
	BFM_HOST_Bus_Read32((SM_SM_APB_UART0_BASE + 0x14), &read);		// status
	if (read & 0x01)
	{
		return 1;
	}
	return 0;
}

void put_char(char ch)
{
	dbg_port_tx_ready();
	BFM_HOST_Bus_Write32((SM_SM_APB_UART0_BASE + 0x00), (unsigned int)ch);
}

void dbg_port_rx_flush(void)
{
	unsigned int read;
	while(dbg_port_rx_ready())
		BFM_HOST_Bus_Read32((SM_SM_APB_UART0_BASE + 0x00), &read);
}

#define UART_BASE               (SM_SM_APB_UART0_BASE)   // uart0
#define UART_GET_STATUS()       (*(volatile unsigned int*)(UART_BASE+0x7c))
#define GET_CHAR()              (*(volatile unsigned int*)(UART_BASE+0));
#define PUT_CHAR(ch)            (*(volatile unsigned int*)(UART_BASE+0) = (ch));

#define UART_BAUDRATE       A_F_V(115200, 115200, UNDEF)
#define UART_CLOCK          (UART0_CLOCK_HZ_REAL)

#define UART_DIV_CAL        (((UART_CLOCK/16) + (UART_BAUDRATE/2)) / UART_BAUDRATE)
#define UART_DIV            A_F_V(UART_DIV_CAL, UART_DIV_CAL, 1)

void uart_init(void)
{
	int i;

	for(i = 0; i < 2000; i++)
	{
		if (((*(volatile unsigned int*)(UART_BASE+0x14)) & (1<<6)) != 0) // 0x14: Line Status Register, [6]: Transmitter Empty bit
		{
			// wait transmit complete
			break;
		}
	}

	*(volatile unsigned int*)(UART_BASE+0x88) = 0x07;       // software reset
	// LCR & divider
	*(volatile unsigned int*)(UART_BASE+0x0C) = *(volatile unsigned int*)(UART_BASE+0x0c) | 0x83;
	*(volatile unsigned int*)(UART_BASE+0x00) = UART_DIV & 0xff;
	*(volatile unsigned int*)(UART_BASE+0x04) = (UART_DIV>>8) & 0xff;
	*(volatile unsigned int*)(UART_BASE+0x0C) = *(volatile unsigned int*)(UART_BASE+0x0c) & (~0x80);

	*(volatile unsigned int*)(UART_BASE+0x08) = 1; // FIFO Control Register, fifo enabled

	delay_us(10);       // add some delay
}
