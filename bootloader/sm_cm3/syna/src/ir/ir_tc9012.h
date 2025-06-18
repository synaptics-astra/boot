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
#ifndef __TC9012_H__
#define __TC9012_H__

#define DATA_0_LEN		(  1125 * SM_SYSTEM_HZ )
#define DATA_0_MIN_LEN	(   900 * SM_SYSTEM_HZ )
#define DATA_0_MAX_LEN	(  1325 * SM_SYSTEM_HZ )
#define DATA_1_LEN		(  2250 * SM_SYSTEM_HZ )
#define DATA_1_MIN_LEN	(  1825 * SM_SYSTEM_HZ )
#define DATA_1_MAX_LEN	(  2650 * SM_SYSTEM_HZ )
#define PREAMB_LEN		( 9000 * SM_SYSTEM_HZ )
#define PREAMB_MIN_LEN	( 8000 * SM_SYSTEM_HZ )
#define PREAMB_MAX_LEN	( 10000 * SM_SYSTEM_HZ )
#define PREAMB_HOLD		( 11314 * SM_SYSTEM_HZ )
#define PREAMB_HOLD_MAX	( 12475 * SM_SYSTEM_HZ )
#define PREAMB_LOW		(  4500 * SM_SYSTEM_HZ )
#define PREAMB_MIN_LOW	(  4000 * SM_SYSTEM_HZ )
#define PREAMB_MAX_LOW	(  5500 * SM_SYSTEM_HZ )
#define FRAME_MAX_LEN	( 110000 * SM_SYSTEM_HZ )
#define FRAME_CYCLE		(108000 * SM_SYSTEM_HZ )
#define FRAME_CYCLE_MAX	(157900 * SM_SYSTEM_HZ )

#define BURST_LEN		(560*SM_SYSTEM_HZ)
#define BURST_MIN_LEN	(500*SM_SYSTEM_HZ)
#define BURST_MAX_LEN	(700*SM_SYSTEM_HZ)


#define FRAME_CYCLE_NORMAL_MAX	(115000 * SM_SYSTEM_HZ )
#define FRAME_CYCLE_FIRSTHOLD_MAX (60000*SM_SYSTEM_HZ)

#define HoldKeySkipTime 2
UINT32 read_tc9012(UINT32 *, UINT32*, UINT32*, BOOL*);

#endif
