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
//! \file	       boot_mode.c
//! \brief 	checking current boot mode for miniloader to select fastboot/normal/recovery.
///////////////////////////////////////////////////////////////////////////////
#include "com_type.h"
#include "io.h"
#include "string.h"

#include "debug.h"
#include "flash_adaptor.h"

#ifdef EMMC_BOOT
#include "load_gpt.h"
#endif

#include "flash_ts.h"
#include "boot_mode.h"
#include "boot_devinfo.h"

#include "system_manager.h"
#include "mem_map_itcm.h"

#define BOOTLOADER_COMMAND_KEY         "bootloader.command"
#define BOOTLOADER_OPT_QUIESCENT_KEY   "bootloader.opt.quiescent"
#define FTS_KEY_DEBUG_LEVEL            "debug.level"
#define FTS_KEY_MACADDR                "macaddr"
#define FTS_KEY_SERIALNO               "serialno"
#define MAX_COMMAND                    100

typedef enum {
	REBOOT_MODE_NORMAL             = 0x0,
	REBOOT_MODE_BOOTLOADER         = 0x29012002,
	REBOOT_MODE_RECOVERY           = 0x11092017,
	REBOOT_MODE_FASTBOOT           = 0x12510399,
	REBOOT_MODE_DEVICE_CORRUPTED   = 0x12513991,
	REBOOT_MODE_RECOVERY_QUIESCENT = 0x06012021,
	REBOOT_MODE_QUIESCENT          = 0x01112021,
} REBOOT_MODE_MAGIC;

static const char kFtsBootcmdRecovery[] = "boot-recovery";
static const char kFtsBootcmdFastboot[] = "bootonce-bootloader";
static const char kBootcmdFastbootd[]   = "boot-fastboot";

static int Bootmode = BOOTMODE_NORMAL;
static int BootOptions = 0;
static bool fts_exist = FALSE;       //fts partition exist
static bool devinfo_exist = FALSE;   //devinfo partition exist

int init_fts(void)
{
#ifdef EMMC_BOOT
	struct gpt_ent gpt_fts;
	int num = find_partition(FTS_NAME);
	if(num >= 0) {
		fetch_partition_info(num, &gpt_fts);
		INFO("FTS GPT INFO: ent_lba_start=0x%x, ent_lba_end=0x%x \n", gpt_fts.ent_lba_start, gpt_fts.ent_lba_end);
		return flash_ts_init((gpt_fts.ent_lba_start)/1024, (gpt_fts.ent_lba_end - gpt_fts.ent_lba_start + 1)/1024);
	}
#endif
	ERR("FTS Partition NOT Found !!\n");
	return -1;
}

static int check_misc_command()
{
	unsigned char * buff = (unsigned char *)(TEMP_BUF_ADDR);
	struct bootloader_message *msg = (struct bootloader_message *)buff;
	unsigned int read_size = 0;
	long long ret;

	//get misc partition informaiton from EMMC by GPT
	struct gpt_ent gpt_misc;
	int num = find_partition(MISC_NAME);
	if(num >= 0) {
		fetch_partition_info(num, &gpt_misc);
	} else {
		// if no misc,  return normal
		ERR("find misc partition error!\n");
		return BOOTMODE_NORMAL;
	}

	//read 8k bytes header to get the size of image stored in the header
	read_size = (get_page_size() > 2048) ? get_page_size() : 2048;
	ret = read_image(&gpt_misc, read_size, buff);
	if(ret){
		ERR("read misc area data failed.\n");
		return BOOTMODE_NORMAL;
	}

	if(strncmp(msg->command, kFtsBootcmdRecovery, sizeof(kFtsBootcmdRecovery)) == 0) {
		return BOOTMODE_RECOVERY;
	}

	if(strncmp(msg->command, kFtsBootcmdFastboot, sizeof(kFtsBootcmdFastboot)) == 0) {
		return BOOTMODE_FASTBOOT;
	}

	if(strncmp(msg->command, kBootcmdFastbootd, sizeof(kBootcmdFastbootd)) == 0) {
		return BOOTMODE_RECOVERY;
	}

	return BOOTMODE_NORMAL;
}

