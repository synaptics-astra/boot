// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013~2023 Synaptics Incorporated. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 or
 * later as published by the Free Software Foundation.
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
#include "sm_type.h"
#include "sm_common.h"
#include "sm_io.h"
#include "sm_memmap.h"
#include "SysMgr.h"
#include "string.h"

#include "platform_config.h"
#include "sm_apb_gpio_defs.h"
#include "sm_apb_ictl_defs.h"

#include "sm_comm.h"
#include "sm_timer.h"
#include "sm_gpio.h"
#include "sm_state.h"
#include "sm_power.h"

#include "sm_printf.h"

#include "sm_rt_module.h"
#include "sm_exception.h"

#include "cec_type.h"
#include "cec_prv.h"
#include "sm_cec.h"

#include "cec_reg_map.h"

UINT32 rememberHdmiCtrlState;
SOC_REQ_AT_STARTUP socReq;	// from cec_src, when he parses a request from TV
CEC_MSG wakeup_msg;
CECConfig SMCECConfig;
static BOOL bCECTaskExit = TRUE;

// select a message table, defining which messages we handle,  based on HDMI Control messages from SoC
extern const CEC_MSG_TABLE *SMCECMsgTable;
extern INT32 cec_msg_table_size;
char cec_osd_name[OSD_NAME_MAX_LENGTH + 1];
INT32 cec_host_type;

const CEC_MSG_TABLE SMCECMsgTable_TV[] =
{
	{CEC_MSG_OPCODE_FEATURE_ABORT,              2,  CEC_MODE_UNICAST,       FALSE},
	{CEC_MSG_OPCODE_ABORT,                      0,  CEC_MODE_UNICAST,       TRUE},
//	{CEC_MSG_OPCODE_ACTIVE_SOURCE,              2,  CEC_MODE_BROADCAST,   FALSE},
	{CEC_MSG_OPCODE_IMAGE_VIEW_ON,              0,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_TEXT_VIEW_ON,               0,  CEC_MODE_UNICAST,       TRUE},
//	{CEC_MSG_OPCODE_REQUEST_ACTIVE_SOURCE,      0,  CEC_MODE_BROADCAST,     FALSE},
	{CEC_MSG_OPCODE_INACTIVE_SOURCE,            2,  CEC_MODE_UNICAST,       TRUE},
//	{CEC_MSG_OPCODE_ROUTING_CHANGE,             4,  CEC_MODE_BROADCAST,     FALSE},
	{CEC_MSG_OPCODE_STANDBY,                    0,  CEC_MODE_UNICAST,       FALSE},
	{CEC_MSG_OPCODE_GET_MENU_LANG,              0,  CEC_MODE_UNICAST,       TRUE},
//	{CEC_MSG_OPCODE_SET_MENU_LANG,              3,  CEC_MODE_BROADCAST,     FALSE},
	{CEC_MSG_OPCODE_GIVE_PHY_ADDR,              0,  CEC_MODE_UNICAST,       TRUE},
//	{CEC_MSG_OPCODE_REPORT_PHY_ADDR,            3,  CEC_MODE_BROADCAST,     FALSE},
	{CEC_MSG_OPCODE_GET_CEC_VERSION,            0,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_SET_OSD_NAME,               1,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_USER_CONTROL_PRESSED,       1,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_USER_CONTROL_RELEASED,      0,  CEC_MODE_UNICAST,       FALSE},
	{CEC_MSG_OPCODE_GIVE_DEVICE_POWER_STATUS,   0,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_REPORT_POWER_STATUS,        1,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_REPORT_AUDIO_STATUS,        1,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_SET_SYS_AUDIO_MODE,         1,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_MENU_STATUS,                1,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_DECK_STATUS,                1,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_RECORD_STATUS,              1,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_RECORD_TV_SCREEN,           0,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_TUNER_DEVICE_STATUS,        5,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_TIMER_STATUS,               1,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_TIMER_CLEARED_STATUS,       1,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_GIVE_DEVICE_VENDOR_ID,      0,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_VENDOR_COMMAND,             1,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_VENDOR_COMMAND_WITH_ID,     3,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_VENDOR_REMOTE_BTN_UP,       0,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_VENDOR_REMOTE_BTN_DOWN,     1,  CEC_MODE_UNICAST,       TRUE}
};

const CEC_MSG_TABLE SMCECMsgTable_BOX[] =
{
	{CEC_MSG_OPCODE_FEATURE_ABORT,              2,  CEC_MODE_UNICAST,       FALSE},
	{CEC_MSG_OPCODE_ABORT,                      0,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_ACTIVE_SOURCE,              2,  CEC_MODE_BROADCAST,     TRUE},
	{CEC_MSG_OPCODE_IMAGE_VIEW_ON,              0,  CEC_MODE_UNICAST,       FALSE},
	{CEC_MSG_OPCODE_REQUEST_ACTIVE_SOURCE,      0,  CEC_MODE_BROADCAST,     FALSE},
	{CEC_MSG_OPCODE_SET_STREAM_PATH,            2,  CEC_MODE_BROADCAST,     TRUE},
	{CEC_MSG_OPCODE_ROUTING_CHANGE,             4,  CEC_MODE_BROADCAST,     TRUE},
	{CEC_MSG_OPCODE_ROUTING_INFO,               2,  CEC_MODE_BROADCAST,     FALSE},
	{CEC_MSG_OPCODE_INACTIVE_SOURCE,            2,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_STANDBY,                    0,  (CEC_MODE_BROADCAST|CEC_MODE_UNICAST), FALSE},
	{CEC_MSG_OPCODE_GET_MENU_LANG,              0,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_SET_MENU_LANG,              3,  CEC_MODE_BROADCAST,     FALSE},
	{CEC_MSG_OPCODE_GIVE_PHY_ADDR,              0,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_REPORT_PHY_ADDR,            3,  CEC_MODE_BROADCAST,     FALSE},
	{CEC_MSG_OPCODE_CEC_VERSION,                1,  CEC_MODE_UNICAST,       FALSE},
	{CEC_MSG_OPCODE_GET_CEC_VERSION,            0,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_DECK_STATUS,                1,  CEC_MODE_UNICAST,       FALSE},
	{CEC_MSG_OPCODE_GIVE_DECK_STATUS,           1,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_DECK_CONTROL,               1,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_PLAY,                       1,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_GIVE_OSD_NAME,              0,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_SET_OSD_NAME,               1,  CEC_MODE_UNICAST,       FALSE},
	{CEC_MSG_OPCODE_USER_CONTROL_PRESSED,       1,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_USER_CONTROL_RELEASED,      0,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_MENU_REQUEST,               1,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_MENU_STATUS,                1,  CEC_MODE_UNICAST,       FALSE},
	{CEC_MSG_OPCODE_GIVE_DEVICE_POWER_STATUS,   0,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_REPORT_POWER_STATUS,        1,  CEC_MODE_UNICAST,       FALSE},
	{CEC_MSG_OPCODE_GIVE_DEVICE_VENDOR_ID,      0,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_VENDOR_REMOTE_BTN_UP,       0,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_VENDOR_REMOTE_BTN_DOWN,     1,  CEC_MODE_UNICAST,       TRUE},
	{CEC_MSG_OPCODE_VENDOR_COMMAND_WITH_ID,     3,  CEC_MODE_UNICAST,       TRUE}
	// exact length of VEND_CMD_W_ID depends on subtype so let it appear small so it doesn't die due to
	// general-purpose length check early in RX. unicast/bcast depends on subtype also, so value here is ignored.
};


