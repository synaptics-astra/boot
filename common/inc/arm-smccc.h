/*
 * Copyright 2020 Carlo Caione <ccaione@baylibre.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ARCH_ARM64_SMCCC_H_
#define _ARCH_ARM64_SMCCC_H_

/*
 * Result from SMC/HVC call
 * @a0-a7 result values from registers 0 to 7
 */
struct arm_smccc_res {
	unsigned long a0;
	unsigned long a1;
	unsigned long a2;
	unsigned long a3;
	unsigned long a4;
	unsigned long a5;
	unsigned long a6;
	unsigned long a7;
};

typedef struct arm_smccc_res arm_smccc_res_t;

/*
 * @brief Make SMC calls
 *
 * @param a0 function identifier
 * @param a1-a7 parameters registers
 * @param res results
 */
void arm_smccc_smc(unsigned long a0, unsigned long a1,
		   unsigned long a2, unsigned long a3,
		   unsigned long a4, unsigned long a5,
		   unsigned long a6, unsigned long a7,
		   struct arm_smccc_res *res);

#endif /* _ARCH_ARM64_SMCCC_H_ */
