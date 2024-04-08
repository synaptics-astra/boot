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
#ifndef __APB_UART_H__
#define __APB_UART_H__

#define APB_UART_RBR		0x00
#define APB_UART_THR		0x00
#define APB_UART_DLL		0x00
#define APB_UART_DLH		0x04
#define APB_UART_IER		0x04
#define APB_UART_IIR		0x08
#define APB_UART_FCR		0x08
#define APB_UART_LCR		0x0C
#define APB_UART_MCR		0x10
#define APB_UART_LSR		0x14
#define APB_UART_MSR		0x18
#define APB_UART_SCR		0x1C
#define APB_UART_LPDLL		0x20
#define APB_UART_LPDLH		0x24
#define APB_UART_SRBR		0x30 //	0x30-0X6C
#define APB_UART_STHR		0x30 //	0x30-0X6C
#define APB_UART_FAR		0x70
#define APB_UART_TFR		0x74
#define APB_UART_RFW		0x78
#define APB_UART_USR		0x7C
#define APB_UART_TFL		0x80
#define APB_UART_RFL		0x84
#define APB_UART_SRR		0x88
#define APB_UART_SRTS		0x8C
#define APB_UART_SBCR		0x90
#define APB_UART_SDMAM		0x94
#define APB_UART_SFE		0x98
#define APB_UART_SRT		0x9C
#define APB_UART_STET		0xA0
#define APB_UART_HTX		0xA4
#define APB_UART_DMASA		0xA8
// RESERVED			0xAC- 0XF0
#define APB_UART_CPR		0xF4
#define APB_UART_UCV		0xF8
#define APB_UART_CTR		0xFC

#endif //__APB_ICTL_H__

