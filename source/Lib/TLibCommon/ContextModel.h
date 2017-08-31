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


/** \file     ContextModel.h
    \brief    context model class (header)
*/

#ifndef __CONTEXT_MODEL__
#define __CONTEXT_MODEL__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CommonDef.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// context model class
class ContextModel
{
public:
#if wlq_AEC
  ContextModel  ()                          { m_MPS = 0; m_LG_PMPS = (QUARTER_AVS2 << LG_PMPS_SHIFTNO_AVS2) - 1; m_cycno = 0;}
#else
  ContextModel  ()                          { m_ucState = 0; }
#endif
  ~ContextModel ()                          {}
#if !wlq_AEC
  const UChar getState()                    { return (m_ucState & 0x7F);                  }
  const UChar getMps()                      { return (m_ucState >> 7);                    }
  Void  toggleMps()                         { m_ucState ^= 0x80;                          }
  Void  setState( UChar ucState )           { m_ucState = (ucState + (getMps() << 7));    }
  Void  setStateMps( UChar ucStateMps )     { m_ucState = (m_ucState & 0x80) ^ ucStateMps;}
#endif
  Void  init()                              { 
#if !wlq_AEC
	  m_ucState = 0;
#endif
#if wlq_AEC
	  m_LG_PMPS = (QUARTER_AVS2 << LG_PMPS_SHIFTNO_AVS2) - 1;   //10 bits precision
	  m_MPS   = 0;
	  m_cycno   = 0;
#endif
  }
#if wlq_AEC
  UChar getMPS()							{return m_MPS;}
  UInt  getLG_PMPS()						{return m_LG_PMPS;}
  UChar getCycno()							{return m_cycno;}

  Void setMPS(UChar MPS)							{ m_MPS = MPS;}
  Void setLG_PMPS(UInt LG_PMPS)				{m_LG_PMPS = LG_PMPS;}
  Void setCycno(UChar cycno)				{m_cycno = cycno;}
  Void update_ctx(register unsigned char is_LPS);
#endif
private:
#if !wlq_AEC
  UChar         m_ucState;    ///< internal state variable
#endif
#if wlq_AEC
  UChar m_MPS;   //1 bit
  UInt  m_LG_PMPS; //10 bits
  UChar m_cycno;  //2 bits
#endif
};

#endif

