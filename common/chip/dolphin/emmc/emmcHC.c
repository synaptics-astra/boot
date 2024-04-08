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

/******************************************************************************
**
**  FILENAME:       emmcHC.c
**
**  PURPOSE: MMC and SD specific low level controller routines for the MM4 controller
**
**
**
**
******************************************************************************/
#include "com_type.h"
#include "emmcHC.h"
#include "debug.h"
#include "io.h"
#include "emmcDiag.h"

//#define EMMC_DEBUG
#ifdef EMMC_DEBUG
#define EMMC_PRN(PRN_LEVEL,fmt, args...) dbg_printf(PRN_LEVEL, fmt, ## args)
#else
#define EMMC_PRN(PRN_LEVEL,fmt, args...)
#endif

#define UNUSED(var) do { (void)(var); } while(0)

#define sd_reg_set(offset, v123)                                \
    do{                                     \
        BFM_HOST_Bus_Write32(offset, v123);         \
    }while(0)

#define sd_reg_get(offset, v123)                                \
    do{                                     \
        BFM_HOST_Bus_Read32(offset, &v123);         \
    }while(0)

#define PHY_REG_OFFSET	0x300
#define PHY_CNFG_REG	0x300
#define PHY_CNFG_PHY_PWRGOOD_MASK	0x2
#define NO_OF_SDIO_CARDS 2

#define PHY_COMMDL_CNFG_REG_OFFSET 0x031C
#define PHY_SMPLDL_CNFG_REG_OFFSET 0x0320
#define PHY_AT_CTRL_R_REG_OFFSET 0x540

/* PHY RX SEL modes */
#define RXSELOFF        0x0
#define SCHMITT1P8		0x1
#define SCHMITT3P3		0x2
#define SCHMITT1P2		0x3
#define COMPARATOR1P8		0x4
#define COMPARATOR1P2		0x5
#define COMPARATOR1P82		0x6
#define INTERNALLPBK		0x7

void EMMCHC_PHYreset(P_MM4_SDMMC_CONTEXT_T pContext, int rst)
{
	unsigned long mm4_regaddr = (uintptr_t)pContext->pMMC4Reg + PHY_CNFG_REG;
	volatile unsigned int val;

	val = *(VUINT_T *)mm4_regaddr;
	val &= ~(0x1 << 0);
	val |= rst;
	*(VUINT_T *)mm4_regaddr = val;
}

/*PHY PAD GENERAL modes */
#define PAD_SP_8    0x8
#define PAD_SP_9    0x9
#define PAD_SN_8    0x8
typedef struct {
	unsigned int addr;
	unsigned int sp_bit;
	unsigned int sn_bit;
	unsigned int mask;
	unsigned int sp_value;
	unsigned int sn_value;
} PHY_PAD_GENERAL;

// BG7 PHY RXSEL config structure
typedef struct {
	unsigned int addr;
	unsigned int bit;
	unsigned int mask;
	unsigned int value;
} PHY_RXSEL;

/* PHY WEAKPULL_EN modes */
#define WPE_DISABLE  0x0
#define WPE_PULLUP   0x1
#define WPE_PULLDOWN 0x2
// BG7 PHY WEAKPULL_EN config structure
typedef struct {
	unsigned int addr;
	unsigned int bit;
	unsigned int mask;
	unsigned int value;
} PHY_WEAKPULL_EN;

/* PHY TXSLEW_CTRL_P modes */
#define TX_SLEW_P_0    0x0
#define TX_SLEW_P_2    0x2
#define TX_SLEW_P_3    0x3
// BG7 PHY TXSLEW_CTRL_P config structure
typedef struct {
	unsigned int addr;
	unsigned int bit;
	unsigned int mask;
	unsigned int value;
} PHY_TXSLEW_CTRL_P;

/* PHY TXSLEW_CTRL_N modes */
#define TX_SLEW_N_2    0x2
#define TX_SLEW_N_3    0x3
// BG7 PHY TXSLEW_CTRL_N config structure
typedef struct {
	unsigned int addr;
	unsigned int bit;
	unsigned int mask;
	unsigned int value;
} PHY_TXSLEW_CTRL_N;

// ********************* AT_CTRL_R 0x540********************************
//Name: Tuning and Auto-tuning control register
//Description: This register controls some aspects of tuning and auto-tuning features.
//*Do not program this register when HOST_CTRL2_R.SAMPLE_CLK_SEL is '1'*
//Size: 32 bits
//Offset: 0x540
typedef struct
{
    unsigned int at_en            : 1;  // 0
    unsigned int ci_sel           : 1;  // 1
    unsigned int swin_th_en       : 1;  // 2
    unsigned int prt_tune_err     : 1;  // 3
    unsigned int sw_tune_en       : 1;  // 4
    unsigned int rsvd2            : 3;  // 5
    unsigned int win_edge_sel     : 4;  // 8
    unsigned int rsvd3            : 4;  // 12
    unsigned int tune_clk_stop_en : 1;  // 16
    unsigned int pre_change_dly   : 2;  // 17
    unsigned int post_change_dly  : 2;  // 19
    unsigned int rsvd4            : 3;  // 21
    unsigned int swin_th_val      : 7;  // 24
    unsigned int rsvd5            : 1;  // 31
} AT_CTRL_R, *P_AT_CTRL_R;

typedef union
{
    AT_CTRL_R     at_ctrl_r_bits;
    unsigned int  at_ctrl_r_value;
} AT_CTRL_R_UNION, *P_AT_CTRL_R_UNION;


// ********************* DelayLine regs 0 0x31C ********************************
typedef struct
{
    //COMMDL_CNFG  @ DWC_MSHC_PTR_PHY_REGS + 0x1C
    unsigned int dlstep_sel     : 1;
    unsigned int dlout_en       : 1;
    unsigned int rsvd0          : 6;
    //SDCLKDL_CNFG @ DWC_MSHC_PTR_PHY_REGS + 0x1D
    unsigned int extdly_en      : 1;
    unsigned int bypass_en      : 1;
    unsigned int inpsel_cnfg    : 2;
    unsigned int update_dc      : 1;
    unsigned int rsvd1          : 3;
    //SDCLKDL_DC   @ DWC_MSHC_PTR_PHY_REGS + 0x1E
    unsigned int cckdl_dc       : 7;
    unsigned int rsvd2          : 1;
} DL_REGS_0, *P_DL_REGS_0;

typedef union
{
    DL_REGS_0       dl_regs_0_bits;
    unsigned int  dl_regs_0_value;
} DL_REGS_0_UNION, *P_DL_REGS_0_UNION;

// ********************* DelayLine regs 1 0x320 ********************************
typedef struct
{
    //SMPLDL_CNFG  @ DWC_MSHC_PTR_PHY_REGS + 0x20
    unsigned int s_extdly_en      : 1;
    unsigned int s_bypass_en        : 1;
    unsigned int s_inpsel_cnfg  : 2;
    unsigned int s_inpsel_override: 1;
    unsigned int rsvd3          : 3;
    //ATDL_CNFG    @ DWC_MSHC_PTR_PHY_REGS + 0x21
    unsigned int a_extdly_en        : 1;
    unsigned int a_bypass_en        : 1;
    unsigned int a_inpsel_cnfg  : 2;
    unsigned int rsvd4          : 4;
} DL_REGS_1, *P_DL_REGS_1;

