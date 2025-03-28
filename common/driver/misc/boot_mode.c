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

#ifdef CONFIG_GPT
#include "load_gpt.h"
#endif

#ifdef CONFIG_VT
#include "load_vt.h"
#endif

#include "flash_ts.h"
#include "boot_mode.h"
#include "boot_devinfo.h"
#include "bm_generic.h"

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
static bool devinfo_exist = FALSE;   //devinfo partition exist
static bool misc_exist = FALSE;   //misc partition exist

static int check_misc_command()
{
	unsigned char * buff = (unsigned char *)(TEMP_BUF_ADDR);
	struct bootloader_message *msg = (struct bootloader_message *)buff;
	unsigned int read_size = 0;
	long long ret;

#ifdef CONFIG_VT
	ver_table_entry_t pt_misc;
#elif CONFIG_GPT
	struct gpt_ent pt_misc;
#endif
	int num = find_partition(MISC_NAME);
	if(num >= 0) {
		fetch_partition_info(num, &pt_misc);
	} else {
		// if no misc,  return normal
		ERR("find misc partition error!\n");
		return BOOTMODE_NORMAL;
	}

	//read one page to get the size of image stored in the header
	read_size = (get_page_size() > 2048) ? get_page_size() : 2048;
	ret = read_image(&pt_misc, read_size, buff);
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

#ifdef CONFIG_VT
	ver_table_entry_t pt_misc;
#elif CONFIG_GPT
	struct gpt_ent pt_misc;
#endif

	int num = find_partition(MISC_NAME);
	if(num >= 0) {
		fetch_partition_info(num, &pt_misc);
	} else {
		ERR("find misc partition error!\n");
		return;
	}

	//read 8k bytes header to get the size of image stored in the header
	read_size = (get_page_size() > 2048) ? get_page_size() : 2048;
	ret = read_image(&pt_misc, read_size, buff);
	if(ret){
		ERR("read misc area data failed.\n");
		return;
	}

	if(!strcmp(command, kFtsBootcmdRecovery) || !strcmp(command, kFtsBootcmdFastboot))
	{
		memset(msg->command, 0x0, sizeof(msg->command));
		strcpy(msg->command, command);
		write_image(&pt_misc, read_size, buff);
		if(0 != ret){
			ERR("set misc bootloader message failed.\n");
		}
	}
}

