/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"

#include "sm_type.h"
#include "sm_rt_module.h"
#include "sm_exception.h"

#include "sm_uart.h"
#include "sm_printf.h"

#include "sm_common.h"
#include "sm_timer.h"
#include "sm_io.h"
#include "sm_memmap.h"
#include "sm_apb_gpio_defs.h"

//#define TEST_GPIO

#ifdef TEST_GPIO
static void set_bits(unsigned int reg_addr, unsigned int mask, int value)
{
	unsigned int reg_value;

	BFM_HOST_Bus_Read32(reg_addr, &reg_value);
	reg_value &= ~(mask);
	reg_value |= (value&mask);
	BFM_HOST_Bus_Write32(reg_addr, reg_value);
}

#define PINMUX_DEF(a)   E_##a,
#define SM_PINMUX_PINLIST()           \
            PINMUX_DEF(TW2_SCL     )  \
            PINMUX_DEF(TW2_SDA     )  \
            PINMUX_DEF(URT1_TXD    )  \
            PINMUX_DEF(URT1_RXD    )  \
            PINMUX_DEF(HDMI_HPD    )  \
            PINMUX_DEF(HDMI_CEC    )  \
            PINMUX_DEF(TMS         )  \
            PINMUX_DEF(TDI         )  \
            PINMUX_DEF(TDO         )  \
            PINMUX_DEF(TW3_SCL     )  \
            PINMUX_DEF(TW3_SDA     )  \
            PINMUX_DEF(SPI2_SS0n   )  \
            PINMUX_DEF(SPI2_SS1n   )  \
            PINMUX_DEF(SPI2_SS2n   )  \
            PINMUX_DEF(SPI2_SS3n   )  \
            PINMUX_DEF(SPI2_SDO    )  \
            PINMUX_DEF(SPI2_SDI    )  \
            PINMUX_DEF(SPI2_SCLK   )  \
            PINMUX_DEF(URT0_TXD    )  \
            PINMUX_DEF(URT0_RXD    )  \
            PINMUX_DEF(HDMIRX_HPD  )  \
            PINMUX_DEF(HDMIRX_PWR5V)  \

typedef enum
{
    SM_PINMUX_PINLIST()		// NO ',' at end
    E_SM_PINMUX_MAX,
} SM_PINMUX_ID;

typedef struct
{
    SM_PINMUX_ID   id;
    int             value;
} pinmux_setting_t;

