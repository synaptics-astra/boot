/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright (C) 2013-2020 Synaptics Incorporated. All rights reserved.
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
 * INFORMATION CONTAINED IN THIS DOCUMENT IS PROVIDED "AS-IS", AND
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

#ifndef _AVPLL_H_
#define _AVPLL_H_

#ifdef __cplusplus
extern "C" {
#endif
// audio channel frequency
typedef enum
{
	AUDIO_FREQ_8P4672_MHZ=0,
	AUDIO_FREQ_11P2896_MHZ,
	AUDIO_FREQ_16P9344_MHZ,
	AUDIO_FREQ_22P5792_MHZ,
	AUDIO_FREQ_12P288_MHZ,
	AUDIO_FREQ_24P576_MHZ,
	AUDIO_FREQ_16P384_MHZ,
	AUDIO_FREQ_65P536_MHZ,
	AUDIO_FREQ_90P3168_MHZ,
	AUDIO_FREQ_98P304_MHZ,

	AUDIO_FREQ_22P636_MHZ,
	AUDIO_FREQ_24P636_MHZ,
	AUDIO_FREQ_16P424_MHZ,
	AUDIO_FREQ_90P544_MHZ,
	AUDIO_FREQ_98P544_MHZ,
	AUDIO_FREQ_65P696_MHZ,

	AUDIO_FREQ_22P522_MHZ,
	AUDIO_FREQ_24P514_MHZ,
	AUDIO_FREQ_16P342_MHZ,
	AUDIO_FREQ_90P088_MHZ,
	AUDIO_FREQ_98P056_MHZ,
	AUDIO_FREQ_65P368_MHZ,
} AUDIO_CLK_FREQ;

typedef struct __VPLL_CONFIG__ {
    float field_rate;
    int frame_rate_enum;
    int h_active;
    int v_active;
    int isInterlaced;
    int h_total;
    int v_total;
    float video_freq;
    float pixel_freq;
    int Dm;
    int Dn;
    float dummy;
    double frac;
    float vco_freq;
    int Dp;
    float vpll_freq;
    int vclk_div;
    float vclk;
} VPLL_CONFIG;

void AVPLL_InitClock(void);
void AVPLL_Enable(void);
void AVPLL_EnableChannel(int avpll, int id, int on);
void AVPLL_EnableMicClk(int en);
int AVPLL_Set(int groupId, int chanId, unsigned int avFreq);
int AVPLL_SetWithPPM1K(int groupId, int chanId,
        int refFreqIndex, float avFreq, int ppm1k_en);
void AVPLL_GetPPM(int grp, double *ppm_base, double *ppm_now);
double AVPLL_AdjustPPM(double ppm_delta, int grp);
int AVPLL_SetVideoFreq(int avpllGroupId, int freqIndex,
        int hdmiMode, int frameRate, float overSampleRate, int chId);
int AVPLL_SetClockFreq(int avpllGroupId, int vco_freq_index,
        unsigned int target_freq, int chId);
int AVPLL_GetChannelOutputFreq(int AVPLLIndex, int chID);
int AVPLL_GetVcoFreq(int hdmiMode, int freqIndex);

void AVPLL_Load_ResConfig(VPLL_CONFIG* vpllcfg);

void AVPLL_DBG_processPrintCmd(unsigned int printCmd);
int AVPLL_GetClkgenparams (int freq, int *Dm, int *Dn, double *Frac, int *Dp);

#ifdef __cplusplus
}
#endif

#endif

