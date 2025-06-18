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
#ifndef _SYSTEMMANAGER_H_
#define _SYSTEMMANAGER_H_

#include "apbRegBase.h"
#include "SysMgr.h"

#define CPUINDEX                                1

#define SM_ITCM_BASE                            0x00000000
#define SM_ITCM_SIZE                            0x00008000      // 32k
#define SM_ITCM_END                             0x00008000
#define SM_ITCM_ALIAS_SIZE                      0x00020000      // 128k

#define SM_DTCM_BASE                            0x04000000
#define SM_DTCM_SIZE                            0x00004000      // 16k
#define SM_DTCM_END                             0x04004000
#define SM_DTCM_ALIAS_SIZE                      0x00020000      // 128k

#define SM_APBC_BASE                            0x10000000
#define SM_APBC_SIZE                            0x00010000      // 64k
#define SM_APBC_END                             0x10010000
#define SM_APBC_ALIAS_SIZE                      0x00010000      // 64k

#define SM_SECM_BASE                            0x40000000
#define SM_SECM_SIZE                            0x00010000      //64k
#define SM_SECM_END                             0x40010000
#define SM_SECM_ALIAS_SIZE                      0x00010000      // 64k

#define SOC_ITCM_BASE                           0xf7f80000
#define SOC_DTCM_BASE                           0xf7fa0000
#define SOC_APBC_BASE                           0xf7fc0000
#define SOC_SECM_BASE                           0xf7fd0000

#if ( CPUINDEX == 2 )
#define SM_APBC( reg )                          ( (INT32)(reg) )
#else
#define SM_APBC( reg )                          ( (INT32)(reg) - SM_APBC_BASE + SOC_APBC_BASE )
#endif

#define SOC_ITCM( addr )                        ( (INT32)addr - SM_ITCM_BASE + SOC_ITCM_BASE )
#define SOC_DTCM( addr )                        ( (INT32)addr - SM_DTCM_BASE + SOC_DTCM_BASE )
#define SOC_APBC( addr )                        ( (INT32)addr - SM_APBC_BASE + SOC_APBC_BASE )
#define SOC_SECM( addr )                        ( (INT32)addr - SM_SECM_BASE + SOC_SECM_BASE )


#define SM_SM_APB_ICTL0_BASE		(SM_APBC_BASE)
#define SM_SM_APB_ICTL1_BASE		(SM_APBC_BASE + 0x1000)
#define SM_SM_APB_ICTL2_BASE		(SM_APBC_BASE + 0x2000)
#define SM_SM_APB_WDT0_BASE		(SM_APBC_BASE + 0x3000)
#define SM_SM_APB_WDT1_BASE		(SM_APBC_BASE + 0x4000)
#define SM_SM_APB_WDT2_BASE		(SM_APBC_BASE + 0x5000)
#define SM_SM_APB_TIMER0_BASE		(SM_APBC_BASE + 0x6000)
#define SM_SM_APB_TIMER1_BASE		(SM_APBC_BASE + 0x7000)
#define SM_SM_APB_GPIO0_BASE		(SM_APBC_BASE + 0x8000)
#define SM_SM_APB_GPIO1_BASE		(SM_APBC_BASE + 0x8000)
#define SM_SM_APB_SPI_BASE		(SM_APBC_BASE + 0xA000)
#define SM_SM_APB_I2C0_BASE		(SM_APBC_BASE + 0xB000)
#define SM_SM_APB_I2C1_BASE		(SM_APBC_BASE + 0xC000)
#define SM_SM_APB_UART0_BASE		(SM_APBC_BASE + 0xD000)
#define SM_SM_APB_UART1_BASE		(SM_APBC_BASE + 0xE000)
#define SM_SM_SYS_CTRL_REG_BASE		(0x10012000)

