/******************************************************************************
*                 2018 Synaptics Incorporated. All Rights Reserved            *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF Synaptics.                   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
* OF Synaptics OR ANY THIRD PARTY. Synaptics RESERVES THE RIGHT AT ITS SOLE   *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO Synaptics.  *
* THIS CODE IS PROVIDED "AS IS". Synaptics MAKES NO WARRANTIES, EXPRESSED,    *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
*                                                                             *
*                   WARNING FOR USING THIS DIAG CODE                          *
*   1. Please take your own responsibility if you refer DIAG source code for  *
*      any type software development.                                         *
*   2. DIAG code is mainly for internal validation purpose, and not intend to *
*      be used in production SW.                                              *
*   3. DIAG code is subject to change without notice.                         *
*******************************************************************************/
#include "sm_type.h"
#include "sm_uart.h"
#include "sm_printf.h"

#define BUF_LEN		16
#define POS_RIGHT	1
#define POS_ZERO	2

#define MAX_BYTE_BUF 256
#ifdef CONFIG_PRODUCTION_BUILD
unsigned int DBG_LEVEL = PRN_NONE;
#else
unsigned int DBG_LEVEL = PRN_INFO;
#endif
static char dbg_buf[MAX_BYTE_BUF];

int SM_print_directly_to_UART = 0;


inline static void printchar(char **ppstrout, int outc)
{
	if (ppstrout)
	{
		**ppstrout = outc;
		++(*ppstrout);
	}
	else
		put_char(outc);
}

inline static int prints(char **ppstrout, const char *strin, int len, int pos)
{
	register int pc = 0, padchar = ' ';

	if (len > 0)
	{
		register int mylen = 0;
		register const char *ptr;
		for (ptr = strin; *ptr; ++ptr)
			++mylen;
		if (mylen >= len)
			len = 0;
		else
			len -= mylen;
		if (pos & POS_ZERO)
			padchar = '0';
	}
	if (!(pos & POS_RIGHT))
	{
		for ( ; len > 0; --len)
		{
			printchar (ppstrout, padchar);
			++pc;
		}
	}
	for ( ; *strin ; ++strin)
	{
		printchar (ppstrout, *strin);
		++pc;
	}
	for ( ; len > 0; --len)
	{
		printchar (ppstrout, padchar);
		++pc;
	}

	return pc;
}


inline static int printdigit(char **ppstrout, int pstr, int n, int p, int len, int pos, int ref)
{
	char print_buf[BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = pstr;

	if (pstr == 0)
	{
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (ppstrout, print_buf, len, pos);
	}

	if (p && n == 10 && pstr < 0)
	{
		neg = 1;
		u = -pstr;
	}

	s = print_buf + BUF_LEN-1;
	*s = '\0';

	while (u)
	{
		t = u % n;
		if( t >= 10 )
			t += ref - '0' - 10;
		*--s = t + '0';
		u /= n;
	}


	if (neg)
	{
		if( len && (pos & POS_ZERO) )
		{
			printchar (ppstrout, '-');
			++pc;
			--len;
		}
		else
		{
			*--s = '-';
		}
	}
	return pc + prints (ppstrout, s, len, pos);
}

int printstr(char **ppstrout, int *ppstrin, size_t size)
{
	register int len, pos;
	register int num = 0;
	register char *pstrin = (char *)(*ppstrin++);
	char scr[2];

	for (; *pstrin != 0; ++pstrin)
	{
		if (*pstrin == '%')
		{
			++pstrin;
			len = pos = 0;
			if (*pstrin == '\0') break;
			if (*pstrin == '%') goto out;
			if (*pstrin == '-')
			{
				++pstrin;
				pos = POS_RIGHT;
			}
			while (*pstrin == '0')
			{
				++pstrin;
				pos |= POS_ZERO;
			}
			for ( ; *pstrin >= '0' && *pstrin <= '9'; ++pstrin)
			{
				len *= 10;
				len += *pstrin - '0';
			}
			if( *pstrin == 's' )
			{
				register char *s = *((char **)ppstrin++);
				num += prints (ppstrout, s?s:"(null)", len, pos);
				continue;
			}
			if((*pstrin == 'd') || (*pstrin == 'D'))
			{
				num += printdigit (ppstrout, *ppstrin++, 10, 1, len, pos, 'a');
				continue;
			}
			if( (*pstrin == 'x') || (*pstrin == 'p'))
			{
				num += printdigit (ppstrout, *ppstrin++, 16, 0, len, pos, 'a');
				continue;
			}
			if( *pstrin == 'X' )
			{
				num += printdigit (ppstrout, *ppstrin++, 16, 0, len, pos, 'A');
				continue;
			}
			if( *pstrin == 'u' )
			{
				num += printdigit (ppstrout, *ppstrin++, 10, 0, len, pos, 'a');
				continue;
			}
			if( *pstrin == 'c' )
			{
				scr[0] = *ppstrin++;
				scr[1] = '\0';
				num += prints (ppstrout, scr, len, pos);
				continue;
			}
		}
		else
		{
out:
			printchar (ppstrout, *pstrin);
			++num;
		}
	}
	if (ppstrout) **ppstrout = '\0';
	return num;
}

/* print a string via uart */
int mv_sm_printf(const char *pstrin, ...)
{
	register int *ppstrin=(int*)(&pstrin);
	return printstr(0, ppstrin, MAX_BYTE_BUF);

	return 0;
}

int snprintf(char* pstrout, size_t size, const char *pstrin, ...)
{
	int i = 0;
	register int *ppstrin=(int*)(&pstrin);
	//FIXME: i should not greater than size
	i = printstr(&pstrout, ppstrin, size);

	return i;
}

/* print a string to a string variable */
int sprintf(char* pstrout,const char *pstrin, ...)
{
	register int *ppstrin=(int*)(&pstrin);
	printstr(&pstrout, ppstrin, MAX_BYTE_BUF);
	return 0;
}

void dbg_port_print( char* str)
{
    SIGN32 i;
	SIGN32 len = strlen(str);
	char ch;
	dbg_port_tx_ready();
	for (i = 0; i < len; i++)
	{
		ch = str[i];
		put_char(ch);
		if (ch == '\n')
		{
			ch = '\r';
			put_char(ch);
		}
		//extra delay
		i = i;
		len = len;
		ch = str[i];
	}
}


//logLevel is for compatability with SOC dbg_printf
void dbg_printf(int logLevel, const char* szFormat, ...)
{
	register int *ppstrin=(int*)(&szFormat);
    char * p = dbg_buf;

	if (logLevel <= DBG_LEVEL)
	{
		switch(logLevel)
		{
			case PRN_ERR:
				dbg_port_print(" ERR:");
				break;
			case PRN_RES:
				//dbg_port_print(" RES:");
				break;
			case PRN_INFO:
				//dbg_port_print("INFO:");
				break;
			case PRN_DBG:
				dbg_port_print(" DBG:");
				break;
			default:
				break;
		}
		printstr(&p,ppstrin, MAX_BYTE_BUF);
		dbg_port_print(dbg_buf);
	}
}

