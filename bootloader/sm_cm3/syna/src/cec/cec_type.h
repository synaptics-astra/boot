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
#ifndef __CEC_TYPE_H__
#define __CEC_TYPE_H__
#ifdef CECENABLE

#include "sm_common.h"
//#define BUDDY_BOX_SM_CEC 1

#define CEC_HOST_TYPE_BOX	0
#define CEC_HOST_TYPE_TV	1

/* CEC task commands */
#define CEC_TX_TIMEOUT                      (6000)
#define CEC_CMD_RX_MSG                      0
#define CEC_CMD_RX_FAILED                   1
#define CEC_CMD_TX_MSG                      2
#define CEC_CMD_TX_DONE                     3
#define CEC_CMD_TX_FAILED                   4
#define CEC_CMD_EXIT                        5

#define CEC_MODE_INVALID                    0
#define CEC_MODE_UNICAST                    1
#define CEC_MODE_BROADCAST                  2

#define CEC_BROADCAST_DESTADDR              0xF

#define CEC_GET_INIT_ADDR(header)    (((header) >> 4) & 0x0F)
#define CEC_GET_FOLL_ADDR(header)    ((header) & 0x0F)
#define CEC_SET_HEADER(init,foll)    ((foll & 0x0F) | ((init & 0x0F) << 4))

#ifdef BUDDY_RELEASE
#ifdef BUDDY_PLATFORM_BEETLE
    #define OSD_NAME_MAX_LENGTH 12
    #define CEC_OSD_NAME "Media Player"
#else   // PLATFORM_RAPTOR
    #define OSD_NAME_MAX_LENGTH 2
    #define CEC_OSD_NAME "BD"
#endif
#define CEC_VERSION CEC_VERSION_1_4a
#define CEC_DEVICE_ID 0x080046

#else  // not Buddy
#define OSD_NAME_MAX_LENGTH 14
#define CEC_OSD_NAME "  Android TV  "
#define CEC_VERSION CEC_VERSION_1_3a
#endif

/*-----------------------------------------------------------------------------
 * HDMI CEC Feature Support Configuration
*-----------------------------------------------------------------------------
*/
#define     CEC_FEATURE_ONE_TOUCH_PLAY              0x00000001
#define     CEC_FEATURE_ROUTING_CONTROL             0x00000002
#define     CEC_FEATURE_SYSTEM_STANDBY              0x00000004
#define     CEC_FEATURE_ONE_TOUCH_RECORD            0x00000008
#define     CEC_FEATURE_SYSTEM_INFO                 0x00000010
#define     CEC_FEATURE_DECK_CONTROL                0x00000020
#define     CEC_FEATURE_TUNER_CONTROL               0x00000040
#define     CEC_FEATURE_VENDOR_SPEC_CMDS            0x00000080
#define     CEC_FEATURE_OSD_STATUS_DISPLAY          0x00000100
#define     CEC_FEATURE_DEVICE_OSD_NAME_TX          0x00000200
#define     CEC_FEATURE_DEVICE_MENU_CONTROL         0x00000400
#define     CEC_FEATURE_REMOTE_CONTROL_PASS_THRU    0x00000800
#define     CEC_FEATURE_DEVICE_POWER_STATUS         0x00001000

#define     CEC_FEATURE_SYSTEM_AUDIO_CONTROL        0x00002000
#define     CEC_FEATURE_AUDIO_RATE_CONTROL          0x00004000
#define     CEC_FEATURE_TIMER_PROGRAMMING           0x00008000

   // Maximum message length
#define     CEC_MAX_MSG_LEN                         16

   // Invalid physical address F.F.F.F
#define     CEC_INVALID_PHY_ADDR                    0xFFFF

   /*-----------------------------------------------------------------------------
    * HDMI CEC Message Opcodes
    *-----------------------------------------------------------------------------
    */
#define CEC_MSG_OPCODE_UNDEFINED                    0xFFFF

   // General Protocol messages
#define CEC_MSG_OPCODE_FEATURE_ABORT                0x00
#define CEC_MSG_OPCODE_ABORT                        0xFF

   // One Touch Play
#define CEC_MSG_OPCODE_ACTIVE_SOURCE                0x82
#define CEC_MSG_OPCODE_IMAGE_VIEW_ON                0x04
#define CEC_MSG_OPCODE_TEXT_VIEW_ON                 0x0D

   // Routing control
#define CEC_MSG_OPCODE_REQUEST_ACTIVE_SOURCE        0x85
#define CEC_MSG_OPCODE_SET_STREAM_PATH              0x86
#define CEC_MSG_OPCODE_ROUTING_CHANGE               0x80
#define CEC_MSG_OPCODE_ROUTING_INFO                 0x81
#define CEC_MSG_OPCODE_INACTIVE_SOURCE              0x9D

   // Standby
#define CEC_MSG_OPCODE_STANDBY                      0x36

   // One touch record
#define CEC_MSG_OPCODE_RECORD_OFF                   0x0B
#define CEC_MSG_OPCODE_RECORD_ON                    0x09
#define CEC_MSG_OPCODE_RECORD_STATUS                0x0A
#define CEC_MSG_OPCODE_RECORD_TV_SCREEN             0x0F

   // System information
#define CEC_MSG_OPCODE_GET_MENU_LANG                0x91
#define CEC_MSG_OPCODE_GIVE_PHY_ADDR                0x83
#define CEC_MSG_OPCODE_REPORT_PHY_ADDR              0x84
#define CEC_MSG_OPCODE_SET_MENU_LANG                0x32
#define CEC_MSG_OPCODE_CEC_VERSION                  0x9E
#define CEC_MSG_OPCODE_GET_CEC_VERSION              0x9F

   // Deck control
