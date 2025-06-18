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
/*
*
* Public C file to initialise the MPCore Interrupt Distributor.
* This file contains one function.
*
* Description:
* This code sets up the primary interrupt controller (GIC) in the MPCore
* to generate an IRQ to CPU 0 or 1
*
* Implementation:
* After the MPCore GIC is initialised with enableMPGIC(), you can use
* setEnableMPGIC() and clearEnableMPGIC() to set and clear interrupt
* sources in the ARM11 MPCore.
*
* Inputs:
* None.
*
* Outputs:
* MPCore Interrupt Distributor registers.
*
* Return Value:
* None.
*/
#include "memmap.h"
#include "soc.h"
#include "apb_perf_base.h"
#include "gic_diag.h"
#include "pic.h"
#include "util.h"
#include "debug.h"

typedef struct {
    int irq_id;
    void (*isr)(void);
    int is_used;
}isr_entry_t;

#define MAX_ISR_ENTRY 4 //define max isr entry to 4

static int is_irq_inited = 0;
static isr_entry_t isr_entry[MAX_ISR_ENTRY];

extern void  IRQ_Handler(void);

static void InstallHighHandler(unsigned int handlerAddr, unsigned int vector)
{
        //load PC with content at current PC + 0x38,
        //since current PC is vector+0x8, the handler should be at vector+0x40
        REG_WRITE32(vector, 0xE59FF038);
        REG_WRITE32(vector+0x40, handlerAddr);
}

extern void EnableIRQ(void);
extern UINT32 getMPid(void);

#if 0
/*******************************************************************************
* Function:    PIC_SetPerPIC_PerVector
*
* Description: Set perPIC registers for sepcific vector. (Target)
*
* Inputs:      PIC_num-- PIC number.
*              vec_num--- Vector number.
*              fIntE--------FIQ interrupt enable bit, 32bits. each bit 1-- enable, 0 -- disable
*              nIntE--------IRQ interrupt enable bit, 32bits. each bit 1-- enable, 0 -- disable
* Outputs:     none
*
* Return:      none
*******************************************************************************/
void PIC_SetPerPIC_PerVector(UINT32 PIC_num, UINT32 vec_num, UINT32 fIntE, UINT32 nIntE)
{
        UINT32 temp;
        UINT32 mask;
        UINT32 base_addr=MEMMAP_PIC_REG_BASE+PIC_num*sizeof(SIE_perPIC);
        mask = ~(1<<vec_num);

        // set fIntE
        REG_READ32(base_addr+RA_perPIC_fIntE, &temp);
        temp &= mask;
        temp |= fIntE << vec_num;

        REG_WRITE32(base_addr+RA_perPIC_fIntE, temp);

        // set nIntE
        REG_READ32(base_addr+RA_perPIC_nIntE, &temp);
        temp &= mask;
        temp |= nIntE << vec_num;
//      temp = 0x80;
        REG_WRITE32(base_addr+RA_perPIC_nIntE, temp);
}


//this is for all the interrupts other than IRQ_dHubIntrAvio0
void Enable_IRQ_PIC(cpu_id, vec_num)
{
        PIC_SetPerPIC_PerVector(cpu_id, vec_num, INT_DISABLE, INT_ENABLE);
}

#endif

#if (BERLIN_CHIP_VERSION != BERLIN_BG2CD_A0)
//this function turns an interrupt on or off for a particular CPU
//by routing the interrupt to that CPU at distributor interface
//this is only useful for intId >=32,
//for intId <= 32, Int target regs are read-only

void diag_GICSetInt(int cpuId, int intId, int enable)
{
        unsigned int reg_num;
        unsigned int bit_pos;
        unsigned int temp;

        if(intId<32)
        {
                dbg_printf(PRN_RES, "Cannot set interrupt target for interrupt id%d(less than 32)\n", intId);
                return;
        }

        reg_num = intId / GIC_CPU_TARGETS_PER_WORD;
        bit_pos = (intId % GIC_CPU_TARGETS_PER_WORD)*8;

        //get current value
        temp = GIC_REGISTER(GIC_CPUTarget0_3_offset+reg_num*GIC_BYTES_PER_WORD);

        if(enable)
        {
                //set the bit
                temp |= 1<<(bit_pos+cpuId);

                if((temp&(0x3<<bit_pos))==0x3)
                        dbg_printf(PRN_RES, "Warning: intr %d is sent to both CPUs\n", intId);
        }
        else
        {
                //clear the bit
                temp &= ~(1<<(bit_pos+cpuId));
        }

        GIC_REGISTER(GIC_CPUTarget0_3_offset+reg_num*GIC_BYTES_PER_WORD)=temp;
}


