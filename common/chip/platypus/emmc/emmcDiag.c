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
//#include "diag_common.h"
//#include "diag_console.h"
//#include "diag_misc.h"
#include "com_type.h"
//#include "printf.h"
#include "debug.h"
#include "string.h"
#include "io.h"

//#include "gic.h"
#include "Galois_memmap.h"
#include "global.h"
#include "soc.h"
#include "irq.h"

#ifdef CONFIG_DCACHE
#include "cache.h"
#endif
#include "sdmmc_api.h"
#ifdef __SDIO_INT_MODE__
#include "gic_diag.h"
#endif //__SDIO_INT_MODE__

#include "emmcHC.h"
#include "emmcDiag.h"

#define UNUSED(var) do { (void)(var); } while(0)

//#define DEBUG_PRN
#ifdef DEBUG_PRN
#define EMMC_PRN(PRN_LEVEL,fmt, args...) dbg_printf(PRN_LEVEL, fmt, ## args)
#else
#define EMMC_PRN(PRN_LEVEL,fmt, args...)
#endif

#define EMMC_REG_BASE 	(MEMMAP_EMMC_REG_BASE)

#define htobe16(x)      ((x&0x00ff) << 8 | (x&0xff00) >> 8)
#define htobe32(x)      ((x&0x000000ff) << 24 | (x&0x0000ff00) << 8 \
		| (x&0x00ff0000) >> 8 | (x&0xff000000) >> 24)

static SDMMC_Properties_T EMMC_Prop;	// for emmc
static P_MM4_SDMMC_T pEMMCCTRL;
static MM4_SDMMC_CONTEXT_T EMMCContext;

#if 0
extern int g_ADMA_wr;
extern int g_ADMA_rd;
extern unsigned int g_device_sector_count;
extern unsigned int g_manufacturerID;
extern unsigned int g_serialNum;
#endif
int g_ADMA_wr = 0;
int g_ADMA_rd = 0;
unsigned int g_device_sector_count = 0;
unsigned int g_manufacturerID = 0;
unsigned int g_serialNum;

unsigned int rpmbPart_sector_count = 0;
unsigned int g_emmc_WP_info=0;//byte[3]=CSD[13],byte[2]=CSD[12],byte[1]=EXT_CSD[173],byte[0]=EXT_CSD[171]
static int autocmd12 = 0;

//#define VELOCE_PLAT
#ifdef VELOCE_PLAT
#define PLATFORM VELOCE
#endif

void EMMC_Get_CSD(P_SDMMC_Properties_T pSDMMCP);
void EMMC_Get_CID(P_SDMMC_Properties_T pSDMMCP);
UINT_T EMMC_Switch_HS(P_SDMMC_Properties_T pSDMMCP);

extern int EMMCHC_PHYconfig(P_MM4_SDMMC_CONTEXT_T pContext, int rxsel);
extern void EMMCHC_PHYreset(P_MM4_SDMMC_CONTEXT_T pContext, int rst);
extern void SDHC_PHYDelayLineSetup(P_MM4_SDMMC_CONTEXT_T pContext);
extern void SDHC_PHYTuningSetup(P_MM4_SDMMC_CONTEXT_T pContext);
extern void SDHC_switch_1p8signal(P_MM4_SDMMC_CONTEXT_T pContext, int enable);
extern void SDHC_EnableInterrupt(P_MM4_SDMMC_CONTEXT_T pContext, int loc, unsigned int enable);

P_SDMMC_Properties_T EMMC_GetProperties(void)
{
    return &EMMC_Prop;
}

void EMMC_SetupProp(void)
{

	P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();
	pSDMMCP->pContext = &(EMMCContext);
	P_MM4_SDMMC_CONTEXT_T pSDCXT = (P_MM4_SDMMC_CONTEXT_T)pSDMMCP->pContext;

	pEMMCCTRL = (P_MM4_SDMMC_T)EMMC_REG_BASE;
	EMMC_PRN(PRN_INFO, "pEMMCCTRL[0] is %x\n ",(uintptr_t)pEMMCCTRL );

	pSDCXT->pMMC4Reg=(P_MM4_SDMMC_T)pEMMCCTRL;// Pointer to MMC control registers
	pSDMMCP->State=UNINITIALIZED;				// Indicate State of the card
}

static void __attribute__((used)) set_bits(uintptr_t reg_addr, unsigned int mask, int value)
{
	unsigned int reg_value;

	reg_value = read32((void *)reg_addr);
	reg_value &= ~(mask);
	reg_value |= (value&mask);
	write32((void *)reg_addr, reg_value);
}

static void emmc_setpinmux(void)
{
}

void EMMC_SetupPinmux(void)
{
    EMMC_PRN(PRN_RES,"setup emmc pinmux\n");
    //pinmux_write(0, E_EMMC_RSTn,0);
    emmc_setpinmux();
}

void emmc_Intr_On(void)
{
#ifdef __SDIO_INT_MODE__
	if(register_isr(EMMC_ISR, IRQ_emmc_int))
            EMMC_PRN(PRN_ERR, "mmc4 isr can't be registered\n");

        set_irq_enable(IRQ_emmc_int);
#endif		// __SDIO_INT_MODE__
}

/****************************************************************
*    EMMC_CheckVoltageCompatibility
*		Checks to make sure that the OCR register of the device supports the
*		voltage range that was selected for the controller
*    Input:
* 		P_MM4_SDMMC_CONTEXT_T pContext, UINT_T ControllerVoltage
*    Output:
*		none
*    Returns:
*       none
*****************************************************************/
UINT_T EMMC_CheckVoltageCompatibility(P_SDMMC_Properties_T pSDMMCP, UINT_T ControllerVoltage)
{
#if PLATFORM==VELOCE
	return NoError;
#endif
	if ((pSDMMCP->CardReg.OCR & (MMC_OCR_VOLTAGE_ALL|SD_OCR_VOLTAGE_1_8)) )
		return NoError;
	EMMC_PRN(PRN_RES,"device voltage not supported, pSDMMCP->CardReg.OCR=0x%x\n",pSDMMCP->CardReg.OCR);

	UNUSED(ControllerVoltage);

	return SDMMCDeviceVoltageNotSupported;
}

/* PHY RX SEL modes */
#define SCHMITT1P8		0x1
#define SCHMITT3P3		0x2
#define SCHMITT1P2		0x3
#define COMPARATOR1P8		0x4
#define COMPARATOR1P2		0x5
#define COMPARATOR1P82		0x6
#define INTERNALLPBK		0x7
unsigned int emmc_bus_vol = 1;  //0-3.3V, 1-1.8V.

void EMMC_Slot_init(void)
{
	UINT_T addr, offset, value;
	P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();
	P_MM4_SDMMC_CONTEXT_T pContext;
	P_MM4_CAP1_2	pMM4_CAP0;
	int ret = 0;

	EMMC_PRN(PRN_RES, "setup host\n");

	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;
	pMM4_CAP0  = (P_MM4_CAP1_2) &pContext->pMMC4Reg->mm4_cap1_2;
	EMMC_PRN(PRN_RES,"*pMM4_CAP0 is %x, voltage supported bits: %d%d%d\n",
			*pMM4_CAP0, pMM4_CAP0->vlg_33_support,
			pMM4_CAP0->vlg_30_support, pMM4_CAP0->vlg_18_support);

	// need to do swreset to host controller per SPEC
	EMMCHC_FullSWReset(pContext);
	mdelay(5);

	addr= EMMC_REG_BASE;
	BFM_HOST_Bus_Read32(addr + 0xE8, &offset);
	EMMC_PRN(PRN_INFO, "0x%x = 0x%x\n", addr+0xE8, offset);
	offset &= 0xFFF;
	offset += 0x2C;

	BFM_HOST_Bus_Read32(addr + offset, &value);
	value |= 0x1;	//EMMC_CTRL_R.CARD_IS_EMMC=1
	BFM_HOST_Bus_Write32(addr + offset, value);

	BFM_HOST_Bus_Read32(addr + 0x3C, &value);
	value = value & (~(0x1<<24));	//HOST_CTRL2_R.UHS2_IF_ENABLE =0
	value = value & (~(0x7<<16));	//HOST_CTRL2_R.UHS_MODE_SEL =0
	BFM_HOST_Bus_Write32(addr + 0x3C, value);

	// BG7 update, PHY initialization involved.
	BFM_HOST_Bus_Read32(addr + 0x28, &value);
	value = value & (~(0x7<<9));
	if(!emmc_bus_vol && pMM4_CAP0->vlg_33_support)
		value = value | (0x7<<9);//0-3.3V:7
	else if(pMM4_CAP0->vlg_18_support)
		value = value | (0x6<<9); //1-1.8V:6
    BFM_HOST_Bus_Write32(addr + 0x28, value);   //PWR_CTRL_R.SD_BUS_VOL_VDD1 = 7-3.3V

	SDHC_switch_1p8signal(pContext, 1);

	ret = EMMCHC_PHYconfig(pContext, SCHMITT1P8);
	dbg_printf(PRN_RES, "PHY Config %s!\n", ret==0?"PASS":"FAIL");

	// de-assert PHY reset
	EMMCHC_PHYreset(pContext, 1);
	UNUSED(pMM4_CAP0);
}

void EMMC_PhyInitialization(void)
{
/* do nothing due to API change for VS680 */
}

/**********************************************************
*   EMMCHC_Init
*      Initializes the HC
*   Input:
*	   none
*   Output:
*      none
*   Returns:
*      WTP recoginized Success/Fail return code
***********************************************************/
UINT_T EMMCHC_Init(void)
{
	//uintptr_t addr;
    //UINT_T value;

    P_MM4_SDMMC_CONTEXT_T pContext;

    // Initialize Flash Properties

    P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();

    // Assign pContext and clock values


    pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;
    //addr= EMMC_REG_BASE;
    EMMC_Slot_init();

#ifndef CONFIG_FPGA
	//ZL: Dolphin_BG7_PHY_bring_up_sequence.xlsx
	//step 2~8
	SDHC_PHYDelayLineSetup(pContext);
	//ZL: Dolphin_BG7_PHY_bring_up_sequence.xlsx
	//step 9
	SDHC_PHYTuningSetup(pContext);
#endif

	if(!EMMCHC_SetControllerVoltage(pContext, 1))
    {
        EMMC_PRN(PRN_RES, "Host doesn't support 1.8V\n");
        return -1;
    }

    // Enable clocks to slow speed
#ifdef CONFIG_FPGA
	EMMCHC_SetBusRate(pContext, SDCLK_SEL_DIV_16);
#else
	EMMCHC_SetBusRate(pContext, SDCLK_SEL_DIV_1024);
#endif
    //----------------------------------------------------

	// Set Read Response Timeout
	EMMCHC_SetDataTimeout(pContext, CLOCK_27_MULT);

	// Enable power

	// Unmask and Enable interrupts

	EMMCHC_EnableDisableIntSources(pContext, ENABLE_INTS);

	return 0;
}


int EMMC_WaitForCardDetect(void)
{
    P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();
    P_MM4_SDMMC_CONTEXT_T pSDCXT = (P_MM4_SDMMC_CONTEXT_T)pSDMMCP->pContext;
    int i=0;

    while(++i<8000000)
    {
        if(pSDCXT->pMMC4Reg->mm4_state & SDIO_CARD_PRESENT_BIT)
        break;
    }
    if (i==8000000)
    {
        EMMC_PRN(PRN_RES," No card preset!\n");
        return 0;
    }
    else
    {
        EMMC_PRN(PRN_RES," card preset. switch power on \n");
        return 1;
    }
}

/******************************************************************************
  Description:
    Set up the registers of the controller to start the transaction to
    communicate to the card for data related command.  The commands are clearly defined in the MMC
    specification.
  Input Parameters:
	   P_SDMMC_Properties_T pSDMMCP - Generic SD/MMC driver properties structure
  Cmd
		Command Index - See MMC or SD specification
  argument
      	the argument of  the command. MSW is for ARGH and LSW is for ARGL
  BlockType
	  	Multiple or Single Block Type
  ResType
		Expected response type
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void EMMC_SendDataCommand (P_SDMMC_Properties_T pSDMMCP,
                  UINT_T Cmd,
                  UINT_T  Argument,
                  UINT_T BlockType,
                  UINT_T DataDirection,
                  UINT_T ResType)
{
	 // Assign our context value
	 P_MM4_SDMMC_CONTEXT_T pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

	 // no need to clear out any fault state that may be left over from a previously failed transaction.
	 // that's because the caller has set State to read or write before calling here.

	 // No Response to the command yet
	 pSDMMCP->CardReponse.CommandComplete = 0;
	 pSDMMCP->Trans.RespType = (ResType >> 8) & 0x000000ff;
	 pSDMMCP->Trans.Cmd = Cmd;	// Fixme: how to know when to set the ACMD flag?

	 EMMC_PRN(PRN_INFO,"\n ready to send cmd %02d arg=0x%08x (resp_type=0x%02X) ",Cmd,Argument,ResType);

	 EMMCHC_SendDataCommand(pContext,
	                  Cmd,
	                  Argument,
	                  BlockType,
	                  DataDirection,
	                  ResType & 0x000000ff);
}

/******************************************************************************
  Description:
    Set up the registers of the controller to start the transaction to
    communicate to the card for data related command.  The commands are clearly defined in the MMC
    specification.
  Input Parameters:
	   P_SDMMC_Properties_T pSDMMCP - Generic SD/MMC driver properties structure
  Cmd
		Command Index - See MMC or SD specification
  argument
      	the argument of  the command. MSW is for ARGH and LSW is for ARGL
  BlockType
	  	Multiple or Single Block Type
  ResType
		Expected response type
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void EMMC_SendDataCommandNoAuto12 (P_SDMMC_Properties_T pSDMMCP,
                  UINT_T Cmd,
                  UINT_T  Argument,
                  UINT_T BlockType,
                  UINT_T DataDirection,
                  UINT_T ResType)
{
	 // Assign our context value
	 P_MM4_SDMMC_CONTEXT_T pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

	 // no need to clear out any fault state that may be left over from a previously failed transaction.
	 // that's because the caller has set State to read or write before calling here.

	 // No Response to the command yet
	 pSDMMCP->CardReponse.CommandComplete = 0;

	 // save the info for use by the isr:
	 pSDMMCP->Trans.RespType = (ResType >> 8) & 0x000000ff;
	 pSDMMCP->Trans.Cmd = Cmd;	// Fixme: how to know when to set the ACMD flag?

	 EMMC_PRN(PRN_INFO,"\n ready to send cmd %02d arg=0x%08x (resp_type=0x%02X) ",Cmd,Argument,ResType);

	 EMMCHC_SendDataCommandNoAuto12(pContext,
	                  Cmd,
	                  Argument,
	                  BlockType,
	                  DataDirection,
	                  ResType & 0x000000ff); // clear out any bits not for the SD_CMD.RES_TYPE field

}


/******************************************************************************
  Description:
    Set up the registers of the controller to start the transaction to
    communicate to the card for setup related commands.
    The commands are clearly defined in the MMC specification.
  Input Parameters:
	   P_SDMMC_Properties_T pSDMMCP - Generic SD/MMC driver properties structure
	Cmd
		Command Index - See MMC or SD specification
    argument
      	the argument of  the command. MSW is for ARGH and LSW is for ARGL
	ResType
		Expected response type
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void EMMC_SendSetupCommand(P_SDMMC_Properties_T pSDMMCP,
                  UINT_T Cmd,
                  UINT_T CmdType,
                  UINT_T Argument,
                  UINT_T ResType)
{
	 // Assign our context value
	 P_MM4_SDMMC_CONTEXT_T pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

	 pSDMMCP->State = READY;

	 // No Response to the command yet
	 pSDMMCP->CardReponse.CommandComplete = 0;

	 pSDMMCP->Trans.RespType = (ResType >> 8) & 0x000000ff;

	 EMMC_PRN(PRN_INFO,"\n ready to send cmd %02d arg=0x%08x (resp_type=0x%02X) ",Cmd,Argument,ResType);

	 EMMCHC_SendSetupCommand(pContext,
	                  Cmd,
	                  CmdType,
	                  Argument,
	                  ResType);
}


/****************************************************************
*   MM4_Read_Response
*	   	Reads the response from the Controller Buffer Registers to the Local Buffer.
* 		According to the last command and response type it does the correct interpretation.
*		There is also a timeout as the routine waits for the ISR to signal last command completion.
*   Input:
*	   	P_SDMMC_Properties_T pSDMMCP - Generic SD/MMC driver properties structure
* 		ResponseType - See SD/MMC specifications
*		ResponseTimeOut - A time out value in millisec's
*   Output:
*      	none
*   Returns:
*      	TimeOutError or NoError
*****************************************************************/
UINT_T EMMC_Interpret_Response(P_SDMMC_Properties_T pSDMMCP, UINT_T ResponseType, UINT_T ResponseTimeOut)
{
 	UINT_T temp, temp2, temp3;	// ElapsedMSecs;
 	UINT_T Retval = NoError;
 	P_MM4_SDMMC_CONTEXT_T pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;		// Assign our context value
 	P_MM4_CMD_XFRMD_UNION pMM4_XFRMD;
	volatile unsigned int i ;


	UNUSED(ResponseTimeOut);
 	EMMC_PRN(PRN_INFO," interpret response! \n");
//	delay_1us(5);
 	// Setup OS Timer register (Register 4)
 	//pContext->pOSTReg->omcr4 = 0x42;				// Increment evrey msec
 	// Clear the value;
 	//pContext->pOSTReg->oscr4 = 0;
 	// Overlap XFRMD register contents using uniun
 	pMM4_XFRMD = (P_MM4_CMD_XFRMD_UNION) &pContext->pMMC4Reg->mm4_cmd_xfrmd;
	pSDMMCP->CardReponse.R1_RESP = 0;
 	i=0;
#ifdef __SDIO_INT_MODE__
	while (!pSDMMCP->CardReponse.CommandComplete)
 	{
 		i++;
		udelay(1);
		if (i >= 100000000) break;
 	}

	if (i >= 100000000)
 	{
		EMMC_PRN(PRN_RES," Time out! \n");
 		Retval = TimeOutError;
 		return Retval;
 	}
#else 	// __SDIO_INT_MODE__ not defined
 	EMMC_PRN(PRN_INFO,"\n wait for cmd %02d (0x%08X) response\n",pMM4_XFRMD->mm4_cmd_xfrmd_bits.cmd_idx,pMM4_XFRMD->mm4_cmd_xfrmd_value);
 	// Wait for the Response based on the CommandComplete interrupt signal
	//while ((pContext->pMMC4Reg->mm4_i_stat & 0x00000001)==0)	//cmdcomp
	while (!pSDMMCP->CardReponse.CommandComplete)
 	{
 		i++;
		mdelay(1);
		EMMC_ISR();
		if (i >= 100000) break;
 	}

	if (i >= 100000)
 	{
		EMMC_PRN(PRN_RES," Time out \n");
 		Retval = TimeOutError;
 		//return Retval;
 	}
#endif

 	// Read in the Buffers
    switch (ResponseType)
    {
        case MMC_RESPONSE_NONE:
            break;
        case MMC_RESPONSE_R1:
        case MMC_RESPONSE_R1B:
            pSDMMCP->CardReponse.R1_RESP = pSDMMCP->CardReponse.pBuffer[0];

            EMMC_PRN(PRN_INFO, "\n The received R1B response is %x", pSDMMCP->CardReponse.R1_RESP);

            break;
        case MMC_RESPONSE_R2:			// This is for CID or CSD register
        {
	 	if (pMM4_XFRMD->mm4_cmd_xfrmd_bits.cmd_idx == XLLP_MMC_CMD9) //CSD
	 	{
			// Copy the CSD values from the buffer
			for (i=0; i<4; i++)
				pSDMMCP->CardReg.CSD.CSD_VALUE[i] = pSDMMCP->CardReponse.pBuffer[i];

			   // Optionally we could record maximum block lengths from the CSD.
			// But some devices cheat and put incorrect values in this field.
			// Save off read Block Size, play it safe, for now hard code to 512 Bytes
			pSDMMCP->ReadBlockSize = HARD512BLOCKLENGTH;
			// Save off Write Block Size
			pSDMMCP->WriteBlockSize = HARD512BLOCKLENGTH;

			// Capture Erase Granularity.
			if (pSDMMCP->SD == XLLP_MMC)
			{
				pSDMMCP->EraseSize = ((pSDMMCP->CardReg.CSD.CSD_VALUE[1] >> 5) & 0x1F) + 1;  		// Get ERASE_GRP_MULT
				pSDMMCP->EraseSize *= (((pSDMMCP->CardReg.CSD.CSD_VALUE[1] >> 10) & 0x1F) + 1);	// Get ERASE_GRP_SIZE
				pSDMMCP->EraseSize *= pSDMMCP->WriteBlockSize;
			}

			// Now calculate the capacity of this card
			temp = ((pSDMMCP->CardReg.CSD.CSD_VALUE[2] >> 16) & 0xF); 		// Get READ_BL_LEN
			temp = 1 << temp; 										// Now we have Max Block Length
			temp2 = ((pSDMMCP->CardReg.CSD.CSD_VALUE[1] >> 15) & 0x7) + 2; 	// Get C_SIZE_MULT
			temp2 = 1 << temp2;
			temp3 = ((pSDMMCP->CardReg.CSD.CSD_VALUE[1] >> 30) & 0x3);		// Get C_SIZE
			temp3 |= ((pSDMMCP->CardReg.CSD.CSD_VALUE[2] & 0x3FF) << 2);	// Get C_SIZE
			temp3++;
			pSDMMCP->CardCapacity = temp3 * temp2 * temp;		// Total Size of the card in Bytes
		}
	 	else // Assume CID
	 	{
			// Copy the CSD values from the buffer
			for (i=0; i<4; i++)
				pSDMMCP->CardReg.CID.CID_VALUE[i] = pSDMMCP->CardReponse.pBuffer[i];

			// Now capture the serial number from the CID - 32 bit number
			if (pSDMMCP->SD == XLLP_MMC)
			{
				pSDMMCP->CardReg.CID.SerialNum = (pSDMMCP->CardReg.CID.CID_VALUE[0] >> 16) & (0xFF);
				pSDMMCP->CardReg.CID.SerialNum |= (pSDMMCP->CardReg.CID.CID_VALUE[1] << 16);
			}
	 	}
 		break;
        }
        case MMC_RESPONSE_R3:
        {
            pSDMMCP->CardReg.OCR = pSDMMCP->CardReponse.pBuffer[0];
            break;
        }
        case MMC_RESPONSE_R4:				// These modes are not supported by the driver
        {
            break;
        }
        case MMC_RESPONSE_R5:
        {
            break;
        }
        case MMC_RESPONSE_R5B:
            break;
        case MMC_RESPONSE_R6:				// Publishes RCA for SD cards
        {
            break;
        }
        case MMC_RESPONSE_R7:
        {
            break;
        }
    }

     return Retval;
}

