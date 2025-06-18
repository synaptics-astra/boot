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
#ifndef	__GALOIS_SPEED_H
#define	__GALOIS_SPEED_H
#include "com_type.h"

typedef enum SOC_FREQ_ID_T {
    SOC_FREQ_INVALID = -1,
    SOC_FREQ_DDR = 0,
    SOC_FREQ_CPU0,
    SOC_FREQ_CPU1,
    SOC_FREQ_SYSPLL,
    SOC_FREQ_SYS,
    SOC_FREQ_VSCOPE,
    SOC_FREQ_PCUBE,
    SOC_FREQ_VPP,
    SOC_FREQ_CFG,
    SOC_FREQ_PERIF,
    SOC_FREQ_GFX,
    SOC_FREQ_DRM,
    SOC_FREQ_NFCECC,
    SOC_FREQ_APP,
    SOC_FREQ_GFX3DCORE,
    SOC_FREQ_GFX3DSYS,
    SOC_FREQ_ARCREF,
    SOC_FREQ_HDMIRXFPLLREF,
    SOC_FREQ_HDMIRXT,
    SOC_FREQ_VIP,
    SOC_FREQ_SDIOXIN,
    SOC_FREQ_SDIO1XIN,
    SOC_FREQ_GFX3DSH,
    SOC_FREQ_AVIF0,
    SOC_FREQ_AVIF1,
    SOC_FREQ_GFX3DEXTRA,
    SOC_FREQ_GC360,
    SOC_FREQ_MAX_NUM,
}ENUM_SOC_FREQ_ID;

typedef struct SOC_FREQ_VECTOR_T{
    UNSG32 ddr;
    UNSG32 cpu0;
#if (BERLIN_CHIP_VERSION < BERLIN_BG2)
    UNSG32 cpu1;
#endif
    UNSG32 mempll;
    UNSG32 cpupll;
    UNSG32 syspll;
    UNSG32 sys;
    UNSG32 vscope;
    UNSG32 pcube;
    UNSG32 vpp;
    UNSG32 cfg;
#if (BERLIN_CHIP_VERSION >= BERLIN_C_0)
    UNSG32 adsp;
#endif
    UNSG32 perif;
    UNSG32 gfx;
    UNSG32 drm;
    UNSG32 nfcecc;
#if (BERLIN_CHIP_VERSION >= BERLIN_B_0)
    UNSG32 app;
#endif
#if (BERLIN_CHIP_VERSION >= BERLIN_BG2)
    UNSG32 gfx3dcore;
    UNSG32 gfx3dsys;
    UNSG32 arcref;
    UNSG32 hdmirxfpllref;
    UNSG32 hdmirxt;
    UNSG32 vip;
    UNSG32 sdioxin;
    UNSG32 sdio1xin;
    UNSG32 gfx3dextra;
    UNSG32 gc360;
#endif
}SOC_FREQ_VECTOR;


void show_speed(void) ;
int GaloisGetAllFreq(SOC_FREQ_VECTOR * freq);
UNSG32 GaloisGetCfgFrequency(void);
UNSG32 GaloisGetFrequency(int FreqID);
UNSG32 Dynamic_Lower_vMeta_Speed(void);
UNSG32 Dynamic_Recover_vMeta_Speed(void);
UNSG32 Dynamic_Lower_ZSP_Speed(void);
UNSG32 Dynamic_Recover_ZSP_Speed(void);
UNSG32 GaloisGetChipID(void);
UNSG32 GaloisGetCPUDivider(unsigned int * cpu0_clock_divider,
                           unsigned int * cpu1_clock_divider) ;
#endif
