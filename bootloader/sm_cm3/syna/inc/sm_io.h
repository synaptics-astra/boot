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
#ifndef __SM_IO_H
#define __SM_IO_H


/************************************************************
    register access
************************************************************/

#define MV_SM_READ_REG32( reg )		( *( (volatile unsigned int *)( reg ) ) )
#define MV_SM_READ_REG8( reg )		( *( (volatile unsigned char *)( reg ) ) )
#define MV_SM_WRITE_REG32( reg, value )	( *( (volatile unsigned int *)( reg ) ) = (unsigned int)( value ) )
#define MV_SM_WRITE_REG8( reg, value )	( *( (volatile unsigned char *)( reg ) ) = (unsigned char)( value ) )

#define BFM_HOST_Bus_Write32(offset, val)       ((*(volatile unsigned int*)(offset))=val)
#define BFM_HOST_Bus_Read32(offset, holder)     ((*(volatile unsigned int*)(holder))=(*(volatile unsigned int*)(offset)))
#define BFM_HOST_Bus_Write16(offset, val)       ((*(volatile unsigned short*)(offset))=val)
#define BFM_HOST_Bus_Read16(offset, holder)     ((*(volatile unsigned short*)(holder))=(*(volatile unsigned short*)(offset)))
#define BFM_HOST_Bus_Write8(offset, val)        ((*(volatile unsigned char*)(offset))=val)
#define BFM_HOST_Bus_Read8(offset, holder)      ((*(volatile unsigned char*)(holder))=(*(volatile unsigned char*)(offset)))

#endif
