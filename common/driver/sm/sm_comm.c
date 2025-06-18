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
#include "com_type.h"
#include "system_manager.h"
#include "debug.h"
#include "io.h"
#ifdef CONFIG_SM_CM3_FW_ENABLE
#include "mem_map_itcm.h"
#endif

#define SM_MSG_SIZE             32
#define SM_MSGQ_TOTAL_SIZE      512
#define SM_MSGQ_HEADER_SIZE     SM_MSG_SIZE
#define SM_MSGQ_SIZE            ( SM_MSGQ_TOTAL_SIZE - SM_MSGQ_HEADER_SIZE )
#define SM_MSGQ_MSG_COUNT       ( SM_MSGQ_SIZE / SM_MSG_SIZE )

typedef struct
{
        INT16   m_iModuleID;
        INT16   m_iMsgLen;
        CHAR    m_pucMsgBody[ SM_MSG_SIZE - sizeof(INT16) * 2 ];
} MV_SM_Message;

typedef struct
{
        INT32   m_iWrite;
        INT32   m_iRead;
        INT32   m_iWriteTotal;
        INT32   m_iReadTotal;
        CHAR    m_Padding[ SM_MSGQ_HEADER_SIZE - sizeof(INT32) * 4 ];
        CHAR    m_Queue[ SM_MSGQ_SIZE ];
} MV_SM_MsgQ;

#ifdef CONFIG_SM_CM3_FW_ENABLE
#define CPU0_INPUT_QUEUE_ADDR   ( SOC_SM_TCM_QRAM_BASE + SM_MSGQ_TOTAL_SIZE * 1)
#define CPU1_INPUT_QUEUE_ADDR   ( SOC_SM_TCM_QRAM_BASE + SM_MSGQ_TOTAL_SIZE * 2)
#define CPU0_OUTPUT_QUEUE_ADDR  ( SOC_SM_TCM_QRAM_BASE + SM_MSGQ_TOTAL_SIZE * 3)
#define CPU1_OUTPUT_QUEUE_ADDR  ( SOC_SM_TCM_QRAM_BASE)
#define SM_Recv_MsgQ (MV_SM_MsgQ*)CPU1_OUTPUT_QUEUE_ADDR
#define SM_Send_MsgQ (MV_SM_MsgQ*)CPU1_INPUT_QUEUE_ADDR
#else
#define CPU0_INPUT_QUEUE_ADDR   ( SM_DTCM_END - SM_MSGQ_TOTAL_SIZE * 4 )
#define CPU1_INPUT_QUEUE_ADDR   ( SM_DTCM_END - SM_MSGQ_TOTAL_SIZE * 3 )
#define CPU0_OUTPUT_QUEUE_ADDR  ( SM_DTCM_END - SM_MSGQ_TOTAL_SIZE * 2 )
#define CPU1_OUTPUT_QUEUE_ADDR  ( SM_DTCM_END - SM_MSGQ_TOTAL_SIZE * 1 )

#define SM_Recv_MsgQ (MV_SM_MsgQ*)SOC_DTCM( CPU1_OUTPUT_QUEUE_ADDR )
#define SM_Recv_Q_Cnt 1
#define SM_Send_MsgQ (MV_SM_MsgQ*)SOC_DTCM( CPU1_INPUT_QUEUE_ADDR )
#endif

#define MV_SM_WD_FASTBOOT	0x48	/* keep syncing to "sm_comm.h" */

typedef struct
{
        INT32 iModuleID;
        INT32 (*Processor)( PVOID pMsgBody, INT32 iLen );
	//INT32 iRemoteCPUID;
        MV_SM_MsgQ* SendQ;
} MV_SM_Msg_Type;

INT32 MV_SM_Send_Msg( INT32 iModuleID, unsigned char * pMsgBody, INT32 iLen )
{
        INT32 i;//DTS for build warning
        volatile MV_SM_MsgQ *MsgQ;
        volatile MV_SM_Message *Msg;
        //INT32 iReg; DTS for build warning

        if (iLen > 28)
                iLen = 28;

                        MsgQ = SM_Send_MsgQ ; //Msg_Type_List[iMsgTypeIndex].SendQ;

                        /* message queue full, ignore the newest message */
                        if (MsgQ->m_iRead == MsgQ->m_iWrite && MsgQ->m_iReadTotal != MsgQ->m_iWriteTotal) {
                                dbg_printf(PRN_ERR,"-----------------> [BL] %s MsgQ Full!\n", __func__);
                                return -1;
                        }

                        Msg = (MV_SM_Message*)(&(MsgQ->m_Queue[MsgQ->m_iWrite]));
                        Msg->m_iModuleID = iModuleID;
                        Msg->m_iMsgLen = iLen;

                        for( i = 0 ; i < iLen ; i ++ )
                        {
                                (Msg->m_pucMsgBody[i]) = ((CHAR*)pMsgBody)[i];
                        }

                        //MV_SM_Dump_Msg(Msg, 0);

                        MsgQ->m_iWrite += SM_MSG_SIZE;
                        if( MsgQ->m_iWrite >= SM_MSGQ_SIZE )
                        {
                                MsgQ->m_iWrite -= SM_MSGQ_SIZE;
                        }
                        MsgQ->m_iWriteTotal += SM_MSG_SIZE;
			/* ensure data are written to SM DTCM before return */
			dsb();

        return S_OK;
}
