/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"

#include "sm_type.h"
#include "sm_rt_module.h"
#include "sm_exception.h"

#include "sm_uart.h"
#include "sm_printf.h"

//#define TEST_TASK

#ifdef TEST_TASK
#include "sm_common.h"
#include "sm_timer.h"
#endif

int set_vector_base_and_clear_exception(unsigned int vector_base)
{
	unsigned int dummy = 0;

	*(volatile unsigned int*)(0xE000E010) = 0;             // disable SysTick
	dummy = *(volatile unsigned int*)(0xE000E010);         // read to clear flag.

	// init about exception and interrupt handler
	*(volatile unsigned int*)(0xE000ED08) = vector_base;
	*(volatile unsigned int*)(0xE000ED04) |= (1<<27) | (1<<25); // clear PendSV, clear SysTick Pending

	UNUSED(dummy);
	return 0;
}

#ifdef TEST_TASK
static void test_task(void * para)
{
	unsigned int tt = 0;
	unsigned int cur_v = 0;

	for ( ;; ) {
		tt = mv_sm_timer_gettimems();
		cur_v = mv_sm_timer_getticks();
		dbg_printf(PRN_RES, "tick: %d, %d\n", tt, cur_v);
		vTaskDelay(1000);
	}
}

static void __attribute__((used)) create_test_task(void)
{
	xTaskCreate(test_task, "test", 128, NULL, TASK_PRIORITY_2, NULL);
}

DECLARE_RT_MODULE(
	test,
	11,
	create_test_task,
	NULL,
	NULL
);
#endif

static void dev_init(void)
{
	int index = 0, prio = 0;
	rt_init_desc_t * dinit = NULL;

	for(prio = INIT_DEV_P_0; prio <= INIT_MAX_99; prio ++) {
		for(index = 0; ; index++) {
			dinit = get_rt_init_desc(index);
			if(dinit == NULL)
				break;
			if((dinit->init_p == prio) && (dinit->init_func)) {
				if(dinit->init_func() == S_OK)
					dbg_printf(PRN_DBG, "%s inited\n", dinit->name);
				else
					dbg_printf(PRN_RES, "%s failed to init\n", dinit->name);
			}
		}
	}
}

static void create_tasks(void)
{
	int index = 0;
	rt_module_desc_t * module = NULL;

	for(index = 0; ; index++) {
		module = get_rt_module_desc(index);
		if(module == NULL)
			break;
		if(module->c_task) {
			module->c_task();
			dbg_printf(PRN_DBG, "%s task was created\n", module->name);
		}
	}
}

/* Stack overflow hook. */
void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName )
{
	/* Force an assert. */
	configASSERT( pcTaskName == 0 );
}

void _start(void)
{
#ifdef FPGA_V
	//do uart iniitialization
	uart_init();
	dbg_port_rx_flush();
#endif

	//dev init
	dev_init();

	//create tasks
	create_tasks();

	/* Set the scheduler running.  This function will not return unless a task calls vTaskEndScheduler(). */
	vTaskStartScheduler();
}
