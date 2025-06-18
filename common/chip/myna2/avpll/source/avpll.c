/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright © 2013-2020 Synaptics Incorporated. All rights reserved.
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

#include "avpll.h"
#include "diag_pll.h"
#include "global.h"
#include "OSAL_api.h"
#include "avio_memmap.h"
#include "vpp_api.h"
#include "vpp_cfg.h"

#define FRAC_BITS           24
#define FRAC_MASK           ((1<<FRAC_BITS) - 1)

#define FREQ_FACTOR         (1000)

#define VCO_HIGH_LIMIT      ((6000 * 1000 * 1000UL) / FREQ_FACTOR)
#define VCO_LOW_LIMIT       ((1200 * 1000 * 1000UL) / FREQ_FACTOR)
#define FRAC_MIN_FPFD       ((5 * 1000 * 1000UL) / FREQ_FACTOR)
#define FRAC_MAX_FPFD       ((7500 * 1000UL) / FREQ_FACTOR)

#define MAX_DP0             ((1 << 5) * 2)
#define MIN_DP0             2
#define MAX_DP0_S           (MAX_DP0 / 2)
#define MIN_DP0_S           (MIN_DP0 / 2)
#define MAX_DP1             7
#define MIN_DP1             1

#define MAX_DM              (1 << 6)
#define MAX_DN              (1 << 7)
#define DIVF_DEF_MULT       4
#define PARENT_RATE_KHZ     25000

extern int gcd(int a, int b);