/****************************************************************
*   SDMMCGetMatchWaitCardStatus
*      	Gets the status of the card by issuing CMD 13. The rerturn from the routine is based
* 		on a check against the expected value which is passed in
*   Input:
*      	pSDMMCP - pointer to the current context
*      	MaxWaitMSec - Maximum wait time in millisec
* 	   	R1_Resp_Match - Desired Value to be matched
*   Output:
*      	none
*   Returns:
*      	none
*****************************************************************/
UINT_T EMMC_CheckCardStatus(P_SDMMC_Properties_T pSDMMCP, UINT_T R1_Resp_Match, UINT_T Mask)
{
    UINT_T argument, cardStatus, retval;
	P_MM4_SDMMC_CONTEXT_T pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;
	SDHC_EnableInterrupt(pContext, 0, 1);
	SDHC_EnableInterrupt(pContext, 1, 0);

    //send CMD13 to check the status of the card
    argument = pSDMMCP->CardReg.RCA;
    EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD13, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R1 | MM4_48_RES);
    retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);

    // Mask out undesired check bits
    cardStatus = (pSDMMCP->CardReponse.R1_RESP) & Mask;

    if(cardStatus&URGENT_BKOPS)
    {//Hynix engineer told to ingore this bit because write command will clear it.
		cardStatus&=~URGENT_BKOPS;
		EMMC_PRN(PRN_RES, "Background operations needed.\n");
    }

    if ((cardStatus == R1_Resp_Match) && (retval == NoError)) {
		//EMMC_PRN(PRN_RES, "cardStatus = 0x%x\n", cardStatus);
        return NoError;
    } else{
        EMMC_PRN(PRN_RES, "\n check state error, state is suppose to be 0x%x \n", R1_Resp_Match);
        EMMC_PRN(PRN_RES, "\n Current state is 0x%x.\n", cardStatus);
        return TimeOutError ;
    }
}


/***************************************************************
*   EMMC_SetBusWidth()
* 	Sets the Bus width highest bus width supported.
*   Input: width = 2: 8bit width, width=6:8bit DDR
*   Output:
*   Returns: NoError, ReadError or NotSupportedError
*
*****************************************************************/
UINT_T EMMC_SetBusWidth(P_SDMMC_Properties_T pSDMMCP, int width)
{
    UINT_T i;
    MMC_CMD6_OVERLAY Cmd6;
    P_MM4_SDMMC_CONTEXT_T pContext;
    volatile P_MM4_CNTL1 pMM4_CNTL1;
    UINT_T Retval = NoError;

    // Assign our context value
    pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;
    pMM4_CNTL1 = (volatile P_MM4_CNTL1) &pContext->pMMC4Reg->mm4_cntl1;

    // Issue CMD 6 to set BUS WIDTH bits in EXT_CSD register byte 183
    Cmd6.MMC_CMD6_Layout.Access = EXT_CSD_ACCESS_WRITE_BYTE;			//Write Byte
    Cmd6.MMC_CMD6_Layout.CmdSet = 0;					// Don't Care
    Cmd6.MMC_CMD6_Layout.Index = BUS_WIDTH_MMC_EXT_CSD_OFFSET; 		// Choose Bus Width
    Cmd6.MMC_CMD6_Layout.Reserved0 = 0;
    Cmd6.MMC_CMD6_Layout.Reserved1 = 0;
    Cmd6.MMC_CMD6_Layout.Value = (UINT8_T) width;	// Choose 8 bit mode.

    EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD6, MM4_CMD_TYPE_NORMAL, Cmd6.MMC_CMD6_Bits, MM4_RT_R1 | MM4_RT_BUSY | MM4_48_RES_WITH_BUSY);
    Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x10);

#if 0
//send CMD13 to check the status of the card
		Retval |= EMMC_CheckCardStatus(pSDMMCP, 0xE00, R1_LOCKEDCARDMASK);		// Make sure card is programming state
		if (Retval != NoError)
		{
			EMMC_PRN(PRN_ERR,"CMD6 status:(after changing bus width) 0x%x\n", pSDMMCP->CardReponse.R1_RESP);
	 		pSDMMCP->State = READY;
			return SDMMC_SWITCH_ERROR;
		}
#else
	// to compatible some special eMMC device.
    mdelay(1);
#endif
    //send CMD13 to check the status of the card
    i = 0;
    Retval = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
    while(Retval != NoError)
    {
	mdelay(1);
        Retval = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
        if(i>10000)
            break;
        i++;
    }
    if (Retval != NoError)
    {
        EMMC_PRN(PRN_ERR,"CMD13: check status(after changing bus width)\n");
        return SDMMCInitializationError;
    }

	if(width == 2 || width == 6)  //8bit SDR or DDR
		pMM4_CNTL1->ex_data_width = 1;
	else if(width == 0)   //1bit dat[0]
	{
		pMM4_CNTL1->ex_data_width = 0;	// buswidth is defined by data transfer width.
		pMM4_CNTL1->datawidth = 0;		// 1bit  DAT[0].
	}
	else if(width == 1 || width == 5)   //4bit SDR or DDR
	{
		pMM4_CNTL1->ex_data_width = 0;	// buswidth is defined by data transfer width.
		pMM4_CNTL1->datawidth = 1;		// 4bit.
	}

    return NoError;
}


/****************************************************************
*   EMMC_SendStopCommand
*	Issues a stop command for open ended read and write block operations
*
*   Input:
*      P_SDMMC_Properties_T pSDMMCP - pointer to the SDMMC context structure
*   Output:
*      none
*   Returns:
*      none
*****************************************************************/
void EMMC_SendStopCommand(P_SDMMC_Properties_T pSDMMCP)
{
	//P_MM4_SDMMC_CONTEXT_T pContext;
 	// Assign our context value
 	//pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

 	// Send a CMD 12 to stop transmissions.
	EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD12, MM4_CMD_TYPE_NORMAL, (uintptr_t)NULL, MM4_48_RES_WITH_BUSY);
	EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);
}


/**********************************************************
*   EMMC_Id
*      Identifies which type of card was inserted
*   Input:
*	   none
*   Output:
*      none
*   Returns:
*      WTP recoginized Success/Fail return code
***********************************************************/
unsigned int EMMC_Id(P_SDMMC_Properties_T pSDMMCP)
{
	//UINT_T vcc = 1;
	// Local Variables
	UINT_T AnyError = 0;
	UINT_T argument = 0;
    	UINT_T cnt = 0;
 	// Assign our context value
	P_MM4_SDMMC_CONTEXT_T pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

	// Enable power,	pControllerVoltage will be returned to up-level function for volatage checking.
/*	 if(EMMCHC_SetControllerVoltage(pContext, vcc))
		EMMCHC_Vcc(vcc);
        else
            return SDMMCDeviceVoltageNotSupported;
    */
	pContext->pMMC4Reg->mm4_blk_cntl = 0;

	// Check for High Capacity Cards First
	// Send CMD0 (GO_IDLE_STATE) to get card into idle state

	EMMC_PRN(PRN_INFO, "\n send CMD0 (GO_IDLE_STATE) to get card into idle state\n");
	argument= 0 ;
	EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD0, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_NONE | MM4_NO_RES);

	AnyError += EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_NONE, 0x10);
	EMMC_PRN(PRN_INFO, "\n CMD0 (GO_IDLE_STATE) done. Error number is %d\n",AnyError);

	pSDMMCP->SD = XLLP_MMC;

	// First time, pass NULL argument to get back values card is compatible with
	// Send appropriate CMD Sequence to Identify the type of card inserted
	argument = 0;
	pSDMMCP->CardReg.OCR = 0;			// Make sure to clear out OCR.
	cnt = 0;

	EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD0, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_NONE | MM4_NO_RES);
	AnyError += EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_NONE, 0x10);
	EMMC_PRN(PRN_INFO, "\n CMD0 (GO_IDLE_STATE) done. Error number is %d\n",AnyError);

	do
	{
		EMMC_PRN(PRN_INFO, "Send XLLP_MMC_CMD1 to MMC card\n");
		EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD1, MM4_CMD_TYPE_NORMAL, argument,  MM4_RT_R3|MM4_48_RES);
		AnyError += EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R3, 0x10);

//		if (cnt<20)
			argument = pSDMMCP->CardReg.OCR | 0x40000000;
//		else
//			argument = pSDMMCP->CardReg.OCR;

		EMMC_PRN(PRN_INFO, "\n CMD1 arg =0x%08X\n", argument);

		if (cnt > 10000)
			break;
		cnt++;
	} while (((pSDMMCP->CardReg.OCR & 0x80000000) != 0x80000000));

	if ((pSDMMCP->CardReg.OCR & 0x80000000) != 0x80000000)
	{
		EMMC_PRN(PRN_RES,"ID Card: NotFoundError,pSDMMCP->CardReg.OCR =0x%X\n",pSDMMCP->CardReg.OCR);
		return NotFoundError;
	}

    if (!(pSDMMCP->CardReg.OCR & (MMC_OCR_VOLTAGE_ALL|SD_OCR_VOLTAGE_1_8)) )
    {
        EMMC_PRN(PRN_RES,"device voltage not supported, pSDMMCP->CardReg.OCR=0x%x\n",pSDMMCP->CardReg.OCR);
        return SDMMCDeviceVoltageNotSupported;
    }
	// Assign Access Mode.
	if (pSDMMCP->CardReg.OCR & OCR_ACCESS_MODE_MASK)
		pSDMMCP->AccessMode = SECTOR_ACCESS;
	else
		pSDMMCP->AccessMode = BYTE_ACCESS;

	EMMC_PRN(PRN_INFO,"\n\n CMD1 Card OCR=0x%08x \n",pSDMMCP->CardReg.OCR);
	return NoError;
}


/**********************************************************
*   EMMC_Init
*      Initializes the inserted card
*   Input:
*	   none
*   Output:
*      none
*   Returns:
*      WTP recoginized Success/Fail return code
***********************************************************/
UINT_T EMMC_Init(void)
{
    UINT_T argument, Retval;
    //UINT_T controllervoltage;
    volatile unsigned int i=0;

    P_MM4_SDMMC_CONTEXT_T pContext;
    P_MM4_BLK_CNTL pMM4_BLK_CNTL;
    // Initialize Flash Properties

    P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();

    // Assign our context value
    pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

    //Must set MMC NUMBLK
    pMM4_BLK_CNTL = (P_MM4_BLK_CNTL) &pContext->pMMC4Reg->mm4_blk_cntl;
    pMM4_BLK_CNTL->blk_cnt = 1;
    pMM4_BLK_CNTL->xfr_blksz =512; //512 byte

#if SDIO_DMA
    pSDMMCP->SDMA_Mode = TRUE;
#else
    pSDMMCP->SDMA_Mode = FALSE;
#endif

	dbg_printf(PRN_INFO,"go to identification\n");
    Retval = EMMC_Id(pSDMMCP);

    if (Retval != NoError)
		return Retval;
	dbg_printf(PRN_INFO,"done!\n");
	// Set up State
    pSDMMCP->State = INITIALIZE;
/*
    Retval = EMMC_CheckVoltageCompatibility(pSDMMCP, controllervoltage);
    if (Retval != NoError)
		return SDMMCInitializationError; // We couldn't find any cards
    EMMC_PRN(PRN_INFO,"EMMC_CheckVoltageCompatibility done!\n");
*/
	//send CMD2 to get the CID numbers
    argument = NO_ARGUMENT;
    EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD2, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R2| MM4_136_RES);
    Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R2, 0x100);
#ifdef MMC_QUIRKS
	if (Retval != NoError) {
		i = 4;
		/* It has been seen that SEND_CID may fail on the first
		 * attampt, let's try a few more times
		 */
		do {
			EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD2, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R2| MM4_136_RES);
			Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R2, 0x100);
			if (NoError == Retval)
				break;
		} while(i--);
	}
#endif

    if (Retval != NoError)
    {
		dbg_printf(PRN_ERR,"CMD2: SDMMCInit Error Retval=0x%x \n",Retval);
		return SDMMCInitializationError;
    }

    EMMC_Get_CID(pSDMMCP);   //KT added ; print out CID

    pSDMMCP->CardReg.RCA = pSDMMCP->CardReg.CID.SerialNum & 0xFFFF0000;
    argument = pSDMMCP->CardReg.RCA;
    EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD3, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R1 | MM4_48_RES);
    Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);

    if (Retval != NoError)
    {
		dbg_printf(PRN_ERR,"CMD3: SDMMCInit Error Retval=0x%x \n",Retval);
		return SDMMCInitializationError;
    }

    EMMC_PRN(PRN_INFO,"\n CID_SerialNum=%08x  RCA=%x \n",pSDMMCP->CardReg.CID.SerialNum,pSDMMCP->CardReg.RCA);

    //send CMD13 to check the status of the card
    Retval = EMMC_CheckCardStatus(pSDMMCP, 0x700, R1_LOCKEDCARDMASK);		// Make sure card is stdby mode
    if (Retval != NoError)
    {
		dbg_printf(PRN_ERR,"CMD13: check status Error Retval=0x%x \n",Retval);
		return SDMMCInitializationError;
    }

    // Send CMD 9 to retrieve the CSD
    argument = pSDMMCP->CardReg.RCA;
    EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD9, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R2 | MM4_136_RES);
    Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R2, 0x100);

    EMMC_Get_CSD(pSDMMCP);   //KT added; print out CSD

    //send CMD7 to get card into transfer state
    argument = pSDMMCP->CardReg.RCA;
    EMMC_PRN(PRN_INFO,"\n Cmd%d Argument=%x  RCA=%x \n",XLLP_MMC_CMD7,argument,pSDMMCP->CardReg.RCA);
    EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD7, MM4_CMD_TYPE_NORMAL, argument, MM4_48_RES_WITH_BUSY);
    Retval |= EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x100);

    //send CMD13 to check the status of the card
    //Retval |= MM4_CheckCardStatus(pSDMMCP, 0x700);		// Make sure card is stdby mode
    Retval |= EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is in transfer mode
    i = 0;
    while(Retval != NoError)
	{
		mdelay(1);
		Retval = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
		if(i>10000)
			break;
		i++;
	}
	if (Retval != NoError)
    {
		dbg_printf(PRN_ERR,"CMD13: check status(after CMD7) Error Retval=0x%x \n",Retval);
		return SDMMCInitializationError;
    }

	// CMD 16 Set Block Length
    argument = pSDMMCP->ReadBlockSize;
    EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD16, MM4_CMD_TYPE_NORMAL, argument, MM4_48_RES);
    Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);
#ifdef MMC_QUIRKS
	if (Retval != NoError) {
		i = 4;
		/* It has been seen that SET_BLOCKLEN may fail on the first
		 * attampt, let's try a few more times
		 */
		do {
			EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD16, MM4_CMD_TYPE_NORMAL, argument, MM4_48_RES);
			Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);
			if (NoError == Retval)
				break;
		} while(i--);
	}