typedef union
{
    DL_REGS_1       dl_regs_1_bits;
    unsigned int  dl_regs_1_value;
} DL_REGS_1_UNION, *P_DL_REGS_1_UNION;


//default PHY config
PHY_PAD_GENERAL pad_general_1v8 =
{
	0x300, 16, 20, 0xF, PAD_SP_8, PAD_SN_8
};

PHY_RXSEL pad_rxsel_1v8[5] =
{
/*CMD*/ {0x304, 0,  0x7, 1},
/*DAT*/ {0x304, 16, 0x7, 1},
/*CLK*/ {0x308, 0,  0x7, 0},
/*STB*/ {0x308, 16, 0x7, 1},
/*RST*/ {0x30C, 0,  0x7, 1}
};

PHY_WEAKPULL_EN pad_weakpull_en_1v8[5] =
{
/*CMD*/ {0x304, 0 + 3,  0x3, WPE_PULLUP},
/*DAT*/ {0x304, 16 + 3,     0x3, WPE_PULLUP},
/*CLK*/ {0x308, 0 + 3,  0x3, WPE_DISABLE},
/*STB*/ {0x308, 16 + 3,     0x3, WPE_PULLDOWN},
/*RST*/ {0x30C, 0 + 3,  0x3, WPE_PULLUP}
};

PHY_TXSLEW_CTRL_P pad_txslew_ctrl_p_1v8[5] =
{
/*CMD*/ {0x304, 0 + 5,  0xF, TX_SLEW_P_0},
/*DAT*/ {0x304, 16 + 5,     0xF, TX_SLEW_P_0},
/*CLK*/ {0x308, 0 + 5,  0xF, TX_SLEW_P_0},
/*STB*/ {0x308, 16 + 5,     0xF, TX_SLEW_P_0},
/*RST*/ {0x30C, 0 + 5,  0xF, TX_SLEW_P_0}
};

PHY_TXSLEW_CTRL_N pad_txslew_ctrl_n_1v8[5] =
{
/*CMD*/ {0x304, 0 + 9,  0xF, TX_SLEW_N_3},
/*DAT*/ {0x304, 16 + 9,     0xF, TX_SLEW_N_3},
/*CLK*/ {0x308, 0 + 9,  0xF, TX_SLEW_N_3},
/*STB*/ {0x308, 16 + 9,     0xF, TX_SLEW_N_3},
/*RST*/ {0x30C, 0 + 9,  0xF, TX_SLEW_N_3}
};

//3v3 PHY config
PHY_PAD_GENERAL pad_general_3v3 =
{
	0x300, 16, 20, 0xF, PAD_SP_9, PAD_SN_8
};

PHY_RXSEL pad_rxsel_3v3[5] =
{
/*CMD*/	{0x304, 0, 	0x7, SCHMITT3P3},
/*DAT*/	{0x304, 16, 	0x7, SCHMITT3P3},
/*CLK*/	{0x308, 0, 	0x7, RXSELOFF},
/*STB*/	{0x308, 16, 	0x7, SCHMITT3P3},
/*RST*/	{0x30C, 0, 	0x7, SCHMITT3P3}
};

PHY_WEAKPULL_EN pad_weakpull_en_3v3[5] =
{
/*CMD*/	{0x304, 0 + 3, 	0x3, WPE_PULLUP},
/*DAT*/	{0x304, 16 + 3, 	0x3, WPE_PULLUP},
/*CLK*/	{0x308, 0 + 3, 	0x3, WPE_DISABLE},
/*STB*/	{0x308, 16 + 3, 	0x3, WPE_PULLDOWN},
/*RST*/	{0x30C, 0 + 3, 	0x3, WPE_PULLUP}
};

PHY_TXSLEW_CTRL_P pad_txslew_ctrl_p_3v3[5] =
{
/*CMD*/ {0x304, 0 + 5,	0xF, TX_SLEW_P_3},
/*DAT*/ {0x304, 16 + 5, 	0xF, TX_SLEW_P_3},
/*CLK*/ {0x308, 0 + 5,	0xF, TX_SLEW_P_3},
/*STB*/ {0x308, 16 + 5, 	0xF, TX_SLEW_P_3},
/*RST*/ {0x30C, 0 + 5,	0xF, TX_SLEW_P_3}
};

PHY_TXSLEW_CTRL_N pad_txslew_ctrl_n_3v3[5] =
{
/*CMD*/ {0x304, 0 + 9,	0xF, TX_SLEW_N_2},
/*DAT*/ {0x304, 16 + 9, 	0xF, TX_SLEW_N_2},
/*CLK*/ {0x308, 0 + 9,	0xF, TX_SLEW_N_2},
/*STB*/ {0x308, 16 + 9, 	0xF, TX_SLEW_N_2},
/*RST*/ {0x30C, 0 + 9,	0xF, TX_SLEW_N_2}
};

int EMMCHC_PHYconfig(P_MM4_SDMMC_CONTEXT_T pContext, int rxsel)
{
	int i;
	volatile unsigned int val;
	unsigned long mm4_regaddr = (uintptr_t)pContext->pMMC4Reg;
	int ret = 0;

	UNUSED(rxsel);
	//config PHY_CNFG, general configuration
	//Dolphin_BG7_PHY_bring_up_sequence.xlsx
	//step 10
	sd_reg_get(mm4_regaddr + pad_general_1v8.addr, val);
	val &= ~(pad_general_1v8.mask<<pad_general_1v8.sp_bit);
	val |= (pad_general_1v8.sp_value<<pad_general_1v8.sp_bit);
	val &= ~(pad_general_1v8.mask<<pad_general_1v8.sn_bit);
	val |= (pad_general_1v8.sn_value<<pad_general_1v8.sn_bit);
	sd_reg_set(mm4_regaddr + pad_general_1v8.addr, val);

	//Dolphin_BG7_PHY_bring_up_sequence.xlsx
	//step 11~15
	for(i=0; i<5; i++)
	{
		//config PHY RXSEL
		sd_reg_get(mm4_regaddr + pad_rxsel_1v8[i].addr, val);
		val &= ~(pad_rxsel_1v8[i].mask<<pad_rxsel_1v8[i].bit);
		val |= (pad_rxsel_1v8[i].value<<pad_rxsel_1v8[i].bit);
		sd_reg_set(mm4_regaddr + pad_rxsel_1v8[i].addr, val);

//TODO: ZL: will cause cannot recongnize card problem, disable for now
#if 1// PLATFORM == ASIC
//#if 0
//#error
		//config PHY WEAKPULL_EN
		sd_reg_get(mm4_regaddr + pad_weakpull_en_1v8[i].addr, val);
		val &= ~(pad_weakpull_en_1v8[i].mask<<pad_weakpull_en_1v8[i].bit);
		val |= (pad_weakpull_en_1v8[i].value<<pad_weakpull_en_1v8[i].bit);
		sd_reg_set(mm4_regaddr + pad_weakpull_en_1v8[i].addr, val);
#endif
		//config PHY TXSLEW_CTRL_P
		sd_reg_get(mm4_regaddr + pad_txslew_ctrl_p_1v8[i].addr, val);
		val &= ~(pad_txslew_ctrl_p_1v8[i].mask<<pad_txslew_ctrl_p_1v8[i].bit);
		val |= (pad_txslew_ctrl_p_1v8[i].value<<pad_txslew_ctrl_p_1v8[i].bit);
		sd_reg_set(mm4_regaddr + pad_txslew_ctrl_p_1v8[i].addr, val);

		//config PHY TXSLEW_CTRL_N
		sd_reg_get(mm4_regaddr + pad_txslew_ctrl_n_1v8[i].addr, val);
		val &= ~(pad_txslew_ctrl_n_1v8[i].mask<<pad_txslew_ctrl_n_1v8[i].bit);
		val |= (pad_txslew_ctrl_n_1v8[i].value<<pad_txslew_ctrl_n_1v8[i].bit);
		sd_reg_set(mm4_regaddr + pad_txslew_ctrl_n_1v8[i].addr, val);
	}

	return ret;
}