#else

void writeEnableMPGIC(unsigned int id, unsigned int offset)
{
    unsigned int register_number;   // which GIC register: 0 = register for IDs 0-31, 1 = register for IDs 32-63
    unsigned int bit_position;      // bit position in that register: 0 = least significant bit (D0)

    register_number = id / GIC_INTERRUPTS_PER_WORD;       // compiler will optimise divide by a power of 2 into shift right
    bit_position = id - register_number * GIC_INTERRUPTS_PER_WORD;
    offset = offset + register_number * GIC_BYTES_PER_WORD;
    GIC_REGISTER(offset) = 1 << bit_position;
}
//enable an interrupt: 0 to 63
void setEnableMPGIC(unsigned int id)
{
    writeEnableMPGIC(id, GIC_SetEnable0_31_offset);
}
//disable an interrupt: 0 to 63
void clearEnableMPGIC(unsigned int id)
{
    writeEnableMPGIC(id, GIC_ClearEnable0_31_offset);
}
//updated for unicore version, MPid is a dummy
int diag_GICSetInt(int MPid, int intId, int enable)
{
    if(enable)
        setEnableMPGIC(intId);
    else
        clearEnableMPGIC(intId);
	return 0;
}
#endif //(BERLIN_CHIP_VERSION != BERLIN_BG2CD_A0)

