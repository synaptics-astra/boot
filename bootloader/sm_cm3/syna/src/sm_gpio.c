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
#include "sm_type.h"
#include "sm_io.h"

#include "sm_timer.h"
#include "sm_comm.h"

#include "sm_printf.h"
#include "sm_common.h"

#include "sm_memmap.h"
#include "sm_apb_gpio_defs.h"

#include "sm_rt_module.h"
#include "sm_exception.h"
#include "sm_gpio.h"

unsigned int g_board_version = 0xffff;

static int gpio_getmap(int *port, unsigned int *reg_base)
{
	if ((*port >= 0) && (*port < 32)) {
		*reg_base = 0;
	} else if ((*port >= 32) && (*port < 64)) {
		*reg_base = 1;
		*port -= 32;
	} else
		return S_FALSE;

	return S_OK;
}

/*
FUNCTION: output 1 bit from SM GPIO port
INPUT:
		port: SM GPIO port
		value:output value
*/
int sm_gpio_set_output(int port, int value)
{
	unsigned int tmp = 0, reg_base = 0;

	if (gpio_getmap(&port, &reg_base) == S_FALSE)
		return S_FALSE;

	tmp = MV_SM_READ_REG32(GPIO_SWPORTA_DR(reg_base));

	if (value)
		tmp |= (1 << port);
	else
		tmp &= ~(1 << port);

	MV_SM_WRITE_REG32(GPIO_SWPORTA_DR(reg_base), tmp);

	tmp = MV_SM_READ_REG32(GPIO_SWPORTA_DDR(reg_base));
	tmp |= (1 << port);
	MV_SM_WRITE_REG32(GPIO_SWPORTA_DDR(reg_base), tmp);

	tmp = MV_SM_READ_REG32(GPIO_SWPORTA_DR(reg_base));
	if (value)
		tmp |= (1 << port);
	else
		tmp &= ~(1 << port);

	MV_SM_WRITE_REG32(GPIO_SWPORTA_DR(reg_base), tmp);
	return S_OK;
}

int sm_gpio_set_input(int port)
{
	unsigned int tmp = 0, reg_base = 0;

	if (gpio_getmap(&port, &reg_base) == S_FALSE)
		return S_FALSE;

	tmp = MV_SM_READ_REG32(GPIO_SWPORTA_DDR(reg_base));
	tmp &= ~(1 << port);
	MV_SM_WRITE_REG32(GPIO_SWPORTA_DDR(reg_base), tmp);

	return S_OK;
}

int sm_gpio_read(int port)
{
	unsigned int tmp = 0, reg_base = 0;

	if (gpio_getmap(&port, &reg_base) == S_FALSE)
		return -1;

	tmp = MV_SM_READ_REG32(GPIO_EXT_PORTA(reg_base));

	return ((tmp >> port) & 0x1);
}

void sm_gpio_set_int(int gpio_port,
			 sm_gpio_inttype_level inttype, sm_gpio_int_polarity polarity)
{
	INT32 tmp = 0;
	UINT32 reg_base = 0;

	if (gpio_getmap(&gpio_port, &reg_base) == S_FALSE)
		return;

	// set gpio data direction
	tmp = MV_SM_READ_REG32(GPIO_SWPORTA_DDR(reg_base));
	tmp &= (~(1 << gpio_port));
	MV_SM_WRITE_REG32(GPIO_SWPORTA_DDR(reg_base), tmp);

	// enable interrupt
	tmp = MV_SM_READ_REG32(GPIO_INTEN(reg_base));
	tmp |= (1 << gpio_port);
	MV_SM_WRITE_REG32(GPIO_INTEN(reg_base), tmp);

	// interrupt type setting
	tmp = MV_SM_READ_REG32(GPIO_INTTYPE_LEVEL(reg_base));
	if (inttype == 0)
		tmp &= (~(1 << gpio_port));
	else
		tmp |= (1 << gpio_port);
	MV_SM_WRITE_REG32(GPIO_INTTYPE_LEVEL(reg_base), tmp);

	//polarity
	tmp = MV_SM_READ_REG32(GPIO_INT_POLARITY(reg_base));
	if (polarity == 0)
		tmp &= (~(1 << gpio_port));
	else
		tmp |= (1 << gpio_port);
	MV_SM_WRITE_REG32(GPIO_INT_POLARITY(reg_base), tmp);

	tmp = MV_SM_READ_REG32(GPIO_DEBOUNCE(reg_base));
	tmp |= (1 << gpio_port);
	MV_SM_WRITE_REG32(GPIO_DEBOUNCE(reg_base), tmp);

	// clear mask
	MV_SM_WRITE_REG32(GPIO_INTMASK(reg_base), 0x00);
}

