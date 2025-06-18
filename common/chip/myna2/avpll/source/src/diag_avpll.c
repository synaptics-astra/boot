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

#if (BERLIN_CHIP_VERSION == BERLIN_BG4_CD || \
        BERLIN_CHIP_VERSION == BERLIN_BG4_CDP)
#define USE_AVPLL_V4G_600M  (1)
#else
#define USE_AVPLL_V4G_600M  (0)
#endif

#if ((BERLIN_CHIP_VERSION == BERLIN_BG4_CT)||(BERLIN_CHIP_VERSION == BERLIN_BG4_CT_A0) || (BERLIN_CHIP_VERSION >= BERLIN_BG4_CDP))
#define AVPLL_ENABLE_PLL_LOCK_CHECK
#endif

#if (BERLIN_CHIP_VERSION >= BERLIN_BG4_CT)
#define AVPLL_ENABLE_OPTIMIZE_SET_VCO
#endif //BERLIN_BG4_CDP_A0

//Let us keep 100ms stabilization time for all corner cases eventhough 10ms is good enough
#define AVPLL_STABILIZATION_DELAY_MS   100

#define IS_AUDIO_CHANNEL(x)                 ((x==3) || (x==4))

extern void avpll_clock_switch(PLL_SWITCH_TYPE toavpll, PLL_CLK_CFG_OPT_FLAG opt_flag);
extern void avpll_clock_reconfig(void);
extern void avpll_clock_init(void);

#if USE_AVPLL_V4G_600M
static unsigned int v4g_switch_saved;
#endif

//1KPPM is determined by video frequence and 59Hz/60Hz mode
static int ppm1kSelectionTable[2][6] =
{
    {-1, 0, 0, -1, 0, -1}, //59.94Hz 1KPPM flag
    { 0, 1, 1,  0, 1,  0}, //60Hz 1KPPM flag
};

static double diag_videoFreqs[] =
{
    //video frequencies, pixel freq
    25.17482517,
    27,
    54,
    74.17582418,
    108,
    148.3516484
};

typedef struct
{
    int vco_freq_index;
    float clkout_freq; //in MHz
    int freq_offset; //signed number
    unsigned int p_sync2;
    unsigned int p_sync1;
    unsigned int post_div;
    unsigned int post_div_0p5;
    unsigned int invalid; //this indicate it is a valid setting or not: 0 is valid, 1 is not
} CLK_FREQ;

static CLK_FREQ clk_freqs_computed[VCO_FREQ_2_970G+1];
//for channel 1 to 7, index starts at 1, [0][0] and [1][0] are not used
static float current_freq[2][8];

//all the VCO freq reqired for video and audio in MHz
double diag_vcoFreqs[]=
{
    1512,	//8 bit HDMI and 12 bit HMDI
    1620,	//12 bit HDMI
    1856.25,//10 bit HDMI
    2160,	//8 bit HDMI
    2227.5,	//12 bit HDMI
    2520,	//10 bit HDMI
    2700,	//10 bit HDMI
    2970,   //8 bit HDMI
};

//from Section 7 table
unsigned char diag_avpllRegFBDIV[]=
{
    60,	//VCO_FREQ_1_512G,
    65, //VCO_FREQ_1_620G,
    74, //VCO_FREQ_1_856G,
    86, //VCO_FREQ_2_160G,
    89, //VCO_FREQ_2_227G,
    101, //VCO_FREQ_2_520G,
    108, //VCO_FREQ_2_700G,
    119, //VCO_FREQ_2_970G,
};

//from Section 7 table, bit18 is sign bit
//Note that sign bit=0 for negative
//sign bit=1 for positive
unsigned int diag_avpllRegFREQ_OFFSET_C8[]=
{
    (33554),	//VCO_FREQ_1_512G,
    ((1<<18)|(12905)), 	//VCO_FREQ_1_620G,
    (14169), 	//VCO_FREQ_1_856G,
    (19508), 	//VCO_FREQ_2_160G,
    (4712),	//VCO_FREQ_2_227G
    ((1<<18)|(8305)), 	//VCO_FREQ_2_520G,
    (00000), 	//VCO_FREQ_2_700G,
    ((1<<18)|(7049)), 	//VCO_FREQ_2_970G,
};

unsigned int diag_avpllRegSPEED[]=
{
    0x2,	//1.5G<F<=1.7G , for 1.512G
    0x2,	//1.5G<F<=1.7G , for 1.62G
    0x3,	//1.7G<F<=1.9G , for 1.856G
    //	0x4,	//1.9G<F<=2.1G
    0x5,	//2.1G<F<=2.3G , for 2.16G
    0x5,	//2.1G<F<=2.3G , for 2.227G
    //	0x6,	//2.3G<F<=2.45G
    0x7,	//2.45G<F<=2.6G, for 2.52G
    0x8, 	//2.6G<F<= 2.75G, for 2.7G
    //	0x9, 	//2.75G<F<= 2.9G
    0xa,	//2.9G<F<=3.0G, for 2.97G
};


/*Interpolator current setting for different frequency
  VCO Frequency '  INTPI
 */
unsigned int diag_avpllRegINTPI[]=
{
    0x3,	//for 1.512G
    0x3,	//for 1.62G
    0x5,	//for 1.856G
    0x7,	//for 2.16G
    0x7,	//for 2.227G
    0x9, 	//for 2.52G
    0xa, 	//for 2.7G
    0xb,	//for 2.97G
};

unsigned int diag_avpllRegINTPR[]=
{
    0x4,	//for 1.512G
    0x4,	//for 1.62G
    0x4,	//for 1.856G
    0x3,	//for 2.16G
    0x3,	//for 2.227G
    0x3, 	//for 2.52G
    0x1, 	//for 2.7G
    0x1,	//for 2.97G
};

int diag_pll_A_VCO_Setting = VCO_FREQ_2_227G;
int diag_pll_B_VCO_Setting = VCO_FREQ_1_620G;