/**********************************************************
*   EMMC_PHY_DL_SET
*   Input:
*	   register address
*   Output:
*      register value
*   Returns:
***********************************************************/
void EMMCHC_PHY_DL_SET(P_MM4_SDMMC_CONTEXT_T pContext, unsigned int dlreg_offset, unsigned dl_setting)
{
	//unsigned int reg_val;
	unsigned long mm4_regaddr = (uintptr_t)pContext->pMMC4Reg + PHY_REG_OFFSET + dlreg_offset;


	EMMCHC_StopBusClock(pContext);

	//reg_val = *(VUINT_T *)mm4_regaddr;
	//EMMC_PRN(PRN_RES, "0x%x=0x%x\n", dlreg_offset, reg_val);

	*(VUINT_T *)mm4_regaddr = dl_setting;

	EMMCHC_StartBusClock(pContext);

	return;
}
void SDHC_switch_1p8signal(P_MM4_SDMMC_CONTEXT_T pContext, int enable)
{
    P_MM4_ACMD12_ER p_ctrl2 = (P_MM4_ACMD12_ER)&pContext->pMMC4Reg->mm4_acmd12_er;
    EMMC_PRN(PRN_INFO, "\n  SD 0x3c value is 0x%x\n", *(UINT_T*)(p_ctrl2));
    p_ctrl2->sgh_v18_en = enable;
    EMMC_PRN(PRN_INFO, "  SD 0x3c value is 0x%x\n", *(UINT_T*)(p_ctrl2));
    return;
}

//ZL: Dolphin_BG7_PHY_bring_up_sequence.xlsx
//step 2~8
void emmc_phy_tx_delay(P_MM4_SDMMC_CONTEXT_T pContext, UINT_T tx_delay_code, int extdly_en)
{
	unsigned long mm4_regaddr = (uintptr_t)pContext->pMMC4Reg, phy_offset = EMMC_PHY_OFFSET, reg_val;

	EMMCHC_StopBusClock(pContext);
	udelay(1);
	BFM_HOST_Bus_Read32(mm4_regaddr + phy_offset + EMMC_PHY_COMMDL_CNFG, &reg_val);
	reg_val |= 0x1000; //SDCLKDL_CNFG.UPDATE_DC =1
	if(extdly_en)
		reg_val |= 0x100; //EXTDL_EN
	else
		reg_val &= ~0x100; //EXTDL_EN
	BFM_HOST_Bus_Write32(mm4_regaddr + phy_offset + EMMC_PHY_COMMDL_CNFG, reg_val);

	BFM_HOST_Bus_Read32(mm4_regaddr + phy_offset + EMMC_PHY_COMMDL_CNFG, &reg_val);
	reg_val &= ~0x7F0000; //CCKDL_DC = tx_delay_code;
	reg_val |= tx_delay_code << 16;
	BFM_HOST_Bus_Write32(mm4_regaddr + phy_offset + EMMC_PHY_COMMDL_CNFG, reg_val);

	BFM_HOST_Bus_Read32(mm4_regaddr + phy_offset + EMMC_PHY_COMMDL_CNFG, &reg_val);
	reg_val &= ~0x1000; //SDCLKDL_CNFG.UPDATE_DC =0
	BFM_HOST_Bus_Write32(mm4_regaddr + phy_offset + EMMC_PHY_COMMDL_CNFG, reg_val);
	EMMCHC_StartBusClock(pContext);

	return;
}

	//ZL: Dolphin_BG7_PHY_bring_up_sequence.xlsx
	//step 9
	void SDHC_PHYTuningSetup(P_MM4_SDMMC_CONTEXT_T pContext)
	{
	unsigned int mm4_regaddr = (uintptr_t)(pContext->pMMC4Reg);

	AT_CTRL_R_UNION at_ctrl_r;

	sd_reg_get(mm4_regaddr + PHY_AT_CTRL_R_REG_OFFSET, at_ctrl_r.at_ctrl_r_value);

	//Before any tuning sequence,do these to avoid the possible cclk_rx clock glitch introduced by delayline
	//1.) AT_CTRL_R.TUNE_CLK_STOP_EN = 1.
	//2.) AT_CTRL_R.POST_CHANGE_DLY = 3.
	//3.) AT_CTRL_R.PRE_CHANGE_DLY = 3.
	at_ctrl_r.at_ctrl_r_bits.tune_clk_stop_en = 1;
	at_ctrl_r.at_ctrl_r_bits.post_change_dly = 3;
	at_ctrl_r.at_ctrl_r_bits.pre_change_dly = 3;
	sd_reg_set(mm4_regaddr + PHY_AT_CTRL_R_REG_OFFSET, at_ctrl_r.at_ctrl_r_value);
	}

