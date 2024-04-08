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
#ifndef _LOAD_VT_H_
#define _LOAD_VT_H_

#include "version_table.h"

#define PARTITION_NOT_EXIST		(-1)

//FIXME: u-boot use this format to set UTF16 charactors currently
#ifndef CONFIG_AB_BOOTFLOW

#define FTS_NAME		"fts"
#define KERNEL_NAME		"boot"
#define RECOVERY_NAME		"recovery"
#define LOGO_NAME		"fastlogo"
#define TZK_NORMAL_NAME		"tzk_normal"
#define TZK_NORMALB_NAME	"tzk_normalB"
#define TZK_RECOVERY_NAME	"tzk_recovery"
#define POST_BL_NAME		"post_bl"
#define BL_NORMAL_NAME		"bl_normal"
#define BL_NORMALB_NAME		"bl_normalB"
#define BL_RECOVERY_NAME	"bl_recovery"
#define MISC_NAME		"misc"
#define FASTBOOT_NAME		"fastboot"
#define FASTBOOTA_NAME		"fastboot_1st"
#define FASTBOOTB_NAME		"fastboot_2nd"
#define FRP_NAME		"frp"
#define KEY1_NAME		"key_1st"
#define KEY2_NAME		"key_2nd"
#define DTBO_NAME		"dtbo"

#else

//add below definitions just for common drivers
#define BL_NORMAL_NAME          "bl_normal"
#define KERNEL_NAME             "boot"
#define RECOVERY_NAME           "recovery"

#define FTS_NAME                "fts"
#define MISC_NAME               "misc"
#define FRP_NAME                "frp"
#define KERNEL_A_NAME           "boot_a"
#define KERNEL_B_NAME           "boot_b"
#define LOGO_A_NAME             "fastlogo_a"
#define LOGO_B_NAME             "fastlogo_b"
#define TZK_A_NAME              "tzk_a"
#define TZK_B_NAME              "tzk_b"
#define BL_A_NAME               "bl_a"
#define BL_B_NAME               "bl_b"
#define FASTBOOT_A_NAME         "fastboot_a"
#define FASTBOOT_B_NAME         "fastboot_b"
#define KEY_A_NAME              "key_a"
#define KEY_B_NAME              "key_b"
#define DTBO_A_NAME             "dtbo_a"
#define DTBO_B_NAME             "dtbo_b"

#endif

void dump_version_table(void);
int parse_version_table(unsigned char* buff);
int get_partition_info(void * tbuff);
int find_partition(const void *name);
int fetch_partition_info(int num, ver_table_entry_t *vt);
unsigned int get_version_table_entry_number();
long long read_image(ver_table_entry_t *vt,unsigned int image_size,unsigned char *image_buff);
long long read_image_from_offset(ver_table_entry_t *vt, unsigned int pt_offset, unsigned int size, unsigned char *image_buf);
void set_flash_ts_param(char *param_buf);
long long read_image_by_ptname(const char *module_name,unsigned int image_size,unsigned char *image_buff);

#endif
