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
#define	MC_ID_REV	  0x000
#define	MC_STAT		  0x004
#define	SR_STAT		  0x008

#define	ADDR_MAP_CS0	  0x010
#define	ADDR_MAP_CS1	  0x014
#define	ADDR_MAP_CS2	  0x018
#define	ADDR_MAP_CS3	  0x01C
#define	SR_CFG_T1_CS0	  0x020
#define	SR_CFG_T1_CS1	  0x024
#define	SR_CFG_T1_CS2	  0x028
#define	SR_CFG_T1_CS3	  0x02C
#define	SR_CFG_T2_CS0	  0x030
#define	SR_CFG_T2_CS1	  0x034
#define	SR_CFG_T2_CS2	  0x038
#define	SR_CFG_T2_CS3	  0x03C

#define	SR_CTL1		  0x050
#define	SR_CTL2		  0x054
#define	SR_CTL4		  0x058
#define	SR_CTL6		  0x05C
#define	SR_CTL7		  0x060
#define	SR_CTL13	  0x064
#define	SR_CTL14	  0x068

#define	SR_TIMING1	  0x080
#define	SR_TIMING2	  0x084
#define	SR_TIMING3	  0x088
#define	SR_TIMING4	  0x08C
#define	SR_TIMING5	  0x090
#define	SR_TIMING6	  0x094
#define	SR_TIMING7	  0x098

#define	EXCL_MON_CTL	  0x100

#define	TZ_SEL		  0x120
#define	TZ_RANGE0	  0x124
#define	TZ_RANGE1	  0x128
#define	TZ_PERMIT	  0x12C

#define	PRIORITY_CTL	  0x140
#define	PRIORITY_UP	  0x144

#define	ITSR_CTL1	  0x150
#define	ITSR_CTL2	  0x154
#define	ITSR_CTL3	  0x158

#define	USER_CMD0	  0x160
#define	USER_CMD1	  0x164

#define	MRR_DATA	  0x170

#define	SMR1		  0x180
#define	SMR2		  0x184

#define	REG_TAB_CTL0	  0x1C0
#define	REG_TAB_DATA0	  0x1C8
#define	REG_TAB_DATA1	  0x1CC

#define	SR_PAD_CAL	  0x200
#define	FTDLL_CFG	  0x204
#define	PHY_CTL1	  0x208
#define	PHY_CTL2	  0x20C
#define	PHY_CTL4	  0x210
#define	PHY_CTL5	  0x214
#define	PHY_CTL6	  0x218

#define	PHY_CTL3	  0x220

#define	PHY_CTL7	  0x230
#define	PHY_CTL8	  0x234
#define	PHY_CTL9	  0x238
#define	PHY_CTL10	  0x23C
#define	PHY_CTL11	  0x240
#define	PHY_CTL12	  0x244
#define	PHY_CTL13	  0x248
#define	PHY_CTL14	  0x24C
#define	PHY_CTL15	  0x250
#define	PHY_CTL16	  0x254
#define	PHY_CTL21	  0x258

#define	PHY_CTL19	  0x280
#define	PHY_CTL20	  0x284
#define	PHY_CTL22	  0x288

#define	PHY_DATA_BSEL	  0x300
#define	PHY_DLL_CTL	  0x304
#define	PHY_DATA_BCTL	  0x308

#define	PHY_CTL_WL_SEL	  0x380
#define	PHY_CTL_WL_CTL0	  0x384
#define	PHY_CTL_WL_CTL1	  0x388
#define	PHY_CTL_WL_CTL2	  0x38C
#define	PHY_CTL_RL_CTL	  0x390

#define	PHY_CTL_TM	  0x400
#define	TEST_MODE0	  0x410
#define	TEST_MODE1	  0x414

#define	PC_CFG		  0x440
#define	PC_STAT		  0x444
#define	PC_CTL		  0x448
#define	PC_COUNTER0	  0x450
#define	PC_COUNTER1	  0x454
#define	PC_COUNTER2	  0x458
#define	PC_COUNTER3	  0x45C

