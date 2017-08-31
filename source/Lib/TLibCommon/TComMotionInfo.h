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

/** \file     TComMotionInfo.h
    \brief    motion information handling classes (header)
    \todo     TComMvField seems to be better to be inherited from TComMv
*/

#ifndef __TCOMMOTIONINFO__
#define __TCOMMOTIONINFO__

#include <memory.h>
#include "CommonDef.h"
#include "TComMv.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================
#if RPS
class TComMvField
{
private:
  TComMv    m_acMv;
  Int       m_iRefIdx;

public:
  TComMvField() : m_iRefIdx(NOT_VALID) {}

  Void setMvField(TComMv const & cMv, Int iRefIdx)
  {
    m_acMv = cMv;
    m_iRefIdx = iRefIdx;
  }

  Void setRefIdx(Int refIdx) { m_iRefIdx = refIdx; }

  TComMv const & getMv() const { return  m_acMv; }
  TComMv       & getMv()       { return  m_acMv; }

  Int &getRefIdx()       { return  m_iRefIdx; }
  Int getHor()          { return  m_acMv.getHor(); }
  Int getVer()          { return  m_acMv.getVer(); }

#if PSKIP
  Void  setMv(Int iHor, Int iVer)     { m_acMv.setHor(iHor);  m_acMv.setVer(iVer); }
  Void  setHor(Int i)                 { m_acMv.setHor(i); }
  Void  setVer(Int i)                 { m_acMv.setVer(i); }
#endif


};
#endif

/// class for motion information in one CU
class TComCUMvField
{
private:
  TComMv*   m_pcMv;
  TComMv*   m_pcMvd;
#if RPS
  Int*      m_piRefIdx;         //MV的参考帧管理
  //分块地址参考信息
  template <typename T>
  Void setAll(T *p, T const & val, PartSize ePartMode, Int iPartAddr, Int iPartIdx, UInt uiDepth);
#endif
  UInt      m_uiNumPartition;

public:
  TComCUMvField()
  {
    m_pcMv     = NULL;
    m_pcMvd    = NULL;
#if RPS
    m_piRefIdx = NULL;
#endif
  }
  ~TComCUMvField()
  {
    m_pcMv     = NULL;
    m_pcMvd    = NULL;
#if RPS
    m_piRefIdx = NULL;
#endif
  }
  
  // ------------------------------------------------------------------------------------------------------------------
  // create / destroy
  // ------------------------------------------------------------------------------------------------------------------
  
  Void    create        ( UInt uiNumPartition );
  Void    destroy       ();
  
  // ------------------------------------------------------------------------------------------------------------------
  // clear / copy
  // ------------------------------------------------------------------------------------------------------------------
  
  Void    clearMv       ( Int iPartAddr, UInt uiDepth );
  Void    clearMvd      ( Int iPartAddr, UInt uiDepth );
  Void    clearMvField  ();
  
  Void    copyFrom          ( TComCUMvField* pcCUMvFieldSrc, Int iNumPartSrc, Int iPartAddrDst );
  Void    copyTo            ( TComCUMvField* pcCUMvFieldDst, Int iPartAddrDst );
  Void    copyTo            ( TComCUMvField* pcCUMvFieldDst, Int iPartAddrDst, UInt uiOffset, UInt uiNumPart );
  Void    copyMvTo          ( TComCUMvField* pcCUMvFieldDst, Int iPartAddrDst );
  
  // ------------------------------------------------------------------------------------------------------------------
  // get
  // ------------------------------------------------------------------------------------------------------------------
  
  TComMv& getMv             ( Int iIdx )               { return  m_pcMv    [iIdx]; }
  TComMv* getMv             ()                         { return  m_pcMv;           }
  TComMv& getMvd            ( Int iIdx )               { return  m_pcMvd   [iIdx]; }
  TComMv* getMvd            ()                         { return  m_pcMvd;          }
#if RPS
  Int& getRefIdx(Int iIdx)         const { return  m_piRefIdx[iIdx]; } //返回一个CU中，最后part后PU的参考帧号 
  Int* getRefIdx()                         { return  m_piRefIdx; }
#endif
  // ------------------------------------------------------------------------------------------------------------------
  // set
  // ------------------------------------------------------------------------------------------------------------------
  
  Void    setMv             ( TComMv  cMv,     Int iIdx ) { m_pcMv    [iIdx] = cMv;     }
  Void    setMvd            ( TComMv  cMvd,    Int iIdx ) { m_pcMvd   [iIdx] = cMvd;    }

  Void    setMvPtr          ( TComMv*  cMvPtr     ) { m_pcMv    = cMvPtr;         }
  Void    setMvdPtr         ( TComMv*  cMvdPtr    ) { m_pcMvd  = cMvdPtr;         }
  Void    setNumPartition   ( Int      iNumPart   ) { m_uiNumPartition=iNumPart;  }
  
  Void    setAllMv          ( TComMv& rcMv,    PartSize ePartMode, Int iPartAddr, Int iPartIdx, UInt uiDepth );
  Void    setAllMvd         ( TComMv& rcMvd,   PartSize ePartMode, Int iPartAddr, Int iPartIdx, UInt uiDepth );

#if RPS
  Void    setRefPtr(Int * iRefPtr) { m_piRefIdx = iRefPtr; }
  Void    setAllRefIdx(Int &iRefIdx, PartSize ePartMode, Int iPartAddr, Int iPartIdx, UInt uiDepth);
  //Void    setAllRefIdx      (Int iRefIdx,            PartSize ePartMode, Int iPartAddr, Int iPartIdx, UInt uiDepth );  //初始化CU的分块的参考信息
  Void    setAllMvField(TComMvField  & mvField, PartSize ePartMode, Int iPartAddr, Int iPartIdx, UInt uiDepth);
#else
  Void    setAllMvField(TComMv& rcMv, PartSize ePartMode, Int iPartAddr, Int iPartIdx, UInt uiDepth);

#endif
#if DPB
  Void compress(const TComCUMvField &source, Int NumPartInWidth);
#endif
};

#endif // __TCOMMOTIONINFO__

