/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright (C) 2013-2021 Synaptics Incorporated. All rights reserved.
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

#ifndef DW_MMC_H
#define DW_MMC_H

#include "Galois_memmap.h"
#include "mmc.h"
#include "apb_watchdog.h"

/* define the timing function for emmc */

#define mmc_writeb(v, a)	writeb((v), (a + dwcmshc_params.reg_base))
#define mmc_writew(v, a)	writew((v), (a + dwcmshc_params.reg_base))
#define mmc_writel(v, a)	writel((v), (a + dwcmshc_params.reg_base))
#define mmc_writex(v, a)	writex((v), (a + dwcmshc_params.reg_base))

#define	mmc_readb(a)	readb((a + dwcmshc_params.reg_base))
#define	mmc_readw(a)	readw((a + dwcmshc_params.reg_base))
#define	mmc_readl(a)	readl((a + dwcmshc_params.reg_base))
#define	mmc_readx(a)	readx((a + dwcmshc_params.reg_base))

typedef unsigned int __u32;

#define ___swab32(x) \
	((__u32)( \
		(((__u32)(x) & (__u32)0x000000ffUL) << 24) | \
		(((__u32)(x) & (__u32)0x0000ff00UL) <<  8) | \
		(((__u32)(x) & (__u32)0x00ff0000UL) >>  8) | \
		(((__u32)(x) & (__u32)0xff000000UL) >> 24) ))

#define __be32_to_cpu(x) ___swab32((__u32)(x))

#define SDCLK_SEL_DIV_1024   0x200
#define SDCLK_SEL_DIV_512    0x100
#define SDCLK_SEL_DIV_256    0x80
#define SDCLK_SEL_DIV_128    0x40
#define SDCLK_SEL_DIV_64     0x20
#define SDCLK_SEL_DIV_32     0x10
#define SDCLK_SEL_DIV_16     0x08
#define SDCLK_SEL_DIV_8      0x04
#define SDCLK_SEL_DIV_4      0x02
#define SDCLK_SEL_DIV_2      0x01
#define SDCLK_SEL_DIV_1      0x00

#define EMMC_BUS_WIDTH		MMC_BUS_WIDTH_8
#define EMMC_HOSTMODE		EMMC_HOSTMODE_HSSDR

/*PHY PAD GENERAL modes */
#define PAD_SP_8    0x8
#define PAD_SP_9    0x9
#define PAD_SN_8    0x8
#define PAD_SN_9    0x9

typedef struct {
	unsigned int addr;
	unsigned int sp_bit;
	unsigned int sn_bit;
	unsigned int mask;
	unsigned int sp_value;
	unsigned int sn_value;
} PHY_PAD_GENERAL;

/* PHY RX SEL modes */
#define RXSELOFF        0x0
#define SCHMITT1P8      0x1
#define SCHMITT3P3      0x2
#define SCHMITT1P2      0x3
#define COMPARATOR1P8   0x4
#define COMPARATOR1P2   0x5
#define COMPARATOR1P82  0x6
#define INTERNALLPBK    0x7
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

typedef struct dwcmshc_mmc_params {
	uintptr_t	reg_base;
	uintptr_t	desc_base;
	size_t		desc_size;
	int		clk_rate;
	int		bus_width;
	unsigned int	flags;
	enum mmc_device_type	mmc_dev_type;
} dwcmshc_mmc_params_t;

typedef struct
{
  unsigned int pBuffer[4];
  volatile unsigned int CommandComplete;
  unsigned int SendStopCommand;
  unsigned int R1_RESP;
} mmc_response;

typedef enum
{
/*cmdcomp	*/ CMD_COMP=0,
/*xfrcomp      	*/ XFR_COMP,
/*bgevnt       	*/ BGEVENT,
/*dmaint       	*/ DMA_INT,
/*bufwrrdy     	*/ BUFWR_RDY,
/*bufrdrdy     	*/ BUFRD_RDY,
/*cdins	        */ CD_INSERT,
/*cdrem	        */ CD_REMOVE,
/*cdint	        */ CD_INT,
/*int_a	        */ INT_A,
/*int_b	        */ INT_B,
/*int_c	        */ INT_C,
/*retuninig_int	*/ RETUNE,
/*FX_EVENT    	*/ FX_EVENT,
/*CQE_EVENT    	*/ CQE_EVENT,
/*errint       	*/ ERROR_INT,
/*ctoerr       	*/ CMD_TO_ERR,
/*ccrcerr      	*/ CMD_CRC_ERR,
/*cenderr      	*/ CMD_END_ERR,
/*cidxerr      	*/ CMD_IDX_ERR,
/*dtoerr       	*/ DATA_TO_ERR,
/*dcrcerr      	*/ DATA_CRC_ERR,
/*denderr      	*/ DATA_END_ERR,
/*ilmterr      	*/ ILL_LIMIT_ERR,
/*ac12err      	*/ ACMD12_ERR,
/*admaerr      	*/ ADMA_ERR,
/*tune_err     	*/ TUNE_ERR,
/*RESP_ERR    	*/ RESP_ERR,
/*BOOTACK_ERR  	*/ BOOTACK_ERR,
/*vendor_err1 	*/ VENDOR_ERR1,
/*vendor_err2 	*/ VENDOR_ERR2,
/*vendor_err3 	*/ VENDOR_ERR3
} INT_SOURCE_INDEX;



void dwcmshc_mmc_init(dwcmshc_mmc_params_t *params, struct mmc_device_info *info);

#endif /* DW_MMC_H */
