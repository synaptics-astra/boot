/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright � 2013-2026 Synaptics Incorporated. All rights reserved.
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

 #ifndef __SM_GPIO_WAKEUP_H
 #define __SM_GPIO_WAKEUP_H

#define DEFAULT_THOLD   (0xFFFFFFFF)

 typedef struct {
    const char *name;
    int port;
    unsigned long threashold0;
    unsigned long threashold1;
    int wakeup_source;
    int has_payload;
    unsigned int payload;
} gpio_wakeup_desc_t;

/*
 * Convenience macro to declare a gpio wakeup
 */
#define DECLARE_GPIO_WAKEUP(_name, _port, _t0, _t1, _src, _has, _payload) \
	static const gpio_wakeup_desc_t __gpio_wakeup_ ## _name \
		__attribute__((__section__("gpio_wakeup_descs"))) __attribute__((__used__)) = { \
            .name = #_name, \
			.port = _port, \
			.threashold0 = _t0, \
            .threashold1 = _t1, \
			.wakeup_source = _src, \
            .has_payload = _has, \
            .payload = _payload}

extern gpio_wakeup_desc_t __gpio_wakeup_descs_start__[];
extern gpio_wakeup_desc_t __gpio_wakeup_descs_end__[];

#define GPIO_WAKEUP_DESCS_START	(__gpio_wakeup_descs_start__)
#define GPIO_WAKEUP_DESCS_END	(__gpio_wakeup_descs_end__)

#define GPIO_WAKEUP_DESCS_NUM		(GPIO_WAKEUP_DESCS_END - GPIO_WAKEUP_DESCS_START)

void sm_gpio_isr(int port);

#endif /* __SM_GPIO_WAKEUP_H */
