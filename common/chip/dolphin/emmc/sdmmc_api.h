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
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//
// (C) Copyright 2006 Marvell International Ltd.
// All Rights Reserved

#ifndef __SDMMC_API_H__
#define __SDMMC_API_H__

#define SDIO_BLK_SIZE    512

typedef volatile unsigned int 	VUINT32_T;
typedef unsigned int 		   	UINT32_T;
typedef volatile unsigned int  	VUINT_T;
typedef unsigned int 		   	UINT_T;
typedef int			 		   	INT_T;
typedef unsigned short 		   	UINT16_T, USHORT;
typedef volatile unsigned short VUINT16_T;
typedef unsigned char 			UINT8_T;
typedef char		 			INT8_T;

// DMA or PIO mode
#define SDIO_DMA	1
#ifndef	DISABLE_EMMC_INT_MODE
#define __SDIO_INT_MODE__
#endif //DISABLE_EMMC_INT_MODE

/* define the timing function for sd/emmc */
#define VELOCE      0
#define FPGA        1
#define ASIC        2

#define PLATFORM	ASIC

#ifndef	PLATFORM
#define PLATFORM	FPGA
#endif

#if PLATFORM == VELOCE
#define sd_delay(n)		udelay(n)
#elif PLATFORM == FPGA
#define sd_delay(n)		udelay(100*n)
#else // ASIC
#define sd_delay(n)		mdelay(n)
#endif


#define GET_P_UINT(P)	((unsigned int*)P)

#define SDIO_RD_BUF		0xA00000
#define SDIO_WD_BUF		0xB00000
#define SDIO_ADMA_RXDESC_BASE 0xC00000
#define SDIO_ADMA_TXDESC_BASE 0xC80000
#define	SDIO_BOOT_BUF	0xA00000
#define SDIO_64K_BOUNDARY 0x10000
#define SDIO_CARD_PRESENT_BIT 0x40000


/**
 * General error code definitions			0x0 - 0x1F
 **/
#define NoError            					0x0
#define NotFoundError      					0x1
#define GeneralError       					0x2
#define WriteError         					0x3
#define ReadError		   					0x4
#define NotSupportedError  					0x5
#define InvalidPlatformConfigError			0x6
#define PlatformBusy						0x7
#define PlatformReady						0x8
#define InvalidSizeError					0x9

// Flash Related Errors 					0x20 - 0x3F
#define EraseError		 					0x20
#define ProgramError						0x21
#define InvalidBootTypeError				0x22
#define ProtectionRegProgramError			0x23
#define NoOTPFound							0x24
#define BBTReadError						0x25
#define MDOCInitFailed						0x26
#define OneNandInitFailed                   0x27
#define MDOCFormatFailed                    0x28
#define BBTExhaustedError                   0x29
#define FlashDriverInitError                0x30
#define FlashFuncNotDefined					0x31
#define OTPError							0x32
#define InvalidAddressRangeError			0x33
#define FlashLockError						0x34
#define ReadDisturbError					0x35

#define TimeOutError						0x96


#define SdioCardResponseR5Error				0x99

// SDMMC Errors 							0xD0-0xE2
#define SDMMC_SWITCH_ERROR	 				0xD0
#define SDMMC_ERASE_RESET_ERROR	 			0xD1
#define SDMMC_CIDCSD_OVERWRITE_ERROR		0xD2
#define SDMMC_OVERRUN_ERROR 				0xD3
#define SDMMC_UNDERUN_ERROR 				0xD4
#define SDMMC_GENERAL_ERROR 				0xD5
#define SDMMC_CC_ERROR 						0xD6
#define SDMMC_ECC_ERROR 					0xD7
#define SDMMC_ILL_CMD_ERROR 				0xD8
#define SDMMC_COM_CRC_ERROR					0xD9
#define SDMMC_LOCK_ULOCK_ERRROR				0xDA
#define SDMMC_LOCK_ERROR 					0xDB
#define SDMMC_WP_ERROR 						0xDC
#define SDMMC_ERASE_PARAM_ERROR				0xDD
#define SDMMC_ERASE_SEQ_ERROR				0xDE
#define SDMMC_BLK_LEN_ERROR 				0xDF
#define SDMMC_ADDR_MISALIGN_ERROR 			0xE0
#define SDMMC_ADDR_RANGE_ERROR 				0xE1
#define SDMMCDeviceNotReadyError			0xE2
#define SDMMCInitializationError			0xE3
#define SDMMCDeviceVoltageNotSupported		0xE4

// Globals
#define ENABLE_PARTITION_SUPPORT	0

#define OSCR_OFFSET         		0x10
#define BOOL                		int
#define OCR_ARG             		0x80FF8000
#define SD_OCR_ARG          		0x40FF8000
#define OCR_ACCESS_MODE_MASK		0x60000000

#define RESPONSE_LENGTH     		8
#define R2_BYTE_LENGTH				16
#define MMC_BUSY_BIT        		0x80000000
#define SD_NO_CMD1          		0x300
#define LOW_ARG             		0x0000ffff
#define NO_FLAGS            		0
#define BLK_CNT             		1
#define BLK_LEN             		512
#define	SDVHS_2_7_TO_3_6	 		0x1
#define	SDVHS_LOW_VOLT		 		0x2
#define	SDVHSARGSHIFT				8
#define SDVHSCHECKPATTERN			0x33
#define HARD512BLOCKLENGTH			512
#define PARTITIONMASK				0xF
#define MMC_SD_BOOT_PARTITION		1
#define MMC_SD_BOOT_PARTITION2		2
#define MMC_SD_USER_PARTITION		0
#define SCRSD1BITMODE 				0x1
#define SCRSD4BITMODE 				0x5
#define SD_CMD6_4BITMODE 			2
#define R1_LOCKEDCARDMASK			0xFDFFFFFF
#define R1_NOMASK					0xFFFFFFFF
#define URGENT_BKOPS				0x00000040

// MMC Controller selections
typedef enum
{
	MMCNOTENABLED		 =  0,
	MMCLEGACYCONTROLLER1 =	1,
	MMCLEGACYCONTROLLER2 =	2,
	MM4CONTROLLER0 		 =	3,
	MM4CONTROLLER1 		 =	4,
	MM4CONTROLLER2 		 =	5,
}CONTROLLER_TYPE;

enum
{
	XLLP_MMC = 0,
	XLLP_SD =1,
	XLLP_eSD = 2
};