#define	LSb32SS_SM_ICTL_IRQ_SM_WDT0				0
#define	LSb32SS_SM_ICTL_IRQ_SM_WDT1				1
#define	LSb32SS_SM_ICTL_IRQ_SM_WDT2				2
#define	LSb32SS_SM_ICTL_IRQ_RSVD0				3
#define	LSb32SS_SM_ICTL_IRQ_SM_GPIO_B				4
#define	LSb32SS_SM_ICTL_IRQ_SM_SPI				5
#define	LSb32SS_SM_ICTL_IRQ_SM_I2C0				6
#define	LSb32SS_SM_ICTL_IRQ_SM_I2C1				7
#define	LSb32SS_SM_ICTL_IRQ_SM_UART0				8
#define	LSb32SS_SM_ICTL_IRQ_SM_UART1				9
#define	LSb32SS_SM_ICTL_IRQ_RSVD1				10
#define	LSb32SS_SM_ICTL_IRQ_SM_GPIO_A				11
#define	LSb32SS_SM_ICTL_IRQ_SM_ADC				12
#define	LSb32SS_SM_ICTL_IRQ_SOC2SM_SW				13
#define	LSb32SS_SM_ICTL_IRQ_SM_TSEN				14
#define	LSb32SS_SM_ICTL_IRQ_SM_WOL				15
#define	LSb32SS_SM_ICTL_IRQ_SM_CEC				16
#define	LSb32SS_SM_ICTL_IRQ_SM_CEC_FIFO				17
#define	LSb32SS_SM_ICTL_IRQ_SM_ETH				18
#define	LSb32SS_SM_ICTL_IRQ_SM_HPD				19
#define	LSb32SS_SM_ICTL_IRQ_SM_IHPD				20
#define	LSb32SS_SM_ICTL_IRQ_SM_TIMER0_0				21
#define	LSb32SS_SM_ICTL_IRQ_SM_TIMER0_1				22
#define	LSb32SS_SM_ICTL_IRQ_SM_TIMER0_2				23
#define	LSb32SS_SM_ICTL_IRQ_SM_TIMER0_3				24
#define	LSb32SS_SM_ICTL_IRQ_SM_TIMER0_4				25
#define	LSb32SS_SM_ICTL_IRQ_SM_TIMER0_5				26
#define	LSb32SS_SM_ICTL_IRQ_SM_TIMER0_6				27
#define	LSb32SS_SM_ICTL_IRQ_SM_TIMER0_7				28
#define	LSb32SS_SM_ICTL_IRQ_SM_TIMER1_0				29
#define	LSb32SS_SM_ICTL_IRQ_SM_TIMER1_1				30
#define	LSb32SS_SM_ICTL_IRQ_SM_TIMER1_2				31
#define	LSb32SS_SM_ICTL_IRQ_SM_TIMER1_3				32
#define	LSb32SS_SM_ICTL_IRQ_SM_TIMER1_4				33
#define	LSb32SS_SM_ICTL_IRQ_SM_TIMER1_5				34
#define	LSb32SS_SM_ICTL_IRQ_SM_TIMER1_6				35
#define	LSb32SS_SM_ICTL_IRQ_SM_TIMER1_7				36

#if 1
#define MV_SM_READ_REG32( reg )                 ( *( (UINT32*)SM_APBC( reg ) ) )
#define MV_SM_READ_REG8( reg )                  ( *( (UCHAR*)SM_APBC( reg ) ) )
#define MV_SM_WRITE_REG32( reg, value )         ( *( (UINT32*)SM_APBC( reg ) ) = (UINT32)( value ) )
#define MV_SM_WRITE_REG8( reg, value )          ( *( (UCHAR*)SM_APBC( reg ) ) = (UCHAR)( value ) )
#else
#define MV_SM_READ_REG32( reg )                 ( *( (UINT32*)SM_APBC( reg ) ) )
#define MV_SM_READ_REG8( reg )                  ( *( (UCHAR*)SM_APBC( reg ) ) )
#define MV_SM_WRITE_REG32( reg, value )         { ( *( (UINT32*)SM_APBC( reg ) ) = (UINT32)( value ) ); printf( "reg 0x%8.8x = 0x%8.8x\n", SM_APBC(reg), value ); }
#define MV_SM_WRITE_REG8( reg, value )          { ( *( (UCHAR*)SM_APBC( reg ) ) = (UCHAR)( value ) ); printf( "reg 0x%8.8x = 0x%2.2x\n", SM_APBC(reg), value ); }
#endif