#endif

    // Set the block length for the controller
    pContext->pMMC4Reg->mm4_blk_cntl = argument;

	// send CMD13 to check the status of the card
	Retval |= EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK); // Make sure card is stdby mode
    if (Retval != NoError)
    {
		dbg_printf(PRN_ERR,"CMD13: check status(after changing block length) Error Retval=0x%x \n",Retval);
		return SDMMCInitializationError;
    }

	// Adjust clock speed to <= 26MHz after identification
    // For EMMC, the controll's PLL is NANDECCCLOCK clock/4. the output clock is SDIO3PLL/(2*rate)
    // if rate = 0, output clock is same with SDIO3PLL..
	EMMCHC_SetBusRate(pContext, SDCLK_SEL_DIV_64);	// 400MHz/32 = 12.5MHz

    //send CMD13 to check the status of the card
    Retval |= EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is stdby mode
    if (Retval != NoError)
    {
		dbg_printf(PRN_ERR,"CMD13: check status(after changing clock rate) Error Retval=0x%x \n",Retval);
		return SDMMCInitializationError;
    }

	// Attempt to Increase Bus width
	// 3/16:	the following code is from APSE

    Retval = EMMC_SetBusWidth(pSDMMCP, 2);
    if (Retval != NoError)
    {
		return SDMMCInitializationError;
    }

	Retval = EMMC_Switch_HS(pSDMMCP);
	if (Retval != NoError)
    {
		return SDMMCInitializationError;
    }

	EMMCHC_SetBusRate(pContext, SDCLK_SEL_DIV_4);

    //send CMD13 to check the status of the card
    Retval |= EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is stdby mode
    if (Retval != NoError)
    {
		dbg_printf(PRN_ERR,"CMD13: check status(HS after changing clock rate) Error Retval=0x%x \n",Retval);
		return SDMMCInitializationError;
    }

    // Set up State, Ready for Data transfers
    pSDMMCP->State = READY;

    return NoError;
}


volatile SDIO_ADMA_DESC_PTR pADma_Tx ;
volatile SDIO_ADMA_DESC_PTR pADma_Rx ;
extern void EMMCHC_DmaSelect(P_MM4_SDMMC_CONTEXT_T pContext, UINT_T select);

void EMMC_SetupADMA(int adma_dir, int blocks)
{
    int i;
    unsigned int buf;
    SDIO_ADMA_DESC_PTR pDma;
    P_MM4_SDMMC_T pSDREG;

    P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();

    P_MM4_SDMMC_CONTEXT_T pSDCXT = (P_MM4_SDMMC_CONTEXT_T)pSDMMCP->pContext;
    pSDREG=(P_MM4_SDMMC_T)(pSDCXT->pMMC4Reg);

    if (adma_dir==1)
    {
        pDma=pADma_Tx;
        buf=SDIO_WD_BUF;
        pSDREG->mm4_adma_system_address[0]=(uintptr_t)pADma_Tx;
    }
    else
    {
        pDma=pADma_Rx;
        buf=SDIO_RD_BUF;
        pSDREG->mm4_adma_system_address[0]=(uintptr_t)pADma_Rx;
    }
    EMMC_PRN(PRN_INFO,"\n ADMA2 setup descriptor at 0x%08x and buffer at 0x%08x. ",(unsigned int)(uintptr_t)pDma,(unsigned int)(uintptr_t)buf);
    for (i=0; i<blocks/2;i++)
    {
        memset(pDma,0,8);

        pDma->valid=1;
        pDma->end=0;
        pDma->interrupt=0;

        pDma->act1=0;  		//Tran
        pDma->act2=1;

        pDma->length=0x400;	// two block per descriptor
        pDma->addr=buf+(i*0x400);
        //pDma->addr=buf+(i*0x400)+(i*4);  // testing purpose

        //end of descriptor for even num of blk
        if (!(blocks%2)&&(i+1)>=(blocks/2))
        	pDma->end=1;      // end of descriptor

        pDma++;
    }

    if(blocks%2)//end of descriptor for odd num of blk
    {
        memset(pDma,0,8);

        pDma->valid=1;
        pDma->end=1;
        pDma->interrupt=0;

        pDma->act1=0;  		//Tran
        pDma->act2=1;

        pDma->length=0x200;	// two block per descriptor
        pDma->addr=buf+(i*0x400);//i already +1 in the last iteration
        pDma++;

    }

	//pSDREG->mm4_adma_system_address[0]=pADma_Tx;
	// DMA select
	EMMCHC_DmaSelect(pSDCXT,0x02);   // dma select=10b 32bit ADMA2

	EMMC_PRN(PRN_INFO,"\n ADMA2 setup for %d descriptor. ",i);
}

#ifndef DISABLE_ERASE_WRITE
/***********************************************************
*    EMMC_EraseBlocks()
*		Erases required number of blocks at CardAddress
*    input:
*		none
*    output:
*		Blocks erased on erase group alignment
*    returns:
*       none
************************************************************/
UINT_T EMMC_EraseBlocks(void)
{
    UINT_T  Cmd;
    UINT_T Retval = NoError;
    long long argument;
    //UINT_T flags = NO_FLAGS;
    //P_MM4_SDMMC_CONTEXT_T pContext;

    // Initialize Flash Properties
    P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();

    // Assign our context value
    //pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

    Cmd = XLLP_MMC_CMD35;

    //argument = pSDMMCP->Trans.CardAddress;
    if (pSDMMCP->CardReg.OCR & OCR_ACCESS_MODE_MASK)
    	argument = pSDMMCP->Trans.CardAddress/pSDMMCP->ReadBlockSize;//for block mode
    else
    	argument = pSDMMCP->Trans.CardAddress;
    EMMC_SendSetupCommand(pSDMMCP, Cmd, MM4_CMD_TYPE_NORMAL, (unsigned int)argument, MM4_48_RES);
    Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);
    if (Retval != NoError)
    {
		pSDMMCP->State = FAULT;
		return EraseError;
    }

    Cmd = XLLP_MMC_CMD36;

    //argument = pSDMMCP->Trans.CardAddress + pSDMMCP->Trans.TransWordSize*4;
    if (pSDMMCP->CardReg.OCR & OCR_ACCESS_MODE_MASK)
        argument = pSDMMCP->Trans.CardAddress/pSDMMCP->ReadBlockSize + pSDMMCP->Trans.NumBlocks-1;//for block mode
    else
        argument = pSDMMCP->Trans.CardAddress + (pSDMMCP->Trans.TransWordSize-1)*4;
    EMMC_SendSetupCommand(pSDMMCP, Cmd, MM4_CMD_TYPE_NORMAL, (unsigned int)argument, MM4_48_RES);
    Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);
    if (Retval != NoError)
    {
		pSDMMCP->State = FAULT;
		return EraseError;
    }

    // CMD 38
    Cmd=XLLP_SD_CMD38;
    argument = 0x00000001; // Use Trim. erase uint size is writeblock instead of erase group
    EMMC_SendSetupCommand(pSDMMCP, Cmd, MM4_CMD_TYPE_NORMAL, (unsigned int)argument, MM4_48_RES_WITH_BUSY);
    Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x8000);
    if (Retval != NoError)
    {
		Retval = EraseError;
		pSDMMCP->State = FAULT;
    }
    else
    {
		pSDMMCP->State = READY;
    }
    return Retval;
}
#endif

/****************************************************************
*   SDMMCGetCardErrorState
*
*   Input:
*		pSDMMCP - pointer to the current context
*   Output:
*      none
*   Returns:
*      Converts the error code read in from the card as R1 value in to
* 	   a WTPTP recognized return value
*****************************************************************/
UINT_T EMMC_GetCardErrorState(P_SDMMC_Properties_T pSDMMCP)
{
	EMMC_PRN(PRN_INFO, "SDIOGetCardErrorState:	pSDMMCP->CardReponse.R1_RESP = 0x%x", pSDMMCP->CardReponse.R1_RESP);

	if ((pSDMMCP->CardReponse.R1_RESP & R1_SWITCH_ERROR) == R1_SWITCH_ERROR){
		EMMC_PRN(PRN_INFO, "SDIOGetCardErrorState:	R1_SWITCH_ERROR");
		return SDMMC_SWITCH_ERROR;
	}
	if ((pSDMMCP->CardReponse.R1_RESP & R1_ERASE_RESET_ERROR) == R1_ERASE_RESET_ERROR){
		EMMC_PRN(PRN_INFO, "SDIOGetCardErrorState:	R1_ERASE_RESET_ERROR");
		return SDMMC_ERASE_RESET_ERROR ;
	}
	if ((pSDMMCP->CardReponse.R1_RESP & R1_CIDCSD_OVERWRITE_ERROR) == R1_CIDCSD_OVERWRITE_ERROR){
		EMMC_PRN(PRN_INFO, "SDIOGetCardErrorState:	R1_CIDCSD_OVERWRITE_ERROR");
		return SDMMC_CIDCSD_OVERWRITE_ERROR;
	}
	if ((pSDMMCP->CardReponse.R1_RESP & R1_OVERRUN_ERROR) == R1_OVERRUN_ERROR){
		EMMC_PRN(PRN_INFO, "SDIOGetCardErrorState:	R1_OVERRUN_ERROR");
		return SDMMC_OVERRUN_ERROR;
	}
	if ((pSDMMCP->CardReponse.R1_RESP & R1_UNDERUN_ERROR) == R1_UNDERUN_ERROR){
		EMMC_PRN(PRN_INFO, "SDIOGetCardErrorState:	R1_UNDERUN_ERROR");
		return SDMMC_UNDERUN_ERROR;
	}
	if ((pSDMMCP->CardReponse.R1_RESP & R1_CC_ERROR) == R1_CC_ERROR){
		EMMC_PRN(PRN_INFO, "SDIOGetCardErrorState:	R1_CC_ERROR");
		return SDMMC_CC_ERROR;
	}
	if ((pSDMMCP->CardReponse.R1_RESP & R1_ECC_ERROR) == R1_ECC_ERROR){
		EMMC_PRN(PRN_INFO, "SDIOGetCardErrorState:	R1_ECC_ERROR");
		return SDMMC_ECC_ERROR;
	}
	if ((pSDMMCP->CardReponse.R1_RESP & R1_ILL_CMD_ERROR) == R1_ILL_CMD_ERROR){
		EMMC_PRN(PRN_INFO, "SDIOGetCardErrorState:	R1_ILL_CMD_ERROR");
		return SDMMC_ILL_CMD_ERROR;
	}
	if ((pSDMMCP->CardReponse.R1_RESP & R1_COM_CRC_ERROR) == R1_COM_CRC_ERROR){
		EMMC_PRN(PRN_INFO, "SDIOGetCardErrorState:	R1_COM_CRC_ERROR");
		return SDMMC_COM_CRC_ERROR;
	}
	if ((pSDMMCP->CardReponse.R1_RESP & R1_LOCK_ULOCK_ERRROR) == R1_LOCK_ULOCK_ERRROR){
		EMMC_PRN(PRN_INFO, "SDIOGetCardErrorState:	R1_LOCK_ULOCK_ERRROR");
		return SDMMC_LOCK_ULOCK_ERRROR;
	}
	if ((pSDMMCP->CardReponse.R1_RESP & R1_LOCK_ERROR) == R1_LOCK_ERROR){
		EMMC_PRN(PRN_INFO, "SDIOGetCardErrorState:	R1_LOCK_ERROR");
		return SDMMC_LOCK_ERROR;
	}
	if ((pSDMMCP->CardReponse.R1_RESP & R1_WP_ERROR) == R1_WP_ERROR){
		EMMC_PRN(PRN_INFO, "SDIOGetCardErrorState:	R1_WP_ERROR");
		return SDMMC_WP_ERROR;
	}
	if ((pSDMMCP->CardReponse.R1_RESP & R1_ERASE_PARAM_ERROR) == R1_ERASE_PARAM_ERROR){
		EMMC_PRN(PRN_INFO, "SDIOGetCardErrorState:	R1_ERASE_PARAM_ERROR");
		return SDMMC_ERASE_PARAM_ERROR;
	}
	if ((pSDMMCP->CardReponse.R1_RESP & R1_ERASE_SEQ_ERROR) == R1_ERASE_SEQ_ERROR){
		EMMC_PRN(PRN_INFO, "SDIOGetCardErrorState:	R1_ERASE_SEQ_ERROR");
		return SDMMC_ERASE_SEQ_ERROR;
	}
	if ((pSDMMCP->CardReponse.R1_RESP & R1_BLK_LEN_ERROR) == R1_BLK_LEN_ERROR){
		EMMC_PRN(PRN_INFO, "SDIOGetCardErrorState:	R1_BLK_LEN_ERROR");
		return SDMMC_BLK_LEN_ERROR;
	}
	if ((pSDMMCP->CardReponse.R1_RESP & R1_ADDR_MISALIGN_ERROR) == R1_ADDR_MISALIGN_ERROR){
		EMMC_PRN(PRN_INFO, "SDIOGetCardErrorState:	R1_ADDR_MISALIGN_ERROR");
		return SDMMC_ADDR_MISALIGN_ERROR;
	}
	if ((pSDMMCP->CardReponse.R1_RESP & R1_ADDR_RANGE_ERROR) == R1_ADDR_RANGE_ERROR){
		EMMC_PRN(PRN_INFO, "SDIOGetCardErrorState:	R1_ADDR_RANGE_ERROR");
		return SDMMC_ADDR_RANGE_ERROR;
	}
	return SDMMC_GENERAL_ERROR;
}

#ifndef DISABLE_ERASE_WRITE
UINT_T EMMC_Erase (long long FlashOffset, unsigned char * LocalBuffer, long long Size)
{
	UINT_T Retval = NoError;
	P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();

	// Make sure State is correct
	if (pSDMMCP->State != READY)
		return SDMMCDeviceNotReadyError;

/*#ifndef MULTI_SDIO
	// Check if Start Address and Size are word aligned
	if ( ((Size % 4) !=0) || ((FlashOffset % 4) !=0) || ((LocalBuffer % 4) !=0))
		return SDMMC_ADDR_MISALIGN_ERROR;
#endif*/
	// Set up State
	pSDMMCP->State = READ;

	// Does the start/end addresses align on Block Boundries? Probably not, record discard bytes
	pSDMMCP->Trans.CardAddress = FlashOffset;
	pSDMMCP->Trans.StartDiscardWords = FlashOffset % pSDMMCP->ReadBlockSize;

	if (((FlashOffset + Size) % pSDMMCP->ReadBlockSize) == 0)
		pSDMMCP->Trans.EndDiscardWords = 0;
	else
		pSDMMCP->Trans.EndDiscardWords = pSDMMCP->ReadBlockSize - ((FlashOffset + Size) % pSDMMCP->ReadBlockSize);

	pSDMMCP->Trans.NumBlocks = ((long long)pSDMMCP->Trans.EndDiscardWords + (long long)pSDMMCP->Trans.StartDiscardWords + Size) / pSDMMCP->ReadBlockSize;
	pSDMMCP->Trans.TransWordSize = pSDMMCP->Trans.NumBlocks * pSDMMCP->ReadBlockSize / 4; 		// Total Transfer Size including pre and post bytes

	// Convert to # of words
	pSDMMCP->Trans.StartDiscardWords /=4;
	pSDMMCP->Trans.EndDiscardWords /= 4;

	pSDMMCP->Trans.LocalAddr = (uintptr_t)LocalBuffer;
	pSDMMCP->Trans.WordIndex = 0;									// Stores Index of Current write position

	// Kick off the Read
	//Retval = pSDMMCP->Funcs.Read_F();

	Retval = EMMC_EraseBlocks();

	if (pSDMMCP->State == FAULT)
		Retval = (EMMC_GetCardErrorState(pSDMMCP));			// TBD s/w reset?

#ifdef __SDIO_USE_INT__
#else
	pSDMMCP->State = READY;
#endif


	return Retval;
}

int do_emmcerase(unsigned long long offset, unsigned long long size)
{
	//FIXME: set LocalBuffer to NULL first
	return EMMC_Erase(offset, NULL, size);
}
#endif

/***********************************************************
*    SDMMCReadOneBlock()
*       Reads the given block off of the SD/MMC card and
*       into LocalAddr or empty buffer
*    input:
*		none
*    output:
*       LocalAddr will contain the contents of the block
*    returns:
*       none
************************************************************/
UINT_T EMMC_ReadOneBlock(void)
{
	UINT_T Retval = NoError;
	P_MM4_SDMMC_CONTEXT_T pContext;
	P_MM4_BLK_CNTL pMM4_BLK_CNTL;
	long long argument;
	P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();

 	// Assign our context value
 	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

	// Must set MMC NUMBLK
	pMM4_BLK_CNTL = (P_MM4_BLK_CNTL) &pContext->pMMC4Reg->mm4_blk_cntl;
	pMM4_BLK_CNTL->blk_cnt = pSDMMCP->Trans.NumBlocks;
	pMM4_BLK_CNTL->xfr_blksz = pSDMMCP->ReadBlockSize;

	// Set up State
	pSDMMCP->State = READ;

	// Do a CMD 17 Read Single Block
	if (pSDMMCP->SDMA_Mode)
	{
		pMM4_BLK_CNTL->dma_bufsz = MM4_512_HOST_DMA_BDRY;
	    pContext->pMMC4Reg->mm4_sysaddr = pSDMMCP->Trans.LocalAddr;

		EMMCHC_DmaSelect(pContext, 0x00);  // set DMA select SDMA
		EMMC_PRN(PRN_RES, "\n %s, SDMA Mode, ADMA index %d\n ", __func__, g_ADMA_rd);
	}

        argument = pSDMMCP->Trans.CardAddress;

	if (pSDMMCP->AccessMode == SECTOR_ACCESS)
			argument /= HARD512BLOCKLENGTH; 	// In sector mode addressing; all addresses need to be specified as block offsets.

    // only need xfer interrupt

        EMMCHC_EnableCmdInterrupt(pContext, 0);

	EMMC_PRN(PRN_DBG, "\n Send CMD17, data address argument is 0x%x\n ", argument);

	EMMC_SendDataCommand(pSDMMCP, XLLP_MMC_CMD17, (unsigned int)argument, MM4_SINGLE_BLOCK_TRAN, MM4_CARD_TO_HOST_DATA, MM4_RT_R1 | MM4_48_RES);

	//while ((pSDMMCP->State != FAULT) && (pSDMMCP->State != READY));		// TBD add timeout Let the ISR run, we'll either get a fault or finish
	//EMMC_PRN(PRN_DBG, "\npSDMMCP->State = 0x%X !\n DATATRAN is %d \n ",pSDMMCP->State,DATATRAN );

	// Get the Card Response
	Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);

	// enable cmd complete interrupt

	EMMCHC_EnableCmdInterrupt(pContext, 1);

	EMMC_PRN(PRN_DBG, "\nRetval = 0x%X !\n ",Retval );	// Get the Card Response