#ifndef BERLIN_SOC_BG3CD
//this function initialize interrupt 0 to 63
//1. Distributor Interface
//   a) Enable All interrupts.
//   b) Do not route any interrupt to CPU0 and CPU1, use this as the control to turn on interrupt at CPU0 or CPU1
//   c) All level-sensitive and 1-N (only one CPU will handle the interrupt)
//   d) All interrupts have highest priority
//2. CPU Interface
//   a) Priority Mask is lowest
//       b) Pre-empty All interrupts
void initMPGIC(void)
{
    unsigned int temp;

    //set the SPI interrupt polarity right
    //2 cti interrupts are active low, so change their polarity
    //REG_WRITE32((MEMMAP_CPUSS_REG_BASE+RA_MoltresReg_sysIntPol), ((1<<IRQ_cti_nctiirq_0)|(1<<IRQ_cti_nctiirq_1)));

    GIC_CPUControl = GIC_CPU_CONTROL_DISABLE;       // Disable interrupts in GIC CPU Interface
    GIC_Control    = GIC_CONTROL_DISABLE;           // Disable interrupts in GIC Distributor

    GIC_ClearEnable0_31  = GIC_CLEAR_ENABLE_ALL;    // Disable all interrupt sources
    GIC_ClearEnable32_63 = GIC_CLEAR_ENABLE_ALL;

    GIC_ClearPending0_31  = GIC_CLEAR_PENDING_ALL;  // Clear all pending interrupts
    GIC_ClearPending32_63 = GIC_CLEAR_PENDING_ALL;

    GIC_SetEnable0_31  = GIC_SET_ENABLE_ALL;            // Enable all interrupt sources
#if (BERLIN_CHIP_VERSION != BERLIN_BG2CD_A0)
    GIC_SetEnable32_63 = GIC_SET_ENABLE_ALL;
#endif

    // Set all interrupt priorities to highest.
    for (temp = GIC_Priority0_3_offset; temp >= GIC_Priority60_63_offset; temp += GIC_PRIORITIES_PER_WORD)
    {
        GIC_REGISTER(temp) = GIC_PRIORITY_HIGHEST_ALL;
    }

#if (BERLIN_CHIP_VERSION != BERLIN_BG2CD_A0)
    //set interrupt goes to none of the CPU0, will be turned on later
    for (temp = GIC_CPUTarget0_3_offset; temp <= GIC_CPUTarget60_63_offset; temp += GIC_CPU_TARGETS_PER_WORD)
    {
        GIC_REGISTER(temp) = GIC_CPU_TARGETS_NONE;
    }
#endif

    // Set all interrupt sources to be level-sensitive and 1-N software model
    // 1-N/N-N bit may be obsolete
    GIC_Configuration0_15  = GIC_CONFIG_ALL_LEVEL_1N;
    GIC_Configuration16_31 = GIC_CONFIG_ALL_LEVEL_1N;
    GIC_Configuration32_47 = GIC_CONFIG_ALL_LEVEL_1N;
    GIC_Configuration48_63 = GIC_CONFIG_ALL_LEVEL_1N;

    // Enable all interrupt priorities (apart from the lowest priority, 0xF)
    // Note that bits [3:0] of this register are not implemented but could be in future
    GIC_PriorityMask = GIC_PRIORITY_MASK_LOWEST;

    GIC_BinaryPoint = GIC_PREEMPT_ALL;          // Enable pre-emption on all interrupts

    GIC_CPUControl = GIC_CPU_CONTROL_ENABLE;    // Enable interrupts in GIC CPU interface
    GIC_Control    = GIC_CONTROL_ENABLE;        // Enable interrupts in GIC Distributor
}
#else
//this function initialize interrupt 0 to 63
//1. Distributor Interface
//   a) Enable All interrupts.
//   b) Do not route any interrupt to CPU0 and CPU1, use this as the control to turn on interrupt at CPU0 or CPU1
//   c) All level-sensitive and 1-N (only one CPU will handle the interrupt)
//   d) All interrupts have highest priority
//2. CPU Interface
//   a) Priority Mask is lowest
//?      b) Pre-empty All interrupts
#define MASTER_MP_CORE 0
extern UNSG32 getMPid(void);
void initMPGIC(void)
{
	unsigned int temp;
	unsigned int MPid=getMPid();

	GIC_CPUControl = GIC_CPU_CONTROL_DISABLE;       // Disable interrupts in GIC CPU Interface

	if(MPid==MASTER_MP_CORE)
	{

		GIC_Control    = GIC_CONTROL_DISABLE;           // Disable interrupts in GIC Distributor

		GIC_ClearEnable0_31  = GIC_CLEAR_ENABLE_ALL;    // Disable all interrupt sources
		GIC_ClearEnable32_63 = GIC_CLEAR_ENABLE_ALL;
		GIC_ClearEnable64_95 = GIC_CLEAR_ENABLE_ALL;
		GIC_ClearEnable96_127 = GIC_CLEAR_ENABLE_ALL;

		GIC_ClearPending0_31  = GIC_CLEAR_PENDING_ALL;  // Clear all pending interrupts
		GIC_ClearPending32_63 = GIC_CLEAR_PENDING_ALL;
		GIC_ClearPending64_95 = GIC_CLEAR_PENDING_ALL;
		GIC_ClearPending96_127 = GIC_CLEAR_PENDING_ALL;

	}

	//Note that SetEnable0_31 is banked for each core
	GIC_SetEnable0_31  = GIC_SET_ENABLE_ALL;	// Enable all interrupt sources
	if(MPid==MASTER_MP_CORE)
	{
		GIC_SetEnable32_63 = GIC_SET_ENABLE_ALL;
		GIC_SetEnable64_95 = GIC_SET_ENABLE_ALL;
		GIC_SetEnable96_127 = GIC_SET_ENABLE_ALL;
	}

	GIC_Security0_31 = 0xFFFFFFFF; //all NS interrupts, will generate IRQ
	if(MPid==MASTER_MP_CORE)
	{
		GIC_Security32_63 = 0xFFFFFFFF; //all NS interrupts, will generate IRQ
		GIC_Security64_95 = 0xBFFFFFFF; //only 94 is S interrupt so it will generate FIQ
		GIC_Security96_127 = 0xFFFFFFFF; //all NS interrupts, will generate IRQ
	}

	if(MPid==MASTER_MP_CORE)
	{
		// Set all interrupt priorities to high.
		for (temp = GIC_Priority0_3_offset; temp <= GIC_Priority124_127_offset; temp += GIC_PRIORITIES_PER_WORD)
		{
			GIC_REGISTER(temp) = GIC_PRIORITY_HIGH_ALL;
		}
		GIC_REGISTER(GIC_Priority92_95_offset) &= 0xFF00FFFF; //make SPI 94 highest priority

		//set interrupt goes to none of the CPU0, will be turned on later
		for (temp = GIC_CPUTarget0_3_offset; temp <= GIC_CPUTarget124_127_offset; temp += GIC_CPU_TARGETS_PER_WORD)  
		{
			GIC_REGISTER(temp) = GIC_CPU_TARGETS_NONE;
		}

		// Set all interrupt sources to be level-sensitive and 1-N software model
		// 1-N/N-N bit may be obsolete
		for (temp = GIC_Configuration0_15_offset; temp <= GIC_Configuration112_127_offset; temp += GIC_CONFIG_PER_WORD )
		{
			GIC_REGISTER(temp) = GIC_CONFIG_ALL_LEVEL_1N;
		}
	}

	// These CPU interface registers are banked for each core
	// Enable all interrupt priorities (apart from the lowest priority, 0xF)
	// Note that bits [3:0] of this register are not implemented but could be in future
	GIC_PriorityMask = GIC_PRIORITY_MASK_LOWEST;

	GIC_BinaryPoint = GIC_PREEMPT_ALL;          // Enable pre-emption on all interrupts

	//have both Secure and NS interrupt to CPU
	//CPU will ack both S and NS interruts
	//S interrupts will generate FIQ
	GIC_CPUControl = (GIC_CPU_CONTROL_ENABLE
		| GIC_CPU_CONTROL_ENABLE_NS
		| GIC_CPU_CONTROL_ACKCTL
		| GIC_CPU_CONTROL_FIQEN);    // Enable interrupts in GIC CPU interface


	if(MPid==MASTER_MP_CORE)
	{
		GIC_Control    = (GIC_CONTROL_ENABLE | GIC_CONTROL_ENABLE_NS);        // Enable NS and S interrupts in GIC Distributor
	}
}

