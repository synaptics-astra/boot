#include "com_type.h"
#include "util.h"
#include "string.h"
#include "debug.h"
#include "mmc.h"
#include "rpmb.h"
#include "cache.h"
#include "tz_nw_boot.h"
#include "bootloader_header.h"

#define PARTITION_RPMB		3

#define WRITE_TRUE			1
#define WRITE_FALSE			0

int rpmb_get_write_count(void)
{
	rpmbFrame *frame_in;
	int cur_part = 0;
	unsigned int ret = -1;

	cur_part = do_emmc_get_current_part();
	if (cur_part != PARTITION_RPMB)
		if(do_emmc_switch_part(PARTITION_RPMB)) {
			ERR("RPMB: failed to switch partition %d\n", PARTITION_RPMB);
			return -1;
		}

	frame_in = malloc_ion_cacheable(sizeof(rpmbFrame) + 64);
	if (!frame_in) {
		ERR("RPMB: failed to alloc ion cacheable memory \n");
		ret = -1;
		goto out2;
	}
	if ((uintptr_t)frame_in % 64) {
		lgpl_printf("frame_in 0x%x bytpe align to 0x%x.\n", frame_in, ALIGN((uintptr_t)frame_in, 64));
		frame_in = (rpmbFrame *)ALIGN((uintptr_t)frame_in, 64);
	}

	memset(frame_in, 0, sizeof(rpmbFrame));
	frame_in->reqResp = htobe16(MMC_RPMB_READ_CNT);

	flush_dcache_range((void*)(uintptr_t)frame_in, (void*)((uintptr_t)frame_in + sizeof(rpmbFrame)));

	ret = rpmb_request((uintptr_t)frame_in, sizeof(rpmbFrame), WRITE_FALSE);
	if (0 != ret) {
		ERR("RPMB: rpmb failed to request write conunter!!!\n");
		goto out1;
	}

	memset(frame_in, 0, sizeof(rpmbFrame));
	invalidate_dcache_range((void*)(uintptr_t)frame_in, (void*)((uintptr_t)frame_in + sizeof(rpmbFrame)));

	ret = rpmb_response((uintptr_t)frame_in, sizeof(rpmbFrame));
	if (0 != ret) {
		ERR("RPMB: rpmb failed to response write conunter!!!\n");
		goto out1;
	}

	invalidate_dcache_range((void*)(uintptr_t)frame_in, (void*)((uintptr_t)frame_in + sizeof(rpmbFrame)));
	ret = htobe32(frame_in->writeCounter);
	dbg_printf(PRN_INFO, "!!!!!!! Response type: 0x%x, write_counter:0x%x, results: 0x%x !\n",
		htobe16(frame_in->reqResp), htobe32(frame_in->writeCounter), htobe16(frame_in->result));

out1:
	free_ion_cacheable(frame_in);
out2:
	do_emmc_switch_part(cur_part);

	return ret;
}