const pinmux_setting_t sm_pinmux_init_settings[] = {
// index      pin name            value  //  mode 0,         mode 1,         mode 2,         mode 3,         mode 4,     mode 5,     mode 6         mode 7          boot_strap
/*  0  */    {E_TW2_SCL       ,   0},    //  RX_EDID_SCL,    SM_TW2_SCL,     SM_GPIO[0],     ,               ,           ,           ,              ,               ,
/*  1  */    {E_TW2_SDA       ,   0},    //  RX_EDID_SDA,    SM_TW2_SDA,     SM_GPIO[1],     ,               ,           ,           ,              ,               ,
/*  2  */    {E_URT1_TXD      ,   0},    //  POR_B_VOUT_1P0, SM_URT1_TXD,    SM_GPIO[4],     PWM[2],         SM_TIMER[0],PORB_AVDD_LV,,             ,               ,
/*  3  */    {E_URT1_RXD      ,   0},    //  SM_GPIO[5],     SM_URT1_RXD,    CLK_25M,        PWM[3],         SM_TIMER[1],POR_VDDSOC_RSTB,,          ,               ,
/*  4  */    {E_HDMI_HPD      ,   0},    //  SM_GPIO[2],     SM_HDMI_HPD,    ,               ,               ,           ,           ,              ,               ,
/*  5  */    {E_HDMI_CEC      ,   0},    //  SM_GPIO[3],     SM_HDMI_CEC,    ,               ,               ,           ,           ,              ,               ,
/*  6  */    {E_TMS           ,   0},    //  SM_TMS,         SM_GPIO[6],     PWM[0],         ,               ,           ,           ,              ,               ,
/*  7  */    {E_TDI           ,   0},    //  SM_TDI,         SM_GPIO[7],     PWM[1],         ,               ,           ,           ,              ,               ,
/*  8  */    {E_TDO           ,   0},    //  SM_TDO,         SM_GPIO[8](O),  ,               ,               ,           ,           ,              ,               ,
/*  9  */    {E_TW3_SCL       ,   1},    //  SM_GPIO[9],     SM_TW3_SCL,     PWM[2],         ,               ,           ,           ,              ,               ,
/* 10  */    {E_TW3_SDA       ,   0},    //  SM_GPIO[10],    SM_TW3_SDA,     PWM[3],         ,               ,           ,           ,              ,               ,
/* 11  */    {E_SPI2_SS0n     ,   0},    //  SM_SPI2_SS0n,   SM_GPIO[17](O), ,               ,               ,           ,           ,              PORB_AVDD33_LV, SM_STRP[2],
/* 12  */    {E_SPI2_SS1n     ,   0},    //  SM_GPIO[16](O), SM_SPI2_SS1n,   ,               ,               ,           ,           SM_URT1_RTSn,  VDD_CPU_PORB,   SM_STRP[3]
/* 13  */    {E_SPI2_SS2n     ,   0},    //  MON_VDD1P8_OUT_1P05,SM_SPI2_SS2n,SM_GPIO[15],   PWM[0],         SM_TIMER[0],URT2_TXD,   ,              CLK_25M,        ,
/* 14  */    {E_SPI2_SS3n     ,   1},    //  SM_PWR_OK,      SM_SPI2_SS3n,   SM_GPIO[14],    PWM[1],         SM_TIMER[1],URT2_RXD,   ,              SM_URT1_CTSn,   ,
/* 15  */    {E_SPI2_SDO      ,   1},    //  SM_SPI2_SDO,    SM_GPIO[13](O), URT2_RTSn,      ,               ,           ,           ,              ,               SM_STRP[1],
/* 16  */    {E_SPI2_SDI      ,   2},    //  SM_SPI2_SDI,    SM_GPIO[12],    URT2_CTSn,      ,               ,           ,           ,              ,               ,
/* 17  */    {E_SPI2_SCLK     ,   1},    //  SM_SPI2_SCLK,   SM_GPIO[11],    ,               ,               ,           ,           ,              ,               ,
/* 18  */    {E_URT0_TXD      ,   2},    //  SM_URT0_TXD,    SM_GPIO[19](O), ,               ,               ,           ,           ,              ,               SM_STRP[0],
/* 19  */    {E_URT0_RXD      ,   1},    //  SM_URT0_RXD,    SM_GPIO[18],    ,               ,               ,           ,           ,              ,               ,
/* 20  */    {E_HDMIRX_HPD    ,   0},    //  SM_HDMIRX_HPD,  SM_GPIO[20],    ,               ,               ,           ,           ,              ,               ,
/* 21  */    {E_HDMIRX_PWR5V  ,   0},    //  SM_HDMIRX_PWR5V,SM_GPIO[21],    ,               ,               ,           ,           ,              ,               ,
};