hresult (*mv_sm_cec_srv_rx_msg)(CEC_TASK_DATA *prxdata);

//#define  TEST_SMTVCEC_WAKEUP     1
//#define  CEC_TV_BG2              1

hresult sm_send_msg(INT32 moduleID, unsigned char * pMsgBody, INT32 len);

/*-----------------------------------------------------------------------------
 * CEC fifo interfaces
 *-----------------------------------------------------------------------------
 */
#define CEC_WRITE_ISR_FIFO(pData)   cec_write_fifo(&stCec_Isr_Fifo,pData)
#define CEC_READ_ISR_FIFO(pData)    cec_read_fifo(&stCec_Isr_Fifo,pData)
#define CEC_WRITE_APP_FIFO(pData)   cec_write_fifo(&stCec_App_Fifo,pData)
#define CEC_READ_APP_FIFO(pData)    cec_read_fifo(&stCec_App_Fifo,pData)
/**********************************************************************
 * FUNCTION     : Write given data to fifo
 * PARAMETERS   : pFifo - Pointer to FIFO
 *              : pData - Pointer to data to write (input)
 * RETURN       : TRUE - if written successfully, FALSE, otherwise
 *********************************************************************/
static UINT8 cec_write_fifo(CEC_TASK_DATA_FIFO *pFifo, CEC_TASK_DATA *pData)
{
	if (CEC_FIFO_WRAP_AROUND(pFifo->ucEnd_index+1) == pFifo->ucStart_index)
		return FALSE; // No space

	memcpy((UINT8*)&pFifo->ucFifo_buf[pFifo->ucEnd_index],
			(UINT8*)pData, sizeof(CEC_TASK_DATA));
	pFifo->ucEnd_index = CEC_FIFO_WRAP_AROUND(pFifo->ucEnd_index+1);
	return TRUE;
}

/**********************************************************************
 * FUNCTION     : Read data from fifo
 * PARAMETERS   : pFifo - Pointer to FIFO
 *              : pData - Pointer to fill data read (output)
 * RETURN       : TRUE - if written successfully, FALSE, otherwise
 *********************************************************************/
static UINT8 cec_read_fifo(CEC_TASK_DATA_FIFO *pFifo, CEC_TASK_DATA *pData)
{
	if (pFifo->ucStart_index == pFifo->ucEnd_index)
		return FALSE; // Empty

	memcpy((UINT8*)pData, (UINT8*)&pFifo->ucFifo_buf[pFifo->ucStart_index],
			sizeof(CEC_TASK_DATA));
	pFifo->ucStart_index = CEC_FIFO_WRAP_AROUND(pFifo->ucStart_index+1);
	return TRUE;
}
/**********************************************************************
 * FUNCTION     : clear fifo
 * PARAMETERS   : pFifo - Pointer to FIFO
 *
 * RETURN       : TRUE - if written successfully, FALSE, otherwise
 *********************************************************************/
static void cec_clear_fifo(CEC_TASK_DATA_FIFO *pFifo)
{
	pFifo->ucStart_index = 0;
	pFifo->ucEnd_index =0;
}

/**********************************************************************
 * FUNCTION     : Clears transmit/receive FIFO
 * PARAMETERS   : pData - tx/rx data to be cleared
 * RETURN       : None
 *********************************************************************/
static void cec_clear_data(CEC_TASK_DATA *pData)
{
	INT32 i;

	for (i = 0; i < 16; i++)
	{
		pData->msg_fifo[i] = 0;
	}
	pData->msg_cfg.mode = 0;
	pData->msg_cfg.retry = 0;
	pData->status = 0;
	pData->msg_len = 0;
}

/**********************************************************************
 * FUNCTION     : Enable/Disable CEC Interrupt (primary)
 * PARAMETERS   : bEnable(1: enable;0 disable)
 * RETURN       : None
 *********************************************************************/
static void cec_enable_global_interrupt( BOOL bEnable)
{
	UINT32 val;
	UINT32 mask;

	val = MV_SM_READ_REG32(APB_ICTL_IRQ_INTEN);
	mask = MV_SM_READ_REG32(APB_ICTL_IRQ_INTMASK);
	PRT_DEBUG("APB_ICTL_IRQ_INT(EN,MASK): 0x%x 0x%x-> ", val, mask);
	if (bEnable) {
		val |= (1 << SMICTL_IRQ_CEC);
		val |= (1 << SMICTL_IRQ_FIFO_CEC);
                mask &= ~(1 << SMICTL_IRQ_CEC);
                mask &= ~(1 << SMICTL_IRQ_FIFO_CEC);
	} else {
		val &= ~(1 << SMICTL_IRQ_CEC);
		val &= ~(1 << SMICTL_IRQ_FIFO_CEC);
                mask |= (1 << SMICTL_IRQ_CEC);
                mask |= (1 << SMICTL_IRQ_FIFO_CEC);
	}
	MV_SM_WRITE_REG32(APB_ICTL_IRQ_INTEN, val);
	MV_SM_WRITE_REG32(APB_ICTL_IRQ_INTMASK, mask);
	val = MV_SM_READ_REG32(APB_ICTL_IRQ_INTEN);
	PRT_DEBUG("0x%x 0x%x\n", val, mask);

	val = MV_SM_READ_REG32(APB_ICTL1_IRQ_INTEN);
	mask = MV_SM_READ_REG32(APB_ICTL1_IRQ_INTMASK);
	PRT_DEBUG("APB_ICTL1_IRQ_INT(EN,MASK): 0x%x 0x%x-> ", val, mask);
	if (bEnable){
		val |= 0x10000;
        mask &= ~0x10000;
    }
	else{
		val &= ~0x10000;
        mask |= 0x10000;
    }
	MV_SM_WRITE_REG32(APB_ICTL1_IRQ_INTEN, val);
	MV_SM_WRITE_REG32(APB_ICTL1_IRQ_INTMASK, mask);
	val = MV_SM_READ_REG32(APB_ICTL1_IRQ_INTEN);
	PRT_DEBUG("0x%x 0x%x\n", val, mask);
}

/**********************************************************************
 * FUNCTION     : Add CEC command to CEC task
 * PARAMETERS   : pData - command data structure
 * RETURN       : None
 *********************************************************************/
UINT8 cec_add_app_cmd (CEC_TASK_DATA *pData)
{
	return (CEC_WRITE_APP_FIFO(pData));
}

#if (BERLIN_CHIP_VERSION >= BERLIN_BG2CT_A0)
static void cec_reg_write(UINT32 offset, UINT8 data)
{
	MV_SM_WRITE_REG32(SM_CEC_BASE + (offset << 2), data);
}

#ifdef HWCEC
static UINT8 cec_reg_read(UINT32 offset)
{
	return MV_SM_READ_REG32(SM_CEC_BASE + (offset << 2));
}