#define CEC_MSG_OPCODE_DECK_STATUS                  0x1B
#define CEC_MSG_OPCODE_GIVE_DECK_STATUS             0x1A
#define CEC_MSG_OPCODE_DECK_CONTROL                 0x42
#define CEC_MSG_OPCODE_PLAY                         0x41

   // Tuner control
#define CEC_MSG_OPCODE_GIVE_TUNER_DEVICE_STATUS     0x08
#define CEC_MSG_OPCODE_SELECT_ANALOGUE_SERVICE      0x92
#define CEC_MSG_OPCODE_SELECT_DIGITAL_SERVICE       0x93
#define CEC_MSG_OPCODE_TUNER_STEP_DECREMENT         0x06
#define CEC_MSG_OPCODE_TUNER_STEP_INCREMENT         0x05
#define CEC_MSG_OPCODE_TUNER_DEVICE_STATUS          0x07

   // Vendor specific commands
#define CEC_MSG_OPCODE_DEVICE_VENDOR_ID             0x87
#define CEC_MSG_OPCODE_GIVE_DEVICE_VENDOR_ID        0x8C
#define CEC_MSG_OPCODE_VENDOR_COMMAND               0x89
#define CEC_MSG_OPCODE_VENDOR_COMMAND_WITH_ID       0xA0
#define CEC_MSG_OPCODE_VENDOR_REMOTE_BTN_DOWN       0x8A
#define CEC_MSG_OPCODE_VENDOR_REMOTE_BTN_UP         0x8B

   // OSD status display
#define CEC_MSG_OPCODE_SET_OSD_STRING               0x64

   // Device OSD name transfer
#define CEC_MSG_OPCODE_GIVE_OSD_NAME                0x46
#define CEC_MSG_OPCODE_SET_OSD_NAME                 0x47

   // Device menu control, Remote control pass-through
#define CEC_MSG_OPCODE_USER_CONTROL_PRESSED         0x44
#define CEC_MSG_OPCODE_USER_CONTROL_RELEASED        0x45
#define CEC_MSG_OPCODE_MENU_REQUEST                 0x8D
#define CEC_MSG_OPCODE_MENU_STATUS                  0x8E

   // Device power status
#define CEC_MSG_OPCODE_GIVE_DEVICE_POWER_STATUS     0x8F
#define CEC_MSG_OPCODE_REPORT_POWER_STATUS          0x90

   // System Audio Control
#define CEC_MSG_OPCODE_GIVE_AUDIO_STATUS            0x71
#define CEC_MSG_OPCODE_GIVE_SYS_AUDIO_MODE_STATUS   0x7D
#define CEC_MSG_OPCODE_REPORT_AUDIO_STATUS          0x7A
#define CEC_MSG_OPCODE_SET_SYS_AUDIO_MODE           0x72
#define CEC_MSG_OPCODE_SYS_AUDIO_MODE_REQUEST       0x70
#define CEC_MSG_OPCODE_SYS_AUDIO_MODE_STATUS        0x7E

   // Audio Rate Control
#define CEC_MSG_OPCODE_SET_AUDIO_RATE               0x9A

   // Timer Programming
#define CEC_MSG_OPCODE_CLR_ANALOGUE_TIMER           0x33
#define CEC_MSG_OPCODE_CLR_DIGITAL_TIMER            0x99
#define CEC_MSG_OPCODE_CLR_EXTERNAL_TIMER           0xA1
#define CEC_MSG_OPCODE_SET_ANALOGUE_TIMER           0x34
#define CEC_MSG_OPCODE_SET_DIGITAL_TIMER            0x97
#define CEC_MSG_OPCODE_SET_EXTERNAL_TIMER           0xA2
#define CEC_MSG_OPCODE_SET_TIMER_PGM_TITLE          0x67
#define CEC_MSG_OPCODE_TIMER_CLEARED_STATUS         0x43
#define CEC_MSG_OPCODE_TIMER_STATUS                 0x35


/*HDMI CEC Feature Support Configuration*/
#define CEC_FEATURE_ONE_TOUCH_PLAY              0x00000001
#define CEC_FEATURE_ROUTING_CONTROL             0x00000002
#define CEC_FEATURE_SYSTEM_STANDBY              0x00000004
#define CEC_FEATURE_ONE_TOUCH_RECORD            0x00000008
#define CEC_FEATURE_SYSTEM_INFO                 0x00000010
#define CEC_FEATURE_DECK_CONTROL                0x00000020
#define CEC_FEATURE_TUNER_CONTROL               0x00000040
#define CEC_FEATURE_VENDOR_SPEC_CMDS            0x00000080
#define CEC_FEATURE_OSD_STATUS_DISPLAY          0x00000100
#define CEC_FEATURE_DEVICE_OSD_NAME_TX          0x00000200
#define CEC_FEATURE_DEVICE_MENU_CONTROL         0x00000400
#define CEC_FEATURE_REMOTE_CONTROL_PASS_THRU    0x00000800
#define CEC_FEATURE_DEVICE_POWER_STATUS         0x00001000

#define CEC_FEATURE_SYSTEM_AUDIO_CONTROL        0x00002000
#define CEC_FEATURE_AUDIO_RATE_CONTROL          0x00004000
#define CEC_FEATURE_TIMER_PROGRAMMING           0x00008000

/*HDMI CEC Message Opcode */
#define CEC_MSG_OPCODE_UNDEFINED                    0xFFFF
// General Protocol messages
#define CEC_MSG_OPCODE_FEATURE_ABORT                0x00
#define CEC_MSG_OPCODE_ABORT                        0xFF

