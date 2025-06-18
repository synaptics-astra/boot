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
#ifdef CECENABLE
#include "sm_common.h"
#include "sm_type.h"

#include "platform_config.h"
#include "sm_timer.h"
#include "sm_printf.h"
#include "sm_state.h"
#include "sm_power.h"

#include "sm_cec.h"
#include "cec_type.h"

extern UINT32 rememberHdmiCtrlState;	// from cec_core.c, passed down from SoC.. HDMI Control on/off by user
extern SOC_REQ_AT_STARTUP socReq;	// global here, we write, cec_core.c reads.
extern char cec_osd_name[OSD_NAME_MAX_LENGTH + 1];

extern CEC_MSG wakeup_msg;
extern CECConfig SMCECConfig;
//In the Following Table all the FALSE Action Required has been changed to TRUE by Shailendra

CEC_MSG_TABLE *SMCECMsgTable;
INT32 cec_msg_table_size;

#define CEC_GET_MSG_MODE(addr)  (((addr&0x0F) == 0x0F)?CEC_MODE_BROADCAST:CEC_MODE_UNICAST)

hresult sm_cec_srv_tx_msg (CEC_MSG *ptxmsg);

static INT32 CEC_SRV_GetMessageIndex (UINT8 ucMsgOpcode)
{
	INT32 ucMsgIndex = -1;
	INT32 i;

	for (i = 0; i < cec_msg_table_size; i++)
	{
		if (SMCECMsgTable[i].msg_opcode == ucMsgOpcode)
		{
			ucMsgIndex = i;
			break;
		}
	}
	return (ucMsgIndex);
}

static UINT32 findmux(UINT32 phyAddr)
{
	UINT32 mux=0xFFFF;
	if((phyAddr&0xF000)==phyAddr)
		mux=0xF000;
	else if((phyAddr&0xFF00)==phyAddr)
		mux=0xFF00;
	else if((phyAddr&0xFFF0)==phyAddr)
		mux=0xFFF0;
	else if((phyAddr&0xFFFF)==phyAddr)
		mux=0xFFFF;
	return mux;
}

/******************************************************************************
 * FUNCTION : Format message from frame data or format frame data from message
 * PARAMS   : pdata        - Pointer to frame data
 *          : pmsg         - Pointer to message
 *          : bmsg2frmst       -TRUE: format message; FALSE: format frame data
 * RETURN   : S_OK, succeed; S_FALSE, fail
 *****************************************************************************/
