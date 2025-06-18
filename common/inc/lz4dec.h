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
//! \file       lz4dec.h
//! \brief      decode a lz4 compressed input stream.
///////////////////////////////////////////////////////////////////////////////

#ifndef _LZ4DEC_H_
#define _LZ4DEC_H_

#define LZ4IO_MAGICNUMBER   (0x184D2204)

#if defined (__cplusplus)
extern "C" {
#endif //__cplusplus

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
int lz4_decompress(const unsigned char *in_buf, const unsigned int input_size, unsigned char *out_buf, unsigned int *output_size);

#if defined (__cplusplus)
}
#endif //__cplusplus

#endif //_LZ4DEC_H_