static void set_misc_command(const char *command)
{
	unsigned char * buff = (unsigned char *)(TEMP_BUF_ADDR);
	struct bootloader_message *msg = (struct bootloader_message *)buff;
	unsigned int read_size = 0;
	long long ret;

	struct gpt_ent gpt_misc;
	int num = find_partition(MISC_NAME);
	if(num >= 0) {
		fetch_partition_info(num, &gpt_misc);
	} else {
		ERR("find misc partition error!\n");
		return;
	}

	//read 8k bytes header to get the size of image stored in the header
	read_size = (get_page_size() > 2048) ? get_page_size() : 2048;
	ret = read_image(&gpt_misc, read_size, buff);
	if(ret){
		ERR("read misc area data failed.\n");
		return;
	}

	if(!strcmp(command, kFtsBootcmdRecovery) || !strcmp(command, kFtsBootcmdFastboot))
	{
		memset(msg->command, 0x0, sizeof(msg->command));
		strcpy(msg->command, command);
		write_image(&gpt_misc, read_size, buff);
		if(0 != ret){
			ERR("set misc bootloader message failed.\n");
		}
	}
}

static void write_android_bootloader_message(const char *command,
					const char *status,
					const char *recovery)
{
	if (fts_exist)
	{
		if(flash_ts_set(BOOTLOADER_COMMAND_KEY, command))
			ERR("ERROR: Failed to set bootloader command\n");
		set_misc_command(command);
	}
	else if (devinfo_exist) {
		set_command_to_boot_info(command);
		set_misc_command(command);
	}
	else
		ERR("No FTS partition and No DEVINFO partition!\n");

	return;
}

int check_bootoptions(void)
{
	char boot_option[256] = {0};

	if (fts_exist)
	{
		flash_ts_get(BOOTLOADER_OPT_QUIESCENT_KEY, boot_option, (sizeof(boot_option) - 1));
		boot_option[sizeof(boot_option) - 1] = '\0';
	}
	else if (devinfo_exist)
	{
		strcpy(boot_option, get_bootoption_from_boot_info());
	}

#if defined(SOC_RAM_TS_ENABLE) || defined(SOC_RAM_PARAM_ENABLE)
	/* Android S launched project remove FTS partition, instead, Linux Kernel writes
	   boot mode to SM SRAM. */
	REBOOT_MODE_MAGIC reboot_mode_magic_value = readl(SOC_REBOOTMODE_ADDR);
	if (reboot_mode_magic_value == REBOOT_MODE_RECOVERY_QUIESCENT ||
		reboot_mode_magic_value == REBOOT_MODE_QUIESCENT) {
		strcpy(boot_option, "true");
	}
#endif

	BootOptions = 0;
	if (strcmp(boot_option, "true") == 0) {
		INFO("BOOTLOADER: bootup with quiescent mode!\n");
		BootOptions |= BOOTOPTION_QUIESCENT;
	}

	return BootOptions;
}

int set_bootoptions(int opt)
{
	if (opt == BootOptions)
		return BootOptions;

	BootOptions = 0;

	if (fts_exist)
	{
		if (opt & BOOTOPTION_QUIESCENT) {
			flash_ts_set(BOOTLOADER_OPT_QUIESCENT_KEY, "true");
			BootOptions |= BOOTOPTION_QUIESCENT;
		} else {
			flash_ts_set(BOOTLOADER_OPT_QUIESCENT_KEY, "");
		}
	}
	else if (devinfo_exist)
	{
		if (opt & BOOTOPTION_QUIESCENT) {
			set_bootoption_to_boot_info("true");
			BootOptions |= BOOTOPTION_QUIESCENT;
		} else {
			set_bootoption_to_boot_info("");
		}
	}

#if defined(SOC_RAM_TS_ENABLE) || defined (SOC_RAM_PARAM_ENABLE)
	if (opt & BOOTOPTION_QUIESCENT) {
		BootOptions |= BOOTOPTION_QUIESCENT;
	} else {
		writel(REBOOT_MODE_NORMAL, SOC_REBOOTMODE_ADDR);
	}
#endif

	return BootOptions;
}

int get_bootoptions(void)
{
	return BootOptions;
}

static int check_bootloader_recovery_mode(void)
{
	char boot_command[256] = {0};

	if (fts_exist)
	{
		flash_ts_get(BOOTLOADER_COMMAND_KEY, boot_command, (sizeof(boot_command) - 1));
		boot_command[sizeof(boot_command) -1] = '\0';
	}
	else if (devinfo_exist) {
		strcpy(boot_command, get_command_from_boot_info());
	}

	if (strncmp(boot_command, kFtsBootcmdRecovery, sizeof(kFtsBootcmdRecovery)) == 0) {
		NOTICE("BOOTLOADER: Recovery Mode got !\n");
		return BOOTMODE_RECOVERY;
	}

	if(check_misc_command() == BOOTMODE_RECOVERY) {
		NOTICE("MISC: Recovery Mode got !\n");
		return BOOTMODE_RECOVERY;
	}

	return BOOTMODE_NORMAL;
}

