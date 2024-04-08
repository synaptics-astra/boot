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
///////////////////////////////////////////////////////////////////////////////
//! \file   boot_devinfo.h
//! \brief  checking current boot mode for miniloader to select fastboot/normal/recovery.
///////////////////////////////////////////////////////////////////////////////

#ifndef _boot_message_H
#define _boot_message_H

#define BOOTCTRL_MAGIC             0x42424100
#define BOOTCTRL_SUFFIX_A          "_a"
#define BOOTCTRL_SUFFIX_B          "_b"
#define BOOT_CONTROL_VERSION       1
#define MISC_BOOT_CONTROL_VERSION  2

typedef struct slot_metadata {
	uint8_t priority : 4;
	uint8_t tries_remaining : 3;
	uint8_t successful_boot : 1;
}slot_metadata_t;

typedef struct boot_ctrl {
	uint32_t magic;		// Magic for identification - '\0ABB' (Boot Contrl Magic)
	uint8_t version; 		// boot ctrl version, 1
	slot_metadata_t slot_info[2]; 	// Information about each slot
	uint8_t slot_a_verity_corrupted:4;
	uint8_t slot_b_verity_corrupted:4;
}boot_ctrl_t;


typedef struct misc_slot_metadata {
    uint8_t priority;
    uint8_t tries_remaining;
    // 1 if this slot has booted successfully, 0 otherwise.
    uint8_t successful_boot;
    // 1 if this slot is corrupted from a dm-verity corruption, 0 otherwise.
    uint8_t verity_corrupted;
    uint8_t reserved;
} misc_slot_metadata_t;

typedef struct misc_boot_ctrl {
    uint32_t magic;
    uint8_t version;
    uint8_t recovery_tries_remaining;
    misc_slot_metadata_t slot_info[2];
    uint8_t reserved[16];
} misc_boot_ctrl_t;

/* Bootloader Message
 *
 * This structure describes the content of a block in flash
 * that is used for recovery and the bootloader to talk to
 * each other.
 *
 * The command field is updated by linux when it wants to
 * reboot into recovery or to update radio or bootloader firmware.
 * It is also updated by the bootloader when firmware update
 * is complete (to boot into recovery for any final cleanup)
 *
 * The status field is written by the bootloader after the
 * completion of an "update-radio" or "update-hboot" command.
 *
 * The recovery field is only written by linux and used
 * for the system to send a message to recovery or the
 * other way around.
 *
 * The stage field is written by packages which restart themselves
 * multiple times, so that the UI can reflect which invocation of the
 * package it is.  If the value is of the format "#/#" (eg, "1/3"),
 * the UI will add a simple indicator of that status.
 */
typedef struct bootloader_message {
	char command[32];
	char status[32];
	char recovery[768];

	// The 'recovery' field used to be 1024 bytes.  It has only ever
	// been used to store the recovery command line, so 768 bytes
	// should be plenty.  We carve off the last 256 bytes to store the
	// stage string (for multistage packages) and possible future
	// expansion.
	char stage[32];
	char reserved[224];
} bootloader_message_t;

/* Boot Info
 *
 * This structure describes the content of a block in flash
 * that is used for device info and recovery and the bootloader to talk to
 * each other.
 *
 * The mac_addr field is written by application according to factory_setting/mac_addr
 * and pass it to kernel to config ethernet mac
 *
 * The serialno field is written by application according to factory_setting/serialno
 * and pass it to application
 *
 * The command field is updated by linux when it wants to
 * reboot into recovery or to update radio or bootloader firmware.
 * It is also updated by the bootloader when firmware update
 * is complete (to boot into recovery for any final cleanup)
 *
 * The boot_option field is only written by linux and used
 * for background update system
 *
 * The debeg_level field is only written by uboot or fastboot and used
 * for changing BSP module debug level
 *
 */
typedef struct boot_info {
	char mac_addr[32];
	char serialno[64];
	char command[64];
	char boot_option[32];
	int  debeg_level;
	char reserved1[32];
	char reserved2[32];
	char reserved3[32];
} boot_info_t;

extern int __img_buff_start;
#define TEMP_BUF_ADDR ((uintptr_t)(&__img_buff_start))

char * get_command_from_boot_info(void);
char * get_serialno_from_boot_info(void);
char * get_macaddr_from_boot_info(void);
char * get_bootoption_from_boot_info(void);
int get_debuglevel_from_boot_info(void);
void set_command_to_boot_info(const char * command);
void set_macaddr_to_boot_info(const char * macaddr);
void set_serialno_to_boot_info(const char * serialno);
void set_bootoption_to_boot_info(const char * bootoption);
void read_boot_info_from_flash(void);
void write_boot_info_to_flash(boot_info_t * pBoot_info);

#endif
