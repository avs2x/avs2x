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

/** \file     TDecCu.h
    \brief    CU decoder class (header)
*/

#ifndef __TDECCU__
#define __TDECCU__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../TLibCommon/TComTrQuant.h"
#include "../TLibCommon/TComPrediction.h"
#include "TDecEntropy.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// CU decoder class
class TDecCu
{
private:
  UInt                m_uiMaxDepth;       ///< max. number of depth
  TComYuv*            m_pcYuvResi;       ///< array of residual buffer
  TComYuv*            m_pcYuvReco;       ///< array of prediction & reconstruction buffer
  TComDataCU**        m_ppcCU;            ///< CU data array
#if inter_direct_skip_bug2
  TComYuv   m_acYuvPred[2];
#endif
#if F_MHPSKIP_SYC
  TComYuv   m_acMHPSkipYuvPred[2];
#endif
  // access channel
  TComTrQuant*        m_pcTrQuant;
  TComPrediction*     m_pcPrediction;
  TDecEntropy*        m_pcEntropyDecoder;

public:
  TDecCu();
  virtual ~TDecCu();

  /// initialize access channels
  Void  init                    ( TDecEntropy* pcEntropyDecoder, TComTrQuant* pcTrQuant, TComPrediction* pcPrediction );
  
  /// create internal buffers
  Void  create                  ( UInt uiMaxDepth, UInt uilog2MaxCUSize );
  
  /// destroy internal buffers
  Void  destroy                 ();
  
  /// decode CU information
  Void  decodeCU                ( TComDataCU* pcCU, UInt& ruiIsLast );
  
  /// reconstruct CU information
  Void  decompressCU            ( TComDataCU* pcCU );

  /// set quantization parameter
  Void setQp( Int iQp, Int iCQp ) { m_pcTrQuant->setQp(iQp, iCQp); }
  
protected:
  
  Void xDecodeCU                ( TComDataCU* pcCU,                       UInt uiAbsPartIdx, UInt uiDepth );
  Void xDecompressCU            ( TComDataCU* pcCU, TComDataCU* pcCUCur,  UInt uiAbsPartIdx, UInt uiDepth );
  
  Void xReconInter              ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#if PSKIP
  Void scalingMV(TComMv &cMvPredL0, TComMv tempmv, Int curT, Int colT);
#endif

#if inter_direct_skip_bug2
  Void xReconInterSkipDirect(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
#endif

  Void xReconIntra              ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#if ZHANGYI_INTRA
  Void xIntraLumaBlk            ( UInt uiPU, TComDataCU* pcCU, UInt uiAbsPartIdx, TComYuv* pcRecoYuv, TComYuv* pcPredYuv, TComYuv* pcResiYuv, UInt uiPartOffset );
#else
  Void xIntraLumaBlk            ( TComDataCU* pcCU, UInt uiAbsPartIdx, TComYuv* pcRecoYuv, TComYuv* pcPredYuv, TComYuv* pcResiYuv, UInt uiPartOffset );
#endif
  Void xIntraChromaBlk          ( TComDataCU* pcCU, UInt uiAbsPartIdx, TComYuv* pcRecoYuv, TComYuv* pcPredYuv, TComYuv* pcResiYuv, UInt uiPartOffset );

  Void xDecodeInterTexture      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#if ZHANGYI_INTRA
  Void xCopyToPicIntra( TextType ttype, TComDataCU* pcCU, TComPic* pcPic, UInt uiZorderIdx, UInt uiDepth );
#endif
  Void xCopyToPic               ( TComDataCU* pcCU, TComPic* pcPic, UInt uiZorderIdx, UInt uiDepth );
};

#endif