#endif

//__irq void GIC_IRQ_Handler(void)
void GIC_IRQ_Handler(void)
{
    unsigned int MPCoreInterruptID; //DTS for build warning
    //unsigned int data; DTS for build warning
    int i = 0;

    //GIC_Acknowledge is effectively banked.
    //that is for CPU0 and CPU1, they are really different registers even with same address
    MPCoreInterruptID = GIC_Acknowledge;                            // reading ID from Acknowledge register changes the interrupt from Pending to Active
    //dbg_printf(PRN_DBG," Gic IRQ received, MPCoreInterruptID = %d\n", MPCoreInterruptID);

    for(i = 0; i < MAX_ISR_ENTRY; i++) {
        if(isr_entry[i].irq_id == (MPCoreInterruptID & GIC_INT_ACK_MASK)) {
            isr_entry[i].isr();
            break;
        }
    }

    // writing ID to End of Interrupt register changes the interrupt from Active to Inactive
    GIC_EndInterrupt = MPCoreInterruptID;
}


static void init_irq(void)
{
    int i = 0;

    for(i = 0; i < MAX_ISR_ENTRY; i++) {
        isr_entry[i].is_used = 0;
    }
    dbg_printf(PRN_DBG,"%s, InstallHighHandler\n", __FUNCTION__) ;
    InstallHighHandler((unsigned int)IRQ_Handler, MEMMAP_HIGH_EXCEPT_IRQ_REG);

    //GIC distribution interface is only needed to be initialized once
    //but the CPU interface needs to be initalized by both CPU cores
    //CPU interface registers are banked (same address)
    //So we will have both first and second CPU to call the same funtion
    dbg_printf(PRN_DBG,"%s, initMPGIC\n", __FUNCTION__) ;
    initMPGIC();

    dbg_printf(PRN_DBG,"%s, EnableIRQ\n", __FUNCTION__) ;
    // enable IRQ
    EnableIRQ();
}

int register_isr(void (*isr)(void), int irq_id)
{
    int i = 0;
    dbg_printf(PRN_DBG, "register irq_id = %d\n", irq_id);

    if(is_irq_inited != 1) {
        init_irq();
        is_irq_inited = 1;
    }

    for(i = 0; i < MAX_ISR_ENTRY; i++) {
        if(isr_entry[i].is_used == 0) {
            isr_entry[i].isr = isr;
            isr_entry[i].irq_id = MP_BERLIN_INTR_ID(irq_id);
            isr_entry[i].is_used = 1;
            break;
        }
    }
    if(i >= MAX_ISR_ENTRY) {
        dbg_printf(PRN_ERR, "######exceed the max number of irq!\n");
        return 1;
    }
    return 0;
}

void set_irq_enable(int irq_id)
{
    diag_GICSetInt(getMPid(), MP_BERLIN_INTR_ID(irq_id), INT_ENABLE);
}
