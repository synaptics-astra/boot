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

#include <stdint.h>
#include "libfdt.h"
#include "libfdt_syna.h"
#include "string.h"
#include "debug.h"
#ifdef PV_COMP
#include "pmic_select.h"
#endif

/*
 * Add or modify node into the provided FDT.
 * add ramdisk addr/ramdisk size and modify cmdline to FDT.
 * REturn values:
 *	 = 0 -> pretend success
 */
int set_fdt(void *fdt, int total_space,unsigned int initrd_start,unsigned int initrd_size,char * cmdline)
{
	int ret;
	int offset = 0;

	/* let's give it all the room it could need */
	ret = fdt_open_into(fdt, fdt, total_space);
	if (ret < 0)
		return ret;

	/* find the offset in FDT */
	offset = fdt_path_offset(fdt,"/chosen");
	if (offset == -FDT_ERR_NOTFOUND){
		offset = fdt_add_subnode(fdt, 0, "/chosen");
		if (offset < 0)
			return offset;
	}

	/* modify the bootargs in FDT */
	if(cmdline != NULL)
	{
		ret = fdt_setprop_string(fdt, offset, "bootargs", cmdline);
		if(ret)
			return ret;
	}

	if(initrd_size != 0) {
		/* add ramdisk addr/ramdisk size to FDT */
		ret = fdt_setprop_cell(fdt, offset,"linux,initrd-start",initrd_start);
		if(ret)
			return ret;
		ret = fdt_setprop_cell(fdt, offset,"linux,initrd-end",initrd_start + initrd_size);
		if(ret)
			return ret;
	}

	return fdt_pack(fdt);
}

static unsigned int g_fdt = 0;
static int g_total_space = 0;

static int check_fdt_validate(void)
{
	if((g_fdt == 0) || (g_total_space <= 0))
		return FDT_ERR_NOTFOUND;

	return 0;
}

int fdt_set_reserved_mem(void *fdt, void * mem, unsigned int reserved_num)
{
	struct fdt_header * header = (struct fdt_header *)fdt;
	unsigned int offset = fdt32_to_cpu(header->off_mem_rsvmap);
	struct reserve_entry * usr_reserve = (struct reserve_entry *)mem;
	int n;

	struct fdt_reserve_entry * mem_reserve = (struct fdt_reserve_entry *)(((unsigned char *)fdt) + offset);

	for (n = 0; ; n++) {
		mem_reserve += n;

		if(n < reserved_num) {
			if(NULL == usr_reserve)
				return -1;

			usr_reserve += n;
			mem_reserve->address = cpu_to_fdt64(usr_reserve->reserved_mem_start);
			mem_reserve->size = cpu_to_fdt64(usr_reserve->reserved_mem_size);
		}
		else {
			if(!fdt64_to_cpu(mem_reserve->size))
				break;
			mem_reserve->address = cpu_to_fdt64(0x0);
			mem_reserve->size = cpu_to_fdt64(0x0);
		}
	}

	return 0;
}

/* set the address and size of dtb first */
void set_fdt_addr(unsigned int fdt, int total_space)
{
	g_fdt = fdt;
	g_total_space = total_space;
}

int get_fdt_addr(unsigned int *fdt, int * total_space)
{
	if(check_fdt_validate() == 0) {
		*fdt = g_fdt;
		*total_space = g_total_space;

		return 0;
	}

	return FDT_ERR_NOTFOUND;
}

