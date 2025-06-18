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
#include "ctype.h"

int isdigit(unsigned char c)
{
	if((c >= '0') && (c <= '9'))
		return 1;
	return 0;

}

int islower(unsigned char c)
{
	if((c >= 'a') && (c <= 'z'))
		return 1;
	return 0;
}

int isupper(unsigned char c)
{
	if((c >= 'A') && (c <= 'Z'))
		return 1;
	return 0;
}

int isalpha(unsigned char c)
{
	if(isupper(c) || islower(c))
		return 1;
	return 0;
}

int isalnum(unsigned char c)
{
	if(isalpha(c) || isdigit(c))
		return 1;
	return 0;
}

int iscntrl(unsigned char c)
{
	//if((c >= 0x0) && (c <= 0x1f))
	if(c <= 0x1f)
		return 1;
	return 0;
}

int isgraph(unsigned char c)
{
	if((c >= 0x21) && (c <= 0x7e))
		return 1;
	return 0;
}

int isprint(unsigned char c)
{
	if((c >= 0x20) && (c <= 0x7e))
		return 1;
	return 0;
}

int ispunct(unsigned char c)
{
	if(isgraph(c) && (!isalnum(c)))
	{
		return 1;
	}
	return 0;
}

int isspace(unsigned char c)
{
	if((c = ' ') || ((c >= 0x9) && (c <= 0xd)))
	{
		return 1;
	}
	return 0;
}

unsigned char toupper(unsigned char c)
{
	if(islower(c))
		c -= 'a' - 'A';
	return c;
}

unsigned char tolower(unsigned char c)
{
	if (isupper(c))
		c -= 'A'-'a';
	return c;
}

int isxdigit(char c)
{
	if(isdigit(c))
		return 1;
	if((c >= 'a') && (c <= 'f'))
		return 1;
	if((c >= 'A') && (c <= 'F'))
		return 1;
	return 0;
}

int isascii(unsigned char c)
{
	if(c <= 0x7f)
		return 1;
	return 0;
}

unsigned char toascii(unsigned char c)
{
	return (c&0x7f);
}

unsigned long str2ul(const char *startp,char **endp,unsigned int base)
{
	unsigned long ret = 0,value;

	if(*startp == '0'){
		startp++;
		if((*startp == 'x') && isxdigit(startp[1])){
			base = 16;
			startp++;
		}
		if(!base){
			base = 8;
		}
	}
	if(!base){
		base = 10;
	}
	while (isxdigit(*startp) && (value = isdigit(*startp) ? *startp-'0' : (islower(*startp)
					? toupper(*startp) : *startp)-'A'+10) < base) {
		ret = ret * base + value;
		startp++;
	}
	if (endp)
		*endp = (char *)startp;
	return ret;
}

