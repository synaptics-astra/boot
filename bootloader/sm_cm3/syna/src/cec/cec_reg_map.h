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

// --  CEC

#define CEC_BASE 0x0 // CEC is now part of SM in BG2

#define CEC_TOGGLE_FOR_WRITE_REG_ADDR        CEC_BASE + 0x0000 
#define CEC_TOGGLE_FOR_READ_REG_ADDR         CEC_BASE + 0x0004 
#define CEC_RDY_ADDR                         CEC_BASE + 0x0008 //01
#define CEC_RX_RDY_ADDR                      CEC_BASE + 0x000c //01
#define CEC_TX_FIFO_RESET_ADDR               CEC_BASE + 0x0010 //03
#define CEC_RX_FIFO_RESET_ADDR               CEC_BASE + 0x0014 //03
#define CEC_PMODE_ADDR                       CEC_BASE + 0x0018 //01
//#define CEC_TX_RDY_ADDR                      CEC_BASE + 0x001c //01
#define CEC_TX_TYPE_ADDR                     CEC_BASE + 0x0020 //01
//#define CEC_TX_RESP_TIME_0_ADDR              CEC_BASE + 0x0024 //FF
//#define CEC_TX_RESP_TIME_1_ADDR              CEC_BASE + 0x0025 //FF
//#define CEC_TX_RESP_TIME_2_ADDR              CEC_BASE + 0x0026 //FF
//#define CEC_TX_RESP_TIME_3_ADDR              CEC_BASE + 0x0027 //FF
#define CEC_SIGNAL_FREE_TIME_0_ADDR          CEC_BASE + 0x0028 //FF
#define CEC_SIGNAL_FREE_TIME_1_ADDR          CEC_BASE + 0x0029 //FF
#define CEC_SIGNAL_FREE_TIME_2_ADDR          CEC_BASE + 0x002a //FF
#define CEC_SIGNAL_FREE_TIME_3_ADDR          CEC_BASE + 0x002b //FF
#define CEC_START_BIT_LO_THRESH_0_ADDR       CEC_BASE + 0x002c //FF
#define CEC_START_BIT_LO_THRESH_1_ADDR       CEC_BASE + 0x002d //FF
#define CEC_START_BIT_LO_THRESH_2_ADDR       CEC_BASE + 0x002e //FF
#define CEC_START_BIT_LO_THRESH_3_ADDR       CEC_BASE + 0x002f //FF
#define CEC_START_BIT_HI_THRESH_0_ADDR       CEC_BASE + 0x0030 //FF
#define CEC_START_BIT_HI_THRESH_1_ADDR       CEC_BASE + 0x0031 //FF
#define CEC_START_BIT_HI_THRESH_2_ADDR       CEC_BASE + 0x0032 //FF
#define CEC_START_BIT_HI_THRESH_3_ADDR       CEC_BASE + 0x0033 //FF
#define CEC_DATA_BIT_0_LO_THRESH_0_ADDR      CEC_BASE + 0x0034 //FF
#define CEC_DATA_BIT_0_LO_THRESH_1_ADDR      CEC_BASE + 0x0035 //FF
#define CEC_DATA_BIT_0_LO_THRESH_2_ADDR      CEC_BASE + 0x0036 //FF
#define CEC_DATA_BIT_0_LO_THRESH_3_ADDR      CEC_BASE + 0x0037 //FF
#define CEC_DATA_BIT_1_LO_THRESH_0_ADDR      CEC_BASE + 0x0038 //FF
#define CEC_DATA_BIT_1_LO_THRESH_1_ADDR      CEC_BASE + 0x0039 //FF
#define CEC_DATA_BIT_1_LO_THRESH_2_ADDR      CEC_BASE + 0x003a //FF
#define CEC_DATA_BIT_1_LO_THRESH_3_ADDR      CEC_BASE + 0x003b //FF
#define CEC_DATA_BIT_0_HI_THRESH_0_ADDR      CEC_BASE + 0x003c //FF
#define CEC_DATA_BIT_0_HI_THRESH_1_ADDR      CEC_BASE + 0x003d //FF
#define CEC_DATA_BIT_0_HI_THRESH_2_ADDR      CEC_BASE + 0x003e //FF
#define CEC_DATA_BIT_0_HI_THRESH_3_ADDR      CEC_BASE + 0x003f //FF
#define CEC_DATA_BIT_1_HI_THRESH_0_ADDR      CEC_BASE + 0x0040 //FF
#define CEC_DATA_BIT_1_HI_THRESH_1_ADDR      CEC_BASE + 0x0041 //FF
#define CEC_DATA_BIT_1_HI_THRESH_2_ADDR      CEC_BASE + 0x0042 //FF
#define CEC_DATA_BIT_1_HI_THRESH_3_ADDR      CEC_BASE + 0x0043 //FF
#define CEC_SSP_ACK_TIME_0_ADDR              CEC_BASE + 0x0044 //FF
#define CEC_SSP_ACK_TIME_1_ADDR              CEC_BASE + 0x0045 //FF
#define CEC_SSP_ACK_TIME_2_ADDR              CEC_BASE + 0x0046 //FF
#define CEC_SSP_ACK_TIME_3_ADDR              CEC_BASE + 0x0047 //FF
#define CEC_INTR_ENABLE0_REG_ADDR            CEC_BASE + 0x0048 //FF
#define CEC_INTR_ENABLE1_REG_ADDR            CEC_BASE + 0x0049 //FF
#define CEC_DATA_REG_ADDR                    CEC_BASE + 0x004c //FF
#define CEC_EOM_REG_ADDR                     CEC_BASE + 0x0050 //01
#define CEC_INTR_STATUS0_REG_ADDR            CEC_BASE + 0x0058
#define CEC_INTR_STATUS1_REG_ADDR            CEC_BASE + 0x0059
#define CEC_NOMINAL_SAMPLE_TIME_0_ADDR       CEC_BASE + 0x005c //FF
#define CEC_NOMINAL_SAMPLE_TIME_1_ADDR       CEC_BASE + 0x005d //FF
#define CEC_NOMINAL_SAMPLE_TIME_2_ADDR       CEC_BASE + 0x005e //FF
#define CEC_NOMINAL_SAMPLE_TIME_3_ADDR       CEC_BASE + 0x005f //FF
#define CEC_HYST_TIME_0_ADDR                 CEC_BASE + 0x0060 //FF
#define CEC_HYST_TIME_1_ADDR                 CEC_BASE + 0x0061 //FF
#define CEC_HYST_TIME_2_ADDR                 CEC_BASE + 0x0062 //FF
#define CEC_HYST_TIME_3_ADDR                 CEC_BASE + 0x0063 //FF
#define CEC_FOLLOWER_ACK_TIME_0_ADDR         CEC_BASE + 0x0064 //FF
#define CEC_FOLLOWER_ACK_TIME_1_ADDR         CEC_BASE + 0x0065 //FF
#define CEC_FOLLOWER_ACK_TIME_2_ADDR         CEC_BASE + 0x0066 //FF
#define CEC_FOLLOWER_ACK_TIME_3_ADDR         CEC_BASE + 0x0067 //FF
#define CEC_RX_BUF_READ_REG_ADDR             CEC_BASE + 0x0068 
#define CEC_RX_EOM_READ_REG_ADDR             CEC_BASE + 0x0069 
#define CEC_LOGICAL_ADDR0_REG_ADDR           CEC_BASE + 0x006a //1F
#define CEC_LOGICAL_ADDR1_REG_ADDR           CEC_BASE + 0x006b //1F
#define CEC_LOGICAL_ADDR2_REG_ADDR           CEC_BASE + 0x006c //1F
#define CEC_LOGICAL_ADDR3_REG_ADDR           CEC_BASE + 0x006d //1F
#define CEC_LOGICAL_ADDR4_REG_ADDR           CEC_BASE + 0x006e //1F
#define CEC_JITTER_CNT_0_ADDR                CEC_BASE + 0x0070 //FF
#define CEC_JITTER_CNT_1_ADDR                CEC_BASE + 0x0071 //FF
#define CEC_JITTER_CNT_2_ADDR                CEC_BASE + 0x0072 //FF
#define CEC_JITTER_CNT_3_ADDR                CEC_BASE + 0x0073 //FF
#define CEC_LINE_STATUS_REG_ADDR             CEC_BASE + 0x0074 
#define CEC_TX_PRESENT_STATE_REG_ADDR        CEC_BASE + 0x0078 
#define CEC_RX_PRESENT_STATE_REG_ADDR        CEC_BASE + 0x0079 
#define CEC_COLL_CTRL_REG_ADDR               CEC_BASE + 0x007a //03
//#define CEC_GLITCH_FILTER_STAGES_ADDR        CEC_BASE + 0x007b //07
#define CEC_COLL_WINDOW_TIME_REG_0_ADDR      CEC_BASE + 0x007c //FF
#define CEC_COLL_WINDOW_TIME_REG_1_ADDR      CEC_BASE + 0x007d //FF
#define CEC_COLL_WINDOW_TIME_REG_2_ADDR      CEC_BASE + 0x007e //FF
#define CEC_COLL_WINDOW_TIME_REG_3_ADDR      CEC_BASE + 0x007f //FF
#define CEC_TX_FIFO_FULL_THRESH              CEC_BASE + 0x0080 //0F
#define CEC_TX_FIFO_WPTR                     CEC_BASE + 0x0081 
#define CEC_TX_FIFO_RPTR                     CEC_BASE + 0x0082 
#define CEC_TX_FIFO_DPTR                     CEC_BASE + 0x0083 
#define CEC_RX_FIFO_FULL_THRESH              CEC_BASE + 0x0084 //0F
#define CEC_RX_FIFO_WPTR                     CEC_BASE + 0x0085 
#define CEC_RX_FIFO_RPTR                     CEC_BASE + 0x0086 
#define CEC_RX_FIFO_DPTR                     CEC_BASE + 0x0087
#define CEC_JITTER_CNT_SB_0                  CEC_BASE + 0x0088 //FF
#define CEC_JITTER_CNT_SB_1                  CEC_BASE + 0x0089 //FF
#define CEC_JITTER_CNT_SB_2                  CEC_BASE + 0x008a //FF
#define CEC_JITTER_CNT_SB_3                  CEC_BASE + 0x008b //FF
#define CEC_ERR_NOTIF_TIME_0                 CEC_BASE + 0x008c //FF
#define CEC_ERR_NOTIF_TIME_1                 CEC_BASE + 0x008d //FF
#define CEC_ERR_NOTIF_TIME_2                 CEC_BASE + 0x008e //FF
#define CEC_ERR_NOTIF_TIME_3                 CEC_BASE + 0x008f //FF
#define CEC_GLITCH_FILT_W_L                  CEC_BASE + 0x0090 //FF
#define CEC_GLITCH_FILT_W_H                  CEC_BASE + 0x0091 //FF
