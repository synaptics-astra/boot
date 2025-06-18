/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __ARCH_SYS_REG_OP__
#define __ARCH_SYS_REG_OP__

typedef unsigned long u_register_t;

#define SCTLR_M_BIT		(1 << 0)
#define SCTLR_C_BIT		(1 << 2)
#define SCTLR_I_BIT		(1 << 12)

/**********************************************************************
 * Macros which create inline functions to read or write CPU system
 * registers
 *********************************************************************/

#define _DEFINE_SYSREG_READ_FUNC(_name, _reg_name)		\
static inline u_register_t read_ ## _name(void)			\
{								\
	u_register_t v;						\
	__asm__ volatile ("mrs %0, " #_reg_name : "=r" (v));	\
	return v;						\
}

#define _DEFINE_SYSREG_WRITE_FUNC(_name, _reg_name)			\
static inline void write_ ## _name(u_register_t v)			\
{									\
	__asm__ volatile ("msr " #_reg_name ", %0" : : "r" (v));	\
}

/* Define read function for system register */
#define DEFINE_SYSREG_READ_FUNC(_name) 			\
	_DEFINE_SYSREG_READ_FUNC(_name, _name)


/* Define read & write function for system register */
#define DEFINE_SYSREG_RW_FUNCS(_name)			\
		_DEFINE_SYSREG_READ_FUNC(_name, _name)		\
		_DEFINE_SYSREG_WRITE_FUNC(_name, _name)

/*Get current EL*/
DEFINE_SYSREG_READ_FUNC(CurrentEl)

/*Get/Set sctlr*/
DEFINE_SYSREG_RW_FUNCS(sctlr_el1)
DEFINE_SYSREG_RW_FUNCS(sctlr_el2)
DEFINE_SYSREG_RW_FUNCS(sctlr_el3)

unsigned int get_current_el(void) \
{
	u_register_t el;

	el = read_CurrentEl();

	return el >> 2;
}

unsigned long get_sctlr_reg(void)
{
	u_register_t el, val;

	el = get_current_el();

	switch(el) {
	case 1:
		val = read_sctlr_el1();
		break;
	case 2:
		val = read_sctlr_el2();
		break;
	default:
		val = read_sctlr_el3();
		break;
	}

	return val;
}

void set_sctlr_reg(unsigned long val)
{
	unsigned int el;

	el = get_current_el();
	switch(el) {
	case 1:
		write_sctlr_el1(val);
		break;
	case 2:
		write_sctlr_el2(val);
		break;
	default:
		write_sctlr_el3(val);
		break;
	}

	asm volatile("isb");
}

#endif // __ARCH_SYS_REG_OP__

