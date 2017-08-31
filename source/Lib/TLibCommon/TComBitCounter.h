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

/** \file     TComBitcounter.h
    \brief    class for counting bits (header)
*/

#ifndef __COMBITCOUNTER__
#define __COMBITCOUNTER__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TComBitStream.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// class for counting bits
class TComBitCounter : public TComBitIf
{
protected:
  UInt  m_uiBitCounter;
  
public:
  TComBitCounter()            {}
#if YQH_AEC_RDO_BUG
  UInt  m_uiCodeBits;
  UInt  m_uiByteCounter;
#endif
  virtual ~TComBitCounter()   {}
#if YQH_AEC_RDO_BUG
  UInt      getCodeBits()                                    { return m_uiCodeBits; }
  UInt      getNumberOfWrittenBytes()                                    { return m_uiByteCounter; }
  Void      write(UInt uiBits, UInt uiNumberOfBits);
  Void      resetBits()                            { m_uiBitCounter = 72;   m_uiCodeBits = 9; m_uiByteCounter = 9; }
  UInt      getNumberOfWrittenBits()                                    { return m_uiBitCounter; }
#else   
  Void        write                 ( UInt uiBits, UInt uiNumberOfBits )  { m_uiBitCounter += uiNumberOfBits; }
  Void        resetBits             ()                                    { m_uiBitCounter = 0;               }
  UInt        getNumberOfWrittenBits()                                    { return m_uiBitCounter;            }
#endif
#if wlq_AEC
  Void  flushBuffer();
  Void  put_one_bit_plus_outstanding(UInt uiBits,UInt*Ebits_to_follow);
#endif
};

#endif