//this is good for all VCOs except 2520GHz
static unsigned int audio_freq1[] =
{
    8467200  ,
    11289600 ,
    90316800 ,
    16934400 ,
    22579200 ,
    33868800 ,
    6144000 ,
    8192000  ,
    65536000 ,
    12288000 ,
    16384000 ,
    24576000 ,
    9216000  ,
    12288000  ,
    98304000 ,
    18432000 ,
    24576000 ,
    36864000 ,
};

//this is for 2520GHz
static unsigned int audio_freq2[] =
{
    705600     ,
    1411200    ,
    2822400    ,
    5644800    ,
    8467200    ,
    11289600   ,
    16934400   ,
    22579200   ,
    33868800   ,
    45158400   ,
    90316800   ,
    512000     ,
    1024000    ,
    2048000    ,
    4096000    ,
    6144000    ,
    8192000    ,
    12288000   ,
    16384000   ,
    24576000   ,
    32768000   ,
    768000     ,
    1536000    ,
    3072000    ,
    6144000    ,
    9216000    ,
    12288000   ,
    18432000   ,
    24576000   ,
    36864000   ,
    49152000   ,
    98304000   ,
};


static int avpll_enabled = 0;
#if USE_AVPLL_V4G_600M
#define V4G_CLKD4_ADDR ((volatile unsigned int *)devmem_phy_to_virt(MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_v4gClk))
#define GFX_CLKD2_ADDR ((volatile unsigned int *)devmem_phy_to_virt(MEMMAP_CHIP_CTRL_REG_BASE + RA_Gbl_gfx3DCoreClk))
static unsigned int target_clk[] =
{
#if USE_AVPLL_V4G_600M
    600000000,      //VCO_FREQ_1_512G,
    575000000, //VCO_FREQ_1_620G,
    600000000, //VCO_FREQ_1_856G,
    600000000, //VCO_FREQ_2_160G,
    600000000, //VCO_FREQ_2_227G,
    600000000, //VCO_FREQ_2_520G,
    600000000, //VCO_FREQ_2_700G,
    600000000, //VCO_FREQ_2_970G,
#else
    500000000,	//VCO_FREQ_1_512G,
    500000000, //VCO_FREQ_1_620G,
    500000000, //VCO_FREQ_1_856G,
    500000000, //VCO_FREQ_2_160G,
    500000000, //VCO_FREQ_2_227G,
    500000000, //VCO_FREQ_2_520G,
    500000000, //VCO_FREQ_2_700G,
    500000000, //VCO_FREQ_2_970G,
#endif
};

// enable 1: AVPLL, 0: sys_pll
static void switch_to_avpll(int enable)
{
#ifdef CONFIG_FPGA_ENABLE
    return;
#endif

#if USE_AVPLL_V4G_600M
    volatile SIE_clkD4 regV4G;
    PHY_HOST_Bus_Read32((((SIE_clkD4 *)V4G_CLKD4_ADDR)->u32clkD4_ctrl), (regV4G.u32clkD4_ctrl));
    dbg_printf(PRN_RES, "V4G clk switch value 0x%x saved 0x%x \n", regV4G.u32clkD4_ctrl, v4g_switch_saved);
    if(enable)
    {// switch to AVPLL channel 2
        regV4G.uctrl_ClkPllSel =0;
        regV4G.uctrl_ClkPllSwitch =1;
        regV4G.uctrl_ClkSwitch =0;
        regV4G.uctrl_ClkD3Switch =0;
        dbg_printf(PRN_RES, "switch V4G core clk to AVPLL \n");
    }
    else
    {
        regV4G.u32clkD4_ctrl = v4g_switch_saved;
        dbg_printf(PRN_RES, "switch V4G core clk to SYSPLL \n");
    }
    dbg_printf(PRN_RES,"New V4G clk switch value 0x%x pllsel %d pllsw %d div %d clksw %d d3 %d \n",
            regV4G.u32clkD4_ctrl, regV4G.uctrl_ClkPllSel,
            regV4G.uctrl_ClkPllSwitch, regV4G.uctrl_ClkSel,
            regV4G.uctrl_ClkSwitch, regV4G.uctrl_ClkD3Switch);
    PHY_HOST_Bus_Write32((((SIE_clkD4 *)V4G_CLKD4_ADDR)->u32clkD4_ctrl), (regV4G.u32clkD4_ctrl));
#endif
}
#endif

static void DelayInNS(int DelayTime)
{
#if 0
    MV_TimeSpec_t   TimeSpec;
    TimeSpec.tv_sec = 0;
    TimeSpec.tv_nsec = DelayTime;
    MV_TimeSpec_Delay (&TimeSpec);
#else
    volatile int i;
    for(i=0; i<10000; i++);
#endif
}

static void cpu_cycle_count_delay(SIGN32 ns)
{
#if ((BERLIN_CHIP_VERSION >= BERLIN_BG4_CD))
    MV_OSAL_Task_Sleep(1);
#else
    unsigned int start,diff, end;

    GA_REG_WORD32_READ(0xF7E82C90, &start);

    do
    {
        GA_REG_WORD32_READ(0xF7E82C90, &end);
        if(start >= end)
            diff = start - end;
    } while(diff < ns/10);
#endif
}

void MV_OSAL_Task_Sleep(int DelayTime) {
    DelayInNS(DelayTime);
}


static void diag_assertPllReset(SIE_avPll *avPllBase)
{
    //assert reset
    //avPllBase->uctrlPLL_RESET=1;
    PHY_HOST_Bus_Read32(avPllBase->u32avPll_ctrlPLL, regSIE_avPll.u32avPll_ctrlPLL);
    regSIE_avPll.uctrlPLL_RESET = 1;
    PHY_HOST_Bus_Write32(avPllBase->u32avPll_ctrlPLL, regSIE_avPll.u32avPll_ctrlPLL);
}

//assert reset for channel1 to 7
static void diag_assertCxReset(SIE_avPll *avPllBase)
{
    int chId;

    for(chId=1; chId<=7; chId++)
    {
        //((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].uctrl_RESET = 1;
        PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl3, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl3);
        ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_RESET = 1;
        PHY_HOST_Bus_Write32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl3, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl3);
    }
}