static INT32 CEC_SRV_FormatConvert (CEC_TASK_DATA *pdata,CEC_MSG *pmsg,BOOL bmsg2frm)
{
	INT32 posinfrm=0;
	INT32 msgTableIndex;
	UINT32 i;
	char* pChar;

	if(bmsg2frm)  //(outbound)
	{
		memset(pdata, 0, sizeof(CEC_TASK_DATA));
		pdata->msg_len=0;
		pdata->msg_cfg.mode=(pmsg->destAddr==CEC_BROADCAST_DESTADDR)?CEC_MODE_BROADCAST:CEC_MODE_UNICAST;
		pdata->msg_cfg.retry = FALSE;

		pdata->msg_fifo[(pdata->msg_len)++]=CEC_SET_HEADER(pmsg->srcAddr,pmsg->destAddr);
		pdata->msg_fifo[(pdata->msg_len)++]=pmsg->opcode;

		//write operand
		switch(pmsg->opcode)
		{
		case CEC_MSG_OPCODE_FEATURE_ABORT:
			pdata->msg_fifo[(pdata->msg_len)++]=pmsg->operand.opFeatureAbort.featureOpcode;
			pdata->msg_fifo[(pdata->msg_len)++]=pmsg->operand.opFeatureAbort.abortReason;
			break;

		case CEC_MSG_OPCODE_REPORT_PHY_ADDR:
			pdata->msg_fifo[(pdata->msg_len)++] = (pmsg->operand.opRepPhyAddr.phyAddr >> 8) & 0xFF;
			pdata->msg_fifo[(pdata->msg_len)++] = pmsg->operand.opRepPhyAddr.phyAddr & 0xFF;
			pdata->msg_fifo[(pdata->msg_len)++]=pmsg->operand.opRepPhyAddr.deviceType;
			break;

		case CEC_MSG_OPCODE_CEC_VERSION:
			pdata->msg_fifo[(pdata->msg_len)++]=pmsg->operand.opVersion;
			break;

               case CEC_MSG_OPCODE_DEVICE_VENDOR_ID:
                       pdata->msg_fifo[(pdata->msg_len)++] = (pmsg->operand.opVendorID >> 16) & 0xFF;
                        pdata->msg_fifo[(pdata->msg_len)++] = (pmsg->operand.opVendorID >> 8) & 0xFF;
                        pdata->msg_fifo[(pdata->msg_len)++] = pmsg->operand.opVendorID & 0xFF;
                        break;

		case CEC_MSG_OPCODE_DECK_STATUS:
			pdata->msg_fifo[(pdata->msg_len)++]=pmsg->operand.opDeckInfo;
			break;

		case CEC_MSG_OPCODE_SET_OSD_NAME:
			memcpy((UINT8 *)&(pdata->msg_fifo[pdata->msg_len]),(UINT8 *)&(pmsg->operand.opDevOsdName.osdName),OSD_NAME_MAX_LENGTH);
			pdata->msg_len+=OSD_NAME_MAX_LENGTH;
			break;

		case CEC_MSG_OPCODE_REPORT_POWER_STATUS:
			pdata->msg_fifo[(pdata->msg_len)++]=pmsg->operand.opPwrSts;
			break;

#ifdef BUDDY_RELEASE

		case CEC_MSG_OPCODE_VENDOR_COMMAND_WITH_ID:		// outbound conversion of vendor-specific cmd from CEC_MSG to CEC_TASK_DATA format

			//remove padding in the CEC_MSG struct (and the structs it includes) to make the CEC_TASK_DATA struct which has no padding in it.

			// copy the VendorCmdWithID portion of the message (containing Buddy Opcode) plus variable-length param block for that Buddy opcode.
			memcpy( &(pdata->msg_fifo[0]), &(pmsg->operand), pmsg->operandlen);
			pdata->msg_len = pmsg->operandlen;

#if 1 //debug tbdzz
						PRT_DEBUG("SMCEC2SM: OUTBOUND VENDOR_COMMAND_WITH_ID msg, (msg_len=%d)\nSMCEC2SM: ", pdata->msg_len);
						pChar=(char*)pdata;
						for (i=0; i< pdata->msg_len; i++)
							PRT_DEBUG("%02x ", (pChar[i])&0xFF);
						PRT_DEBUG("\n");
#endif // debug tbdzz

			break;

#endif // BUDDY_RELEASE



		default:
			PRT_DEBUG("WARNING, cec_srv got a cmd with a bogus ID=0x%x!\n", pmsg->opcode);
			break;
		}
	}
	else//frame data to message  (inbound)
	{
		memset(pmsg, 0, sizeof(CEC_MSG));
		pmsg->opcode=pdata->msg_fifo[1];
		pmsg->srcAddr=CEC_GET_INIT_ADDR(pdata->msg_fifo[0]);
		pmsg->destAddr=CEC_GET_FOLL_ADDR(pdata->msg_fifo[0]);
		pmsg->operandlen=pdata->msg_len-2;
		posinfrm+=2;

		PRT_DEBUG("[RX MSG]\n");
		PRT_DEBUG("      msg_fifo: ");
		for (i = 0; i < pdata->msg_len; i++)
			PRT_DEBUG("0x%02x ", pdata->msg_fifo[i]);
		PRT_DEBUG("\n");
		PRT_DEBUG("      srcaddr: 0x%02x destaddr: 0x%02x opcode: 0x%02x\n",
			   pmsg->srcAddr, pmsg->destAddr, pmsg->opcode);


		// Handle Messages originating from unregistered address, 0x0F
		if (pmsg->srcAddr == 0x0F)
		{
			if (!((pmsg->opcode == CEC_MSG_OPCODE_REQUEST_ACTIVE_SOURCE) ||
						(pmsg->opcode == CEC_MSG_OPCODE_SET_STREAM_PATH)       ||
						(pmsg->opcode == CEC_MSG_OPCODE_ACTIVE_SOURCE)       ||
						(pmsg->opcode == CEC_MSG_OPCODE_ROUTING_CHANGE)        ||
						(pmsg->opcode == CEC_MSG_OPCODE_ROUTING_INFO)          ||
						(pmsg->opcode == CEC_MSG_OPCODE_STANDBY)               ||
						(pmsg->opcode == CEC_MSG_OPCODE_GIVE_PHY_ADDR)         ||
						(pmsg->opcode == CEC_MSG_OPCODE_GET_MENU_LANG)		   ||
						(pmsg->opcode == CEC_MSG_OPCODE_VENDOR_COMMAND_WITH_ID) ||
						(pmsg->opcode == CEC_MSG_OPCODE_GIVE_DEVICE_VENDOR_ID)))
			{
				PRT_DEBUG("W/cec: 1, msg ignored\n");
				return (CEC_STATUS_ERR_MSG_IGNORED);
			}
		}

		msgTableIndex = CEC_SRV_GetMessageIndex(pmsg->opcode);
		if (msgTableIndex == -1)
		{
			// Message not found in table - message not supported
			// Unsupported broadcast messages has to be ignored
			// Unsupported unicast messages has to be responded with feature abort
			if (pmsg->destAddr == 0x0F) // Broadcast message
			{
				PRT_DEBUG("W/cec: 2, msg ignored\n");
				return CEC_STATUS_ERR_MSG_IGNORED;
			} else {
				PRT_DEBUG("W/cec: 3, msg unsupported\n");
				return CEC_STATUS_ERR_MSG_UNSUPPORTED;
			}
		}

		// Message supported
		// Check message mode

		if (!(SMCECMsgTable[msgTableIndex].msg_mode & CEC_GET_MSG_MODE(pmsg->destAddr)))

#ifndef BUDDY_RELEASE
		{
			PRT_DEBUG("W/cec: 4, msg ignored\n");
			return CEC_STATUS_ERR_MSG_IGNORED;
		}
#else
		if (pmsg->opcode != CEC_MSG_OPCODE_VENDOR_COMMAND_WITH_ID) // some subtypes uni, some bcast. Handled individually below.
		{
			PRT_DEBUG("W/cec: 5, msg ignored\n");
			return CEC_STATUS_ERR_MSG_IGNORED;
		}
#endif

		// Check length
		if (pmsg->operandlen < SMCECMsgTable[msgTableIndex].msg_len)
		{
			PRT_DEBUG("W/cec: 6, msg ignored\n");
			// Number of operands is less than the required number
			return CEC_STATUS_ERR_MSG_IGNORED;
		}

		// Check if any action is required on the message
		if (SMCECMsgTable[msgTableIndex].msg_rxaction_reqd == FALSE)
		{
			PRT_DEBUG("W/cec: 7, msg ignored\n");
			// Ignore the message - no action required
			return CEC_STATUS_ERR_MSG_IGNORED;
		}

		//Fill the operands
		switch(pmsg->opcode)
		{
		case CEC_MSG_OPCODE_SET_STREAM_PATH:
			pmsg->operand.opPhyAddr= (pdata->msg_fifo[posinfrm] << 8) | pdata->msg_fifo[posinfrm+1];
			break;
		case CEC_MSG_OPCODE_ACTIVE_SOURCE:
			pmsg->operand.opPhyAddr= (pdata->msg_fifo[posinfrm] << 8) | pdata->msg_fifo[posinfrm+1];
			break;

		case CEC_MSG_OPCODE_ROUTING_CHANGE:
			pmsg->operand.opRoutingChange.origPhyAddr=(pdata->msg_fifo[posinfrm] << 8) | pdata->msg_fifo[posinfrm+1];
			posinfrm+=2;
			pmsg->operand.opRoutingChange.newPhyAddr=(pdata->msg_fifo[posinfrm] << 8) | pdata->msg_fifo[posinfrm+1];
			break;

		case CEC_MSG_OPCODE_GIVE_DECK_STATUS:
			pmsg->operand.opStatusRequest=pdata->msg_fifo[posinfrm];
			break;

		case CEC_MSG_OPCODE_DECK_CONTROL:
			pmsg->operand.opDeckControlMode=pdata->msg_fifo[posinfrm];
			break;

		case CEC_MSG_OPCODE_PLAY:
			pmsg->operand.opPlayMode=pdata->msg_fifo[posinfrm];
			break;

		case CEC_MSG_OPCODE_GIVE_PHY_ADDR:
		case CEC_MSG_OPCODE_GIVE_OSD_NAME:
			break;

		case CEC_MSG_OPCODE_USER_CONTROL_PRESSED://operand len>=1
			//Here, only support play function with additional operands
			if( pdata->msg_fifo[posinfrm]==CEC_UC_PLAY_FN)
			{
				if(pmsg->operandlen==1)
				{
					pmsg->operand.opUserControlCode.opUserControlCodeonebyte=pdata->msg_fifo[posinfrm];
				}
				else if(pmsg->operandlen==2)
				{
					pmsg->operand.opUserControlCode.opUserControlPlayext.uccode=pdata->msg_fifo[posinfrm];
					posinfrm++;
					pmsg->operand.opUserControlCode.opUserControlPlayext.playmode=pdata->msg_fifo[posinfrm];
				}
				else
				{
					PRT_DEBUG("W/cec: 8, msg unsupported\n");
					return CEC_STATUS_ERR_MSG_UNSUPPORTED;
				}
			}
			else
			{
				pmsg->operand.opUserControlCode.opUserControlCodeonebyte=pdata->msg_fifo[posinfrm];
			}
			break;

		case CEC_MSG_OPCODE_VENDOR_COMMAND_WITH_ID:

			//inbound addr conversions:  CEC_TASK_DATA --> CEC_MSG


			// inbound: the frame is represented by a Vendor Command with ID struct, starting in CEC_TASK_DATA.msg_fifo.
			// But in the CEC_MSG struct the compiler added padding before CEC_MSG.operand so that it lands on a UINT32 boundary.
			// The function of these 2 memcpy's is to add equivalent padding.

			// copy the VendorCmdWithID portion, which contains buddy opcode, plus variable-length param block of that opcode.

			memcpy( &(pmsg->operand), &(pdata->msg_fifo), pdata->msg_len);

			pmsg->operandlen = pdata->msg_len;

			PRT_DEBUG("\n\nSM cec_src: inbound, operandlen=%d. dest addr=0x%x:\nSMCEC2SM:  ", pmsg->operandlen, (UINT32)&(pmsg->operand));
				pChar = (char*)&(pmsg->operand);
				for (i=0; i<pmsg->operandlen; i++)
					PRT_DEBUG("%02x ", pChar[i]&0xFF);
			PRT_DEBUG("\n");
			break;

		default:
			PRT_INFO("SM: cec_srv: inbound msg with unsupported opcode=0x%x! Ignoring!\n", pmsg->opcode );
			break;
		}
	}

	return CEC_STATUS_SUCCESS;
}

