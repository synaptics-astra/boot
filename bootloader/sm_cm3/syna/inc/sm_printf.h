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
#ifndef __SM_PRINTF_HEADER__
#define __SM_PRINTF_HEADER__

unsigned int strlen(const char *s);
int memcmp ( const void * dst, const void * src, unsigned int length);
int sprintf(char* pstrout,const char *pstrin, ...);
int snprintf(char* pstrout, size_t size, const char *pstrin, ...);
void * memcpy(void * dst, const void * src, unsigned int length );
void * memset(void * dst, int data, unsigned int length );


enum PRINT_LEVEL
{
    PRN_NONE=0,
    PRN_ERR,
    PRN_RES,
    PRN_INFO,
    PRN_DBG,
    PRN_BUF
};

void dbg_printf(int logLevel, const char* szFormat, ...);

#define PRT_DEBUG(format, args...) dbg_printf(PRN_DBG, format, ##args)
#define PRT_INFO(format, args...) dbg_printf(PRN_INFO, format, ##args)
#define PRT_RES(format, args...) dbg_printf(PRN_RES, format, ##args)
#define PRT_WARN(format, args...) dbg_printf(PRN_RES, format, ##args)
#define PRT_ERROR(format, args...) dbg_printf(PRN_ERR, format, ##args)

#endif /* __SM_PRINTF_HEADER__ */