static void diag_deassertPllReset(SIE_avPll *avPllBase)
{
    volatile int i;

    for(i=0; i<10000*10; i++);

    //deassert reset
    //avPllBase->uctrlPLL_RESET=0;
    PHY_HOST_Bus_Read32(avPllBase->u32avPll_ctrlPLL, regSIE_avPll.u32avPll_ctrlPLL);
    regSIE_avPll.uctrlPLL_RESET = 0;
    PHY_HOST_Bus_Write32(avPllBase->u32avPll_ctrlPLL, regSIE_avPll.u32avPll_ctrlPLL);
}

static void diag_setDPll(SIE_avPll *avPllBase, int enable, int p_sync1, int p_sync2, int chId)
{
    //disable DPll first
    //((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].uctrl_EN_DPLL = 0;
    PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl);
    ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_EN_DPLL = 0;
    PHY_HOST_Bus_Write32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl);

    if(!p_sync1)
    {
        if(enable) {
            dbg_printf(PRN_RES, "Warning p_sync1 is 0\n");
        }
    }

    //((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].uctrl_P_SYNC1 = p_sync1;
    PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl2, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl2);
    ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_P_SYNC1 = p_sync1;
    PHY_HOST_Bus_Write32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl2, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl2);

    //((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].uctrl_P_SYNC2 = p_sync2;
    PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl3, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl3);
    ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_P_SYNC2 = p_sync2;
    PHY_HOST_Bus_Write32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl3, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl3);

    if(enable)
    {
        //enable DPLL
        //((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].uctrl_EN_DPLL = 1;
        PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl);
        ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_EN_DPLL = 1;
        PHY_HOST_Bus_Write32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl);
    }
}

static void diag_setVDDL_VCOref(SIE_avPll *avPllBase)
{
#ifdef CONFIG_FPGA_ENABLE
    return;
#endif
    //T32avPll_ctrlPLL ctrlPLL;
    //ctrlPLL.u32 = avPllBase->u32avPll_ctrlPLL;
    PHY_HOST_Bus_Read32(avPllBase->u32avPll_ctrlPLL, regSIE_avPll.u32avPll_ctrlPLL);

    //ctrlPLL.uctrlPLL_VCO_REF1P45_SEL = 0x3;
    //ctrlPLL.uctrlPLL_VDDL = 0xF;// set VDDL to 1.16v
    regSIE_avPll.uctrlPLL_VCO_REF1P45_SEL = 0x3;
    regSIE_avPll.uctrlPLL_VDDL = 0xF;// set VDDL to 1.16v

    //* (volatile unsigned int *)(&(avPllBase->u32avPll_ctrlPLL)) = ctrlPLL.u32;
    PHY_HOST_Bus_Write32(avPllBase->u32avPll_ctrlPLL, regSIE_avPll.u32avPll_ctrlPLL);
}

//assert reset for channel1 to 7
static void diag_deassertCxReset(SIE_avPll *avPllBase)
{
    int chId;

    for(chId=1; chId<=7; chId++)
    {
        //((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].uctrl_RESET = 0;
        PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl3, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl3);
        ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_RESET = 0;
        PHY_HOST_Bus_Write32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl3, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl3);
    }
}

void diag_setChanOffset(SIE_avPll *avPllBase, int offset, int chId)
{
    unsigned int reg_offset = 0;

    dbg_printf(PRN_RES, "avPllBase(0x%X), offset(%d),  chId(%d)\n", avPllBase, offset, chId);
    if(offset>0)
        reg_offset = (1<<18) | (offset) ;
    else
        reg_offset = -offset;

    dbg_printf(PRN_INFO, "set 0x%X offset to 0x%x for C%d\n", avPllBase, reg_offset, chId);

    //set offset register
    //((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].uctrl_FREQ_OFFSET = reg_offset;
    PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl1, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl1);
    ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_FREQ_OFFSET = reg_offset;
    PHY_HOST_Bus_Write32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl1, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl1);

    //toggle the offset_rdy bit
    {
        //((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].uctrl_FREQ_OFFSET_READY = 1;
        PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl1, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl1);
        ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_FREQ_OFFSET_READY = 1;
        PHY_HOST_Bus_Write32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl1, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl1);

        //add some delay because need to be asserted by 30ns
        if(IS_AUDIO_CHANNEL(chId)) {
            DelayInNS(500);
        }
        else {
            cpu_cycle_count_delay(200);
        }

        //((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].uctrl_FREQ_OFFSET_READY = 0;
        PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl1, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl1);
        ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_FREQ_OFFSET_READY = 0;
        PHY_HOST_Bus_Write32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl1, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl1);
    }
}

void diag_set_Post_Div(SIE_avPll *avPllBase, int div, int chId)
{
    //((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].uctrl_POSTDIV = div;
    PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl);
    ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_POSTDIV = div;
    PHY_HOST_Bus_Write32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl);
}

void diag_set_Post_0P5_Div(SIE_avPll *avPllBase, int enable, int chId)
{
    //((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].uctrl_POSTDIV_0P5 = enable;
    PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl);
    ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_POSTDIV_0P5 = enable;
    PHY_HOST_Bus_Write32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl);
}

#define OFFSET_1KPPM_INC	(4190)
#define OFFSET_1KPPM_DEC	(-4194)