/******************************************************************************
  Description:
    Start MMC bus clock. Only after starting bus clock, communication between
    controller and card is possible
  Input Parameters:
    pContext--Pointer to MMC context structure
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
#if 0
void MMC4ModeSelect(P_MM4_SDMMC_CONTEXT_T pContext, UINT_T sd_mode)
{
  P_MM4_CNTL1 pMM4_CNTL1;
  MM4_CNTL1_UNION MM4_cntl1;

  pMM4_CNTL1 = (P_MM4_CNTL1)((VUINT32_T) &pContext->pMMC4Reg->mm4_cntl1);
  MM4_cntl1.mm4_cntl1_value = *(VUINT_T *)pMM4_CNTL1;
  MM4_cntl1.mm4_cntl1_bits.eightbitmd = sd_mode;

  *(VUINT_T *)pMM4_CNTL1 = MM4_cntl1.mm4_cntl1_value;
  return;
}
#endif
void EMMCHC_HighSpeedSelect(P_MM4_SDMMC_CONTEXT_T pContext, UINT_T hs_mode)
{
  P_MM4_CNTL1 pMM4_CNTL1;
  MM4_CNTL1_UNION MM4_cntl1;

  pMM4_CNTL1 = (P_MM4_CNTL1)((uintptr_t) &pContext->pMMC4Reg->mm4_cntl1);
  MM4_cntl1.mm4_cntl1_value = *(VUINT_T *)pMM4_CNTL1;
  MM4_cntl1.mm4_cntl1_bits.hispeed = hs_mode & 0x1;

  *(VUINT_T *)pMM4_CNTL1 = MM4_cntl1.mm4_cntl1_value;
  return;
}

/*******************************************************************
 * BG6CD only support HS-SDR and HS-DDR mode
 * set emmc mode, mode0: high speed SDR, mode1:high speed DDR
********************************************************************/
void EMMCHC_host_mode(P_MM4_SDMMC_CONTEXT_T pContext, UINT_T uhs_mode)
{
	volatile P_MM4_ACMD12_ER pMM4_crtl2;
	UINT32_T	MM4_crtl2;

	MM4_crtl2 = pContext->pMMC4Reg->mm4_acmd12_er;
	pMM4_crtl2 = (volatile P_MM4_ACMD12_ER)&MM4_crtl2;

	if(uhs_mode == 0)
		pMM4_crtl2->uhs_mode_sel = 1;	 //HS-SDR
	else if(uhs_mode == 1)
		pMM4_crtl2->uhs_mode_sel = 4;	 //HS DDR
	else if(uhs_mode == 2)
		pMM4_crtl2->uhs_mode_sel = 3;	 //HS200
	else if(uhs_mode == 3)
		pMM4_crtl2->uhs_mode_sel = 7;	 //HS400

	pContext->pMMC4Reg->mm4_acmd12_er = MM4_crtl2;
}

#if 0
void MMC4BusWidthSelect(P_MM4_SDMMC_CONTEXT_T pContext, UINT_T sd_4bit)
{
  P_MM4_CNTL1 pMM4_CNTL1;
  MM4_CNTL1_UNION MM4_cntl1;

  pMM4_CNTL1 = (P_MM4_CNTL1)((VUINT32_T) &pContext->pMMC4Reg->mm4_cntl1);
  MM4_cntl1.mm4_cntl1_value = *(VUINT_T *)pMM4_CNTL1;
  MM4_cntl1.mm4_cntl1_bits.datawidth = sd_4bit;

  *(VUINT_T *)pMM4_CNTL1 = MM4_cntl1.mm4_cntl1_value;
  return;
}
#endif



#if 1
void EMMCHC_DmaSelect(P_MM4_SDMMC_CONTEXT_T pContext, UINT_T select)
{
  P_MM4_CNTL1 pMM4_CNTL1;
  MM4_CNTL1_UNION MM4_cntl1;

  pMM4_CNTL1 = (P_MM4_CNTL1)((uintptr_t) &pContext->pMMC4Reg->mm4_cntl1);
  MM4_cntl1.mm4_cntl1_value = *(VUINT_T *)pMM4_CNTL1;
  MM4_cntl1.mm4_cntl1_bits.dma_sel = select & 0x3;

  *(VUINT_T *)pMM4_CNTL1 = MM4_cntl1.mm4_cntl1_value;
  return;
}
#endif

void SDHC_EnableInterrupt(P_MM4_SDMMC_CONTEXT_T pContext, int loc, unsigned int enable)
{
	P_MM4_I_STAT_EN pMM4_I_STAT_EN;
	P_MM4_I_SIGN_EN pMM4_I_SIGN_EN;
	MM4_I_STAT_UNION MM4_i_stat;

	// Capture existing Value
	pMM4_I_SIGN_EN = (P_MM4_I_SIGN_EN)((uintptr_t) &pContext->pMMC4Reg->mm4_i_sig_en);
	pMM4_I_STAT_EN = (P_MM4_I_STAT_EN)((uintptr_t) &pContext->pMMC4Reg->mm4_i_stat_en);

	MM4_i_stat.mm4_i_stat_value = *(VUINT_T*)pMM4_I_STAT_EN;
	dbg_printf(PRN_INFO, "\n  Interrupt Signal enable value is 0x%x",MM4_i_stat.mm4_i_stat_value);

	// Route the interrupt signal enable register
	 MM4_i_stat.mm4_i_stat_value &= ~(1<<loc);
	 MM4_i_stat.mm4_i_stat_value |= ((enable&0x1)<<loc);

	*(VUINT_T*)pMM4_I_STAT_EN = MM4_i_stat.mm4_i_stat_value;

	// Route the interrupt signal enable register
	*(VUINT_T*)pMM4_I_SIGN_EN = MM4_i_stat.mm4_i_stat_value;

	MM4_i_stat.mm4_i_stat_value = *(VUINT_T*)pMM4_I_STAT_EN;
	dbg_printf(PRN_INFO, "\n  Interrupt Signal enable value is 0x%x\n",MM4_i_stat.mm4_i_stat_value);
	return;
}

