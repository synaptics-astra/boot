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
/////////////////////////////////////////////////////////////////////////////
//                                    NOTICE                               //
//                                                                         //
//            COPYRIGHT MARVELL INTERNATIONAL LTD. AND ITS AFFILIATES      //
//                             ALL RIGHTS RESERVED                         //
//                                                                         //
//    The source code for this computer program is  CONFIDENTIAL  and a    //
//    TRADE SECRET of MARVELL  INTERNATIONAL  LTD. AND  ITS  AFFILIATES    //
//    ('MARVELL'). The receipt or possession of  this  program does not    //
//    convey any rights to  reproduce or  disclose  its contents, or to    //
//    manufacture,  use, or  sell  anything  that it  may  describe, in    //
//    whole or in part, without the specific written consent of MARVELL.   //
//    Any  reproduction  or  distribution  of this  program without the    //
//    express written consent of MARVELL is a violation of the copyright   //
//    laws and may subject you to criminal prosecution.                    //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDE_BCM_PRIMITIVES_H
#define INCLUDE_BCM_PRIMITIVES_H

// Following definitions need to be consistent with BCM f/w
typedef enum _DRM_PRIMITIVE
{
    // Dummy
    DRM_GET_VERSION                 = 0x0000,

    // Secure boot
    VERIFY_IMAGE                    = 0x1000,
    VERIFY_SPLIT_IMAGE              = 0x1001,

    //WideWine
    DRM_WV_INITIALIZE               = 0x2000,
    DRM_WV_TERMINATE                = 0x2001,
    DRM_WV_OPENSESSION              = 0x2002,
    DRM_WV_CLOSESESSION             = 0x2003,
    DRM_WV_GENERATEDERIVEDKEYS      = 0x2004,
    DRM_WV_LOADKEYS_CHECKSIGNATURE  = 0x2005,
    DRM_WV_LOADKEYS                 = 0x2006,
    DRM_WV_LOADKEYS_UPDATE          = 0x2007,
    DRM_WV_REFRESHKEYS_CHECKSIGNATURE = 0x2008,
    DRM_WV_REFRESHKEYS              = 0x2009,
    DRM_WV_REFRESHKEYS_UPDATE       = 0x200A,
    DRM_WV_GENERATENONCE            = 0x200B,
    DRM_WV_GENERATESIGNATURE        = 0x200C,
    DRM_WV_REWRAPDEVICERSAKEY       = 0x200D,
    DRM_WV_LOADDEVICERSAKEY         = 0x200E,
    DRM_WV_GENERATERSASIGNATURE     = 0x200F,
    DRM_WV_DERIVEKEYSFROMSESSIONKEY = 0x2010,
    DRM_WV_GETKEYDATA               = 0x2011,
    DRM_WV_GETRANDOM                = 0x2012,
    DRM_WV_REMOVEKEY                = 0x2013,

    // GTV CA
    DRM_GTV_CA_LOADKEY_ADVANCE      = 0x3000,
    DRM_GTV_CA_SIGN                 = 0x3001,
    DRM_GTV_CA_UNLOADKEY            = 0x3002,

    // Get prodcut lifecycle
    DRM_GET_PRODUCT_LIFE_CYCLE      = 0x4008,

    // HDCP
    DRM_HDCP_HDMI_TX_LOADKEYS       = 0x5000,

    // PlayReady

    // Standard TEE

    //USB Device Mode
    EROM_TURN_OFF_USB_DEVICE = 0x8000,
    EROM_RESET_USB_CONSOLE_BUF = 0x8001,
    EROM_TURN_ON_DEVICE        = 0x8002,
    EROM_FLUSH_USB_CONSOLE        = 0x8003,
} DRM_PRIMITIVE;

enum {
    PRODUCT_LIFECYCLE_BAD      = 0xFFFFFFFF,
    PRODUCT_LIFECYCLE_UNKNOWN  = 0x6E6B554C, /* LUkn*/
    PRODUCT_LIFECYCLE_DEV      = 0x7665444C, /* LDev */
    /* TODO: enrich later */
};

#endif //INCLUDE_BCM_PRIMITIVES_H

