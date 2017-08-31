/* ====================================================================================================================

  The copyright in this software is being made available under the License included below.
  This software may be subject to other third party and contributor rights, including patent rights, and no such
  rights are granted under this license.

  Copyright (c) 2012, SAMSUNG ELECTRONICS CO., LTD. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted only for
  the purpose of developing standards within Audio and Video Coding Standard Workgroup of China (AVS) and for testing and
  promoting such standards. The following conditions are required to be met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
      the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
      the following disclaimer in the documentation and/or other materials provided with the distribution.
    * The name of SAMSUNG ELECTRONICS CO., LTD. may not be used to endorse or promote products derived from 
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 * ====================================================================================================================
*/

/** \file     TComDataCU.h
    \brief    CU data structure (header)
    \todo     not all entities are documented
*/

#include "TComBitCounter.h"
#include <memory.h>
#if wlq_AEC
Void TComBitCounter :: flushBuffer(){}
Void TComBitCounter :: put_one_bit_plus_outstanding(UInt uiBits,UInt*Ebits_to_follow){
	write(uiBits,1);
	while ((*Ebits_to_follow) > 0) 
	{ 
		(*Ebits_to_follow)--; 
		write(!(uiBits),1);
	} 
  }
#endif
#if YQH_AEC_RDO_BUG
Void TComBitCounter::write(UInt uiBits, UInt uiNumberOfBits) {
	m_uiBitCounter += uiNumberOfBits;
	if (--m_uiCodeBits == 0)
	{
		m_uiCodeBits = 8;
		m_uiByteCounter++;
	}

}
#endif