void sm_gpio_set_polarity(int gpio_port, sm_gpio_int_polarity polarity)
{
	int tmp = 0;
	unsigned int reg_base = 0;

	if (gpio_getmap(&gpio_port, &reg_base) == S_FALSE)
		return;

	tmp = MV_SM_READ_REG32(GPIO_INT_POLARITY(reg_base));
	if (polarity == 0)
		tmp &= (~(1 << gpio_port));
	else
		tmp |= (1 << gpio_port);
	MV_SM_WRITE_REG32(GPIO_INT_POLARITY(reg_base), tmp);
}

unsigned int sm_gpio_get_polarity(int gpio_port)
{
	unsigned int tmp = 0;
	unsigned int reg_base = 0;

	if (gpio_getmap(&gpio_port, &reg_base) == S_FALSE)
		return -1;

	tmp = MV_SM_READ_REG32(GPIO_INT_POLARITY(reg_base));
	tmp &= (0x1 << gpio_port);

	return tmp;
}

void sm_gpio_set_inttype(int gpio_port, sm_gpio_inttype_level inttype)
{
	int tmp = 0;
	unsigned int reg_base = 0;

	if (gpio_getmap(&gpio_port, &reg_base) == S_FALSE)
		return;

	tmp = MV_SM_READ_REG32(GPIO_INTTYPE_LEVEL(reg_base));
	if (inttype == 0)
		tmp &= (~(1 << gpio_port));
	else
		tmp |= (1 << gpio_port);
	MV_SM_WRITE_REG32(GPIO_INTTYPE_LEVEL(reg_base), tmp);
}

void sm_gpio_set_debounce(int gpio_port, sm_gpio_debounce deb)
{
	int tmp = 0;
	unsigned int reg_base = 0;

	if (gpio_getmap(&gpio_port, &reg_base) == S_FALSE)
		return;

	tmp = MV_SM_READ_REG32(GPIO_DEBOUNCE(reg_base));
	if (deb == DEBOUNCE_DISABLE)
		tmp &= (~(1 << gpio_port));
	else
		tmp |= (1 << gpio_port);
	MV_SM_WRITE_REG32(GPIO_DEBOUNCE(reg_base), tmp);
}

void sm_gpio_int_enable(int gpio_port)
{
	int tmp = 0;
	unsigned int reg_base = 0;

	if (gpio_getmap(&gpio_port, &reg_base) == S_FALSE)
		return;

	tmp = MV_SM_READ_REG32(GPIO_INTEN(reg_base));
	tmp |= (1 << gpio_port);
	MV_SM_WRITE_REG32(GPIO_INTEN(reg_base), tmp);
}

void sm_gpio_int_disable(int gpio_port)
{
	int tmp = 0;
	unsigned int reg_base = 0;

	if (gpio_getmap(&gpio_port, &reg_base) == S_FALSE)
		return;

	tmp = MV_SM_READ_REG32(GPIO_INTEN(reg_base));
	tmp &= (~(1 << gpio_port));
	MV_SM_WRITE_REG32(GPIO_INTEN(reg_base), tmp);
}

void sm_gpio_clearint(int gpio_port)
{
	unsigned int reg_base = 0;

	if (gpio_getmap(&gpio_port, &reg_base) == S_FALSE)
		return;

	MV_SM_WRITE_REG32(GPIO_PORTA_EOI(reg_base), (1 << gpio_port));
}