/******************************************************************************
 * FUNCTION : Process CEC_CMD_PROC_RX_MSG cmd sent to task (TV)
 * PARAMS   :
 *  RETURN   : S_OK, succeed; S_FALSE, fail
 ******************************************************************************/

hresult MV_SM_CEC_SRV_RX_MSG_TV(CEC_TASK_DATA *prxdata)
{
    CEC_MSG RecvMsg,SendMsg;
    int featureAbortReason = -1;
    int ret =0;
	INT32 iSysState;

	iSysState = mv_sm_get_state();

    if (rememberHdmiCtrlState == 0)    // HDMI control off
    {
        PRT_INFO("SM:CEC_SRV: rejecting because HDMI Ctrl is off.\n");
        return (S_OK);
    }

    ret = CEC_SRV_FormatConvert(prxdata,&RecvMsg,FALSE);

    if((ret  == CEC_STATUS_ERR_MSG_IGNORED) && (RecvMsg.destAddr == 0x0F))
    {
        //PRT_INFO("This message 0x%x is Ignored\n", RecvMsg.opcode);
        return (S_OK);
    }

    PRT_INFO("RecvMsg srcaddr=0x%02x,destaddr=0x%02x,opcode=0x%02x\n",RecvMsg.srcAddr,RecvMsg.destAddr,RecvMsg.opcode);
    switch(RecvMsg.opcode)
    {
        /*  On receiving <Abort> message: Send <Feature Abort> message to message sender. */
        case CEC_MSG_OPCODE_ABORT:
             //PRT_INFO("Received CEC_MSG_OPCODE_ABORT\n");
             featureAbortReason = CEC_ABORT_REASON_REFUSED;
             break;

        case CEC_MSG_OPCODE_IMAGE_VIEW_ON:
             // Turn On and Request Active Source
             if((MV_SM_STATE_NORMALSTANDBY == iSysState)||
			 	(MV_SM_STATE_LOWPOWERSTANDBY == iSysState)||
			 	(MV_SM_STATE_SUSPEND == iSysState))
             {
                //wake up TV
                 memcpy(&wakeup_msg, &RecvMsg, sizeof(CEC_MSG));
                 mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_CEC);
                 mv_sm_power_enterflow_bysmstate();
             }
             break;

        case CEC_MSG_OPCODE_TEXT_VIEW_ON:
             if((MV_SM_STATE_NORMALSTANDBY == iSysState)||
			 	(MV_SM_STATE_LOWPOWERSTANDBY == iSysState)||
			 	(MV_SM_STATE_SUSPEND == iSysState))
             {
                //wake up TV
                //PRT_INFO("cec TV Soc wake up\n");
                 memcpy(&wakeup_msg, &RecvMsg, sizeof(CEC_MSG));
                 mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_CEC);
                 mv_sm_power_enterflow_bysmstate();
             }
             break;

        case CEC_MSG_OPCODE_GIVE_PHY_ADDR:
             //Broadcast Report Physical Address
             //PRT_INFO("CEC_MSG_OPCODE_GIVE_PHY_ADDR: Send REPORT_PHY_ADDR\n");
             SendMsg.destAddr = 0x0F;
             SendMsg.opcode = CEC_MSG_OPCODE_REPORT_PHY_ADDR;
             SendMsg.operand.opRepPhyAddr.phyAddr = SMCECConfig.uiPhyAddr;
             SendMsg.operand.opRepPhyAddr.deviceType = SMCECConfig.iDeviceType;
             sm_cec_srv_tx_msg(&SendMsg);
             break;

        case CEC_MSG_OPCODE_GET_CEC_VERSION:
             //Send CEC Version as 1.4 a
             //PRT_INFO("OPCODE_GET_CEC_VERSION: Send OPCODE_CEC_VERSION  VERSION_1_4a\n");
             SendMsg.destAddr=RecvMsg.srcAddr;
             SendMsg.opcode = CEC_MSG_OPCODE_CEC_VERSION;
             SendMsg.operand.opVersion = CEC_VERSION_1_4a;
             sm_cec_srv_tx_msg(&SendMsg);
             break;

        case CEC_MSG_OPCODE_USER_CONTROL_PRESSED:
             //Do as per Excel sheet
             //PRT_INFO("OPCODE_USER_CONTROL_PRESSED: \n");
             if(RecvMsg.operand.opUserControlCode.opUserControlCodeonebyte == CEC_UC_POWER ||
                RecvMsg.operand.opUserControlCode.opUserControlCodeonebyte == CEC_UC_POWER_TOGGLE ||
                RecvMsg.operand.opUserControlCode.opUserControlCodeonebyte == CEC_UC_POWER_ON)
             {
                if((MV_SM_STATE_NORMALSTANDBY == iSysState)||
					(MV_SM_STATE_LOWPOWERSTANDBY == iSysState)||
					(MV_SM_STATE_SUSPEND == iSysState))
                {
                   //wake up TV
                   //PRT_INFO("cec TV Soc wake up\n");
                    memcpy(&wakeup_msg, &RecvMsg, sizeof(CEC_MSG));
                    mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_CEC);
                    mv_sm_power_enterflow_bysmstate();
                }
             }
             else
             {
                featureAbortReason = CEC_ABORT_REASON_INV_MODE_TO_RESPOND;
             }
             break;

        case CEC_MSG_OPCODE_GIVE_DEVICE_POWER_STATUS:
             //Send Report Power Status
             //PRT_INFO("GIVE_DEVICE_POWER_STATUS: Send REPORT_POWER_STATUS\n");
             SendMsg.destAddr=RecvMsg.srcAddr;
             SendMsg.opcode = CEC_MSG_OPCODE_REPORT_POWER_STATUS;
             SendMsg.operand.opPwrSts = CEC_PWR_STS_STANDBY;
             sm_cec_srv_tx_msg(&SendMsg);
             break;


        case CEC_MSG_OPCODE_INACTIVE_SOURCE:
        case CEC_MSG_OPCODE_GET_MENU_LANG:
        case CEC_MSG_OPCODE_SET_OSD_NAME:
        case CEC_MSG_OPCODE_REPORT_POWER_STATUS:
        case CEC_MSG_OPCODE_REPORT_AUDIO_STATUS:
        case CEC_MSG_OPCODE_SET_SYS_AUDIO_MODE:
        case CEC_MSG_OPCODE_MENU_STATUS:
        case CEC_MSG_OPCODE_DECK_STATUS:
        case CEC_MSG_OPCODE_RECORD_STATUS:
        case CEC_MSG_OPCODE_RECORD_TV_SCREEN:
        case CEC_MSG_OPCODE_TUNER_DEVICE_STATUS:
        case CEC_MSG_OPCODE_TIMER_STATUS:
        case CEC_MSG_OPCODE_TIMER_CLEARED_STATUS:
        case CEC_MSG_OPCODE_GIVE_DEVICE_VENDOR_ID:
        case CEC_MSG_OPCODE_VENDOR_COMMAND:
        case CEC_MSG_OPCODE_VENDOR_COMMAND_WITH_ID:
        case CEC_MSG_OPCODE_VENDOR_REMOTE_BTN_UP:
        case CEC_MSG_OPCODE_VENDOR_REMOTE_BTN_DOWN:

             featureAbortReason = CEC_ABORT_REASON_INV_MODE_TO_RESPOND;
             //PRT_INFO("Received Message = 0x%x, Invalid Mode case\n", RecvMsg.opcode);
             break;

        default:
            // Retcode unsupported would fall in here
             featureAbortReason = CEC_ABORT_REASON_UNREC_OPCODE;
             //PRT_INFO("default: Received Message = 0x%x\n", RecvMsg.opcode);
             break;
    }
    if (featureAbortReason != -1)
    {
        //PRT_INFO("Send CEC_MSG_OPCODE_FEATURE_ABORT with reason = %d\n", featureAbortReason);
        SendMsg.destAddr=RecvMsg.srcAddr;
        SendMsg.opcode = CEC_MSG_OPCODE_FEATURE_ABORT;
        SendMsg.operand.opFeatureAbort.featureOpcode = RecvMsg.opcode;
        SendMsg.operand.opFeatureAbort.abortReason = featureAbortReason;
        sm_cec_srv_tx_msg(&SendMsg);
    }
   return S_OK;
}