static int check_android_bootcommand(void)
{
	char boot_command[256] = {0};

	if (fts_exist)
	{
		flash_ts_get(BOOTLOADER_COMMAND_KEY, boot_command, (sizeof(boot_command) - 1));
		boot_command[sizeof(boot_command) -1] = '\0';
	}
	else if (devinfo_exist) {
		strcpy(boot_command, get_command_from_boot_info());
	}

	if (strncmp(boot_command, kFtsBootcmdFastboot, sizeof(kFtsBootcmdFastboot)) == 0) {
		NOTICE("FTS/DEVINFO: Fastboot Mode got !\n");
		return BOOTMODE_FASTBOOT;
	}

	if (strncmp(boot_command, kFtsBootcmdRecovery, sizeof(kFtsBootcmdRecovery)) == 0) {
		NOTICE("FTS/DEVINFO: Recovery Mode got !\n");
		return BOOTMODE_RECOVERY;
	}

	if(check_misc_command() == BOOTMODE_RECOVERY) {
		NOTICE("MISC: Recovery Mode got !\n");
		return BOOTMODE_RECOVERY;
	}

	if(check_misc_command() == BOOTMODE_FASTBOOT) {
		NOTICE("MISC: Fastboot Mode got !\n");
		return BOOTMODE_FASTBOOT;
	}

#ifdef SOC_RAM_PARAM_ENABLE
	REBOOT_MODE_MAGIC reboot_mode_magic_value = readl(SOC_REBOOTMODE_ADDR);
	if (reboot_mode_magic_value == REBOOT_MODE_RECOVERY) {
		NOTICE("SM SRAM: Recovery Mode got !\n");
		return BOOTMODE_RECOVERY;
	}
	else if (reboot_mode_magic_value == REBOOT_MODE_BOOTLOADER) {
		NOTICE("SM SRAM: Fastboot Mode got !\n");
		return BOOTMODE_FASTBOOT;
	}
#endif
	return BOOTMODE_NORMAL;
}


int init_bootmode(void)
{
	int num = 0;

	num = find_partition(FTS_NAME);
	if(num > 0)
	{
		struct gpt_ent gpt_fts;
		fts_exist = TRUE;

		fetch_partition_info(num, &gpt_fts);

		if(gpt_fts.ent_lba_end  < gpt_fts.ent_lba_start) {
			ERR("entry_lba_start is larger than entry_lba_end! \n");
			return FLASH_OP_ERR;
		}

		flash_ts_init((gpt_fts.ent_lba_start)/1024, (gpt_fts.ent_lba_end - gpt_fts.ent_lba_start + 1)/1024);
	}

	num = find_partition(DEVINFO_NAME);
	if(num > 0)
	{
		devinfo_exist = TRUE;
		read_boot_info_from_flash();
	}

	return 0;
}

int check_bootmode(void)
{
	Bootmode = check_android_bootcommand();
	return Bootmode;
}

int get_bootmode(void)
{
	return Bootmode;
}

int check_bootloader_bootmode(void)
{
	Bootmode = check_bootloader_recovery_mode();
	return Bootmode;
}

int set_bootmode(int boot_mode)
{
	if(BOOTMODE_RECOVERY == boot_mode) {
		write_android_bootloader_message(kFtsBootcmdRecovery, "", "recovery\n--show_text\n");
		Bootmode = BOOTMODE_RECOVERY;
	}
	else if(BOOTMODE_FASTBOOT == boot_mode) {
		write_android_bootloader_message(kFtsBootcmdFastboot, "", "fastboot\n--show_text\n");
		Bootmode = BOOTMODE_FASTBOOT;
	}
	else if(BOOTMODE_NORMAL == boot_mode) {
		INFO("Clear FTS for next normal boot !\n");
		write_android_bootloader_message("done", "", "");
		Bootmode = BOOTMODE_NORMAL;
	}
	else {
		Bootmode = boot_mode;
	}

	return Bootmode;
}


