/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  
 *
 * Copyright (c) 2010-2012, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     ContextModel.cpp
    \brief    context model class
*/

#include "ContextModel.h"

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/**
 - initialize context model with respect to QP and initial probability
 .
 \param  iQp         input QP value
 \param  asCtxInit   initial probability table
 */
#if wlq_AEC
Void ContextModel::update_ctx(register unsigned char is_LPS)
{
	register unsigned char cwr, cycno = m_cycno;
	register unsigned int lg_pmps = m_LG_PMPS;

	cwr = (cycno <= 1) ? 3 : (cycno == 2) ? 4 : 5;     //FAST ADAPTION PARAMETER
	//update other parameters
	if (is_LPS) {
		cycno = (cycno <= 2) ? (cycno + 1) : 3;
	} else if (cycno == 0) {
		cycno = 1;
	}

	m_cycno = cycno;

	//update probability estimation
	if (is_LPS) {
		switch (cwr) {
		case 3:
			lg_pmps = lg_pmps + 197;
			break;
		case 4:
			lg_pmps = lg_pmps + 95;
			break;
		default:
			lg_pmps = lg_pmps + 46;
		}

		if (lg_pmps >= (256 << LG_PMPS_SHIFTNO_AVS2)) {
			lg_pmps = (512 << LG_PMPS_SHIFTNO_AVS2) - 1 - lg_pmps;
			m_MPS = !(m_MPS);
		}
	} else {
		lg_pmps = lg_pmps - (unsigned int)(lg_pmps >> cwr) - (unsigned int)(lg_pmps >> (cwr + 2));
	}

	m_LG_PMPS = lg_pmps;
}
#endif