#ifdef BUDDY_RELEASE
// sendBuddyVendorFeatureAbort() - unicast a Vendor Feature Abort (almost the same as a Feature Abort but
// it's a subtype under VENDOR_CMD_WITH_ID.) It applies only to errors encountered with other subtypes
// of VENDOR_CMD_WITH_ID. Format it as a CEC_MSG, and MC_SM_CEC_SRC_TX_MSG will remove the padding.
hresult sendBuddyVendorFeatureAbort (UINT32 vendorErrorCode, CEC_MSG* pRecvMsg, CEC_MSG* pSendMsg)
{
	CEC_VENDOR_CMD_WITH_ID *pVCWID;
	BUDDY_FUNC_ABORT  *pBuddyFuncAbort;			// vendor-specific feature abort defined by Buddy
	UINT32 i;
	char* pChar;

	// set up CEC_MSG portion of the vendor_feature_abort msg.
	pSendMsg->opcode = CEC_MSG_OPCODE_VENDOR_COMMAND_WITH_ID;
	pSendMsg->destAddr = pRecvMsg->srcAddr; 					// message we're responding to with VFA.
	pSendMsg->operandlen = expected_length_VENDOR_FEATURE_ABORT;  // non-padded, we only consider the part after the ->operand..

	pVCWID = &(pSendMsg->operand.opVendCmdWithID);
	pVCWID->opcode = CEC_MSG_OPCODE_VENDOR_COMMAND_WITH_ID;
	pVCWID->vendorUID[0] = 0x08;
	pVCWID->vendorUID[1] = 0x00;
	pVCWID->vendorUID[2] = 0x46;
	pVCWID->buddy_opcode =  /*(UINT16)*/CEC_VEND_CMD_BUDDY_VEND_FEATURE_ABORT;
	pVCWID->decrCtr = 0;

	pBuddyFuncAbort = &(pVCWID->vendCmd.abort);
	pBuddyFuncAbort->offendingBuddyOpcodeH = (pRecvMsg->opcode>>8)&0xFF;
	pBuddyFuncAbort->offendingBuddyOpcodeL = pRecvMsg->opcode&0xFF;
	pBuddyFuncAbort->reason = vendorErrorCode;

#if 1 //debug tbdzz
	PRT_DEBUG("SMCEC2SM: sendBuddyVFAbort sends msg (len=%d)\nSMCEC2SM:   ", pSendMsg->operandlen);
		pChar = (char*)pSendMsg;
		for (i=0; i<pSendMsg->operandlen+5; i++)
		PRT_DEBUG("%02x	", pChar[i]&0xFF);
	PRT_DEBUG("\n");
#endif // debug tbdzz

	sm_cec_srv_tx_msg(pSendMsg);


}

// sendReportAddlMenuInfo() - broken out for readability - broadcast a REPORT_ADDL_MENU_INFO msg in response to a
// GIVE_ADDL_MENU_INFO from a Buddy TV.
hresult sendReportAddlMenuInfo (CEC_MSG* pSendMsg, CEC_MSG* pRecvMsg)
{
	CEC_VENDOR_CMD_WITH_ID *pVCWID;
	BUDDY_MENU_REPORT  *pBuddyReportMenu;			// vendor-specific feature abort defined by Buddy
	UINT32 i;
	char* pChar;

	// CEC portion...
	pSendMsg->opcode = CEC_MSG_OPCODE_VENDOR_COMMAND_WITH_ID;
	pSendMsg->destAddr = CEC_BROADCAST_DESTADDR;
	pSendMsg->operandlen = expected_length_BUDDY_MENU_REPORT;

	pVCWID = &(pSendMsg->operand.opVendCmdWithID);

	pVCWID->initfoll =  (SMCECConfig.iLogicAddr << 4) | 0x0F;
	pVCWID->opcode = CEC_MSG_OPCODE_VENDOR_COMMAND_WITH_ID;
	pVCWID->vendorUID[0] = 0x08;
	pVCWID->vendorUID[1] = 0x00;
	pVCWID->vendorUID[2] = 0x46;
	pVCWID->null_HOB_of_buddy_opcode = 0x00;
	pVCWID->buddy_opcode = /*(UINT16)*/CEC_VEND_CMD_BUDDY_REPORT_ADDL_MENU_INFO;
	pVCWID->decrCtr = 0;

	pBuddyReportMenu = &(pVCWID->vendCmd.report);
	pBuddyReportMenu->menu_category = 0x00;		// "Buddy BTV Box"
	pBuddyReportMenu->table_id = 0x00;		// "Simple Menu for Android"
	pBuddyReportMenu->reservedH = 0x00;
	pBuddyReportMenu->reservedL = 0x00;

#if 1 //debug tbdzz
		PRT_DEBUG("SMCEC2SM: sendReportAddlMenuInfo sends msg (len=%d)\nSMCEC2SM:   ", pSendMsg->operandlen);
		pChar = (char*)pSendMsg;
		for (i=0; i < pSendMsg->operandlen+5; i++)
			PRT_DEBUG("%02x	", pChar[i]&0xFF);
		PRT_DEBUG("\n");
#endif // debug tbdzz

	sm_cec_srv_tx_msg(pSendMsg);
}

#endif //BUDDY_RELEASE