int get_debuglevel_from_flash(void)
{
	int dbg_level = 0;

	if(fts_exist) {
		dbg_level = flash_ts_get_int(FTS_KEY_DEBUG_LEVEL, 0);
	}
	else if(devinfo_exist){
		dbg_level = get_debuglevel_from_boot_info();
	}

	return dbg_level;
}

void get_serialno_from_flash(char * serilno_buf)
{
	if(serilno_buf) {
		if(fts_exist) {
			flash_ts_get(FTS_KEY_SERIALNO, serilno_buf, 64);
		}
		else if(devinfo_exist){
			strcpy(serilno_buf, get_serialno_from_boot_info());
		}
	}
}

void get_macaddr_from_flash(char * mac_addr)
{
	if(mac_addr) {
		if(fts_exist) {
			flash_ts_get(FTS_KEY_MACADDR, mac_addr, 64);
		}
		else if(devinfo_exist){
			strcpy(mac_addr, get_macaddr_from_boot_info());
		}
	}
}

#ifdef CONFIG_FASTLOGO
int check_fastlogo_start()
{
	if ((get_bootmode() != BOOTMODE_RECOVERY &&
			get_bootmode() != BOOTMODE_NORMAL) ||
			(BootOptions & BOOTOPTION_QUIESCENT))
		return 0;
	else
		return 1;
}
#endif


/***********************************************
 * A/B Mode check *
 *
 ***********************************************/

#define ANDROID_AB_COMMAND_KEY		"bootctrl.metadata"
#define SYNA_SPACE_OFFSET_IN_MISC   4096



static int BootAB_sel = BOOTSEL_A;
static boot_ctrl_t bctrl = {0};
static misc_boot_ctrl_t misc_bctrl = {0};

static bool is_valid_bootctrl(void)
{
	if (fts_exist) {
		return ((BOOTCTRL_MAGIC == bctrl.magic) &&
			(BOOT_CONTROL_VERSION == bctrl.version));
	}
	else if (devinfo_exist) {
		return ((BOOTCTRL_MAGIC == misc_bctrl.magic) &&
			(MISC_BOOT_CONTROL_VERSION == misc_bctrl.version));
	}
	return 0;
}

static bool slot_is_bootable(int slot_num)
{
	if (fts_exist) {
		slot_metadata_t *pABSlot = &bctrl.slot_info[slot_num];
		if(slot_num ? bctrl.slot_b_verity_corrupted : bctrl.slot_a_verity_corrupted) {
			NOTICE("slot_%s is dm-verity corrupted\n", slot_name(slot_num));
			return 0;
		}
		else
			return (pABSlot->priority > 1 &&
				(pABSlot->successful_boot || (pABSlot->tries_remaining > 0)));
	}
	else if (devinfo_exist) {
		misc_slot_metadata_t *pABSlot = &misc_bctrl.slot_info[slot_num];
		return (pABSlot->priority > 1 && pABSlot->verity_corrupted == 0 &&
			(pABSlot->successful_boot || (pABSlot->tries_remaining > 0)));
	}
	return 0;
}

static int bootab_slot_select(void)
{
	unsigned int bootab_sel = BOOTSEL_DEFAULT;

	if (fts_exist) {
		if(bctrl.slot_info[0].priority >= bctrl.slot_info[1].priority) {
			bootab_sel = BOOTSEL_A;
		}
		else{
			bootab_sel = BOOTSEL_B;
		}
	}
	else if (devinfo_exist) {
		if(misc_bctrl.slot_info[0].priority >= misc_bctrl.slot_info[1].priority) {
			bootab_sel = BOOTSEL_A;
		}
		else{
			bootab_sel = BOOTSEL_B;
		}
	}

	return bootab_sel;
}

static int write_bootctrl_metadata(void * pbootctrl)
{
	char metadata_str[32] = {0};

	if(NULL == pbootctrl) {
		ERR("ERROR: unvalid bootctrl metadata for write !\n");
		return -1;
	}

	if (fts_exist) {
		snprintf(metadata_str, 17, "%016llx", *((uint64_t *)pbootctrl));
		if(flash_ts_set(ANDROID_AB_COMMAND_KEY, metadata_str)) {
			ERR("ERROR: Failed to write bootctrl metadata !\n");
			return -1;
		}
	}
	else if (devinfo_exist) {
		struct gpt_ent gpt_misc;
		long long ret;
		long long start = 0;
		int num = find_partition(MISC_NAME);
		unsigned char * buff = (unsigned char *)(TEMP_BUF_ADDR);

		if(num >= 0) {
			fetch_partition_info(num, &gpt_misc);
		} else {
			ERR("find misc partition error!\n");
		}

		/* bootctrl_metadata in misc partition position */
		start = gpt_misc.ent_lba_start * 512 + SYNA_SPACE_OFFSET_IN_MISC;

		memcpy(buff, (unsigned char *)pbootctrl, sizeof(misc_boot_ctrl_t));

		ret =write_flash(start, sizeof(misc_boot_ctrl_t), buff);
		if(0 != ret){
			ERR("set misc bootloader message failed.\n");
			return -1;
		}
	}

	return 0;
}

