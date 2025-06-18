#ifndef __APB_REG_BASE__
#define __APB_REG_BASE__

#include "Galois_memmap.h"
#include "i_apbPerif_apb_map.h"

//APB Peripherals
#define APB_TIMER0_BASE         (MEMMAP_APBPERIF_REG_BASE + i_apbPerif_timers_BaseAddress)
#define APB_TIMER1_BASE         (MEMMAP_APBPERIF_REG_BASE + i_apbPerif_timers_1_BaseAddress)
#define APB_UART0_BASE          (MEMMAP_APBPERIF_REG_BASE + i_apbPerif_uart_0_BaseAddress)
#define APB_UART1_BASE          (MEMMAP_APBPERIF_REG_BASE + i_apbPerif_uart_1_BaseAddress)
#define APB_I2C0_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_i2c_0_BaseAddress)
#define APB_I2C1_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_i2c_1_BaseAddress)
#define APB_I2C2_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_i2c_2_BaseAddress)
#define APB_I2C3_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_i2c_3_BaseAddress)
#define APB_WDT0_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_wdt_0_BaseAddress)
#define APB_SSI1_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_ssi_0_BaseAddress)
#define APB_SSI2_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_ssi_1_BaseAddress)
#define APB_WDT1_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_wdt_1_BaseAddress)
#define APB_WDT2_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_wdt_2_BaseAddress)
#define APB_GPIO0_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_gpio_0_BaseAddress)
#define APB_GPIO1_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_gpio_1_BaseAddress)
#define APB_GPIO2_BASE			(MEMMAP_APBPERIF_REG_BASE + i_apbPerif_gpio_2_BaseAddress)
 

#define SOC_SM_ITCM_BASE		(MEMMAP_SM_REG_BASE + 0)
#define SOC_SM_DTCM_BASE		(MEMMAP_SM_REG_BASE + 0x20000)
#define SOC_SM_APB_REG_BASE		(MEMMAP_SM_REG_BASE + 0x40000)
#define SOC_SM_SEC_RAM_BASE		(MEMMAP_SM_REG_BASE + 0x50000)
#define SOC_SM_WOL_REG_BASE		(MEMMAP_SM_REG_BASE + 0x60000)
#define SOC_SM_CEC_REG_BASE		(MEMMAP_SM_REG_BASE + 0x61000)
#define SOC_SM_SYS_CTRL_REG_BASE	(MEMMAP_SM_REG_BASE + 0x62000)



//#define SM_APB_ICTL0_BASE		(SOC_SM_APB_REG_BASE + 0x0000)
//#define SM_APB_ICTL1_BASE		(SOC_SM_APB_REG_BASE + 0x1000)
//#define SM_APB_ICTL2_BASE		(SOC_SM_APB_REG_BASE + 0x2000)      // interrupt output to soc
//#define SM_APB_WDT0_BASE		(SOC_SM_APB_REG_BASE + 0x3000)      // interrupt output to soc
//#define SM_APB_WDT1_BASE		(SOC_SM_APB_REG_BASE + 0x4000)
//#define SM_APB_WDT2_BASE		(SOC_SM_APB_REG_BASE + 0x5000)
//#define SM_APB_TIMER0_BASE		(SOC_SM_APB_REG_BASE + 0x6000)
//#define SM_APB_TIMER1_BASE		(SOC_SM_APB_REG_BASE + 0x7000)
//#define SM_APB_GPIO0_BASE		(SOC_SM_APB_REG_BASE + 0x8000)

//#define SM_APB_SPI_BASE			(SOC_SM_APB_REG_BASE + 0xA000)
//#define SM_APB_I2C0_BASE		(SOC_SM_APB_REG_BASE + 0xB000)
//#define SM_APB_I2C1_BASE		(SOC_SM_APB_REG_BASE + 0xC000)
//#define SM_APB_UART0_BASE		APB_UART0_BASE //(SOC_SM_APB_REG_BASE + 0xD000)
//#define SM_APB_UART1_BASE		(SOC_SM_APB_REG_BASE + 0xE000)

#endif