#if PLATFORM!=VELOCE
    if ((Retval != NoError) || ((pSDMMCP->CardReponse.R1_RESP&0xF00) != 0x900))
    {
        Retval = ReadError;
        pSDMMCP->State = FAULT;
        // Send a stop command
        EMMC_SendStopCommand(pSDMMCP);
    }
    else
    {
        pSDMMCP->State = READY;
    }
#else
    Retval |= EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
    if (Retval != NoError)
    {
        pSDMMCP->State = READY;
        return SDMMC_SWITCH_ERROR;
    }
#endif
    EMMC_PRN(PRN_DBG, "\n Send CMD17 done , read one block data done!\n ", argument);

    return Retval;
}

/***********************************************************
*    SDMMCReadBlocks()
*       Reads the given block off of the SD/MMC card and
*       into LocalAddr or empty buffer
*    input:
*		none
*    output:
*       LocalAddr will contain the contents of the block
*    returns:
*       none
************************************************************/
UINT_T EMMC_ReadBlocks(void)
{
	UINT_T Retval = NoError;
	P_MM4_SDMMC_CONTEXT_T pContext;
	P_MM4_BLK_CNTL pMM4_BLK_CNTL;
	long long argument;
	int i;
	// Initialize Flash Properties

	P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();

 	// Assign our context value
 	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

	// Must set MMC NUMBLK
	pMM4_BLK_CNTL = (P_MM4_BLK_CNTL) &pContext->pMMC4Reg->mm4_blk_cntl;
	pMM4_BLK_CNTL->blk_cnt = pSDMMCP->Trans.NumBlocks;
	pMM4_BLK_CNTL->xfr_blksz = pSDMMCP->ReadBlockSize;

	// Do a CMD 18 Read Multiple Block
	if (pSDMMCP->SDMA_Mode)
	{
		pMM4_BLK_CNTL->dma_bufsz = MM4_512_HOST_DMA_BDRY;
	    pContext->pMMC4Reg->mm4_sysaddr = pSDMMCP->Trans.LocalAddr;

		EMMCHC_DmaSelect(pContext, 0x00);  // set DMA select SDMA
		EMMC_PRN(PRN_INFO, "\n %s, SDMA Mode, ADMA index %d\n ", __func__, g_ADMA_rd);
	}

    if (g_ADMA_rd == 1)
		EMMC_SetupADMA(0, pSDMMCP->Trans.NumBlocks);    // for ADMA

	argument = pSDMMCP->Trans.CardAddress;
	if (pSDMMCP->AccessMode == SECTOR_ACCESS)
		argument /= HARD512BLOCKLENGTH; 	// In sector mode addressing; all addresses need to be specified as block offsets.

	EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD23, MM4_CMD_TYPE_NORMAL, pMM4_BLK_CNTL->blk_cnt, MM4_RT_R1 | MM4_48_RES);
	Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);

	EMMC_PRN(PRN_RES, "\n Send CMD18, MULTI_BLOCK READ, block cnt is 0x%x\n ", pSDMMCP->Trans.NumBlocks);

	//cmd_int disable, xfer_int enable for data transfer
	SDHC_EnableInterrupt(pContext, 0, 0);
	SDHC_EnableInterrupt(pContext, 1, 1);

	if(autocmd12 == 0)
		EMMC_SendDataCommandNoAuto12(pSDMMCP, XLLP_MMC_CMD18, (unsigned int)argument, MM4_MULTI_BLOCK_TRAN, MM4_CARD_TO_HOST_DATA, MM4_RT_R1|MM4_48_RES);
	else
		EMMC_SendDataCommand(pSDMMCP, XLLP_MMC_CMD18, (unsigned int)argument, MM4_MULTI_BLOCK_TRAN, MM4_CARD_TO_HOST_DATA, MM4_RT_R1|MM4_48_RES);

	// Wait for the Read to Complete
	while ((pSDMMCP->State != FAULT) && (pSDMMCP->State != READY));		// TBD add timeout Let the ISR run, we'll either get a fault or finish
	EMMC_PRN(PRN_INFO, "\npSDMMCP->State = 0x%X !\n ",pSDMMCP->State );
	// Get the Card Response
	Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);
	// enable cmd complete interrupt

#if 0//PLATFORM!=VELOCE
	if ((Retval != NoError) || ((pSDMMCP->CardReponse.R1_RESP&0xF00) != 0x900))
	{
		Retval = ReadError;
		pSDMMCP->State = FAULT;
		// Send a stop command
		EMMC_SendStopCommand(pSDMMCP);
	}
	else
	{
		pSDMMCP->State = READY;
	}
#else
    Retval |= EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
    i = 0;
    while(Retval != NoError)
    {
        mdelay(1);
        Retval = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
        if((unsigned int)i>30*pSDMMCP->Trans.NumBlocks)
            break;
        i++;
    }
    if (Retval != NoError)
    {
        pSDMMCP->State = READY;
        return SDMMC_SWITCH_ERROR;
    }
#endif
    EMMCHC_DmaSelect(pContext,0x00);  // set DMA select back to SDMA

    return Retval;
}

#ifndef DISABLE_ERASE_WRITE
/***********************************************************
*    EMMC_WriteOneBlock()
*		Writes the required number of blocks to CardAddress
*    input:
*		none
*    output:
*       Address starting with CardAddress will contain content from LocalAddress
*    returns:
*       none
************************************************************/
UINT_T EMMC_WriteOneBlock(void)
{
	UINT_T Retval = NoError;
	P_MM4_SDMMC_CONTEXT_T pContext;
	P_MM4_BLK_CNTL pMM4_BLK_CNTL;
	long long argument;
	// Initialize Flash Properties

	P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();

 	// Assign our context value
 	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

	// Must set MMC NUMBLK
	pMM4_BLK_CNTL = (P_MM4_BLK_CNTL) &pContext->pMMC4Reg->mm4_blk_cntl;
	pMM4_BLK_CNTL->blk_cnt = pSDMMCP->Trans.NumBlocks;

	// Do a CMD 24 Write single Block
	if (pSDMMCP->SDMA_Mode)
	{
            pMM4_BLK_CNTL->dma_bufsz = MM4_512_HOST_DMA_BDRY;
            pContext->pMMC4Reg->mm4_sysaddr = pSDMMCP->Trans.LocalAddr;
	}

        argument = pSDMMCP->Trans.CardAddress;

	if (pSDMMCP->AccessMode == SECTOR_ACCESS){
		argument /= HARD512BLOCKLENGTH; 	// In sector mode addressing; all addresses need to be specified as block offsets.
	}


	EMMC_PRN(PRN_DBG, "\n Send CMD24, data address argument is 0x%x\n ", argument);

    // only need xfer interrupt -KT

	EMMCHC_EnableCmdInterrupt(pContext, 0);

	EMMC_SendDataCommand(pSDMMCP, XLLP_MMC_CMD24, (unsigned int)argument, MM4_SINGLE_BLOCK_TRAN, MM4_HOST_TO_CARD_DATA, MM4_RT_R1 | MM4_48_RES);

	// Wait for the Write to Complete
	//while ((pSDMMCP->State != FAULT) && (pSDMMCP->State != READY)&&(pSDMMCP->State !=DATATRAN));		// TBD add timeout Let the ISR run, we'll either get a fault or finish
	//EMMC_PRN(PRN_DBG, "\npSDMMCP->State = 0x%X !\n ",pSDMMCP->State );	// Get the Card Response

	Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);

	// enable cmd complete interrupt

	EMMCHC_EnableCmdInterrupt(pContext, 1);
	EMMC_PRN(PRN_DBG, "\nRetval = 0x%X !\n ",Retval );	// Get the Card Response
#if PLATFORM!=VELOCE

	if ((Retval != NoError) || (pSDMMCP->State == FAULT) || ((pSDMMCP->CardReponse.R1_RESP&0xF00) != 0x900))
	{
		Retval = WriteError;
		pSDMMCP->State = FAULT;
		// Send a stop command
		EMMC_SendStopCommand(pSDMMCP);
	}
	else
	{
		pSDMMCP->State = READY;
	}
#else
        Retval |= EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
	if (Retval != NoError)
	{
	 	pSDMMCP->State = READY;
		return SDMMC_SWITCH_ERROR;
	}
#endif
	EMMC_PRN(PRN_DBG, "\n CMD24 complete, write one block data done !\n ", argument);
	return Retval;
}

/***********************************************************
*    EMMC_WriteBlocks()
*		Writes the required number of blocks to CardAddress
*    input:
*		none
*    output:
*       Address starting with CardAddress will contain content from LocalAddress
*    returns:
*       none
************************************************************/
UINT_T EMMC_WriteBlocks(void)
{
	UINT_T Retval = NoError;
	P_MM4_SDMMC_CONTEXT_T pContext;
	P_MM4_BLK_CNTL pMM4_BLK_CNTL;
	long long argument;
	int i;
	// Initialize Flash Properties

	P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();

 	// Assign our context value
 	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

	// Must set MMC NUMBLK
	pMM4_BLK_CNTL = (P_MM4_BLK_CNTL) &pContext->pMMC4Reg->mm4_blk_cntl;
	pMM4_BLK_CNTL->blk_cnt = pSDMMCP->Trans.NumBlocks;
	pMM4_BLK_CNTL->xfr_blksz = pSDMMCP->WriteBlockSize;

	// Do a CMD 25 Write Multiple Blocks
	if (pSDMMCP->SDMA_Mode)
	{
            EMMC_PRN(PRN_INFO, "\n MM4_DMA mode\n ");
            pMM4_BLK_CNTL->dma_bufsz = MM4_512_HOST_DMA_BDRY;
            pContext->pMMC4Reg->mm4_sysaddr = pSDMMCP->Trans.LocalAddr;
			EMMCHC_DmaSelect(pContext, 0x00);  // set DMA select SDMA
	}

        argument = pSDMMCP->Trans.CardAddress;

	if (pSDMMCP->AccessMode == SECTOR_ACCESS)
		argument /= HARD512BLOCKLENGTH; 	// In sector mode addressing; all addresses need to be specified as block offsets.

	EMMC_PRN(PRN_DBG, "\n EMMC_WriteBlocks, write 0x%x blocks to 0x%x\n ", pMM4_BLK_CNTL->blk_cnt, argument);

	if (g_ADMA_wr==1)
		EMMC_SetupADMA( 1,pSDMMCP->Trans.NumBlocks);    // for ADMA

    // only need xfer interrupt -KT

	//EMMCHC_EnableCmdInterrupt(pContext, 0);

	EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD23, MM4_CMD_TYPE_NORMAL, pMM4_BLK_CNTL->blk_cnt, MM4_RT_R1 | MM4_48_RES);

	Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);

	//cmd_int disable, xfer_int enable for data transfer
	SDHC_EnableInterrupt(pContext, 0, 0);
	SDHC_EnableInterrupt(pContext, 1, 1);
	//pSDMMCP->State = WRITE;
	if(autocmd12 == 0)
		EMMC_SendDataCommandNoAuto12(pSDMMCP, XLLP_MMC_CMD25, (unsigned int)argument, MM4_MULTI_BLOCK_TRAN, MM4_HOST_TO_CARD_DATA, MM4_RT_R1 | MM4_48_RES);
	else
		EMMC_SendDataCommand(pSDMMCP, XLLP_MMC_CMD25, (unsigned int)argument, MM4_MULTI_BLOCK_TRAN, MM4_HOST_TO_CARD_DATA, MM4_RT_R1 | MM4_48_RES);
	// Wait for the Write to Complete
	while ((pSDMMCP->State != FAULT) && (pSDMMCP->State != READY));		// TBD add timeout Let the ISR run, we'll either get a fault or finish
	EMMC_PRN(PRN_DBG, "\npSDMMCP->State = 0x%X !\n ",pSDMMCP->State );
	//SDIOGetCardErrorState(pSDMMCP) ;
	// Get the Card Response
	Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);

#if 0//PLATFORM!=VELOCE

	if ((Retval != NoError) || (pSDMMCP->State == FAULT) || ((pSDMMCP->CardReponse.R1_RESP&0xF00) != 0x900))
	{
		Retval = WriteError;
		pSDMMCP->State = FAULT;
		// Send a stop command
		EMMC_SendStopCommand(pSDMMCP);
	}
	else
	{
		pSDMMCP->State = READY;
	}
#else
    Retval |= EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
    i = 0;
    while(Retval != NoError)
    {
        mdelay(2);
        Retval = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
        if((unsigned int)i>50*pSDMMCP->Trans.NumBlocks)
            break;
        i++;
    }
    if (Retval != NoError)
    {
        pSDMMCP->State = READY;
        return SDMMC_SWITCH_ERROR;
    }
#endif

    EMMCHC_DmaSelect(pContext,0x00);  // set DMA select back to SDMA
    return Retval;
}
#endif

UINT_T EMMC_ReadBlk (long long  FlashOffset, unsigned char * LocalBuffer, long long  Size, unsigned char ForceMultiBlks)
{
	UINT_T Retval = NoError;
    //UINT_T flags = NO_FLAGS;
	// Initialize Flash Properties

	P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();

	// Make sure State is correct
	if (pSDMMCP->State != READY)
		return SDMMCDeviceNotReadyError;

	// Set up State
	pSDMMCP->State = READ;
	pSDMMCP->ReadBlockSize = HARD512BLOCKLENGTH;
	// Does the start/end addresses align on Block Boundries? Probably not, record discard bytes
	pSDMMCP->Trans.CardAddress = FlashOffset;
	pSDMMCP->Trans.StartDiscardWords = FlashOffset % pSDMMCP->ReadBlockSize;

	if (((FlashOffset + Size) % pSDMMCP->ReadBlockSize) == 0)
		pSDMMCP->Trans.EndDiscardWords = 0;
	else
		pSDMMCP->Trans.EndDiscardWords = pSDMMCP->ReadBlockSize - ((FlashOffset + Size) % pSDMMCP->ReadBlockSize);

	pSDMMCP->Trans.NumBlocks = ((long long)pSDMMCP->Trans.EndDiscardWords + (long long)pSDMMCP->Trans.StartDiscardWords + Size) / pSDMMCP->ReadBlockSize;
	pSDMMCP->Trans.TransWordSize = pSDMMCP->Trans.NumBlocks * pSDMMCP->ReadBlockSize / 4; 		// Total Transfer Size including pre and post bytes

	// Convert to # of words
	pSDMMCP->Trans.StartDiscardWords /=4;
	pSDMMCP->Trans.EndDiscardWords /= 4;

	pSDMMCP->Trans.LocalAddr = (uintptr_t)LocalBuffer;
	pSDMMCP->Trans.WordIndex = 0;									// Stores Index of Current write position

	// Kick off the Read
	//Retval = pSDMMCP->Funcs.Read_F();
	if(ForceMultiBlks)
			Retval = EMMC_ReadBlocks();
	else if(pSDMMCP->Trans.NumBlocks == 1)
            Retval = EMMC_ReadOneBlock();
	else // multiple blocks
            Retval = EMMC_ReadBlocks();

	if (pSDMMCP->State == FAULT)
            Retval = (EMMC_GetCardErrorState(pSDMMCP));			// TBD s/w reset?

#ifdef __SDIO_USE_INT__
#else
	pSDMMCP->State = READY;
#endif

	return Retval;
}

#ifndef DISABLE_ERASE_WRITE
/***********************************************************
*    EMMC_WriteBlk()
*    input:
*	 Description:
*		This function will write as many bytes as specified from LocalBuffer to Flash
*		Offset. The flash driver must have been previously initialized using
*		InitializeSDMMCDriver. The relavent addresses must have been previously erased.
* 		If the function encounters an error it will not
* 		re-attempt the operation.
*    Inputs:
*	   FlashOffset - The source address on the flash where data will be written to
*			specified as a byte value. Must be 32 bits aligned
*		LocalBuffer - Local address where data will be copied from.
*		Size - Specifies number of bytes to write - 32 bits aligned
*		FlashBootType - Normal or Save State flash
*    output:
*       Desired Values are written to flash
*    returns:
*       WTPTP recognized errors
************************************************************/
UINT_T EMMC_WriteBlk (long long FlashOffset, unsigned char * LocalBuffer, long long Size, unsigned char ForceMultiBlks)
{
	UINT_T Retval = NoError;
    //UINT_T flags = NO_FLAGS;
	// Initialize Flash Properties

	P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();

	// Make sure State is correct
	if (pSDMMCP->State != READY)
		return SDMMCDeviceNotReadyError;

	// Set up State
	pSDMMCP->State = WRITE;

	// Does the start/end addresses align on Block Boundries? Probably not, record discard bytes
	pSDMMCP->Trans.CardAddress = FlashOffset;
	pSDMMCP->Trans.StartDiscardWords = FlashOffset % pSDMMCP->WriteBlockSize;

	if (((FlashOffset + Size) % pSDMMCP->WriteBlockSize) == 0)
		pSDMMCP->Trans.EndDiscardWords = 0;
	else
		pSDMMCP->Trans.EndDiscardWords = pSDMMCP->WriteBlockSize - ((FlashOffset + Size) % pSDMMCP->WriteBlockSize);

	pSDMMCP->Trans.NumBlocks = ((long long)pSDMMCP->Trans.EndDiscardWords + (long long)pSDMMCP->Trans.StartDiscardWords + Size) / pSDMMCP->ReadBlockSize;
	pSDMMCP->Trans.TransWordSize = pSDMMCP->Trans.NumBlocks * pSDMMCP->WriteBlockSize / 4; 		// Total Transfer Size including pre and post, in words

	// Convert to # of words
	pSDMMCP->Trans.StartDiscardWords /= 4;
	pSDMMCP->Trans.EndDiscardWords /= 4;
	pSDMMCP->Trans.LocalAddr = (uintptr_t)LocalBuffer;
	pSDMMCP->Trans.WordIndex = 0;									// Stores Index of Current write position

	if(ForceMultiBlks)
		Retval = EMMC_WriteBlocks();
	else if(pSDMMCP->Trans.NumBlocks == 1)
		Retval = EMMC_WriteOneBlock();
	else
		Retval = EMMC_WriteBlocks();

	if (pSDMMCP->State == FAULT)
		Retval = (EMMC_GetCardErrorState(pSDMMCP));		// TBD s/w reset?
#ifdef __SDIO_USE_INT__
#else
	pSDMMCP->State = READY;
#endif

	return Retval;
}
#endif


int EMMC_READ( unsigned int start_blk, unsigned int blk, unsigned char *puchReadBuff, unsigned char ForceMultiBlks)
{
	//int rtn = 0;
	unsigned int iResult=0xFFFF;
	P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();

	while(iResult != NoError)
	{
		mdelay(1);
        	REG_READ32(((uintptr_t)pEMMCCTRL+0x24),&iResult);
			if((iResult&0x307)==0)
			{
				iResult = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
			}
	}
	iResult = EMMC_ReadBlk((long long)0x200 * start_blk, puchReadBuff, (long long)0x200 * blk, ForceMultiBlks);
	if (iResult != NoError)
	{
		EMMC_PRN(PRN_RES, "\n Read failed. \n");
		return -1;
	}
	return 0;
}