static void init_bootctrl(int default_slot)
{
	if(default_slot > 1) {
		ERR("invalid default slot number for bootctrl !\n");
		return;
	}

	if (fts_exist) {
		boot_ctrl_t * pbootctrl = &bctrl;

		pbootctrl->slot_info[default_slot].priority        = 8;
		pbootctrl->slot_info[default_slot].tries_remaining = 2;
		pbootctrl->slot_info[default_slot].successful_boot = 0;

		pbootctrl->slot_info[1 - default_slot].priority        = 3;
		pbootctrl->slot_info[1 - default_slot].tries_remaining = 2;
		pbootctrl->slot_info[1 - default_slot].successful_boot = 0;

		pbootctrl->version = BOOT_CONTROL_VERSION;
		pbootctrl->magic   = BOOTCTRL_MAGIC;

		write_bootctrl_metadata((void*)pbootctrl);
	}
	else if (devinfo_exist) {
		misc_boot_ctrl_t * pbootctrl = &misc_bctrl;

		pbootctrl->slot_info[default_slot].priority        = 8;
		pbootctrl->slot_info[default_slot].tries_remaining = 2;
		pbootctrl->slot_info[default_slot].successful_boot = 0;

		pbootctrl->slot_info[1 - default_slot].priority        = 3;
		pbootctrl->slot_info[1 - default_slot].tries_remaining = 2;
		pbootctrl->slot_info[1 - default_slot].successful_boot = 0;

		pbootctrl->version = MISC_BOOT_CONTROL_VERSION;
		pbootctrl->magic   = BOOTCTRL_MAGIC;

		write_bootctrl_metadata((void*)pbootctrl);
	}
}

static int check_dmverity_device_corrupted(int bootab_sel)
{
	unsigned int bootab_sel_final = bootab_sel;

#if defined(CONFIG_SM)
	if (readl(SOC_REBOOTMODE_ADDR) == REBOOT_MODE_DEVICE_CORRUPTED){
		bootab_sel_final = (bootab_sel == BOOTSEL_A) ? BOOTSEL_B : BOOTSEL_A;

		if (slot_is_bootable(bootab_sel_final)){
			if(fts_exist){
				if(bootab_sel)
					bctrl.slot_b_verity_corrupted = 1;
				else
					bctrl.slot_a_verity_corrupted = 1;
				write_bootctrl_metadata((void*)&bctrl);
			}
			else if(devinfo_exist){
				misc_bctrl.slot_info[bootab_sel].verity_corrupted = 1;
				write_bootctrl_metadata((void*)&misc_bctrl);
			}
			writel(REBOOT_MODE_NORMAL, SOC_REBOOTMODE_ADDR);
		}
		else{
			ERR("Slot_%s 'dm-verity device corrupted', and slot_%s is not bootable\n",
				slot_name(bootab_sel), slot_name(bootab_sel_final));
			bootab_sel_final = BOOTSEL_INVALID;
		}
	}
#else
	ERR("Skip %s!\n", __FUNCTION__);
#endif
	return bootab_sel_final;
}

static int check_android_abcommand(void)
{
	unsigned int bootab_sel = BOOTSEL_DEFAULT;

	if(is_valid_bootctrl()) {
		if(slot_is_bootable(0) && slot_is_bootable(1)) {
			bootab_sel = bootab_slot_select();
		}
		else if (slot_is_bootable(0)) {
			bootab_sel = BOOTSEL_A;
		}
		else if (slot_is_bootable(1)) {
			bootab_sel = BOOTSEL_B;
		}
		else {
			/* No bootable slots! */
			ERR("No bootable slots found !!!\n");
			bootab_sel = BOOTSEL_INVALID;
			goto out;
		}
	}
	else {
		ERR("No valid metadata for bootctrl, initialize to default slot %d !\n", BOOTSEL_DEFAULT);
		init_bootctrl(BOOTSEL_DEFAULT);
		bootab_sel = BOOTSEL_DEFAULT;
	}

	bootab_sel = check_dmverity_device_corrupted(bootab_sel);

out:
	return bootab_sel;
}