//this is done by using offset and dpll
//inc=1 means increase freq to 1001/1000
//inc=0 means disable 1KPPM (by setting offset to 0 and disable DPLL)
//inc=-1 means decrease freq to 1000/1001
void diag_set1KPPM(SIE_avPll *avPllBase, int inc, int chId)
{
    dbg_printf(PRN_RES, "avPllBase=0x%X, inc=%i, chId=%i\n", avPllBase, inc, chId);
    if(inc)
    {
        if(inc>0)
        {
            //increase by 1KPPM
            diag_setChanOffset(avPllBase, OFFSET_1KPPM_INC, chId);
            diag_setDPll(avPllBase, 1, 1000, 1001, chId);
        }
        else
        {
            //decrease by 1KPPM
            diag_setChanOffset(avPllBase, OFFSET_1KPPM_DEC, chId);
            diag_setDPll(avPllBase, 1, 1001, 1000, chId);
        }
    }
    else
    {
        //set offset to 0 and disable DPLL
        diag_setChanOffset(avPllBase, 0, chId);

#if (BERLIN_CHIP_VERSION < BERLIN_BG5_CT)
        if (chId == 1) {
            diag_setDPll(avPllBase, 1, 1, 1, chId);
        }
        else {
            diag_setDPll(avPllBase, 0, 0, 0, chId);
        }
#else
        diag_setDPll(avPllBase, 0, 0, 0, chId);
#endif
    }
}

static void diag_powerDownChannel(SIE_avPll *avPllBase, int chId)
{
    PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl1, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl1);
    ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_PU = 0;
    PHY_HOST_Bus_Write32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl1, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl1);
}

static void diag_powerUpChannel(SIE_avPll *avPllBase, int chId)
{
#if (BERLIN_CHIP_VERSION == BERLIN_BG4_CDP_A0)
    //Don't power up AVPLL-CH7 : unused (previously used for TMDS clk, currently using TxPLL)
    if(chId == 7) {
        return;
    }
#endif
    PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl1, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl1);
    ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_PU = 1;
    PHY_HOST_Bus_Write32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl1, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl1);
}

void diag_powerDown(SIE_avPll *avPllBase)
{
    int chId;

    //avPllBase->uctrlPLL_PU = 0;
    PHY_HOST_Bus_Read32(avPllBase->u32avPll_ctrlPLL, regSIE_avPll.u32avPll_ctrlPLL);
    regSIE_avPll.uctrlPLL_PU = 0;
    PHY_HOST_Bus_Write32(avPllBase->u32avPll_ctrlPLL, regSIE_avPll.u32avPll_ctrlPLL);

    for(chId=1; chId<=7; chId++)
    {
        diag_powerDownChannel(avPllBase, chId);
    }


    //avPllBase->ie_C8.uctrl_PU = 0;
    PHY_HOST_Bus_Read32((avPllBase->ie_C8).u32avpllCh8_ctrl1, (regSIE_avPll.ie_C8).u32avpllCh8_ctrl1);
    (regSIE_avPll.ie_C8).uctrl_PU = 0;
    PHY_HOST_Bus_Write32((avPllBase->ie_C8).u32avpllCh8_ctrl1, (regSIE_avPll.ie_C8).u32avpllCh8_ctrl1);
}

void diag_powerUp(SIE_avPll *avPllBase)
{
    int chId;

    for(chId=1; chId<=7; chId++)
    {
        diag_powerUpChannel(avPllBase, chId);
    }


    //avPllBase->ie_C8.uctrl_PU = 1;
    PHY_HOST_Bus_Read32((avPllBase->ie_C8).u32avpllCh8_ctrl1, (regSIE_avPll.ie_C8).u32avpllCh8_ctrl1);
    (regSIE_avPll.ie_C8).uctrl_PU = 1;
    PHY_HOST_Bus_Write32((avPllBase->ie_C8).u32avpllCh8_ctrl1, (regSIE_avPll.ie_C8).u32avpllCh8_ctrl1);

    //avPllBase->uctrlPLL_PU = 1;
    PHY_HOST_Bus_Read32(avPllBase->u32avPll_ctrlPLL, regSIE_avPll.u32avPll_ctrlPLL);
    regSIE_avPll.uctrlPLL_PU = 1;
    PHY_HOST_Bus_Write32(avPllBase->u32avPll_ctrlPLL, regSIE_avPll.u32avPll_ctrlPLL);
}

void diag_calibrate(SIE_avPll *avPllBase, double fvco)
{
    volatile int i=100000;
    //MV_TimeSpec_t Time_Start, Time_End;

    //set the speed_thresh for current Fvco
    //avPllBase->uctrlCAL_SPEED_THRESH = (int)(fvco/100+0.5);
    PHY_HOST_Bus_Read32(avPllBase->u32avPll_ctrlCAL, regSIE_avPll.u32avPll_ctrlCAL);
    regSIE_avPll.uctrlCAL_SPEED_THRESH = (int)(fvco/100+0.5);
    PHY_HOST_Bus_Write32(avPllBase->u32avPll_ctrlCAL, regSIE_avPll.u32avPll_ctrlCAL);


    //assert PLL resebratet
    //avPllBase->uctrlPLL_RESET=1;
    PHY_HOST_Bus_Read32(avPllBase->u32avPll_ctrlPLL, regSIE_avPll.u32avPll_ctrlPLL);
    regSIE_avPll.uctrlPLL_RESET = 1;
    PHY_HOST_Bus_Write32(avPllBase->u32avPll_ctrlPLL, regSIE_avPll.u32avPll_ctrlPLL);

    cpu_cycle_count_delay(20000);
    //MV_Time_GetSysTime(&Time_Start);
    //avPllBase->uctrlPLL_RESET=0;
    PHY_HOST_Bus_Read32(avPllBase->u32avPll_ctrlPLL, regSIE_avPll.u32avPll_ctrlPLL);
    regSIE_avPll.uctrlPLL_RESET = 0;
    PHY_HOST_Bus_Write32(avPllBase->u32avPll_ctrlPLL, regSIE_avPll.u32avPll_ctrlPLL);

    //>20us and <50us must start calibration
    cpu_cycle_count_delay(35000);

    //avPllBase->uctrlCAL_PLL_CAL_START = 1;
    PHY_HOST_Bus_Read32(avPllBase->u32avPll_ctrlCAL, regSIE_avPll.u32avPll_ctrlCAL);
    regSIE_avPll.uctrlCAL_PLL_CAL_START = 1;
    PHY_HOST_Bus_Write32(avPllBase->u32avPll_ctrlCAL, regSIE_avPll.u32avPll_ctrlCAL);

    //MV_Time_GetSysTime(&Time_End);
    //poll PLL CAL done bit
    //while(!avPllBase->ustatus_PLL_CAL_DONE)
    while(1)
    {
        PHY_HOST_Bus_Read32(avPllBase->u32avPll_status, regSIE_avPll.u32avPll_status);
        if(regSIE_avPll.ustatus_PLL_CAL_DONE)
            break;

        if(--i<0)
        {
            dbg_printf(PRN_RES, "PLL calibration Failed\n");
            break;
        }
    }
    cpu_cycle_count_delay(20000);

    //avPllBase->uctrlCAL_PLL_CAL_START = 0;
    PHY_HOST_Bus_Read32(avPllBase->u32avPll_ctrlCAL, regSIE_avPll.u32avPll_ctrlCAL);
    regSIE_avPll.uctrlCAL_PLL_CAL_START = 0;
    PHY_HOST_Bus_Write32(avPllBase->u32avPll_ctrlCAL, regSIE_avPll.u32avPll_ctrlCAL);

}

