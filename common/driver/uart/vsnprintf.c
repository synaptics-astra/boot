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

#include <com_type.h>
#include <stdarg.h>

/*
 * vsnprintf
 *    format output into buffer.
 *    ONLY support %d %s %x.
*/
INT32 vsnprintf(INT8 *buf, UINT32 size, const INT8 *fmt, va_list ap)
{
    INT32 base, idx, width, sign, lead, up, is_64bit;
    INT32 val, flag;
	INT64 val64;
    UINT32 uval,i;
	UINT64 uval64;
    INT8 *s, *digit;
    INT8 *lowstr = "0123456789abcdef";
    INT8 *upstr = "0123456789ABCDEF";
    INT8 tmp[40];

    size--;/* reserve space for string end\0  */
    for (i=0; i<size && *fmt; fmt++) {
        if (*fmt != '%') {
            buf[i++] = *fmt;
            continue;
        }
        fmt++;
        if (*fmt == '0') {
            lead = 1;
            fmt++;
        } else {
            lead = 0;
        }
        /* strip width */
        width = 0;
        while (*fmt >= '0' && *fmt <= '9') {
            width *= 10;
            width += (*fmt - '0');
            fmt++;
        }

		is_64bit = 0;

        if (*fmt == 'l') {
            fmt++;
			if(*fmt == 'l') {
				//support 64bit print
				is_64bit = 1;
				fmt++;
			}
        }
        sign = up = 0;
        /* parse option */
        switch(*fmt) {
            case '%':
                buf[i++] = '%';
                continue;
            case 'c':
				is_64bit = 0;
                val = va_arg(ap, INT32);
                buf[i++] = val & 0xff;
                continue;
            case 's':
				is_64bit = 0;
                s = va_arg(ap, INT8 *);
                if (!s) {
                    s = "NULL";
                }
                while(i<size && *s) {
                    buf[i++] = *s++;
                };
                continue;
            case 'd':
                sign = 1;
            case 'u':
                base = 10;
		is_64bit = 0;//only support %llx
                break;
            case 'X':
                up = 1;
            case 'x':
            case 'p':
                base = 16;
                break;
            default:
                buf[i++] = '~';
                continue;
        }
        /* format number according to base */
        if (up) {
            digit = upstr;
        } else {
            digit = lowstr;
        }
        flag = 0;
        /* get number */
		if(is_64bit) {
			val64 = va_arg(ap, INT64);
	        if (sign && val64 < 0) {
	            val64 = -val64;
	            flag = 1;
	        }

	        /* format number into temp buffer */
	        uval64 = (UINT64)val64;
	        idx = 0;
	        do {
	            tmp[idx++] = digit[uval64 & (base - 1)];
	            uval64 >>= 4;
	        } while(uval64);
		} else {
	        val = va_arg(ap, INT32);
	        if (sign && val < 0) {
	            val = -val;
	            flag = 1;
	        }

	        /* format number into temp buffer */
	        uval = (UINT32)val;
	        idx = 0;
	        do {
	            tmp[idx++] = digit[uval%base];
	            uval /= base;
	        } while(uval);
		}
        /* pad lead char */
        if (width && flag) {
            width--;
        }
        width -= idx;
        if (lead) { /* add leading 0 */
            while(width-- > 0) {
                tmp[idx++] = '0';
            }
        }
        if (flag) { /* add - flag */
            tmp[idx++] = '-';
        }
        while(width-- > 0) { /* fill whitespace on remaining width */
            tmp[idx++] = ' ';
        }
        /* copy into buffer */
        while(i<size && idx>0) {
            buf[i++] = tmp[--idx];
        };
    }
    buf[i] = '\0'; /* mark string end */
    return i;
}

int snprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list ap;
    int i;

    va_start(ap, fmt);
    i=vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    return i;
}