static int __attribute__((used)) sm_process_gpio_msg(void *data, int len)
{
	int *msg_content = (INT32 *) data;
	int reg;

	switch (*msg_content) {
	case MV_SM_GPIO_SET:
		if (sm_gpio_set_output(msg_content[1], msg_content[2]) ==
		    S_FALSE) {
			PRT_INFO("GPIO port %d not exist. Write error.\n",
				 msg_content[1]);
		} else {
			PRT_INFO("io[%d]->%d.\n", msg_content[1],
				 msg_content[2]);
		}
		break;
	case MV_SM_GPIO_GET:
		reg = sm_gpio_read(msg_content[1]);
		if (reg == -1) {
			PRT_INFO("GPIO port %d not exist. Read error.\n",
				 msg_content[1]);
		} else {
			sm_send_msg(MV_SM_ID_SYS, (unsigned char *)(&reg), sizeof(reg));
		}
		break;
	case MV_SM_BOARDVER_GET:
		g_board_version = msg_content[1];
		PRT_INFO("MV_SM_BOARDVER_GET: board_version( 0x%2x)\n", g_board_version);
		break;
	default:
		PRT_DEBUG("receive old misc message.\n");
		break;
	}

	return S_OK;
}

DECLARE_RT_MODULE(
	gpio,
	MV_SM_ID_GPIO,
	NULL,
	sm_process_gpio_msg,
	NULL
);

static int __attribute__((used)) setup_gpio_init(void)
{
	int i = 0;
	int gpioisrnum = ((unsigned int)(&__gpio_isr_descs_end__)
		- (unsigned int)(&__gpio_isr_descs_start__))
		/ sizeof(gpio_isr_desc_t);

	int gpio_port;
	sm_gpio_inttype_level inttype;
	sm_gpio_int_polarity polarity;

	gpio_isr_desc_t * gpio_isr_desc = (gpio_isr_desc_t *)(&__gpio_isr_descs_start__);

	// clear mask
	MV_SM_WRITE_REG32(GPIO_INTMASK(0), 0x00);
	// falling edge sensitive by default
	MV_SM_WRITE_REG32(GPIO_INT_POLARITY(0), 0x00);

	// initialize the gpio int
	for(i = 0;i < gpioisrnum; i++) {
		gpio_port = gpio_isr_desc[i].port;
		inttype = gpio_isr_desc[i].inttype;
		polarity = gpio_isr_desc[i].polarity;
		sm_gpio_set_int(gpio_port, inttype, polarity);
	}

	return S_OK;
}

DECLARE_RT_INIT(gpioisr, INIT_DEV_P_25, setup_gpio_init);

#define GPIO_ISR_START	(&__gpio_isr_descs_start__)
#define GPIO_ISR_END	(&__gpio_isr_descs_end__)

static gpio_isr_desc_t * gpio_isr_descs;

#define GPIO_ISR_DESCS_NUM		(((unsigned int)GPIO_ISR_END - (unsigned int)GPIO_ISR_START)\
					/ sizeof(gpio_isr_desc_t))


static int is_gpio_isr_happen(unsigned int intstatus, unsigned int port)
{
	if(port < 32) {
		if(intstatus & (1 << port))
			return S_OK;
	}
	return S_FALSE;
}

static void __attribute__((used)) sm_gpio_isr(void)
{
	int i = 0;
	gpio_isr_desc_t * gpio_isr_desc = NULL;

	unsigned int gpio_intstatus = MV_SM_READ_REG32(GPIO_INTSTATUS(0));

	gpio_isr_descs = (gpio_isr_desc_t *)GPIO_ISR_START;
	for(i = 0; i < GPIO_ISR_DESCS_NUM; i++) {
		gpio_isr_desc = &gpio_isr_descs[i];
		if(is_gpio_isr_happen(gpio_intstatus, gpio_isr_desc->port) == S_OK) {
			if(gpio_isr_desc->gpio_isr != NULL) {
				gpio_isr_desc->gpio_isr();
			}
		}
	}

	// clear all gpio interrupt
	MV_SM_WRITE_REG32(GPIO_PORTA_EOI(0), gpio_intstatus);
}

DECLARE_ISR(SMICTL_IRQ_SM_GPIO0, sm_gpio_isr);