/******************************************************************************
 * FUNCTION : Process CEC_CMD_PROC_RX_MSG cmd sent to task (BOX)
 * PARAMS   :
 *  RETURN   : S_OK, succeed; S_FALSE, fail
 *  XXX we are here!
 ******************************************************************************/

hresult MV_SM_CEC_SRV_RX_MSG_BOX(CEC_TASK_DATA *prxdata)
{
	CEC_MSG SendMsg;
	CEC_MSG RecvMsg;
	int featureAbortReason = -1;
	UINT32 i;
	char *pChar;
	char *pcPhysical;
#ifdef BUDDY_RELEASE
	CEC_MSG *pRecvMsg;
	UINT32 offendingOpHL;
#endif
	INT32 iSysState;

	iSysState = mv_sm_get_state();

	pChar = (char*)(&RecvMsg);
	for (i=0; i<sizeof(CEC_MSG); i++) { pChar[i]=0xAA; } //debug

	if(CEC_SRV_FormatConvert(prxdata,&RecvMsg,FALSE) == CEC_STATUS_ERR_MSG_IGNORED)
		return (S_OK);

	// if HDMI_Control was turned off by the user, all messages but VENDOR_CMD_WITH_ID should be rejected.
	if (rememberHdmiCtrlState == 0)    // HDMI control off
	{
		//if (RecvMsg.opcode != CEC_MSG_OPCODE_VENDOR_COMMAND_WITH_ID)
		//{
                        return (S_OK);
		//}
	}

	switch(RecvMsg.opcode)
	{
		/* General Protocol messages. */
		/* On receiving <Abort> message:
		 * Send <Feature Abort> message to message sender.
		 */
	case CEC_MSG_OPCODE_ABORT:
		featureAbortReason = CEC_ABORT_REASON_REFUSED;
		break;
		/* On recv <Active source> messsge; */
	case CEC_MSG_OPCODE_ACTIVE_SOURCE:
		PRT_DEBUG("opPhyAddr=0x%02x, uiPhyAddr=0x%02x, state: %d\n", RecvMsg.operand.opPhyAddr, SMCECConfig.uiPhyAddr, iSysState);

		/* bypass physical address checking on Burnley/Bolton.
		 * But if the parameter 'physical address' is 0.0.0.0, we should ignore it because it indicates that a TV is switching to its internal tuner or to another non-HDMI source
		 */
		if (0 != RecvMsg.operand.opPhyAddr)
//		if (((RecvMsg.operand.opPhyAddr) & findmux(SMCECConfig.uiPhyAddr)) == SMCECConfig.uiPhyAddr)
		{
			/*FIXME: suspend?*/
			if((MV_SM_STATE_LOWPOWERSTANDBY == iSysState) ||
				(MV_SM_STATE_NORMALSTANDBY == iSysState) ||
				(MV_SM_STATE_SUSPEND == iSysState))
			{
				//wake up system
				PRT_INFO("cec wake up:Active Source for 0x%x\n", RecvMsg.operand.opPhyAddr);

				pcPhysical = (char *) &RecvMsg.operand.opPhyAddr;
				socReq.operands[0] = MV_SM_CEC_ACTIVE_SOURCE_EVENT;
				socReq.operands[1] = socReq.operands[2] = socReq.operands[3] = 0;               // so we don't have to clean out junk when sending to things that expect a UINT32 type code.
				//byte order swapped when received by SoC
				socReq.operands[5] = *pcPhysical;
				socReq.operands[4] = *(pcPhysical + 1);
				socReq.operands[6] = socReq.operands[7] = 0;
				socReq.length = 8;

				memcpy(&wakeup_msg, &RecvMsg, sizeof(CEC_MSG));
				mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_CEC);
				mv_sm_power_enterflow_bysmstate();
			}
		}
		break;

		/* On receiving <Set Stream Path> message:
		 * Send one touch play message.
		 */
	case CEC_MSG_OPCODE_SET_STREAM_PATH:
		if ((RecvMsg.operand.opPhyAddr & findmux(SMCECConfig.uiPhyAddr)) == SMCECConfig.uiPhyAddr)
		{
			if((MV_SM_STATE_LOWPOWERSTANDBY == iSysState) ||
				(MV_SM_STATE_NORMALSTANDBY == iSysState) ||
				(MV_SM_STATE_SUSPEND == iSysState))
			{
				//wake up system
				PRT_INFO("cec wake up:set stream path for 0x%x\n", RecvMsg.operand.opPhyAddr);
				pcPhysical = (char *) &RecvMsg.operand.opPhyAddr;
				socReq.operands[0] = MV_SM_CEC_STREAM_PATH_EVENT;
				socReq.operands[1] = socReq.operands[2] = socReq.operands[3] = 0;               // so we don't have to clean out junk when sending to things that expect a UINT32 type code.
				//byte order swapped when received by SoC
				socReq.operands[5] = *pcPhysical;
				socReq.operands[4] = *(pcPhysical + 1);
				socReq.operands[6] = socReq.operands[7] = 0;
				socReq.length = 8;
				memcpy(&wakeup_msg, &RecvMsg, sizeof(CEC_MSG));
				mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_CEC);
				mv_sm_power_enterflow_bysmstate();
			}
		}
		break;

		/* On receiving <Routing Change> message:
		 * Send one touch play message.
		 */
	case CEC_MSG_OPCODE_ROUTING_CHANGE:
#if 1
		/*
		 * Do nothing here.
		 * Disable routing change wake up for RDK DMP board.
		 */
		break;
#else
		if (RecvMsg.operand.opRoutingChange.newPhyAddr == SMCECConfig.uiPhyAddr)
		{
			if((MV_SM_STATE_LOWPOWERSTANDBY == iSysState) ||
				(MV_SM_STATE_NORMALSTANDBY == iSysState) ||
				(MV_SM_STATE_SUSPEND == iSysState))
			{
				//wake up system
				PRT_INFO("cec wake up:routing change\n");
				mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_CEC);
				mv_sm_power_enterflow_bysmstate();
			}
		}
		break;
#endif

		/******************** Standby **************************************/
		/* On receiving <Standby> message:
		 * Go to standby mode, when system is in warmup stage, APP CEC is not ready,but SOC is powered on.
		 *so system should go back to standby
		 */
	case CEC_MSG_OPCODE_STANDBY:
		if((MV_SM_STATE_LOWPOWERSTANDBY == iSysState) ||
				(MV_SM_STATE_NORMALSTANDBY == iSysState) ||
				(MV_SM_STATE_SUSPEND == iSysState))
		{
			//PRT_INFO("########standby msg##########\n");
			mv_sm_power_enterflow_bysmstate();
		}
		break;

		/******************** System Information ***************************/
		/* On receiving <Give Physical Address> message:
		 *  Broadcast <Report Physical Address> message.
		 */
	case CEC_MSG_OPCODE_GIVE_PHY_ADDR:
		SendMsg.opcode = CEC_MSG_OPCODE_REPORT_PHY_ADDR;
		SendMsg.destAddr = CEC_BROADCAST_DESTADDR; /* broadcast. */
		SendMsg.operand.opRepPhyAddr.phyAddr = SMCECConfig.uiPhyAddr;
		SendMsg.operand.opRepPhyAddr.deviceType = SMCECConfig.iDeviceType;
		sm_cec_srv_tx_msg(&SendMsg);
		break;

		/* On receiving <Get Menu Language> message:
		 *  do nothing in standby.
		 */
	case CEC_MSG_OPCODE_GET_MENU_LANG:
		featureAbortReason = CEC_ABORT_REASON_INV_MODE_TO_RESPOND;
		break;

		/* On receiving <Get CEC Version> message:*/
	case CEC_MSG_OPCODE_GET_CEC_VERSION:
		SendMsg.opcode = CEC_MSG_OPCODE_CEC_VERSION;
		SendMsg.destAddr=RecvMsg.srcAddr;
		SendMsg.operand.opVersion=CEC_VERSION;
		sm_cec_srv_tx_msg(&SendMsg);
		break;

        /******************** Vendor Commands ******************************/
