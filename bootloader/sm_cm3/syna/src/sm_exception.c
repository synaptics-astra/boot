/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright � 2013-2018 Synaptics Incorporated. All rights reserved.
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
#include "sm_type.h"
#include "sm_io.h"
#include "sm_memmap.h"

#include "sm_apb_ictl_defs.h"

#include "sm_exception.h"

#define ISR_FUNC_START	(&__isr_descs_start__)
#define ISR_FUNC_END	(&__isr_descs_end__)

static isr_desc_t * isr_descs;

#define ISR_FUNC_DESCS_NUM		(((unsigned int)ISR_FUNC_END - (unsigned int)ISR_FUNC_START)\
					/ sizeof(isr_desc_t))


static int is_isr_happen(unsigned low, unsigned high, unsigned int index)
{
	if(index >= SMICTL_IRQ_MAX)
		return S_FALSE;

	if(index < 32) {
		if(low & (1 << index))
			return S_OK;
	} else {
		if(high & (1 << (index - 32)))
			return S_OK;
	}
	return S_FALSE;
}

void IRQ_0_Handler(void)
{
}

void IRQ_1_Handler(void)
{
	int i = 0;
	unsigned int irqs_low;
    unsigned int irqs_high;
	isr_desc_t * isr_desc = NULL;

	// find active handle
    BFM_HOST_Bus_Read32(APB_ICTL_IRQ_FINALSTATUS_L, &irqs_low);
    BFM_HOST_Bus_Read32(APB_ICTL_IRQ_FINALSTATUS_H, &irqs_high);

	isr_descs = (isr_desc_t *)ISR_FUNC_START;
	for(i = 0; i < ISR_FUNC_DESCS_NUM; i++) {
		isr_desc = &isr_descs[i];
		if(S_OK == is_isr_happen(irqs_low, irqs_high, isr_desc->index)) {
			if(isr_desc->isr_handler != NULL) {
				isr_desc->isr_handler();
			}
		}
	}
}