/******************************************************************************
  Description:
    Start MMC/SD Internal bus clock.  MUST be done to start MM4CLK!
    Only after starting bus clock, communication between
    controller and card is possible
  Input Parameters:
    pContext--Pointer to MMC context structure
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void EMMCHC_StartInternalBusClock(P_MM4_SDMMC_CONTEXT_T pContext)
{
  P_MM4_CNTL2 pMM4_CNTL2;
  MM4_CNTL2_UNION MM4_cntl2;

  pMM4_CNTL2 = (P_MM4_CNTL2)((uintptr_t) &pContext->pMMC4Reg->mm4_cntl2);
  MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
  MM4_cntl2.mm4_cntl2_bits.inter_clk_en = 1;
  *(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;

  // Wait for clock to become stable. * TBD * Add timeout
  mdelay(10);
  do
  {
  	MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
	udelay(1);
  } while (!MM4_cntl2.mm4_cntl2_bits.inter_clk_stable);

  return;
}

/******************************************************************************
  Description:
    Stops the MMC/SD Internal bus clock.
  Input Parameters:
    pContext--Pointer to MMC context structure
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void EMMCHC_StopInternalBusClock(P_MM4_SDMMC_CONTEXT_T pContext)
{
  P_MM4_CNTL2 pMM4_CNTL2;
  MM4_CNTL2_UNION MM4_cntl2;

  pMM4_CNTL2 = (P_MM4_CNTL2)((uintptr_t) &pContext->pMMC4Reg->mm4_cntl2);
  MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
  MM4_cntl2.mm4_cntl2_bits.inter_clk_en = 0;
  *(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;

  // Wait for clock to become stable. * TBD * Add timeout
  do
  {
  	MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
  } while (MM4_cntl2.mm4_cntl2_bits.inter_clk_stable);

  return;
}


/******************************************************************************
  Description:
    Start MMC bus clock. Only after starting bus clock, communication between
    controller and card is possible
  Input Parameters:
    pContext--Pointer to MMC context structure
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void EMMCHC_StartBusClock(P_MM4_SDMMC_CONTEXT_T pContext)
{
  P_MM4_CNTL2 pMM4_CNTL2;
  MM4_CNTL2_UNION MM4_cntl2;

  pMM4_CNTL2 = (P_MM4_CNTL2)((uintptr_t) &pContext->pMMC4Reg->mm4_cntl2);
  MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;

  MM4_cntl2.mm4_cntl2_bits.mm4clken = 1;

  *(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;
  return;
}

/******************************************************************************
  Description:
    Stops MMC bus clock.
  Input Parameters:
    pContext--Pointer to MMC context structure
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void EMMCHC_StopBusClock (P_MM4_SDMMC_CONTEXT_T pContext)
{
  //UINT32_T retry_count = 0xff;
  P_MM4_CNTL2 pMM4_CNTL2;
  MM4_CNTL2_UNION MM4_cntl2;

  // Request bus clock stop
  pMM4_CNTL2 = (P_MM4_CNTL2)((uintptr_t) &pContext->pMMC4Reg->mm4_cntl2);
  MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
  MM4_cntl2.mm4_cntl2_bits.mm4clken = 0;
  *(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;

  return;
}

extern void EMMC_PhyInitialization(void);

/******************************************************************************
  Description:
    Set a new MMC bus clock rate. This function stops and resumes bus clock.
  Input Parameters:
    pContext
      Pointer to MMC context structure
    rate
      bus clock speed
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void EMMCHC_SetBusRate(P_MM4_SDMMC_CONTEXT_T pContext, UINT_T rate)
{
	P_MM4_CNTL2 pMM4_CNTL2;
	MM4_CNTL2_UNION MM4_cntl2;
	unsigned int timeout = 0;

	EMMC_PRN(PRN_INFO,"Start to setup freq divide rate = %d\n", rate);
	// Request bus clock stop, set rate, start clock.
	//EMMCHC_StopBusClock(pContext);

	// BG6CD/NIUE design, internal clock must be off to change clock divider
	// and on again to sync to target clock.
	//EMMCHC_StopInternalBusClock(pContext);
	pMM4_CNTL2 = (P_MM4_CNTL2)((uintptr_t) &pContext->pMMC4Reg->mm4_cntl2);
	MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
	MM4_cntl2.mm4_cntl2_bits.mm4clken = 0;
	MM4_cntl2.mm4_cntl2_bits.inter_clk_en = 0;
	*(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;

	MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
	// Update the rate and start the clock.
	MM4_cntl2.mm4_cntl2_bits.sd_freq_sel_lo = (rate & 0xFF);
	MM4_cntl2.mm4_cntl2_bits.sd_freq_sel_hi = ((rate >> 8) & 3);
	MM4_cntl2.mm4_cntl2_bits.inter_clk_en = 1;
	*(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;

	// Wait for clock to become stable. * TBD * Add timeout
	/* max 20ms */
	timeout = 20;
	do {
		if(timeout == 0) {
			dbg_printf(PRN_ERR, "%s: Internal clock never stabilised.\n",
					__func__);
			return;
		}

		mdelay(1);
		timeout--;

		MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
	} while (!MM4_cntl2.mm4_cntl2_bits.inter_clk_stable);
	//--------------------------------------------------
	//EMMCHC_StartInternalBusClock(pContext);
	EMMC_PRN(PRN_INFO,"freq divide rate = %d\n", rate);
	EMMC_PhyInitialization();

	//----------------------------------------------
	EMMCHC_StartBusClock(pContext);
	return;
}