// One Touch Play
#define CEC_MSG_OPCODE_ACTIVE_SOURCE                0x82
#define CEC_MSG_OPCODE_IMAGE_VIEW_ON                0x04
#define CEC_MSG_OPCODE_TEXT_VIEW_ON                 0x0D

// Routing control
#define CEC_MSG_OPCODE_REQUEST_ACTIVE_SOURCE        0x85
#define CEC_MSG_OPCODE_SET_STREAM_PATH              0x86
#define CEC_MSG_OPCODE_ROUTING_CHANGE               0x80
#define CEC_MSG_OPCODE_ROUTING_INFO                 0x81
#define CEC_MSG_OPCODE_INACTIVE_SOURCE              0x9D

// Standby
#define CEC_MSG_OPCODE_STANDBY                      0x36

// One touch record
#define CEC_MSG_OPCODE_RECORD_OFF                   0x0B
#define CEC_MSG_OPCODE_RECORD_ON                    0x09
#define CEC_MSG_OPCODE_RECORD_STATUS                0x0A
#define CEC_MSG_OPCODE_RECORD_TV_SCREEN             0x0F

// System information
#define CEC_MSG_OPCODE_GET_MENU_LANG                0x91
#define CEC_MSG_OPCODE_GIVE_PHY_ADDR                0x83
#define CEC_MSG_OPCODE_REPORT_PHY_ADDR              0x84
#define CEC_MSG_OPCODE_SET_MENU_LANG                0x32
#define CEC_MSG_OPCODE_CEC_VERSION                  0x9E
#define CEC_MSG_OPCODE_GET_CEC_VERSION              0x9F

// Deck control
#define CEC_MSG_OPCODE_DECK_STATUS                  0x1B
#define CEC_MSG_OPCODE_GIVE_DECK_STATUS             0x1A
#define CEC_MSG_OPCODE_DECK_CONTROL                 0x42
#define CEC_MSG_OPCODE_PLAY                         0x41

// Tuner control
#define CEC_MSG_OPCODE_GIVE_TUNER_DEVICE_STATUS     0x08
#define CEC_MSG_OPCODE_SELECT_ANALOGUE_SERVICE      0x92
#define CEC_MSG_OPCODE_SELECT_DIGITAL_SERVICE       0x93
#define CEC_MSG_OPCODE_TUNER_STEP_DECREMENT         0x06
#define CEC_MSG_OPCODE_TUNER_STEP_INCREMENT         0x05
#define CEC_MSG_OPCODE_TUNER_DEVICE_STATUS          0x07

// Vendor specific commands
#define CEC_MSG_OPCODE_DEVICE_VENDOR_ID             0x87
#define CEC_MSG_OPCODE_GIVE_DEVICE_VENDOR_ID        0x8C
#define CEC_MSG_OPCODE_VENDOR_COMMAND               0x89
#define CEC_MSG_OPCODE_VENDOR_COMMAND_WITH_ID       0xA0
#define CEC_MSG_OPCODE_VENDOR_REMOTE_BTN_DOWN       0x8A
#define CEC_MSG_OPCODE_VENDOR_REMOTE_BTN_UP         0x8B

// OSD status display
#define CEC_MSG_OPCODE_SET_OSD_STRING               0x64

// Device OSD name transfer
#define CEC_MSG_OPCODE_GIVE_OSD_NAME                0x46
#define CEC_MSG_OPCODE_SET_OSD_NAME                 0x47

// Device menu control, Remote control pass-through
#define CEC_MSG_OPCODE_USER_CONTROL_PRESSED         0x44
#define CEC_MSG_OPCODE_USER_CONTROL_RELEASED        0x45
#define CEC_MSG_OPCODE_MENU_REQUEST                 0x8D
#define CEC_MSG_OPCODE_MENU_STATUS                  0x8E

// Device power status
#define CEC_MSG_OPCODE_GIVE_DEVICE_POWER_STATUS     0x8F
#define CEC_MSG_OPCODE_REPORT_POWER_STATUS          0x90

// System Audio Control
#define CEC_MSG_OPCODE_GIVE_AUDIO_STATUS            0x71
#define CEC_MSG_OPCODE_GIVE_SYS_AUDIO_MODE_STATUS   0x7D
#define CEC_MSG_OPCODE_REPORT_AUDIO_STATUS          0x7A
#define CEC_MSG_OPCODE_SET_SYS_AUDIO_MODE           0x72
#define CEC_MSG_OPCODE_SYS_AUDIO_MODE_REQUEST       0x70
#define CEC_MSG_OPCODE_SYS_AUDIO_MODE_STATUS        0x7E

// Audio Rate Control
#define CEC_MSG_OPCODE_SET_AUDIO_RATE               0x9A

// Timer Programming
#define CEC_MSG_OPCODE_CLR_ANALOGUE_TIMER           0x33
#define CEC_MSG_OPCODE_CLR_DIGITAL_TIMER            0x99
#define CEC_MSG_OPCODE_CLR_EXTERNAL_TIMER           0xA1
#define CEC_MSG_OPCODE_SET_ANALOGUE_TIMER           0x34
#define CEC_MSG_OPCODE_SET_DIGITAL_TIMER            0x97
#define CEC_MSG_OPCODE_SET_EXTERNAL_TIMER           0xA2
#define CEC_MSG_OPCODE_SET_TIMER_PGM_TITLE          0x67
#define CEC_MSG_OPCODE_TIMER_CLEARED_STATUS         0x43
#define CEC_MSG_OPCODE_TIMER_STATUS                 0x35