void force_init_abmode(void)
{
	init_bootctrl(BOOTSEL_DEFAULT);
	init_abmode();
}

void init_abmode(void)
{
	if(fts_exist) {
		char ab_command[32] = {0};
		uint64_t bootctrl_data = 0x0;
		boot_ctrl_t * pbootctrl = (boot_ctrl_t *)(uintptr_t)(&bootctrl_data);

		memset((void *)&bctrl, 0, sizeof(boot_ctrl_t));

		flash_ts_get(ANDROID_AB_COMMAND_KEY, ab_command, (sizeof(ab_command) - 1));
		ab_command[sizeof(ab_command) - 1] = '\0';
		INFO("First fts: %s: %s\n", ANDROID_AB_COMMAND_KEY, ab_command);

		if(strnlen(ab_command, MAX_COMMAND)) {
			bootctrl_data = strtoul((ab_command), NULL, 16);
			//Always display below message for bootctrl.metadata recognization
			NOTICE("-----bootctrl_data:  0x%x-%x-----\n", bootctrl_data >> 32, bootctrl_data);
			memcpy((void *)&bctrl, (void *)pbootctrl, sizeof(boot_ctrl_t));
		}
	}
	else if(devinfo_exist) {
		long long ret;
		long long start = 0;
		int read_size;
		unsigned char * buff = (unsigned char *)(TEMP_BUF_ADDR);
		struct gpt_ent gpt_misc;

		int num = find_partition(MISC_NAME);
		if(num >= 0) {
			fetch_partition_info(num, &gpt_misc);
		} else {
			ERR("find fts partition error!\n");
		}
		read_size = (get_page_size() > 2048) ? get_page_size() : 2048;
		start = gpt_misc.ent_lba_start * 512 + SYNA_SPACE_OFFSET_IN_MISC;
		ret =read_flash(start, read_size, buff);
		if(0 != ret){
			ERR("read misc boot control data failed.\n");
		}
		memcpy((unsigned char *)&misc_bctrl, buff, sizeof(misc_boot_ctrl_t));
	}
}

int check_abmode(void)
{
	BootAB_sel = check_android_abcommand();
	return BootAB_sel;
}

int get_abmode(void)
{
	return BootAB_sel;
}

int try_abmode(int abmode_sel)
{
	/* ... and decrement tries remaining, if applicable. */
	if(fts_exist) {
		if(is_valid_bootctrl() && (bctrl.slot_info[abmode_sel].tries_remaining > 0)) {
			bctrl.slot_info[abmode_sel].tries_remaining -= 1;
			write_bootctrl_metadata((void*)&bctrl);
			return 0;
		}
		else if(is_valid_bootctrl()) {
			bctrl.slot_info[abmode_sel].priority-=2;
			write_bootctrl_metadata((void*)&bctrl);
			return -1;
		}
		else {
			// invalid bootctrl metadata
			return 1;
		}
	}
	else if(devinfo_exist) {
		if(is_valid_bootctrl() && (misc_bctrl.slot_info[abmode_sel].tries_remaining > 0)) {
			misc_bctrl.slot_info[abmode_sel].tries_remaining -= 1;
			write_bootctrl_metadata((void*)&misc_bctrl);
			return 0;
		}
		else if(is_valid_bootctrl()) {
			misc_bctrl.slot_info[abmode_sel].priority-=2;
			write_bootctrl_metadata((void*)&misc_bctrl);
			return -1;
		}
		else {
			// invalid bootctrl metadata
			return 1;
		}
	}
	return -1;
}

bool check_boot_success(void) {
	bool ret = false;
	if (BootAB_sel != BOOTSEL_A && BootAB_sel != BOOTSEL_B) {
		return false;
	}
	if (fts_exist) {
		ret = (bctrl.slot_info[BootAB_sel].successful_boot != 0);
	} else if (devinfo_exist) {
		ret = (misc_bctrl.slot_info[BootAB_sel].successful_boot != 0);
	} else {
		//should never reach here
	}
	return ret;
}

// vim: noexpandtab
