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
#ifndef __PV_COMP_H__
#define __PV_COMP_H__

#include "com_type.h"

typedef struct dvfs_ops {
	int (* get_vcpu_volt)(void);
	int (* get_vcore_volt)(void);
	int (* set_vcpu_volt)(int from, int to);
	int (* set_vcore_volt)(int from, int to);
	int (* init)(void);
}dvfs_ops_t;


extern const dvfs_ops_t m88pg86x_ops;
extern const dvfs_ops_t sy8824b_ops;
extern const dvfs_ops_t sy8827n_ops;
extern const dvfs_ops_t ncp6335d_ops;
extern const dvfs_ops_t mp886x_ops;
extern const dvfs_ops_t hl7593_ops;
extern const dvfs_ops_t TPS62868_ops;
extern const dvfs_ops_t rt5739_ops;
extern const dvfs_ops_t fp6359_ops;

int get_vcpu_opp(unsigned int *opp);

void get_vcpu_vh_vl(unsigned int *opp);
void get_vcore_vh_vl(unsigned int *opp);

void pv_comp(int cpu_pll, int vmeta_pll);

#endif /* __PV_COMP_H__ */