#ifdef BUDDY_RELEASE
       case CEC_MSG_OPCODE_GIVE_DEVICE_VENDOR_ID:
               SendMsg.opcode = CEC_MSG_OPCODE_DEVICE_VENDOR_ID;
               SendMsg.destAddr = CEC_BROADCAST_DESTADDR; /* broadcast. */
               SendMsg.operand.opVendorID = CEC_DEVICE_ID;
               sm_cec_srv_tx_msg(&SendMsg);
               break;

       case CEC_MSG_OPCODE_VENDOR_REMOTE_BTN_UP:
       case CEC_MSG_OPCODE_VENDOR_REMOTE_BTN_DOWN:
               featureAbortReason = CEC_ABORT_REASON_UNREC_OPCODE;
               break;



       case CEC_MSG_OPCODE_VENDOR_COMMAND_WITH_ID:


#if 1 //debug tbdzz
			PRT_DEBUG("SM: cec_srv: CEC_MSG:	");
				pRecvMsg = &RecvMsg;
				pChar=(char*)pRecvMsg;
				for (i=0; i<RecvMsg.operandlen+8; i++)
				PRT_DEBUG("%02x ", pChar[i]&0xFF);
			PRT_DEBUG("\n");
#endif // debug tbdzz

			// ************* Validate the message we received ****************
			CEC_VENDOR_CMD_WITH_ID *pVCWID;
			BUDDY_FUNC_ABORT  *pBuddyFuncAbort;			// vendor-specific feature abort defined by Buddy

			#if 1 //--------------debug

			PRT_DEBUG("SMCEC2SM  CEC_MSG hdr:	src=%x, dest=%x, opcode=0x%x, oplen=%d\n",
				RecvMsg.srcAddr, RecvMsg.destAddr, RecvMsg.opcode, RecvMsg.operandlen);
			#endif //------------debug



#if 1 //test1
			//Validate the CEC_MSG portion of the message
			if (RecvMsg.srcAddr != 0)		// from someone other than the Tv?
			{
				PRT_DEBUG("SMCEC2SM: CEC_SRV: ignoring VEND_CMD_WITH_ID from logaddr=0x%x, not TV.\n", RecvMsg.srcAddr);
				break;
			}

			// ***************  Validate the VENDOR_CMD_WITH_ID portion of the message  *******************

			pVCWID = (CEC_VENDOR_CMD_WITH_ID*)&(RecvMsg.operand.opVendCmdWithID);

			if (pVCWID->buddy_opcode == (UINT8)CEC_VEND_CMD_BUDDY_GIVE_ADDL_MENU_INFO ||
				pVCWID->null_HOB_of_buddy_opcode == (UINT8)CEC_VEND_CMD_BUDDY_GIVE_ADDL_MENU_INFO)

			{
				PRT_DEBUG("\nGIVE_ADDL_MENU_INFO\n\n", &RecvMsg, (UINT32)(pVCWID), pVCWID->decrCtr);
			}


			#if 1 //--------------debug
			PRT_DEBUG("SM: cec_srv: VCWID hdr:  i/f=%02x, op=0x%x, UID=%x.%x.%x, buddyOp=0x%x, dcr=%d.\n",
				pVCWID->initfoll, pVCWID->opcode, pVCWID->vendorUID[0], pVCWID->vendorUID[1],
				pVCWID->vendorUID[2], pVCWID->buddy_opcode, pVCWID->decrCtr);
			#endif //-------------debug

			if (pVCWID->vendorUID[0] != 0x08 || pVCWID->vendorUID[1] != 0x00 || pVCWID->vendorUID[2] != 0x46)
			{
				if (prxdata->msg_cfg.mode == CEC_MODE_UNICAST)  // tolerate this in bcast, it might be intended for someone else.
				{
					PRT_DEBUG("SMCEC2SM: CEC_SRV: refusing VEND_CMD_WITH_ID from UID=%02x%02x%02x, not Buddy.\n",
						pVCWID->vendorUID[0], pVCWID->vendorUID[1], pVCWID->vendorUID[2] );
					sendBuddyVendorFeatureAbort ((enum buddy_abort_reason)(refused), &SendMsg, &RecvMsg);
				}
				else
				{
					PRT_DEBUG("SMCEC2SM: CEC_SRV: ignoring VEND_CMD_WITH_ID not from Buddy.\n");
				}
				break;
			}

			if (pVCWID->decrCtr != 0)
			{
				if (prxdata->msg_cfg.mode == CEC_MODE_UNICAST)  //don't tolerate this in unicast, but be quiet about bcast msgs with this error.
				{
					PRT_DEBUG("SMCEC2SM: CEC_SRV: rejecting VEND_CMD_WITH_ID with Decr Ctr=%d, not zero.\n",
						pVCWID->decrCtr);
					sendBuddyVendorFeatureAbort ((enum buddy_abort_reason)(decr_ctr_nonzero), &SendMsg, &RecvMsg);
				}
				else
				{
					PRT_DEBUG("SMCEC2SM: CEC_SRV: ignoring VEND_CMD_WITH_ID with Decr Ctr=%d, not zero.\n",
						pVCWID->decrCtr);
				}
				break;
			}

