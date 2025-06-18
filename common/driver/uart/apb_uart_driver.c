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
#include "com_type.h" /* include <common.h> for __UBOOT__ vsprintf */

#include "io.h"
#include "apb_uart.h"
#include "apb_uart_drv.h"
#include "global.h"

#define __UART_OUTPUT_BUF_SIZE 80
static UNSG32 g_uart_base;

/*******************************************************************************
* Function:    APB_UART_putc
*
* Description: initialize UART
*
* Inputs:      channel-- choose UART0 or UART1, 0 means UART0, 1 means UART1
*			   c-- output characterPB_UART_init
* Outputs:     none
*
* Return:      none
*******************************************************************************/
void APB_UART_putc(UINT32 channel, CHAR ch)
{
	uintptr_t base = g_uart_base;
	UINT32 status, ulReg;
	UINT32 timeout = 100000;

	if (channel == 0) base = g_uart_base;
	//FIXME: For a0, UART 1 movded from sm to soc
	//for bringup, we just comment it out
	//else if (channel == 1) base = SM_APB_UART1_BASE;

	//Enable thr for write? LCR
	REG_READ32(base+APB_UART_LCR, &ulReg);
	REG_WRITE32(base+APB_UART_LCR, ulReg & ~(BIT(7)));

	// Wait for Tx FIFO not full(Check THRE bit)
	do
	{
		REG_READ32(base+APB_UART_LSR, &status);
	}
	while (!(status & BIT(5)) && (timeout-- > 0)) ;

	//UART TX data register
	REG_WRITE32(base+APB_UART_THR, ch);
}

/*******************************************************************************
* Function: APB_UART_tstc
*
* Description: Test if there's input character from UART.
*
* Inputs: channel-- choose UART0 or UART1, 0 means UART0, 1 means UART1
*
* Outputs: none
*
* Return: 0-- there's no char input
* 		  1-- there's char input
*******************************************************************************/
UCHAR APB_UART_tstc(UINT32 channel)
{
	uintptr_t base = g_uart_base;
	UINT32 status;

	if (channel == 0) base = g_uart_base;
	//FIXME: For a0, UART 1 movded from sm to soc
	//for bringup, we just comment it out
	//else if (channel == 1) base = SM_APB_UART1_BASE;
	REG_READ32(base+APB_UART_LSR, &status);
	if (status & BIT(0))
		return 1;
	else
		return 0;
}

void APB_UART_puts(UINT32 channel, const char *str)
{
	while (*str) {
		char c = *str;
		APB_UART_putc(channel, c);
		if (c == '\n'){
			APB_UART_putc(channel, '\r');
		}
		str++;
	}
}

void set_uart_pinmux()
{
#if defined(BCM_EN)

#else
	T32smSysCtl_SM_GSM_SEL reg;
	REG_READ32(SM_SYS_CTRL_REG_BASE + RA_smSysCtl_SM_GSM_SEL, &(reg.u32));

	/*
	 *Remove other group settings, and only ENABLE UART0 RXD & TXD (GSM4 mode 0).
	 *Note: It should sync with global pinmux configurations in bootloader.
	 *      And currently, SM uses UART 0 too.
	 */
	reg.uSM_GSM_SEL_GSM4_SEL = 0;

	REG_WRITE32(SM_SYS_CTRL_REG_BASE + RA_smSysCtl_SM_GSM_SEL, (reg.u32));
#endif
}

#define UART_DLL_25M_115200 	0x0E
#define UART_DLH_25M_115200 	0x00
#define UART_DLL_36M_115200 	0x14
#define UART_DLH_36M_115200 	0x00
#define UART_DLL_40M_115200 	0x16
#define UART_DLH_40M_115200 	0x00
#define UART_DLL_48M_115200 	0x1A
#define UART_DLH_48M_115200 	0x00
#define UART_DLL				UART_DLL_25M_115200
#define UART_DLH				UART_DLH_25M_115200

void init_uart_base(UNSG32 uart_base)
{
	g_uart_base = uart_base;
}

void init_dbg_port(UNSG32 uart_base)
{
	UINT32 read;
	g_uart_base = uart_base;

	// check uart
	REG_READ32((uart_base+0x0c),&read);          // LCR
	read |=0x83;                                         // DLAB bit 7; 8 bit data latch
	REG_WRITE32( (uart_base+0x0c),read);         // enable DLAB
	// program DLL and DLH
	REG_WRITE32( (uart_base+0x00),UART_DLL);     // DLL 5M->19200 baudrate  -->Eswar
	REG_WRITE32( (uart_base+0x04),UART_DLH);     // DLH
	read &=0x7F;
	REG_WRITE32( (uart_base+0x0c),read);         // disable DLAB

#if UART_BAUD_SETTING_DELAY
	/*
	 *this delay is needed for baudrate setting after the chip is coming out of reset
	 *without the delay, the UART is not functioning the first time and only works
	 *after the second time the baudrate is programmed.
	 */
	for (dump = 10000; dump > 0; dump--);
#endif

	REG_WRITE32( (uart_base+0x08),0x01);        // fifo mode
	REG_READ32((uart_base+0x10),&read);

	read &=0xFFFFFFEF;
	REG_WRITE32( (uart_base+0x10),read);

	REG_READ32((uart_base+0x10),&read);

	REG_READ32((uart_base+0x7c),&read);         // read USR status;0x06
	REG_READ32((uart_base+0x80),&read);         // TFL ; 0x00
}


int dbg_port_rx_ready()
{
	unsigned int read;
	REG_READ32((g_uart_base+0x7c),&read);      // status
	if(read&0x08)
	{
		REG_READ32((g_uart_base+0x84),&read);  // RFL
		if (read>0)
		return 1;
	}
	return 0;
}

unsigned char get_char()
{
	unsigned int data32;
	unsigned char data8;
	REG_READ32((g_uart_base+0x00), &data32);

	data8=data32&0xFF;
	return data8;

}

#define PUT_CHAR(ch)  REG_WRITE32((g_uart_base+0x00), (unsigned int)ch)

//////////////////////////////////////////////////////
// read command via UART (RS232)
#define CR_CHAR		13
#define LF_CHAR		10
#define BS_CHAR		8
#define UP_CHAR		'.'
#define DIR1_CHAR	0x1B
#define DIR2_CHAR	'['
#define UP3_CHAR	'A'
#define DOWN3_CHAR	'B'

int getline(char *pCmd, int size)
{
   unsigned char ch;
   SIGN32 i = 0;
//   SIGN32 comment = 0;
   unsigned char arrow = 0;
   int	input_cmd_line_length = 0;


   for (i = 0; i < size; i++)
		pCmd[i] = 0;

	i = 0;
	input_cmd_line_length = 0;
	while (1)
	{
		while (dbg_port_rx_ready() == 0);
		ch = get_char();
		arrow += 2;
		input_cmd_line_length++;
		if(input_cmd_line_length > size){
			//dbg_printf(PRN_ERR, "cmd line overflow, please input the command line again\n") ;
			return 1;
		}

		switch (ch)
		{
			case CR_CHAR:
			case LF_CHAR:
			case '\0':
				pCmd[i] = '\0';
				return 0 ;

			default:
				pCmd[i] = ch;
				PUT_CHAR('*');
				break;
		}
		i++;
	}
}

