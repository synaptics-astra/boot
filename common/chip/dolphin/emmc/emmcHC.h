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
#ifndef __SDIO_CONTROLLER_H
#define __SDIO_CONTROLLER_H

#include "sdmmc_api.h"
#include "timer.h"

//Function Prototypes
void EMMCHC_StopInternalBusClock(P_MM4_SDMMC_CONTEXT_T pContext);
void EMMCHC_StartInternalBusClock(P_MM4_SDMMC_CONTEXT_T pContext);
void EMMCHC_StartBusClock(P_MM4_SDMMC_CONTEXT_T pContext);
void EMMCHC_StopBusClock (P_MM4_SDMMC_CONTEXT_T pContext);
void EMMCHC_SetBusRate(P_MM4_SDMMC_CONTEXT_T pContext, unsigned int rate);
void EMMCHC_EnableDisableIntSources(P_MM4_SDMMC_CONTEXT_T pContext, unsigned char Desire);
void EMMCHC_SetDataTimeout(P_MM4_SDMMC_CONTEXT_T pContext, unsigned char CounterValue);
void EMMCHC_DataSWReset(P_MM4_SDMMC_CONTEXT_T pContext);
void EMMCHC_FullSWReset(P_MM4_SDMMC_CONTEXT_T pContext);
void EMMCHC_HighSpeedSelect(P_MM4_SDMMC_CONTEXT_T pContext, UINT_T hs_mode);
void EMMCHC_host_mode(P_MM4_SDMMC_CONTEXT_T pContext, UINT_T uhs_mode);

void EMMCHC_SendDataCommand(P_MM4_SDMMC_CONTEXT_T pContext,
                  UINT_T Cmd,
                  UINT_T  Argument,
                  UINT_T BlockType,
                  UINT_T DataDirection,
                  UINT_T ResType);

void EMMCHC_SendSetupCommand(P_MM4_SDMMC_CONTEXT_T pContext,
                  UINT_T Cmd,
                  UINT_T CmdType,
                  UINT_T Argument,
                  UINT_T ResType);

void EMMCHC_SendDataCommandNoAuto12(P_MM4_SDMMC_CONTEXT_T pContext,
                  UINT_T Cmd,
                  UINT_T  Argument,
                  UINT_T BlockType,
                  UINT_T DataDirection,
                  UINT_T ResType);

extern UINT_T EMMCHC_SetControllerVoltage (P_MM4_SDMMC_CONTEXT_T pContext, UINT_T vcc);

extern void EMMCHC_EnableCmdInterrupt(P_MM4_SDMMC_CONTEXT_T pContext, unsigned int enable);

extern void EMMCHC_cdInt_enable(P_MM4_SDMMC_CONTEXT_T pContext, UINT8_T Desire);

#endif
