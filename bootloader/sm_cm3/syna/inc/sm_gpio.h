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
#ifndef __SM_GPIO_H
#define __SM_GPIO_H

typedef enum
{
	LEVEL_SENSITIVE = 0,
	EDGE_SENSITIVE,
}sm_gpio_inttype_level;

typedef enum
{
	FALLING_EDGE = 0,
	RISING_EDGE,
	ACTIVE_LOW = 0,
	ACTIVE_HIGH,
}sm_gpio_int_polarity;

typedef enum
{
	DEBOUNCE_ENABLE = 0,
	DEBOUNCE_DISABLE = 1,
}sm_gpio_debounce;

/* Prototype for gpio isr function */
typedef void (* gpio_isr_func)(void);

typedef struct gpio_isr_desc {
	int port;						/* SM gpio port number */
	sm_gpio_inttype_level inttype;	/* 0 Level-sensitive; 1 Edge-sensitive; */
	sm_gpio_int_polarity polarity;	/* 0 Falling edge; 1 Rising edge; (0 Active-low; 1 Active-high;) */
	gpio_isr_func gpio_isr;
} gpio_isr_desc_t;

/*
 * Convenience macro to declare a service descriptor
 */
#define DECLARE_GPIO_ISR(_port, _inttype, _polarity, _gpio_isr) \
	static const gpio_isr_desc_t __gpio_isr_desc_ ## _port \
		__attribute__((__section__("gpio_isr_descs"))) __attribute__((__used__)) = { \
			.port = _port, \
			.inttype = _inttype, \
			.polarity = _polarity, \
			.gpio_isr = _gpio_isr, }

extern unsigned int __gpio_isr_descs_start__;
extern unsigned int __gpio_isr_descs_end__;

int sm_gpio_set_input(int port);
int sm_gpio_read(int port);
void sm_gpio_set_int(int gpio_port, sm_gpio_inttype_level inttype, sm_gpio_int_polarity polarity);
void sm_gpio_clearint(int gpio_port);
void sm_gpio_int_enable(int gpio_port);
void sm_gpio_int_disable(int gpio_port);
void sm_gpio_set_polarity(int gpio_port, sm_gpio_int_polarity polarity);
unsigned int sm_gpio_get_polarity(int gpio_port);
int sm_gpio_set_output(int port, int value);

#endif