typedef	enum
{
    XLLP_MMC_CMD0               = 0x0,	// go idle state
    XLLP_MMC_CMD1               = 0x1,	// send op command
    XLLP_MMC_CMD2               = 0x2,	// all send cid
    XLLP_MMC_CMD3               = 0x3,	// set relative addr
    XLLP_MMC_CMD4               = 0x4,	// set dsr
    XLLP_MMC_CMD5               = 0x5,  // SLEEP/AWAKE
    XLLP_MMC_CMD7               = 0x7,	 // select/deselect card
    XLLP_SD_CMD6				= 0x6, 	 // SD Switch Function Command
    XLLP_MMC_CMD6				= 0x6, 	// MMC Switch Function Command
	XLLP_SD_ACMD6				= 0x6,	// SD ACMD Command for SET_BUS_WIDTH
    XLLP_SD_CMD8               	= 0x8,	 // SD Card Interface Condition
    XLLP_MMC_CMD8				= 0x8,	// MMC request to read EXT CSD
    XLLP_MMC_CMD9             	= 0x9,	// send csd
    XLLP_MMC_CMD10            	= 0xa,	// send cid
    XLLP_MMC_CMD11              = 0xb,	//  read data until stop
    XLLP_MMC_CMD12              = 0xc,	 // stop transmission
    XLLP_MMC_CMD13              = 0xd,	 // send status
    XLLP_MMC_CMD15              = 0xf,	// go inactive state
    XLLP_MMC_CMD16              = 0x10,	//  set block length
    XLLP_MMC_CMD17              = 0x11,	// read single block
    XLLP_MMC_CMD18              = 0x12,	//  read multiple block
    XLLP_SD_CMD19				= 0x13,	//	UHSI Tuning
    XLLP_MMC_CMD20              = 0x14,	 // write data until stop
    XLLP_MMC_CMD21              = 0x15,	 // eMMC UHSI Tuning
	XLLP_MMC_CMD23            	= 0x17,	 // Set Block Count
    XLLP_MMC_CMD24            	= 0x18,	 // write block
    XLLP_MMC_CMD25              = 0x19,	 // write multiple block
    XLLP_MMC_CMD26              = 0x1a,	// program CID
    XLLP_MMC_CMD27            	= 0x1b,	// program CSD
    XLLP_MMC_CMD28            	= 0x1c,	// set write prot
    XLLP_MMC_CMD29            	= 0x1d,  // clr write prot
    XLLP_MMC_CMD30            	= 0x1e,	// send write prot
    XLLP_SD_CMD32            	= 0x20,	 // tag sector start
    XLLP_SD_CMD33            	= 0x21,	// tag sector end
    XLLP_SD_CMD38            	= 0x26,	 // erase
    XLLP_MMC_CMD34            	= 0x22,	// untag sector
    XLLP_MMC_CMD35            	= 0x23,	// tag erase group start
    XLLP_MMC_CMD36            	= 0x24,	//  tag erase group end
    XLLP_MMC_CMD37            	= 0x25,	 // untag erase group
	XLLP_eSD_CMD37				= 0x25,  // SD PartitionManagement group Command
    XLLP_eSD_CMD57				= 0x39,
    XLLP_MMC_CMD38            	= 0x26,	 // erase
    XLLP_MMC_CMD39              = 0x27,	//fast IO
    XLLP_MMC_CMD40              = 0x28,	// go irq state
    XLLP_MMC_CMD42            	= 0x2a,	// lock-unlock
    XLLP_eSD_CMD43				= 0x2b, // Select Partition
    XLLP_SD_CMD55              	= 0x37,	// app cmd
    XLLP_MMC_CMD56            	= 0x38,	//  gen cmd
    XLLP_SPI_CMD58              = 0x3a,	 // read ocr
    XLLP_SPI_CMD59              = 0x3b,	 // crc on-off
    XLLP_SD_ACMD41	            = 0x29,
    XLLP_SD_ACMD13      	    = 0x0d,	  // Read SD Status
/* #ifndef IO_CARD_SUPPORT
    XLLP_SD_ACMD51      	    = 0x33	  // Read SD Configuration Register (SCR)
#else*/
    XLLP_SD_ACMD51      	    = 0x33,	  // Read SD Configuration Register (SCR)
    XLLP_SDIO_CMD5				= 0x05,	//Satya
    XLLP_SDIO_CMD52				= 0x34,
    XLLP_SDIO_CMD53				= 0x35
//#endif
} XLLP_MMC_CMD;

typedef struct
{
	UINT_T		CID_VALUE[4];
	UINT_T		SerialNum;
} CID_LAYOUT;

typedef struct
{
	UINT_T		CSD_VALUE[4];
} CSD_LAYOUT;

typedef struct
{
	UINT_T		SD_VALUE[4];
} SD_LAYOUT;

typedef struct
{
	UINT_T		SCR_VALUE[2];
} SCR_LAYOUT;

// CID: card identification(128 bits = 16 bytes)
typedef struct      // check whether there is any word alligment issue.
{
    unsigned char   manufactureID;
    char            OEM[3];
    char            name[7];            // SD: 5 byte string, MMC: 6 byte string
    unsigned char   revision;
    unsigned int   serialNumber;
    unsigned short  manufactureDate;    // SD: 12 bit code, MMC: 8 bit code.

} SDIO_CID;

// CSD
typedef struct
{
    unsigned char   structure;					// [127:126] for eMMC and SD
    unsigned char   specificationVersion;
    unsigned char   dataReadAccessTime2;
    unsigned char   dataReadAccessTime1;
    unsigned char   maxDataTransferRate;
    unsigned short  cardCommandClass;
    unsigned char   readBlockLength;
    unsigned char   partialReadFlag;
    unsigned char   writeBlockMisalign;
    unsigned char   readBlockMisalign;
    unsigned char   DSR_implemented;
    unsigned short  deviceSize;
    unsigned char   maxreadCurrent_VddMin;
    unsigned char   maxReadCurrent_VddMax;
    unsigned char   maxWriteCurrent_VddMin;
    unsigned char   maxWriteCurrent_VddMax;
    unsigned char   deviceSizeMultiplier;
    unsigned char   eraseBlockEnable;           //SD only
    unsigned char   eraseSectorSize;            //SD:7 bits, MMC: 5 bits
    unsigned char   eraseGroupSize;             //MMC: only
    unsigned char   WP_GroupSize;               //SD:7 bits, MMC: 5 bits
    unsigned char   WP_GroupEnable;
    unsigned char   writeSpeedFactor;
    unsigned char   writeBlockLength;
    unsigned char   partialWriteFlag;
    unsigned char   fileFormatGroup;
    unsigned char   copyFlag;
    unsigned char   WP_perm;
    unsigned char   WP_temp;
    unsigned char   fileFormat;

} SDIO_CSD;