VPLL_CONFIG vpll_cfg[] = {
    { 23.98, FRAME_RATE_23P98, 1280, 720, SCAN_PROGRESSIVE, 3300, 750, 23.98, 59.340659, 1, 20, 0.362637363, 6084045, 2136.263736, 36, 59.34065934, 1, 59.340659},
    { 23.98, FRAME_RATE_23P98, 1920, 1080, SCAN_PROGRESSIVE, 2750, 1125, 23.98, 74.175824, 1, 16, 0.802197802, 13458645, 1780.21978, 24, 74.17582418, 1, 74.175824},
    { 23.98, FRAME_RATE_23P98, 3840, 2160, SCAN_PROGRESSIVE, 5500, 2250, 23.98, 296.703297, 1, 16, 0.802197802, 13458645, 1780.21978, 6, 296.7032967, 1, 296.703297},
    { 24.00, FRAME_RATE_24, 1280, 720, SCAN_PROGRESSIVE, 3300, 750, 24.00, 59.400000, 1, 20, 0.384, 6442450, 2138.4, 36, 59.4, 1, 59.400000},
    { 24.00, FRAME_RATE_24, 1920, 1080, SCAN_PROGRESSIVE, 2750, 1125, 24.00, 74.250000, 1, 16, 0.82, 13757317, 1782, 24, 74.25, 1, 74.250000},
    { 24.00, FRAME_RATE_24, 3840, 2160, SCAN_PROGRESSIVE, 5500, 2250, 24.00, 297.000000, 1, 16, 0.82, 13757317, 1782, 6, 297, 1, 297.000000},
    { 29.97, FRAME_RATE_29P97, 1280, 720, SCAN_PROGRESSIVE, 3300, 750, 29.97, 74.175824, 1, 16, 0.802197802, 13458645, 1780.21978, 24, 74.17582418, 1, 74.175824},
    { 29.97, FRAME_RATE_29P97, 1920, 1080, SCAN_PROGRESSIVE, 2200, 1125, 29.97, 74.175824, 1, 16, 0.802197802, 13458645, 1780.21978, 24, 74.17582418, 1, 74.175824},
    { 29.97, FRAME_RATE_29P97, 3840, 2160, SCAN_PROGRESSIVE, 4400, 2250, 29.97, 296.703297, 1, 16, 0.802197802, 13458645, 1780.21978, 6, 296.7032967, 1, 296.703297},
    { 30.00, FRAME_RATE_30, 1280, 720, SCAN_PROGRESSIVE, 3300, 750, 30.00, 74.250000, 1, 16, 0.82, 13757317, 1782, 24, 74.25, 1, 74.250000},
    { 30.00, FRAME_RATE_30, 1920, 1080, SCAN_PROGRESSIVE, 2200, 1125, 30.00, 74.250000, 1, 16, 0.82, 13757317, 1782, 24, 74.25, 1, 74.250000},
    { 30.00, FRAME_RATE_30, 3840, 2160, SCAN_PROGRESSIVE, 4400, 2250, 30.00, 297.000000, 4, 296, 0, 0, 5940, 9, 297, 1, 297.000000},
    { 50.00, FRAME_RATE_50, 720, 576, SCAN_PROGRESSIVE, 864, 625, 50.00, 27.000000, 1, 11, 0.96, 16106127, 1296, 48, 27, 1, 27.000000},
    { 50.00, FRAME_RATE_50, 1280, 720, SCAN_PROGRESSIVE, 1980, 750, 50.00, 74.250000, 1, 16, 0.82, 13757317, 1782, 24, 74.25, 1, 74.250000},
    { 50.00, FRAME_RATE_50, 1920, 1080, SCAN_INTERLACED, 2640, 1125, 50.00, 74.250000, 1, 16, 0.82, 13757317, 1782, 24, 74.25, 1, 74.250000},
    { 50.00, FRAME_RATE_50, 1920, 1080, SCAN_PROGRESSIVE, 2640, 1125, 50.00, 148.500000, 1, 16, 0.82, 13757317, 1782, 12, 148.5, 1, 148.500000},
    { 50.00, FRAME_RATE_50, 3840, 2160, SCAN_PROGRESSIVE, 5280, 2250, 50.00, 594.000000, 1, 34, 0.64, 10737418, 3564, 6, 594, 1, 594.000000},
    { 59.94, FRAME_RATE_59P94, 640, 480, SCAN_PROGRESSIVE, 800, 525, 59.94, 25.174825, 1, 11, 0.083916084, 1407878, 1208.391608, 48, 25.17482517, 1, 25.174825},
    { 59.94, FRAME_RATE_59P94, 720, 480, SCAN_PROGRESSIVE, 858, 525, 59.94, 27.000000, 1, 11, 0.96, 16106127, 1296, 48, 27, 1, 27.000000},
    { 59.94, FRAME_RATE_59P94, 1280, 720, SCAN_PROGRESSIVE, 1650, 750, 59.94, 74.175824, 1, 16, 0.802197802, 13458645, 1780.21978, 24, 74.17582418, 1, 74.175824},
    { 59.94, FRAME_RATE_59P94, 1920, 1080, SCAN_INTERLACED, 2200, 1125, 59.94, 74.175824, 1, 16, 0.802197802, 13458645, 1780.21978, 24, 74.17582418, 1, 74.175824},
    { 59.94, FRAME_RATE_59P94, 1920, 1080, SCAN_PROGRESSIVE, 2200, 1125, 59.94, 148.351648, 1, 16, 0.802197802, 13458645, 1780.21978, 12, 148.3516484, 1, 148.351648},
    { 59.94, FRAME_RATE_59P94, 3840, 2160, SCAN_PROGRESSIVE, 4400, 2250, 59.94, 593.406593, 1, 34, 0.604395604, 10140075, 3560.43956, 6, 593.4065934, 1, 593.406593},
    { 60.00, FRAME_RATE_60, 640, 480, SCAN_PROGRESSIVE, 800, 525, 60.00, 25.200000, 1, 11, 0.096, 1610612, 1209.6, 48, 25.2, 1, 25.200000},
    { 60.00, FRAME_RATE_60, 720, 480, SCAN_PROGRESSIVE, 858, 525, 60.00, 27.027000, 1, 11, 0.97296, 16323560, 1297.296, 48, 27.027, 1, 27.027000},
    { 60.00, FRAME_RATE_60, 1280, 720, SCAN_PROGRESSIVE, 1650, 750, 60.00, 74.250000, 3, 171, 0.82, 4362076, 1782, 28, 74.25, 1, 74.250000},
    { 60.00, FRAME_RATE_60, 1920, 1080, SCAN_INTERLACED, 2200, 1125, 60.00, 74.250000, 3, 171, 0.82, 4362076, 1782, 28, 74.25, 1, 74.250000},
    { 60.00, FRAME_RATE_60, 1920, 1080, SCAN_PROGRESSIVE, 2200, 1125, 60.00, 148.500000, 4, 296, 0, 0, 5940, 19, 148.5, 1, 148.500000},
    { 60.00, FRAME_RATE_60, 3840, 2160, SCAN_PROGRESSIVE, 4400, 2250, 60.00, 594.000000, 4, 296, 0, 0, 5940, 4, 594, 1, 594.000000},
    { 60.00, FRAME_RATE_60,  800, 1280, SCAN_PROGRESSIVE,  882, 1327, 60.00, 70.2200000, 0,  11, 0.63960, 10730707, 1263.960, 8,  70.220000, 1,  70.220000},
    { 60.00, FRAME_RATE_60, 1920, 1080, SCAN_PROGRESSIVE, 2080, 1111, 60.00, 138.500000, 0,  12, 0.85000, 14260633, 1385.000, 4, 138.500000, 1, 138.500000},

    /* Dummy entry for laoding dynamic resolution info. If any info need addition, add it before this entry*/
    {     0,             0,    0,    0,                0,    0,    0,   0.0,        0.0, 0,   0,     0.0,        0,      0.0, 0,        0.0, 0,        0.0},
};

