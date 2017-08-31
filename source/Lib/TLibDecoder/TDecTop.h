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

/** \file     TDecTop.h
    \brief    decoder class (header)
*/

#ifndef __TDECTOP__
#define __TDECTOP__

#include "../TLibCommon/CommonDef.h"
#include "../TLibCommon/TComList.h"
#include "../TLibCommon/TComPicYuv.h"
#include "../TLibCommon/TComPic.h"
#include "../TLibCommon/TComTrQuant.h"

#include "TDecGop.h"
#include "TDecEntropy.h"
#include "TDecSbac.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// decoder class
class TDecTop
{
private:
  Int                     m_iGopSize;
  Bool                    m_bGopSizeSet;
  int                     m_iMaxRefPicNum;
  
  UInt                    m_uiValidPS;
  TComList<TComPic*>      m_cListPic;         //  Dynamic buffer
  TComSPS                 m_cSPS;
#if !AVS3_DELETE_PPS_SYC
  TComPPS                 m_cPPS;
#endif
#if AVS3_PIC_HEADER_ZL
  TComPicHeader*          m_pcPicHeader;
#endif
  TComPicture*            m_apcPicturePilot;
  
  // functional classes
  TComPrediction          m_cPrediction;
  TComTrQuant             m_cTrQuant;
  TDecGop                 m_cGopDecoder;
  TDecPicture             m_cPictureDecoder;
  TDecCu                  m_cCuDecoder;
  TDecEntropy             m_cEntropyDecoder;
  TDecSbac                m_cSbacDecoder;
  TDecLoopFilter          m_cLoopFilter;

public:
  TDecTop();
  virtual ~TDecTop();
  
  Void  create  ();
  Void  destroy ();
  
  Void  init();
  Void  decode ( Bool bEos, TComBitstream* pcBitstream, UInt& ruiPOC, TComList<TComPic*>*& rpcListPic );
  
  Void  deletePicBuffer();
  
protected:
#if RPS
  Void  xGetNewPicBuffer(TComPicHeader* m_pcPicHeader, TComPic*& rpcPic);
  Void  xRemovePicBuffer(TComPicHeader* m_pcPicHeader);
#else
  Void  xGetNewPicBuffer(TComPicture* pcPicture, TComPic*& rpcPic);
#endif
 // Void  xGetNewPicBuffer  (TComPicture* pcPicture, TComPic*& rpcPic);
  Void  xUpdateGopSize    (TComPicture* pcPicture);
  
};// END CLASS DEFINITION TDecTop


#endif // __TDECTOP__

