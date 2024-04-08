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
//! \file	       boot_devinfo.c
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

#include "boot_devinfo.h"

boot_info_t boot_info;

void read_boot_info_from_flash(void)
{
	struct gpt_ent gpt_devinfo;
	unsigned char * buff = (unsigned char *)(TEMP_BUF_ADDR);

	int num = find_partition(DEVINFO_NAME);
	if(num >= 0) {
		fetch_partition_info(num, &gpt_devinfo);
	} else {
		ERR("find fts partition error!\n");
	}

	if(read_image(&gpt_devinfo, sizeof(boot_info_t), buff)){
		ERR("set fts boot message failed.\n");
	}
	else
	{
		memcpy((unsigned char *)&boot_info, buff, sizeof(boot_info_t));
		INFO("command: %s\n mac: %s\n serialno: %s\n", boot_info.command, boot_info.mac_addr, boot_info.serialno);
	}

}

void write_boot_info_to_flash(boot_info_t * pBoot_info)
{
	struct gpt_ent gpt_devinfo;
	unsigned char * buff = (unsigned char *)(TEMP_BUF_ADDR);

	int num = find_partition(DEVINFO_NAME);
	if(num >= 0) {
		fetch_partition_info(num, &gpt_devinfo);
	} else {
		ERR("find devinfo partition error!\n");
	}

	memcpy(buff, (unsigned char *)pBoot_info, sizeof(boot_info_t));
	if(write_image(&gpt_devinfo, sizeof(boot_info_t), buff)){
		ERR("write boot info to flash failed.\n");
	}
}

boot_info_t *get_boot_info(void)
{
	return &boot_info;
}

char * get_command_from_boot_info(void)
{
	return boot_info.command;
}

char * get_macaddr_from_boot_info(void)
{
	return boot_info.mac_addr;
}

char * get_serialno_from_boot_info(void)
{
	return boot_info.serialno;
}

char * get_bootoption_from_boot_info(void)
{
	return boot_info.boot_option;
}

int get_debuglevel_from_boot_info(void)
{
	return boot_info.debeg_level;
}

void set_command_to_boot_info(const char * command)
{
	strcpy(boot_info.command, command);
	write_boot_info_to_flash(&boot_info);
}

void set_macaddr_to_boot_info(const char * macaddr)
{
	strcpy(boot_info.mac_addr, macaddr);
	write_boot_info_to_flash(&boot_info);
}

void set_serialno_to_boot_info(const char * serialno)
{
	strcpy(boot_info.serialno, serialno);
	write_boot_info_to_flash(&boot_info);
}

void set_bootoption_to_boot_info(const char * bootoption)
{
	if(bootoption)
		strcpy(boot_info.boot_option, bootoption);
	else
		memset(boot_info.boot_option, 0x0, 32);

	write_boot_info_to_flash(&boot_info);
}

void set_debuglevel_to_boot_info(int default_debug_level)
{
	boot_info.debeg_level = default_debug_level;
	write_boot_info_to_flash(&boot_info);
}