#ifdef LOG_PLL_CFG
static void log_vpll_cfg(void) {
    int i,n;
    n = sizeof(vpll_cfg) / sizeof(VPLL_CONFIG);

    printf("fld_rt\trt_en\tactive_hXv\tIsInt\ttotalhXv\tvid_fq\tpix_fq\tDm\tDn\tdummy\t\tfrac\t\tvco_fq\tDp\tvpll_fq\tvclk_div  vclk_fq\n");
    for(i=0;i<n;i++) {
        printf("%2.2f\t%d\t%4dx%4d\t%d\t%4dx%4d\t%2.2f\t%2.2f\t%d\t%d\t%0.6f\t%8.0f\t%4.2f\t%d\t%2.2f\t%d\t%2.2f\n",
            vpll_cfg[i].field_rate,
            vpll_cfg[i].frame_rate_enum,
            vpll_cfg[i].h_active,
            vpll_cfg[i].v_active,
            vpll_cfg[i].isInterlaced,
            vpll_cfg[i].h_total,
            vpll_cfg[i].v_total,
            vpll_cfg[i].video_freq,
            vpll_cfg[i].pixel_freq,
            vpll_cfg[i].Dm,
            vpll_cfg[i].Dn,
            vpll_cfg[i].dummy,
            vpll_cfg[i].frac,
            vpll_cfg[i].vco_freq,
            vpll_cfg[i].Dp,
            vpll_cfg[i].vpll_freq,
            vpll_cfg[i].vclk_div,
            vpll_cfg[i].vclk
            );
    }
}
#endif

typedef struct __SetClockFreq_Data_ {
    int pllsrc;
    int isInterlaced;
    int refresh_rate;
    int h_active;
    int v_active;
    int h_total;
    int v_total;
} SetClockFreq_Data;

//pack data in TA
void AVPLL_SetClockFreq_Pack(int *p_packed_data, SetClockFreq_Data *p_data) {
    p_packed_data[0] = p_data->pllsrc;
    p_packed_data[1] = p_data->isInterlaced << 8 | p_data->refresh_rate;
    p_packed_data[2] = p_data->h_active << 16 | p_data->v_active;
    p_packed_data[3] = 0;
}

//unpack in CA-AVPLL and pass it to get_pll_cfg() to get corresponding APLLCFG_t
void AVPLL_SetClockFreq_UnPack(int *p_packed_data, SetClockFreq_Data *p_data) {
    p_data->pllsrc = p_packed_data[0];
    p_data->isInterlaced = p_packed_data[1] >> 8;
    p_data->refresh_rate = p_packed_data[1] & 0xFF;
    p_data->h_active = p_packed_data[2] >> 16;
    p_data->v_active = p_packed_data[2] & 0xFFFF;
    p_data->h_total = p_packed_data[3] >> 16;
    p_data->v_total = p_packed_data[3] & 0xFFFF;
}

static int get_pll_cfg(SetClockFreq_Data *p_data, APLLCFG_t *p_apll_cfg) {
    int i,n;
    n = sizeof(vpll_cfg) / sizeof(VPLL_CONFIG);

    for(i=0;i<n;i++) {
        if((p_data->isInterlaced == vpll_cfg[i].isInterlaced) &&
                (p_data->refresh_rate == vpll_cfg[i].frame_rate_enum) &&
                (p_data->h_active == vpll_cfg[i].h_active) &&
                (p_data->v_active == vpll_cfg[i].v_active) &&
                (p_data->h_total == vpll_cfg[i].h_total) &&
                (p_data->v_total == vpll_cfg[i].v_total)) {
                p_apll_cfg->dm = vpll_cfg[i].Dm;
                p_apll_cfg->dn = vpll_cfg[i].Dn;
                p_apll_cfg->frac = vpll_cfg[i].frac;
                p_apll_cfg->dp = vpll_cfg[i].Dp;
                p_apll_cfg->dp1 = 0;
                return 1;
        }
    }
    return 0;
}

#ifdef LOG_PLL_CFG
static int show_SetClockFreq_Data(SetClockFreq_Data *p_data) {
        printf("pllsrc:%d, isInterlaced:%d, refresh_rate:%d, active=%dx%d\n",
                        p_data->pllsrc, p_data->isInterlaced,  p_data->refresh_rate, p_data->h_active, p_data->v_active);
    return 0;
}

static int show_pll_cfg(APLLCFG_t *p_apll_cfg) {
        printf("dm : %d, dn : %d, frac : %d, dp : %d, dp1 : %d\n",
                        p_apll_cfg->dm, p_apll_cfg->dn, p_apll_cfg->frac, p_apll_cfg->dp, p_apll_cfg->dp1);
    return 0;
}
#else
static int show_SetClockFreq_Data(SetClockFreq_Data *p_data) {
    return 0;
}