/* pass system info (rkek_id, chip_ver) to kernel.*/
int fdt_add_system_info(void *fdt,
		int total_space,
		const char *pnode,
		const system_info_t *system_info)
{
	int ret;
	int offset = 0;
        char buf[128] = {0};

	ret = fdt_open_into(fdt, fdt, total_space);
	if (ret < 0)
		return ret;

	offset = fdt_path_offset(fdt, pnode);
	if (offset == -FDT_ERR_NOTFOUND){
		offset = fdt_add_subnode(fdt, 0, pnode);
		if (offset < 0)
			return offset;
	}

	sprintf(buf, "%08x\n", system_info->chip_ver);
	ret = fdt_setprop(fdt, offset, "system_rev", buf, strlen(buf) + 1);
	if(ret)
		return ret;

	sprintf(buf, "%08x%08x\n", system_info->system_serial_low, system_info->system_serial_high);
	ret = fdt_setprop(fdt, offset, "system_serial", buf, strlen(buf) + 1);
	if(ret)
		return ret;

	sprintf(buf, "Revision\t: %08x\nSerial\t\t: %08x%08x\n", system_info->chip_ver, system_info->system_serial_low, system_info->system_serial_high);
	ret = fdt_setprop(fdt, offset, "system_info", buf, strlen(buf) + 1);
	if(ret)
		return ret;

	sprintf(buf, "%08x\n", system_info->leakage_current);
	ret = fdt_setprop(fdt, offset, "leakage_current", buf, strlen(buf) + 1);
	if(ret)
		return ret;

	sprintf(buf, "%08x\n", system_info->pvcomp_rev);
	ret = fdt_setprop(fdt, offset, "pvcomp_rev", buf, strlen(buf) + 1);
	if(ret)
		return ret;

	return fdt_pack(fdt);
}

/* update the i2c slave address of pmic */
int fdt_update_i2c_slave_addr(void *fdt, int total_space, unsigned int masterid, unsigned int slave_addr)
{
	int ret;
	int offset = 0;
	unsigned int reg = 0;

	/* let's give it all the room it could need */
	ret = fdt_open_into(fdt, fdt, total_space);
	if (ret < 0)
		return ret;

	if(masterid == 0)
		offset = fdt_path_offset(fdt, "/soc/apb@e80000/i2c@1400/pg867");
	else
		offset = fdt_path_offset(fdt, "/soc/apb@e80000/i2c@3400/pg867");
	if (offset == -FDT_ERR_NOTFOUND) {
		ERR("didn't find point of i2c\n");
		return -1;
	}

	reg = cpu_to_fdt32(slave_addr);

	fdt_setprop(fdt, offset, "reg", &reg, sizeof(unsigned int));

	return fdt_pack(fdt);
}


/* pass opp table to kernel */
int fdt_add_opp(void *fdt, int total_space, unsigned int * opp, int count)
{
	int ret;
	int offset = 0, i = 0;

	if(count == 0) {
		NOTICE("opp table is NULL!!!!\n");
		return -1;
	}

	for(i = 0; i < count; i++) {
		opp[i] = cpu_to_fdt32(opp[i]);
	}

	/* let's give it all the room it could need */
	ret = fdt_open_into(fdt, fdt, total_space);
	if (ret < 0)
		return ret;

	offset = fdt_path_offset(fdt, "/cpus/cpu@0");
	if (offset == -FDT_ERR_NOTFOUND) {
		ERR("didn't find point of cpus\n");
		return -1;
	}

	fdt_setprop(fdt, offset, "operating-points", opp, sizeof(unsigned int) * count);
	return fdt_pack(fdt);
}

static void update_opp(void *buf, int offset, unsigned int vh, unsigned int vl)
{
	int node;
	const unsigned int *p;
	unsigned int v1, v3, vol_h[3], vol_l[3];

	/* opp-microvolt = <vh vh vh>; */
	vol_h[2] = vol_h[1] = vol_h[0] = cpu_to_fdt32(vh);

	/* opp-microvolt = <vl vl vh>; */
	vol_l[2] = cpu_to_fdt32(vh);
	vol_l[1] = vol_l[0] = cpu_to_fdt32(vl);

	for(node = fdt_first_subnode(buf, offset); node >= 0;
			node = fdt_next_subnode(buf, node)) {
		p = fdt_getprop(buf, node, "opp-microvolt", NULL);
		if (!p)
			continue;
		v1 = fdt32_to_cpu(*p);
		p += 2;
		v3 = fdt32_to_cpu(*p);
		if (v1 < v3)
			fdt_setprop(buf, node, "opp-microvolt", vol_l, sizeof(vol_l));
		else
			fdt_setprop(buf, node, "opp-microvolt", vol_h, sizeof(vol_h));
	}
}

