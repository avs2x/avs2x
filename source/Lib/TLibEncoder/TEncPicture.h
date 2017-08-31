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

/** \file     TEncPicture.h
    \brief    picture encoder class (header)
*/

#ifndef __TENCPICTURE__
#define __TENCPICTURE__

// Include files
#include "../TLibCommon/CommonDef.h"
#include "../TLibCommon/TComList.h"
#include "../TLibCommon/TComPic.h"
#include "../TLibCommon/TComPicYuv.h"
#include "TEncCu.h"

class TEncTop;
class TEncGOP;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// picture encoder class
class TEncPicture
{
private:
  // encoder configuration
  TEncCfg*                m_pcCfg;                              ///< encoder configuration class
  
  // pictures
  TComList<TComPic*>*     m_pcListPic;                          ///< list of pictures
  TComPicYuv*             m_apcPicYuvPred;                      ///< prediction picture buffer
  TComPicYuv*             m_apcPicYuvResi;                      ///< residual picture buffer
  
  // processing units
  TEncGOP*                m_pcGOPEncoder;                       ///< GOP encoder
  TEncCu*                 m_pcCuEncoder;                        ///< CU encoder
  
  // encoder search
  TEncSearch*             m_pcPredSearch;                       ///< encoder search class
  
  // coding tools
  TEncEntropy*            m_pcEntropyCoder;                     ///< entropy encoder
  TEncSbac*               m_pcSbacCoder;                        ///< SBAC encoder
  TComTrQuant*            m_pcTrQuant;                          ///< transform & quantization
  
  // RD optimization
  TComBitCounter*         m_pcBitCounter;                       ///< bit counter
  TComRdCost*             m_pcRdCost;                           ///< RD cost computation
  TEncSbac***             m_pppcRDSbacCoder;                    ///< storage for SBAC-based RD optimization
  TEncSbac*               m_pcRDGoOnSbacCoder;                  ///< go-on SBAC encoder
  UInt64                  m_uiPicTotalBits;                     ///< total bits for the picture
  UInt64                  m_uiPicDist;                          ///< total distortion for the picture
  Double                  m_dPicRdCost;                         ///< picture-level RD cost
  Double*                 m_pdRdPicLambda;                      ///< array of lambda candidates
  Int*                    m_piRdPicQp;                          ///< array of picture QP candidates (int-type)
  Int*                    m_piRdPicChromaQp;                          ///< array of picture QP candidates (int-type)
  Double*                 m_pdRdPicChromaLambda;                      ///< array of lambda candidates
  
public:
  TEncPicture();
  virtual ~TEncPicture();
  
  Void    create              ( Int iWidth, Int iHeight, UInt uiLog2MaxCUSize, UChar uhTotalDepth );
  Void    destroy             ();
  Void    init                ( TEncTop* pcEncTop );
  
  /// preparation of picture encoding (reference marking, QP and lambda)
#if RPS
  Void    initEncPicture(TComPic*  pcPic, Int iPOCLast, UInt uiPOCCurr, Int iNumPicRcvd,
    Int iTimeOffset, Int iGOPid, TComPicture*& rpcPicture, TComPicHeader*& pcPicHeader, TComSPS* pcSeqHeader);
#else
#if AVS3_PIC_HEADER_ZL
  Void    initEncPicture(TComPic*  pcPic, Int iPOCLast, UInt uiPOCCurr, Int iNumPicRcvd,
    Int iTimeOffset, Int iDepth, TComPicture*& rpcPicture, TComPicHeader*& pcPicHeader, TComSPS* pcSeqHeader);
#else
  Void    initEncPicture(TComPic*  pcPic, Int iPOCLast, UInt uiPOCCurr, Int iNumPicRcvd,
    Int iTimeOffset, Int iDepth, TComPicture*& rpcPicture);
#endif
#endif
  
  // compress and encode picture
  Void    compressPicture       ( TComPic*& rpcPic                                );      ///< analysis stage of picture
  Void    encodePicture         ( TComPic*& rpcPic, TComBitstream*& rpcBitstream  );      ///< entropy coding of picture
  
  // misc. functions
  Void    setSearchRange      ( TComPicture* pcPicture  );                                  ///< set ME range adaptively
  UInt64  getTotalBits        ()  { return m_uiPicTotalBits; }
#if wlq_WQ
  TComTrQuant*	getTrQuant()		{ return  m_pcTrQuant; }
  TEncCfg*		getCfg()			{ return  m_pcCfg; }
#endif
  TEncCu*        getCUEncoder() { return m_pcCuEncoder; }                        ///< CU encoder
#if wlq_FME
  TEncSearch*    getPredSearch() { return  m_pcPredSearch; }
  TComRdCost*    getRdCost()     { return  m_pcRdCost; }
#endif
};


#endif // __TENCPICTURE__