static void diag_dump_CLK_FREQ(CLK_FREQ* pCLK_FREQ)
{
    dbg_printf(PRN_RES, "vco_freq_index=%i\n", pCLK_FREQ->vco_freq_index);
    dbg_printf(PRN_RES, "clkout_freq=%f\n", pCLK_FREQ->clkout_freq);
    dbg_printf(PRN_RES, "freq_offset=%i\n", pCLK_FREQ->freq_offset);
    dbg_printf(PRN_RES, "p_sync2=%i\n", pCLK_FREQ->p_sync2);
    dbg_printf(PRN_RES, "p_sync1=%i\n", pCLK_FREQ->p_sync1);
    dbg_printf(PRN_RES, "post_div=%i\n", pCLK_FREQ->post_div);
    dbg_printf(PRN_RES, "post_div_0p5=%i\n", pCLK_FREQ->post_div_0p5);
    dbg_printf(PRN_RES, "invalid=%i\n", pCLK_FREQ->invalid);
}

#ifdef AVPLL_ENABLE_PLL_LOCK_CHECK
static void diag_lockcheck(void)
{
    T32avPll_status reg;
    int timeout = 15;   //timeout minimum duration should be 100ms (6 CPCB0 interrupts)
    UINT32 avpllBaseAddr = MEMMAP_AVIO_GBL_BASE + RA_avPll_status;
#if (BERLIN_CHIP_VERSION >= BERLIN_BG4_CDP)
    avpllBaseAddr = (MEMMAP_AVIO_REG_BASE + AVIO_MEMMAP_AVIO_GBL_BASE + RA_avioGbl_AVPLLA);
    avpllBaseAddr += RA_avPll_status;
#endif
    while (timeout)
    {
        BFM_HOST_Bus_Read32(avpllBaseAddr, &reg.u32);
        //dbg_printf(PRN_RES, "PLL lock reg value : 0x%08X\n", reg.u32);
        if (0 == (reg.u32 & 0x1))
        {
            dbg_printf(PRN_RES, "AVPLLA is Unlock.\n");
            MV_OSAL_Task_Sleep(10);
        }
        else
        {
             dbg_printf(PRN_RES, "AVPLLA is Lock.\n");
             break;
        }
        timeout--;
    }
    dbg_printf(PRN_INFO, "AVPLLDBG: PLL lock status (%s) - (timeout:%d): 0x%x=0x%x\n",(reg.u32 & 0x1)? "LOCKED" : "UN-LOCKED", timeout, avpllBaseAddr, reg.u32);
    return;
}
#endif //AVPLL_ENABLE_PLL_LOCK_CHECK


//freq_index, 0 to 5, the 6 VCLKs
//hdmiMode, HDMI_8BIT_MODE, HDMI_10BIT_MODE, HDMI_12BIT_MODE
//frameRate, FRAME_RATE_59P94 FRAME_RATE_60
//overSampleRate, 1, 2, 4, 8, 10, 12.5, 15
void diag_videoFreq(SIE_avPll *avPllBase, int freqIndex, int hdmiMode, int frameRate, float overSampleRate, int chId)
{
    double vcoFreq, videoFreq;
    int ppm1K=ppm1kSelectionTable[frameRate][freqIndex];
    double divider;

    vcoFreqIndex=vcoSelectionTable[hdmiMode][freqIndex];

    //check to see if ppm1K is need
    vcoFreq = diag_vcoFreqs[vcoFreqIndex];
    if(ppm1K<0)
    {
        vcoFreq = vcoFreq*1000/1001;
    }

    if(ppm1K>0)
    {
        vcoFreq = vcoFreq*1001/1000;
    }
    diag_set1KPPM(avPllBase, ppm1K, chId);

    //get the video freq
    videoFreq = diag_videoFreqs[freqIndex];
    if(frameRate)
    {
        //60Hz vclk is 1001/1000 times 59.94Hz vclk
        videoFreq = videoFreq*1001/1000;
    }
    dbg_printf(PRN_RES, "Video freq=%f\n", videoFreq);

    divider = vcoFreq*2/videoFreq/overSampleRate;
    dbg_printf(PRN_RES, "AV post divider is %f %d\n", divider/2, (int)(divider+0.5)/2);

    //disable dividers
    diag_set_Post_Div(avPllBase, 0 , chId);
    diag_set_Post_0P5_Div(avPllBase, 0 , chId);

    //this enables post divider
    if(divider>2)
    {
        diag_set_Post_Div(avPllBase, ((int)(divider+0.5)/2) , chId);
    }

    if(((int)(divider+0.5))&1)
    {
        //fractional divider, use POSTDIV_0P5_CX
        diag_set_Post_0P5_Div(avPllBase, 1 , chId);
    }

    dbg_printf(PRN_RES, "PLL(%x) C%d=%f\n", avPllBase, chId, videoFreq*overSampleRate);
#if ((BERLIN_CHIP_VERSION >= BERLIN_BG4_CD))
    gAVPLLA_Channel_OutputClock[chId] = (unsigned int)(videoFreq*overSampleRate*1000000);
#else
    if(((unsigned int)avPllBase&0xffff0000)== MEMMAP_AVIO_GBL_BASE)
        gAVPLLA_Channel_OutputClock[chId] = (unsigned int)(videoFreq*overSampleRate*1000000);
    else
        gAVPLLB_Channel_OutputClock[chId] = (unsigned int)(videoFreq*overSampleRate*1000000);
#endif

    diag_dump_CLK_FREQ(&(clk_freqs_computed[freqIndex]));

#ifdef AVPLL_ENABLE_PLL_LOCK_CHECK
    diag_lockcheck();
#endif //AVPLL_ENABLE_PLL_LOCK_CHECK
}