int do_emmcread(unsigned long long start, unsigned int blks, unsigned char * buffer)
{
	int ret = 0;
	uint32_t start1 = (uint32_t)(start/SDIO_BLK_SIZE);
	uint32_t blks1 = blks;
	int cur = 0, offset = 0;

	EMMC_PRN(PRN_RES, "\n read %x %x %x \n", start1, blks1, buffer);

#ifdef CONFIG_DCACHE
	if (((uintptr_t)buffer) % 64 != 0)
		dbg_printf(PRN_ERR, "%s: target buffer (0x%08x) is not 64 byte alignment!!!\n", __func__, buffer);
	invalidate_dcache_range(buffer, buffer + (SDIO_BLK_SIZE * blks));
#endif

	do {
		/*
		 * The 65535 constraint comes from some hardware has
		 * only 16 bit width block number counter
		 */
		cur = (blks1 > 65535) ? 65535 : blks1;

		ret = EMMC_READ(start1 + offset, cur, buffer + offset * SDIO_BLK_SIZE, 0x1);
		if (ret != 0)
		{
			dbg_printf(PRN_ERR,"EMMC: read emmc image fail.\n");
			return ret;
		}

		blks1 -= cur;
		offset += cur;
	} while (blks1 > 0);

#ifdef CONFIG_DCACHE
	invalidate_dcache_range(buffer, buffer + (SDIO_BLK_SIZE * blks));
#endif

	return ret;
}

int EMMC_WRITE( unsigned int start_blk, unsigned int blk, unsigned char *puchWriteBuff, unsigned char ForceMultiBlks)
{
#ifndef DISABLE_ERASE_WRITE
	//int rtn;
	unsigned int iResult=0xFFFF;
	P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();

	while(iResult != NoError)
	{
		mdelay(1);
        	REG_READ32(((uintptr_t)pEMMCCTRL)+0x24,&iResult);
		if((iResult&0x307)==0)
		{
			iResult = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
		}
	}

	iResult = EMMC_WriteBlk((long long)0x200 * start_blk, puchWriteBuff, (long long)0x200 * blk, ForceMultiBlks);

	if (iResult != NoError)
	{
		EMMC_PRN(PRN_ERR, "\n write failed. \n");
		return -1;
	}
#endif
	return 0;
}

int do_emmcwrite(unsigned long long start, unsigned int blks, unsigned char * buffer)
{
	uint32_t start1 = (uint32_t)(start/0x200);
	EMMC_PRN(PRN_ERR, "\n write %x %x %x \n", start, blks, buffer);
#ifdef CONFIG_DCACHE
	if (((uintptr_t)buffer) % 64 != 0)
		dbg_printf(PRN_ERR, "%s: target buffer (0x%08x) is not 64 byte alignment!!!\n", __func__, buffer);
	flush_dcache_range(buffer, (buffer + (blks*0x200)));
#endif
	return EMMC_WRITE(start1, blks, buffer, 0x0);
}

/***************************************************************
*   MM4_MMCReadEXTCSD
* 	Reads in 512 bytes of Extended CSD
*   Input: Pointer to 512 byte buffer
*   Output:
*   Returns: NoError, ReadError or NotSupportedError
*
*****************************************************************/
UINT_T EMMC_ReadEXTCSD (UINT_T *pBuffer)
{
	UINT_T Retval;
	P_MM4_SDMMC_CONTEXT_T pContext;
 	//P_MM4_CNTL1 pMM4_CNTL1;
 	P_MM4_BLK_CNTL pMM4_BLK_CNTL;
	int times = 10;

	// Initialize Flash Properties
	P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();

	while (times--) {
		mdelay(1);
		Retval = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);
		if (Retval == NoError)
			break;
	}

	if (times == 0) {
		dbg_printf(PRN_ERR, "ReadEXTCSD check card fail\n");
	}/**/

	// Assign our context value
	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;
	//pMM4_CNTL1 = (P_MM4_CNTL1) &pContext->pMMC4Reg->mm4_cntl1;

	// Set up State
	pSDMMCP->State = READ;
	pSDMMCP->ReadBlockSize = HARD512BLOCKLENGTH;

	// This requires a transfer over the data lines.
	pMM4_BLK_CNTL = (P_MM4_BLK_CNTL) &pContext->pMMC4Reg->mm4_blk_cntl;
	pMM4_BLK_CNTL->blk_cnt = 1;
	pMM4_BLK_CNTL->xfr_blksz = pSDMMCP->ReadBlockSize;
	if (pSDMMCP->SDMA_Mode)
	{
		pMM4_BLK_CNTL->dma_bufsz = MM4_512_HOST_DMA_BDRY;
		pContext->pMMC4Reg->mm4_sysaddr = (UINT_T)(uintptr_t)pBuffer;

		EMMCHC_DmaSelect(pContext, 0x00);  // set DMA select SDMA
		EMMC_PRN(PRN_RES, "\n %s, SDMA Mode, ADMA index %d\n ", __func__, g_ADMA_rd);
	} //MMC_SDMA_MODE
	pSDMMCP->Trans.StartDiscardWords = 0;
	pSDMMCP->Trans.EndDiscardWords = 0;								// We'll take all 512 bytes
	pSDMMCP->Trans.TransWordSize = pSDMMCP->ReadBlockSize / 4; 		// Total Transfer Size including pre and post bytes
	pSDMMCP->Trans.LocalAddr = (UINT_T)(uintptr_t)pBuffer;
	pSDMMCP->Trans.WordIndex = 0;									// Stores Index of Current write position
	pSDMMCP->Trans.NumBlocks = 1;

	//cmd_int disable, xfer_int enable for data transfer
	SDHC_EnableInterrupt(pContext, 0, 0);
	SDHC_EnableInterrupt(pContext, 1, 1);

	EMMC_SendDataCommandNoAuto12(pSDMMCP, XLLP_MMC_CMD8, NO_ARGUMENT, MM4_SINGLE_BLOCK_TRAN, MM4_CARD_TO_HOST_DATA, MM4_RT_R1 | MM4_48_RES);

	// Wait for the Read to Complete
	//while( (pSDMMCP->State != FAULT) && (pSDMMCP->State != READY) ){
	//	mdelay(2);
	//	EMMC_PRN(PRN_INFO, "EMMC_ReadEXTCSD: pSDMMCP->State: %d", pSDMMCP->State);
	//}

	// Get the Card Response
	Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);
	EMMC_PRN(PRN_INFO, "\nRetval = 0x%X !, state = %d\n RSP1 = 0x%x\n",Retval, pSDMMCP->State, pSDMMCP->CardReponse.R1_RESP);	// Get the Card Response

	// enable cmd complete interrupt
	EMMCHC_EnableCmdInterrupt(pContext, 1);

	times = 10;
	while (times--) {
		mdelay(2);
		Retval = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);
		if (Retval == NoError)
			break;
	}

	if (0 == times) {
		dbg_printf(PRN_ERR, "ReadEXTCSD 2 check card fail\n");
	}/**/

	pSDMMCP->State = READY;

	if (Retval != NoError)
		return SDMMC_SWITCH_ERROR;

	return NoError;
}

static unsigned char SDIO_RD_BUFF[512];
UINT_T EMMC_Get_Extcsd(void)
{
    UINT_T i=0,Retval=0;
    UINT_T * buffer=(UINT_T *)SDIO_RD_BUFF;
    P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();
#ifdef CONFIG_DCACHE
    invalidate_dcache_range((char*)buffer, ((char*)buffer + 0x200));
#endif

    Retval = EMMC_ReadEXTCSD(buffer);
#ifdef CONFIG_DCACHE
    invalidate_dcache_range((char*)buffer, ((char*)buffer + 0x200));
#endif
    pSDMMCP->State = READY;
    EMMC_PRN(PRN_INFO,"eMMC EXT CSD registers:\n");

    for(i=0;i<128;i++)
    {
		if(i%4==0)
			EMMC_PRN(PRN_NOTICE, "\nBytes %d ~ %d:\t",i*4,i*4+15);
		EMMC_PRN(PRN_NOTICE, "0x%x  0x%x  0x%x  0x%x  ",buffer[i]&0xFF,(buffer[i]&0xFF00)>>8,(buffer[i]&0xFF0000)>>16,(buffer[i]&0xFF000000)>>24);
    }
	g_device_sector_count=buffer[53];
	rpmbPart_sector_count = ((unsigned char *)buffer)[168];
	EMMC_PRN(PRN_INFO,"\nDevice sector count (capacity) is %d \n",g_device_sector_count);

    g_emmc_WP_info&=0xFFFF0000;
    g_emmc_WP_info|=((unsigned char*)buffer)[171];
    g_emmc_WP_info|=(((unsigned char*)buffer)[173])<<8;
    return Retval;
}

long long do_emmc_capacity(void)
{
	EMMC_Get_Extcsd();
	EMMC_PRN(PRN_INFO, "cmd_EMMC_EXTCSD capacity=0x%x Bytes\n", (g_device_sector_count * 512));
	return (g_device_sector_count * 512);
}

#if 0
int EMMC_M2M_EX(unsigned int start_blk, unsigned int cnt, unsigned char *buf_rd, unsigned char *buf_wr,unsigned int alignment,unsigned int flag)
{
    int rtn,i,j,buf_rd_base,buf_wr_base;
    unsigned int blk;

    srand(0x94725408);
	// pre-fill write pattern

   	//memXOR(buf_wr,512,1);
    buf_rd_base=buf_rd-alignment;
    buf_wr_base=buf_wr-alignment;

    for (i=cnt;i>0;i--)
    {
        blk=start_blk;
        EMMC_PRN(PRN_RES,"\n blk is 0x%x ", blk);
        buf_rd=buf_rd_base;
        buf_wr=buf_wr_base;
        memset(buf_rd,0x5A,0x200000);
        memset(buf_wr,0x5A,0x200000);
        rtn=EMMC_WriteBlk((long long)blk*0x200, buf_wr+alignment, SDIO_64K_BOUNDARY*0x20);

        EMMC_PRN(PRN_RES, "\n*\n") ;
        for(j=0;j<0x20;j++)
        {
            EMMC_PRN(PRN_RES, "@") ;
            buf_rd+=SDIO_64K_BOUNDARY;
            buf_wr+=SDIO_64K_BOUNDARY;

            if(flag==0)
            {
			if((unsigned int)buf_rd%0x80000+SDIO_64K_BOUNDARY>0x80000)
			{
				buf_rd+=SDIO_64K_BOUNDARY;
				EMMC_PRN(PRN_RES,"\n Skip boundary:Now bur_rd is 0x%x \n",buf_rd);
			}
			if((unsigned int)buf_wr%0x80000+SDIO_64K_BOUNDARY>0x80000)
			{
				buf_wr+=SDIO_64K_BOUNDARY;
				EMMC_PRN(PRN_RES,"\n Skip boundary:Now bur_wr is 0x%x \n",buf_wr);
			}
		}

            //			memXOR(buf_wr,SDIO_64K_BOUNDARY,i);
            //blk=rand()%1024+start_blk;
            blk+=SDIO_64K_BOUNDARY/0x200;
            EMMC_PRN(PRN_INFO,"\n iteration %d block %d M2M test ",i,blk);

            /*#ifndef MULTI_SDIO
            rtn=SDIOWriteOneBlk(0x200*blk,buf_wr,0x200);
            rtn=SDIOReadOneBlk(0x200*blk,buf_rd,0x200);
#else*/
            EMMC_PRN(PRN_INFO,"\n write data to block 0x%x ", blk);

            //rtn=SDIOWriteOneBlk(uchCardId, (long long)blk*0x200, buf_wr, SDIO_64K_BOUNDARY);
            rtn=EMMC_WRITE(blk,SDIO_64K_BOUNDARY/0x200,buf_wr);

            EMMC_PRN(PRN_INFO,"\n read data from block 0x%x ", blk);
            //rtn=SDIOReadOneBlk(uchCardId, (long long)blk*0x200, buf_rd, SDIO_64K_BOUNDARY);
            rtn=EMMC_READ(blk,SDIO_64K_BOUNDARY/0x200,buf_rd);
            //#endif

            //rtn = memCheck(buf_rd, buf_wr,512,1);
            //			rtn = 	memCheck(buf_rd, buf_wr,SDIO_64K_BOUNDARY,i);
            rtn = 	memcmp(buf_rd, buf_wr,SDIO_64K_BOUNDARY);
            if (rtn == 1)
            {
                EMMC_PRN(PRN_RES,"... Fail at buf_rd starting 0x%08X, buf_wr starting 0x%08X\n",buf_rd,buf_wr);
                break;
            }
            else
            {
				EMMC_PRN(PRN_INFO,"... Pass at buf_rd starting 0x%08X, buf_wr starting 0x%08X\n",buf_rd,buf_wr);
            }

	}

    }

	if(rtn == 1){
		//	dump the read and write data
		EMMC_PRN(PRN_INFO,"............. Data in Write buffer ...........\n");
		for(i=0; i<512; i++){
			EMMC_PRN(PRN_INFO, "0x%02x\t", *((unsigned char *)(buf_wr+i)) ) ;
			if(i%8 == 7)EMMC_PRN(PRN_INFO, "\n") ;
		}

		EMMC_PRN(PRN_INFO,"\n\n\n............. Data in read buffer ...........\n");
		for(i=0; i<512; i++){
			EMMC_PRN(PRN_INFO, "0x%02x\t", *((unsigned char *)(buf_rd+i)) ) ;
			if(i%8 == 7)EMMC_PRN(PRN_INFO, "\n") ;
		}
	}

	if (0 == rtn)
		EMMC_PRN(PRN_RES, "\n SDIO M2M Passed (DMA alignment buf_rd 0x%X, buf_wr 0x%X)\n",buf_rd_base,buf_wr_base);
	else
		EMMC_PRN(PRN_RES,"\n SDIO M2M Failed (DMA alignment buf_rd 0x%X, buf_wr 0x%X)\n",buf_rd_base,buf_wr_base);
        return rtn;
}
#endif


