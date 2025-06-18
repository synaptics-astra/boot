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
#ifndef __APB_UART_DRV_H__
#define __APB_UART_DRV_H__

#include "com_type.h"
#include "apb_uart.h"

/*******************************************************************************
* Function:    APB_UART_init
*
* Description: initialize UART
*
* Inputs:      channel-- choose UART0 or UART1, 0 means UART0, 1 means UART1
*			   tclk-- UART source clock
*			   baud-- UART baud rate
* Outputs:     none
*
* Return:      none
*******************************************************************************/
void APB_UART_init(UINT32 channel, UINT32 tclk, UINT32 baud);

/*******************************************************************************
* Function:    APB_UART_putc
*
* Description: initialize UART
*
* Inputs:      channel-- choose UART0 or UART1, 0 means UART0, 1 means UART1
*			   c-- output character
* Outputs:     none
*
* Return:      none
*******************************************************************************/
void APB_UART_putc(UINT32 channel, CHAR ch);

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
UCHAR APB_UART_tstc(UINT32 channel);

/*******************************************************************************
* Function:    APB_UART_getc_nonblock
*
* Description: non blocking mode read charactor from UART
*
* Inputs:      channel-- choose UART0 or UART1, 0 means UART0, 1 means UART1
*			   c-- output character
* Outputs:     none
*
* Return:      0-- didn't get any character
*			   1-- get one character
*******************************************************************************/
UCHAR APB_UART_getc_nonblock(UINT32 channel, UCHAR *ch);

/*******************************************************************************
* Function:    APB_UART_getc
*
* Description: blocking mode read charactor from UART. Wait until get something
*
* Inputs:      channel-- choose UART0 or UART1, 0 means UART0, 1 means UART1
* Outputs:     none
*
* Return:      return UIN8 value.
*******************************************************************************/
UCHAR APB_UART_getc(UINT32 channel);

/*******************************************************************************
* Function:    APB_UART_write
*
* Description: Write the buffer into UART.
*
* Inputs:      channel-- choose UART0 or UART1, 0 means UART0, 1 means UART1
*			   buf-- pointer point to the string what will be written to UART
*			   len-- length of the string
* Outputs:     none
*
* Return:      none.
*******************************************************************************/
void APB_UART_write(UINT32 channel, CHAR *buf);

/*******************************************************************************
* Function:    APB_UART_read
*
* Description: read the string from UART.
*
* Inputs:      channel-- choose UART0 or UART1, 0 means UART0, 1 means UART1
*			   buf-- pointer point to the string what will be written to UART
*			   len-- length of the string
* Outputs:     none
*
* Return:      none.
*******************************************************************************/
void APB_UART_read(UINT32 channel, CHAR *buf, UINT32 len);

/*******************************************************************************
* Function:    APB_UART_printf
*
* Description: printf sytle output.print the output to UART
*
* Inputs:      channel-- choose UART0 or UART1, 0 means UART0, 1 means UART1
*			   szFormat-- print format
* Outputs:     none
*
* Return:      none
*******************************************************************************/
void APB_UART_printf(UINT32 channel, UCHAR* szFormat, ...);

int dbg_port_rx_ready();

unsigned char get_char();

int getline(char *pCmd, int size);

#endif //__APB_UART_DRV_H__