static int show_pll_cfg(APLLCFG_t *p_apll_cfg) {
    return 0;
}
#endif

void AVPLL_InitClock(void) {
}

void AVPLL_Enable(void) {
}

void AVPLL_EnableChannel(int avpll, int id, int on) {
}

void AVPLL_EnableMicClk(int en) {
}

int AVPLL_Set(int groupId, int chanId, unsigned int avFreq) {
    return 0;
}

int AVPLL_SetWithPPM1K(int groupId, int chanId, int refFreqIndex, float avFreq, int ppm1k_en) {
    return 0;
}

void AVPLL_GetPPM(int grp, double *ppm_base, double *ppm_now) {
}

double AVPLL_AdjustPPM(double ppm_delta, int grp) {
    return 0;
}

int AVPLL_SetVideoFreq(int avpllGroupId, int freqIndex, int hdmiMode, int frameRate, float overSampleRate, int chId) {
    return 0;
}


int AVPLL_SetClockFreq(int avpllGroupId, int vco_freq_index, unsigned int target_freq, int chId) {
    int packed_data[4];
    SetClockFreq_Data data;
    APLLCFG_t apll_cfg;

    packed_data[0] = avpllGroupId;
    packed_data[1] = vco_freq_index;
    packed_data[2] = target_freq;
    packed_data[3] = chId;

    //unpack in CA-AVPLL and pass it to get_pll_cfg() to get corresponding APLLCFG_t
    AVPLL_SetClockFreq_UnPack(&packed_data[0], &data);

    get_pll_cfg(&data, &apll_cfg);

    show_SetClockFreq_Data(&data);
    show_pll_cfg(&apll_cfg);
    diag_change_avpll(0+data.pllsrc, 0, 0, 0, 0, 0, 0, apll_cfg);

    return 0;
}

int AVPLL_GetChannelOutputFreq(int AVPLLIndex, int chID) {
    return 0;
}


int AVPLL_GetVcoFreq(int hdmiMode, int freqIndex) {
    return 0;
}


void AVPLL_DBG_processPrintCmd(unsigned int printCmd) {
}

void AVPLL_Load_ResConfig(VPLL_CONFIG* vpllcfg)
{
    int pllentry;

    pllentry = sizeof(vpll_cfg)/sizeof(VPLL_CONFIG);

    /* Load the pll info to the table */
    memcpy(&vpll_cfg[pllentry-1], vpllcfg, sizeof(VPLL_CONFIG));
}

int AVPLL_GetClkgenparams (int freq, int *Dm, int *Dn, double *Frac, int *Dp) {
    unsigned int dn, dm, gcdv;
    int dp0;
    unsigned long frac;
    unsigned long vco;
    unsigned long rate0 = freq*2;
    unsigned long parent_rate = PARENT_RATE_KHZ;

    dp0 = VCO_HIGH_LIMIT / rate0;
    if (dp0 > MAX_DP0_S)
        dp0 = MAX_DP0_S;

    vco = rate0 * dp0;
    dp0 = vco / rate0;

    if ((dp0 > MAX_DP0_S)) {
        dbg_printf(PRN_ERR, "dp0 invalid \n");
        return MV_VPP_EUNSUPPORT;
    }

    /* try to let vco near the high limit */
    while ((dp0 <= (MAX_DP0_S / 2))
        && (vco <= VCO_HIGH_LIMIT /2)) {
        vco *= 2;
    }

    /* caculate the dn, dm */
    gcdv = gcd(vco, parent_rate);
    dn = vco / gcdv;
    dm = parent_rate / gcdv * DIVF_DEF_MULT;
    frac = 0;
    /* If dn and dm are valid values, we don't use frac mode. */
    if ((dm > MAX_DM) || (dn > MAX_DN)) {
        /*
         * For frac mode, the FPFD is limited from 5M to 7.5M.
         * For 25M input, dm is only valid for 5 or 4, here we got
         * dm = parent / FRAC_MIN_FPFD.
         */
        dm = parent_rate / FRAC_MIN_FPFD;
        if (((parent_rate / dm) >= FRAC_MAX_FPFD)
            || (dm > MAX_DM)) {
            dbg_printf(PRN_ERR, "dm invalid \n");
            return MV_VPP_EUNSUPPORT;
        }

        frac = (vco << FRAC_BITS) * dm / (DIVF_DEF_MULT * parent_rate);
        dn = frac >> FRAC_BITS;
        frac = frac & FRAC_MASK;
    }

    *Dm = dm-1;
    *Dn = dn-1;
    *Frac = frac;
    *Dp = dp0-1;

    dbg_printf(PRN_INFO,"DN[%d] DM[%d] Frac[%lu] dp0[%d]", *Dn, *Dm, *Frac, *Dp);

    return MV_VPP_OK;
}