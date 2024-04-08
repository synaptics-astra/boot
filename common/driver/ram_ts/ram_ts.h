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

#ifndef _RAM_TS_H
#define _RAM_TS_H

#define RAM_TS_MAGIC	(0x53544D52)	/*RMTS*/

/* Keep in sync with 'struct ram_ts' */
#define RAM_TS_HDR_SIZE		(4 * sizeof(UINT32))
#define RAM_TS_MAX_DATA_SIZE		(RAM_TS_MAX_SIZE - RAM_TS_HDR_SIZE)

/* Physical flash layout */
struct ram_ts {
	UINT32 magic;		/* "RMTS" */
	UINT32 crc;			/* doesn't include magic and crc fields */
	UINT32 len;			/* real size of data */
	UINT32 version;		/* we fix it to '0x1' */

	/* data format is very similar to Unix environment:
	 *   key1=value1\0key2=value2\0\0
	 */
	char data[RAM_TS_MAX_DATA_SIZE];
};

#ifndef SM_RAM_TS_ENABLE
int ram_ts_init(void);
#endif

/* Get/set value, returns 0 on success */
int ram_ts_set(const char *key, const char *value);
void ram_ts_get(const char *key, char *value, unsigned int size);

/* Get value as an integer, if missing/invalid return 'default_value' */
int ram_ts_get_int(const char *key, int default_value);

#endif  /* _RAM_TS_H */