/******************************************************************************
  Description:
    This routine unmasks and enables or masks and disables required interrupts
    needed by the driver
    Input Parameters:
    	pContext
      		Pointer to MMC context structure
			Desire - Enable or Disable the interrupts
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void EMMCHC_EnableDisableIntSources(P_MM4_SDMMC_CONTEXT_T pContext, UINT8_T Desire)
{
	P_MM4_I_STAT pMM4_I_STAT;
	MM4_I_STAT_UNION MM4_i_stat;

	// Capture existing Value
	pMM4_I_STAT = (P_MM4_I_STAT)((uintptr_t) &pContext->pMMC4Reg->mm4_i_sig_en);

	MM4_i_stat.mm4_i_stat_value = *(VUINT_T*)pMM4_I_STAT;
	// Route the interrupt signal enable register
	MM4_i_stat.mm4_i_stat_bits.cmdcomp = Desire;
	MM4_i_stat.mm4_i_stat_bits.xfrcomp = Desire;
	MM4_i_stat.mm4_i_stat_bits.bufwrrdy = Desire;
	MM4_i_stat.mm4_i_stat_bits.bufrdrdy = Desire;
	// KT added for card detection
	MM4_i_stat.mm4_i_stat_bits.cdins = Desire;
	MM4_i_stat.mm4_i_stat_bits.cdrem = Desire;
	//
	MM4_i_stat.mm4_i_stat_bits.cdint = Desire;
	MM4_i_stat.mm4_i_stat_bits.errint = Desire;
	MM4_i_stat.mm4_i_stat_bits.cmdtout = Desire;   // KT added
	MM4_i_stat.mm4_i_stat_bits.cmdcrcerr= Desire;   // KT added
	MM4_i_stat.mm4_i_stat_bits.cmdenderr = Desire;
	MM4_i_stat.mm4_i_stat_bits.cmdidxerr = Desire;  //
	MM4_i_stat.mm4_i_stat_bits.datatout = Desire;
	MM4_i_stat.mm4_i_stat_bits.datacrcerr = Desire;
	MM4_i_stat.mm4_i_stat_bits.dataenderr = Desire;
#if SDIO_DMA
	MM4_i_stat.mm4_i_stat_bits.dmaint = Desire;    //KT added
	MM4_i_stat.mm4_i_stat_bits.ac12err = Desire;   //KT added auto CMD12 error
	MM4_i_stat.mm4_i_stat_bits.admaerr = Desire;   //TB added adma error interrupt
#endif
	MM4_i_stat.mm4_i_stat_bits.boot_ack_err = Desire;	//Lig added

	// Write it out
	*(VUINT_T*)pMM4_I_STAT = MM4_i_stat.mm4_i_stat_value;

	// Now remove the masks
	pMM4_I_STAT = (P_MM4_I_STAT)((uintptr_t) &pContext->pMMC4Reg->mm4_i_stat_en);
	MM4_i_stat.mm4_i_stat_value = *(VUINT_T*)pMM4_I_STAT;
	MM4_i_stat.mm4_i_stat_bits.cmdcomp = Desire;
	// we handle xfrcomp interrupt in separate function
	// MARVELL LEAGACY IP has the problem of reporting xfrcomp interrupt
	// twice.
	//MM4_i_stat.mm4_i_stat_bits.xfrcomp = Desire;
	MM4_i_stat.mm4_i_stat_bits.bufwrrdy = Desire;
	MM4_i_stat.mm4_i_stat_bits.bufrdrdy = Desire;
	// KT added for card detection
	MM4_i_stat.mm4_i_stat_bits.cdins = Desire;
	MM4_i_stat.mm4_i_stat_bits.cdrem = Desire;
	//
#if PLATFORM == ASIC
	MM4_i_stat.mm4_i_stat_bits.cdint = Desire;//	--> Satya  ; KT removed the workaround
#endif
	MM4_i_stat.mm4_i_stat_bits.errint = Desire;
	MM4_i_stat.mm4_i_stat_bits.cmdtout = Desire;
	MM4_i_stat.mm4_i_stat_bits.cmdcrcerr= Desire;   // KT added
	MM4_i_stat.mm4_i_stat_bits.cmdenderr = Desire;  //
	MM4_i_stat.mm4_i_stat_bits.cmdidxerr = Desire;  //
	MM4_i_stat.mm4_i_stat_bits.datatout = Desire;
	MM4_i_stat.mm4_i_stat_bits.datacrcerr = Desire;
	MM4_i_stat.mm4_i_stat_bits.dataenderr = Desire;

#if SDIO_DMA
	MM4_i_stat.mm4_i_stat_bits.dmaint = Desire;    //KT added
	MM4_i_stat.mm4_i_stat_bits.ac12err = Desire;   //KT added auto CMD12 error
	MM4_i_stat.mm4_i_stat_bits.admaerr = Desire;   //TB added adma error interrupt
#endif
	MM4_i_stat.mm4_i_stat_bits.boot_ack_err = Desire;	//Lig added

	// Write it out
	*(VUINT_T*)pMM4_I_STAT = MM4_i_stat.mm4_i_stat_value;

	dbg_printf(PRN_INFO, "\n MM4_i_stat.mm4_i_stat_value is 0x%x\n", MM4_i_stat.mm4_i_stat_value);


	return;
}


/******************************************************************************
  Description:
    Set the data response timeout value.
  Input Parameters:
    pContext
      Pointer to MMC context structure
    CounterValue
      the value which will be written into DTOCNTR
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void EMMCHC_SetDataTimeout(P_MM4_SDMMC_CONTEXT_T pContext, UINT8_T CounterValue)
{
 P_MM4_CNTL2 pMM4_CNTL2;
 MM4_CNTL2_UNION MM4_cntl2;

 // Set the register
 pMM4_CNTL2 = (P_MM4_CNTL2)((uintptr_t) &pContext->pMMC4Reg->mm4_cntl2);
 MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
 MM4_cntl2.mm4_cntl2_bits.dtocntr = CounterValue;

 // Write Back
 *(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;
 return;
}

/******************************************************************************
  Description:
    This function will induce a software reset of all MMC4 data lines
  Input Parameters:
    pContext
      Pointer to MMC context structure
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void EMMCHC_DataSWReset(P_MM4_SDMMC_CONTEXT_T pContext)
{
 P_MM4_CNTL2 pMM4_CNTL2;
 MM4_CNTL2_UNION MM4_cntl2;

 // Set the register
 pMM4_CNTL2 = (P_MM4_CNTL2)((uintptr_t) &pContext->pMMC4Reg->mm4_cntl2);
 MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
 MM4_cntl2.mm4_cntl2_bits.datswrst = 1;

 // Write Back
 *(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;
 return;
}

/******************************************************************************
  Description:
    This function will induce a full software reset of all MMC4 components except
	MM4_CAPX
  Input Parameters:
    pContext
      Pointer to MMC context structure
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void EMMCHC_FullSWReset(P_MM4_SDMMC_CONTEXT_T pContext)
{
 P_MM4_CNTL2 pMM4_CNTL2;
 MM4_CNTL2_UNION MM4_cntl2;

 // Set the register
 pMM4_CNTL2 = (P_MM4_CNTL2)((uintptr_t) &pContext->pMMC4Reg->mm4_cntl2);
 MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
 MM4_cntl2.mm4_cntl2_bits.mswrst = 1;

 // Write Back
 *(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;
 return;
}

/******************************************************************************
  Description:
    This function will induce a software reset of all MMC4 data lines
  Input Parameters:
    pContext
      Pointer to MMC context structure
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void EMMCHC_CMDSWReset(P_MM4_SDMMC_CONTEXT_T pContext)
{
 P_MM4_CNTL2 pMM4_CNTL2;
 MM4_CNTL2_UNION MM4_cntl2;

 // Set the register
 pMM4_CNTL2 = (P_MM4_CNTL2)((uintptr_t) &pContext->pMMC4Reg->mm4_cntl2);
 MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
 MM4_cntl2.mm4_cntl2_bits.cmdswrst = 1;

 // Write Back
 *(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;
 return;
}


/**************************************************************************/
void EMMCHC_StopAtBlockGap(P_MM4_SDMMC_CONTEXT_T pContext)
{
 P_MM4_CNTL1 pMM4_CNTL1;
 MM4_CNTL1_UNION MM4_cntl1;

 // Set the register
 pMM4_CNTL1 = (P_MM4_CNTL1)((uintptr_t) &pContext->pMMC4Reg->mm4_cntl1);
 MM4_cntl1.mm4_cntl1_value = *(VUINT_T *)pMM4_CNTL1;
 MM4_cntl1.mm4_cntl1_bits.bgreqstp = 1;

 // Write Back
 *(VUINT_T *)pMM4_CNTL1 = MM4_cntl1.mm4_cntl1_value;
 return;
}
/******************************************************************************
  Description:
    Set up the registers of the controller to start the transaction to
    communicate to the card for data related command.  The commands are clearly defined in the MMC
    specification.
  Input Parameters:
    pContext
      	Pointer to MMC context structure
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
void EMMCHC_SendDataCommand(P_MM4_SDMMC_CONTEXT_T pContext,
                  UINT_T Cmd,
                  UINT_T  Argument,
                  UINT_T BlockType,
                  UINT_T DataDirection,
                  UINT_T ResType)
{
 MM4_CMD_XFRMD_UNION xfrmd;
 P_MM4_STATE pMM4_STATE;

 // Make sure the controller is ready to accept the next command
 pMM4_STATE = (P_MM4_STATE) &pContext->pMMC4Reg->mm4_state;
 while (pMM4_STATE->dcmdinhbt)
 	{;}	// Wait.

 // Set the Argument Field
 pContext->pMMC4Reg->mm4_arg = Argument;
#if 0
 // Set the Data Transfer Command fields.
 xfrmd.mm4_cmd_xfrmd_value = 0;
 xfrmd.mm4_cmd_xfrmd_bits.cmd_idx = Cmd;
 xfrmd.mm4_cmd_xfrmd_bits.cmd_type = MM4_CMD_TYPE_NORMAL;
 xfrmd.mm4_cmd_xfrmd_bits.dpsel = MM4_CMD_DATA;
 xfrmd.mm4_cmd_xfrmd_bits.idxchken = TRUE;
 xfrmd.mm4_cmd_xfrmd_bits.crcchken = TRUE;
 xfrmd.mm4_cmd_xfrmd_bits.res_type = ResType;
 xfrmd.mm4_cmd_xfrmd_bits.ms_blksel = BlockType;
 xfrmd.mm4_cmd_xfrmd_bits.dxfrdir = DataDirection;
 //xfrmd.mm4_cmd_xfrmd_bits.autocmd12 = TRUE;
 //xfrmd.mm4_cmd_xfrmd_bits.blkcbten = TRUE;
 xfrmd.mm4_cmd_xfrmd_bits.autocmd12 = FALSE;
 xfrmd.mm4_cmd_xfrmd_bits.blkcbten  = FALSE;
#endif

	 // Set the Data Transfer Command fields.
	 xfrmd.mm4_cmd_xfrmd_value = 0;
	 xfrmd.mm4_cmd_xfrmd_bits.cmd_idx = Cmd;
	 xfrmd.mm4_cmd_xfrmd_bits.cmd_type = MM4_CMD_TYPE_NORMAL;
	 xfrmd.mm4_cmd_xfrmd_bits.dpsel = MM4_CMD_DATA;
	 xfrmd.mm4_cmd_xfrmd_bits.idxchken = TRUE;
	 xfrmd.mm4_cmd_xfrmd_bits.crcchken = TRUE;
	 xfrmd.mm4_cmd_xfrmd_bits.res_type = ResType;
	 xfrmd.mm4_cmd_xfrmd_bits.ms_blksel = BlockType;
	 xfrmd.mm4_cmd_xfrmd_bits.dxfrdir = DataDirection;
	 xfrmd.mm4_cmd_xfrmd_bits.autocmd12 = FALSE;
	 xfrmd.mm4_cmd_xfrmd_bits.blkcbten = TRUE;

#if SDIO_DMA
	 // enable DMA
	 xfrmd.mm4_cmd_xfrmd_bits.dma_en = 1;
#if 1
	 if (Cmd==25 || Cmd==18 )
	 {  // multiple blocks with Auto STOP command
	 	xfrmd.mm4_cmd_xfrmd_bits.autocmd12 = TRUE;
	// 	xfrmd.mm4_cmd_xfrmd_bits.blkcbten = TRUE;

	 	// Stop at Block Gap request; set bit 0 of 2Ah
	 	//MMC4StopAtBlockGap(pContext);
	 }
#endif

#endif
	EMMC_PRN(PRN_DBG,"\n Sending command: xfrmd = 0x%x ",xfrmd.mm4_cmd_xfrmd_value);
	 // Kick off the command
	 pContext->pMMC4Reg->mm4_cmd_xfrmd = xfrmd.mm4_cmd_xfrmd_value;
	 return;
}

/******************************************************************************
  Description:
    Set up the registers of the controller to start the transaction to
    communicate to the card for data related command.  The commands are clearly defined in the MMC
    specification.
  Input Parameters:
    pContext
      	Pointer to MMC context structure
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
void EMMCHC_SendDataCommandNoAuto12(P_MM4_SDMMC_CONTEXT_T pContext,
                  UINT_T Cmd,
                  UINT_T  Argument,
                  UINT_T BlockType,
                  UINT_T DataDirection,
                  UINT_T ResType)
{
	 MM4_CMD_XFRMD_UNION xfrmd;
	 P_MM4_STATE pMM4_STATE;

	 // Make sure the controller is ready to accept the next command
	 pMM4_STATE = (P_MM4_STATE) &pContext->pMMC4Reg->mm4_state;
	 while (pMM4_STATE->dcmdinhbt)
	 	{;}	// Wait.

	 // Set the Argument Field
	 pContext->pMMC4Reg->mm4_arg = Argument;

	 // Set the Data Transfer Command fields.
	 xfrmd.mm4_cmd_xfrmd_value = 0;
	 xfrmd.mm4_cmd_xfrmd_bits.cmd_idx = Cmd;
	 xfrmd.mm4_cmd_xfrmd_bits.cmd_type = MM4_CMD_TYPE_NORMAL;
	 xfrmd.mm4_cmd_xfrmd_bits.dpsel = MM4_CMD_DATA;
	 // xfrmd.mm4_cmd_xfrmd_bits.idxchken = TRUE;
	 xfrmd.mm4_cmd_xfrmd_bits.crcchken = TRUE;
	 xfrmd.mm4_cmd_xfrmd_bits.res_type = ResType;
	 xfrmd.mm4_cmd_xfrmd_bits.ms_blksel = BlockType;
	 xfrmd.mm4_cmd_xfrmd_bits.dxfrdir = DataDirection;
	 xfrmd.mm4_cmd_xfrmd_bits.autocmd12 = FALSE;
	 xfrmd.mm4_cmd_xfrmd_bits.blkcbten = TRUE;

#if SDIO_DMA
	xfrmd.mm4_cmd_xfrmd_bits.dma_en = TRUE;
#endif

	 EMMC_PRN(PRN_INFO,"\n %s, Sending command: xfrmd = 0x%x ",__func__, xfrmd.mm4_cmd_xfrmd_value);

	 // Kick off the command
	 pContext->pMMC4Reg->mm4_cmd_xfrmd = xfrmd.mm4_cmd_xfrmd_value;
 return;
}


/******************************************************************************
  Description:
    Set up the registers of the controller to start the transaction to
    communicate to the card for setup related commands.
    The commands are clearly defined in the MMC specification.
  Input Parameters:
    pContext
      	Pointer to MMC context structure
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
void EMMCHC_SendSetupCommand(P_MM4_SDMMC_CONTEXT_T pContext,
                  UINT_T Cmd,
                  UINT_T CmdType,
                  UINT_T Argument,
                  UINT_T ResType)
{
	 MM4_CMD_XFRMD_UNION xfrmd;
	 P_MM4_STATE pMM4_STATE;
	 volatile uint32_t i = 0;

	 // Make sure the controller is ready to accept the next command
	 pMM4_STATE = (P_MM4_STATE) &pContext->pMMC4Reg->mm4_state;
	 while (pMM4_STATE->ccmdinhbt)
		{i++; if(i > 100) { break;} mdelay(10);}	// Wait.

	 // Set the Argument Field
	 pContext->pMMC4Reg->mm4_arg = Argument;

	 // Set the Data Transfer Command fields.
	 xfrmd.mm4_cmd_xfrmd_value = 0;
	 xfrmd.mm4_cmd_xfrmd_bits.cmd_idx = Cmd;
	 xfrmd.mm4_cmd_xfrmd_bits.cmd_type = CmdType;
	  if((ResType  & 0x0000ff00) == MM4_RT_R3)
	 {
	 	xfrmd.mm4_cmd_xfrmd_bits.idxchken = FALSE;
	 	xfrmd.mm4_cmd_xfrmd_bits.crcchken = FALSE;
	 }
	 else  if((ResType  & 0x0000ff00) == MM4_RT_R2)
	 {
	 	xfrmd.mm4_cmd_xfrmd_bits.idxchken = FALSE;
	 	xfrmd.mm4_cmd_xfrmd_bits.crcchken = TRUE;
	 }
	 else
	 {
	 	xfrmd.mm4_cmd_xfrmd_bits.idxchken = TRUE;
	 	xfrmd.mm4_cmd_xfrmd_bits.crcchken = TRUE;
	 }
	 xfrmd.mm4_cmd_xfrmd_bits.res_type = ResType;

	 EMMC_PRN(PRN_DBG,"\n Sending command: xfrmd = 0x%x ",xfrmd.mm4_cmd_xfrmd_value);

	 // Kick off the command
	 pContext->pMMC4Reg->mm4_cmd_xfrmd = xfrmd.mm4_cmd_xfrmd_value;
	 return;

}




/****************************************************************
*    EMMCHC_SetControllerVoltage
*		Inspects the Capabilities Register for supported voltage types by the
*		controller. Then programs the CNTL1 register with the desired range.
*		Enables bus power
*    Input:
* 		P_MM4_SDMMC_CONTEXT_T pContext
*    Output:
*		none
*    Returns:
*       none
*****************************************************************/
UINT_T EMMCHC_SetControllerVoltage (P_MM4_SDMMC_CONTEXT_T pContext,  UINT_T vcc)
{
 UINT_T controllervoltage = 0;
 P_MM4_CAP1_2	pMM4_CAP0  = (P_MM4_CAP1_2) &pContext->pMMC4Reg->mm4_cap1_2;
 P_MM4_CNTL1 pMM4_CNTL1 = (P_MM4_CNTL1) &pContext->pMMC4Reg->mm4_cntl1;
 MM4_CNTL1_UNION MM4_cntl1;


	// Capture the Value
	MM4_cntl1.mm4_cntl1_value = *(VUINT_T*) pMM4_CNTL1;
	EMMC_PRN(PRN_DBG,"*pMM4_CNTL1 %x,\n",*pMM4_CNTL1);
	EMMC_PRN(PRN_DBG,"*pMM4_CAP0 is %x, voltage supported bits: %d%d%d\n",
		*pMM4_CAP0, pMM4_CAP0->vlg_33_support,
		pMM4_CAP0->vlg_30_support, pMM4_CAP0->vlg_18_support);

	// Read the CAP0 register
	if (pMM4_CAP0->vlg_33_support && (vcc == 3))
		controllervoltage = MM4_VLTGSEL_3_3;
	else if (pMM4_CAP0->vlg_30_support && (vcc == 4))
		controllervoltage = MM4_VLTGSEL_3_0;
	else if (pMM4_CAP0->vlg_18_support && (vcc == 1))
		controllervoltage = MM4_VLTGSEL_1_8;
	else // default to 3.3V
		controllervoltage = MM4_VLTGSEL_3_3;

	if(MM4_VLTGSEL_1_8 == controllervoltage) {
		volatile P_MM4_ACMD12_ER pMM4_crtl2 = \
			(volatile P_MM4_ACMD12_ER) &pContext->pMMC4Reg->mm4_acmd12_er;
		UINT_T data = *(VUINT_T*) pMM4_crtl2;

		data |= (0x1 << 19);  // sgh_v18_en
		*(VUINT_T*) pMM4_crtl2 = data;
		EMMC_PRN(PRN_RES, "SDHCI: switch host to 1.8V ! \n");

		controllervoltage = 0x6;	//DWCMSH_MMC: 1-1.8V:6 !!!!!
	}

 //EMMC_PRN(PRN_INFO,"controllervoltage: %d\n",controllervoltage);

 // Set the voltage to controller
 MM4_cntl1.mm4_cntl1_bits.vltgsel = controllervoltage;

 // Enable Bus Power
 MM4_cntl1.mm4_cntl1_bits.buspwr = 1;

 // Write back out.
 *(VUINT_T*) pMM4_CNTL1 = MM4_cntl1.mm4_cntl1_value;
EMMC_PRN(PRN_DBG,"*pMM4_CNTL1 %x,\n",*pMM4_CNTL1);

 return controllervoltage;
}