#define PARTITION_MANAGEMENT_CMD_SET 	0xFFFF2F
#define DEFAULT_CMD_SET					0xFFFFFF
#define CHECK_FUNCTION_MODE 			0
#define SWITCH_FUNCTION_MODE			1
#define PARTITION_MANAGEMENT_FUNCTION 	2
#define MMC_ALTERNATE_BOOT_ARGUMENT 	0xFFFFFFFA
#define MMC_CMD0_PRE_IDLE_ARGUMENT		0xF0F0F0F0


typedef enum
{
	UNINITIALIZED,			// Controller and Card are uninitialized
	INITIALIZE,				// Controller and Card are being ninitialized
	READ,					// Multiple Block Read State
	ERASE,					// Erase State
	WRITE,					// Multiple Block Write State
	READY,					// The Card is ready for Data Transfer
	DATATRAN,				// The controller has finished data transfer but card may be busy
	FAULT					// Fault
} SDMMC_IO_TRANSFERS;

// Vital information used in Data Transfers by the ISR and driver routines.
typedef struct
{
	long long				CardAddress;		// Starting Card Address
	UINT_T					TransWordSize; 		// Total Number of Bytes involved in this transaction
	UINT_T					NumBlocks; 			// Total Number of Blocks involved in this transaction
	UINT_T					LocalAddr;			// Destination Address Pointer for reads, and source addr for writes
	UINT_T 					StartDiscardWords;	// Words - from the first block that caller doesn't want
	UINT_T					EndDiscardWords;	// Words - from the last block that caller doesn't want
	UINT_T					WordIndex;			// Words - Word index to the progress in this transfer request
	UINT8_T					Cmd;				// The command that started the transaction
	UINT8_T					ACmdFlag;			// Indicates if the executing command is standard vs. application specific (ACMD)
	UINT8_T					AutoCMD12Mode;		// When "true", the controller is operating in auto command 12 mode.
	UINT8_T					RespType;			// Info about the response type and potential for busy state.
} SDMMC_TRANSFER;

typedef struct
{
	UINT_T					RCA;			// RCA of the card
  	UINT_T					OCR;			// OCR Register Contents
	CID_LAYOUT				CID;			// CID Register
  	CSD_LAYOUT			  	CSD;			// CSD Register Contents
  	SCR_LAYOUT				SCR;			// SCR Register Contents
	SD_LAYOUT               SD;             // SD status
}	SDMMC_CARD_REGISTERS;

#define SD_SPEC_MASK	0x0F000000
#define SD_SPEC_OFFSET	24


typedef struct
{
  UINT_T pBuffer[4];							// Buffer to read values in to
  volatile UINT_T CommandComplete;
// TBD  UINT_T CommandError;
  UINT_T SendStopCommand;					// If some read/write transactions require stop command
  UINT_T R1_RESP;							// Capture the R1 Response of the Card for most commands
} SDMMC_RESPONSE;

typedef enum
{
	BYTE_ACCESS = 0,
	SECTOR_ACCESS = 1
}ACCESS_MODE;

// This Properties structure is shared between MM4 and legacy type drivers. The pContext field
// is specific to the variant.

typedef struct
{
	CONTROLLER_TYPE			ControllerType;		// See CONTROLLER_TYPE platformconfig.h
	void				 	*pContext; 			// Pointer to MMC control registers
	UINT8_T					SD; 				// Indicates if the card is SD, eSD or MMC
  	UINT8_T					Slot; 				// Indicates which slot used
	SDMMC_CARD_REGISTERS	CardReg;			// Card Registers
	SDMMC_RESPONSE			CardReponse;		// Card Response Related
	UINT_T 					SD_VHS; 			// SD Voltage Acceptance Return Value (SD only)
	volatile SDMMC_IO_TRANSFERS	State;			// Indicate State of the card
	ACCESS_MODE				AccessMode;			// High Density Card
	SDMMC_TRANSFER			Trans;				// Transfer State of the Card
	UINT_T					ReadBlockSize;		// Bytes - Block Size Used for Reads
	UINT_T					WriteBlockSize;		// Bytes - Block Size Used for Writes
	UINT_T					EraseSize;			// Bytes - Minimum Size of an erasable unit
	UINT_T					CardCapacity; 		// Bytes - Maximum Capacity of the card
	UINT_T					SDMA_Mode; 			// Enable SDMA Mode or Not.
	UINT_T					StrictErrorCheck;	// Relax error checking during card init. Not all cards are strictly compliant.
}
SDMMC_Properties_T, *P_SDMMC_Properties_T;


//#ifdef IO_CARD_SUPPORT
//Satya
typedef struct SDIO_Properties_s
{
	UINT8_T		uchCardReadyAfterInit:1;
	UINT8_T 	uchNoOfIoFunc:3;
	UINT8_T		uchMemPresent:1;
	UINT8_T		uchStuffBits:3;
//	UINT32_T	uchIoOcer:24;
} __attribute__((packed)) SDIO_Properties_T;




typedef struct SDIO_CMD52_Arg_s
{
	UINT_T	uchWriteData:8;
	UINT_T	uchStuffBit1:1;
	UINT_T	uiRegAddr:17;
	UINT_T	uchStuffBit:1;
	UINT_T 	uchRawFlag:1;
	UINT_T	uchFuncNum:3;
	UINT_T 	uchRwFlag:1;
} __attribute__((packed)) SDIO_CMD52_Arg_T;

typedef union SDIO_CMD52_Arg_Union
{
	SDIO_CMD52_Arg_T stCmd52Arg;
	UINT32_T uiCmd52Arg;
}SDIO_CMD52_Arg_Union_T;

typedef struct SDIO_CMD53_Arg_s
{
	UINT_T	uchByte_Blk_cnt:9;
	UINT_T	uiRegAddr:17;
	UINT_T	uchOPCode:1;
	UINT_T	uchBlkMode:1;
	UINT_T	uchFuncNum:3;
	UINT_T 	uchRwFlag:1;
} __attribute__((packed)) SDIO_CMD53_Arg_T;