#ifdef CEC_SM_SOC_COMM
// following are Buddy subtypes under VENDOR_COMMAND_WITH_ID
#define CEC_VEND_CMD_BUDDY_VEND_FEATURE_ABORT				0x00
			// could come if we send a malformed REPORT_ADDL_MENU_INFO.
#define CEC_VEND_CMD_BUDDY_SET_HDMI_CTRL					0x06
			// in stby, don't respond to this request.
#define CEC_VEND_CMD_BUDDY_GIVE_ADDL_MENU_INFO			0x1E	//
			// if OK, respond with REPORT_ADDL_MENU_INFO below.
			// possible errors:
			//		decr count!=0		- send vendor feature abort with DECR_CTR_ERR
			//		if didn't come from TV - send vendor feature abort with REFUSED
#define CEC_VEND_CMD_BUDDY_REPORT_ADDL_MENU_INFO			0x1F	// Tell TV what kind of menu we have, see details below.
			// must broadcast it.
#define CEC_VEND_CMD_BUDDY_LAUNCH_SPECIFIC_APP			0x20	// TV sends
			// if OK, we get 4 bytes defined below
			// NG if not 4 params		- send vendor feature abort with invalid operand
			// NG if decr count != 0	- send vendor featuer abort with DECR_CTR_ERR

// following are contents of REPORT_ADDL_MENU_INFO msg. These are the only values that are currently possible.
typedef struct vendcmdreport_ {
	char menu_category;			/* 0x0 = "Buddy BTV Box" */
	char table_id;				/* 0x0 = Simple Menu for Android" */
	UINT8 reservedH;			/* must be 0x0  HOB comes first in the message. */
	UINT8 reservedL;			/* must be 0x0 */




} BUDDY_MENU_REPORT;


typedef struct buddyAppId_ {			/* parameters on a LAUNCH_SPECIFIC_APP msg */
	char requestedID[4];
} BUDDY_APP_ID_PARAMS;

// error codes for VENDOR_FEATURE_ABORT messages, in response to errors in above VENDOR_CMD_WITH_ID msgs

enum buddy_abort_reason {
	unrecognized_internal_opcode 	= 0x00,
	not_in_correct_mode				= 0x01,
	invalid_operand					= 0x03,
	refused							= 0x04,
	decr_ctr_nonzero				= 0x05
};


typedef struct _buddy_func_abort_ {
	UINT8 offendingBuddyOpcodeH;
	UINT8 offendingBuddyOpcodeL;
	char reason;
} BUDDY_FUNC_ABORT;

// build parameter structs for the params of the Buddy vendor_cmd_with_id messages.
typedef union _buddy_vend_cmd {
		BUDDY_APP_ID_PARAMS	launch;		// for LAUNCH_SPECIFIC_APP
		BUDDY_MENU_REPORT 	report;	// for REPORT_ADDL_MENU_INFO
		BUDDY_FUNC_ABORT		abort;		// from cec_vend_cmd_Buddy_err above
} BUDDY_VEND_CMD;





typedef struct _CEC_operand_vend_cmd_w_ID_
{
	UINT8		initfoll;			// initiator and follower
	UINT8		opcode;				// must be VENDOR_CMD_WITH_ID
	UINT8		vendorUID[3];
	UINT8		null_HOB_of_buddy_opcode;		// does this make addressing decrCtr better?
	UINT8		buddy_opcode;
	UINT8		decrCtr;
	BUDDY_VEND_CMD	vendCmd;

} CEC_VENDOR_CMD_WITH_ID;		// this is added to CEC_MSG_OPERAND union below.




// The following are to account for differences in offsets to the various structs and struct members based on padding done by the compiler. This matters in
// cec_srv.c CEC_SRV_FormatConvert(), then all the code that uses the data can simply use the structs defined here (above.)
//
// In the CEC_MSG struct, CEC_MSG.operand is padded out to 8 because the fixed portion is only 5 bits. In Vendor Command with ID, 6 bytes are padded with 2.
// However the CEC_TASK_DATA struct has no padding - from CEC_TASK_DATA.msg_fifo to the end, it's contiguous data (like it looks in the frame.)
// On the positive side, it means we can use the struct members defined in cec_type.h, but on the minus side we have to remember that they
// are adjusted for UINT32 boundaries, in other words, on the source side :
//		- CEC_MSG.operand = offset 8 (5 meaningful bytes plus 3 padding bytes) so Vend Cmd with ID begins at CEC_MSG + 8.
//		- opVendCmdWithID.vendCmd = offset 8 (6 meaningful plus 2 padding) so operand.opVendCmdWithID.vendCmd begins at CEC_MSG+8+8.
//


/* structs in CEC_TASK_DATA  (the way it looks in an actual CEC message)
	4 bytes CEC_TASK_DATA header (type, status, broad/unicast, etc.)
								<--------------------CEC_TASK_DATA.msg_fifo
	8 bytes of standard Vendor_Cmd_with_ID
								<--------------------CEC_TASK_DATA.msg_fifo[8]
	variable number of bytes for vendCmd, depending on Buddy subtype code.
								<--------------------CEC_TASK_DATA.msg_len gives this length

 structs in CEC_MSG
	5 bytes of header (src, destaddr, msg type, length)
	3 bytes of padding
								<----------------------CEC_MSG.operands
	8 bytes of standard vendor_cmd_with_ID
								<----------------------CEC_MSG.operands.opVendCmdWithID.vendCmd
	variable number of bytes of parameter data   (Vendor feature abort, etc.)
								<----------------------CEC_MSG.operandlen gives this length.
*/

#define CEC_MSG_meaningful	5
// 8 = offset in destination between pmsg->operand and pmsg, considering padding to UINT32 boundary
#define CEC_MSG_padded  	8			/*  ((CEC_MSG_meaningful)/4 + 1)*4)  */
#define CEC_MSG_padding		3			/* (CEC_MSG_padded - CEC_MSG_meaningful)  */

