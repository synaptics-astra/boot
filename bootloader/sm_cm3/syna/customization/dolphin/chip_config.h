/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013~2023 Synaptics Incorporated. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 or
 * later as published by the Free Software Foundation.
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
#ifndef __CHIP_CONFIG_H__
#define __CHIP_CONFIG_H__


#define SM_SYSTEM_HZ 25

/*****************************************************************************/
/* define modules here */
/*****************************************************************************/

/*************************************/
/* overheat protection function */
/*************************************/
#ifdef TEMPENABLE

#ifdef OVER_HEAT_TEMPERATURE
//define the threshold of temperature here
#define OVERHEATHRESHOLD  110
#define PVT_BASE (SM_MEMMAP_SMREG_BASE + RA_smSysCtl_TSEN)
#endif

#endif

/*************************************/
/* wakeup online function */
/*************************************/

/*************************************/
/* watchdog function */
/*************************************/
#ifdef WDTENABLE
/* timeout = (2^(16 + i) -1) /(xx_SYSTEM_HZ * 1000).
  * i from 0 to 15.
  * SM (clock 25Mhz)
  * 0: 2.6214 ms; 1: 5.2428 ms; 2; 10.48 ms;
  * 3: 20.97 ms; 4: 41.94 ms; 5: 83.89 ms;
  * 6: 167.77 ms; 7: 335.54 ms; 8: 671.09 ms;
  * 9: 1.342 s; 10: 2.684 s; 11: 5.369 s;
  * 12: 10.737 s; 13: 21.475 s; 14: 42.950 s;
  * 15: 85.899 s
*/
// ms
#define WDT0TIMEOUT	(11)
#define WDT1TIMEOUT	(12)
#endif


/*************************************/
/* IR function */
/*************************************/
#ifdef IRENABLE

#define SM_GPIO_PORT_IR 2

#define IR_CBUFF_LENGTH	256//if all tasks is nonblocking, the buffer could be decreased further	// IR_CBUFF_LENGTH must be power of 2
#define CIRC_BUFF_MASK	( IR_CBUFF_LENGTH - 1 )

#define	SM_IR_Flavor	0x3
#define IR_HOLDKEY_SEND	(SM_IR_Flavor&0x1)
#define IR_UPKEY_SEND	((SM_IR_Flavor>>1)&0x1)

#endif /* IRENABLE */

/*************************************/
/* default standby type */
/*************************************/
#define DEFAULT_STANDBY_REQUEST_TYPE	MV_SM_POWER_NORMALSTANDBY_REQUEST

/*************************************/
/* sm mini shell support */
/*************************************/
//#define	SMSHELLENABLE


/*************************************/
/* twsi support */
/*************************************/
#endif /* __CHIP_CONFIG_H__ */