/******************************************************************************
  Description:
    Set up the registers of the controller to start the transaction to
    communicate to the card for setup related commands.
    The commands are clearly defined in the MMC specification.
  Input Parameters:
	   P_SDMMC_Properties_T pSDMMCP - Generic SD/MMC driver properties structure
	Cmd
		Command Index - See MMC or SD specification
    argument
      	the argument of  the command. MSW is for ARGH and LSW is for ARGL
	ResType
		Expected response type
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
////////////////////////////////////////////////////////////////////
//
// [127:8] --> REG[119:0]
//
////////////////////////////////////////////////////////////////////
void EMMC_Get_CID(P_SDMMC_Properties_T pSDMMCP)
{
    unsigned int   m_cmdResponse[4];
    SDIO_CID m_cid;
    m_cmdResponse[0]=pSDMMCP->CardReponse.pBuffer[0];
    m_cmdResponse[1]=pSDMMCP->CardReponse.pBuffer[1];
    m_cmdResponse[2]=pSDMMCP->CardReponse.pBuffer[2];
    m_cmdResponse[3]=pSDMMCP->CardReponse.pBuffer[3];

	////////////////////////////////////////////////////////
    // decode m_cid fields: high 8 bits ignored
    // Note: card information is not used by this driver. If
    //       OEM asks for this, add an API.
	////////////////////////////////////////////////////////

    m_cid.manufactureID = (unsigned char)(m_cmdResponse[3]>>16&0x00ff);

    m_cid.OEM[0] = (m_cmdResponse[3]>>8 &0xff);
    m_cid.OEM[1] = (m_cmdResponse[3]&0xff);
    m_cid.OEM[2] = 0;

    m_cid.name[0] = (m_cmdResponse[2]>>24 &0xff);
    m_cid.name[1] = (m_cmdResponse[2]>>16 &0xff);
    m_cid.name[2] = (m_cmdResponse[2]>>8 &0xff);
    m_cid.name[3] = (m_cmdResponse[2] &0xff);
    m_cid.name[4] = (m_cmdResponse[1]>>24 &0xff);;


    m_cid.name[5] = (m_cmdResponse[1]>>16 &0xff);;
    m_cid.name[6] =	0;
    m_cid.revision = (m_cmdResponse[1]>>8 &0xff);;
    m_cid.serialNumber =(m_cmdResponse[1] &0xFF);
    m_cid.serialNumber <<= 24;
    m_cid.serialNumber |= (m_cmdResponse[0]>>8 &0xFFFFFF);
    m_cid.manufactureDate = (m_cmdResponse[0] &0xFF);;


    EMMC_PRN(PRN_INFO,"\n CID:");
    EMMC_PRN(PRN_INFO,"\n %08x %08x %08x %08x:",m_cmdResponse[0],m_cmdResponse[1],m_cmdResponse[2],m_cmdResponse[3]);
    EMMC_PRN(PRN_INFO,"\n Manufacture ID:%d",m_cid.manufactureID);
    EMMC_PRN(PRN_INFO,"\n Product name:%s",m_cid.name);
    EMMC_PRN(PRN_INFO,"\n Serial Number:%x",m_cid.serialNumber);
    {
#ifdef DEBUG_PRN
    unsigned short month,year;
    month = m_cid.manufactureDate>>4&0xF;
    year = (m_cid.manufactureDate&0x0F)+1997;
    EMMC_PRN(PRN_INFO,"\n month=%d, year=%d",month,year);
#endif
    }

    g_manufacturerID=m_cid.manufactureID ;
    g_serialNum=(m_cid.serialNumber&0xFFFFFFFF);
}


////////////////////////////////////////////////////////////////////////
//
//  Get card CSD
////////////////////////////////////////////////////////////////////////
void EMMC_Get_CSD(P_SDMMC_Properties_T pSDMMCP)
{
	//unsigned short SDIO_Response[8];
    unsigned int   m_cmdResponse[4];
    SDIO_CSD m_csd;
    m_cmdResponse[0] = pSDMMCP->CardReponse.pBuffer[0];
    m_cmdResponse[1] = pSDMMCP->CardReponse.pBuffer[1];
    m_cmdResponse[2] = pSDMMCP->CardReponse.pBuffer[2];
    m_cmdResponse[3] = pSDMMCP->CardReponse.pBuffer[3];

	////////////////////////////////////////////////////////
    // decode m_csd data
	////////////////////////////////////////////////////////
    m_csd.structure = (m_cmdResponse[3]>> 22) & 0x03;

    m_csd.specificationVersion = (m_cmdResponse[3]>> 16) & 0x3F;

    m_csd.dataReadAccessTime1 = (m_cmdResponse[3] >> 8) & 0xFF ;
    m_csd.dataReadAccessTime2 = (m_cmdResponse[3]) & 0xFF ;

    /////////////////////////////////////////////////////////

    m_csd.maxDataTransferRate = (m_cmdResponse[2] >> 24) & 0xFF;
    m_csd.cardCommandClass =(m_cmdResponse[2] >> 12) & 0xFFF;
    //m_csd.cardCommandClass <<= 4;
    //m_csd.cardCommandClass |= ((SDIO_Response[3] >> 12) & 0x000f);

    m_csd.readBlockLength = (m_cmdResponse[2] >> 8) & 0x0F;
    m_csd.partialReadFlag = (m_cmdResponse[2] >> 7) & 0x01;;
    m_csd.writeBlockMisalign = (m_cmdResponse[2] >> 6) & 0x01;
    m_csd.readBlockMisalign = ((m_cmdResponse[2] >> 5)& 0x0001);
    m_csd.DSR_implemented = ((m_cmdResponse[2] >> 4)& 0x0001);
    // 2 bit reserved
    m_csd.deviceSize = (m_cmdResponse[2]) & 0x03;
    m_csd.deviceSize <<= 10;
    m_csd.deviceSize |= (m_cmdResponse[1] >> 22) & 0x3FF;

    ///////////////////////////////////////////////////////////
    m_csd.maxreadCurrent_VddMin = (m_cmdResponse[1] >> 19) & 0x07;
    m_csd.maxReadCurrent_VddMax = (m_cmdResponse[1] >> 16) & 0x07;
    m_csd.maxWriteCurrent_VddMin= (m_cmdResponse[1] >> 13) & 0x07;
    m_csd.maxWriteCurrent_VddMax= (m_cmdResponse[1] >> 10) & 0x07;
    m_csd.deviceSizeMultiplier = (m_cmdResponse[1] >> 7) & 0x07;
    m_csd.eraseBlockEnable =  (m_cmdResponse[1] >> 6) & 0x01;             // 1 bit
    m_csd.eraseSectorSize = (m_cmdResponse[1]) & 0x3F;;                 // 7 bits:6+1
    m_csd.eraseSectorSize <<= 6;
    m_csd.eraseSectorSize |= (m_cmdResponse[0] >> 31) & 0x01;

    /////////////////////////////////////////////////////////////
    m_csd.WP_GroupSize =   (m_cmdResponse[0] >> 24) & 0x7F;                 // 7 bits
    m_csd.WP_GroupEnable = (m_cmdResponse[0] >> 23) & 0x01;
    // 2 bit reserved
    m_csd.writeSpeedFactor = (m_cmdResponse[0] >> 18) & 0x07;              // 3 bits
    m_csd.writeBlockLength = (m_cmdResponse[0] >> 14) & 0x0F;              // 4 bits:

    m_csd.partialWriteFlag = (m_cmdResponse[0] >> 13) & 0x01;             // 1 bit
    // 5 bits reserved
    m_csd.fileFormatGroup = (m_cmdResponse[0]>> 7) & 0x01;
    m_csd.copyFlag = (m_cmdResponse[0] >> 6) & 0x01;
    m_csd.WP_perm =  (m_cmdResponse[0] >> 5) & 0x01;
    m_csd.WP_temp = (m_cmdResponse[0] >> 4) & 0x01;
    m_csd.fileFormat = (m_cmdResponse[0] >> 2) & 0x03;;
                                         // CRC is ignored
    g_emmc_WP_info&=0x0000FFFF;
    g_emmc_WP_info|=(m_csd.WP_perm<<24)|(m_csd.WP_temp<<16);

#if 1	// debug
	{
	EMMC_PRN(PRN_INFO,"\n\n CSD: %d",pSDMMCP->CardCapacity);
	EMMC_PRN(PRN_INFO,"\n %08x %08x %08x %08x:",m_cmdResponse[0],m_cmdResponse[1],m_cmdResponse[2],m_cmdResponse[3]);
	if(m_csd.structure==0)
	{
        	unsigned int mult = 1<<(m_csd.deviceSizeMultiplier+2);
        	unsigned int capacity= (m_csd.deviceSize+1) * mult * (1<<m_csd.readBlockLength);
        	EMMC_PRN(PRN_INFO,"\n Device Size: %d = c_size(%d) x mult(%d) x blk_len(%d)",capacity,(m_csd.deviceSize+1),mult,(1<<m_csd.readBlockLength));
		g_device_sector_count=capacity/512;
	}
	else
	{
		//v2.0
		m_csd.deviceSize = (m_cmdResponse[1])>>8 &0x03FFFFF;
		unsigned int capacity= (m_csd.deviceSize+1) *512;
		EMMC_PRN(PRN_INFO,"\n detected CSD v2.0 SDHC csd_structure=%x",m_csd.structure);
		EMMC_PRN(PRN_INFO,"\n Device Size: %d KB",capacity);
		//EMMC_PRN(PRN_RES,"SD High Capacity and Extended Capacity\n");;
		//EMMC_PRN(PRN_RES,"Device Size: %d KB\n",capacity);
		g_device_sector_count=capacity*2;

	}

	EMMC_PRN(PRN_INFO,"\n command class:%x",m_csd.cardCommandClass);
	EMMC_PRN(PRN_INFO,"\n Access time(TAAC):unit=%d,value=%d",m_csd.dataReadAccessTime1&0x07,(m_csd.dataReadAccessTime1&0x78)>>3);
	EMMC_PRN(PRN_INFO,"\n Access time(NSAC):%d",m_csd.dataReadAccessTime2);
	EMMC_PRN(PRN_INFO,"\n Max transfer rate:%x",m_csd.maxDataTransferRate);
//	EMMC_PRN(PRN_RES,"\n Max transfer rate:unit=%d,value=%d",m_csd.maxDataTransferRate&0x07,(m_csd.maxDataTransferRate&0x78)>>3);
	EMMC_PRN(PRN_INFO,"\n Read block len:%d",1<<m_csd.readBlockLength);
	EMMC_PRN(PRN_INFO,"\n Write block len:%d",1<<m_csd.writeBlockLength);
	EMMC_PRN(PRN_INFO,"\n Erase sector size:%d",m_csd.eraseSectorSize+1);
	EMMC_PRN(PRN_INFO,"\n File format:%d",m_csd.fileFormat);
	EMMC_PRN(PRN_INFO,"\n DSR implemented:%d",m_csd.DSR_implemented);
	}
#endif
}

/***********************************************************
*   SDMMCWriteFifo
*      Writes 2048 bytes (512 words) to the FIFO
*   Input:
*      P_SDMMC_Properties_T pSDMMCP - pointer to the SDMMC context structure
*   Output:
*      none
*   Returns:
*      none
*************************************************************/
void EMMC_WriteFifo(P_SDMMC_Properties_T pSDMMCP)
{
    int i, t = 0;
    UINT_T Buffer =0x0;
    P_MM4_SDMMC_CONTEXT_T pContext;
    volatile UINT_T *pMMC_TX_Fifo;

    // Assign our context value
    pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

    pMMC_TX_Fifo = (volatile UINT_T *)&(pContext->pMMC4Reg->mm4_dp);
    t = pSDMMCP->Trans.WordIndex;

    // Ignore Pre Bytes
    for (i=0; (i < MM4FIFOWORDSIZE) && ((unsigned int)t < pSDMMCP->Trans.StartDiscardWords); i++, t++)
        *pMMC_TX_Fifo = Buffer;

    // Write Requested Data
    for (; ((i < MM4FIFOWORDSIZE) && ((unsigned int)t < (pSDMMCP->Trans.TransWordSize-pSDMMCP->Trans.EndDiscardWords))); i++, t++)
        *pMMC_TX_Fifo = ((UINT_T*)(uintptr_t)(pSDMMCP->Trans.LocalAddr))[t];

    // Ignore Trailing Bytes
    for (; (i < MM4FIFOWORDSIZE) && ((unsigned int)t < pSDMMCP->Trans.TransWordSize); i++, t++)
        *pMMC_TX_Fifo = Buffer;

    pSDMMCP->Trans.WordIndex = t;
}


/***********************************************************
*   EMMC_ReadFifo
*      Reads the contents of the read fifo (512 words)
*   Input:
*      P_SDMMC_Properties_T pSDMMCP - pointer to the SDMMC context structure
*   Output:
*      buffer will contain the contents of the read fifo
*   Returns:
*      none
*************************************************************/
void EMMC_ReadFifo(P_SDMMC_Properties_T pSDMMCP)
{
    volatile int i, t = 0;
    //UINT_T Buffer =0x0;
    P_MM4_SDMMC_CONTEXT_T pContext;
    volatile UINT_T *pMMC_RX_Fifo;

    	// Assign our context value
    	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

    pMMC_RX_Fifo = (volatile UINT_T *)&(pContext->pMMC4Reg->mm4_dp);
    t = pSDMMCP->Trans.WordIndex;

    // Ignore Pre Bytes
    for (i=0; (i < MM4FIFOWORDSIZE) && ((unsigned int)t < pSDMMCP->Trans.StartDiscardWords); i++, t++){
        //Buffer = *pMMC_RX_Fifo;
    }

    // Read Requested Data
    for (; ((i < MM4FIFOWORDSIZE) && ((unsigned int)t < (pSDMMCP->Trans.TransWordSize-pSDMMCP->Trans.EndDiscardWords))); i++, t++){
        ((UINT_T*)(uintptr_t) (pSDMMCP->Trans.LocalAddr))[t] = (uint32_t)(uintptr_t)(*pMMC_RX_Fifo);
    }

	// Ignore Trailing Bytes
    for (; (i < MM4FIFOWORDSIZE) && ((unsigned int)t < pSDMMCP->Trans.TransWordSize); i++, t++){
        //Buffer = *pMMC_RX_Fifo;
    }

    pSDMMCP->Trans.WordIndex = t;
}

extern void EMMCHC_CMDSWReset(P_MM4_SDMMC_CONTEXT_T pContext);

/****************************************************************
*   EMMC_ISR
*      	Interrupt Service Routine for SDMMC controller
*		Controls flow and catches faults asynchronously
*   Input:
*	   P_SDMMC_Properties_T pSDMMCP
*   Output:
*      none
*   Returns:
*      none
*****************************************************************/
void EMMC_ISR()
{
    UINT_T i;
    VUINT_T *pControllerBuffer;
    UINT_T i_stat_copy;
    UINT_T i_err_stat;		// Keep a copy of i stat register
    UINT_T i_acmd12_err_stat;
    P_MM4_I_STAT  p_i_stat_copy;			// Pointer to the copy.
    P_MM4_I_STAT_UNION pMM4_I_STAT_U;
    //volatile int state ;
    int complete;
    P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();
    P_MM4_SDMMC_CONTEXT_T pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;		// Assign our context value
    //P_MM4_CMD_XFRMD_UNION pCmdXfer= (	P_MM4_CMD_XFRMD_UNION)&pContext->pMMC4Reg->mm4_cmd_xfrmd;
    //P_MM4_STATE_UNION pState = (P_MM4_STATE_UNION)&pContext->pMMC4Reg->mm4_state;
    p_i_stat_copy = (P_MM4_I_STAT)&i_stat_copy;
    complete =  pSDMMCP->CardReponse.CommandComplete;

    // Save off the interrupt source to the copy
    pMM4_I_STAT_U = (P_MM4_I_STAT_UNION) &pContext->pMMC4Reg->mm4_i_stat;
    i_stat_copy = pMM4_I_STAT_U->mm4_i_stat_value;

    EMMC_PRN(PRN_INFO,"\n enter EMMC_ISR i_stat_copy = 0x%x \n", i_stat_copy);

    if(pMM4_I_STAT_U->mm4_i_stat_bits.cdint)
    {
		EMMC_PRN(PRN_RES, "\n Got card interrupt from DAT1 !\n ");
    	EMMCHC_cdInt_enable(pContext, DISABLE_INTS);
    }

#ifdef __SDIO_INT_MODE__
    if (i_stat_copy & 0xc0)    // card detect interrupt
    {
		EMMC_PRN(PRN_RES,"\n card remove/insert int--> i_stat=0x%08x\n",i_stat_copy);
    	pMM4_I_STAT_U->mm4_i_stat_value = CLEAR_INTS_MASK;
    	i = pMM4_I_STAT_U->mm4_i_stat_value;		// Must make a dummy read to allow interrupts to clear.

    	return;
    }
#endif

    // Check for any error
    if (p_i_stat_copy->errint)
    {
        i_err_stat = i_stat_copy >> 16 ;
        // TBD pSDMMCP->CardReponse.CommandError = 1;
        // Reset MM4CMD pin in case it's hung waiting on a response
        //pContext->pMMC4Reg->mm4_cntl2 |= 0x02000000;
        EMMCHC_CMDSWReset(pContext);	// this cleas the command inhibit flag in sd_present_state_1.
        EMMCHC_DataSWReset(pContext);	// this clears the data inhibit flag and stops mclk

        if ((pSDMMCP->State == WRITE) || (pSDMMCP->State == READ) || (pSDMMCP->State == DATATRAN))
        {
            EMMC_PRN(PRN_ERR,"\n errint:i_stat=0x%08x\n",i_stat_copy);
			pSDMMCP->State = FAULT;
            pContext->pMMC4Reg->mm4_i_stat &= 0x0000FFFF;	//Satya
        }

        if( i_err_stat & SD_ERROR_INT_STATUS_AUTO_CMD12_ERR )
        {
            i_acmd12_err_stat = pContext->pMMC4Reg->mm4_acmd12_er;
            EMMC_PRN(PRN_ERR,"\n acmd12 errin stat= 0x%08x \n",pContext->pMMC4Reg->mm4_acmd12_er);
            // clear the acmd12 error bits.
            pContext->pMMC4Reg->mm4_acmd12_er = i_acmd12_err_stat;
        }
    }

    // is this an sdma interrupt event?
    if(pContext->pMMC4Reg->mm4_i_stat & (1u<<3))	// bit 3 is dmaint
    {
        // the transfer halted because the boundary specified in ... was reached.
        // rewriting the sysaddr with the next address allows the transfer to resume.
        // fortunately the sysaddr register itself contains the next address.
        // so, just re-write the sysaddr register with its own current contents.
        pContext->pMMC4Reg->mm4_sysaddr = pContext->pMMC4Reg->mm4_sysaddr;	// sysaddr points to next addr to write.
    }

	for (i = 0; i < 4; i++)
 		pSDMMCP->CardReponse.pBuffer[i] = 0;

	// Has the Command completed? If so read the response register
	while(p_i_stat_copy->cmdcomp != 1)
	{
		i_stat_copy = pContext->pMMC4Reg->mm4_i_stat;
		EMMC_PRN(PRN_INFO,"\n i_stat_copy = 0x%x \n", i_stat_copy);
		i++;
		if(i>6)
			break;
	}

	if (p_i_stat_copy->cmdcomp)
	{
 		// Indicate that the response has been read
		// TBD pSDMMCP->CardReponse.CommandError = 0;
		complete = 1;
		pControllerBuffer = (VUINT_T *) &pContext->pMMC4Reg->mm4_resp0;
 		for (i = 0; i < 4; i++)
 			pSDMMCP->CardReponse.pBuffer[i] = pControllerBuffer[i];
	}
	//EMMC_PRN(PRN_RES," ISR: i_stat = 0x%08x \n",i_stat_copy);

	// Are we SDMA mode enabled?
	if (pSDMMCP->SDMA_Mode)
	{
		if (p_i_stat_copy->xfrcomp)
		{
			pSDMMCP->State = READY;
			complete = 1;
			// get response if cmd complete interrupt is masked
			pControllerBuffer = (VUINT_T *) &pContext->pMMC4Reg->mm4_resp0;
 			for (i = 0; i < 4; i++)
 				pSDMMCP->CardReponse.pBuffer[i] = pControllerBuffer[i];
		}
 	}

	// Clear the interrupts
	pMM4_I_STAT_U->mm4_i_stat_value = i_stat_copy; 	// Clear the interrupt source.
	i = pMM4_I_STAT_U->mm4_i_stat_value;			// Must make a dummy read to allow interrupts to clear.


	// Handle State based interrupts XFRCOMP, BUFRDRDY, BUFWRRDY

	if (pSDMMCP->SDMA_Mode == FALSE)
	{
		// Handle State based interrupts XFRCOMP, BUFRDRDY, BUFWRRDY
		switch (pSDMMCP->State)
		{
			case WRITE:
			{
				if (p_i_stat_copy->bufwrrdy){
					EMMC_PRN(PRN_RES,"MM4 ISR: write fifo, blk_cntl = 0x%x\n", pContext->pMMC4Reg->mm4_blk_cntl);
					EMMC_WriteFifo(pSDMMCP);
				}

				// Are we done sending all of data?
				if (pSDMMCP->Trans.TransWordSize == pSDMMCP->Trans.WordIndex)
					pSDMMCP->State = DATATRAN;
				break;
			}
			case READ:
			{
				if (p_i_stat_copy->bufrdrdy){
					EMMC_PRN(PRN_RES,"MM4 ISR: read fifo, blk_cntl = 0x%x\n", pContext->pMMC4Reg->mm4_blk_cntl);
					EMMC_ReadFifo(pSDMMCP);
				}

				// Are we done sending all of data?
				if (pSDMMCP->Trans.TransWordSize == pSDMMCP->Trans.WordIndex)
					pSDMMCP->State = DATATRAN;
				break;
			}
			case DATATRAN:
			{
				// Wait for Transfer Complete Signal
				EMMC_PRN(PRN_RES,"MM4 ISR: transfer complete, blk_cntl = 0x%x\n", pContext->pMMC4Reg->mm4_blk_cntl);
				if (p_i_stat_copy->xfrcomp)
				{
					pSDMMCP->State = READY;
					complete = 1;
					// get response if cmd complete interrupt is masked
					pControllerBuffer = (VUINT_T *) &pContext->pMMC4Reg->mm4_resp0;
 					for (i = 0; i < 4; i++)
 						pSDMMCP->CardReponse.pBuffer[i] = pControllerBuffer[i];
				}
				break;
			}
			default:
				break;
		}
	}

    pSDMMCP->CardReponse.CommandComplete = complete;
    EMMC_PRN(PRN_DBG,"MM4 ISR end, mm4_i_stat_value = 0x%x  \n",pMM4_I_STAT_U->mm4_i_stat_value);
    return;
}