// meaningful portion from CEC_TASK_DATA, before CEC_TASK_DATA.msg_fifo
#define CEC_TASK_DATA_meaningful 4


// Vendor Cmd With ID msg with padding before the variable part or not
#define VCWID_meaningful	8

// 8 = offset in dest between VCWID and VCWID.vendCmd, considering padding.
#define VCWID_padded 		8			/*   ((VCWID_meaningful/4 + 1)*4)  */
#define VCWID_padding 		0			/*   (VCWID_padded - VCWID_meaningful)  */

// The following are used to validate the size of CEC messages, so they are constructed with non-padded sizes (i.e. they are the same as CEC_TASK_DATA.)
#define expected_length_LAUNCH_SPECIFIC_APP (VCWID_meaningful + sizeof(BUDDY_APP_ID_PARAMS))
#define expected_length_VENDOR_FEATURE_ABORT (VCWID_meaningful + sizeof(BUDDY_FUNC_ABORT))
#define expected_length_BUDDY_MENU_REPORT (VCWID_meaningful + sizeof(BUDDY_MENU_REPORT))
#define expected_length_GIVE_ADDL_MENU_INFO (VCWID_meaningful)


//---------------------------------------------------
// Global structure for cec_core to read when he needs to tell SoC what to do after boot.
// --------------------------------------------------

#define MAX_SOC_REQ_LENGTH (SM_MSG_BODY_SIZE)
typedef struct SoC_Request_At_Startup_tag_ {
	UINT32 length;			// of the buffer which follows, including 4 bytes of type and however many params are appropriate
	union
	{
		int iOperands[MAX_SOC_REQ_LENGTH/sizeof(int)];
		char operands[MAX_SOC_REQ_LENGTH];	// data of the request. First 4 bytes are reserved for type code. The remainder is whatever
	};
							//params related to the request that SoC CEC will need.
} SOC_REQ_AT_STARTUP;


#endif /* CEC_SM_SOC_COMM */




/*HDMI CEC Message enums and structres*/
// Logical device types
#define CEC_DEVICE_TV             (0)
#define CEC_DEVICE_REC            (1)
#define CEC_DEVICE_RES            (2)
#define CEC_DEVICE_TUNER          (3)
#define CEC_DEVICE_PB             (4)
#define CEC_DEVICE_AUDIO_SYS      (5)

// Status information
#define CEC_STATUS_SUCCESS                    (0)
#define CEC_STATUS_INVALID_PARAM              (1)
#define CEC_STATUS_ERR_INVALID_MODE           (2)
#define CEC_STATUS_ERR_MSG_UNSUPPORTED        (3)
#define CEC_STATUS_ERR_INVALID_FRAME_LEN      (4)
#define CEC_STATUS_ERR_MSG_IGNORED            (5)
#define CEC_STATUS_TX_ERR_COMM_FAIL           (6)
#define CEC_STATUS_RX_ERR_COMM_FAIL           (7)

// Feature abort reason
#define CEC_ABORT_REASON_UNREC_OPCODE         (0)
#define CEC_ABORT_REASON_INV_MODE_TO_RESPOND  (1)
#define CEC_ABORT_REASON_CANNOT_PROVIDE_SRC   (2)
#define CEC_ABORT_REASON_INV_OPCODE           (3)
#define CEC_ABORT_REASON_REFUSED              (4)

// Menu request type
#define CEC_MENU_REQUEST_ACTIVATE             (0)
#define CEC_MENU_REQUEST_DEACTIVATE           (1)
#define CEC_MENU_REQUEST_QUERY                (2)

// Menu States
#define CEC_MENU_STATE_ACTIVATED              (0)
#define CEC_MENU_STATE_DEACTIVATED            (1)

