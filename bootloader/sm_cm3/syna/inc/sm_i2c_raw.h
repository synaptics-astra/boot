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
#ifndef __SM_I2C_RAW__HEADER__
#define __SM_I2C_RAW_HEADER__

int i2c_master_init(int id, int speed);
//int i2c_master_read(int id, int target_addr, int num_read, unsigned char *pdata);
//int i2c_master_write(int id, int target_addr, int num_write, unsigned int send_data);
int i2c_master_write_and_read(int id, int target_addr, unsigned char* wbuf, int num_write, unsigned int* rbuf, int num_read);
//int i2c_master_write_and_read_wrapper(int id, int target_addr, int num_write, int num_read, unsigned int send_data);
//int i2c_master_write_with_mask(int id, int target_addr, int byte_addr, int byte_value, int byte_mask);

#endif /* __SM_I2C_RAW_HEADER__ */