//freqIndex is the vco freq index into clk_freqs_computed[] array
int diag_clockFreq_computed(SIE_avPll *avPllBase, int freqIndex, int chId)
{
    int diag_pll_VCO_Setting;

    if(avPllBase == (SIE_avPll *)AVPLL_A_ADDR)
    {
        diag_pll_VCO_Setting=diag_pll_A_VCO_Setting;
        dbg_printf(PRN_RES, "set A%d to %fMHz\n", chId, clk_freqs_computed[freqIndex].clkout_freq);
    }
    else
    {
        diag_pll_VCO_Setting=diag_pll_B_VCO_Setting;
        dbg_printf(PRN_RES, "set B%d to %fMHz\n", chId, clk_freqs_computed[freqIndex].clkout_freq);
    }

    if((unsigned int)freqIndex > (sizeof(clk_freqs_computed) / sizeof(CLK_FREQ)))
    {
        dbg_printf(PRN_RES, "VCO Freq Index not found\n");
        return 1;
    }

    if(clk_freqs_computed[freqIndex].invalid)
    {
        dbg_printf(PRN_RES, "Frequency entry is invalid!\n");
        return 1;
    }

    //change offset
    diag_setChanOffset(avPllBase, clk_freqs_computed[freqIndex].freq_offset, chId);

    //change p_sync
    diag_setDPll(avPllBase, (clk_freqs_computed[freqIndex].p_sync1!=0),
            clk_freqs_computed[freqIndex].p_sync1,
            clk_freqs_computed[freqIndex].p_sync2, chId);

    //update now div
    diag_set_Post_Div(avPllBase, clk_freqs_computed[freqIndex].post_div, chId);
    diag_set_Post_0P5_Div(avPllBase, clk_freqs_computed[freqIndex].post_div_0p5, chId);

    dbg_printf(PRN_RES, "-------------clk_freqs_computed[%i]----------------------------\n", freqIndex);
    diag_dump_CLK_FREQ(&(clk_freqs_computed[freqIndex]));
    return 0;
}

void diag_avpllFreq(SIE_avPll *avPllBase, int chId)
{
    double vco_freq;
    double freq_from_dpll, freq_from_offset;
    int offset, sync1, sync2, post_div, post_0p5_div;
    int enDPLL;

    dbg_printf(PRN_RES, "avPllBase=0x%x, diag_pll_A_VCO_Setting=%i, diag_pll_B_VCO_Setting=%i\n", \
            avPllBase, diag_pll_A_VCO_Setting, diag_pll_B_VCO_Setting);
    dbg_printf(PRN_RES, "diag_vcoFreqs[diag_pll_A_VCO_Setting]=%f, diag_vcoFreqs[diag_pll_B_VCO_Setting]=%f\n", \
            diag_vcoFreqs[diag_pll_A_VCO_Setting], diag_vcoFreqs[diag_pll_B_VCO_Setting]);
    if(avPllBase == (SIE_avPll *)AVPLL_A_ADDR)
        vco_freq=diag_vcoFreqs[diag_pll_A_VCO_Setting];
    else
        vco_freq=diag_vcoFreqs[diag_pll_B_VCO_Setting];

    //find offset, sync1, sync2, divHDMI, divAV1, divAV2, divAV3 for this channel
    //offset = 		((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].uctrl_FREQ_OFFSET;
    PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl1, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl1);
    offset = ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_FREQ_OFFSET;

    //sync1 =  		((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].uctrl_P_SYNC1;
    PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl2, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl2);
    sync1 = ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_P_SYNC1;

    //sync2 =  		((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].uctrl_P_SYNC2;
    PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl3, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl3);
    sync2 = ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_P_SYNC2;

    //post_div = 		((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].uctrl_POSTDIV;
    PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl);
    post_div = ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_POSTDIV;

    //post_0p5_div = 	((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].uctrl_POSTDIV_0P5;
    PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl);
    post_0p5_div = ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_POSTDIV_0P5;

    //enDPLL = 		((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].uctrl_EN_DPLL;
    PHY_HOST_Bus_Read32(((SIE_avpllCh *)(&(avPllBase->ie_C1)))[chId-1].u32avpllCh_ctrl, ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].u32avpllCh_ctrl);
    enDPLL = ((SIE_avpllCh *)&(regSIE_avPll.ie_C1))[chId-1].uctrl_EN_DPLL;


    dbg_printf(PRN_RES, "offset=%i, sync1=%i, sync2=%i\n", offset, sync1, sync2);
    dbg_printf(PRN_RES, "post_div=%i, post_0p5_div=%i, enDPLL=%i\n", \
            post_div, post_0p5_div, enDPLL);

    if(enDPLL != 0)
        freq_from_dpll = vco_freq*sync2/sync1;
    else
        freq_from_dpll = vco_freq;

    if(offset & (1<<18))
    {
        offset = (offset & ((1<<18)-1));
    }
    else
    {
        offset = -offset;
    }
    freq_from_offset = 1/(1-(double)offset/4194304)*vco_freq;

    if(post_div)
    {
        freq_from_offset = freq_from_offset/(post_div+post_0p5_div*.5);
        freq_from_dpll = freq_from_dpll/(post_div+post_0p5_div*.5);
    }

    dbg_printf(PRN_RES, "PLLA C%d Freq is: %fMHz(Offset) %fMHz(DPLL)\n", chId, freq_from_offset, freq_from_dpll);

    if(avPllBase == (SIE_avPll *)AVPLL_A_ADDR)
        current_freq[0][chId]=freq_from_dpll;
    else
        current_freq[1][chId]=freq_from_dpll;

}

