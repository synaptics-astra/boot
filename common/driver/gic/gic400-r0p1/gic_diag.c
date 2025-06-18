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
#include "apbRegBase.h"
#include "gic_diag.h"
#include "debug.h"

typedef struct {
    int irq_id;
    void (*isr)(void);
    int is_used;
}isr_entry_t;

#define NUM_MP_CORE         4
#define MASTER_MP_CORE      0

#define MAX_ISR_ENTRY 4 //define max isr entry to 4

static int is_irq_inited = 0;
static isr_entry_t isr_entry[MAX_ISR_ENTRY];

#define GIC_BASE                    MEMMAP_GIC_REG_BASE

#define GICD_BASE                   0x1000
#define GICC_BASE                   0x2000

#define GICD_CTLR                   (GICD_BASE + 0x00)          // Distributor Control Register
#define GICD_TYPER                  (GICD_BASE + 0x04)          // Interrupt Controller Type Register
#define GICD_IGROUPRn               (GICD_BASE + 0x080)         // Interrupt Group Registers
#define GICD_ISENABLERn             (GICD_BASE + 0x100)         // Interrupt Set-Enable Registers
#define GICD_ICENABLERn             (GICD_BASE + 0x180)         // Interrupt Clear-Enable Registers
#define GICD_ICPENDRn               (GICD_BASE + 0x280)         // Interrupt Clear-Pending Registers
#define GICD_ISACTIVERn             (GICD_BASE + 0x300)         // Interrupt Set-Active Registers
#define GICD_ICACTIVERn             (GICD_BASE + 0x380)         // Interrupt Clear-Active Registers
#define GICD_IPRIORITYRn            (GICD_BASE + 0x400)         // Interrupt Priority Registers
#define GICD_ITARGETSRn             (GICD_BASE + 0x800)         // Interrupt Processor Targets Registers
#define GICD_ICFGRn                 (GICD_BASE + 0xC00)         // Interrupt Configuration Registers
#define GICD_SGIR                   (GICD_BASE + 0xF00)         // Software Generated Interrupt Register
#define GICD_ICPIDR2                (GICD_BASE + 0xFE8)         // Peripheral ID2 Register

#define GICC_CTLR                   (GICC_BASE + 0x00)          // CPU Interface Control Register
#define GICC_PMR                    (GICC_BASE + 0x04)          // Interrupt Priority Mask RegisteR
#define GICC_BPR                    (GICC_BASE + 0x08)          // Binary Point Register
#define GICC_IAR                    (GICC_BASE + 0x0C)          // Interrupt Acknowledge Register
#define GICC_EOIR                   (GICC_BASE + 0x10)          // End of Interrupt Register

#if 0
uint32_t read32_f(uint32_t reg)
{
	uintptr_t addr = GIC_BASE + (reg);
	uint32_t value = 0;

	INFO("read addr = 0x%08x\n", addr);

	value = read32((void *)addr);

	return value;
}

void write32_f(uint32_t reg, uint32_t v)
{
	uintptr_t addr = GIC_BASE + (reg);
	uint32_t value = v;

	INFO("write addr = 0x%08x, value = 0x%08x\n", addr, value);

	write32((void *)addr, value);
}

#define GIC_REG_READ(reg)           read32_f(reg)//(*(volatile unsigned int*)((uintptr_t)(GIC_BASE+(reg))))
#define GIC_REG_WRITE(reg, v)       write32_f(reg, v) //(*(volatile unsigned int*)((uintptr_t)(GIC_BASE+(reg))) = (v))
#else
#define GIC_REG_READ(reg)           (*(volatile unsigned int*)((uintptr_t)(GIC_BASE+(reg))))
#define GIC_REG_WRITE(reg, v)       (*(volatile unsigned int*)((uintptr_t)(GIC_BASE+(reg))) = (v))
#endif
#define GIC_REG_WRITE_WITH_MASK(a, v, m)                      GIC_REG_WRITE(a, (GIC_REG_READ(a)&(~(m))) | ((v)&(m)))
#define GIC_REG_SET_1BIT_WITH_MASK(base, index, value)        GIC_REG_WRITE_WITH_MASK(base+(index/32*4), (value)<<((index)&0x1f), (1<<((index)&0x1f)))
#define GIC_REG_SET_8BIT_WITH_MASK(base, index, value)        GIC_REG_WRITE_WITH_MASK(base+(index/4*4), (value)<<(8*((index)&0x3)), (0xff<<(8*((index)&0x3))))

#define GIC_REG_SET_1BIT(base, index, value)        GIC_REG_WRITE(base+(index/32*4), (value)<<((index)&0x1f))