void EMMCHC_EnableCmdInterrupt(P_MM4_SDMMC_CONTEXT_T pContext, unsigned int enable)
{
    //unsigned int data;
	P_MM4_I_STAT_EN pMM4_I_STAT_EN;
	P_MM4_I_SIGN_EN pMM4_I_SIGN_EN;
  	MM4_I_STAT_UNION MM4_i_stat;


  	// Capture existing Value
  	pMM4_I_SIGN_EN = (P_MM4_I_SIGN_EN)((uintptr_t) &pContext->pMMC4Reg->mm4_i_sig_en);
  	pMM4_I_STAT_EN = (P_MM4_I_STAT_EN)((uintptr_t) &pContext->pMMC4Reg->mm4_i_stat_en);

  	MM4_i_stat.mm4_i_stat_value = *(VUINT_T*)pMM4_I_STAT_EN;
  	// Route the interrupt signal enable register
  	MM4_i_stat.mm4_i_stat_bits.cmdcomp = enable&0x01;

  //	EMMC_PRN(PRN_INFO," enable is %x	MM4_i_stat.mm4_i_stat_bits.cmdcomp  : %x\n", enable,MM4_i_stat.mm4_i_stat_bits.cmdcomp);
  	// Write it out
  	*(VUINT_T*)pMM4_I_STAT_EN = MM4_i_stat.mm4_i_stat_value;
  	*(VUINT_T*)pMM4_I_SIGN_EN = MM4_i_stat.mm4_i_stat_value;

}