#endif //test1

			// validate the request by type, compose/send response.
				switch ((int)pVCWID->buddy_opcode)
				{
					case CEC_VEND_CMD_BUDDY_VEND_FEATURE_ABORT:
						PRT_DEBUG("SMCEC2SM: CEC_SRV: VENDOR_FEATURE_ABORT msg\n");

						if (prxdata->msg_cfg.mode != CEC_MODE_UNICAST)
						{
							PRT_DEBUG("SM: cec: vendorFeatureAbort must be unicast, ignoring.\n");
							return S_OK;
						}

						if (RecvMsg.operandlen != expected_length_VENDOR_FEATURE_ABORT)	// length ok for this type?
						{
							PRT_DEBUG("SMCEC2SM: CEC_SRV: ignoring VEND_FEATURE_ABORT with illegal length=%d, expected=%d.\n",
								RecvMsg.operandlen,  expected_length_VENDOR_FEATURE_ABORT);
							break;
						}



						pBuddyFuncAbort = (BUDDY_FUNC_ABORT*)&(pVCWID->vendCmd);

						offendingOpHL = (((pBuddyFuncAbort->offendingBuddyOpcodeH)<<8) + pBuddyFuncAbort->offendingBuddyOpcodeL);
						PRT_DEBUG("SMCEC2SM: CEC_SRV: Note: rec'd VENDOR_FEATURE_ABORT from 0x%x, offending opcode=0x%x, reason=0x%x.\n",
							RecvMsg.srcAddr, offendingOpHL, pBuddyFuncAbort->reason );
						break;

					case CEC_VEND_CMD_BUDDY_GIVE_ADDL_MENU_INFO:
						PRT_DEBUG("SMCEC2SM: CEC_SRV: GIVE_ADDL_MENU_INFO msg\n");

#if 1 //test1
						if (prxdata->msg_cfg.mode != CEC_MODE_BROADCAST)
						{
							PRT_DEBUG("SM: cec: GiveAddlMenuInfo must be bcast.\n");

							return S_OK;
						}
						if (prxdata->msg_len != expected_length_GIVE_ADDL_MENU_INFO )	// length ok for this type? (note going back to CEC_TASK_DATA)
						{
							PRT_DEBUG("SMCEC2SM: CEC_SRV: rejecting GIVE_ADDL_MENU_INFO with illegal length=%d, expected=%d.\n",
								prxdata->msg_len,  expected_length_GIVE_ADDL_MENU_INFO);
								break;
						}
#endif //test1
						sendReportAddlMenuInfo(&SendMsg, &RecvMsg); // format and send the report of our menu to the TV
						break;

					case CEC_VEND_CMD_BUDDY_LAUNCH_SPECIFIC_APP:
						PRT_DEBUG("SMCEC2SM: CEC_SRV: LAUNCH_SPECIFIC_APP msg\n");


						if (prxdata->msg_cfg.mode != CEC_MODE_UNICAST)
						{
							PRT_DEBUG("SM: cec: LaunchSpecificApp must be unicast, ignoring.\n");
							return S_OK;
						}

						if (prxdata->msg_len != expected_length_LAUNCH_SPECIFIC_APP )	// length ok for this type? (note going back to CEC_TASK_DATA)
						{
							PRT_DEBUG("SMCEC2SM: CEC_SRV: rejecting LAUNCH_SPECIFIC_APP with illegal length=%d, expected=%d.\n",
								prxdata->msg_len,  expected_length_LAUNCH_SPECIFIC_APP);
								break;
						}



						/*if((MV_SM_STATE_WARMDOWN == iSysState) ||
							(MV_SM_STATE_COLD == iSysState))*/
						if(MV_SM_STATE_COLD == iSysState)
						{
							// If this msg occurs and if SM CEC is active (because we're in S0/S1) then we must boot SoC and wait for
							// notification from SoC-CEC that he's ready to handle the app request. When the MV_SM_CEC_APPREADY
							// msg comes, cec_core.c MV_SM_Process_CEC_Msg() will send the saved message.
							// For reporting, we'll send the entire message up to SoC. (We know it's OK now). Last 4 bytes = App ID from TV.

							socReq.operands[0] = MV_SM_CEC_APP_REQUESTED;
							socReq.operands[1] = socReq.operands[2] = socReq.operands[3] = 0;		// so we don't have to clean out junk when sending to things that expect a UINT32 type code.
							memcpy(&socReq.operands[4], &((char*)prxdata)[4], prxdata->msg_len );	// save until SoC boots.
							socReq.length = prxdata->msg_len + 4; //adjust for type message and physical/logical info
							//wake up system
							PRT_DEBUG("SMCEC2SM: CEC_SRV: waking SoC.\n");
							memcpy(&wakeup_msg, &RecvMsg, sizeof(CEC_MSG));
							mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_CEC);
							mv_sm_power_enterflow_bysmstate();
						}

						break;
					default:
						PRT_DEBUG("SMCEC: CEC_SRV note: ignoring VEND_CMD_WITH_ID with unimplemented opcode=0x%x\n",
							RecvMsg.operand.opVendCmdWithID.buddy_opcode);
						break;
				}

            break;
#endif //BUDDY_RELEASE


		/******************** Deck Control *********************************/
		/* On receiving <Give Deck Status> message:
		 *  Send <Deck Status> to message to message sender.
		 */
	case CEC_MSG_OPCODE_GIVE_DECK_STATUS:
		//only support CEC_STATUS_REQUEST_ONCE
		if ((RecvMsg.operand.opStatusRequest == CEC_STATUS_REQUEST_OFF) ||
				(RecvMsg.operand.opStatusRequest == CEC_STATUS_REQUEST_ON))
		{
			featureAbortReason = CEC_ABORT_REASON_REFUSED;
		}
		else
		{
			SendMsg.opcode = CEC_MSG_OPCODE_DECK_STATUS;
			SendMsg.destAddr=RecvMsg.srcAddr;
			SendMsg.operand.opDeckInfo = CEC_DECK_INFO_STOP;
			sm_cec_srv_tx_msg(&SendMsg);
		}
		break;

		/* On receiving <Deck Control> message:
		 *  Perform deck control action.
		 */
	case CEC_MSG_OPCODE_DECK_CONTROL:
#ifndef BUDDY_RELEASE
		if(CEC_DECK_CTRL_MODE_EJECT==RecvMsg.operand.opDeckControlMode)
		{
#ifdef LOADEREnable
			//loader operation
			MV_SM_Loader_FastEjectKeyHandle( );
#endif
			if((MV_SM_STATE_LOWPOWERSTANDBY == iSysState) ||
				(MV_SM_STATE_NORMALSTANDBY == iSysState) ||
				(MV_SM_STATE_SUSPEND == iSysState))
			{
				//wake up system
				memcpy(&wakeup_msg, &RecvMsg, sizeof(CEC_MSG));
				mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_CEC);
				mv_sm_power_enterflow_bysmstate();
			}
		}
		else
		{
			featureAbortReason = CEC_ABORT_REASON_INV_MODE_TO_RESPOND;
		}
#else //BUDDY_RELEASE
        featureAbortReason = CEC_ABORT_REASON_INV_MODE_TO_RESPOND;
#endif //BUDDY_RELEASE
		break;

		/* On receiving <Play> message:
		 * perform play action
		 */
	case CEC_MSG_OPCODE_PLAY:
#ifndef BUDDY_RELEASE
		if(CEC_PLAY_MODE_FORWARD==RecvMsg.operand.opPlayMode)
		{
			if((MV_SM_STATE_LOWPOWERSTANDBY == iSysState) ||
				(MV_SM_STATE_NORMALSTANDBY == iSysState) ||
				(MV_SM_STATE_SUSPEND == iSysState))
			{
				//wake up system
				memcpy(&wakeup_msg, &RecvMsg, sizeof(CEC_MSG));
				mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_CEC);
				mv_sm_power_enterflow_bysmstate();
			}
		}
#else //BUDDY_RELEASE
        featureAbortReason = CEC_ABORT_REASON_INV_MODE_TO_RESPOND;