#define GIC_PPI_NUM                 (0x20)
#define GIC_SPI_NUM                 (IRQ_TOTAL_IRQ)
#define MAX_GIC_IRQ_NUM             (GIC_PPI_NUM + GIC_SPI_NUM)


extern uint32_t getMPid(void);
extern void EnableIRQ(void);
void GIC_IRQ_Handler(void)
{
    unsigned int MPCoreInterruptID;
    int irq_id, i = 0;
    //int MPid = getMPid();
    MPCoreInterruptID = GIC_REG_READ(GICC_IAR);                            // reading ID from Acknowledge register changes the interrupt from Pending to Active
    irq_id = MPCoreInterruptID & 0x3FF;
    if (irq_id == 0x3FF)
	{
        INFO("GIC IRQ spurious irq\n");
		GIC_REG_WRITE(GICC_EOIR, MPCoreInterruptID);
        return;
    }
    //dbg_printf(PRN_DBG, " Gic IRQ %d received\n", irq_id);
    for(i = 0; i < MAX_ISR_ENTRY; i++) {
        if(isr_entry[i].irq_id == irq_id) {
            isr_entry[i].isr();
            break;
        }
    }
    GIC_REG_WRITE(GICC_EOIR, MPCoreInterruptID);
    return;
}
#if 0
void GIC_FIQ_Handler(void)
{
    unsigned int IAR_value;
    int irq_id, i = 0;
    int MPid = getMPid();
    IAR_value = GIC_REG_READ(GICC_IAR);                            // reading ID from Acknowledge register changes the interrupt from Pending to Active
    irq_id = IAR_value & 0x3FF;
    if (irq_id == 0x3FF)
    {
        INFO("GIC FIG spurious irq\n");
        return;
}
	INFO(" Gic FIQ %d received at CPU%d\n", irq_id, MPid);
    for(i = 0; i < MAX_ISR_ENTRY; i++) {
        if(isr_entry[i].irq_id == irq_id) {
            isr_entry[i].isr();
            break;
        }
    }
    GIC_REG_WRITE(GICC_EOIR, IAR_value);
    return;
}
#endif
#if 0
int diag_GICSetHandle(int irq, irq_handle_func handle, void* param)
{
    if (irq >= ARRAY_NUM(gic_irq_handles))
    {
        return -1;
    }
    INFO("IRQ set handle, irq:%d, old handle:0x%08x, new handle:0x%08x\n",
                irq, gic_irq_handles[irq], handle);
    gic_irq_handles[irq] = handle;
    gic_irq_handles_param[irq] = param;
    return 0;
}
#endif
static void fix_for_missing_ack()
{
    int i;
    if (0 == (GIC_REG_READ(GICD_CTLR) & 1))
    {
        return;
    }
    for (i = 0; i < MAX_GIC_IRQ_NUM; i += 32)
    {
        unsigned int temp;
        temp = GIC_REG_READ(GICD_ISACTIVERn+(i/32)*4);
        if (temp)
        {
            int j;
            for (j = 0; j < 32; j++)
			{
				if (temp & (1<<j))
				{
                    GIC_REG_WRITE(GICC_EOIR, i + j);
				}
			}
		}
	}
}
#if 0
static void disable_irqs()
{
    arm_set_CNTP_CTL(0);
    diag_ictl_disable_all(0, 0);
    diag_GICSetHandle(MP_BERLIN_INTR_ID(IRQ_ictlInst0CPUIrq),  ictl_irq_handle,  (void*)0);
    diag_GICSetInt(MASTER_MP_CORE, MP_BERLIN_INTR_ID(IRQ_ictlInst0CPUIrq), INT_ENABLE);
    diag_ictl_disable_all(0, 1);
    diag_GICSetHandle(MP_BERLIN_INTR_ID(IRQ_ictlInst1CPUIrq),  ictl_irq_handle, (void*)1);
    diag_GICSetInt(MASTER_MP_CORE, MP_BERLIN_INTR_ID(IRQ_ictlInst1CPUIrq), INT_ENABLE);
    diag_ictl_disable_all(0, 2);
    diag_GICSetHandle(MP_BERLIN_INTR_ID(IRQ_ictlInst2CPUIrq),  ictl_irq_handle, (void*)2);
    diag_GICSetInt(MASTER_MP_CORE, MP_BERLIN_INTR_ID(IRQ_ictlInst2CPUIrq), INT_ENABLE);
    diag_ictl_disable_all(1, 0);
    diag_GICSetHandle(MP_BERLIN_INTR_ID(IRQ_sm2socHwInt0),     sm_ictl_irq_handle, (void*)0);
    diag_GICSetInt(MASTER_MP_CORE, MP_BERLIN_INTR_ID(IRQ_sm2socHwInt0), INT_ENABLE);
    diag_ictl_disable_all(1, 1);
    diag_GICSetHandle(MP_BERLIN_INTR_ID(IRQ_sm2socHwInt1),     sm_ictl_irq_handle, (void*)1);
    diag_GICSetInt(MASTER_MP_CORE, MP_BERLIN_INTR_ID(IRQ_sm2socHwInt1), INT_ENABLE);
}
#endif