static void cec_reg_bulk_write(UINT32 offset, UINT8 *pdata, UINT32 nb)
{
	UINT32 i;
	UINT32 addr;

	addr = SM_CEC_BASE + (offset << 2);
	for (i = 0; i < nb; i++) {
		MV_SM_WRITE_REG32(addr + 4 * i, *pdata);
		pdata++;
	}
}
#endif

static void cec_hw_disable(void)
{
	cec_reg_write(CEC_RDY_ADDR, 0);
	cec_reg_write(CEC_RX_RDY_ADDR, 0);
}
#endif

#define CEC_INTR_NONE			0x0000
#define CEC_INTR_TX_UNICAST_NOACK	0x0001
#define CEC_INTR_TX_BRDCAST_NOACK	0x0002
#define CEC_INTR_TX_COLLISION		0x0004
#define CEC_INTR_TX_SFT_FAIL		0x2008
#define CEC_INTR_TX_FAIL		0x200F
#define CEC_INTR_TX_COMPLETE		0x0010
#define CEC_INTR_RX_COMPLETE		0x0020
#define CEC_INTR_RX_LOW_DT_ERROR	0x0040
#define CEC_INTR_RX_HIGH_DT_ERROR	0x0080
#define CEC_INTR_RX_FAIL		0x00C0
#define CEC_INTR_TX_FIFO_FULL		0x0100
#define CEC_INTR_TX_FIFO_EMPTY		0x0200
#define CEC_INTR_RX_FIFO_FULL		0x0400
#define CEC_INTR_RX_FIFO_EMPTY		0x0800
#define CEC_INTR_WAKEUP			0x1000
#define CEC_INTR_ALL	(CEC_INTR_TX_FAIL       |\
			 CEC_INTR_TX_COMPLETE   |\
			 CEC_INTR_RX_COMPLETE   |\
			 CEC_INTR_RX_FAIL       |\
			 CEC_INTR_TX_FIFO_FULL  |\
			 CEC_INTR_TX_FIFO_EMPTY |\
			 CEC_INTR_RX_FIFO_FULL  |\
			 CEC_INTR_RX_FIFO_EMPTY |\
			 CEC_INTR_WAKEUP)

#if 0
static void dump_cec_reg(void)
{
	PRT_INFO("%s\n", __func__);
	PRT_INFO("=============\n");
	PRT_INFO("0x%04x CEC_SIGNAL_FREE_TIME_0_ADDR: 0x%x\n",
		   CEC_SIGNAL_FREE_TIME_0_ADDR,
		   cec_reg_read(CEC_SIGNAL_FREE_TIME_0_ADDR));
	PRT_INFO("0x%04x CEC_SIGNAL_FREE_TIME_1_ADDR: 0x%x\n",
		   CEC_SIGNAL_FREE_TIME_1_ADDR,
		   cec_reg_read(CEC_SIGNAL_FREE_TIME_1_ADDR));
	PRT_INFO("0x%04x CEC_SIGNAL_FREE_TIME_2_ADDR: 0x%x\n",
		   CEC_SIGNAL_FREE_TIME_2_ADDR,
		   cec_reg_read(CEC_SIGNAL_FREE_TIME_2_ADDR));
	PRT_INFO("0x%04x CEC_SIGNAL_FREE_TIME_3_ADDR: 0x%x\n",
		   CEC_SIGNAL_FREE_TIME_3_ADDR,
		   cec_reg_read(CEC_SIGNAL_FREE_TIME_3_ADDR));
	PRT_INFO("0x%04x CEC_INTR_STATUS0_REG_ADDR: 0x%02x\n",
		   CEC_INTR_STATUS0_REG_ADDR,
		   cec_reg_read(CEC_INTR_STATUS0_REG_ADDR));
	PRT_INFO("0x%04x CEC_INTR_STATUS1_REG_ADDR: 0x%02x\n",
		   CEC_INTR_STATUS1_REG_ADDR,
		   cec_reg_read(CEC_INTR_STATUS1_REG_ADDR));
	PRT_INFO("0x%04x CEC_LOGICAL_ADDR0_REG_ADDR: 0x%02x\n",
		   CEC_LOGICAL_ADDR0_REG_ADDR,
		   cec_reg_read(CEC_LOGICAL_ADDR0_REG_ADDR));
	PRT_INFO("0x%04x CEC_LOGICAL_ADDR1_REG_ADDR: 0x%02x\n",
		   CEC_LOGICAL_ADDR1_REG_ADDR,
		   cec_reg_read(CEC_LOGICAL_ADDR1_REG_ADDR));
	PRT_INFO("0x%04x CEC_LOGICAL_ADDR2_REG_ADDR: 0x%02x\n",
		   CEC_LOGICAL_ADDR2_REG_ADDR,
		   cec_reg_read(CEC_LOGICAL_ADDR2_REG_ADDR));
	PRT_INFO("0x%04x CEC_LOGICAL_ADDR3_REG_ADDR: 0x%02x\n",
		   CEC_LOGICAL_ADDR3_REG_ADDR,
		   cec_reg_read(CEC_LOGICAL_ADDR3_REG_ADDR));
	PRT_INFO("0x%04x CEC_LOGICAL_ADDR4_REG_ADDR: 0x%02x\n",
		   CEC_LOGICAL_ADDR4_REG_ADDR,
		   cec_reg_read(CEC_LOGICAL_ADDR4_REG_ADDR));
}

static void dump_cec_data(CEC_TASK_DATA *p)
{
	UINT32 i;
	PRT_INFO("%s\n", __func__);
	PRT_INFO("=============\n");
	PRT_INFO("cmd: %d\n", p->cmd);
	PRT_INFO("cfg.mode: %d\n", p->msg_cfg.mode);
	PRT_INFO("cfg.retry: %d\n", p->msg_cfg.retry);
	PRT_INFO("status: %d\n", p->status);
	PRT_INFO("msg_len: %d\n", p->msg_len);
	PRT_INFO("msg_fifo (hex): ");
	for (i = 0; i < p->msg_len; i++)
		PRT_INFO("%02x ", p->msg_fifo[i]);
	PRT_INFO("\n");
	PRT_INFO("txtime: %d\n", p->txtime);
}
#endif

static void cec_hw_enable_interrupt(UINT16 intr, BOOL enable)
{
	UINT8 reg;
	reg = cec_reg_read(CEC_INTR_ENABLE0_REG_ADDR);
	if (enable) {
		reg = cec_reg_read(CEC_INTR_ENABLE0_REG_ADDR);
		reg |= (intr & 0x00ff);
		cec_reg_write(CEC_INTR_ENABLE0_REG_ADDR, reg);
		reg = cec_reg_read(CEC_INTR_ENABLE1_REG_ADDR);
		reg |= ((intr >> 8) & 0x00ff);
		cec_reg_write(CEC_INTR_ENABLE1_REG_ADDR, reg);
	} else {
		reg = cec_reg_read(CEC_INTR_ENABLE0_REG_ADDR);
		reg &= ~(intr & 0x00ff);
		cec_reg_write(CEC_INTR_ENABLE0_REG_ADDR, reg);
		reg = cec_reg_read(CEC_INTR_ENABLE1_REG_ADDR);
		reg &= ~((intr >> 8) & 0x00ff);
		cec_reg_write(CEC_INTR_ENABLE1_REG_ADDR, reg);
	}
}

