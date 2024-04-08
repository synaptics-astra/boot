/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 1994-2009  Red Hat, Inc.
 * All rights reserved.
 *
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
#include "com_type.h"
#include "string.h"

/**
 * strcpy - Copy a %NUL terminated string
 * param strDest: Where to copy the string to
 * param strSrc: Where to copy the string from
 */
char *strcpy(char *strDest,const char *strSrc)
{
	char *tmp = strDest;

	if((strDest != NULL) && (strSrc != NULL))
	{

		while ((*strDest++ = *strSrc++) != '\0')
	  		;
		return tmp;
	}

	return NULL;
}

/**
 * strncpy - Copy a length-limited, %NUL-terminated string
 * param strDest: Where to copy the string to
 * param strSrc: Where to copy the string from
 * param size: The maximum number of bytes to copy
 *
 * Note that unlike userspace strncpy, this does not %NUL-pad the buffer.
 * However, the result is not %NUL-terminated if the source exceeds
 * size bytes.
 */
char *strncpy(char *strDest,const char *strSrc,size_t size)
{
	char *dscan;
	const char *sscan;

	if((strDest != NULL) && (strSrc != NULL))
	{
		dscan = strDest;
		sscan = strSrc;
		while (size > 0)
		  {
			--size;
			if ((*dscan++ = *sscan++) == '\0')
		  break;
		  }
		while (size-- > 0)
		  *dscan++ = '\0';

		return strDest;
	}

	return NULL;
}

/**
 * strcat - Append one %NUL-terminated string to another
 * param strDest: The string to be appended to
 * param strSrc: The string to append to it
 */
char *strcat(char *strDest,const char *strSrc)
{
	char *tmp = strDest;

	if((strDest != NULL) && (strSrc != NULL))
	{
	    while(*strDest)
	        strDest++;
	    while((*strDest++ = *strSrc++) != '\0')
	        ;

		return tmp;
	}

	return NULL;
}

/**
 * strncat - Append a length-limited, %NUL-terminated string to another
 * param strDest: The string to be appended to
 * param strSrc: The string to append to it
 * param size: The maximum numbers of bytes to copy
 *
 * Note that in contrast to strncpy, strncat ensures the result is
 * terminated.
 */
char *strncat(char *strDest,const char *strSrc,size_t size)
{
    char *tmp = strDest;

	if((strDest != NULL) && (strSrc != NULL))
	{
	    while(*strDest)
	        strDest++;
	    while(size--)
	        if(!(*strDest++ = *strSrc++))
				break;
	    *strDest = '\0';

		return tmp;
	}

	return NULL;
}

/**
 * strcmp - Compare two strings
 * param str1: One string
 * param str2: Another string
 */
int strcmp(const char * str1,const char * str2)
{
	char res = 0;
	while (1)
	{
		if ((res = *str1 - *str2++) != 0 || !*str1++)
			break;
	}
	return res;
}

/**
 * strncmp - Compare two length-limited strings
 * param str1: One string
 * param str2: Another string
 * param size: The maximum number of bytes to compare
 */
int strncmp(const char *str1,const char *str2,size_t size)
{
	char res = 0;
	while (size) {
		if ((res = *str1 - *str2++) != 0 || !*str1++)
			break;
		size--;
	}
	return res;
}

/**
 * strchr - Find the first occurrence of a character in a string
 * param str: The string to be searched
 * param c: The character to search for
 */
char * strchr(const char * str, int c)
{
	if(str != NULL)
	{
		for(; *str != (char) c; ++str)
			if (*str == '\0')
				return NULL;
		return (char *) str;
	}
	return NULL;
}

/**
 * strlen - Find the length of a string
 * param str: The string to be sized
 */
size_t strlen(const char * str)
{
	size_t len = 0;
	if(str != NULL)
		while(*str++ != '\0')
			++len;
	return len;
}

/**
 * strnlen - Find the length of a length-limited string
 * param str: The string to be sized
 * param size: The maximum number of bytes to search
 */
size_t strnlen(const char * str, size_t maxsize)
{
	size_t maxlen = 0;
	if(str != NULL)
		while(maxsize-- && (*str++ != '\0'))
			++maxlen;
	return maxlen;
}

/**
 * memset - Fill a region of memory with the given value
 * param str: Pointer to the start of the area.
 * param c: The byte to fill the area with
 * param size: The size of the area.
 *
 * Do not use memset() to access IO space, use memset_io() instead.
 */
void * memset(void *str,int c,size_t size)
{
	if(str != NULL)
	{
		char *address = (char *)str;
		while (size--)
			*address++ = c;
	}
	return str;
}

/**
 * memchr - Find a character in an area of memory.
 * param str: The memory area
 * param c: The byte to search for
 * param n: The size of the area.
 *
 * returns the address of the first occurrence of c, or %NULL
 * if c is not found
 */
