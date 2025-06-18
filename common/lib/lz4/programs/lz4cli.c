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

///////////////////////////////////////////////////////////////////////////////
//! \file       lz4dec.c
//! \brief      decode a lz4 compressed input stream.
///////////////////////////////////////////////////////////////////////////////


/****************************
 *  Includes
 *****************************/
//#include <stdint.h>
#include "debug.h"
#include "lz4frame.h"

#define LZ4IO_MAGICNUMBER   (0x184D2204)

/** @brief decode a compressed lz4input stream, only
 *         started with LZ4IO_MAGICNUMBER supported.
 *
 *
 *  @param in_buf the input stream buffer, contains the full
 *         input data, started with LZ4IO_MAGICNUMBER.
 *  @param input_size the whole input tream size, including the magic,
 *         in bytes.
 *  @param out_buf the decode result.
 *  @param output_size the decode output buffer size when used as input,
 *         and final decoded stream length in bytes as output.
 *  @return 0 if success, negative if failed.
 */
int lz4_decompress(const unsigned char *in_buf, const unsigned int input_size, unsigned char *out_buf, unsigned int *output_size)
{
        int ret = 0;

        if (in_buf == NULL || out_buf == NULL || output_size == NULL || *output_size < input_size) {
                NOTICE("%s, %d.\n", __func__, __LINE__);
                return -1;
        }

        uint32_t lz4_dec_magic = *(uint32_t *)in_buf;
        if (lz4_dec_magic != LZ4IO_MAGICNUMBER) {
                NOTICE("%s, %d.\n", __func__, __LINE__);
                return -1;
        }

        LZ4F_decompressionContext_t ctx;
        LZ4F_errorCode_t err;

        /* init */
        err = LZ4F_createDecompressionContext(&ctx, LZ4F_VERSION);
        if (LZ4F_isError(err)) {
                NOTICE("%s, %d, Can't create context : %s", __func__, __LINE__, LZ4F_getErrorName(err));
                return -1;
        }

        unsigned int pos = 0;
        unsigned int decoded = 0;
        size_t dec_out_size = *output_size;
        size_t remaining = input_size - pos;

        while (remaining > 0) {
                dec_out_size = *output_size - decoded;
                if (dec_out_size == 0) {
                        NOTICE("%s, Out of space in output buffer. Buffer size=%d Remaining (compressed) data=%d\n", __func__, *output_size, remaining);
                        return -1;
                }
                err = LZ4F_decompress(ctx, out_buf + decoded, &dec_out_size, (char*)in_buf + pos, &remaining, NULL);
                if (LZ4F_isError(err)) {
                        NOTICE("%s, %d, Decompression error : %s", __func__, __LINE__, LZ4F_getErrorName(err));
                        return -1;
                }
                pos += remaining;
                remaining = input_size - pos;
                decoded += dec_out_size;
                if (pos > input_size) {
                        NOTICE("%s, %d.\n", __func__, __LINE__);
                        return -1;
                }
                if (decoded > *output_size) {
                        NOTICE("%s, %d.\n", __func__, __LINE__);
                        return -1;
                }
        }

        *output_size = decoded;

        err = LZ4F_freeDecompressionContext(ctx);
        if (LZ4F_isError(err)) {
                NOTICE("%s, %d, Error : can't free LZ4F context resource : %s", __func__, __LINE__,  LZ4F_getErrorName(err));
                return -1;
        }

        return ret;
}

#ifdef _PC_TEST
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
        FILE *in_file, *out_file;

        in_file = fopen(argv[1], "rb");
        if (in_file == NULL) {
                NOTICE("%s, %d.\n", __func__, __LINE__);
                exit(0);
        }
        out_file = fopen(argv[2], "wb");
        if (out_file == NULL) {
                NOTICE("%s, %d.\n", __func__, __LINE__);
                exit(0);
        }

        fseek(in_file, 0L, SEEK_END);
        long input_size = ftell(in_file);
        fseek(in_file, 0L, SEEK_SET);

        unsigned char *in_buf = (unsigned char *)malloc(input_size);
        if (in_buf == NULL){
                NOTICE("%s, %d.\n", __func__, __LINE__);
                exit(0);
        }

        fread(in_buf, 1, input_size, in_file);
        fclose(in_file);

        unsigned char *out_buf = (unsigned char *)malloc(input_size*3);
        if (out_buf == NULL){
                NOTICE("%s, %d.\n", __func__, __LINE__);
                exit(0);
        }

        //used as output buffer size when input, and actuall decoded size as output
        unsigned int output_size = input_size*3;
        int ret = lz4_decompress(in_buf, input_size, out_buf, &output_size);
        if (ret != 0) {
                NOTICE("%s, %d.\n", __func__, __LINE__);
                exit(0);
        }

        fwrite(out_buf, 1, output_size, out_file);
        free(in_buf);
        free(out_buf);
        fclose(out_file);

        return 0;
}
#endif //_PC_TEST
