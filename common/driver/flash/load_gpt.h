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
#ifndef _LOAD_GPT_H_
#define _LOAD_GPT_H_

#include "gpt.h"

#define PARTITION_NOT_EXIST		(-1)

//FIXME: u-boot use this format to set UTF16 charactors currently
#ifndef CONFIG_AB_BOOTFLOW

#define FTS_NAME                "fts"
#define DEVINFO_NAME            "devinfo"
#define KERNEL_NAME             "boot"
#define RECOVERY_NAME           "recovery"
#define LOGO_NAME               "fastlogo"
#define TZK_NORMAL_NAME         "tzk_normal"
#define TZK_NORMALB_NAME        "tzk_normalB"
#define TZK_RECOVERY_NAME       "tzk_recovery"
#define POST_BL_NAME            "post_bl"
#define BL_NORMAL_NAME          "bl_normal"
#define BL_NORMALB_NAME         "bl_normalB"
#define BL_RECOVERY_NAME        "bl_recovery"
#define MISC_NAME               "misc"
#define FASTBOOT_NAME           "fastboot"
#define FASTBOOTA_NAME          "fastboot_1st"
#define FASTBOOTB_NAME          "fastboot_2nd"
#define FRP_NAME                "frp"
#define KEY1_NAME               "key_1st"
#define KEY2_NAME               "key_2nd"
#define DTBO_NAME               "dtbo"
#define FIRMWARE_NAME           "firmware"
#define VENDOR_BOOT_NAME        "vendor_boot"
#define INIT_BOOT_NAME          "init_boot"

#else

//add below definitions just for common drivers
#define BL_NORMAL_NAME          "bl_normal"
#define KERNEL_NAME             "boot"
#define RECOVERY_NAME           "recovery"
#define FTS_NAME                "fts"
#define DEVINFO_NAME            "devinfo"
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
#define FIRMWARE_A_NAME         "firmware_a"
#define FIRMWARE_B_NAME         "firmware_b"
#define VENDOR_BOOT_A_NAME      "vendor_boot_a"
#define VENDOR_BOOT_B_NAME      "vendor_boot_b"
#define INIT_BOOT_A_NAME        "init_boot_a"
#define INIT_BOOT_B_NAME        "init_boot_b"
#endif

#define SDIO_BLK_SIZE	0x200
#define UUID_STR_LEN	37

void dump_gpt_table(void);
int parse_gpt_table(struct gpt_hdr * hdr, struct gpt_ent * entry);
void uuid_to_str(char *s, size_t len, const struct uuid *u);
int get_gpt(void * tbuff);
int find_partition(const char * name);
int fetch_partition_info(int num, struct gpt_ent *gpte);
unsigned int get_gpt_entry_number();
long long read_image(struct gpt_ent *gpte, unsigned int image_size, unsigned char *image_buff);
long long read_image_from_offset(struct gpt_ent *gpte, unsigned int pt_offset, unsigned int size, unsigned char *image_buf);
long long read_image_by_ptname(const char *module_name,unsigned int image_size,unsigned char *image_buff);
long long write_image(struct gpt_ent *gpte, unsigned int image_size, unsigned char *image_buff);
long long write_image_to_offset(struct gpt_ent *gpte, unsigned int pt_offset, unsigned int size, unsigned char *image_buf);

int get_partition_info(void * tbuff);

#endif