void diag_vcoFreq(SIE_avPll *avPllBase)
{
    double ref_freq=25.0;
    double freq_from_offset __attribute__((unused));
    int offset, fbDiv;

    //offset = (avPllBase->ie_C8).uctrl_FREQ_OFFSET;
    PHY_HOST_Bus_Read32((avPllBase->ie_C8).u32avpllCh8_ctrl1, (regSIE_avPll.ie_C8).u32avpllCh8_ctrl1);
    offset = (regSIE_avPll.ie_C8).uctrl_FREQ_OFFSET;

    //fbDiv = avPllBase->uctrlPLL_FBDIV;
    PHY_HOST_Bus_Read32(avPllBase->u32avPll_ctrlPLL, regSIE_avPll.u32avPll_ctrlPLL);
    fbDiv = regSIE_avPll.uctrlPLL_FBDIV;

    if(offset& (1<<18))
    {
        offset = (offset & ((1<<18)-1));
    }
    else
    {
        offset = -offset;
    }

    freq_from_offset = ref_freq*fbDiv*(1 - (double)offset/4194304);

    dbg_printf(PRN_RES, "PLLA VCO Freq is: %f\n", freq_from_offset);
}

static int gcd(unsigned int m, unsigned int n)
{
    dbg_printf(PRN_INFO, "R=%d, Q=%d\n", m, n);

    int rem;
    while(n!=0)
    {
        rem=m%n;
        m=n;
        n=rem;
    }

    dbg_printf(PRN_INFO, "S=%d\n", m);
    return(m);
}


//target_freq is in Hz
//compute channel post div, freq_offset for target freq
int diag_compute_freq_setting(unsigned int vco_freq_index, unsigned int target_freq)
{
    double offset, offset_percent;
    double divider;
    double ratio;
    int int_divider;

    memset(&(clk_freqs_computed[vco_freq_index]), 0, sizeof(CLK_FREQ));

    clk_freqs_computed[vco_freq_index].vco_freq_index=vco_freq_index;
    clk_freqs_computed[vco_freq_index].clkout_freq=(float)target_freq/1000000;

    //.5 divider is only used when divider is less than 24
    //This matches the settings in audio freq table in the IP doc

    ratio = diag_vcoFreqs[vco_freq_index]*1000000/target_freq;

    if(ratio < 24)
    {
        //allow 0.5 divider, round to closest 0.5
        int_divider = (int)(ratio*2+0.5);
    }
    else
    {
        //round to closest int
        int_divider = ((int)(ratio+0.5))*2;
    }
    divider = ((double)int_divider)/2;

    dbg_printf(PRN_INFO, "int_div=%d, divider=%f\n", int_divider, divider);

    clk_freqs_computed[vco_freq_index].post_div_0p5= (int_divider&1);
    clk_freqs_computed[vco_freq_index].post_div=((int)(divider));

    //now figure out the offset
    offset_percent = (target_freq*divider/1000000 - diag_vcoFreqs[vco_freq_index])/diag_vcoFreqs[vco_freq_index];

    dbg_printf(PRN_INFO, "offset_percent=%f\n", offset_percent);

    offset = 4194304*offset_percent/(1+offset_percent);

    dbg_printf(PRN_INFO, "offset=%f\n", offset);


    if(offset<=-262144 || offset>= 262144)
    {
        //offset cannot be achieved
        clk_freqs_computed[vco_freq_index].invalid = 1;
        return 1;
    }
    else
    {
        unsigned int vco_ratio, freq_ratio, gcd_val;

        //for rounding
        if(offset>0) offset+=0.5;
        else offset-=0.5;

        clk_freqs_computed[vco_freq_index].freq_offset = (int)(offset);
        dbg_printf(PRN_INFO, "target=%d, divider=%f, target_freq*divider=%f\n", target_freq, divider, (target_freq*divider));
        gcd_val= gcd(diag_vcoFreqs[vco_freq_index]*1000000, target_freq*divider);
        vco_ratio = (int)(diag_vcoFreqs[vco_freq_index]*1000000/gcd_val);
        freq_ratio = (int)(target_freq*divider/gcd_val);
        dbg_printf(PRN_INFO, "T=%d, U=%d\n", freq_ratio, vco_ratio);

#if (BERLIN_CHIP_VERSION >= BERLIN_BG4_CD)
        if(freq_ratio>= 131072 || vco_ratio>= 131072) //psync has 17 BITs
        {
            //offset cannot be achieved
            clk_freqs_computed[vco_freq_index].invalid = 1;
        }
        else {

            if((gcd_val/1000)<1000)
            {
                clk_freqs_computed[vco_freq_index].p_sync2=freq_ratio;
                clk_freqs_computed[vco_freq_index].p_sync1=vco_ratio;
            }
            else
            {
                clk_freqs_computed[vco_freq_index].p_sync2=freq_ratio*((int)(gcd_val/1000000+0.5));
                clk_freqs_computed[vco_freq_index].p_sync1=vco_ratio*((int)(gcd_val/1000000+0.5));
            }
        }
#else
        if((gcd_val/1000)<1000)
        {
            clk_freqs_computed[vco_freq_index].p_sync2=freq_ratio;
            clk_freqs_computed[vco_freq_index].p_sync1=vco_ratio;
        }
        else
        {
            clk_freqs_computed[vco_freq_index].p_sync2=freq_ratio*((int)(gcd_val/1000000+0.5));
            clk_freqs_computed[vco_freq_index].p_sync1=vco_ratio*((int)(gcd_val/1000000+0.5));
        }
#endif
    }

    return 0;
}