static void write_android_bootloader_message(const char *command,
					const char *status,
					const char *recovery)
{
	if (devinfo_exist) {
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

	if (devinfo_exist)
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

	if (devinfo_exist)
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

	if (devinfo_exist) {
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

	if (devinfo_exist) {
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
	int num = find_partition(DEVINFO_NAME);
	if(num > 0)
	{
		devinfo_exist = TRUE;
		read_boot_info_from_flash();
	}

	num = find_partition(MISC_NAME);
	if(num > 0)
		misc_exist = TRUE;

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

	if(devinfo_exist){
		dbg_level = get_debuglevel_from_boot_info();
	}

	return dbg_level;
}

void get_serialno_from_flash(char * serilno_buf)
{
	if(serilno_buf && devinfo_exist){
		strcpy(serilno_buf, get_serialno_from_boot_info());
	}
}

void get_macaddr_from_flash(char * mac_addr)
{
 	if(mac_addr && devinfo_exist){
		strcpy(mac_addr, get_macaddr_from_boot_info());
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
static int BootAB_sel = BOOTSEL_A;
static struct misc_boot_ctrl misc_bctrl __attribute__((aligned(4)))= {0};

static bool bootctrl_is_exist(void) {
	return devinfo_exist || misc_exist;
}

static bool bootctrl_is_valid(void)
{
	if (bootctrl_is_exist()) {
		return ((BOOTCTRL_MAGIC == misc_bctrl.magic) &&
			(MISC_BOOT_CONTROL_VERSION == misc_bctrl.version));
	}
	return 0;
}

static bool slot_is_bootable(int slot_num)
{
	if (bootctrl_is_exist()) {
		struct misc_slot_metadata *pABSlot = &misc_bctrl.slot_info[slot_num];
		return (pABSlot->priority > 1 && pABSlot->verity_corrupted == 0 &&
			(pABSlot->successful_boot || (pABSlot->tries_remaining > 0)));
	}
	return 0;
}

static int bootab_slot_select(void)
{
	if (bootctrl_is_exist()) {
		if(misc_bctrl.slot_info[0].priority != misc_bctrl.slot_info[1].priority)
			return (misc_bctrl.slot_info[0].priority - misc_bctrl.slot_info[1].priority) > 0 ? BOOTSEL_A : BOOTSEL_B;

		if(misc_bctrl.slot_info[0].successful_boot != misc_bctrl.slot_info[1].successful_boot)
			return (misc_bctrl.slot_info[0].successful_boot - misc_bctrl.slot_info[1].successful_boot) > 0 ? BOOTSEL_A : BOOTSEL_B;

		if(misc_bctrl.slot_info[0].tries_remaining != misc_bctrl.slot_info[1].tries_remaining)
			return (misc_bctrl.slot_info[0].tries_remaining - misc_bctrl.slot_info[1].tries_remaining) > 0 ? BOOTSEL_A : BOOTSEL_B;
	}

	return BOOTSEL_DEFAULT;
}

static int write_bootctrl_metadata(void * pbootctrl)
{
	if(NULL == pbootctrl) {
		ERR("ERROR: unvalid bootctrl metadata for write !\n");
		return -1;
	}

	if (bootctrl_is_exist()) {
#ifdef CONFIG_VT
		ver_table_entry_t pt_misc;
#elif CONFIG_GPT
		struct gpt_ent pt_misc;
#endif
		long long ret;
		long long start = 0;
		int num = find_partition(MISC_NAME);
		unsigned char * buff = (unsigned char *)(TEMP_BUF_ADDR);

		if(num >= 0) {
			fetch_partition_info(num, &pt_misc);
		} else {
			ERR("find misc partition error!\n");
		}

		memcpy(buff, (unsigned char *)pbootctrl, sizeof(struct misc_boot_ctrl));

		/* bootctrl_metadata in misc partition position */
#ifdef CONFIG_VT
		start = pt_misc.part1_start_blkind * get_block_size() + SYNA_SPACE_OFFSET_IN_MISC;
		ret = erase_flash((start & (~(get_block_size() - 1))), get_block_size());
		if(0 != ret){
			ERR("erase misc bootloader message failed.\n");
			return -1;
		}

		ret =write_flash(start, get_page_size(), buff);
#elif CONFIG_GPT
		start = pt_misc.ent_lba_start * 512 + SYNA_SPACE_OFFSET_IN_MISC;
		ret =write_flash(start, sizeof(struct misc_boot_ctrl), buff);
#endif
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

	if (bootctrl_is_exist()) {
		struct misc_boot_ctrl * pbootctrl = &misc_bctrl;

		memset((void *)pbootctrl, 0, sizeof(struct misc_boot_ctrl));

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
			if(devinfo_exist){
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

	if(bootctrl_is_valid()) {
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
	else if (bootctrl_is_exist()) {
		ERR("No valid metadata for bootctrl, initialize to default slot %d !\n", BOOTSEL_DEFAULT);
		init_bootctrl(BOOTSEL_DEFAULT);
		bootab_sel = BOOTSEL_DEFAULT;
	} else
		bootab_sel = GenX_BCM_Get_Boot_Slot();

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
	if (bootctrl_is_exist()) {
		long long ret;
		long long start = 0;
		int read_size;
		unsigned char * buff = (unsigned char *)(TEMP_BUF_ADDR);
#ifdef CONFIG_GPT
		struct gpt_ent gpt_misc;

		int num = find_partition(MISC_NAME);
		if(num >= 0) {
			fetch_partition_info(num, &gpt_misc);
		} else {
			ERR("find fts partition error!\n");
		}
		read_size = (get_page_size() > 2048) ? get_page_size() : 2048;
		start = gpt_misc.ent_lba_start * 512 + SYNA_SPACE_OFFSET_IN_MISC;
#endif
#ifdef CONFIG_VT
		ver_table_entry_t vt_misc;

		int num = find_partition(MISC_NAME);

		if(num >= 0) {
			fetch_partition_info(num, &vt_misc);
		} else {
			ERR("find fts partition error!\n");
		}

		read_size = (get_page_size() > 2048) ? get_page_size() : 2048;
		start = vt_misc.part1_start_blkind;
		start *= get_block_size();
		start += SYNA_SPACE_OFFSET_IN_MISC;
#endif
		ret =read_flash(start, read_size, buff);
		if(0 != ret){
			ERR("read misc boot control data failed.\n");
		}
		memcpy((unsigned char *)&misc_bctrl, buff, sizeof(struct misc_boot_ctrl));
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
	if (bootctrl_is_exist()) {
		if(bootctrl_is_valid() && (misc_bctrl.slot_info[abmode_sel].tries_remaining > 0)) {
			misc_bctrl.slot_info[abmode_sel].tries_remaining -= 1;
			write_bootctrl_metadata((void*)&misc_bctrl);
			return 0;
		}
		else if(bootctrl_is_valid()) {
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
	if (devinfo_exist) {
		ret = (misc_bctrl.slot_info[BootAB_sel].successful_boot != 0);
	} else {
		//should never reach here
	}
	return ret;
}

// vim: noexpandtab