static void cec_hw_set_mode(CEC_MODE mode, BOOL enable)
{
	if (mode == CEC_MODE_TX) {
		if (enable) {
			cec_reg_write(CEC_TX_FIFO_RESET_ADDR, 0);
			cec_reg_write(CEC_TX_FIFO_RESET_ADDR, 1);
			cec_hw_enable_interrupt(CEC_INTR_TX_FAIL | CEC_INTR_TX_COMPLETE, FALSE);
			cec_hw_enable_interrupt(CEC_INTR_TX_FAIL | CEC_INTR_TX_COMPLETE, TRUE);
		} else {
			cec_reg_write(CEC_RDY_ADDR, 0);
		}
	} else if (mode == CEC_MODE_RX) {
		if (enable) {
			cec_reg_write(CEC_RX_FIFO_RESET_ADDR, 0);
			cec_reg_write(CEC_RX_FIFO_RESET_ADDR, 1);
			cec_reg_write(CEC_RX_RDY_ADDR, 1);
			cec_hw_enable_interrupt(CEC_INTR_RX_FAIL | CEC_INTR_RX_COMPLETE, FALSE);
			cec_hw_enable_interrupt(CEC_INTR_RX_FAIL | CEC_INTR_RX_COMPLETE, TRUE);
		} else {
			cec_reg_write(CEC_RX_RDY_ADDR, 0);
		}
	} else {
		PRT_INFO("%s, undefined mode.\n", __func__);
	}
}

static void cec_hw_enable_logic_addr(BOOL enable, UINT8 index, UINT8 logic_addr)
{
	UINT32 reg;
	switch (index) {
	case 0: reg = CEC_LOGICAL_ADDR0_REG_ADDR; break;
	case 1: reg = CEC_LOGICAL_ADDR1_REG_ADDR; break;
	case 2: reg = CEC_LOGICAL_ADDR2_REG_ADDR; break;
	case 3: reg = CEC_LOGICAL_ADDR3_REG_ADDR; break;
	case 4: reg = CEC_LOGICAL_ADDR4_REG_ADDR; break;
	default:
		PRT_INFO("%s, use default.\n", __func__);
		reg = CEC_LOGICAL_ADDR2_REG_ADDR;
		break;
	}

	if (enable)
		logic_addr |= (1 << 4);
	else
		logic_addr = 0;
	cec_reg_write(reg, logic_addr);
}

static void cec_hw_set_logicaddr(INT32 device_type, INT32 addr)
{
	if (device_type < CEC_DEVICE_RES)
		cec_hw_enable_logic_addr(TRUE, device_type, addr);
	else
		cec_hw_enable_logic_addr(TRUE, device_type - 1, addr);
}

/* XXX load default value */
static void cec_hw_reset(void)
{
	UINT32 reg;

	// Write signal free time count
	reg = CEC_SIGNAL_FREE_TIME * SM_SYSTEM_HZ;
	cec_reg_bulk_write(CEC_SIGNAL_FREE_TIME_0_ADDR, (UINT8 *)&reg, 4);

	// Write start bit low duration
	reg = CEC_START_BIT_LOW_TIME * SM_SYSTEM_HZ;
	cec_reg_bulk_write(CEC_START_BIT_LO_THRESH_0_ADDR, (UINT8 *)&reg, 4);

	// Write start bit total duration
	reg = CEC_START_BIT_TOT_TIME * SM_SYSTEM_HZ;
	cec_reg_bulk_write(CEC_START_BIT_HI_THRESH_0_ADDR, (UINT8 *)&reg, 4);

	// Write data bit 0 low duration
	reg = CEC_DATA_BIT_0_LOW_TIME * SM_SYSTEM_HZ;
	cec_reg_bulk_write(CEC_DATA_BIT_0_LO_THRESH_0_ADDR, (UINT8 *)&reg, 4);

	// Write data bit 1 low duration
	reg = CEC_DATA_BIT_1_LOW_TIME * SM_SYSTEM_HZ;
	cec_reg_bulk_write(CEC_DATA_BIT_1_LO_THRESH_0_ADDR, (UINT8 *)&reg, 4);

	// Write data bit 0 total duration
	reg = CEC_DATA_BIT_TOT_TIME * SM_SYSTEM_HZ;
	cec_reg_bulk_write(CEC_DATA_BIT_0_HI_THRESH_0_ADDR, (UINT8 *)&reg, 4);

	// Write data bit 1 total duration
	reg = CEC_DATA_BIT_TOT_TIME * SM_SYSTEM_HZ;
	cec_reg_bulk_write(CEC_DATA_BIT_1_HI_THRESH_0_ADDR, (UINT8 *)&reg, 4);

	// Write safe sampling time for ACK
	reg = CEC_ACK_SSP_TIME * SM_SYSTEM_HZ;
	cec_reg_bulk_write(CEC_SSP_ACK_TIME_0_ADDR, (UINT8 *)&reg, 4);

	// Write nominal sampling time for data bit
	reg = CEC_NOMINAL_SAMPLE_TIME * SM_SYSTEM_HZ;
	cec_reg_bulk_write(CEC_NOMINAL_SAMPLE_TIME_0_ADDR, (UINT8 *)&reg, 4);

	// Write hysterisis time count
	reg = CEC_FOLL_HYST_TIME * SM_SYSTEM_HZ;
	cec_reg_bulk_write(CEC_HYST_TIME_0_ADDR, (UINT8 *)&reg, 4);

	// Write follower ack time count
	reg = CEC_FOLL_ACK_ASSRT_TIME * SM_SYSTEM_HZ;
	cec_reg_bulk_write(CEC_FOLLOWER_ACK_TIME_0_ADDR, (UINT8 *)&reg, 4);

	// Write collision window time count
	reg = CEC_COLL_WINDOW_TIME * SM_SYSTEM_HZ;
	cec_reg_bulk_write(CEC_COLL_WINDOW_TIME_REG_0_ADDR, (UINT8 *)&reg, 4);

	// Enable collision detection
	cec_reg_write(CEC_COLL_CTRL_REG_ADDR, 0x01);

	// Write start bit jitter count
	reg = CEC_START_BIT_JITTER_TIME * SM_SYSTEM_HZ;
	cec_reg_bulk_write(CEC_JITTER_CNT_SB_0, (UINT8 *)&reg, 4);

	// Write data bit jitter count
	reg = CEC_DATA_BIT_JITTER_TIME * SM_SYSTEM_HZ;
	cec_reg_bulk_write(CEC_JITTER_CNT_0_ADDR, (UINT8 *)&reg, 4);

	// Write error notification time
	reg = CEC_BIT_ERR_NOTIF_TIME * SM_SYSTEM_HZ;
	cec_reg_bulk_write(CEC_ERR_NOTIF_TIME_0, (UINT8 *)&reg, 4);

	cec_reg_write(CEC_GLITCH_FILT_W_L, 0x04);

	// Disable all logical addresses
//	for (i = 0; i < 5; i++)
//		cec_hw_enable_logic_addr(FALSE, i, 0);

	cec_reg_write(CEC_PMODE_ADDR, 0);
	cec_hw_enable_interrupt(CEC_INTR_ALL, FALSE);
	cec_hw_enable_interrupt(CEC_INTR_TX_COMPLETE |
				CEC_INTR_TX_FAIL |
				CEC_INTR_RX_COMPLETE |
				CEC_INTR_RX_FAIL, TRUE);
	cec_hw_set_mode(CEC_MODE_TX, FALSE);
	cec_hw_set_mode(CEC_MODE_RX, TRUE);
}

