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
#if (defined(SM_RAM_TS_ENABLE) || defined (SOC_RAM_TS_ENABLE))

#include "string.h"
#ifdef SM_RAM_TS_ENABLE
#include "sm_type.h"
#include "sm_printf.h"
#include "sm_rt_module.h"
#include "SysMgr.h"
#include "sm_mem_map_itcm.h"
#else
#include "debug.h"
#include "system_manager.h"
#include "mem_map_itcm.h"
#endif

#include "ram_ts.h"
#include "ramts_errno.h"

#ifdef SM_RAM_TS_ENABLE
extern unsigned long strtoul(const char *startp,char **endp,unsigned int base);
#else
extern unsigned long str2ul(const char *cp,char **endp,unsigned int base);
#endif
/* Compatibility defines to keep code as similar as possible
 * to the Linux version.
 */
#define DRV_NAME		"ramts"

#define likely(x)		__builtin_expect(!!(x), 1)
#define unlikely(x)		__builtin_expect(!!(x), 0)

static struct ram_ts *__ts = NULL;

static char *ram_ts_key_search(char *ts_data, const char *key,
                           size_t len)
{
	if (ts_data != NULL) {
		while (*ts_data != 0) {
			if (!strncmp(ts_data, key, len)) {
				if (*(ts_data+len) == '=')
					return ts_data;
			}

			ts_data += strlen(ts_data) + 1;
		}
	}
	return NULL;
}

static inline UINT32 ram_ts_crc(const struct ram_ts *cache)
{
	const unsigned char *p;
	UINT32 crc = 0;
	size_t len;

	/* skip magic and crc fields */
	len = cache->len + 2 * sizeof(UINT32);
	p = (const unsigned char*)&cache->len;

	while (len--) {
		int i;

		crc ^= *p++;
		for (i = 0; i < 8; i++)
			crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320 : 0);
	}
	return crc ^ ~0;
}

/* Verifies cache consistency and locks it */
static struct ram_ts *__ram_ts_get(void)
{
	struct ram_ts *ts = __ts;

	if (likely(ts)) {
		if (unlikely(ts->crc != ram_ts_crc(ts))) {
			dbg_printf(PRN_RES,DRV_NAME
				": memory corruption detected\n");
			ts = NULL;
		}
	} else {
		dbg_printf(PRN_ERR,DRV_NAME ": not initialized yet\n");
	}

	return ts;
}

static inline void __ram_ts_put(void)
{
}

int ram_ts_set(const char *key, const char *value)
{
	struct ram_ts *ts;
	size_t klen = strlen(key);
	size_t vlen = strlen(value);
	int res = 0;
	char *p;

	ts = __ram_ts_get();
	if (unlikely(!ts))
		return -EINVAL;

	p = ram_ts_key_search(ts->data, key, klen);
	if (p) {
		/* we are replacing existing entry,
		* empty value (vlen == 0) removes entry completely.
		*/
		dbg_printf(PRN_INFO,DRV_NAME ": ram_ts_set: replacing existing entry !\n");

		size_t cur_len = strlen(p) + 1;
		size_t new_len = vlen ? klen + 1 + vlen + 1 : 0;

		if (cur_len != new_len) {
			/* we need to move stuff around */

			if ((ts->len - cur_len) + new_len >
				sizeof(ts->data))
				goto no_space;

			memmove(p + new_len, p + cur_len,
			ts->len - (p - ts->data + cur_len));

			ts->len = (ts->len - cur_len) + new_len;
		} else if (!strcmp(p + klen + 1, value)) {
			/* skip update if new value is the same as the old one */
			res = 0;
			goto out;
		}

		if (vlen) {
			p += klen + 1;
			memcpy(p, value, vlen);
			p[vlen] = '\0';
		}
	} else {
		size_t len = klen + 1 + vlen + 1;

		/* don't do anything if value is empty */
		if (!vlen) {
			res = 0;
			goto out;
		}

		if (ts->len + len > sizeof(ts->data))
			goto no_space;

		/* add new entry at the end */
		p = ts->data + ts->len - 1;
		memcpy(p, key, klen);
		p += klen;
		*p++ = '=';
		memcpy(p, value, vlen);
		p += vlen;
		*p++ = '\0';
		*p = '\0';
		ts->len += len;
	}

	ts->crc = ram_ts_crc(ts);
	goto out;

no_space:
	dbg_printf(PRN_ERR,DRV_NAME ": no space left for '%s=%s'\n", key, value);
	res = -ENOSPC;
out:
	__ram_ts_put();

	return res;
}

void ram_ts_get(const char *key, char *value, unsigned int size)
{
	size_t klen = strlen(key);
	struct ram_ts *ts;
	const char *p;

	*value = '\0';

	ts = __ram_ts_get();
	if (unlikely(!ts))
		return;

	p = ram_ts_key_search(ts->data, key, klen);
	if (p)
	{
		strncpy(value, p + klen + 1, size);
	}

	__ram_ts_put();
}

int ram_ts_get_int(const char *key, int default_value)
{
	char value[16];
	int res;

	ram_ts_get(key, value, sizeof(value));
	if (!value[0])
		return default_value;

#ifdef SM_RAM_TS_ENABLE
	res = (int)strtoul(value, (char**)NULL, 0);
#else
	res = (int)str2ul(value, (char**)NULL, 0);
#endif
	return res;
}

#ifdef SM_RAM_TS_ENABLE
static int __attribute__((used)) ram_ts_init(void)
#else
int ram_ts_init(void)
#endif
{
	__ts = (struct ram_ts *)(RAM_TS_BASE);
	if (__ts->magic != RAM_TS_MAGIC || __ts->crc != ram_ts_crc(__ts)) {
		memset((void *)__ts, 0, RAM_TS_MAX_SIZE);

		/* write ramts information */
		__ts->magic = RAM_TS_MAGIC;
		__ts->version = 0x1;
		__ts->len = 1;
		__ts->data[0] = '\0';
		__ts->crc = ram_ts_crc(__ts);
	}
	return 0;
}

#ifdef SM_RAM_TS_ENABLE
DECLARE_RT_INIT(rtsinit, INIT_DEV_P_25, ram_ts_init);
#endif

#endif //#if (defined(SM_RAM_TS_ENABLE) || defined (SOC_RAM_TS_ENABLE))