//this function initialize interrupt 0 to 63
//1. Distributor Interface
//   a) Enable All interrupts.
//   b) Do not route any interrupt to CPU0 and CPU1, use this as the control to turn on interrupt at CPU0 or CPU1
//   c) All level-sensitive and 1-N (only one CPU will handle the interrupt)
//   d) All interrupts have highest priority
//2. CPU Interface
//   a) Priority Mask is lowest
//   b) Pre-empty All interrupts
void initMPGIC(void)
{
    unsigned int MPid = getMPid();
    int i;
    unsigned int temp;
	  INFO("Mpid = %d\n", MPid);

    if (MPid == MASTER_MP_CORE)
    {
        fix_for_missing_ack();
        GIC_REG_WRITE(GICC_CTLR, 0);        // Disable interrupts in GIC CPU Interface
        GIC_REG_WRITE(GICD_CTLR, 0);        // Disable interrupts in GIC Distributor

        for (i = 0; i < MAX_GIC_IRQ_NUM; i += 32)
        {
            GIC_REG_WRITE(GICD_IGROUPRn     +(i/32)*4, 0xFFFFFFFF);    // all generate IRQ
            GIC_REG_WRITE(GICD_ICENABLERn+(i/32)*4, 0xFFFFFFFF);    // Disable all interrupt sources
            GIC_REG_WRITE(GICD_ICPENDRn+(i/32)*4,   0xFFFFFFFF);    // Clear all pending interrupts

            // In case interrupt is not acked, clear active interrupt
        }

    //Note that SetEnable0_31 is banked for each core
    GIC_REG_WRITE(GICD_ISENABLERn,   0xFFFFFFFF);   // Enable all SGI PPI
        GIC_REG_WRITE(GICD_IGROUPRn,    0xFFFFFFFF);    //all NS interrupts, will generate IRQ

    // check configurated cpu numbers for GIC
    temp = GIC_REG_READ(GICD_TYPER);
    if ((((temp>>5)&0x7) + 1) > 1) // more than one core
    {
        // We set distributor's GICD_ITARGETSR to enable/disable interrupt for one core while keeping interrupt enabled.
        // For single core, this method doesn't work because interrupt always is send to core. We should use disable/enable on interrupt.
            for (i = 32; i < MAX_GIC_IRQ_NUM; i += 32)
            {
                GIC_REG_WRITE(GICD_ISENABLERn+(i/32)*4, 0xFFFFFFFF);
            }
        }


        // vpp uses FIQ

        // Set all interrupt priorities to high.
        for (i = 0; i < MAX_GIC_IRQ_NUM; i += 4)
        {
            GIC_REG_WRITE(GICD_IPRIORITYRn+(i/4)*4, 0x80808080); // all generate IRQ
        }
        //GIC_REG_SET_1BIT_WITH_MASK(GICD_IGROUPRn,    (IRQ_dHubIntrAvio0+32), 0);
        //GIC_REG_SET_8BIT_WITH_MASK(GICD_IPRIORITYRn, (IRQ_dHubIntrAvio0+32), 0); // set to highest priority

        //set interrupt goes to none of the CPU0, will be turned on later
        for (i = 0; i < MAX_GIC_IRQ_NUM; i += 4)
        {
            GIC_REG_WRITE(GICD_ITARGETSRn+(i/4)*4, 0);
         }

        // Set all interrupt sources to be level-sensitive and 1-N software model
        // 1-N/N-N bit may be obsolete
        for (i = 0; i < MAX_GIC_IRQ_NUM; i += 16)
        {
            GIC_REG_WRITE(GICD_ICFGRn+(i/16)*4, 0x55555555);
        }


    // These CPU interface registers are banked for each core
    // Enable all interrupt priorities (apart from the lowest priority, 0xF)
    // Note that bits [3:0] of this register are not implemented but could be in future
    GIC_REG_WRITE(GICC_PMR, 0xFF);
    GIC_REG_WRITE(GICC_BPR, 0x3);           // Enable pre-emption on all interrupts

    //have both Secure and NS interrupt to CPU
    //CPU will ack both S and NS interruts
    //S interrupts will generate FIQ
    GIC_REG_WRITE(GICC_CTLR, 0x7);          // [3]: FIQEn, [2]:AckCtl, [1]: EnableGrp1, [0]: EnableGrp0

        //disable_irqs();

        GIC_REG_WRITE(GICD_CTLR, 0x3);      // [1]: EnableGrp1, [0]: EnableGrp0
    }
    else
    {
        GIC_REG_WRITE(GICC_CTLR, 0);            // Disable interrupts in GIC CPU Interface
        GIC_REG_WRITE(GICD_ISENABLERn,  0xFFFFFFFF);    // Enable all SGI PPI
        GIC_REG_WRITE(GICD_IGROUPRn,    0xFFFFFFFF);    //all NS interrupts, will generate IRQ
        GIC_REG_WRITE(GICC_PMR, 0xFF);          // Enable all interrupt priorities (apart from the lowest priority, 0xF)
        GIC_REG_WRITE(GICC_BPR, 0x3);           // Enable pre-emption on all interrupts
        GIC_REG_WRITE(GICC_CTLR, 0xF);          // [3]: FIQEn, [2]:AckCtl, [1]: EnableGrp1, [0]: EnableGrp0
    }
}