/***************************************************************
*   MM4SwitchPartitionForAlternateBootMode
*		eMMC Alternate Boot Mode Setup
*   Input:
*		PartitionNumber - Contains the partition Number to switch to and enable bits for the boot partitions.
*   Output:
*   Returns: NoError, ReadError or NotSupportedError
*
*****************************************************************/
UINT_T EMMCSwitchPartitionForAlternateBootMode(int partition)
{
	 UINT_T Retval = NoError;
	 //P_MM4_SDMMC_CONTEXT_T pContext;
	 MMC_CMD6_OVERLAY Cmd6;
	 volatile int delay,ii;
	 int i = 0;

	 UNUSED(partition);
	 EMMC_PRN(PRN_DBG,"MM4SwitchPartition: switch to part %d\n:", partition);

	 // Initialize Flash Properties
	 P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();

	 Retval = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);

	 if (Retval != NoError) {
		EMMC_PRN(PRN_RES,"card status  error %d\n", Retval);
	        return SDMMCInitializationError;
	} else {
		EMMC_PRN(PRN_RES,"card status no error\n");
	}

	 // Assign our context value
	 //pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

	 // Issue CMD 6 write byte 179 (0xB3)
	 Cmd6.MMC_CMD6_Layout.Access = EXT_CSD_ACCESS_WRITE_BYTE;				// Write byte
	 Cmd6.MMC_CMD6_Layout.CmdSet = 0;                		  				// Don't Care
	 Cmd6.MMC_CMD6_Layout.Index = PARTITION_CONFIG_MMC_EXT_CSD_OFFSET;       // Choose Boot Config
	 Cmd6.MMC_CMD6_Layout.Reserved0 = 0;
	 Cmd6.MMC_CMD6_Layout.Reserved1 = 0;
	 Cmd6.MMC_CMD6_Layout.Value = 0x49;	 // 0x49			 		// Boot from Partition 1, no send boot acknowledge
	 EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD6, MM4_CMD_TYPE_NORMAL, Cmd6.MMC_CMD6_Bits, MM4_48_RES_WITH_BUSY);
	 Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x100);

	do {
		for(delay=0; delay<=0x10000; delay++)
			ii++;
		Retval = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);
		if (Retval == NoError)
			break;
		i++;
	} while(i < 10);


	 // Issue CMD 6 to write byte 177 (0xB1)
	 Cmd6.MMC_CMD6_Layout.Access = EXT_CSD_ACCESS_WRITE_BYTE;				// Clear bits
	 Cmd6.MMC_CMD6_Layout.CmdSet = 0;                		  				// Don't Care
	 Cmd6.MMC_CMD6_Layout.Index = BOOT_BUS_WIDTH_MMC_EXT_CSD_OFFSET;       // Choose Boot Config
	 Cmd6.MMC_CMD6_Layout.Reserved0 = 0;
	 Cmd6.MMC_CMD6_Layout.Reserved1 = 0;
	 //	Set to 8 bit mode
	 Cmd6.MMC_CMD6_Layout.Value = 0xA ; //0x2; //0xA; //  Use 2 for Samsung 4.3+ parts, they don't allow setting fast timing bits 0x2;

	 do {
	 	i++;
	 	EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD6, MM4_CMD_TYPE_NORMAL, Cmd6.MMC_CMD6_Bits, MM4_48_RES_WITH_BUSY);
		 Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x100);
		 if (Retval != NoError)
		 	continue;
		 for(delay=0; delay<=0x10000; delay++)
			 ii++;
		 Retval = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);
		 if (Retval == NoError)
			break;
		 else {
			EMMC_PRN(PRN_RES,"card status end error %d\n", Retval);
		 }
	} while(i <= 10);

	//MM4_CheckCardStatus(pSDMMCP, 0x0900, R1_LOCKEDCARDMASK);

#if 0
	 Retval |= MM4_CheckCardStatus(pSDMMCP, 0x0900, R1_LOCKEDCARDMASK);
	 //Not in the ready state?
	 if(Retval != NoError)
	 {
	    pContext->pMMC4Reg->mm4_cntl2 |= (1 << 25); // set reset bit
	    pSDMMCP->State = READY;           // Ignore the error if the card doesn't support it
	    return SDMMC_SWITCH_ERROR;
	 }
#endif
 	return Retval;
}

UINT_T EMMCSwitchPartition(int PartitionNumber)
{
    UINT_T Retval = NoError;
    //P_MM4_SDMMC_CONTEXT_T pContext;
    MMC_CMD6_OVERLAY Cmd6;
    // Initialize Flash Properties
    P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();

    static UINT_T power_on=0;

	int i;
    volatile int delay,ii;

    switch(PartitionNumber)
    {
    	case 0:
			EMMC_PRN(PRN_RES,"Default access to user data area\n");
    	break;
    	case 1:
			EMMC_PRN(PRN_RES,"Switch access to boot partition 1\n");
    	break;
    	case 2:
			EMMC_PRN(PRN_RES,"Switch access to boot partition 2\n");
		break;
		case 3:
			EMMC_PRN(PRN_RES,"Switch access to partition 3\n");
		break;
		default:
			EMMC_PRN(PRN_RES,"Other parition access funtion not implement yet.Please use 0~3\n");
		return 0;
    }

     // Assign our context value
     //pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

     //send CMD13 to check the status of the card
    Retval = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
    if (Retval != NoError)
    	return SDMMCInitializationError;
     // Must set partition
     if (pSDMMCP->SD == XLLP_MMC)
     {
        if(power_on==0)
        {
        // Issue CMD 6 to clear PARTITION_ACCESS bits in EXT_CSD register byte 179
            Cmd6.MMC_CMD6_Layout.Access = EXT_CSD_ACCESS_SET_BITS;				// Clear bits
            Cmd6.MMC_CMD6_Layout.CmdSet = 0;                		  				// Don't Care
            Cmd6.MMC_CMD6_Layout.Index = ERASE_GROUP_DEF;       // Choose Boot Config
            Cmd6.MMC_CMD6_Layout.Reserved0 = 0;
            Cmd6.MMC_CMD6_Layout.Reserved1 = 0;
            Cmd6.MMC_CMD6_Layout.Value = 1;				 		// Clear out Partition Access bits

            EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD6, MM4_CMD_TYPE_NORMAL, Cmd6.MMC_CMD6_Bits, MM4_RT_R1 | MM4_RT_BUSY | MM4_48_RES_WITH_BUSY);
            Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x10);


			do {
				mdelay(10);
				Retval = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);
				EMMC_PRN(PRN_RES,"Retval = %d\n", Retval);
			} while(Retval != NoError);
        	power_on=1;
        }

	    // Issue CMD 6 to clear PARTITION_ACCESS bits in EXT_CSD register byte 179
            Cmd6.MMC_CMD6_Layout.Access = EXT_CSD_ACCESS_CLEAR_BITS;				// Clear bits
            Cmd6.MMC_CMD6_Layout.CmdSet = 0;                		  				// Don't Care
            Cmd6.MMC_CMD6_Layout.Index = PARTITION_CONFIG_MMC_EXT_CSD_OFFSET;       // Choose Boot Config
            Cmd6.MMC_CMD6_Layout.Reserved0 = 0;
            Cmd6.MMC_CMD6_Layout.Reserved1 = 0;
            Cmd6.MMC_CMD6_Layout.Value = PARTITION_ACCESS_BITS;				 		// Clear out Partition Access bits


			for (i = 0; i < 10; i++) {
				EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD6, MM4_CMD_TYPE_NORMAL, Cmd6.MMC_CMD6_Bits, MM4_RT_R1 | MM4_RT_BUSY | MM4_48_RES_WITH_BUSY);
            	Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x10);
				if (Retval != NoError) {
					for(delay=0; delay<=0x10000; delay++)
						ii++;
				} else {
					for(delay=0; delay<=0x1000; delay++)
						ii++;
                    break;
				}
			}

			do {
				mdelay(10);
				Retval = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);
				EMMC_PRN(PRN_RES,"Retval = %d\n", Retval);
			} while(Retval != NoError);
			 if (Retval != NoError)
			 {
			 	pSDMMCP->State = READY;
			 	return SDMMC_SWITCH_ERROR;
			 }
		// Now issue CMD 6 again to set the right bits.
            Cmd6.MMC_CMD6_Layout.Access = EXT_CSD_ACCESS_SET_BITS;				// Clear bits
            Cmd6.MMC_CMD6_Layout.CmdSet = 0;                		  			// Don't Care
            Cmd6.MMC_CMD6_Layout.Index = PARTITION_CONFIG_MMC_EXT_CSD_OFFSET;   // Choose Boot Config
            Cmd6.MMC_CMD6_Layout.Reserved0 = 0;
            Cmd6.MMC_CMD6_Layout.Reserved1 = 0;
            Cmd6.MMC_CMD6_Layout.Value = PartitionNumber;			 			// Set the correct partition

			for (i = 0; i < 10; i++) {
				EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD6, MM4_CMD_TYPE_NORMAL, Cmd6.MMC_CMD6_Bits, MM4_RT_R1 | MM4_RT_BUSY | MM4_48_RES_WITH_BUSY);
            	Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x10);
				if (Retval != NoError) {
					for(delay=0; delay<=0x10000; delay++)
						ii++;
				} else {
					for(delay=0; delay<=0x1000; delay++)
						ii++;
                    break;
				}
			}

    }


	 do {
		mdelay(10);
		Retval = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);
		EMMC_PRN(PRN_RES,"Retval = %d\n", Retval);
	} while(Retval != NoError);
	if (Retval != NoError)
	{
		pSDMMCP->State = READY;
		return SDMMC_SWITCH_ERROR;
	}
	EMMC_PRN(PRN_RES,"#2\n");
	return NoError;
}

int do_emmc_switch_part(UINT32 PartitionNumber)
{
	return EMMCSwitchPartition(PartitionNumber);
}

UINT_T EMMC_SendTuningFuncCmd(void)
{
	uintptr_t addr;
    UINT_T  i , timout = 0;
    UINT_T Retval = TimeOutError;

    UINT_T TuningPat[16];

    addr = (uintptr_t)pEMMCCTRL;

    //change block size
    REG_WRITE32(addr + 0x4,0x80);

    //disable signal of buffer ready interrupt
    REG_READ32(addr + 0x38 , &i);
    i &= ~0x20;
    REG_WRITE32(addr + 0x38 , i);


	// issue CMD21 tuning command
    REG_WRITE32(addr + 0xC , 0x153A0010);

  	// wait buffer ready interrupt
    while(timout++ != 0x200000)
    {
        REG_READ32(addr + 0x30 , &i);

        if((i&0x20) == 0x20)
        {
        	REG_WRITE32(addr + 0x30 , i);
        	Retval = NoError;
        	break;
        }
    }
    if(timout >= 0x200000)
    {
        EMMC_PRN(PRN_INFO, "time out ! 0x%x = 0x %x   Retval = 0x%x\n", addr + 0x30, i, Retval);
        return Retval;
    }
	//enable signal of buffer ready interrupt
    REG_READ32(addr + 0x38 , &i);
//    i |= 0x20;
    REG_WRITE32(addr + 0x38 , i);


    for(i=1;i<17;i++)
    {
        REG_READ32(addr+0x20,&(TuningPat[i-1]));
        EMMC_PRN(PRN_INFO, "0x%x ", TuningPat[i-1]);
        if(i%4==0)
            EMMC_PRN(PRN_INFO, "\n");
    }

	// Set the block length for the controller
    REG_WRITE32(addr + 0x4,0x200);

    return Retval;
}


int eMMC_Sleep()
{
    unsigned int argument;
    P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();
    int Retval;

    // send CMD7 to get card into standby state
    argument = 0; // de-select
    EMMC_PRN(PRN_INFO,"\n Cmd%d Argument=%x  RCA=%x \n",XLLP_MMC_CMD7,argument,pSDMMCP->CardReg.RCA);
    EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD7, MM4_CMD_TYPE_NORMAL, argument, MM4_48_RES_WITH_BUSY); // no response

    mdelay(1); // delay to make sure CMD7 complete

    //send CMD13 to check the status of the card
    Retval = EMMC_CheckCardStatus(pSDMMCP, 0x700, R1_LOCKEDCARDMASK);		// Make sure card is stdby mode
    if (Retval != NoError)
    {
        EMMC_PRN(PRN_RES,"CMD13: check status Error Retval=0x%x \n",Retval);
        return SDMMCDeviceNotReadyError;
    }

    // send CMD5 to enter sleep
    argument = pSDMMCP->CardReg.RCA |(1<<15);
    EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD5, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R1 | MM4_RT_BUSY | MM4_48_RES_WITH_BUSY);
    Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x10);
    if (Retval != NoError)
    {
        EMMC_PRN(PRN_RES,"CMD5: sleep fail, Retval=0x%x \n",Retval);
        return SDMMCDeviceNotReadyError;
    }

    EMMC_PRN(PRN_RES,"EMMC is in sleep now, reacts only to the commands RESET and SLEEP/AWAKE\n");
    return 0;
}

int eMMC_Wakeup()
{
    unsigned int argument;
    P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();
    int Retval;
    int timeout;

    // send CMD5 to wakeup
    argument = pSDMMCP->CardReg.RCA | (0<<15);
    EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD5, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R1 | MM4_48_RES_WITH_BUSY);
    Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x10);
    if (Retval != NoError)
    {
        EMMC_PRN(PRN_RES,"CMD5: wakeup fail, Retval=0x%x \n",Retval);
        return SDMMCDeviceNotReadyError;
    }

    // wait DAT0 to 1
    timeout = 10000;
    while(timeout-- > 0)
    {
        unsigned int state;
        REG_READ32(((uintptr_t)pEMMCCTRL)+0x24,&state);
        if (state & (1<<20))
            break;
    }
    if (timeout <= 0)
    {
        EMMC_PRN(PRN_RES,"Wait CMD5 complete timeout\n", Retval);
        return SDMMCDeviceNotReadyError;
    }

    //send CMD13 to check the status of the card
    Retval = EMMC_CheckCardStatus(pSDMMCP, 0x700, R1_LOCKEDCARDMASK);        // Make sure card is stdby mode
    if (Retval != NoError)
    {
        EMMC_PRN(PRN_RES,"CMD13: check status Error Retval=0x%x \n",Retval);
        return SDMMCDeviceNotReadyError;
    }

    //send CMD7 to get card into transfer state
    argument = pSDMMCP->CardReg.RCA;
    EMMC_PRN(PRN_INFO,"\n Cmd%d Argument=%x  RCA=%x \n",XLLP_MMC_CMD7,argument,pSDMMCP->CardReg.RCA);
    EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD7, MM4_CMD_TYPE_NORMAL, argument, MM4_48_RES_WITH_BUSY);
    Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x100);
    if (Retval != NoError)
    {
        EMMC_PRN(PRN_RES,"CMD7: check status Error Retval=0x%x \n",Retval);
        return SDMMCDeviceNotReadyError;
    }

    //send CMD13 to check the status of the card
    Retval = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);        // Make sure card is trans mode
    if (Retval != NoError)
    {
        EMMC_PRN(PRN_RES,"CMD13: check status Error Retval=0x%x \n",Retval);
        return SDMMCDeviceNotReadyError;
    }

    EMMC_PRN(PRN_RES,"EMMC is awake\n");
    return 0;
}


int eMMC_Sleep_check()
{
    unsigned int argument;
    P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();
    int Retval;

   	//send CMD13 to check the status of the card
   	argument = pSDMMCP->CardReg.RCA;
    EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD13, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R1 | MM4_48_RES);
    Retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);
    if (Retval != NoError)
    {
        EMMC_PRN(PRN_RES,"CMD13 has no response, maybe eMMC is in sleep\n");
        return 0;
    }
    EMMC_PRN(PRN_RES,"eMMC is awake, CMD13 R1=0x%08x\n", pSDMMCP->CardReponse.R1_RESP);

    return 0;
}

UINT_T cmd_EMMC_EXTCSD()
{
	return EMMC_Get_Extcsd();
}

int cmd_EMMC_INIT(void)
{
	int iResult = 0;

	EMMC_SetupProp();	 // setup SW structure
	EMMC_SetupPinmux();	 // setup PinMux
	iResult = EMMCHC_Init();	   // Host controller initialization
	emmc_Intr_On() ;

	EMMC_PRN(PRN_RES, " SDIO init\n");
	return iResult;
}

/*************************
set emmc mode, mode0: high speed SDR, mode1:high speed DDR, mode2:HS200, mode3:HS400
*************************/
UINT_T EMMC_Switch_HS(P_SDMMC_Properties_T pSDMMCP)
{
	UINT_T Retval = NoError;
	volatile int i = 0;
    MMC_CMD6_OVERLAY Cmd6;
    P_MM4_SDMMC_CONTEXT_T pContext;

    pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

	// Issue CMD 6 to set HS TIMING bits in EXT_CSD register byte 185
    Cmd6.MMC_CMD6_Layout.Access = EXT_CSD_ACCESS_WRITE_BYTE;			//Write Byte
    Cmd6.MMC_CMD6_Layout.CmdSet = 0;									// Don't Care
    Cmd6.MMC_CMD6_Layout.Index = HS_TIMING_MMC_EXT_CSD_OFFSET; 			// Choose HS_TIMING
    Cmd6.MMC_CMD6_Layout.Reserved0 = 0;
    Cmd6.MMC_CMD6_Layout.Reserved1 = 0;
    //Cmd6.MMC_CMD6_Layout.Value = (UINT8_T) Mode;	// SET HS_TIMING.
    Cmd6.MMC_CMD6_Layout.Value = (UINT8_T) 1;	// SET HS_TIMING.
    pSDMMCP->State = WRITE;

    EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD6, MM4_CMD_TYPE_NORMAL, Cmd6.MMC_CMD6_Bits, MM4_RT_R1 | MM4_RT_BUSY | MM4_48_RES_WITH_BUSY);
    Retval += EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x10);

    mdelay(1);

    i = 0;
    Retval = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
    while(Retval != NoError)
    {
	    mdelay(1);
	    Retval = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
	    if(i>10000)
		    break;
	    i++;
    }
    if (Retval != NoError)
    {
	    EMMC_PRN(PRN_ERR,"CMD13 fail\n");
	    EMMC_PRN(PRN_ERR,"SWITCH MODE fail\n");
	    return SDMMCInitializationError;
    }

    EMMCHC_HighSpeedSelect(pContext, 1);   //high speed

    EMMC_PRN(PRN_RES,"SWITCH HS MODE pass\n");

    return Retval;
}

int cmd_EMMC_RESET(void)
{
	unsigned int val;
	unsigned int offset;

	/* clock enable for modules using perifSysClk as clock source */
	BFM_HOST_Bus_Read32(MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_clkEnable, &val);
	val |= (0x1 << LSb32Gbl_clkEnable_emmcSysClkEn);
	BFM_HOST_Bus_Write32(MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_clkEnable, val);

	// EMMC reset host
	BFM_HOST_Bus_Read32(MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_perifReset, &val);
	val &= ~MSK32Gbl_perifReset_emmcSyncReset;
	val |= 0x1<<LSb32Gbl_perifReset_emmcSyncReset;
	BFM_HOST_Bus_Write32(MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_perifReset, val);
	udelay(1);
	val &= ~MSK32Gbl_perifReset_emmcSyncReset;
	BFM_HOST_Bus_Write32(MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_perifReset, val);

	// EMMC reset device
	BFM_HOST_Bus_Read32((EMMC_REG_BASE + 0xE8), &offset);
	offset &= 0xFFF;
	offset += 0x2C;

	BFM_HOST_Bus_Read32((EMMC_REG_BASE + offset), &val);
	val |= 0x1;
	BFM_HOST_Bus_Write32((EMMC_REG_BASE + offset), val);
	udelay(1);

	BFM_HOST_Bus_Read32((EMMC_REG_BASE + offset), &val);
	val |= 0x1 << 3;
	val &= ~(0x1 << 2);
	BFM_HOST_Bus_Write32((EMMC_REG_BASE + offset), val);
	sd_delay(1);	// mdelay(1);

	BFM_HOST_Bus_Read32((EMMC_REG_BASE + offset), &val);
	val |= (0x1<<2);
	BFM_HOST_Bus_Write32((EMMC_REG_BASE + offset), val);
	udelay(1);
	BFM_HOST_Bus_Read32((EMMC_REG_BASE + offset), &val);

	dbg_printf(PRN_RES, " eMMC controller reset\n");
	return 0;
}