typedef union SDIO_CMD53_Arg_Union
{
	SDIO_CMD53_Arg_T stCmd53Arg;
	UINT32_T uiCmd53Arg;
}SDIO_CMD53_Arg_Union_T;


//#endif
/* Bit Position Macros */
#define XLLP_BIT_0    ( 1u << 0 )
#define XLLP_BIT_1    ( 1u << 1 )
#define XLLP_BIT_2    ( 1u << 2 )
#define XLLP_BIT_3    ( 1u << 3 )
#define XLLP_BIT_4    ( 1u << 4 )
#define XLLP_BIT_5    ( 1u << 5 )
#define XLLP_BIT_6    ( 1u << 6 )
#define XLLP_BIT_7    ( 1u << 7 )
#define XLLP_BIT_8    ( 1u << 8 )
#define XLLP_BIT_9    ( 1u << 9 )
#define XLLP_BIT_10   ( 1u << 10 )
#define XLLP_BIT_11   ( 1u << 11 )
#define XLLP_BIT_12   ( 1u << 12 )
#define XLLP_BIT_13   ( 1u << 13 )
#define XLLP_BIT_14   ( 1u << 14 )
#define XLLP_BIT_15   ( 1u << 15 )
#define XLLP_BIT_16   ( 1u << 16 )
#define XLLP_BIT_17   ( 1u << 17 )
#define XLLP_BIT_18   ( 1u << 18 )
#define XLLP_BIT_19   ( 1u << 19 )
#define XLLP_BIT_20   ( 1u << 20 )
#define XLLP_BIT_21   ( 1u << 21 )
#define XLLP_BIT_22   ( 1u << 22 )
#define XLLP_BIT_23   ( 1u << 23 )
#define XLLP_BIT_24   ( 1u << 24 )
#define XLLP_BIT_25   ( 1u << 25 )
#define XLLP_BIT_26   ( 1u << 26 )
#define XLLP_BIT_27   ( 1u << 27 )
#define XLLP_BIT_28   ( 1u << 28 )
#define XLLP_BIT_29   ( 1u << 29 )
#define XLLP_BIT_30   ( 1u << 30 )
#define XLLP_BIT_31   ( 1u << 31 )


/*	SD_ERROR_INT_STATUS				0x0032	Error Interrupt Status Register */
/* CRC Status Error */
#define	SD_ERROR_INT_STATUS_CRC_STATUS_ERR			XLLP_BIT_15
/* Command Completion Signal Timeout Error */
#define	SD_ERROR_INT_STATUS_CPL_TIMEOUT_ERR			XLLP_BIT_14
/* AXI Bus Response Error */
#define	SD_ERROR_INT_STATUS_AXI_RESP_ERR			XLLP_BIT_13
/* SPI Mode Error */
#define	SD_ERROR_INT_STATUS_SPI_ERR					XLLP_BIT_12
/*		Bit(s) SD_ERROR_INT_STATUS_RSRV_11_10 reserved */
#define	SD_ERROR_INT_STATUS_ADMA_ERR				XLLP_BIT_9			/* ADMA Error */
/* Auto CMD12 Error */
#define	SD_ERROR_INT_STATUS_AUTO_CMD12_ERR			XLLP_BIT_8
/* Current Limit Error */
#define	SD_ERROR_INT_STATUS_CUR_LIMIT_ERR			XLLP_BIT_7
/* ReadDataEnd Bit Error */
#define	SD_ERROR_INT_STATUS_RD_DATA_END_BIT_ERR		XLLP_BIT_6
/* Read Data CRC Error */
#define	SD_ERROR_INT_STATUS_RD_DATA_CRC_ERR			XLLP_BIT_5
/* Data Timeout Error */
#define	SD_ERROR_INT_STATUS_DATA_TIMEOUT_ERR		XLLP_BIT_4
/* Command Index Error */
#define	SD_ERROR_INT_STATUS_CMD_INDEX_ERR			XLLP_BIT_3
/* Command End Bit Error */
#define	SD_ERROR_INT_STATUS_CMD_END_BIT_ERR			XLLP_BIT_2
/* Command CRC Error */
#define	SD_ERROR_INT_STATUS_CMD_CRC_ERR				XLLP_BIT_1
/* Command Timeout Error */
#define	SD_ERROR_INT_STATUS_CMD_TIMEOUT_ERR			XLLP_BIT_0





/*********************************************************************
*	SDMMC R1 Response Bits
**********************************************************************/
typedef enum
{
	R1_SWITCH_ERROR	 			= XLLP_BIT_0,
	R1_ERASE_RESET_ERROR	 	= XLLP_BIT_13,
	R1_CIDCSD_OVERWRITE_ERROR	= XLLP_BIT_16,
	R1_OVERRUN_ERROR 			= XLLP_BIT_17,
	R1_UNDERUN_ERROR 			= XLLP_BIT_18,
	R1_GENERAL_ERROR 			= XLLP_BIT_19,
	R1_CC_ERROR 				= XLLP_BIT_20,
	R1_ECC_ERROR 				= XLLP_BIT_21,
	R1_ILL_CMD_ERROR 			= XLLP_BIT_22,
	R1_COM_CRC_ERROR			= XLLP_BIT_23,
	R1_LOCK_ULOCK_ERRROR		= XLLP_BIT_24,
	R1_LOCK_ERROR 				= XLLP_BIT_25,
	R1_WP_ERROR 				= XLLP_BIT_26,
	R1_ERASE_PARAM_ERROR		= XLLP_BIT_27,
	R1_ERASE_SEQ_ERROR			= XLLP_BIT_28,
	R1_BLK_LEN_ERROR 			= XLLP_BIT_29,
	R1_ADDR_MISALIGN_ERROR 		= XLLP_BIT_30,
	R1_ADDR_RANGE_ERROR 		= XLLP_BIT_31
} R1_SDMMC_RESPONSE_CODES;