// User Control Code
#define CEC_UC_SELECT               (0x00)
#define CEC_UC_UP                   (0x01)
#define CEC_UC_DOWN                 (0x02)
#define CEC_UC_LEFT                 (0x03)
#define CEC_UC_RIGHT                (0x04)
#define CEC_UC_RIGHT_UP             (0x05)
#define CEC_UC_RIGHT_DOWN           (0x06)
#define CEC_UC_LEFT_UP              (0x07)
#define CEC_UC_LEFT_DOWN            (0x08)
#define CEC_UC_ROOT_MENU            (0x09)
#define CEC_UC_SETUP_MENU           (0x0A)
#define CEC_UC_CONTENTS_MENU        (0x0B)
#define CEC_UC_FAV_MENU             (0x0C)
#define CEC_UC_EXIT                 (0x0D)
#define CEC_UC_NUM_00               (0x20)
#define CEC_UC_NUM_01               (0x21)
#define CEC_UC_NUM_02               (0x22)
#define CEC_UC_NUM_03               (0x23)
#define CEC_UC_NUM_04               (0x24)
#define CEC_UC_NUM_05               (0x25)
#define CEC_UC_NUM_06               (0x26)
#define CEC_UC_NUM_07               (0x27)
#define CEC_UC_NUM_08               (0x28)
#define CEC_UC_NUM_09               (0x29)
#define CEC_UC_DOT                  (0x2A)
#define CEC_UC_ENTER                (0x2B)
#define CEC_UC_CLEAR                (0x2C)
#define CEC_UC_CH_UP                (0x30)
#define CEC_UC_CH_DOWN              (0x31)
#define CEC_UC_PREV_CH              (0x32)
#define CEC_UC_SOUND_SEL            (0x33)
#define CEC_UC_INPUT_SEL            (0x34)
#define CEC_UC_DISP_INFO            (0x35)
#define CEC_UC_HELP                 (0x36)
#define CEC_UC_PAGE_UP              (0x37)
#define CEC_UC_PAGE_DOWN            (0x38)
#define CEC_UC_POWER                (0x40)
#define CEC_UC_VOL_UP               (0x41)
#define CEC_UC_VOL_DOWN             (0x42)
#define CEC_UC_MUTE                 (0x43)
#define CEC_UC_PLAY                 (0x44)
#define CEC_UC_STOP                 (0x45)
#define CEC_UC_PAUSE                (0x46)
#define CEC_UC_RECORD               (0x47)
#define CEC_UC_REWIND               (0x48)
#define CEC_UC_FAST_FORWARD         (0x49)
#define CEC_UC_EJECT                (0x4A)
#define CEC_UC_FORWARD              (0x4B)
#define CEC_UC_BACKWARD             (0x4C)
#define CEC_UC_ANGLE                (0x50)
#define CEC_UC_SUB_PICT             (0x51)
#define CEC_UC_PLAY_FN              (0x60)
#define CEC_UC_PAUSE_PLAY_FN        (0x61)
#define CEC_UC_RECORD_FN            (0x62)
#define CEC_UC_PAUSE_RECORD_FN      (0x63)
#define CEC_UC_STOP_FN              (0x64)
#define CEC_UC_MUTE_FN              (0x65)
#define CEC_UC_RESTORE_VOL_FN       (0x66)
#define CEC_UC_TUNE_FN              (0x67)
#define CEC_UC_SEL_DISK_FN          (0x68)
#define CEC_UC_SEL_AV_INPUT_FN      (0x69)
#define CEC_UC_SEL_AUDIO_INPUT_FN   (0x6A)
#define CEC_UC_POWER_TOGGLE         (0x6B)
#define CEC_UC_POWER_OFF            (0x6C)
#define CEC_UC_POWER_ON             (0x6D)
#define CEC_UC_F1_BLUE              (0x71)
#define CEC_UC_F2_RED               (0x72)
#define CEC_UC_F3_GREEN             (0x73)
#define CEC_UC_F4_YELLOW            (0x74)
#define CEC_UC_F5                   (0x75)

// Recording Source
#define CEC_REC_SRC_OWN_SRC         (1)
#define CEC_REC_SRC_DIG_SRV         (2)

// Recording status
#define CEC_REC_STS_RECORDING_OWN_SRC             (0x01)
#define CEC_REC_STS_RECORDING_DIG_SRV             (0x02)
#define CEC_REC_STS_NO_REC_UNABLE_TO_REC_DIG_SRV  (0x05)
#define CEC_REC_STS_NO_REC_NO_MEDIA               (0x10)
#define CEC_REC_STS_NO_REC_PLAYING                (0x11)
#define CEC_REC_STS_NO_REC_ALREADY_RECORDING      (0x12)
#define CEC_REC_STS_NO_REC_MEDIA_PROTECTED        (0x13)
#define CEC_REC_STS_NO_REC_NO_SOURCE              (0x14)
#define CEC_REC_STS_NO_REC_MEDIA_PROBLEM          (0x15)
#define CEC_REC_STS_NO_REC_OTHER_REASON           (0x1F)

// Digital broadcast system
#define CEC_DIG_BROADCAST_SYS_ARIB          (0)
#define CEC_DIG_BROADCAST_SYS_ATSC          (1)
#define CEC_DIG_BROADCAST_SYS_DVB           (2)

// Deck control mode
#define CEC_DECK_CTRL_MODE_SKIPFWD_WINDFWD  (0x01)
#define CEC_DECK_CTRL_MODE_SKIPBWD_REWIND   (0x02)
#define CEC_DECK_CTRL_MODE_STOP             (0x03)
#define CEC_DECK_CTRL_MODE_EJECT            (0x04)

// Deck Info
#define CEC_DECK_INFO_PLAY                (0x11)
#define CEC_DECK_INFO_RECORD              (0x12)
#define CEC_DECK_INFO_PLAY_REV            (0x13)
#define CEC_DECK_INFO_STILL               (0x14)
#define CEC_DECK_INFO_SLOW                (0x15)
#define CEC_DECK_INFO_SLOW_REV            (0x16)
#define CEC_DECK_INFO_SEARCH_FWD          (0x17)
#define CEC_DECK_INFO_SEARCH_REV          (0x18)
#define CEC_DECK_INFO_NO_MEDIA            (0x19)
#define CEC_DECK_INFO_STOP                (0x1A)
#define CEC_DECK_INFO_WIND                (0x1B)
#define CEC_DECK_INFO_REWIND              (0x1C)
#define CEC_DECK_INFO_INDEX_SEARCH_FWD    (0x1D)
#define CEC_DECK_INFO_INDEX_SEARCH_REV    (0x1E)
#define CEC_DECK_INFO_OTHER_STS           (0x1F)

// Status Request
#define CEC_STATUS_REQUEST_ON             (0x01)
#define CEC_STATUS_REQUEST_OFF            (0x02)
#define CEC_STATUS_REQUEST_ONCE           (0x03)