void *memchr(const void *str, int c, size_t n)
{
	if(str != NULL)
	{
		const unsigned char *p = str;
		while (n-- != 0) {
			if ((unsigned char)c == *p++) {
				return (void *)(p-1);
			}
		}
	}
	return NULL;
}

/**
 * memcpy - Copy one area of memory to another
 * param strDest: Where to copy to
 * param strSrc: Where to copy from
 * param size: The size of the area.
 *
 * You should not use this function to access IO space, use memcpy_toio()
 * or memcpy_fromio() instead.
 */
void * memcpy(void *strDest, const void *strSrc, size_t count)
{
	unsigned long *dst_ul = (unsigned long *)strDest;
	unsigned long *src_ul = (unsigned long *)strSrc;
	unsigned char *dst_u8, *src_u8;

	if (!strDest || !strSrc)
		return NULL;

	if (strSrc == strDest)
		return strDest;

	if ( (((unsigned long)strDest | (unsigned long)strSrc) & (sizeof(*dst_ul) - 1)) == 0) {
		while (count >= sizeof(*dst_ul)) {
			*dst_ul++ = *src_ul++;
			count -= sizeof(*dst_ul);
		}
	}

	dst_u8 = (unsigned char *)dst_ul;
	src_u8 = (unsigned char *)src_ul;
	while (count--)
		*dst_u8++ = *src_u8++;

	return strDest;
}

/**
 * memmove - Copy one area of memory to another
 * param strDest: Where to copy to
 * param strSrc: Where to copy from
 * param size: The size of the area.
 *
 * Unlike memcpy(), memmove() copes with overlapping areas.
 */
void * memmove(void * strDest,const void *strSrc,size_t size)
{
	if((strDest != NULL) && (strSrc != NULL))
	{
		char *pstrDest,*pstrSrc;
		if (strDest <= strSrc) {
			pstrDest = (char *)strDest;
			pstrSrc =(char *)strSrc;
			while(size--)
				*pstrDest++ = *pstrSrc++;
		}
		else
		{
			pstrDest = (char *)strDest + size;
			pstrSrc =(char *)strSrc + size;
			while(size--)
				*--pstrDest = *--pstrSrc;
		}
	}
	return strDest;
}

/**
 * memcmp - Compare two areas of memory
 * param str1: One area of memory
 * param str2: Another area of memory
 * param size: The size of the area.
 */
int memcmp(const void * str1,const void * str2,size_t size)
{
	int res = 1;
	if((str1 != NULL) && (str2 != NULL))
	{
		const unsigned char *pstr1, *pstr2;
		for( pstr1 = str1, pstr2 = str2; 0 < size; ++pstr1, ++pstr2, size--)
			if ((res = *pstr1 - *pstr2) != 0)
				break;
	}
	return res;
}

/**
 * strstr - Find the first substring in a %NUL terminated string
 * param str1: The string to be searched
 * param str2: The string to search for
 */
char * strstr(const char * str1,const char * str2)
{
	if(str1 != NULL)
	{
		int len1,len2;
		len2 = strlen(str2);
		if(!len2)
			return (char*)str1;
		len1 = strlen(str1);
		while (len1 >= len2)
		{
			len1--;
			if (!memcmp(str1,str2,len2))
				return (char *)str1;
			str1++;
		}
	}
	return NULL;
}

#define TOLOWER(x) ((x) | 0x20)
#define in_area(x, a, b) ((x)>=(a) && (x)<=(b))
#define isdigit(x) (in_area(x, '0', '9'))
#define isxdigit(x) (isdigit(x) || in_area(x, 'a', 'f') || in_area(x, 'A', 'F'))
unsigned long int strtoul(const char *nptr, char **endptr, int base)
{
	unsigned long int result = 0;

	if(!base) {
		if(nptr[0] == '0') {
			if(TOLOWER(nptr[1]) == 'x' && isxdigit(nptr[2]))
				base = 16;
			else
				base = 8;
		} else {
			base = 10;
		}
	}

	if((base == 16) && (nptr[0] == '0') && (TOLOWER(nptr[1]) == 'x'))
		nptr += 2;

	while(isxdigit(*nptr)) {
		unsigned int value;

		value = isdigit(*nptr) ? (*nptr - '0') : (TOLOWER(*nptr) - 'a' + 10);
		if(value >= base)
			break;
		result = result * base + value;
		nptr++;
	};

	if(endptr != NULL)
		*endptr = (char *)nptr;

	return result;
}

int ctoi(char c)
{
	if((c>='0')&&(c<='9'))
		return c-'0';
	else if((c>='a')&&(c<='f'))
		return c-'a' + 10;
	else if((c>='A')&&(c<='F'))
		return c-'A' + 10;
	else
		return -1;
}

char itoc(char i)
{
	int j = i&0xf;
	if(j<10)
		return j+'0';
	else
		return j-10+'A';
}
