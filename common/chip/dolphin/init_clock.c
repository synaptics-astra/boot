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
#include "memmap.h"
#include "global.h"
#include "soc.h"
#include "io.h"
#include "debug.h"
#include "cpu_wrp.h"
#include "timer.h"

#define NO_SSC			{0,0,0,0,0,0,0,0}

#define CPUPLL_1600	 	{NO_SSC, 0x12220017, 0x0f008065, 0x00048b00}
#define CPUPLL_1500	 	{NO_SSC, 0x1221e017, 0x0f008064, 0x00048b00}
#define CPUPLL_1200	 	{NO_SSC, 0x12218017, 0x0f008062, 0x00048b00}
#define CPUPLL_1000	 	{NO_SSC, 0x12214017, 0x0f008061, 0x00048b00}
#define CPUPLL_800	 	{NO_SSC, 0x12220017, 0x0f0100a5, 0x00048b00}

struct ssc_setting
{
	unsigned int en		:1;
	unsigned int down	:1;	//down spread not center spread
	unsigned int fbdiv	:9;
	unsigned int kvco	:4;
	unsigned int div_sel	:3;
	unsigned int intpi	:4;
	unsigned int freq_div	:16;
	unsigned int rnge	:11;
};

struct pll_setting
{
	struct ssc_setting 	ssc;
	unsigned int 		ctrl;
	unsigned int 		ctrl1;
	unsigned int 		ctrl2;
};

#if (CPUPLL == 800)
const struct pll_setting cpupll_setting = CPUPLL_800;
#elif (CPUPLL == 1000)
const struct pll_setting cpupll_setting = CPUPLL_1000;
#elif (CPUPLL == 1200)
const struct pll_setting cpupll_setting = CPUPLL_1200;
#elif (CPUPLL == 1500)
const struct pll_setting cpupll_setting = CPUPLL_1500;
#elif (CPUPLL == 1600)
const struct pll_setting cpupll_setting = CPUPLL_1600;
#elif (CPUPLL == 1800)
// FIXME: correct pll settings of 1.8GHz
const struct pll_setting cpupll_setting = CPUPLL_1600;
#else
#error "cpupll setting error."
#endif

#define CPUPLL_CTRL_BASE	(MEMMAP_CA7_REG_BASE + RA_CPU_WRP_PLL_REG)

void init_cpuPll(void)
{

}
