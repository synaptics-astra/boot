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
#ifndef __CEC_PRV_H__
#define __CEC_PRV_H__
#ifdef CECENABLE

/*-----------------------------------------------------------------------------
 * Macros and Constants
 *-----------------------------------------------------------------------------
 */
/* CEC GPIO */

#define SM_CEC_BASE		0x40010000
#define SM_CEC_SIZE		0x00001000	//4k
#define SM_CEC_END		(SM_CEC_BASE + SM_CEC_SIZE)
#define SM_CEC_ALIAS_SIZE	0x00001000	// 4k

/* Circular buffer constants */
#define CEC_FIFO_LENGTH                    3
#define CEC_FIFO_WRAP_AROUND(index)        (((index)>=(CEC_FIFO_LENGTH))?0:(index))

/* Bit timings in microseconds */
#define CEC_SIGNAL_FREE_TIME                12000
#define CEC_START_BIT_LOW_TIME              3700
#define CEC_START_BIT_HI_TIME               800
#define CEC_START_BIT_TOT_TIME              (CEC_START_BIT_LOW_TIME + CEC_START_BIT_HI_TIME)
#define CEC_START_BIT_JITTER_TIME           (200 + 30)

#define CEC_DATA_BIT_0_LOW_TIME             1500
#define CEC_DATA_BIT_0_HI_TIME              900
#define CEC_DATA_BIT_1_LOW_TIME             600
#define CEC_DATA_BIT_1_HI_TIME              1800
#define CEC_DATA_BIT_TOT_TIME               (CEC_DATA_BIT_0_LOW_TIME + CEC_DATA_BIT_0_HI_TIME)
#define CEC_DATA_BIT_LOW_JITTER_TIME        (200 + 30)
#define CEC_DATA_BIT_JITTER_TIME            (350 + 30)
#define CEC_ACK_SSP_TIME                    1050
#define CEC_NOMINAL_SAMPLE_TIME             1050
#define CEC_FOLL_ACK_ASSRT_TIME             1500
#define CEC_TX_SIGNAL_FREE_TIME             (7 * CEC_DATA_BIT_TOT_TIME)
#define CEC_RETX_SIGNAL_FREE_TIME           (5 * CEC_DATA_BIT_TOT_TIME)
#define CEC_FOLL_HYST_TIME                  800

#define CEC_BIT_ERR_NOTIF_TIME              3600
#define CEC_COLL_WINDOW_TIME                250
#define CEC_MAX_RISE_TIME                   250

/* CEC Bit Types */
#define CEC_DATA_BIT_0                      0
#define CEC_DATA_BIT_1                      1
#define CEC_START_BIT                       2
#define CEC_ERROR_BIT                       3
#define CEC_UNKNOWN_BIT                     4

#define FALL_EDGE                           0
#define RISE_EDGE                           1

/*Max retransmission time */
#define MAX_RETRANS_TIME                    5

/* CEC bit protocol states */
typedef enum tagCEC_STATE
{
	CEC_STATE_IDLE = 0,

	CEC_STATE_TX_TRIG,
	CEC_STATE_TX_START,
	CEC_STATE_TX_DATA,
	CEC_STATE_TX_EOM,
	CEC_STATE_TX_WAIT_FOR_ACK,
	CEC_STATE_TX_NEXT_BYTE,

	CEC_STATE_RX_START,
	CEC_STATE_RX_DATA,
	CEC_STATE_RX_SEND_ACK,
	CEC_STATE_RX_NEXT_BYTE,
	CEC_STATE_RX_SEND_ERR,
}CEC_STATE;

/* CEC state triggers */
typedef enum tagCEC_TRIGGER
{
	CEC_TRIGGER_NONE = 0,
	CEC_TRIGGER_EDGE,
	CEC_TRIGGER_TIMER,
	CEC_TRIGGER_DATA,
	CEC_TRIGGER_INTERNAL,
	CEC_TRIGGER_MAX
}CEC_TRIGGER;

/* CEC bit protocol states */
typedef enum tagCEC_TASK_STATE
{
	CEC_TASK_STATE_IDLE = 0,
	CEC_TASK_STATE_LISTENING,
	CEC_TASK_STATE_SENDING,
	CEC_TASK_STATE_MAX
}CEC_TASK_STATE;

