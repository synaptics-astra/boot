#ifndef RPMB_H
#define RPMB_H

#include "com_type.h"
#include "string.h"
#include "debug.h"
#include "mmc.h"
#include "emmc.h"
#include "cache.h"

#define RPMB_KEY_SIZE	32
#define RPMB_DATA_SIZE	256
#define RPMB_FILE_PATH_LEN	63
#define MAC_LEN			32
#define NONCE_LEN		16
/* if RPMB_USE_COMMERCIAL_KEY defined as 1, it will program secure key
   if RPMB_USE_COMMERCIAL_KEY defined as 0, it will program default key*/
#define RPMB_USE_COMMERCIAL_KEY		1
#define htobe16(x)	((x&0x00ff) << 8 | (x&0xff00) >> 8)
#define htobe32(x)	((x&0x000000ff) << 24 | (x&0x0000ff00) << 8 \
			| (x&0x00ff0000) >> 8 | (x&0xff000000) >> 24)
#define ALIGN(x, a)	(((x) + ((a) - 1)) & ~((a) - 1))

enum rpmb_op_type {
	MMC_RPMB_WRITE_KEY = 0x01,
	MMC_RPMB_READ_CNT  = 0x02,
	MMC_RPMB_WRITE     = 0x03,
	MMC_RPMB_READ      = 0x04,

	/* For internal usage only, do not use it directly */
	MMC_RPMB_READ_RESP = 0x05,
	MMC_RPMB_READ_SIZE = 0x06
};

typedef struct {
	uint8_t  stuff[196];
	uint8_t  keyMAC[MAC_LEN];
	uint8_t  data[RPMB_DATA_SIZE];
	uint8_t  nonce[NONCE_LEN];
	uint32_t writeCounter;
	uint16_t addr;
	uint16_t blockCount;
	uint16_t result;
	uint16_t reqResp;
} rpmbFrame;

//} rpmbFrame __attribute__((aligned(64)));



//RPMB FAST CALL
#define RPMB_FAST_CALL_SUB_ID	0x4

enum {
        RPMB_CONFIG_CMD,
        RPMB_ENABLE_CMD,
        RPMB_FRAME_PACK_CMD,
        RPMB_FRAME_UNPACK_CMD,

};

enum {
	RPMB_CAPACITY_ERROR = 0xF0,
	RPMB_CONFIG_ERROR = 0xF1,
	RPMB_ENABLE_ERROR = 0xF2,
	RPMB_KEY_PROGRAM_ERROR = 0xF3,
	RPMB_READ_COUNTER_ERROR = 0xF4,
	RPMB_KEY_NOT_PROGRAM_ERROR = 0xF5,
	RRPMB_FRAME_PACK_ERROR = 0xF6,
	RRPMB_FRAME_UNPACK_ERROR = 0xF7,
};

struct rpmb_fastcall_param {
	int rpmb_sub_cmd;               //in
	int rpmb_size;                  //in
	int rpmb_authkey_flag;          //out
	uint8_t rpmb_authkey[32];       //out
	rpmbFrame frame;                // in/out
};

struct rpmb_fastcall_generic_param {
	/*
	 * the first is always the sub command id
	 */
	unsigned int sub_cmd_id;   //4 for rpmb fastcall
	unsigned int param_len;
	struct rpmb_fastcall_param rpmb_param;
};


int rpmb_get_write_count(void);

int rpmb_read_blocks(int blk_index, int blk_num, void *buff, size_t buff_len);
int rpmb_write_blocks(int blk_index, int blk_num, void *buff, size_t buff_len);

#endif /* RPMB+H*/
