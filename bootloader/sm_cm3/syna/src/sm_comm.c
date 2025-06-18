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
#include "FreeRTOS.h"
#include "task.h"

#include "sm_type.h"
#include "sm_common.h"
#include "sm_io.h"
#include "sm_rt_module.h"
#include "SysMgr.h"
#include "sm_memmap.h"

#include "sm_printf.h"

#define SM_RECV_Q_CNT 2

#define COMM_STACK_SIZE ((uint16_t) 128)

// msgQ start address defined in lds
extern const unsigned int __queue_o1;
extern const unsigned int __queue_i0;
extern const unsigned int __queue_i1;
extern const unsigned int __queue_o0;

static MV_SM_MsgQ * msgq_output0 = NULL;
static MV_SM_MsgQ * msgq_output1 = NULL;
static MV_SM_MsgQ * msgq_input0 = NULL;
static MV_SM_MsgQ * msgq_input1 = NULL;

static MV_SM_MsgQ * get_msgq_input(int index)
{
	if(index == 0)
		return msgq_input0;
	else
		return msgq_input1;
}

static MV_SM_MsgQ * get_msgq_output(INT32 index)
{
	if(index == 0)
		return msgq_output0;
	else
		return msgq_output1;
}

static void try_to_dispatch_msg(MV_SM_Message * msg)
{
	hresult ret = S_OK;
	rt_module_desc_t * module = NULL;
	int i = 0;

	for(i = 0; ;i++) {
		module = get_rt_module_desc(i);

		if(module == NULL)
			break;

		if(module->msg_handle != NULL) {
			if((msg->m_iModuleID == BOARDCASTMSG) ||
				(msg->m_iModuleID == module->module_id)) {
				//rom_msg_trace(msg, (void *)msgproc->msg_func);
				ret = module->msg_handle(msg->m_pucMsgBody, msg->m_iMsgLen);
				//rom_msg_handler_leave();
				if(ret != S_OK) {
					//PRT_WARN("Return error(message send to = %d, message handler - %d).\n",
					//	msg->m_iModuleID, msgproc->moduleid);
				}
				if (msg->m_iModuleID == module->module_id)
					break;
			}
		}
	}
}

static void comm_task(void * para)
{
	MV_SM_MsgQ * msgQ = NULL;
	MV_SM_Message * msg = NULL;
	int i = 0;

	for ( ;; ) {
		// fetch message from msgQ (external)
		for(i = 0; i < SM_RECV_Q_CNT; i++) {
			msgQ = get_msgq_input(i);
			if(msgQ->m_iReadTotal < msgQ->m_iWriteTotal) {
				//read the received message
				msg = (MV_SM_Message*)(&(msgQ->m_Queue[msgQ->m_iRead]));

				//dispatch the message
				try_to_dispatch_msg(msg);

				msgQ->m_iRead += SM_MSG_SIZE;
				if(msgQ->m_iRead >= SM_MSGQ_SIZE ) {
					msgQ->m_iRead -= SM_MSGQ_SIZE;
				}
				msgQ->m_iReadTotal += SM_MSG_SIZE;
			}
		}
		vTaskDelay(1);
	}
}

static void __attribute__((used)) create_comm_task(void)
{
	msgq_output0 = (MV_SM_MsgQ *)&__queue_o0;
	msgq_output1 = (MV_SM_MsgQ *)&__queue_o1;
	msgq_input0 = (MV_SM_MsgQ *)&__queue_i0;
	msgq_input1 = (MV_SM_MsgQ *)&__queue_i1;

	// clear msgQ
	memset((void *)msgq_output0, 0, SM_MSGQ_TOTAL_SIZE);
	memset((void *)msgq_output1, 0, SM_MSGQ_TOTAL_SIZE);
	memset((void *)msgq_input0, 0, SM_MSGQ_TOTAL_SIZE);
	memset((void *)msgq_input1, 0, SM_MSGQ_TOTAL_SIZE);

	xTaskCreate(comm_task, "comm", COMM_STACK_SIZE, NULL, TASK_PRIORITY_2, NULL);
}

DECLARE_RT_MODULE(
	comm,
	MV_SM_ID_COMM,
	create_comm_task,
	NULL,
	NULL
);

int sm_send_internal_msg(int moduleID, unsigned char * pMsgBody, int len)
{
	hresult ret = S_OK;
	MV_SM_Message msg;
	rt_module_desc_t * module = NULL;
	int i = 0;

	if (len > SM_MSG_BODY_SIZE) {
		//PRT_WARN("%s msg len %d exceed buffer size %d\n",
		//	 __func__, len, SM_MSG_BODY_SIZE);
		return S_FALSE;
	}

	msg.m_iModuleID = moduleID;
	msg.m_iMsgLen = len;
	for(i = 0;i < len;i++) {
		msg.m_pucMsgBody[i] = ((char *)pMsgBody)[i];
	}

	for(i = 0; ; i++) {
		module = get_rt_module_desc(i);

		if(module == NULL)
			break;

		if(module->in_msg_handle != NULL) {
			if((msg.m_iModuleID == BOARDCASTMSG) ||
				(msg.m_iModuleID == module->module_id)) {
				//rom_msg_trace(msg, (void *)msgproc->msg_func);
				ret = module->in_msg_handle(msg.m_pucMsgBody, msg.m_iMsgLen);
				//rom_msg_handler_leave();
				if(ret != S_OK) {
					//PRT_WARN("Return error(message send to = %d, message handler - %d).\n",
					//	msg->m_iModuleID, msgproc->moduleid);
				}
				if (msg.m_iModuleID == module->module_id)
					break;
			}
		}
	}

	return S_OK;
}

int sm_send_msg(int moduleID, unsigned char * pMsgBody, int len)
{
	MV_SM_MsgQ *msgQ = get_msgq_output(0);
	MV_SM_Message *msg = NULL;
	int i = 0;
	T32smSysCtl_SM_CTRL reg;

	if (len > SM_MSG_BODY_SIZE) {
		//PRT_WARN("%s msg len %d exceed buffer size %d\n",
		//	 __func__, len, SM_MSG_BODY_SIZE);
		return S_FALSE;
	}

	msg = (MV_SM_Message*)(&(msgQ->m_Queue[msgQ->m_iWrite]));

	if((msgQ->m_iRead == msgQ->m_iWrite)
		&& (msgQ->m_iReadTotal != msgQ->m_iWriteTotal))	{
		// message queue full, ignore the newest message
		return S_FALSE;
	}

	msg->m_iModuleID = moduleID;
	msg->m_iMsgLen = len;
	for(i = 0;i < len;i++) {
		msg->m_pucMsgBody[i] = ((char *)pMsgBody)[i];
	}

	msgQ->m_iWrite += SM_MSG_SIZE;
	if(msgQ->m_iWrite >= SM_MSGQ_SIZE) {
		msgQ->m_iWrite -= SM_MSGQ_SIZE;
	}

	msgQ->m_iWriteTotal += SM_MSG_SIZE;

	//When sending message to CPU0, the SM to SoC interrupt is raised together.
	reg.u32 = MV_SM_READ_REG32(SM_MEMMAP_SYS_CTRL_BASE + RA_smSysCtl_SM_CTRL);
	reg.uSM_CTRL_SM2SOC_SW_INTR = 1; //enable interrupt
	MV_SM_WRITE_REG32(SM_MEMMAP_SYS_CTRL_BASE + RA_smSysCtl_SM_CTRL, reg.u32);

	return S_OK;
}
