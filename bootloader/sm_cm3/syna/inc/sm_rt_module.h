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
#ifndef __SM_RT_MODULE_H
#define __SM_RT_MODULE_H

#define MAX_RT_MODULE	32

/* Prototype for create runtime task function */
typedef void (*create_rt_module_task)(void);

/* Prototype for module message handle function
 * (both external and internal message) */
typedef int (*module_msg_proc)(void * data, int len);

typedef struct rt_module_desc {
	const char *name;
	int module_id;
	create_rt_module_task c_task;
	module_msg_proc msg_handle;
	module_msg_proc in_msg_handle;
} rt_module_desc_t;

/*
 * Convenience macro to declare a service descriptor
 */
#define DECLARE_RT_MODULE(_name, _id, _create_task, _msg_hl, _in_msg_hl) \
	static const rt_module_desc_t __rt_module_desc_ ## _name \
		__attribute__((__section__("rt_module_descs"))) __attribute__((__used__)) = { \
			.name = #_name, \
			.module_id = _id, \
			.c_task = _create_task, \
			.msg_handle = _msg_hl, \
			.in_msg_handle = _in_msg_hl }

rt_module_desc_t * get_rt_module_desc(int index);

/* module init descriptions */
typedef enum {
	INIT_DEV_P_0 = 0x1,
	INIT_DEV_P_25,
	INIT_DEV_P_50,
	INIT_MOD_P_75,
	INIT_MAX_99
} INIT_PRIORITY;

typedef int (*module_init)(void);

typedef struct rt_init_desc {
	const char *name;
	unsigned int init_p;
	module_init init_func;
}rt_init_desc_t;

/*
 * Convenience macro to declare a moudule init descriptor
 */
#define DECLARE_RT_INIT(_name, _init_p, _init_func) \
	static const rt_init_desc_t __rt_init_desc_ ## _name \
		__attribute__((__section__("rt_init_descs"))) __attribute__((__used__)) = { \
			.name = #_name, \
			.init_p = _init_p, \
			.init_func = _init_func }

rt_init_desc_t * get_rt_init_desc(int index);

#endif