int cmd_EMMC_START(void)
{
    int iResult=0;

    if (EMMC_WaitForCardDetect() == 1)
    {
        iResult = EMMC_Init();
    }
    else
        iResult =NotFoundError;

    return iResult;
}

unsigned int do_emmc_get_param_customized(void)
{
	enum EMMC_VEDOR_ID {
		EMMC_VEDOR_TOSHIBA 	= 0x11,
		EMMC_VENDOR_MICRON	= 0x13,
		EMMC_VENDOR_SAMSUNG	= 0x15,
		EMMC_VENDOR_SANDISK	= 0x45,
	};
	unsigned int dll_phsel0 = 0x0;

	if(!g_manufacturerID) {
	/* driver shall not enter into this branch, as CID was already got during Identification stage,
	 * otherwise potential potential risk like RCA would be re-intialized for next transfer ? */
		UINT_T argument = 0, retval = NoError;
		P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();

		// Send CMD10 to retrieve the CID
		argument = pSDMMCP->CardReg.RCA;
		EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD10, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R2 | MM4_136_RES);
		retval = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R2, 0x100);
		if(retval != NoError) {
			dbg_printf(PRN_ERR, "CMD10: Error to get CID, Retval=0x%x \n", retval);
			return 0;
		}

		EMMC_Get_CID(pSDMMCP);   //print out CID
	}

	switch(g_manufacturerID) {
		case EMMC_VENDOR_SAMSUNG: dll_phsel0 = 0x2A; break;
		case EMMC_VEDOR_TOSHIBA:  dll_phsel0 = 0x0B; break;
		case EMMC_VENDOR_SANDISK: dll_phsel0 = 0x08; break;
		case EMMC_VENDOR_MICRON:  dll_phsel0 = 0x1F; break;
		default:	dll_phsel0 = 0;		     break;
	}

	return dll_phsel0;
}

unsigned long do_emmc_rpmbCapacity(void)
{
	if(NoError != EMMC_Get_Extcsd()) {
		dbg_printf(PRN_ERR, "ERR: get EXTCSD Error !\n");
		return 0;
	}

	if(rpmbPart_sector_count > 0x80) {
		dbg_printf(PRN_ERR, "ERR: Invalid rpmb partition Size index !\n");
		return 0;
	}

	dbg_printf(PRN_INFO, "RPMB Partition Size: 0x%x Bytes\n", rpmbPart_sector_count * 0x20000);
	return (rpmbPart_sector_count);
}

//Setup Request and Send to Device
static int rpmb_request_write(const rpmbFrame *frame_in) //(unsigned long long start_blk, unsigned long blks,)
{
	unsigned int iResult = NoError;
	// Initialize Flash Properties
	P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();

	P_MM4_SDMMC_CONTEXT_T pContext;
	P_MM4_BLK_CNTL pMM4_BLK_CNTL;
	unsigned int argument;
	int i = 0x0;

	//Check Card Status for write operation
	while(iResult != NoError)
	{
		mdelay(1);
        REG_READ32(((uintptr_t)pEMMCCTRL + 0x24), &iResult);
		if((iResult & 0x307) == 0x0)
			iResult = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
	}
	// Make sure State is correct
	if (pSDMMCP->State != READY)
		return SDMMCDeviceNotReadyError;

	// Set up State
	pSDMMCP->State = WRITE;
	// Does the start/end addresses align on Block Boundries? Probably not, record discard bytes
	//half sector for (256B) for RPMB, and address argument for CMD18 and CMD25 would be ignored.
	pSDMMCP->Trans.CardAddress = frame_in->address * RPMB_256_HALF_SECTOR_SIZE;  //would be ignored !
	pSDMMCP->Trans.StartDiscardWords = 0x0;
	pSDMMCP->Trans.EndDiscardWords = 0x0;
	//always be 0x1 for single Frame transfer, block_count * 512 !
	pSDMMCP->Trans.NumBlocks = ((frame_in->block_count) ? frame_in->block_count : 0x1) + \
		((long long)pSDMMCP->Trans.EndDiscardWords + (long long)pSDMMCP->Trans.StartDiscardWords) / pSDMMCP->ReadBlockSize;
	pSDMMCP->Trans.TransWordSize = pSDMMCP->Trans.NumBlocks * HARD512BLOCKLENGTH / 4; 		// Total Transfer Size including pre and post, in words

	// Convert to # of words
	pSDMMCP->Trans.StartDiscardWords /= 4;
	pSDMMCP->Trans.EndDiscardWords /= 4;
	pSDMMCP->Trans.LocalAddr = (uintptr_t)frame_in;
	pSDMMCP->Trans.WordIndex = 0;	// Stores Index of Current write position
//==============================================================================================

	// Assign our context value
	pContext = (P_MM4_SDMMC_CONTEXT_T)(pSDMMCP->pContext);
	// Must set MMC NUMBLK
	pMM4_BLK_CNTL = (P_MM4_BLK_CNTL) &pContext->pMMC4Reg->mm4_blk_cntl;
	pMM4_BLK_CNTL->blk_cnt = pSDMMCP->Trans.NumBlocks;

	// Do a CMD 25 Write Multiple Blocks
	if (pSDMMCP->SDMA_Mode)
	{
		EMMC_PRN(PRN_INFO, "\n MM4_DMA mode\n ");
		pMM4_BLK_CNTL->dma_bufsz = MM4_512_HOST_DMA_BDRY;
		pContext->pMMC4Reg->mm4_sysaddr = pSDMMCP->Trans.LocalAddr;
	}

	if (g_ADMA_wr==1)
		EMMC_SetupADMA( 1,pSDMMCP->Trans.NumBlocks);	// for ADMA

	if((MMC_RPMB_WRITE_KEY == htobe16(frame_in->req_resp)) || (MMC_RPMB_WRITE == htobe16(frame_in->req_resp)))
		argument = 0x1 | (0x1 << 31);
	else
		argument = 0x1; //(pMM4_BLK_CNTL->blk_cnt)
	EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD23, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R1 | MM4_48_RES);
	iResult = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);

	EMMCHC_EnableCmdInterrupt(pContext, 0);

	argument = frame_in->address; //Sector address, would be ignored !!
	EMMC_PRN(PRN_DBG, "\n EMMC_WriteBlocks, CMD25 write 0x%x blocks to 0x%x\n ", pMM4_BLK_CNTL->blk_cnt, argument);

	if(autocmd12 == 0)
		EMMC_SendDataCommandNoAuto12(pSDMMCP, XLLP_MMC_CMD25, argument, MM4_MULTI_BLOCK_TRAN, MM4_HOST_TO_CARD_DATA, MM4_RT_R1 | MM4_48_RES);
	else
		EMMC_SendDataCommand(pSDMMCP, XLLP_MMC_CMD25, argument, MM4_MULTI_BLOCK_TRAN, MM4_HOST_TO_CARD_DATA, MM4_RT_R1 | MM4_48_RES);
	// Wait for the Write to Complete
	while ((pSDMMCP->State != FAULT) && (pSDMMCP->State != READY)); 	// TBD add timeout Let the ISR run, we'll either get a fault or finish
	EMMC_PRN(PRN_DBG, "\npSDMMCP->State = 0x%X !\n ",pSDMMCP->State );
	iResult = EMMC_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);

	// enable cmd complete interrupt
	EMMCHC_EnableCmdInterrupt(pContext, 1);
	iResult |= EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
	while(iResult != NoError)
	{
		mdelay(2);
		iResult = EMMC_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
		if((unsigned int)i > 50*pSDMMCP->Trans.NumBlocks)
			break;
		i++;
	}
	if (iResult != NoError)
	{
		//pSDMMCP->State = READY;
		dbg_printf(PRN_ERR, "Card check status failure and timeout !\n");
	}

	EMMCHC_DmaSelect(pContext, 0x00);  // set DMA select back to SDMA

	if (pSDMMCP->State == FAULT) {
		iResult = EMMC_GetCardErrorState(pSDMMCP);		// TBD s/w reset?
		pSDMMCP->State = READY;
	}

#ifdef __SDIO_USE_INT__
#else
	pSDMMCP->State = READY;
#endif

	return iResult;
}

static int check_rpmb_results(UINT16_T result)
{
	int ret = NoError;

	switch(result) {
	case 0x0000:
	case 0x0080:
		dbg_printf(PRN_INFO, "INFO: Operation OK !\n");
		ret = NoError;
		break;
	case 0x0001:
	case 0x0081:
		dbg_printf(PRN_ERR, "ERR: General failure !\n");
		ret = result;
		break;
	case 0x0002:
	case 0x0082:
		dbg_printf(PRN_INFO, "ERR: Authentication(MAC) failure !\n");
		ret = result;
		break;
	case 0x0003:
	case 0x0083:
		dbg_printf(PRN_INFO, "ERR: Counter failure !\n");
		ret = result;
		break;
	case 0x0004:
	case 0x0084:
		dbg_printf(PRN_ERR, "ERR: Address failure !\n");
		ret = result;
		break;
	case 0x0005:
	case 0x0085:
		dbg_printf(PRN_ERR, "ERR: Write failure !\n");
		ret = result;
		break;
	case 0x0006:
	case 0x0086:
		dbg_printf(PRN_ERR, "ERR: Read failure !\n");
		ret = result;
		break;
	case 0x0007:
		dbg_printf(PRN_ERR, "ERR: Authentication Key not yet programmed ! \n");
		ret = result;
		break;
	default:
		dbg_printf(PRN_INFO, "ERR: unknow result status !\n");
		ret = -1;
	}

	return ret;
}

//Setup DataFrame and Parse Dataframe for Usage
int do_rpmb_op(UINT16_T rpmb_type, rpmbFrame *frame_in, rpmbFrame *frame_out,unsigned char *DatatoWrite, unsigned long WriteCnt,
					unsigned long DataCnt_perSector, unsigned char *KeyMac, unsigned long CardAddr_perSector)
{
	int err = NoError;
	unsigned int i = 0x0;

	if(EMMCSwitchPartition(0x3)) {
		err = SDMMC_SWITCH_ERROR;
		goto out;
	}

	if(!frame_in || !frame_out) {
		dbg_printf(PRN_ERR, "invalid frame data for rpmb operation!\n");
		err = -1;
		goto out;
	}
	memset(frame_in, 0, sizeof(rpmbFrame));
	memset(frame_out, 0, sizeof(rpmbFrame));

#ifdef CONFIG_DCACHE
	flush_dcache_all();
#endif

	dbg_printf(PRN_INFO, "INFO: RPMB Operation is %d.\n", rpmb_type);
	switch(rpmb_type) {
	case MMC_RPMB_WRITE:
		if(NULL == DatatoWrite) {
			dbg_printf(PRN_ERR, "invalid Source buffer for rpmb write!\n");
			err = -1;
			goto out;
		}
		frame_in->req_resp = htobe16(MMC_RPMB_WRITE);
		frame_in->result = 0x0;
		frame_in->block_count = htobe16(DataCnt_perSector);	//Block counter, the total count of half sectors
		frame_in->address = htobe16(CardAddr_perSector);		//Start Address
		frame_in->write_counter = htobe32(WriteCnt);
		memset(frame_in->nonce, 0x00, 0x10);		//16Bytes, Nonce
		//memcpy(frame_in->data, DatatoWrite, 0x100); //256Bytes, Data, MSB
		for(i=0; i<256; i++)
			frame_in->data[i] = DatatoWrite[255-i];
#ifdef CONFIG_DCACHE
		flush_dcache_all();
#endif
		if(rpmb_request_write(frame_in)) {
			dbg_printf(PRN_ERR, "Write Request Failed for rpmb operation!\n");
			err = -1;
			goto out;
		}
		break;
	case MMC_RPMB_WRITE_KEY:
		if(NULL == KeyMac) {
			dbg_printf(PRN_ERR, "invalid KeyMac for rpmb operation!\n");
			err = -1;
			goto out;
		}

		//Prepare to write key request
		frame_in->req_resp = htobe16(MMC_RPMB_WRITE_KEY);
		frame_in->result = 0x0;
		frame_in->block_count = 0x0;
		frame_in->address = 0x0;
		frame_in->write_counter = 0x0;
		memcpy(frame_in->key_mac, KeyMac, 0x20); //32Bytes, Key, MSB
		//for(i=0; i<32; i++)
		//	frame_in->key_mac[i] = KeyMac[31-i];
#ifdef CONFIG_DCACHE
		flush_dcache_all();
#endif
		if(rpmb_request_write(frame_in)) {
			dbg_printf(PRN_ERR, "write key cmd failed! \n");
			err = -1;
			goto out;
		}
		dbg_printf(PRN_INFO, "INFO: finished to write key for RPMB !\n");

		//Prepare to check write key result
		frame_in->req_resp = htobe16(MMC_RPMB_READ_RESP);
		memset(frame_in->key_mac, 0x0, 0x20); //32Bytes, Clear Key
#ifdef CONFIG_DCACHE
		flush_dcache_all();
#endif
		if(rpmb_request_write(frame_in)) {
			dbg_printf(PRN_ERR, "ERR: failed to send READ results CMD !\n");
			err = -1;
			goto out;
		}
		dbg_printf(PRN_INFO, "INFO: finished to send READ results CMD !\n");

		if(EMMC_READ((unsigned int)frame_in->address, 0x1, (unsigned char *)frame_out, TRUE)) {
			dbg_printf(PRN_ERR, "ERR: failed to read results !\n");
			err = -1;
			goto out;
		}
#ifdef CONFIG_DCACHE
		invalidate_dcache_all();
#endif

		if(MMC_RPMB_WRITE_KEY_RESP == htobe16(frame_out->req_resp)) {
			dbg_printf(PRN_INFO, "INFO: got write key response !\n");
			if(frame_out->block_count || frame_out->address || frame_out->write_counter) {
				err = -1;
				dbg_printf(PRN_ERR, "ERR: invalid content for write key response !\n");
				goto out;
			}
			else {
				err = check_rpmb_results(htobe16(frame_out->result));
				if(err)
					goto out;
			}
			dbg_printf(PRN_INFO, "INFO: write key operation successfully !\n");
		}
		else {
			dbg_printf(PRN_INFO, "!!!!!!! Response type: 0x%x, results: 0x%x !\n",
					htobe16(frame_out->req_resp), htobe16(frame_out->result));
			err = -1;
		}

		break;
	case MMC_RPMB_READ_CNT:
		frame_in->req_resp = htobe16(MMC_RPMB_READ_CNT);
		frame_in->result = 0x0;
		frame_in->block_count = 0x0;
		frame_in->address = 0x0;
		frame_in->write_counter = 0x0;
		for(i=0; i<16; i++)
			frame_in->nonce[i] = (0x10 + i);
		memset(frame_in->data, 0x0, 256);
#ifdef CONFIG_DCACHE
		flush_dcache_all();
#endif
		if(rpmb_request_write(frame_in)) {
			dbg_printf(PRN_ERR, "read counter cmd failed! \n");
			err = -1;
			goto out;
		}
		dbg_printf(PRN_INFO, "INFO: finished to send READ CNT CMD !\n");

		if(EMMC_READ((unsigned int)frame_in->address, 0x1, (unsigned char *)frame_out, TRUE)) {
			err = -1;
			dbg_printf(PRN_ERR, "ERR: failed to read results !\n");
			goto out;
		}
		dbg_printf(PRN_INFO, "INFO: finished to read counter for RPMB !\n");
#ifdef CONFIG_DCACHE
		invalidate_dcache_all();
#endif
		if(MMC_RPMB_READ_CNT_RESP == htobe16(frame_out->req_resp)) {
			dbg_printf(PRN_INFO, "INFO: got read cnt response !\n");
			if(frame_out->block_count || frame_out->address) {
				dbg_printf(PRN_ERR, "ERR: invalid content for read cnt response !\n");
				err = -1;
				goto out;
			}
			else {
				err = check_rpmb_results(htobe16(frame_out->result));
				if(err)
					goto out;
			}
			dbg_printf(PRN_INFO, "INFO: read cnt operation success, cnt: 0x%x \n", htobe32(frame_out->write_counter));
		}
		else {
			dbg_printf(PRN_INFO, "!!!!!!! Response type: 0x%x, write_counter:0x%x, results: 0x%x !\n",
			htobe16(frame_out->req_resp), htobe32(frame_out->write_counter), htobe16(frame_out->result));
			for(i=0; i<512; i++) {
				dbg_printf(PRN_ERR, "0x%x ", *((unsigned char *)frame_out + i));
				if(!((i+1)%16))
					dbg_printf(PRN_ERR, "\n");
			}
			err = -1;
			goto out;
		}

		break;
	case MMC_RPMB_READ:
	/*
		frame_in->req_resp = htobe16(MMC_RPMB_READ);
		frame_in->result = 0x0;
		frame_in->block_count = 0x0;
		frame_in->address = htobe16(CardAddr_perSector);
		frame_in->write_counter = 0x0;
		err = rpmb_request_write(frame_in);

		break;
	*/
	case MMC_RPMB_READ_CONFIG:
		dbg_printf(PRN_ERR, "ERR: Unsupported RPMB Operations due to MAC limitation !\n");
		err = NoError;
		break;
	default:
		err = -2;
		goto out;
	}

out:
	if(err)
		dbg_printf(PRN_ERR, "ERR: RPMB Operation failed for request: 0x%x, ErrCode:0x%x !\n",
						htobe16(frame_in->req_resp), err);

	EMMCSwitchPartition(0x0);
	return err;
}

int do_emmcinit(void)
{
	int iResult = 0;

	cmd_EMMC_RESET();
	EMMC_PRN(PRN_RES, "cmd_EMMC_RESET\n");
	iResult = cmd_EMMC_INIT();
	EMMC_PRN(PRN_RES, "cmd_EMMC_INIT\n");
	iResult = cmd_EMMC_START();
	EMMC_PRN(PRN_RES, "cmd_EMMC_START\n");
	//iResult = cmd_EMMC_EXTCSD();
	//EMMC_PRN(PRN_NOTICE, "cmd_EMMC_EXTCSD\n");
	return iResult;
}
void do_emmc_stop_alt_boot_mode(void)
{
	do_emmcinit();
	P_SDMMC_Properties_T pSDMMCP = EMMC_GetProperties();
	EMMC_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD0, MM4_CMD_TYPE_NORMAL, 0x0, MM4_NO_RES);
}