// Play Mode
#define CEC_PLAY_MODE_FORWARD             (0x24)
#define CEC_PLAY_MODE_REVERSE             (0x20)
#define CEC_PLAY_MODE_STILL               (0x25)
#define CEC_PLAY_MODE_SCAN_FWD_MIN_SPEED  (0x05)
#define CEC_PLAY_MODE_SCAN_FWD_MED_SPEED  (0x06)
#define CEC_PLAY_MODE_SCAN_FWD_MAX_SPEED  (0x07)
#define CEC_PLAY_MODE_SCAN_REV_MIN_SPEED  (0x09)
#define CEC_PLAY_MODE_SCAN_REV_MED_SPEED  (0x0A)
#define CEC_PLAY_MODE_SCAN_REV_MAX_SPEED  (0x0B)
#define CEC_PLAY_MODE_SLOW_FWD_MIN_SPEED  (0x15)
#define CEC_PLAY_MODE_SLOW_FWD_MED_SPEED  (0x16)
#define CEC_PLAY_MODE_SLOW_FWD_MAX_SPEED  (0x17)
#define CEC_PLAY_MODE_SLOW_REV_MIN_SPEED  (0x19)
#define CEC_PLAY_MODE_SLOW_REV_MED_SPEED  (0x1A)
#define CEC_PLAY_MODE_SLOW_REV_MAX_SPEED  (0x1B)

// Display Control
#define CEC_DISPCTRL_DISP_FOR_DEF_TIME    (0x00)
#define CEC_DISPCTRL_DISP_UNTIL_CLEARED   (0x40)
#define CEC_DISPCTRL_CLR_PREV_MSG         (0x80)
#define CEC_DISPCTRL_RESERVED             (0xC0)

// Power Status
#define CEC_PWR_STS_ON                     (0)
#define CEC_PWR_STS_STANDBY                (1)
#define CEC_PWR_STS_IN_TRANS_STANDBY_TO_ON (2)
#define CEC_PWR_STS_IN_TRANS_ON_TO_STANDBY (3)

// Tuner Display Info
#define MV_PE_VOUT_TUNER_DISPLAY_INFO_TUNER_DISPLAYED     (0)
#define MV_PE_VOUT_TUNER_DISPLAY_INFO_TUNER_NOT_DISPLAYED (1)

// Recording Sequence
#define CEC_REC_SEQUENCE_ONCE_ONLY  (0)
#define CEC_REC_SEQUENCE_SUN        (1)
#define CEC_REC_SEQUENCE_MON        (2)
#define CEC_REC_SEQUENCE_TUES       (4)
#define CEC_REC_SEQUENCE_WED        (8)
#define CEC_REC_SEQUENCE_THUR       (16)
#define CEC_REC_SEQUENCE_FRI        (32)
#define CEC_REC_SEQUENCE_SAT        (64)

// Analogue Broadcast Type
#define CEC_ANALOGUE_BROADCAST_CABLE       (0)
#define CEC_ANALOGUE_BROADCAST_SATELLITE   (1)
#define CEC_ANALOGUE_BROADCAST_TERRESTRIAL (2)


// Broadcast system
#define CEC_BROADCAST_SYSTEM_PAL_BG      (0)
#define CEC_BROADCAST_SYSTEM_SECAM_LDASH (1)
#define CEC_BROADCAST_SYSTEM_PAL_M       (2)
#define CEC_BROADCAST_SYSTEM_NTSC_M      (3)
#define CEC_BROADCAST_SYSTEM_PAL_I       (4)
#define CEC_BROADCAST_SYSTEM_SECAM_DK    (5)
#define CEC_BROADCAST_SYSTEM_SECAM_BG    (6)
#define CEC_BROADCAST_SYSTEM_SECAM_L     (7)
#define CEC_BROADCAST_SYSTEM_PAL_DK      (8)
#define CEC_BROADCAST_SYSTEM_OTHER_SYS   (0x1F)


// External source specifier
#define CEC_EXT_SRC_PLUG      (4)
#define CEC_EXT_SRC_PHY_ADDR  (5)


// Timer Cleared Status Data
#define CEC_TIMER_STS_NOT_CLRED_REC         (0)
#define CEC_TIMER_STS_NOT_CLRED_NO_MATCHING (1)
#define CEC_TIMER_STS_NOT_CLRED_NO_INFO     (2)
#define CEC_TIMER_STS_CLRED                 (0x80)


// Timer Overlap Warning
#define CEC_TIMER_OVERLAP_WARNING_NO_OVERLAP     (0)
#define CEC_TIMER_OVERLAP_WARNING_BLOCKS_OVERLAP (1)


// Media Info
#define CEC_MEDIA_INFO_PRESENT_NOT_PROT (0)
#define CEC_MEDIA_INFO_PRESENT_PROT     (1)
#define CEC_MEDIA_INFO_NOT_PRESENT      (2)
#define CEC_MEDIA_INFO_FUTURE_USE       (3)


// Timer Programmed Indicator
#define CEC_TIMER_PGM_IND_NOT_PROGRAMMED (0)
#define CEC_TIMER_PGM_IND_PROGRAMMED     (1)


// Timer Programmed Info
#define CEC_TIMER_PGMED_INFO_SPACE_FOR_REC     (8)
#define CEC_TIMER_PGMED_INFO_NO_SPACE_FOR_REC  (9)
#define CEC_TIMER_PGMED_INFO_NO_MEDIA_INFO        (0x0A)
#define CEC_TIMER_PGMED_INFO_MAYNOT_SPACE_FOR_REC (0x0B)


