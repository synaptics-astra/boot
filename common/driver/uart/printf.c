/*-
 * Copyright (c) 1990, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 * All rights reserved.
 * Portions of this software were developed by David Chisnall
 * under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <com_type.h>
//#include <stdio.h>
#include <stdarg.h>
//#include <xlocale.h>

extern void APB_UART_putc(UINT32 channel, CHAR ch);
extern INT32 vsnprintf(INT8 *buf, UINT32 size, const INT8 *fmt, va_list ap);

static int putchar(int c){
	APB_UART_putc(0, c) ;
	if (c == '\n'){
		APB_UART_putc(0, '\r');
	}
	return 0;
}

int puts(const char *str)
{
	while (*str) {
		putchar(*str);
		str++;
	}
	return 0;
}

int printf(char const * fmt, ...)
{
	//int ret;
	INT8 buf[1024];
	INT32 i;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf (buf, sizeof(buf), fmt, ap);
	va_end(ap);

	i = 0;
	while(buf[i]) {
		putchar(buf[i++]);
	}
	return 0;
}

int lgpl_printf(const char *fmt, ...)
{
#ifndef CONFIG_PRODUCTION_BUILD
    INT8 buf[256];
    INT32 i;
    va_list args;

    va_start (args, fmt);
    vsnprintf (buf, sizeof(buf), fmt, args);
    va_end (args);

    i = 0;
    while(buf[i]) {
        putchar(buf[i++]);
    }
#endif
    return 0;
}


int sprintf(char * str, char const * fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsnprintf (str, -1, fmt, ap);
	va_end(ap);
	return 0;
}

int lgpl_sprintf(char *out, const char *fmt, ...)
{
    va_list args;

    va_start (args, fmt);
    vsnprintf (out, -1, fmt, args);
    va_end (args);
    return 0;
}