#endif //BUDDY_RELEASE^
		break;

		/******************** OSD Display *********************************/
		/* On receiving <Give OSD Name> message:
		 *Send <Set OSD Name>
		 */
	case CEC_MSG_OPCODE_GIVE_OSD_NAME:
		SendMsg.opcode = CEC_MSG_OPCODE_SET_OSD_NAME;
		SendMsg.destAddr=RecvMsg.srcAddr;
		SendMsg.operand.opDevOsdName.osdNameLen = OSD_NAME_MAX_LENGTH;
		memcpy(SendMsg.operand.opDevOsdName.osdName,cec_osd_name,OSD_NAME_MAX_LENGTH);
		sm_cec_srv_tx_msg(&SendMsg);
		break;

		/******************** Device Menu Control *************************/
	case CEC_MSG_OPCODE_MENU_REQUEST:
		featureAbortReason = CEC_ABORT_REASON_INV_MODE_TO_RESPOND;
		break;
		/* On receiving <User control pressed> and <User Control Released>message:
		 * pass through user key to UI manager
		 */
		//TBD, consider the case: TV lost release key
	case CEC_MSG_OPCODE_USER_CONTROL_PRESSED:
#ifndef BUDDY_RELEASE
		if(RecvMsg.operandlen==sizeof(CEC_OPERAND_USERCTL_PLAYEXT))
		{
			if((CEC_UC_PLAY_FN==RecvMsg.operand.opUserControlCode.opUserControlPlayext.uccode)
					&&(CEC_PLAY_MODE_FORWARD==RecvMsg.operand.opUserControlCode.opUserControlPlayext.playmode))
			{
				if((MV_SM_STATE_LOWPOWERSTANDBY == iSysState) ||
					(MV_SM_STATE_NORMALSTANDBY == iSysState) ||
					(MV_SM_STATE_SUSPEND == iSysState))
				{
					//wake up system
					memcpy(&wakeup_msg, &RecvMsg, sizeof(CEC_MSG));
					mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_CEC);
					mv_sm_power_enterflow_bysmstate();
				}
			}
			else
				featureAbortReason = CEC_ABORT_REASON_INV_MODE_TO_RESPOND;
		}
#else //BUDDY_RELEASE
		if(RecvMsg.operandlen==sizeof(CEC_OPERAND_USERCTL_PLAYEXT))
		{
			featureAbortReason = CEC_ABORT_REASON_REFUSED;
		}
#endif //BUDDY_RELEASE
		else
		{
			switch(RecvMsg.operand.opUserControlCode.opUserControlCodeonebyte)
			{
#ifndef BUDDY_RELEASE
			case CEC_UC_EJECT:
#ifdef LOADEREnable
				//loader operation
				MV_SM_Loader_FastEjectKeyHandle( );
#endif
#endif //BUDDY_RELEASE
			case CEC_UC_PLAY_FN:
			case CEC_UC_POWER:
			case CEC_UC_PLAY:
			case CEC_UC_POWER_TOGGLE:
			case CEC_UC_POWER_ON:
				if((MV_SM_STATE_LOWPOWERSTANDBY == iSysState) ||
					(MV_SM_STATE_NORMALSTANDBY == iSysState) ||
					(MV_SM_STATE_SUSPEND == iSysState))
				{
					PRT_INFO("Wake up SoC, <User Control Pressed>\n");
					socReq.operands[0] = MV_SM_CEC_UC_PRESSED_EVENT;
					socReq.operands[1] = socReq.operands[2] = socReq.operands[3] = 0;               // so we don't have to clean out junk when sending to things that expect a UINT32 type code.
				//byte order swapped when received by SoC
					socReq.length = 4;
					//wake up system
					memcpy(&wakeup_msg, &RecvMsg, sizeof(CEC_MSG));
					mv_sm_power_setwakeupsource(MV_SM_WAKEUP_SOURCE_CEC);
					mv_sm_power_enterflow_bysmstate();
				}
				break;

			default:
#ifndef BUDDY_RELEASE
				featureAbortReason = CEC_ABORT_REASON_INV_MODE_TO_RESPOND;
#else //BUDDY_RELEASE
		                featureAbortReason = CEC_ABORT_REASON_REFUSED;
#endif //BUDDY_RELEASE
				break;
			}
		}
		break;

	case CEC_MSG_OPCODE_USER_CONTROL_RELEASED:
		break;

		/******************** Device Power Status **************************/
		/* On receiving <Give device power status> message:
		 *  Give device power status.
		 */
	case CEC_MSG_OPCODE_GIVE_DEVICE_POWER_STATUS:
		SendMsg.opcode = CEC_MSG_OPCODE_REPORT_POWER_STATUS;
		SendMsg.destAddr=RecvMsg.srcAddr;
		if((MV_SM_STATE_ACTIVE_2_LOWPOWERSTANDBY == iSysState) ||
			(MV_SM_STATE_ACTIVE_2_NORMALSTANDBY == iSysState) ||
			(MV_SM_STATE_ACTIVE_2_SUSPEND == iSysState))
			SendMsg.operand.opPwrSts = CEC_PWR_STS_IN_TRANS_ON_TO_STANDBY;
		else if((MV_SM_STATE_LOWPOWERSTANDBY == iSysState) ||
				(MV_SM_STATE_NORMALSTANDBY == iSysState) ||
				(MV_SM_STATE_SUSPEND == iSysState))
			SendMsg.operand.opPwrSts =CEC_PWR_STS_STANDBY;
		else
			SendMsg.operand.opPwrSts =CEC_PWR_STS_IN_TRANS_STANDBY_TO_ON;
		sm_cec_srv_tx_msg(&SendMsg);
		break;

	default:
		// Retcode unsupported would fall in here
		featureAbortReason = CEC_ABORT_REASON_UNREC_OPCODE;
		break;
	}
	if (featureAbortReason != -1)
	{
		SendMsg.opcode = CEC_MSG_OPCODE_FEATURE_ABORT;
		SendMsg.destAddr=RecvMsg.srcAddr;
		SendMsg.operand.opFeatureAbort.featureOpcode = RecvMsg.opcode;
		SendMsg.operand.opFeatureAbort.abortReason = featureAbortReason;
		sm_cec_srv_tx_msg(&SendMsg);
	}

	return S_OK;
}

UINT8 cec_add_app_cmd (CEC_TASK_DATA *pData);
/******************************************************************************
 * FUNCTION : Send message
 * PARAMS   : ptxmsg    -Pointer to the message
 *  RETURN   : S_OK, succeed; S_FALSE, fail
 ******************************************************************************/
hresult sm_cec_srv_tx_msg (CEC_MSG *ptxmsg)
{
	int i = 0;
	CEC_TASK_DATA txdata;

	ptxmsg->srcAddr = SMCECConfig.iLogicAddr;
	CEC_SRV_FormatConvert(&txdata,ptxmsg,TRUE);
	txdata.txtime = mv_sm_timer_gettimems();

	PRT_DEBUG("[TX MSG]\n");
	PRT_DEBUG("      cmd: 0x%02x, cfg: 0x%02x, status: 0x%02x, txtime: %d\n",
		   txdata.cmd, txdata.msg_cfg, txdata.status, txdata.txtime);
	PRT_DEBUG("      msg_fifo: ");
	for (i = 0; i < txdata.msg_len; i++)
		PRT_DEBUG("0x%02x ", txdata.msg_fifo[i]);
	PRT_DEBUG("\n");

	txdata.cmd = CEC_CMD_TX_MSG;
	cec_add_app_cmd(&txdata);

	return S_OK;
}

#endif // CECENABLE