int rpmb_read_blocks(int blk_index, int blk_num, void *buff, size_t buff_len)
{
	int cur_part = 0;
	rpmbFrame *frame_in;
	rpmbFrame *frame_out;
	unsigned char *data;
	unsigned int ret = -1;

	if (!buff || (buff_len < blk_num * RPMB_DATA_SIZE))
		return -1;

	cur_part = do_emmc_get_current_part();
	if (cur_part != PARTITION_RPMB)
		if(do_emmc_switch_part(PARTITION_RPMB)) {
			ERR("RPMB: failed to switch partition %d\n", PARTITION_RPMB);
			return -1;
		}

	frame_in = malloc_ion_cacheable(sizeof(rpmbFrame) + 64);
	if (!frame_in) {
		ERR("RPMB: failed to alloc ion cacheable memory \n");
		ret = -1;
		goto out3;
	}
	if ((uintptr_t)frame_in % 64) {
		lgpl_printf("frame_in 0x%x bytpe align to 0x%x.\n", frame_in, ALIGN((uintptr_t)frame_in, 64));
		frame_in = (rpmbFrame *)ALIGN((uintptr_t)frame_in, 64);
	}
	memset(frame_in, 0, sizeof(rpmbFrame));

	data = malloc_ion_cacheable(blk_num * MMC_BLOCK_SIZE + 64);
	if (!data) {
		ERR("RPMB: failed to alloc ion cacheable memory \n");
		ret = -1;
		goto out2;
	}
	if ((uintptr_t)data % 64) {
		lgpl_printf("data 0x%x bytpe align to 0x%x.\n", data, ALIGN((uintptr_t)data, 64));
		data = (unsigned char *)ALIGN((uintptr_t)data, 64);
	}
	memset(data, 0, sizeof(rpmbFrame));

	frame_in->reqResp = htobe16(MMC_RPMB_READ);
	frame_in->addr = htobe16(blk_index);

	flush_dcache_range((void*)(uintptr_t)frame_in, (void*)((uintptr_t)frame_in + sizeof(rpmbFrame)));

	ret = rpmb_request((uintptr_t)frame_in, sizeof(rpmbFrame), WRITE_FALSE);
	if (0 != ret) {
		ERR("RPMB: rpmb failed to request read block!!!\n");
		goto out1;
	}

	invalidate_dcache_range((void*)(uintptr_t)data, (void*)((uintptr_t)data +  MMC_BLOCK_SIZE * blk_num));

	ret = rpmb_response((uintptr_t)data, MMC_BLOCK_SIZE * blk_num);
	if (0 != ret) {
		ERR("RPMB: rpmb failed to response read block!!!\n");
		goto out1;
	}

	invalidate_dcache_range((void*)(uintptr_t)data, (void*)((uintptr_t)data + MMC_BLOCK_SIZE * blk_num));

	frame_out = (rpmbFrame *)data;
	lgpl_printf("!!!!!!! Response type: 0x%x, write_counter:0x%x, results: 0x%x !\n",
		htobe16(frame_out->reqResp), htobe32(frame_out->writeCounter), htobe16(frame_out->result));

	memcpy(buff, frame_out->data, buff_len);

	ret = 0;
out1:
	free_ion_cacheable(data);
out2:
	free_ion_cacheable(frame_in);
out3:
	do_emmc_switch_part(cur_part);

	return ret;
}

