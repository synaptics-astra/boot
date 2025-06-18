#ifndef __APB_REG_BASE__
#define __APB_REG_BASE__

#include "Galois_memmap.h"
#include "i_apbPerif_apb_map.h"
#include "i_SmApb_apb_map.h"
#include "SysMgr.h"

//APB Peripherals
#define APB_TIMER1_BASE         (MEMMAP_APBPERIF_REG_BASE + i_apbPerif_timers_1_BaseAddress)    // 0x0000
#define APB_TIMER0_BASE         (MEMMAP_APBPERIF_REG_BASE + i_apbPerif_timers_BaseAddress)      // 0x400
#define APB_UART0_BASE          (MEMMAP_APBPERIF_REG_BASE + i_apbPerif_uart_0_BaseAddress)      // 0x0800
#define APB_UART1_BASE          (MEMMAP_APBPERIF_REG_BASE + i_apbPerif_uart_1_BaseAddress)      // 0x0C00
#define APB_I2C0_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_i2c_0_BaseAddress)       // 0x1000
#define APB_I2C1_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_i2c_1_BaseAddress)       // 0x1400
#define APB_WDT0_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_wdt_0_BaseAddress)       // 0x1800
#define APB_SSI1_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_ssi_BaseAddress)         // 0x1C00
#define APB_WDT1_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_wdt_1_BaseAddress)       // 0x2000
#define APB_WDT2_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_wdt_2_BaseAddress)       // 0x2400
#define APB_GPIO0_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_gpio_0_BaseAddress)      // 0x2800
#define APB_GPIO1_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_gpio_1_BaseAddress)      // 0x2C00
#define APB_GPIO2_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_gpio_2_BaseAddress)      // 0x3000
 

#define SOC_SM_ITCM_BASE		SOC_SM_MEMMAP_ITCM_BASE     // (MEMMAP_SM_REG_BASE + 0)         // 0xF7F80000
#define SOC_SM_DTCM_BASE		SOC_SM_MEMMAP_ITCM_BASE
#define SOC_SM_APB_REG_BASE		SOC_SM_MEMMAP_SMAPB_BASE    // (MEMMAP_SM_REG_BASE + 0x40000)   // 0xF7FC0000
#define SOC_SM_SEC_RAM_BASE		SOC_SM_MEMMAP_SSRAM_BASE    // (MEMMAP_SM_REG_BASE + 0x50000)   // 0xF7FD0000
//#define SOC_SM_WOL_REG_BASE		(MEMMAP_SM_REG_BASE + 0x60000)
#define SOC_SM_CEC_REG_BASE		SOC_SM_MEMMAP_CEC_BASE      // (MEMMAP_SM_REG_BASE + 0x61000)   // 0xF7FE1000
#define SOC_SM_SYS_CTRL_REG_BASE	SOC_SM_MEMMAP_SMREG_BASE// (MEMMAP_SM_REG_BASE + 0x62000)   // 0xF7FE2000



#define SM_APB_ICTL0_BASE		(SOC_SM_APB_REG_BASE + i_SmApb_ictl_0_BaseAddress)      // 0x0000
#define SM_APB_ICTL1_BASE		(SOC_SM_APB_REG_BASE + i_SmApb_ictl_1_BaseAddress)      // 0x1000
#define SM_APB_ICTL2_BASE		(SOC_SM_APB_REG_BASE + i_SmApb_ictl_2_BaseAddress)      // 0x2000 interrupt output to soc
#define SM_APB_WDT0_BASE		(SOC_SM_APB_REG_BASE + i_SmApb_wdt_0_BaseAddress)       // 0x3000 interrupt output to soc
#define SM_APB_WDT1_BASE		(SOC_SM_APB_REG_BASE + i_SmApb_wdt_1_BaseAddress)       // 0x4000
#define SM_APB_WDT2_BASE		(SOC_SM_APB_REG_BASE + i_SmApb_wdt_2_BaseAddress)       // 0x5000
#define SM_APB_TIMER0_BASE		(SOC_SM_APB_REG_BASE + i_SmApb_timers_0_BaseAddress)    // 0x6000
#define SM_APB_TIMER1_BASE		(SOC_SM_APB_REG_BASE + i_SmApb_timers_1_BaseAddress)    // 0x7000
#define SM_APB_GPIO0_BASE		(SOC_SM_APB_REG_BASE + i_SmApb_gpio_0_BaseAddress)      // 0x8000

#define SM_APB_SPI_BASE			(SOC_SM_APB_REG_BASE + i_SmApb_ssi_0_BaseAddress)       // 0xA000
#define SM_APB_I2C0_BASE		(SOC_SM_APB_REG_BASE + i_SmApb_i2c_0_BaseAddress)       // 0xB000
#define SM_APB_I2C1_BASE		(SOC_SM_APB_REG_BASE + i_SmApb_i2c_1_BaseAddress)       // 0xC000
#define SM_APB_UART0_BASE		(SOC_SM_APB_REG_BASE + i_SmApb_uart_0_BaseAddress)      // 0xD000
#define SM_APB_UART1_BASE		(SOC_SM_APB_REG_BASE + i_SmApb_uart_1_BaseAddress)      // 0xE000

#endif
