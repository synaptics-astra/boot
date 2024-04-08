/******************************************************************************
*                 2018 Synaptics Incorporated. All Rights Reserved            *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF Synaptics.                   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
* OF Synaptics OR ANY THIRD PARTY. Synaptics RESERVES THE RIGHT AT ITS SOLE   *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO Synaptics.  *
* THIS CODE IS PROVIDED "AS IS". Synaptics MAKES NO WARRANTIES, EXPRESSED,    *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
*                                                                             *
*                   WARNING FOR USING THIS DIAG CODE                          *
*   1. Please take your own responsibility if you refer DIAG source code for  *
*      any type software development.                                         *
*   2. DIAG code is mainly for internal validation purpose, and not intend to *
*      be used in production SW.                                              *
*   3. DIAG code is subject to change without notice.                         *
*******************************************************************************/
#ifndef __SM_FXL6408__HEADER__
#define __SM_FXL6408__HEADER__

void sm_fxl6408_read_deviceID(int id, int slave_addr);
int sm_fxl6408_set_iodir(int id, int slave_addr, unsigned int mask, unsigned int value);
int sm_fxl16408_set_output_state(int id, int slave_addr, unsigned int mask, unsigned int value);
int sm_fxl16408_set_output_enable(int id, int slave_addr, unsigned int mask, unsigned int value);
int sm_fxl16408_gpios_write(int id, int slave_addr, unsigned int mask, unsigned int value);

#endif /* __SM_FXL6408__HEADER__ */