// SDHC 3.1 controller registers definition
typedef struct
{
  VUINT32_T mm4_sysaddr;                  		// DMA system address				 	0x0
  VUINT32_T mm4_blk_cntl;                       // Block size control register		 		0x4
  VUINT32_T mm4_arg;                      		// Command argument					0x8
  VUINT32_T mm4_cmd_xfrmd;                      // Command and transfer mode		 	0xC
  VUINT32_T mm4_resp0;                          // cmd response 0					 		0x10
  VUINT32_T mm4_resp1;                          // cmd response 1					 		0x14
  VUINT32_T mm4_resp2;                          // cmd response 2					 		0x18
  VUINT32_T mm4_resp3;                          // cmd response 3					 		0x1C
  VUINT32_T mm4_dp;                             // buffer data port					 		0x20
  VUINT32_T mm4_state;                          // mm4 state						 		0x24
  VUINT32_T mm4_cntl1;                          // host control 1					 		0x28
  VUINT32_T mm4_cntl2;                          // host control 2					 		0x2C
  VUINT32_T mm4_i_stat;                         // status of current command 		 		0x30
  VUINT32_T mm4_i_stat_en;                      // interrupt status enable			 		0x34
  VUINT32_T mm4_i_sig_en;                       // interrupt signal enable			 		0x38
  VUINT32_T mm4_acmd12_er;                      // auto cmd12 error status			 		0x3C
  VUINT32_T mm4_cap1_2;                         // capabilities 1,2					 		0x40
  VUINT32_T mm4_cap3_4;                         // capabilities 3,4							0x44
  VUINT32_T mm4_sd_max_current1_2;              // max current capabilities					0x48
  VUINT32_T mm4_sd_max_current3_4;              // max current capabilities					0x4C
  VUINT32_T mm4_force_event;              		// force event for AutoCMD12 Error Status	0x50
  VUINT32_T mm4_adma_err_stat;                  // ADMA Error Status						0x54
  VUINT32_T mm4_adma_system_address[2];      	// ADMA Address 63:0						0x58
  VUINT32_T mm4_preset_value_for_init_ds; 		//											0x60
  VUINT32_T mm4_preset_value_for_hs_sdr12;		//											0x64
  VUINT32_T mm4_preset_value_for_hs_sdr25_50;	//											0x68
  VUINT32_T mm4_preset_value_for_hs_sdr104_50;	//											0x6C
  VUINT32_T reserved_X1[28];                    // reserved fields							0x70
  VUINT32_T mm4_shared_bus_control;        		// Shared Bus Control Register				0xE0
  VUINT32_T reserved_X2[6];                   	// reserved									0xE4
  VUINT32_T mm4_sd_slot_int_stat_ctrl_ver;      // SD Interrupt Line and Version Support	0xFC
  VUINT32_T mm4_sdhc_ipid;				// SDHC IP Core ID Register				0x100
  VUINT32_T mm4_sdhc_cfg_info;					// SDHC configuration information register				0x104
  VUINT32_T mm4_sdhc_op_ctrl;		// SDHC operation control register						0x108
  VUINT32_T mm4_sdhc_ext_op_ctrl;					// SDHC extend operation control register				0x10C
  VUINT32_T mm4_sdhc_test_out;				// SDHC test output register						0x110
  VUINT32_T mm4_sdhc_testout_mux;				// SDHC test output mux select register			0x114
  VUINT32_T reserved_X3[2];                   	// r									0x118
  VUINT32_T mm4_sdhc_slot_ext_int;				// SDHC slot extend interrupt/error status register			0x120
  VUINT32_T mm4_sdhc_slot_ext_int_en;				// SDHC slot extend interrupt/error status enable register			0x124
  VUINT32_T mm4_sdhc_slot_op_ctrl;				// SDHC slot operation control and status register			0x128
  VUINT32_T mm4_sdhc_slot_fifo;				// SDHC slot FIFO register			0x12c
  VUINT32_T mm4_sdhc_slot_emmc_ctrl;				// SDHC slot emmc control register			0x130
  VUINT32_T mm4_sdhc_slot_output_dly_ctrl;				// SDHC slot output delay  control register			0x134
  VUINT32_T mm4_sdhc_slot_dll_ctrl;				// SDHC slot dll  control register			0x138
  VUINT32_T mm4_sdhc_slot_dll_phase_sel;				// SDHC slot dll phase sel register			0x13c
  VUINT32_T mm4_sdhc_slot_strobe_dly_ctrl;				// SDHC slot strobe delay control register			0x140
  VUINT32_T mm4_sdhc_slot_retuning_req_ctrl;				// SDHC slot retuning request  control register			0x144
  VUINT32_T mm4_sdhc_slot_auto_retuning_ctrl;				// SDHC slot auto retuning  control register			0x148
  VUINT32_T mm4_sdhc_slot_ext_pres_state;				// SDHC slot extend present state register			0x14c
  VUINT32_T mm4_sdhc_slot_dll_cur_dly;				// SDHC slot dll current delay register			0x150
  VUINT32_T mm4_sdhc_slot_tuning_cur_dly;				// SDHC slot tuning current delay register			0x154
  VUINT32_T mm4_sdhc_slot_strobe_cur_dly;				// SDHC slot strobe current delay register			0x158
  VUINT32_T mm4_sdhc_slot_sub_cmd_ctrl;				// SDHC slot sub command control register			0x15c
} MM4_SDMMC_T, *P_MM4_SDMMC_T;

/*************************** Register Masks ********************************/
/**************************************************************************/
// ******************** MM4_BLK_CNTL 0x4**********************************
typedef struct
{
 unsigned int xfr_blksz       : 12;       // Transfer Block Size
 unsigned int dma_bufsz       : 3;        // Host DMA buffer size
 unsigned int reserved		  : 1;		  // ZL-- here should be one reservred bit
 unsigned int blk_cnt         : 16;       // Block Count for Current Transfer
} MM4_BLK_CNTL, *P_MM4_BLK_CNTL;

#define MM4_512_HOST_DMA_BDRY  0x7


// ********************* MM4_CMD_XFRMD 0xc********************************
//
typedef struct
{

unsigned int dma_en 		 : 1;		 // DMA enable						   0
unsigned int blkcbten		 : 1;		 // Block Count Enable				   1
unsigned int autocmd12		 : 2;		 // AutoCMD12						   2
//	xiufeng	From APSE's code there is one reserved1 bit here because
//	autocmd12 is 1 bit
//	unsigned int reserved1		 : 1;		 // 								   3
unsigned int dxfrdir		 : 1;		 // Data Transfer Direction Select	   4
unsigned int ms_blksel		 : 1;		 // Multi Block Select				   5
unsigned int reserved2		 : 10;		  //								   6
unsigned int res_type		 : 2;		 // Response Type					   16
unsigned int reserved3		 : 1;		 // 								   18
unsigned int crcchken		 : 1;		 // CRC check enable				   19
unsigned int idxchken		 : 1;		 // Command Index Check Enable		   20
unsigned int dpsel			 : 1;		 // Data present   select			   	   21
unsigned int cmd_type		 : 2;		 // Cmd Type						   22
unsigned int cmd_idx		 : 6;		 // Cmd Index						   24
unsigned int reserved4		 : 2;		 // 								   30

} MM4_CMD_XFRMD, *P_MM4_CMD_XFRMD;