static UINT8 cec_hw_line_status(void)
{
	return cec_reg_read(CEC_RX_PRESENT_STATE_REG_ADDR);
	/* XXX why not use this? */
//	return cec_reg_read(CEC_LINE_STATUS_REG_ADDR);
}

/* XXX Do we need a return value? */
static void cec_hw_transmit(CEC_TASK_DATA *ptx)
{
	UINT32 signal_free_time;
	UINT32 i;

	if (ptx == NULL) {
		PRT_INFO("%s, args check failed.\n", __func__);
		return;
	} else {
		/* args validation check */
		if (ptx->msg_len > CEC_MAX_MSG_LEN) {
			PRT_INFO("%s, msg too large.\n", __func__);
			return;
		}
	}
//	dump_cec_data(ptx);
//	dump_cec_reg();

	if (ptx->msg_cfg.retry == TRUE)
		signal_free_time = CEC_TX_SIGNAL_FREE_TIME * SM_SYSTEM_HZ;
	else
		signal_free_time = CEC_RETX_SIGNAL_FREE_TIME * SM_SYSTEM_HZ;
	cec_reg_bulk_write(CEC_SIGNAL_FREE_TIME_0_ADDR, (UINT8 *)&signal_free_time, 4);

	cec_hw_set_mode(CEC_MODE_TX, TRUE);

	if (ptx->msg_cfg.mode == CEC_MODE_BROADCAST)
		cec_reg_write(CEC_TX_TYPE_ADDR, 1);
	else
		cec_reg_write(CEC_TX_TYPE_ADDR, 0);

	for (i = 0; i < ptx->msg_len; i++) {
		cec_reg_write(CEC_DATA_REG_ADDR, ptx->msg_fifo[i]);
		cec_reg_write(CEC_EOM_REG_ADDR, (i == ptx->msg_len-1)?0x01:0x00);
		cec_reg_write(CEC_TOGGLE_FOR_WRITE_REG_ADDR, 0);
	}

	cec_reg_write(CEC_RDY_ADDR, 1);

	PRT_INFO("%s, cmd 0x%x msg launched.\n", __func__, ptx->cmd);
//	dump_cec_reg();
}

static void cec_hw_send_status(CEC_TASK_DATA *p, UINT8 cmd, UINT8 status)
{
	p->cmd = cmd;
	p->status = status;
	CEC_WRITE_ISR_FIFO(p);
}

#ifdef SM_CEC_DEBUG_MODE
static char *s2n[CEC_TASK_STATE_MAX] = {
	"IDLE",
	"LISTENING",
	"SENDING"
};
#endif

static void cec_change_state(CEC_TASK_STATE state)
{
#ifdef SM_CEC_DEBUG_MODE
	PRT_INFO("change state: %s -> %s\n", s2n[SMCECTaskstate], s2n[state]);
#endif
	SMCECTaskstate = state;
	switch (state) {
	case CEC_TASK_STATE_SENDING:
		break;
	case CEC_TASK_STATE_LISTENING:
		break;
	default:
		break;
	}
}

hresult MV_SM_CEC_SRV_RX_MSG_TV(CEC_TASK_DATA *prxdata);
hresult MV_SM_CEC_SRV_RX_MSG_BOX(CEC_TASK_DATA *prxdata);

/* FIXME */

hresult sm_cec_isr(void)
{
	UINT16 reg=0;
	UINT16 reg_clear;
	UINT8 tmp;
	UINT8 i = 0;

#ifdef VS680_Z1
	int state;

	//FIXME: workaround to wait for TX INT state reg be stable
    do{
	    state = cec_reg_read(CEC_TX_PRESENT_STATE_REG_ADDR);
		i++;
    }while(state != 0x01 && i < 100);
#endif

	reg = (UINT16)cec_reg_read(CEC_INTR_STATUS0_REG_ADDR);
	tmp = cec_reg_read(CEC_INTR_STATUS1_REG_ADDR);
	reg |= ((UINT16)tmp << 8);

	// Clear CEC interrupt
	if (reg & CEC_INTR_TX_FAIL) {
		cec_reg_write(CEC_RDY_ADDR, 0);
		reg_clear = cec_reg_read(CEC_INTR_ENABLE0_REG_ADDR);
		reg_clear &= ~(CEC_INTR_TX_FAIL & 0x00ff);
		cec_reg_write(CEC_INTR_ENABLE0_REG_ADDR, reg_clear);

		/* XXX can these errors occur at the same time? */
		if (reg & (CEC_INTR_TX_COLLISION & 0x00ff)) {
			cec_hw_send_status(&ucCec_tx_data,
					   CEC_CMD_TX_FAILED,
					   CEC_TX_FAIL_LINE_BUSY);
		} else if (reg & (CEC_INTR_TX_SFT_FAIL & 0x00ff)) {
			cec_hw_send_status(&ucCec_tx_data,
					   CEC_CMD_TX_FAILED,
					   CEC_TX_FAIL_SIGNAL_FREE_TIME);
		} else if (reg & ((CEC_INTR_TX_UNICAST_NOACK |
				   CEC_INTR_TX_BRDCAST_NOACK) & 0x00ff)) {
			cec_hw_send_status(&ucCec_tx_data,
					   CEC_CMD_TX_FAILED,
					   CEC_TX_FAIL_NACK);
		} else {
			cec_hw_send_status(&ucCec_tx_data,
					   CEC_CMD_TX_FAILED,
					   CEC_TX_FAIL_INDATA);
		}

		cec_hw_set_mode(CEC_MODE_TX, FALSE);
		if (ucCec_tx_data.status == CEC_TX_FAIL_LINE_BUSY)
			cec_hw_set_mode(CEC_MODE_RX, TRUE);

		/* XXX if not fail in signal_free_time, retx... */

//		dump_cec_data(&ucCec_tx_data);
//		dump_cec_reg();
	}
	if (reg & CEC_INTR_TX_COMPLETE) {
		cec_hw_set_mode(CEC_MODE_TX, FALSE);
		reg_clear = cec_reg_read(CEC_INTR_ENABLE0_REG_ADDR);
		reg_clear &= ~(CEC_INTR_TX_COMPLETE & 0x00ff);
		cec_reg_write(CEC_INTR_ENABLE0_REG_ADDR, reg_clear);
		cec_hw_send_status(&ucCec_tx_data,
				   CEC_CMD_TX_DONE,
				   CEC_FAIL_NONE);
	}
	if (reg & CEC_INTR_RX_FAIL) {
		reg_clear = cec_reg_read(CEC_INTR_ENABLE0_REG_ADDR);
		reg_clear &= ~(CEC_INTR_RX_FAIL & 0x00ff);
		cec_reg_write(CEC_INTR_ENABLE0_REG_ADDR, reg_clear);
		cec_hw_send_status(&ucCec_rx_data,
				   CEC_CMD_RX_FAILED,
				   reg);
	}
	if (reg & CEC_INTR_RX_COMPLETE) {
		reg_clear = cec_reg_read(CEC_INTR_ENABLE0_REG_ADDR);
		reg_clear &= ~(CEC_INTR_RX_COMPLETE & 0x00ff);
		cec_reg_write(CEC_INTR_ENABLE0_REG_ADDR, reg_clear);

		/* clear rx buffer for new data */
		cec_clear_data(&ucCec_rx_data);

		tmp = cec_reg_read(CEC_RX_FIFO_DPTR);
		ucCec_rx_data.msg_len = tmp;
		for (i = 0; i < tmp; i++) {
			ucCec_rx_data.msg_fifo[i] = cec_reg_read(CEC_RX_BUF_READ_REG_ADDR);
			cec_reg_write(CEC_TOGGLE_FOR_READ_REG_ADDR, 0x01);
		}

		/* enable rx again */
		cec_reg_write(CEC_RX_RDY_ADDR, 0);
		cec_reg_write(CEC_RX_RDY_ADDR, 1);
		reg_clear = cec_reg_read(CEC_INTR_ENABLE0_REG_ADDR);
		reg_clear |= (CEC_INTR_RX_COMPLETE & 0x00ff);
		cec_reg_write(CEC_INTR_ENABLE0_REG_ADDR, reg_clear);
		cec_hw_send_status(&ucCec_rx_data,
				   CEC_CMD_RX_MSG,
				   reg);
	}

	return S_OK;
}

