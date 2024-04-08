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
//! \file	       boot_mode.h
//! \brief 	checking current boot mode for miniloader to select fastboot/normal/recovery.
///////////////////////////////////////////////////////////////////////////////

#ifndef _BOOT_MODE_H
#define _BOOT_MODE_H

#define BOOTMODE_NORMAL			   0
#define BOOTMODE_RECOVERY		   1
#define BOOTMODE_RESUME				2
#define BOOTMODE_FASTBOOT		   5

#define BOOTOPTION_QUIESCENT       0x01

typedef enum {
	NORMAL_BOOT,
	WARM_BOOT,
	RECOVER = 0x80,
	GENX_TZK = 0x100
}BOOT_TYPE;

#ifdef CONFIG_FAST_BOOT
int check_fastboot_mode(void);
#endif


int init_fts(void);
int init_rts(void);
int init_bootmode(void);

//for miniloder
int check_bootmode(void);
//for bootloader
int check_bootloader_bootmode(void);

int get_bootmode(void);
int set_bootmode(int boot_mode);

int check_bootoptions(void);

int get_bootoptions(void);
int set_bootoptions(int opt);
int get_debuglevel_from_flash(void);
void get_serialno_from_flash(char * serilno_buf);
void get_macaddr_from_flash(char * mac_addr);


//for bootloader check reboot details
int set_recovery_reason(int boot_mode, const char *recovery);

#ifdef CONFIG_FASTLOGO
int check_fastlogo_start();
#endif


/***********************************************
 * A/B Mode check *
 *
 ***********************************************/
#define BOOTSEL_A				0x0		//0x11
#define BOOTSEL_B				0x1		//0x22
#define BOOTSEL_DEFAULT			BOOTSEL_A
#define BOOTSEL_INVALID			0xff
#define slot_name(abmode)		((abmode) ? "b" : "a")

void force_init_abmode(void);
void init_abmode(void);
int check_abmode(void);
int get_abmode(void);
int set_abmode(int abmode_sel);
int try_abmode(int abmode_sel);
bool check_boot_success(void);

#endif