typedef union
{
	MM4_CMD_XFRMD mm4_cmd_xfrmd_bits;
	unsigned int		  mm4_cmd_xfrmd_value;
} MM4_CMD_XFRMD_UNION, *P_MM4_CMD_XFRMD_UNION;

#define NO_ARGUMENT					0xffffffff
#define MM4_CMD_TYPE_NORMAL 		0
#define MM4_CMD_TYPE_SUSPEND  		1
#define MM4_CMD_TYPE_RESUME 		2
#define MM4_CMD_TYPE_ABORT 			3
#define MM4_CMD_DATA 				1
#define MM4_CMD_NODATA				0
#define MM4_NO_RES					0
#define MM4_136_RES					1
#define MM4_48_RES					2
#define MM4_48_RES_WITH_BUSY		3
#define MM4_MULTI_BLOCK_TRAN		1
#define MM4_SINGLE_BLOCK_TRAN		0
#define MM4_HOST_TO_CARD_DATA		0
#define MM4_CARD_TO_HOST_DATA		1



// this information will be included in the response type argument of relevant apis.
// it will occupy bits 15:8 of the RespType parameter.
#define MM4_RT_MASK					0x7f00
#define MM4_RT_NONE					0x0000
#define	MM4_RT_R1					0x0100
#define	MM4_RT_R2					0x0200
#define	MM4_RT_R3					0x0300
#define	MM4_RT_R4					0x0400
#define	MM4_RT_R5					0x0500
#define	MM4_RT_R6					0x0600
#define	MM4_RT_R7					0x0700		// sd card interface condition

#define MM4_RT_BUSYMASK				0x8000
#define MM4_RT_BUSY					0x8000


// ********************* MM4_STATE 0x24********************************
typedef struct
{

	unsigned int ccmdinhbt	   : 1;
	unsigned int dcmdinhbt	   : 1;
	unsigned int datactv	   : 1;
	unsigned int retuning_req  : 1;
	unsigned int reserved0	   : 4;
	unsigned int wractv 	   : 1;
	unsigned int rdactv 	   : 1;
	unsigned int bufwren	   : 1;
	unsigned int bufrden	   : 1;
	unsigned int reserved1	   : 4;
	unsigned int cdinstd	   : 1;
	unsigned int cdstbl 	   : 1;
	unsigned int cddetlvl	   : 1;
	unsigned int wpswlvl	   : 1;
	unsigned int lwrdatlvl	   : 4;
	unsigned int cmdlvl 	   : 1;
	//	3/12: Xiufeng
	//	From APSE's code, there is one param uprdatlvl which is 4 bits. our controller
	// 	does not have this parameter and reservers the following 7 bits
//	unsigned int uprdatlvl	   : 4;
	unsigned int reserved2	   : 7;

} MM4_STATE, *P_MM4_STATE;

typedef union
{
	MM4_STATE     mm4_state_bits;
	unsigned int  mm4_state_value;
} MM4_STATE_UNION, *P_MM4_STATE_UNION;

// ********************* MM4_CNTL1 0x28********************************
typedef struct
{
	unsigned int ledcntl	   : 1; 	   // 0
	unsigned int datawidth	   : 1; 	   // 1
	unsigned int hispeed	   : 1; 	   // 2
	unsigned int dma_sel	   : 2; 	   // 3
	unsigned int ex_data_width : 1; 	   // 5
	unsigned int card_det_l    : 1; 	   // 6
	unsigned int card_det_s    : 1; 	   // 7
	unsigned int buspwr 	   : 1; 	   // 8
	unsigned int vltgsel	   : 3; 	   // 9
	unsigned int reserved2	   : 4; 	   // 12
	unsigned int bgreqstp	   : 1; 	   // 16
	unsigned int contreq	   : 1;		   // 17
	unsigned int rdwcntl	   : 1;		   // 18
	unsigned int bgirqen	   : 1;        // 19
	unsigned int reserved3	   : 4;
	unsigned int card_int_wake : 1;        // 24
	unsigned int card_ins_wake : 1;		   // 25
	unsigned int card_rem_wake : 1;		   // 26
	unsigned int reserved4	   : 5;

} MM4_CNTL1, *P_MM4_CNTL1;

typedef union
{
	MM4_CNTL1     mm4_cntl1_bits;
	unsigned int		  mm4_cntl1_value;
} MM4_CNTL1_UNION, *P_MM4_CNTL1_UNION;

#define MM4_VLTGSEL_1_8		0x5
#define MM4_VLTGSEL_3_0		0x6
#define MM4_VLTGSEL_3_3		0x7

typedef struct
{
	unsigned int inter_clk_en	   	: 1;    // Internal Clock Enable
	unsigned int inter_clk_stable  		: 1;    // Internal Clock Stable
	unsigned int mm4clken		   	: 1;    // Clock Enable
	unsigned int pll_enable         : 1;
	unsigned int reserved4   	   	: 1;    // bits4
	unsigned int clk_gen_sel	   	: 1;    // bit 5
	unsigned int sd_freq_sel_hi    		: 2;    // 6
	unsigned int sd_freq_sel_lo    		: 8;    // 8
	unsigned int dtocntr		   	: 4;    // bit 0 Data Timeout Counter Value
	unsigned int reserved2		   	: 4;    //
	unsigned int mswrst 		   	: 1;    // bit 8 Software Reset for All
	unsigned int cmdswrst		   	: 1;    // bit 9 Software Reset for MM4CMD Line
	unsigned int datswrst		   	: 1;    // bit 10 Software Reset for MM4DATx Lines
	unsigned int reserved3		   	: 5;    // bits 11-15

} MM4_CNTL2, *P_MM4_CNTL2;

#define MM4CLKEN_EN_CLOCK    1