// define module ID for each module
#define MV_SM_ID_SYS                            1
#define MV_SM_ID_COMM                           2
#define MV_SM_ID_IR                             3
#define MV_SM_ID_KEY                            4
#define MV_SM_ID_POWER                          5
#define MV_SM_ID_WD                             6
#define MV_SM_ID_TEMP                           7
#define MV_SM_ID_VFD                            8
#define MV_SM_ID_SPI                            9
#define MV_SM_ID_I2C                            10
#define MV_SM_ID_UART                           11
#define MV_SM_ID_GPIO                           22

//      define SM state
#define MV_SM_STATE_COLDBOOTUP                  1
#define MV_SM_STATE_WAIT_WARMUP_0_RESP          2
#define MV_SM_STATE_WARMUP                      3
#define MV_SM_STATE_WAIT_WARMDOWN_RESP          4
#define MV_SM_STATE_WARMDOWN                    5
#define MV_SM_STATE_WAIT_WARMUP_RESP            6

// following assignment is so we can assign Beetle-specific value that is different from
// a normal STATE_WARMUP_1...

#define reserved_MV_SM_STATE                    7
#define MV_SM_POWER_BOOT_MODE                   8 /* also used as msg type to SM */

#define MV_SM_STATE_WAIT_WARMDOWN_2_RESP        (0xA0)
#define MV_SM_STATE_WARMDOWN_2                  (0xA1)
#define MV_SM_STATE_WAIT_WARMUP_2_RESP          (0xA2)
#define MV_SM_STATE_WARMUP_2                    (0xA3)


//      define   message  content
#define MV_SM_POWER_WARMUP_0_RESP               (MV_SM_STATE_WAIT_WARMUP_0_RESP)
#define MV_SM_POWER_WARMDOWN_1_RESP             (MV_SM_STATE_WAIT_WARMDOWN_RESP)
#define MV_SM_POWER_WARMUP_1_RESP               (MV_SM_STATE_WAIT_WARMUP_RESP)
#define MV_SM_POWER_WARMDOWN_2_RESP             (MV_SM_STATE_WAIT_WARMDOWN_2_RESP)
#define MV_SM_POWER_WARMUP_2_RESP               (MV_SM_STATE_WAIT_WARMUP_2_RESP)
#define MV_SoC_STATE_COLDBOOT                   0x21

//      define  SoC state
//
#define MV_SoC_STATE_SYSPOWERON                 0
#define MV_SoC_STATE_SYSRESET                   0xFF
#define MV_SoC_STATE_COLDBOOTUP                 (MV_SM_STATE_COLDBOOTUP)
#define MV_SoC_STATE_WARMUP_0_RESP              (MV_SM_POWER_WARMUP_0_RESP)
#define MV_SoC_STATE_WARMUP_0                   (MV_SM_STATE_WAIT_WARMUP_0_RESP)

#define MV_SoC_STATE_WARMDOWN_1_RESP            (MV_SM_POWER_WARMDOWN_1_RESP)
#define MV_SoC_STATE_WARMDOWN_1                 (MV_SM_STATE_WAIT_WARMDOWN_RESP)

#define MV_SoC_STATE_WARMUP_1_RESP              (MV_SM_POWER_WARMUP_1_RESP)
#define MV_SoC_STATE_WARMUP_1                   (MV_SM_STATE_WAIT_WARMUP_RESP)
#define MV_SoC_STATE_WARMUP_2                   (MV_SM_STATE_WAIT_WARMUP_2_RESP)
#define MV_SoC_STATE_WARMDOWN_2                 (MV_SM_STATE_WAIT_WARMDOWN_2_RESP)