// Timer not programmed error info
#define CEC_TIMER_NOT_PGMED_ERR_NO_FREE_TIMER      (1)
#define CEC_TIMER_NOT_PGMED_ERR_DATE_OUT_OF_RANGE  (2)
#define CEC_TIMER_NOT_PGMED_ERR_REC_SEQ_ERR        (3)
#define CEC_TIMER_NOT_PGMED_ERR_INV_EXT_PLUG_NUM   (4)
#define CEC_TIMER_NOT_PGMED_ERR_INV_EXT_PHY_ADDR   (5)
#define CEC_TIMER_NOT_PGMED_ERR_CA_SYS_UNSUPPORT   (6)
#define CEC_TIMER_NOT_PGMED_ERR_CA_ENTITLEMENTS    (7)
#define CEC_TIMER_NOT_PGMED_ERR_RES_UNSUPPORT      (8)
#define CEC_TIMER_NOT_PGMED_ERR_PAR_LOCK_ON        (9)
#define CEC_TIMER_NOT_PGMED_ERR_CLK_FAILURE        (0x0A)
#define CEC_TIMER_NOT_PGMED_ERR_ALREADY_PGMED      (0x0E)


// Audio Rate Control
#define CEC_AUDIO_RATE_CTRL_OFF                 (0)
#define CEC_AUDIO_RATE_CTRL_WIDERANGE_STD_RATE  (1)
#define CEC_AUDIO_RATE_CTRL_WIDERANGE_FAST_RATE (2)
#define CEC_AUDIO_RATE_CTRL_WIDERANGE_SLOW_RATE (3)
#define CEC_AUDIO_RATE_CTRL_NARROWRANGE_STD_RATE  (4)
#define CEC_AUDIO_RATE_CTRL_NARROWRANGE_FAST_RATE (5)
#define CEC_AUDIO_RATE_CTRL_NARROWRANGE_SLOW_RATE (6)


// CEC Version
#define CEC_VERSION_1_1   (0)
#define CEC_VERSION_1_2   (1)
#define CEC_VERSION_1_2a  (2)
#define CEC_VERSION_1_3   (3)
#define CEC_VERSION_1_3a  (4)
#define CEC_VERSION_1_4  (5)
#define CEC_VERSION_1_4a  (5)


// Feature abort message operand
typedef struct _CEC_operand_feature_abort_
{
    UINT8         featureOpcode;
    UINT8         abortReason;  // Feature abort reason
}CEC_OPERAND_FEATURE_ABORT;

// Report physical address message operand
typedef struct _CEC_operand_report_phy_addr_
{
    UINT16        phyAddr;
    UINT8         deviceType;  // Logical device types
}CEC_OPERAND_REPORT_PHY_ADDR;
// OSD Name
typedef struct _CEC_operand_osd_name_
{
    UINT8         osdNameLen;
    UINT8         osdName[14];
}CEC_OPERAND_OSD_NAME;
// Routing Change
typedef struct _CEC_operand_routing_change_
{
    UINT16      origPhyAddr;
    UINT16      newPhyAddr;
}CEC_OPERAND_ROUTING_CHANGE;
//Play user control with play mode
typedef struct _CEC_operand_usercontrol_playext
{
    UINT8      uccode;
    UINT8      playmode;
}CEC_OPERAND_USERCTL_PLAYEXT;


typedef union _cec_operand_userctl_
{
   UINT8               opUserControlCodeonebyte;  // User Control Code
    CEC_OPERAND_USERCTL_PLAYEXT opUserControlPlayext;//Play user control with play mode
}CEC_OPERAND_USERCTL;


// Union of message operands
typedef union _CEC_msg_operand_
{
    UINT16                                  opPhyAddr;
    CEC_OPERAND_FEATURE_ABORT           opFeatureAbort;
    CEC_OPERAND_REPORT_PHY_ADDR         opRepPhyAddr;
    CEC_OPERAND_USERCTL               opUserControlCode;  // User Control Code
    UINT8               opDeckControlMode;  // Deck control mode
    UINT8               opDeckInfo;  // Deck Info
    UINT8               opStatusRequest;  // Status Request
    UINT8               opPlayMode;  // Play Mode
    CEC_OPERAND_OSD_NAME         opDevOsdName;
    UINT8                                     opPwrSts;  // Power Status
    UINT8                                     opVersion; // VPP_CEC_VERSION_TYPE
    UINT32		opVendorID;
    CEC_OPERAND_ROUTING_CHANGE   opRoutingChange;

#ifdef BUDDY_RELEASE
    CEC_VENDOR_CMD_WITH_ID opVendCmdWithID;
#endif

}CEC_MSG_OPERAND;

// Structure of CEC message passed to/from application
typedef struct _CEC_msg_
{
    UINT8                   srcAddr;
    UINT8                   destAddr;
    UINT16                  opcode;
    UINT8                    operandlen;
    CEC_MSG_OPERAND     operand;
}CEC_MSG;

/* TX/RX data structure */
typedef struct tagCEC_TASK_DATA
{
    UINT8 cmd;
    struct
    {
        UINT8 mode      : 2; //Unicast/Broadcast
        UINT8 retry     : 1; //Valid for only transmit
        UINT8 reserved  : 5;
    }msg_cfg;
    UINT8 status;
    UINT8 msg_len;
    UINT8 msg_fifo[16];
    UINT32 txtime;
}CEC_TASK_DATA;

/*CEC config structure*/
typedef struct _CECConfig
{
    INT32 iDeviceType; /* device type get from MV_VPP_EVENT_CEC_LOG_ADDR_STS msg. */
    INT32 iLogicAddr; /* logic address get from MV_VPP_EVENT_CEC_LOG_ADDR_STS msg. */
    UINT32 uiPhyAddr; /* Physic address read from EDID. */
    INT32 bActiveSource;
    INT32 bValid;
} CECConfig;

typedef struct _CEC_msg_table_
{
    UINT16 msg_opcode           : 8;
    UINT16 msg_len              : 5;
    UINT16 msg_mode             : 2;
    UINT16 msg_rxaction_reqd    : 1;
}CEC_MSG_TABLE;

#endif  /* CECENABLE */
#endif  /* __CEC_TYPE_H__ */