void fdt_set_vcpu_opp(void * fdt, unsigned int vcpuh, unsigned int vcpul)
{
	int offset;

	offset = fdt_path_offset(fdt, "/cpus/opp_table0");
	if (offset < 0) {
		ERR("didn't find point of /cpus/opp_table0\n");
		return;
	}

	update_opp(fdt, offset, vcpuh, vcpul);
}

void fdt_set_vcore_opp(void * fdt, unsigned int vcoreh, unsigned int vcorel)
{
	int offset;

	offset = fdt_path_offset(fdt, "/soc/vcore_opp_table");
	if (offset < 0) {
		ERR("didn't find point of /soc/vcore_opp_table\n");
		return;
	}

	update_opp(fdt, offset, vcoreh, vcorel);
}

int fdt_set_leakage(void *fdt, int total_space, unsigned int leakage_num)
{
	int ret;
	int offset = 0;
	unsigned int leakage = 0x0;

	/* let's give it all the room it could need */
	ret = fdt_open_into(fdt, fdt, total_space);
	if(ret < 0)
		return ret;

	/* find the offset in FDT*/
	offset = fdt_path_offset(fdt, "/soc/cpm");
	if(offset == -FDT_ERR_NOTFOUND) {
		ERR("didn't find point of /soc/cpm\n");
		return offset;
	}

	leakage = cpu_to_fdt32(leakage_num);
	fdt_setprop(fdt, offset, "leakage", (const void *)(uintptr_t)(&leakage), sizeof(leakage));

	return fdt_pack(fdt);
}

int fdt_set_chiprev(void *fdt, int total_space, unsigned int chip_rev)
{
	int ret;
	int offset = 0;
	unsigned int rev = 0x0;

	/* let's give it all the room it could need */
	ret = fdt_open_into(fdt, fdt, total_space);
	if(ret < 0)
		return ret;

	/* find the offset in FDT*/
	offset = fdt_path_offset(fdt, "/soc/chipid");
	if(offset == -FDT_ERR_NOTFOUND) {
		ERR("didn't find point of /soc/chipid\n");
		return offset;
	}

	rev = cpu_to_fdt32(chip_rev);
	fdt_setprop(fdt, offset, "chip-revision", (const void *)(uintptr_t)(&rev), sizeof(rev));

	return fdt_pack(fdt);
}


#ifdef EMMC_BOOT
extern unsigned int do_emmc_get_param_customized(void);
int fdt_set_mmc_param(void *fdt, int total_space)
{
	int offset = 0, ret = 0;
	unsigned int dll_phsel0 = do_emmc_get_param_customized();

	if(!dll_phsel0) {
		ERR("Fail to match Vendor, use default eMMC param !\n");
		goto Out;
	}

	/* let's give it all the room it could need */
	ret = fdt_open_into(fdt, fdt, total_space);
	if(ret < 0) {
		ERR("Fail to do eMMC param customization for dll_phsel0 !\n");
		goto Out;
	}

	offset = fdt_path_offset(fdt, "/soc/sdhci@aa0000");
	if(offset == -FDT_ERR_NOTFOUND) {
		ERR("can't find /soc/sdhci@aa0000 node in dtb !\n");
		ret = offset;
		goto Out;
	}

	ERR("INFO: set eMMC dll_phsel0 to 0x%x for kernel.\n", dll_phsel0);
	dll_phsel0 = cpu_to_fdt32(dll_phsel0);
	fdt_setprop(fdt, offset, "marvell,xenon-phy-dll-phsel0", (const void *)(uintptr_t)(&dll_phsel0),
						sizeof(dll_phsel0));
	ret = fdt_pack(fdt);

Out:
	return ret;
}
#endif
