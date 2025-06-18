/*
 * INTERNAL USE ONLY
 *
 * Unpublished Work Copyright (C) 2013-2020 Synaptics Incorporated.
 * All rights reserved.
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
 */


#ifndef _VPP_CFG_H_
#define _VPP_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NUM_RESS 150

/* definition of video resolution type */
enum {
    TYPE_SD = 0,
    TYPE_HD = 1,
    TYPE_UHD = 2,
};

/* definition of video scan mode */
enum {
    SCAN_PROGRESSIVE = 0,
    SCAN_INTERLACED  = 1,
};

/* definition of video frame-rate */
enum {
    FRAME_RATE_23P98 = 0,
    FRAME_RATE_24    = 1,
    FRAME_RATE_25    = 2,
    FRAME_RATE_29P97 = 3,
    FRAME_RATE_30    = 4,
    FRAME_RATE_47P96 = 5,
    FRAME_RATE_48    = 6,
    FRAME_RATE_50    = 7,
    FRAME_RATE_59P94 = 8,
    FRAME_RATE_60    = 9,
    FRAME_RATE_100   = 10,
    FRAME_RATE_119P88 = 11,
    FRAME_RATE_120   = 12,
    FRAME_RATE_89P91 = 13,
    FRAME_RATE_90    = 14
};

int video_framerate_num_den(int resID, int * num, int * den);
#ifndef _VPP_CFG_C_

/* VPP module constant tables */
extern RESOLUTION_INFO m_resinfo_table[MAX_NUM_RESS];

#endif

#ifdef __cplusplus
}
#endif

#endif

