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
#ifndef __APB_GPIO_H__
#define __APB_GPIO_H__

/*
 * The GPIO inst0 is located at (MEMMAP_APBPERIF_REG_BASE + 0x0C00)
 * i.e. 0xf7f30c00.
 */
#define APB_GPIO_SWPORTA_DR		0x00
#define APB_GPIO_SWPORTA_DDR		0x04
#define APB_GPIO_PORTA_CTL		0x08
#define APB_GPIO_SWPORTB_DR		0x0c
#define APB_GPIO_SWPORTB_DDR		0x10
#define APB_GPIO_PORTB_CTL		0x14
#define APB_GPIO_SWPORTC_DR		0x18
#define APB_GPIO_SWPORTC_DDR		0x1c
#define APB_GPIO_PORTC_CTL		0x20
#define APB_GPIO_SWPORTD_DR		0x24
#define APB_GPIO_SWPORTD_DDR		0x28
#define APB_GPIO_PORTD_CTL		0x2c
#define APB_GPIO_INTEN			0x30
#define APB_GPIO_INTMASK		0x34
#define APB_GPIO_INTTYPE_LEVEL		0x38
#define APB_GPIO_INT_POLARITY		0x3c
#define APB_GPIO_INTSTATUS		0x40
#define APB_GPIO_RAWINTSTATUS		0x44
#define APB_GPIO_DEBOUNCE		0x48
#define APB_GPIO_PORTA_EOI		0x4c
#define APB_GPIO_EXT_PORTA		0x50
#define APB_GPIO_EXT_PORTB		0x54
#define APB_GPIO_EXT_PORTC		0x58
#define APB_GPIO_EXT_PORTD		0x5c
#define APB_GPIO_LS_SYNC		0x60
#define APB_GPIO_ID_CODE		0x64
#define APB_GPIO_RESERVED		0x68
#define APB_GPIO_COMP_VERSION		0x6c

#endif