#define MM4CLOCK200KHZRATE	0x1FF	// Set also additional SD_FREQ_HI bit.	So SD_FREQ_SEL = 0x1FF = 511 * 2 = 1022 (clock divider)
#define MM4CLOCK50MHZRATE	2
#define MM4CLOCK25MHZRATE	4
#define MM4CLOCK13MHZRATE	8
#define MM4CLOCK6MHZRATE	16

#define MM4CLOCK187KHZRATE	0x80
#define MM4CLOCK48MHZRATE	0x0
#define CLOCK_27_MULT		0xE

#define SDCLK_SEL_DIV_1024	0x200
#define SDCLK_SEL_DIV_512	0x100
#define SDCLK_SEL_DIV_256	0x80
#define SDCLK_SEL_DIV_128	0x40
#define SDCLK_SEL_DIV_64	0x20
#define SDCLK_SEL_DIV_32	0x10
#define SDCLK_SEL_DIV_16	0x08
#define SDCLK_SEL_DIV_8		0x04
#define SDCLK_SEL_DIV_2		0x01
#define SDCLK_SEL_DIV_1		0x00

#define SDBUS_4BIT			0x01
typedef union
{
	MM4_CNTL2     mm4_cntl2_bits;
	unsigned int		  mm4_cntl2_value;
} MM4_CNTL2_UNION, *P_MM4_CNTL2_UNION;

// ********************* MM4_I_STAT, MM4_I_STAT_EN, MM4_I_SIGN_EN -0x34************
typedef struct
{
	unsigned int cmdcomp	   : 1;   //0
	unsigned int xfrcomp	   : 1;   //1
	unsigned int bgevnt 	   : 1;   //2
	unsigned int dmaint 	   : 1;   //3
	unsigned int bufwrrdy	   : 1;   //4
	unsigned int bufrdrdy	   : 1;   //5
	unsigned int cdins		   : 1;   //6
	unsigned int cdrem		   : 1;   //7
	unsigned int cdint		   : 1;   //8
	unsigned int int_a		   : 1;   //9
	unsigned int int_b		   : 1;   //10
	unsigned int int_c		   : 1;   //11
	unsigned int retuninig_int : 1;   //12
	unsigned int reserved0	   : 2;   //13
	unsigned int errint 	   : 1;   //15
	unsigned int cmdtout 	   	: 1;   //16
	unsigned int cmdcrcerr	   	: 1;   //17
	unsigned int cmdenderr	   	: 1;   //18
	unsigned int cmdidxerr	   	: 1;   //19
	unsigned int datatout 	   	: 1;   //20
	unsigned int datacrcerr	   	: 1;   //21
	unsigned int dataenderr	   	: 1;   //22
	unsigned int ilmterr	   	: 1;   //23
	unsigned int ac12err	   	: 1;   //24
	unsigned int admaerr	   	: 1;   //25
	unsigned int tune_err	   	: 1;   //26
	unsigned int resp_err	   	: 1;   //27
	unsigned int boot_ack_err    	: 1;   //28
	unsigned int vendor_err1  	: 1;   //29
	unsigned int vendor_err2  	: 1;   //30
	unsigned int vendor_err3  	: 1;   //31

} MM4_I_STAT, *P_MM4_I_STAT, MM4_I_STAT_EN, *P_MM4_I_STAT_EN, MM4_I_SIGN_EN, *P_MM4_I_SIGN_EN;

#define DISABLE_INTS 	0
#define ENABLE_INTS		1

typedef union
{
	MM4_I_STAT 	  mm4_i_stat_bits;
	unsigned int		  mm4_i_stat_value;
} MM4_I_STAT_UNION, *P_MM4_I_STAT_UNION;

#define CLEAR_INTS_MASK		0xFFFF7EFF


// ********************* MM4_ACMD12_ER 0x3c*******************************************
typedef struct
{
	unsigned int ac12nexe		   : 1;   // 0
	unsigned int ac12toer		   : 1;   // 1
	unsigned int ac12crcer		   : 1;   // 2
	unsigned int ac12ender		   : 1;   // 3
	unsigned int ac12idxer		   : 1;   // 4
	unsigned int reserved0		   : 2;   // 5
	unsigned int cmdnisud		   : 1;   // 7
	unsigned int reserved1		   : 8;   // 8
	unsigned int uhs_mode_sel	   : 3;   // 16
	unsigned int sgh_v18_en 	   : 1;   // 19
	unsigned int drv_strength_sel  : 2;   // 20
	unsigned int exe_tuning 	   : 1;   // 22
	unsigned int sampling_clk_sel  : 1;   // 23
	unsigned int uhs2_if_en        : 1;   // 24
	unsigned int reserved2		   : 1;   // 25
	unsigned int adma2_len_26bit   : 1;   // 26
	unsigned int cmd23_en          : 1;   // 27
	unsigned int host_ver4_en      : 1;   // 28
	unsigned int addressing_64bit  : 1;   // 29
	unsigned int async_int_en	   : 1;   // 30
	unsigned int pre_val_en 	   : 1;   // 31

} MM4_ACMD12_ER, *P_MM4_ACMD12_ER;

// ********************* MM4_CAP0 0x40*******************************************
typedef struct
{
 unsigned int toclkfreq				: 6;
 unsigned int reserved0				: 1;
 unsigned int toclkunit				: 1;
 unsigned int bsclkfreq				: 8;
 //	3/12 Xiufeng
 //	the following is not same with ASPE's code
 unsigned int max_blk_len			: 2;
 unsigned int ex_data_width_support	: 1;
 //	3/12 Xiufeng
 //	no reserved bit here
 //unsigned int reserved1				: 1;
 unsigned int adma2_support			: 1;
 unsigned int adma1_support			: 1;
 unsigned int hi_speed_support		: 1;
 unsigned int sdma_support			: 1;
 unsigned int sus_res_support		: 1;
 unsigned int vlg_33_support		: 1;
 unsigned int vlg_30_support		: 1;
 unsigned int vlg_18_support		: 1;
 unsigned int reserved2				: 1;
 unsigned int sys_bus_64_support	: 1;
 unsigned int async_int_support		: 1;
 //	3/12 Xiufeng
 //	cfg_slot_type is 2 bits now
 //unsigned int cfg_slot_type			: 1;
 unsigned int cfg_slot_type			: 2;
} MM4_CAP1_2, *P_MM4_CAP1_2;

