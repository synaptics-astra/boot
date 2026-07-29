/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright © 2013-2026 Synaptics Incorporated. All rights reserved.
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

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

#include "platform_config.h"

#include "sm_type.h"
#include "SysMgr.h"
#include "sm_common.h"

#include "sm_printf.h"
#include "sm_gpio.h"
#include "sm_state.h"
#include "sm_timer.h"

#include "sm_rt_module.h"
#include "sm_power.h"
#include "sm_func.h"
#include "sm_mem_map_itcm.h"
#include "sm_io.h"
#include "sm_memmap.h"

#include "sm_gpio_wakeup.h"

gpio_isr_desc_t *get_gpio_isr_desc(int port)
{
    gpio_isr_desc_t *gpio_isr_desc = (gpio_isr_desc_t *)GPIO_ISR_START;
    int i = 0;

    for (i = 0; i < GPIO_ISR_DESCS_NUM; i++) {
        if (gpio_isr_desc[i].port == port)
            return &gpio_isr_desc[i];
    }

    return NULL;
}

#define WIFIBT_STACK_SIZE  ((uint16_t) 128)
#define MAX_GAP    (1000)

#ifdef POWERKEY_2_ANDROID
typedef struct {
	unsigned int times;
	unsigned int level;
	BOOL is_btinfo;
} Itcm_bt_info;

static Itcm_bt_info *g_itcm = (Itcm_bt_info *)(uintptr_t)(SM_BT_INFO_BASE);

static void sm_trigger_irq(void)
{
	unsigned int reg;

	reg = MV_SM_READ_REG32(SM_MEMMAP_SYS_CTRL_BASE + RA_smSysCtl_SM_CTRL);
	reg |= 0x2; //enable sw interrupt
	MV_SM_WRITE_REG32(SM_MEMMAP_SYS_CTRL_BASE + RA_smSysCtl_SM_CTRL, reg);
}
#endif

static TimerHandle_t gpio_timer;
static SemaphoreHandle_t gpio_sem;
static unsigned long start_time;
static unsigned long gpio_gap;

static int current_port;

BOOL state_to_wait_wakeup(void)
{
	MV_SM_STATE cur_state = mv_sm_get_state();

	if((MV_SM_STATE_LOWPOWERSTANDBY == cur_state) ||
		(MV_SM_STATE_NORMALSTANDBY == cur_state) ||
		(MV_SM_STATE_SUSPEND == cur_state)) {
		return TRUE;
	}

	return FALSE;
}

void __attribute__((used)) sm_gpio_isr(int port)
{
    sm_gpio_clearint(port);

    current_port = port;
    //PRT_INFO("int from gpio %d\n", port);

    /* start a timer to caculate gap */
    if(gpio_gap == 0)
        start_time = mv_sm_timer_gettimems_formISR();

    if (gpio_sem != NULL) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(gpio_sem, &xHigherPriorityTaskWoken);
    }
}

static void sm_gpio_timer_isr(TimerHandle_t xtimer)
{
    gpio_isr_desc_t *gpioisrd = get_gpio_isr_desc(current_port);
    gpio_wakeup_desc_t *gpio_wakeup_desc = (gpio_wakeup_desc_t *)GPIO_WAKEUP_DESCS_START;;
    unsigned int edge = ((gpioisrd->polarity == FALLING_EDGE) ?  RISING_EDGE : FALLING_EDGE);
    int wakeup_source = -1;
    int i = 0;

	gpio_gap = mv_sm_timer_gettimems_formISR() - start_time;

    for (i = 0; i < GPIO_WAKEUP_DESCS_NUM; i++) {
        if (gpio_wakeup_desc[i].port == current_port) {
            if ((gpio_wakeup_desc[i].threashold1 == DEFAULT_THOLD) &&
                (gpio_gap > gpio_wakeup_desc[i].threashold0)) {
                break;
            } else if ((sm_gpio_read(current_port) == edge) &&
                        (gpio_gap > gpio_wakeup_desc[i].threashold0) &&
                        (gpio_gap <= gpio_wakeup_desc[i].threashold1)) {
                break;
            }
        }
    }

    if (i >= GPIO_WAKEUP_DESCS_NUM) {
        /* ignore the wrong signal */
        if (gpio_gap >= MAX_GAP) {
            if(xTimerStop(gpio_timer, 0) == pdFAIL)
                PRT_ERROR("wifibt timer can't be stopped\n");
            gpio_gap = 0;
        }
        return;
    }

    wakeup_source = gpio_wakeup_desc[i].wakeup_source;


    /* additional operation for BT */
    if (wakeup_source == MV_SM_WAKEUP_SOURCE_BT) {
#ifdef POWERKEY_2_ANDROID
        /* it's the exact BT key. +5 directly */
        g_itcm->times += 5;
        g_itcm->is_btinfo = TRUE;
        if (sm_gpio_read(current_port) == RISING_EDGE)
            g_itcm->level = 1;
        else
            g_itcm->level = 0;

        sm_trigger_irq();
#endif
    }

    if(xTimerStop(gpio_timer, 0) == pdFAIL)
        PRT_ERROR("wifibt timer can't be stopped\n");

    /* check power state after done the handle of bt info */
    if (!state_to_wait_wakeup()) {
        gpio_gap = 0;
        return;
    }

    PRT_INFO("%s gpio_gap: %d\n", gpio_wakeup_desc[i].name, gpio_gap);
    gpio_gap = 0;

    mv_sm_power_setwakeupsource(wakeup_source);
    if (gpio_wakeup_desc[i].has_payload)
        mv_sm_power_setwakeuppayload(gpio_wakeup_desc[i].payload);
    mv_sm_power_enterflow_bysmstate();
}

static int __attribute__((used)) gpio_wakeup_init(void)
{
#ifdef POWERKEY_2_ANDROID
    g_itcm->times = 0;
	g_itcm->level = 0;
	g_itcm->is_btinfo = FALSE;
#endif

	/* Create notify semaphore */
	gpio_sem = xSemaphoreCreateBinary();
	if (gpio_sem == NULL)
		PRT_ERROR("gpio_wakeup semaphore create failed\n");

	/* Crate Wifibt timer */
	if (gpio_timer == NULL) {
		// Create timer during system init
		gpio_timer = xTimerCreate("gpio_timer",
				pdMS_TO_TICKS(5), // 5ms
				pdTRUE,
				0,
				sm_gpio_timer_isr);
	}
	if (gpio_timer == NULL)
		PRT_ERROR("gpio_wakeup timer request failed\n");

	return S_OK;
}

DECLARE_RT_INIT(gpio_wakeup, INIT_DEV_P_25, gpio_wakeup_init);

static void gpio_wake_task(void *para)
{
	while (1) {
		xSemaphoreTake(gpio_sem, portMAX_DELAY);

		/* need to detect BT key in active mode */
		//if(state_to_wait_wakeup()) {
			if (gpio_timer != NULL) {
				if ( xTimerIsTimerActive(gpio_timer) == pdFALSE ) {
					if(xTimerStart(gpio_timer, pdMS_TO_TICKS(1)) == pdFAIL)
						PRT_ERROR("gpio_timer can't be started\n");
				}
			}
		//}
	}
}

static void __attribute__((used)) create_gpio_wakeup_task(void)
{
	/* Create timer trigger task */
	xTaskCreate(gpio_wake_task, "gpio_wake", WIFIBT_STACK_SIZE, NULL,
		TASK_PRIORITY_2, NULL);
}

DECLARE_RT_MODULE(
	gpio_wakeup,
	MV_SM_ID_ETH,
	create_gpio_wakeup_task,
	NULL,
	NULL
);