void EMMCHC_cdInt_enable(P_MM4_SDMMC_CONTEXT_T pContext, UINT8_T Desire)
{
	P_MM4_I_STAT_UNION pMM4_I_STAT_EN_U;
	MM4_I_STAT_UNION  stat_en_copy;


	pMM4_I_STAT_EN_U = (P_MM4_I_STAT_UNION) &pContext->pMMC4Reg->mm4_i_stat_en;
	stat_en_copy.mm4_i_stat_value= pMM4_I_STAT_EN_U->mm4_i_stat_value;

	if(Desire==1)
	{
  		EMMC_PRN(PRN_INFO, "\n  Interrupt Signal enable value is 0x%x",stat_en_copy.mm4_i_stat_value);
  		stat_en_copy.mm4_i_stat_bits.cdint = 1;
		pMM4_I_STAT_EN_U->mm4_i_stat_value = stat_en_copy.mm4_i_stat_value;
		stat_en_copy.mm4_i_stat_value= pMM4_I_STAT_EN_U->mm4_i_stat_value;
  		EMMC_PRN(PRN_INFO, "\n  Interrupt Signal enable value is 0x%x",stat_en_copy.mm4_i_stat_value);
  		EMMC_PRN(PRN_RES, " Card Interrupt is enabled.\n ");
	}
	else
	{
  		EMMC_PRN(PRN_INFO, "\n  Interrupt Signal enable value is 0x%x",stat_en_copy);
  		stat_en_copy.mm4_i_stat_bits.cdint = 0;
		pMM4_I_STAT_EN_U->mm4_i_stat_value = stat_en_copy.mm4_i_stat_value;
		stat_en_copy.mm4_i_stat_value= pMM4_I_STAT_EN_U->mm4_i_stat_value;
  		EMMC_PRN(PRN_INFO, "\n  Interrupt Signal enable value is 0x%x",stat_en_copy.mm4_i_stat_value);
		EMMC_PRN(PRN_RES, " Card Interrupt is disabled.\n ");
	}
  return;
}

