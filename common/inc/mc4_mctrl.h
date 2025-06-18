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
#ifndef performance_counter_h
#define performance_counter_h

	#define		w32Perc_Config	 	{									\
			UINT32				uConfig_pc_event_sel0		:  5;		\
			UINT32				rsvd0						:  1;		\
			UINT32				uConfig_pc_int_en0			:  1;		\
			UINT32				uConfig_pc_reg_en0		 	:  1;		\
			UINT32				uConfig_pc_event_sel1		:  5;		\
			UINT32				rsvd1						:  1;		\
			UINT32				uConfig_pc_int_en1			:  1;		\
			UINT32				uConfig_pc_reg_en1		 	:  1;		\
			UINT32				uConfig_pc_event_sel2		:  5;		\
			UINT32				rsvd2						:  1;		\
			UINT32				uConfig_pc_int_en2			:  1;		\
			UINT32				uConfig_pc_reg_en2		 	:  1;		\
			UINT32				uConfig_pc_event_sel3		:  5;		\
			UINT32				rsvd3						:  1;		\
			UINT32				uConfig_pc_int_en3			:  1;		\
			UINT32				uConfig_pc_reg_en3		 	:  1;		\
		}

	#define		w32Perc_Status	 	{									\
			UINT32				uStatus_pc_overflow0		:  1;		\
			UINT32				uStatus_pc_overflow1		:  1;		\
			UINT32				uStatus_pc_overflow2		:  1;		\
			UINT32				uStatus_pc_overflow3		:  1;		\
			UINT32				rsvd0						:  28;		\
		}

	#define		w32Perc_Control	 	{									\
			UINT32				uControl_pc_start_cond		:  1;		\
			UINT32				rsvd0						:  3;		\
			UINT32				uControl_pc_stop_cond		:  1;		\
			UINT32				rsvd1						:  11;		\
			UINT32				uControl_pc_clk_div			:  3;		\
			UINT32				rsvd2						:  13;		\
		}

    typedef union  T32Perc_Config
          { UINT32 u32;
            struct w32Perc_Config;
                 } T32Perc_Config;

    typedef union  T32Perc_Status
          { UINT32 u32;
            struct w32Perc_Status;
                 } T32Perc_Status;

    typedef union  T32Perc_Control
          { UINT32 u32;
            struct w32Perc_Control;
                 } T32Perc_Control;

	/*
		Port0: CPU
		Port1: AVIO (VPP)
		Port2: AVIO (AG)
		Port3: Peripheral / MTEST
		Port4: ZSP / GC360 (Vector Graphics)
		Port5: Gfx3D (GC1000_3D)
		Port6: vMeta
		Port7: Gfx2D (GC1000_2D)
	*/
	#define		w32port_Priority_Control	 	{						\
			UINT32				priority_p0					:  2;		\
			UINT32				priority_p1					:  2;		\
			UINT32				priority_p2					:  2;		\
			UINT32				priority_p3					:  2;		\
			UINT32				priority_p4					:  2;		\
			UINT32				priority_p5					:  2;		\
			UINT32				priority_p6					:  2;		\
			UINT32				priority_p7					:  2;		\
			UINT32				rsvd1						:  4;		\
			UINT32				low_latency_disable			:  1;		\
			UINT32				block_high					:  1;		\
			UINT32				block_low					:  1;		\
			UINT32				rsvd2						:  1;		\
			UINT32				port_prio_en		 		:  8;		\
		}

    typedef union  T32port_Priority_Control
          { UINT32 u32;
            struct w32port_Priority_Control;
                 } T32port_Priority_Control;

#endif