typedef enum tagCEC_MODE
{
	CEC_MODE_IDLE = 0,
	CEC_MODE_TX,
	CEC_MODE_RX
}CEC_MODE;

/* Transmission Failed reason */
typedef enum tagCEC_FAIL_REASON
{
	CEC_FAIL_NONE = 0,
	CEC_TX_FAIL_LINE_BUSY,
	CEC_TX_FAIL_SIGNAL_FREE_TIME,
	CEC_TX_FAIL_INSTART,
	CEC_TX_FAIL_INDATA,
	CEC_TX_FAIL_NACK,
}CEC_FAIL_REASON;

typedef struct tagCEC_TASK_DATA_FIFO
{
	UINT8 ucStart_index;
	UINT8 ucEnd_index;
	CEC_TASK_DATA ucFifo_buf[CEC_FIFO_LENGTH];
}CEC_TASK_DATA_FIFO;

/*-----------------------------------------------------------------------------
 * Static Data
 *-----------------------------------------------------------------------------
 */
static CEC_TASK_DATA_FIFO stCec_Isr_Fifo;
static CEC_TASK_DATA_FIFO stCec_App_Fifo;

#ifdef HWCEC
#else
static INT32    iCec_edge_time[3];
static INT32	iCec_aided_time[3];
static INT32    iCec_time_index = 0;
static INT32    iCec_det_edge = 0;

static UINT8    ucCec_bitcntr;
static UINT8    ucCec_bytecntr;
static UINT8    ucCec_mode  = CEC_MODE_IDLE;
static UINT8    ucCec_state = CEC_STATE_IDLE;
#endif /* HWCEC */
static UINT8    ucCec_retrans_time = 0;

static CEC_TASK_DATA    ucCec_tx_data;
static CEC_TASK_DATA    ucCec_rx_data;

static CEC_TASK_STATE   SMCECTaskstate=CEC_TASK_STATE_IDLE;

/*-----------------------------------------------------------------------------
 * Static Function Prototypes
 *-----------------------------------------------------------------------------
 */

#ifndef HWCEC
/*-----------------------------------------------------------------------------
 * GPIO Interfaces
 *-----------------------------------------------------------------------------
 */
void GPIO_SET_DIR_IN();

/*-----------------------------------------------------------------------------
 * Timer Interfaces
 *-----------------------------------------------------------------------------
 */
static INT32 GetTimeUS();
static void cec_trigger_timer(int delay);
static void cec_cancel_timer();
static void delay (INT32 iTime_delay);
#endif

/*-----------------------------------------------------------------------------
 * CEC fifo interfaces
 *-----------------------------------------------------------------------------
 */
static UINT8 cec_write_fifo(CEC_TASK_DATA_FIFO *pFifo, CEC_TASK_DATA *pData);
static UINT8 cec_read_fifo(CEC_TASK_DATA_FIFO *pFifo, CEC_TASK_DATA *pData);

/*-----------------------------------------------------------------------------
 * CEC bit protocol
 *-----------------------------------------------------------------------------
 */
#ifndef HWCEC
static UINT8 cec_record_edge(INT32 iCur_time, INT32);
static UINT8 cec_detect_bit();
static UINT8 cec_ack_block (INT32 iEdge_time, INT32);
static UINT8 cec_send_bit(UINT8 ucBit, UINT8 ucDataOut);
static void cec_select_int_edge ( void );
static void cec_trigger_sgnlfreechk(void);

static void cec_clear_data(CEC_TASK_DATA *pData);
static void cec_send_status (CEC_TASK_DATA *pData, UINT8 cmd, UINT8 status);
static void cec_set_mode (UINT8 mode);

static void cec_handle_trigger (UINT8 ucTrigger);
static UINT8 cec_mode_idle (UINT8 ucTrigger, INT32 iCurr_time);
static UINT8 cec_mode_tx (UINT8 ucTrigger, INT32 iCurr_time);
static UINT8 cec_mode_rx (UINT8 ucTrigger, INT32 iCurr_time, INT32);
#endif
#endif  /* CECENABLE */
#endif  /* __CEC_PRV_H__ */