hresult sm_cec_setconfig( INT32 uiPhyaddr,UINT8 uilogaddr,UINT8 uidevicetype )
{
	SMCECConfig.iLogicAddr=uilogaddr;
	SMCECConfig.iDeviceType=uidevicetype;
	SMCECConfig.uiPhyAddr=uiPhyaddr;
	cec_change_state(CEC_TASK_STATE_LISTENING);
	return S_OK;
}

/**********************************************************************
 * FUNCTION: Disable CEC before soc power on and
 *      Enable CEC before enter standby.
 * PARAMETERS: bEnable(1: enable;0 disable)
 *
 * RETURN: S_OK - succeed
 *
 *********************************************************************/
hresult sm_cec_enable( BOOL bEnable)
{
	cec_clear_fifo(&stCec_Isr_Fifo);
	cec_clear_fifo(&stCec_App_Fifo);

	if(bEnable)
	{
		//If logic address is 15, unallocated, don't enable CEC task
		if(SMCECConfig.iLogicAddr!=0x0F)
		{
			bCECTaskExit=FALSE;
			cec_change_state(CEC_TASK_STATE_LISTENING);
			/* XXX really need this? seems it's done by app cec service */
			cec_enable_global_interrupt(TRUE);
			cec_hw_reset();
		}
		if (cec_host_type == CEC_HOST_TYPE_TV)
			sm_cec_setconfig(0x0000,0,0);
		memset(&wakeup_msg, 0, sizeof(CEC_MSG));
	}
	else
	{
		bCECTaskExit=TRUE;
		cec_change_state(CEC_TASK_STATE_IDLE);
		/* XXX shall this be disabled? */
		cec_enable_global_interrupt(FALSE);
		cec_hw_disable();
	}
	return S_OK;
}