int rpmb_write_blocks(int blk_index, int blk_num, void *buff, size_t buff_len)
{
	int ret = -1;
	int cur_part = 0;
	rpmbFrame *frame_in;
	rpmbFrame *frame_req;
	uint32_t write_count;
	int rpmb_fastcall_generic_param_size;
	struct rpmb_fastcall_generic_param *pGenericCallParam = NULL;

	if (!buff || (buff_len < blk_num * RPMB_DATA_SIZE))
		return -1;

	cur_part = do_emmc_get_current_part();
	if (cur_part != PARTITION_RPMB)
		if(do_emmc_switch_part(PARTITION_RPMB)) {
			ERR("RPMB: failed to switch partition %d\n", PARTITION_RPMB);
			return -1;
		}

	rpmb_fastcall_generic_param_size = sizeof(struct rpmb_fastcall_generic_param);
	pGenericCallParam = (struct rpmb_fastcall_generic_param *)malloc_ion_cacheable(rpmb_fastcall_generic_param_size);
	if(!pGenericCallParam) {
		ERR("RPMB: failed to alloc ion cacheable memory for pGenericCallParam\n");
		ret = -1;
		goto out3;
	}

	frame_req = malloc_ion_cacheable(sizeof(rpmbFrame) + 64);
	if (!frame_req) {
		ERR("RPMB: failed to alloc ion cacheable memory for frame_req\n");
		ret = -1;
		goto out2;
	}
	if ((uintptr_t)frame_req % 64) {
		lgpl_printf("frame_in 0x%x bytpe align to 0x%x.\n", frame_req, ALIGN((uintptr_t)frame_req, 64));
		frame_req = (rpmbFrame *)ALIGN((uintptr_t)frame_req, 64);
	}

	memset((void *)pGenericCallParam, 0x00, rpmb_fastcall_generic_param_size);

	frame_in = &pGenericCallParam->rpmb_param.frame;

	write_count = rpmb_get_write_count();
	if (write_count < 0)
	{
		ERR("RPMB: rpmb failed to get write counter\n");
		ret = -1;
		goto out1;
	}

	frame_in->reqResp = htobe16(MMC_RPMB_WRITE);
	frame_in->addr = htobe16(blk_index);
	frame_in->blockCount = htobe16(1);
	frame_in->writeCounter = htobe32(write_count);
	memcpy(frame_in->data, buff, buff_len);

	dbg_printf(PRN_ERR, "INFO: Fastcall to PACK RPMB Frame!\n");
	pGenericCallParam->sub_cmd_id = RPMB_FAST_CALL_SUB_ID;
	pGenericCallParam->rpmb_param.rpmb_sub_cmd = RPMB_FRAME_PACK_CMD;
	pGenericCallParam->param_len = sizeof(struct rpmb_fastcall_param);

	flush_dcache_range((void*)pGenericCallParam, (void*)pGenericCallParam + rpmb_fastcall_generic_param_size);

	ret = tz_nw_rpmb_ops((void *)pGenericCallParam, rpmb_fastcall_generic_param_size);
	if(ret)
	{
		dbg_printf(PRN_ERR, "INFO: Fastcall to PACK RPMB Frame fail !\n");
		ret = RRPMB_FRAME_PACK_ERROR;
		goto out1;
	}
	dbg_printf(PRN_ERR, "INFO: Fastcall to PACK RPMB Frame Done!\n");

	memcpy(frame_req, frame_in, sizeof(rpmbFrame));

	flush_dcache_range((void*)(uintptr_t)frame_req, (void*)((uintptr_t)frame_req + sizeof(rpmbFrame)));

	// send request frame
	ret = rpmb_request((uintptr_t)frame_req, sizeof(rpmbFrame), WRITE_TRUE);
	if (0 != ret) {
		ERR("RPMB: rpmb failed to request write block!!!\n");
		goto out1;
	}

	memset(frame_req, 0, sizeof(rpmbFrame));
	frame_req->reqResp = htobe16(MMC_RPMB_READ_RESP);
	flush_dcache_range((void*)(uintptr_t)frame_req, (void*)((uintptr_t)frame_req + sizeof(rpmbFrame)));

	// send request result frame
	ret = rpmb_request((uintptr_t)frame_req, sizeof(rpmbFrame), WRITE_FALSE);
	if (0 != ret) {
		ERR("RPMB: rpmb failed to request write block!!!\n");
		goto out1;
	}

	// get response
	memset(frame_req, 0, sizeof(rpmbFrame));
	flush_dcache_range((void*)(uintptr_t)frame_req, (void*)((uintptr_t)frame_req + sizeof(rpmbFrame)));

	invalidate_dcache_range((void*)(uintptr_t)frame_req, (void*)((uintptr_t)frame_req + sizeof(rpmbFrame)));
	rpmb_response((uintptr_t)frame_req, sizeof(rpmbFrame));
	if (0 != ret) {
		ERR("RPMB: rpmb failed to response write block!!!\n");
		goto out1;
	}

	invalidate_dcache_range((void*)(uintptr_t)frame_in, (void*)((uintptr_t)frame_in + sizeof(rpmbFrame)));

	lgpl_printf("!!!!!!! Response type: 0x%x, write_counter:0x%x, results: 0x%x !\n",
		htobe16(frame_req->reqResp), htobe32(frame_req->writeCounter), htobe16(frame_req->result));

	ret = 0;

out1:
	free_ion_cacheable(frame_req);
out2:
	free_ion_cacheable(pGenericCallParam);
out3:
	do_emmc_switch_part(cur_part);

	return ret;
}