typedef union
{
	MM4_CAP1_2 	  mm4_cap1_2_bits;
	UINT_T		  mm4_cap1_2_value;
} MM4_CAP1_2_UNION, *P_MM4_CAP1_2_UNION;



// ********************* MM4_SD_MAX_CURRENT1_2 0x48*******************************************
typedef struct
{
 unsigned int v3_3vmaxi		: 8;
 unsigned int v3_0vmaxi		: 8;
 unsigned int v1_8vmaxi		: 8;
 unsigned int reserved0 	: 8;
} MM4_SD_MAX_CURRENT1_2, *P_MM4_SD_MAX_CURRENT1_2;

typedef union
{
	MM4_SD_MAX_CURRENT1_2 	  	mm4_sd_max_current1_2_bits;
	UINT_T		  				mm4_sd_max_current1_2_value;
} MM4_SD_MAX_CURRENT1_2_UNION, *P_MM4_SD_MAX_CURRENT1_2_UNION;





/*********** End MM4 Register Def's **************************************/
// response types
enum {
  MMC_RESPONSE_NONE = 1L<<8,
  MMC_RESPONSE_R1 = 2L<<8,
  MMC_RESPONSE_R1B = 3L<<8,
  MMC_RESPONSE_R2 = 4L<<8,
  MMC_RESPONSE_R3 = 5L<<8,
  MMC_RESPONSE_R4 = 6L<<8,
  MMC_RESPONSE_R5 = 7L<<8,
  MMC_RESPONSE_R5B = 8L<<8,
  MMC_RESPONSE_R6 = 9L<<8,
  MMC_RESPONSE_R7 = 0xAL<<8,
  MMC_RESPONSE_MASK = 0x0000FF00
};

#define SD_OCR_VOLTAGE_3_3_TO_3_6 	0xE00000
#define SD_OCR_VOLTAGE_1_8_TO_3_3 	0x1F8000
#define SD_OCR_VOLTAGE_1_8		 	0x80
#define MMC_OCR_VOLTAGE_ALL			0xFF8000
#define	MM4FIFOWORDSIZE				0x80

// device context for MMC API, containing anything needed to operate on
// this API. It is always first parameter for all APIs.
typedef struct
{
  P_MM4_SDMMC_T            	pMMC4Reg;				// MMC1 register base
} MM4_SDMMC_CONTEXT_T, *P_MM4_SDMMC_CONTEXT_T;




//#ifdef IO_CARD_SUPPORT

#define SDIO_OCR_VOLTAGE_2_0_TO_2_1		0x000100
#define SDIO_OCR_VOLTAGE_2_1_TO_2_2		0x000200
#define SDIO_OCR_VOLTAGE_2_2_TO_2_3		0x000400
#define SDIO_OCR_VOLTAGE_2_3_TO_2_4		0x000800
#define SDIO_OCR_VOLTAGE_2_4_TO_2_5		0x001000
#define SDIO_OCR_VOLTAGE_2_5_TO_2_6		0x002000
#define SDIO_OCR_VOLTAGE_2_6_TO_2_7		0x004000
#define SDIO_OCR_VOLTAGE_2_7_TO_2_8		0x008000
#define SDIO_OCR_VOLTAGE_2_8_TO_2_9		0x010000
#define SDIO_OCR_VOLTAGE_2_9_TO_3_0		0x020000
#define SDIO_OCR_VOLTAGE_3_0_TO_3_1		0x040000
#define SDIO_OCR_VOLTAGE_3_1_TO_3_2		0x080000
#define SDIO_OCR_VOLTAGE_3_2_TO_3_3		0x100000
#define SDIO_OCR_VOLTAGE_3_3_TO_3_4		0x200000
#define SDIO_OCR_VOLTAGE_3_4_TO_3_5		0x400000
#define SDIO_OCR_VOLTAGE_3_5_TO_3_6		0x800000
#define SDIO_OCR_ALL_VOLTAGES			0xFFFFFF

//#endif




// ADMA descriptor
typedef struct sdio_adma_desc
{
	unsigned int valid 		:1;
	unsigned int end		:1;
	unsigned int interrupt	:1;
	unsigned int zero		:1;
	unsigned int act1		:1;
	unsigned int act2		:1;
	unsigned int resvered	:10;
	unsigned int length		:16;
	unsigned int addr;
} SDIO_ADMA_DESC, *SDIO_ADMA_DESC_PTR;


#define RPMB_256_HALF_SECTOR_SIZE	0x100
enum rpmb_op_type {
	MMC_RPMB_WRITE_KEY = 0x01,
	MMC_RPMB_READ_CNT  = 0x02,
	MMC_RPMB_WRITE     = 0x03,
	MMC_RPMB_READ      = 0x04,

	/* For internal usage only, do not use it directly */
	MMC_RPMB_READ_RESP = 0x05,
	MMC_RPMB_WRITE_CONFIG = 0x06,
	MMC_RPMB_READ_CONFIG  = 0x07
};

enum rpmb_resp_type {
	MMC_RPMB_WRITE_KEY_RESP	= 0x0100,
	MMC_RPMB_READ_CNT_RESP	= 0x0200,
	MMC_RPMB_WRITE_RESULT_RESP		= 0x0300,
	MMC_RPMB_READ_RESULT_RESP		= 0x0400,

	MMC_RPMB_RESERVED_RESP	= 0x0500,
	MMC_RPMB_WRITE_CONFIG_RESP	= 0x0600,
	MMC_RPMB_READ_CONFIG_RESP	= 0x0700
};


typedef struct
{
	UINT8_T  stuff[196];
	UINT8_T  key_mac[32];
	UINT8_T  data[256];
	UINT8_T  nonce[16];
	UINT32_T write_counter;
	UINT16_T address;
	UINT16_T block_count;
	UINT16_T result;
	UINT16_T req_resp;
}rpmbFrame;

unsigned int do_emmc_get_param_customized(void);
unsigned long do_emmc_rpmbCapacity(void);
int do_rpmb_op(UINT16_T rpmb_type, rpmbFrame *frame_in, rpmbFrame *frame_out,unsigned char *DatatoWrite, unsigned long WriteCnt,\
	unsigned long DataCnt_perSector, unsigned char *KeyMac, unsigned long CardAddr_perSector);
void do_emmc_stop_alt_boot_mode(void);

#endif		// __SDMMC_API_H__