hresult sm_process_cec_msg( void* data, INT32 iLen )
{
	INT32 *pMsgBody = (INT32*)data;
	char *pcBody = (char*)data;
	int cmdid=*pMsgBody;
	INT32 iMsgContent;
	iMsgContent=*((INT32*)pMsgBody+1);	// first param in msg
	CECConfig *pconfig;
	//CEC_TASK_DATA cecTaskCmd;
	int i;
	unsigned char sentMsg[8];
	UINT8 *pdata;
	INT32 ret;

	//unsigned char CEC_Launch_Specific_App launchReq;

//	UINT32 launchReqType;		// this is requested MV_SM_CEC_*** type
//	UINT32 launchReqLength;		// length of the char buffer which follows
//	char* launchReqBuf;


	PRT_INFO("----------SMCEC2SM: received cec msg:id=%d----------\n",cmdid);

	switch(cmdid)
	{
		//no need to do task handover
		/*
		   case    MV_SM_CEC_APPREADY:
		//CEC Application in CPU0/1 is ready, so SM stops to handle CEC task
		cecTaskCmd.cmd = CEC_CMD_EXIT;
		CEC_WRITE_APP_FIFO(&cecTaskCmd);
		break;
		*/
	case    MV_SM_CEC_INFO:
		pconfig=(CECConfig*)(pMsgBody+1);
		SMCECConfig=*pconfig;
		PRT_INFO("SM CEC:Logical address:0x%x,Physical address:0x%x\n",SMCECConfig.iLogicAddr,SMCECConfig.uiPhyAddr);
		PRT_INFO("SM CEC:Device Type:%d, bActiveSrc:%d, bValid:%d\n",SMCECConfig.iDeviceType,SMCECConfig.bActiveSource,SMCECConfig.bValid);
#ifdef HWCEC
		cec_hw_set_logicaddr(SMCECConfig.iDeviceType, SMCECConfig.iLogicAddr);
#endif
		break;

	case    MV_SM_CEC_OSD_INFO:
		for (i = 0; i < OSD_NAME_MAX_LENGTH; i++) {
			if (i < iLen - sizeof(UINT32))
				cec_osd_name[i] = *(pcBody + sizeof(UINT32) + i);
			else
				cec_osd_name[i] = ' ';
		}
		PRT_INFO("CEC OSD: %s\n", cec_osd_name);
		break;

		// SoC tells us he is ready for us to send up an app request that came from a TV while he was asleep.
		//
	case	MV_SM_CEC_APPREADY:

		PRT_INFO("Got Appready packet from SoC CEC: ");
		for (i=0; i<iLen; i++)
			PRT_INFO("%02x  ", (pcBody[i]&0xFF));
		PRT_INFO("\n");

		if (iLen == 4)
		{
			PRT_INFO("Ignoring the message because its length is wrong. Let's fix this thing.\n"); //tbdzz
			break;
		}

		if (iMsgContent != 0 && iMsgContent != 1)			// in byte #4
			PRT_INFO("SM CEC:Unexpected Google Setup state=0x%x!!\n",iMsgContent);

			PRT_INFO("SM CEC:pcBody[4]=0x%x.\n",pcBody[4]);		// any better with a byte offset???

		// Send the message buffer to SoC. Either it will be all 0's (in the case of no Launch request from TV)
		// or else it will have the CEC message that we received in it.

		switch (socReq.operands[0])
		{
			case MV_SM_CEC_APP_REQUESTED:
					// nothing special to do here. Just send as-is.
					break;

			case MV_SM_CEC_ACTIVE_SOURCE_EVENT:
					// nothing special to do here. Just send as-is.
					break;

			case MV_SM_CEC_STREAM_PATH_EVENT:
					// nothing special to do here. Just send as-is.
					break;

			case MV_SM_CEC_UC_PRESSED_EVENT:
					// nothing special to do here. Just send as-is.
					break;

			case MV_SM_CEC_COLD_BOOT:
					break;

			case 0:				// no request made since the last time we wiped it out. (i.e. we booted SoC up because of a power key or something with no bearing on CEC.
					socReq.operands[0] = MV_SM_CEC_NO_APP_REQUESTED;
					socReq.operands[1] = socReq.operands[2] = socReq.operands[3] = 0; // make it look like a nice UINT32
					socReq.length = 4;		// no params, only UINT32 type code.
					break;

			default:
					PRT_INFO("SMCEC2SM: bogus typecode in the launch request buf. Erasing! type=0x%x, len=%d\nSMCEC2SM:  ", socReq.operands[0], socReq.length);

					break;


		}

		PRT_INFO("SMCEC2SM: sending launch req to SOC: ");
		for (i=0; i<socReq.length; i++)
			PRT_INFO("%02x ", socReq.operands[i]&0xFF); 	// include 4 bytes of type at the beginning
		PRT_INFO("\n");

		sm_send_msg(MV_SM_ID_CEC, (unsigned char*)socReq.operands, socReq.length);		// already includes 4 bytes for the type code.

		memset(socReq.operands, 0, MAX_SOC_REQ_LENGTH);					// wipe it out so we don't use it again
		socReq.length=0;
		break;

	case 0x5678:			//tbdzz debug code
		PRT_INFO("[%d]CEC2SM cec_core debug code: got 0x5678, sending0xCEC12CEC to MV_SM_ID_CEC\n", mv_sm_timer_gettimems());
		sentMsg[0] = 0xCE;
		sentMsg[1] = 0xC1;
		sentMsg[2] = 0x2C;
		sentMsg[3] = 0xEC;
		ret = sm_send_msg(MV_SM_ID_CEC, sentMsg, sizeof(sentMsg));
		if (ret != S_OK)
			PRT_DEBUG("CEC2SM: debug: got err=%d when sending 0xCEC12CEC to CEC!\n");

/*
		PRT_INFO("[%d]CEC2SM cec_core debug code: also sending 0x43214321 to MV_SM_ID_SYS\n",MV_SM_GetTimeMS());
		sentMsg[0] = 0x43;
		sentMsg[1] = 0x21;
		sentMsg[2] = 0x43;
		sentMsg[3] = 0x21;
		ret = sm_send_msg(MV_SM_ID_SYS, sentMsg, sizeof(sentMsg));
		if (ret != S_OK)
			PRT_INFO("CEC2SM: debug: got err=%d when sending 0x43214321 to SYS!\n");
*/
		break;

		// SoC tells us that User has set HDMI Control on or off using the UI.
		//
	case	MV_SM_CEC_HDMICTRL_ONOFF:
		if (iMsgContent<0||iMsgContent>1)
			PRT_INFO("SMCEC2SM: Unexpected HDMI ONOFF state=0x%x!!\n",iMsgContent);
		rememberHdmiCtrlState = (iMsgContent != 0)?1:0;
		break;

	case	MV_SM_CEC_WAKEUP_MSG:
		PRT_INFO("src: 0x%x, dest: 0x%x, op: 0x%x, tot sz: %d\n", wakeup_msg.srcAddr, wakeup_msg.destAddr, wakeup_msg.opcode, sizeof(wakeup_msg));
		pdata = (UINT8 *)&wakeup_msg.operand;
		for (i = 0; i < wakeup_msg.operandlen; i++) {
			PRT_INFO("0x%02x ", pdata[i]);
		}
		PRT_INFO("\n");
		ret = sm_send_msg(MV_SM_ID_CEC, (unsigned char*)&wakeup_msg, sizeof(wakeup_msg));
		if (ret != S_OK)
			PRT_INFO("send msg fail!\n");
		break;

	default:
		PRT_INFO("Unexpected message type=0x%x, len=%d: ", cmdid, iLen);
		for (i=0; i<iLen; i++)
			PRT_INFO("%02x ", (pcBody[i]&0xFF));
		PRT_INFO("\n");

	}
	return S_OK;
}

hresult sm_cec_task( void* pData )
{
	CEC_TASK_DATA cecTaskCmd;
	UINT8 line_status;

	if (bCECTaskExit)
		return S_OK;

	if (!CEC_READ_ISR_FIFO(&cecTaskCmd))
		if (!CEC_READ_APP_FIFO(&cecTaskCmd))
			return S_OK;

	switch (SMCECTaskstate) {
	case CEC_TASK_STATE_IDLE:
		PRT_INFO("STATE: IDLE\n");
		if(CEC_CMD_EXIT == cecTaskCmd.cmd)
			sm_cec_enable(FALSE);
		break;

	case CEC_TASK_STATE_LISTENING:
		PRT_INFO("STATE: LISTENING\n");
		if (CEC_CMD_EXIT == cecTaskCmd.cmd) {
			sm_cec_enable(FALSE);
		} else if (CEC_CMD_RX_MSG == cecTaskCmd.cmd) {
			mv_sm_cec_srv_rx_msg(&cecTaskCmd);
		} else if (CEC_CMD_TX_MSG == cecTaskCmd.cmd) {
			ucCec_retrans_time = 0;
			memcpy((UINT8 *)&ucCec_tx_data, (UINT8 *)&cecTaskCmd, sizeof(CEC_TASK_DATA));
			line_status = cec_hw_line_status();
			if ((line_status & 0x01) == 0x01) {
				cec_hw_transmit(&ucCec_tx_data);
			} else {
				PRT_WARN("check line status fail: 0x%x\n", line_status);
				ucCec_tx_data.cmd = CEC_CMD_TX_FAILED;
				ucCec_tx_data.status = CEC_TX_FAIL_LINE_BUSY;
				CEC_WRITE_APP_FIFO(&ucCec_tx_data);
			}
			cec_change_state(CEC_TASK_STATE_SENDING);
		} else {
			PRT_INFO("cmd: 0x%x, not supported\n", cecTaskCmd.cmd);
		}
		break;

	case CEC_TASK_STATE_SENDING:
		PRT_INFO("STATE: SENDING\n");
		if (CEC_CMD_TX_DONE == cecTaskCmd.cmd) {
			PRT_INFO("Tx Done.\n");
			cec_clear_data(&ucCec_tx_data);
			cec_change_state(CEC_TASK_STATE_LISTENING);
		} else if (CEC_CMD_TX_FAILED == cecTaskCmd.cmd) {
			int timeout;

			timeout = mv_sm_timer_gettimems() - ucCec_tx_data.txtime;
			if ((ucCec_retrans_time < MAX_RETRANS_TIME) ||
			    (timeout > CEC_TX_TIMEOUT)) {
				PRT_INFO("Tx Failed, reason code=%d, retry\n",ucCec_tx_data.status);
				ucCec_tx_data.msg_cfg.retry = TRUE;
				line_status = cec_hw_line_status();
				if ((line_status & 0x01) == 0x01) {
					cec_hw_transmit(&ucCec_tx_data);
				} else {
					PRT_INFO("check line status fail: 0x%x\n", line_status);
					CEC_WRITE_APP_FIFO(&ucCec_tx_data);
				}
				if (ucCec_tx_data.status != CEC_TX_FAIL_SIGNAL_FREE_TIME)
					ucCec_retrans_time++;
				cec_change_state(CEC_TASK_STATE_SENDING);
			} else {
				PRT_INFO("Tx Failed, reason code=%d, give up this msg\n",ucCec_tx_data.status);
				cec_clear_data(&ucCec_tx_data);
				cec_change_state(CEC_TASK_STATE_LISTENING);
			}
		} else if (CEC_CMD_RX_MSG == cecTaskCmd.cmd) {
			mv_sm_cec_srv_rx_msg(&cecTaskCmd);
		} else if (CEC_CMD_TX_MSG == cecTaskCmd.cmd) {
			PRT_INFO("cmd: 0x%x, another msg while tx.\n", cecTaskCmd.cmd);
			CEC_WRITE_APP_FIFO (&cecTaskCmd);
		} else {
			PRT_INFO("@@Line:%d, cmd: 0x%x, not supported\n", __LINE__, cecTaskCmd.cmd);
		}
		break;

	default:
		PRT_INFO("STATE: DEFAULT\n");
		break;
	}

	return S_OK;
}