// Beetle specific variation on STATE_WARMUP_1. When bootloader sees it, he should skip
// loading SM with sm.bin as he'd normally do, and then respond to it with the same message
// he would use with MV_SoC_STATE_WARMUP_1.
#define MV_SoC_STATE_WARMUP_FROM_DDR_OFF_1 (reserved_MV_SM_STATE)
#define MV_SoC_STATE_BOOT_RECOVERY_KERNEL  (MV_SM_POWER_BOOT_MODE) /* state requesting recovery mode*/

#define MV_SM_RSTFLAG_COLD                      0xFFFF
#define MV_SM_RSTFLAG_WARM                      0xFF00
#define MV_SM_TEMP_CPU_TYPE                     0xCC
#define MV_SM_TEMP_CPU_TYPE_B0                  0xB0
#define MV_SM_SOC_TSEN_ID                       0xDD
#define MV_SM_BOARDVER_GET                      0x08

#define SM_ITCM_SPACE_SIZE                      (0x00020000)                            // 128KB
#define SM_DTCM_REAL_SIZE                       (0x00004000)                            // 16 KB


#define SM_BOOT_FLAG_REG                        (SOC_SM_SYS_CTRL_REG_BASE + RA_smSysCtl_SM_BOOT_STATUS)

#define SM_RST_STATUS                           (SM_SM_SYS_CTRL_REG_BASE + RA_smSysCtl_SM_RST_STATUS - SM_APBC_BASE + SOC_APBC_BASE)
#define SM_CPU_CTRL                             (SM_SM_SYS_CTRL_REG_BASE + RA_smSysCtl_SM_CPU_CTRL - SM_APBC_BASE + SOC_APBC_BASE)

#define SM_SM_CTRL                              (SM_SM_SYS_CTRL_REG_BASE + RA_smSysCtl_SM_CTRL - SM_APBC_BASE + SOC_APBC_BASE)
#define SM_ADC_STATUS                           (SM_SM_SYS_CTRL_REG_BASE + RA_smSysCtl_SM_ADC_STATUS - SM_APBC_BASE + SOC_APBC_BASE)
#define SM_ADC_DATA                             (SM_SM_SYS_CTRL_REG_BASE + RA_smSysCtl_SM_ADC_DATA - SM_APBC_BASE + SOC_APBC_BASE)

#define SM_WARM_BOOT_CMD_TABLE_BASE             (MEMMAP_SM_REG_BASE + SM_ITCM_SPACE_SIZE + SM_DTCM_REAL_SIZE - 512)

#define SM_WARMDOWN_2_LINUX_ADDR                (MEMMAP_SM_REG_BASE + SM_ITCM_SPACE_SIZE + SM_DTCM_REAL_SIZE - 4)
#define SM_MSG_EXTRA_BUF_ADDR                   (MEMMAP_SM_REG_BASE + SM_ITCM_SPACE_SIZE + SM_DTCM_REAL_SIZE - 512)
#define SM_MSG_EXTRA_BUF_SIZE                   494

#define SM_SECM_DTCM_SIZE						(4 * 1024)
#define SM_BOOTMODE_ADDR						(SOC_SECM_BASE + SM_SECM_DTCM_SIZE - 64 - 96)
#define SM_DDRSCRAMBLING_CONTEXT                (SM_BOOTMODE_ADDR - 32)

//Notice: SM can not access Secure SRAM
//The Actual Address Offset: SM_DTCM_SIZE - sizeof(rebootflag_t) - sizeof(UINT32)
#define SM_REBOOTREASON_ADDR_SOC		(SOC_DTCM_BASE + SM_DTCM_SIZE - 0xC)
//#define SM_REBOOTREASON_ADDR_SM		(SM_REBOOTREASON_ADDR_SOC - SOC_DTCM_BASE + SM_DTCM_BASE)

#define SM_MSG_PINMUX_TABLE_ADDR                0xF7FA3600
#define SM_MSG_PINMUX_TABLE_SIZE                128

//#define DYNAMIC_MODULE

#endif // #ifndef _SYSTEMMANAGER_H_