//enable an interrupt
void setEnableMPGIC(unsigned int id)
{
    INFO("%s, EnableIRQ %d\n", __FUNCTION__, id);
    GIC_REG_SET_1BIT(GICD_ISENABLERn, id, 1);
}


//disable an interrupt
void clearEnableMPGIC(unsigned int id)
{
    GIC_REG_SET_1BIT(GICD_ICENABLERn, id, 1);
}

//this function turns an interrupt on or off for a particular CPU
//by routing the interrupt to that CPU at distributor interface
//this is only useful for intId >=32,
//for intId <= 32, Int target regs are read-only
int diag_GICSetInt(int MPid, int intId, int enable)
{
    unsigned int temp;

    // check configurated cpu numbers for GIC
    temp = GIC_REG_READ(GICD_TYPER);
    if ((((temp>>5)&0x7) + 1) > 1) // more than one core
    {
        GIC_REG_SET_8BIT_WITH_MASK(GICD_ITARGETSRn, intId, ((enable ? 1:0) << MPid));
    }
    else // one core only
    {
        if(enable)
            setEnableMPGIC(intId);
        else
            clearEnableMPGIC(intId);

        return 0;
    }

    return 0;
}


int diag_GicSGI(int cpuList, int sgiId)



    //GIC_Acknowledge is effectively banked.
    //that is for CPU0 and CPU1, they are really different registers even with same address



    // writing ID to End of Interrupt register changes the interrupt from Active to Inactive
{
    //int MPid = getMPid();

    //GIC_Acknowledge is effectively banked.
    //that is for CPU0 and CPU1, they are really different registers even with same address

    GIC_REG_WRITE(GICD_SGIR, (cpuList<<16) | sgiId); // Secure SGI
    GIC_REG_WRITE(GICD_SGIR, (cpuList<<16) | (1<<15) | sgiId); // // [15]: SATT=1 for Non-Secure SGI
    return 0;
}

void do_irq ()
{
	GIC_IRQ_Handler();
}
static void init_irq(void)
{
    int i = 0;

    for(i = 0; i < MAX_ISR_ENTRY; i++) {
        isr_entry[i].is_used = 0;
    }

    //GIC distribution interface is only needed to be initialized once
    //but the CPU interface needs to be initalized by both CPU cores
    //CPU interface registers are banked (same address)
    //So we will have both first and second CPU to call the same funtion
    dbg_printf(PRN_DBG,"%s, initMPGIC\n", __FUNCTION__) ;
    initMPGIC();

    INFO("%s, EnableIRQ\n", __FUNCTION__) ;
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
	  INFO("irq_id = %d\n", irq_id);
    diag_GICSetInt(getMPid(), MP_BERLIN_INTR_ID(irq_id), INT_ENABLE);
}

void reinitMPGIC(void)
{
	//GIC distribution interface is only needed to be initialized once
	//but the CPU interface needs to be initalized by both CPU cores
	//CPU interface registers are banked (same address)
	//So we will have both first and second CPU to call the same funtion
	INFO("%s, initMPGIC\n", __FUNCTION__) ;
	initMPGIC();

	INFO("%s, EnableIRQ\n", __FUNCTION__) ;
	// enable IRQ
	EnableIRQ();
}
/**/

