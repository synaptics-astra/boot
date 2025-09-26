/*
 * Copyright (c) 2019 Carlo Caione <ccaione@baylibre.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * This file implements the common calling mechanism to be used with the Secure
 * Monitor Call (SMC) and Hypervisor Call (HVC).
 *
 * See https://developer.arm.com/docs/den0028/latest
 */

#include "config.h"
#include "asm.h"
#include "asm_macro.h"

.macro SMCCC instr
	\instr  #0
	ldr     x4, [sp]
	stp     x0, x1, [x4, #0]
	stp     x2, x3, [x4, #16]
	stp     x4, x5, [x4, #32]
	stp     x6, x7, [x4, #48]
	ret
.endm

/*
 * The SMC instruction is used to generate a synchronous exception that is
 * handled by Secure Monitor code running in EL3.
 */
ENTRY(arm_smccc_smc)
	SMCCC	smc
ENDPROC(arm_smccc_smc)