#undef  PINMUX_DEF
#define PINMUX_DEF(a)   case E_##a: \
                        {           \
                            set_bits(base + (BA_smSysCtl_smPinMuxCntlBus_SM_##a & (~3)),    \
                                    MSK32smSysCtl_smPinMuxCntlBus_SM_##a,                   \
                                    value << LSb32smSysCtl_smPinMuxCntlBus_SM_##a);         \
                            break;  \
                        }

int sm_pinmux_write(int id, int value)
{
    unsigned int base = SM_MEMMAP_SYS_CTRL_BASE;
    switch (id)
    {
    SM_PINMUX_PINLIST()       // NO ',' at end
    default: return -1; break;
    }

    return 0;
}

#define ARRAY_NUM(a)            (sizeof(a)/sizeof(a[0]))
int pinmux_load_init_setting()
{
    int i;
    int ret = 0;
    for (i = 0; i < ARRAY_NUM(sm_pinmux_init_settings); i++)
    {
        ret |= sm_pinmux_write(sm_pinmux_init_settings[i].id, sm_pinmux_init_settings[i].value);
    }

    return ret;
}

/*TEST NO.0000
 *FUNCTION: GPIO interrupt
 *STEP: 1. configure sm pinmux
 *		1. set GPIO3 and GPIO10 pin to pin physically
 *		2. SW set GPIO3 output, and GPIO10 input
 *		3. every second GPIO3 will change level
 *		4. the interrupt will assert by GPIO10 every second
 *		5. we should put a print message in the isr function of GPIO10(ir)
 *		6. this message will print when every interrupt assert by GPIO10
 */
static void sm_gpio_test_task(void * para)
{
	static unsigned int level =0;

	dbg_printf(PRN_RES, "------pin init-------\n");
	int ret = 0;
	ret = pinmux_load_init_setting();
	if(ret != 0)
		dbg_printf(PRN_RES, "pin_init failed!\n");
	else
		dbg_printf(PRN_RES, "pin_init successed!\n");

	for ( ;; ) {
		extern int sm_gpio_set_output(int port, int value);
		sm_gpio_set_output(3, level);
		level ^= 1;

		dbg_printf(PRN_RES, "ddr_10:%d, ddr_3:%d, ddr[0x%x]:%x\n",
			(MV_SM_READ_REG32(GPIO_SWPORTA_DDR(0)) >> 10)&1,
			(MV_SM_READ_REG32(GPIO_SWPORTA_DDR(0)) >> 3)&1,
			GPIO_SWPORTA_DDR(0),
			MV_SM_READ_REG32(GPIO_SWPORTA_DDR(0)));

		dbg_printf(PRN_RES, "output_10:%d, output_3:%d, dr[0x%x]:%x\n",
			(MV_SM_READ_REG32(GPIO_SWPORTA_DR(0)) >> 10)&1,
			(MV_SM_READ_REG32(GPIO_SWPORTA_DR(0)) >> 3)&1,
			GPIO_SWPORTA_DR(0),
			MV_SM_READ_REG32(GPIO_SWPORTA_DR(0)));

		dbg_printf(PRN_RES, "input_10:%d, input_3:%d, input[0x%x]:%x\n",
			(MV_SM_READ_REG32(GPIO_EXT_PORTA(0)) >> 10)&1,
			(MV_SM_READ_REG32(GPIO_EXT_PORTA(0)) >> 3)&1,
			GPIO_EXT_PORTA(0),
			MV_SM_READ_REG32(GPIO_EXT_PORTA(0)));

		vTaskDelay(1000);
	}
}

static void __attribute__((used)) create_sm_gpio_test_task(void)
{
	xTaskCreate(sm_gpio_test_task, "sm_gpio_test", 128, NULL, TASK_PRIORITY_2, NULL);
}

DECLARE_RT_MODULE(
	sm_gpio_test,
	11,
	create_sm_gpio_test_task,
	NULL,
	NULL
);

/*TEST NO.0001
 *FUNCTION: GPIO set/get function
 *STEP: 1. configure sm pinmux
 *		2. use external message of gpio to test GPIO set/get function
 *		3. message box like this:
 *			addr: msgq_input0(0x1f200)
 *			0x00000020 0x00000000 0x00000020 0x00000000
 *			0x00000000 0x00000000 0x00000000 0x00000000
 *			0x00000016 0x00000006 0x0000000a 0x00000001
 *
 *			0x16-module_id=GPIO
 *			0x00000006-GPIO set(0x00000007-GPIO get)
 *			0x0000000a-GPIO10
 *			0x00000001-GPIO set value
 */
#endif