int diag_clockFreq(SIE_avPll *avPllBase, int vco_freq_index, unsigned int target_freq, int chId)
{
    dbg_printf(PRN_RES, "avPllBase(0x%X), vco_freq_index(%d), target_freq(%d), chId(%d)\n", avPllBase, vco_freq_index, target_freq, chId);

    if(diag_compute_freq_setting(vco_freq_index, target_freq))
    {
        dbg_printf(PRN_RES, "freq %fHz cannot be achieved with VCO=%fMHz\n", (float)target_freq/1000000, diag_vcoFreqs[vco_freq_index]);
        return 1;
    }
    else
    {

        //frequency ok, set it
#if (BERLIN_CHIP_VERSION >= BERLIN_BG4_CD)
        diag_clockFreq_computed((SIE_avPll *)AVPLL_A_ADDR, vco_freq_index, chId);
        diag_avpllFreq((SIE_avPll *)AVPLL_A_ADDR,chId);
#else
        diag_clockFreq_computed(avPllBase, vco_freq_index, chId);
#endif
    }

    return 0;
}
void diag_print_freqs_computed(int i)
{
    if(clk_freqs_computed[i].invalid)
    {
        dbg_printf(PRN_RES, "%d: %4.2fGHz\t%4.3fMHz\tN\n", i,
                diag_vcoFreqs[i],
                clk_freqs_computed[i].clkout_freq);
    }
    else
    {
        dbg_printf(PRN_RES, "%d: %4.2fGHz\t%4.3fMHz\t%6d\t%6d\t%6d\t%d\t%d\t%1.3f\n", i,
                diag_vcoFreqs[i],
                clk_freqs_computed[i].clkout_freq,
                clk_freqs_computed[i].freq_offset,
                clk_freqs_computed[i].p_sync2,
                clk_freqs_computed[i].p_sync1,
                clk_freqs_computed[i].post_div,
                clk_freqs_computed[i].post_div_0p5,
                (((double)clk_freqs_computed[i].p_sync2-(double)clk_freqs_computed[i].p_sync1))/clk_freqs_computed[i].p_sync1);
    }
}

void diag_check_audioFreq()
{
    unsigned int i;

    dbg_printf(PRN_RES, "VCO=%f:\n", diag_vcoFreqs[VCO_FREQ_2_227G]);
    for(i=0; i<sizeof(audio_freq1)/sizeof(int); i++)
    {
        diag_compute_freq_setting(VCO_FREQ_2_227G, audio_freq1[i]);
        diag_print_freqs_computed(VCO_FREQ_2_227G);
    }


    dbg_printf(PRN_RES, "VCO=%f:\n", diag_vcoFreqs[VCO_FREQ_1_620G]);
    for(i=0; i<sizeof(audio_freq1)/sizeof(int); i++)
    {
        diag_compute_freq_setting(VCO_FREQ_1_620G, audio_freq1[i]);
        diag_print_freqs_computed(VCO_FREQ_1_620G);
    }

    dbg_printf(PRN_RES, "VCO=%f:\n", diag_vcoFreqs[VCO_FREQ_1_512G]);
    for(i=0; i<sizeof(audio_freq1)/sizeof(int); i++)
    {
        diag_compute_freq_setting(VCO_FREQ_1_512G, audio_freq1[i]);
        diag_print_freqs_computed(VCO_FREQ_1_512G);
    }

    dbg_printf(PRN_RES, "VCO=%f:\n", diag_vcoFreqs[VCO_FREQ_2_700G]);
    for(i=0; i<sizeof(audio_freq1)/sizeof(int); i++)
    {
        diag_compute_freq_setting(VCO_FREQ_2_700G, audio_freq1[i]);
        diag_print_freqs_computed(VCO_FREQ_2_700G);
    }

    dbg_printf(PRN_RES, "VCO=%f:\n", diag_vcoFreqs[VCO_FREQ_1_856G]);
    for(i=0; i<sizeof(audio_freq1)/sizeof(int); i++)
    {
        diag_compute_freq_setting(VCO_FREQ_1_856G, audio_freq1[i]);
        diag_print_freqs_computed(VCO_FREQ_1_856G);
    }

    dbg_printf(PRN_RES, "VCO=%f:\n", diag_vcoFreqs[VCO_FREQ_2_520G]);
    for(i=0; i<sizeof(audio_freq2)/sizeof(int); i++)
    {
        diag_compute_freq_setting(VCO_FREQ_2_520G, audio_freq2[i]);
        diag_print_freqs_computed(VCO_FREQ_2_520G);
    }

    dbg_printf(PRN_RES, "VCO=%f:\n", diag_vcoFreqs[VCO_FREQ_2_160G]);
    for(i=0; i<sizeof(audio_freq1)/sizeof(int); i++)
    {
        diag_compute_freq_setting(VCO_FREQ_2_160G, audio_freq1[i]);
        diag_print_freqs_computed(VCO_FREQ_2_160G);
    }

    dbg_printf(PRN_RES, "VCO=%f:\n", diag_vcoFreqs[VCO_FREQ_2_970G]);
    for(i=0; i<sizeof(audio_freq1)/sizeof(int); i++)
    {
        diag_compute_freq_setting(VCO_FREQ_2_970G, audio_freq1[i]);
        diag_print_freqs_computed(VCO_FREQ_2_970G);
    }
}


void diag_videoFreq_A(int freqIndex, int hdmiMode, int frameRate, float overSampleRate, int chId)
{
}

void diag_videoFreq_B(int freqIndex, int hdmiMode, int frameRate, float overSampleRate, int chId)
{
}

#if (BERLIN_CHIP_VERSION == BERLIN_BG4_CDP_A0)
extern void avpll_set_clock_configure(PLL_CLK_CONFIG_TYPE clkConfigType);
extern void avpll_set_channel_configure(AVPLL_CHANNEL_CLK_CONFIG *pChanConfig, int nConfig);

void diag_setClockConfigure(PLL_CLK_CONFIG_TYPE clkConfigType) {
    avpll_set_clock_configure(clkConfigType);
}

void diag_setChannelConfigure(AVPLL_CHANNEL_CLK_CONFIG *pChanConfig, int nConfig) {
    avpll_set_channel_configure(pChanConfig, nConfig);
}
#endif