/*
 * Debug purpose only.
 */
void test_cec_prepare(int cmd)
{
	switch (cmd)
	{
	case 0:
		SMCECConfig.bActiveSource = 0;
		SMCECConfig.bValid = 1;
		SMCECConfig.iDeviceType = 4;
		SMCECConfig.iLogicAddr = 4;
		SMCECConfig.uiPhyAddr = 0x2000;
		PRT_INFO("SM CEC:Logical address:0x%x,Physical address:0x%x\n",SMCECConfig.iLogicAddr,SMCECConfig.uiPhyAddr);
		PRT_INFO("SM CEC:Device Type:%d, bActiveSrc:%d, bValid:%d\n",SMCECConfig.iDeviceType,SMCECConfig.bActiveSource,SMCECConfig.bValid);
#ifdef HWCEC
		cec_hw_set_logicaddr(SMCECConfig.iDeviceType, SMCECConfig.iLogicAddr);
#endif
		break;
	default:
		PRT_INFO("%s default?\n", __func__);
		break;
	}

	rememberHdmiCtrlState = 1;
}

hresult sm_cec_srv_tx_msg (CEC_MSG *ptxmsg);

void test_cec_start(void)
{
	CEC_MSG SendMsg;
	sm_cec_enable(TRUE);
	SendMsg.opcode = CEC_MSG_OPCODE_REPORT_POWER_STATUS;
	SendMsg.destAddr = CEC_DEVICE_TV;
	SendMsg.operand.opPwrSts = CEC_PWR_STS_STANDBY;
	sm_cec_srv_tx_msg(&SendMsg);
}

void sm_cec_init(int type)
{
	rememberHdmiCtrlState = 0x13572468; // magic, not meaningful
	strcpy(cec_osd_name, CEC_OSD_NAME);
	cec_host_type = type;
	if (type == CEC_HOST_TYPE_TV) {
		mv_sm_cec_srv_rx_msg = MV_SM_CEC_SRV_RX_MSG_TV;
		SMCECMsgTable = SMCECMsgTable_TV;
		cec_msg_table_size = sizeof(SMCECMsgTable_TV)/sizeof(CEC_MSG_TABLE);
	} else {
		mv_sm_cec_srv_rx_msg = MV_SM_CEC_SRV_RX_MSG_BOX;
		SMCECMsgTable = SMCECMsgTable_BOX;
		cec_msg_table_size = sizeof(SMCECMsgTable_BOX)/sizeof(CEC_MSG_TABLE);
	}

	return;
}

static int __attribute__((used)) mv_sm_cec_init(void)
{
#if defined(CEC_TV_BG2)
	sm_cec_init(CEC_HOST_TYPE_TV);
#else
	sm_cec_init(CEC_HOST_TYPE_BOX);
#endif

	sm_cec_enable(FALSE);

	return S_OK;
}
DECLARE_RT_INIT(cecinit, INIT_DEV_P_25, mv_sm_cec_init);

static void __attribute__((used)) mv_sm_cec_isr(void)
{
	//if(!bCECTaskExit)
		sm_cec_isr();
	return;
}
DECLARE_ISR(SMICTL_IRQ_CEC, mv_sm_cec_isr);


hresult mv_sm_cec_enable( BOOL bEnable)
{
	return sm_cec_enable(bEnable);
}

hresult mv_sm_cec_setconfig( INT32 uiPhyaddr,UINT8 uilogaddr,UINT8 uidevicetype )
{
	return sm_cec_setconfig(uiPhyaddr, uilogaddr, uidevicetype);
}

static int __attribute__((used))  mv_sm_process_cec_msg( void* data, INT32 iLen )
{
	return sm_process_cec_msg(data, iLen);
}

static int __attribute__((used)) mv_sm_process_internal_cec_msg(void * data, INT32 len)
{
	UINT32 *msg = (UINT32*)data;

	if(msg[0] == MV_SM_ID_POWER) {
		switch(msg[1]) {
		case FLOW_COLD_2_LOWPOWERSTANDBY:
		case FLOW_ACTIVE_2_LOWPOWERSTANDBY:
		case FLOW_ACTIVE_2_NORMALSTANDBY:
		case FLOW_ACTIVE_2_SUSPEND:
			if(msg[2] == STA_LEAVEFLOW) {
				CEC_MSG SendMsg;
				mv_sm_cec_enable(TRUE);
				SendMsg.opcode = CEC_MSG_OPCODE_REPORT_POWER_STATUS;
				SendMsg.destAddr = CEC_DEVICE_TV;
				SendMsg.operand.opPwrSts = CEC_PWR_STS_STANDBY;
				sm_cec_srv_tx_msg(&SendMsg);
			}
			break;
		case FLOW_LOWPOWERSTANDBY_2_ACTIVE:
		case FLOW_NORMALSTANDBY_2_ACTIVE:
		case FLOW_SUSPEND_2_ACTIVE:
			if(msg[2] == STA_ENTERFLOW)
				mv_sm_cec_enable(FALSE);
			break;
		default:
			break;
		}
	}
	return S_OK;
}

static void mv_sm_cec_task( void* pData )
{
	for ( ;; ) {
		sm_cec_task(pData);
		vTaskDelay(1);
	}
}

#define CEC_STACK_SIZE ((uint16_t) 128)

static void __attribute__((used)) create_cec_task(void)
{
	xTaskCreate(mv_sm_cec_task, "cec", CEC_STACK_SIZE, NULL, TASK_PRIORITY_2, NULL);
}

DECLARE_RT_MODULE(
	cec,
	MV_SM_ID_CEC,
	create_cec_task,
	mv_sm_process_cec_msg,
	mv_sm_process_internal_cec_msg
);

INT32 cmd_cec(INT32 argc, CHAR **argv)
{
	if (argc <= 1)
		return -1;

	if (strcmp(argv[1], "start") == 0) {
		PRT_INFO("cec msg size: %d\n", sizeof(CEC_MSG));
		test_cec_prepare(0);
		test_cec_start();
	} else if (strcmp(argv[1], "stop") == 0) {
		sm_cec_enable(FALSE);
	}
	return 0;
}

#endif  /* CECENABLE */
