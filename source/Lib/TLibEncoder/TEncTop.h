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

/** \file     TEncTop.h
    \brief    encoder class (header)
*/

#ifndef __TENCTOP__
#define __TENCTOP__

// Include files
#include "../TLibCommon/TComList.h"
#include "../TLibCommon/TComPrediction.h"
#include "../TLibCommon/TComTrQuant.h"
#include "../TLibCommon/TComBitStream.h"

#include "../TLibVideoIO/TVideoIOYuv.h"
#include "../TLibVideoIO/TVideoIOBits.h"

#include "TEncCfg.h"
#include "TEncGOP.h"
#include "TEncPicture.h"
#include "TEncEntropy.h"
#include "TEncSbac.h"
#include "TEncSearch.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// encoder class
class TEncTop : public TEncCfg
{
private:
  // picture
  Int                     m_iPOCLast;                     ///< time index (POC)
  Int                     m_iNumPicRcvd;                  ///< number of received pictures
  UInt                    m_uiNumAllPicCoded;             ///< number of coded pictures
  TComList<TComPic*>      m_cListPic;                     ///< dynamic list of pictures
  
  // encoder search
  TEncSearch              m_cSearch;                      ///< encoder search class
  TEncEntropy*            m_pcEntropyCoder;                     ///< entropy encoder 

  // coding tool
  TComTrQuant             m_cTrQuant;                     ///< transform & quantization class
  TEncLoopFilter          m_cLoopFilter;                  ///< deblocking filter class
  TEncEntropy             m_cEntropyCoder;                ///< entropy encoder
  TEncSbac                m_cSbacCoder;                   ///< SBAC encoder
  
  // processing unit
  TEncGOP                 m_cGOPEncoder;                  ///< GOP encoder
  TEncPicture               m_cPictureEncoder;                ///< picture encoder
  TEncCu                  m_cCuEncoder;                   ///< CU encoder
  // SPS
  TComSPS                 m_cSPS;                         ///< SPS
#if !AVS3_DELETE_PPS_SYC
  TComPPS                 m_cPPS;                         ///< PPS
#endif
  
  // RD cost computation
  TComBitCounter          m_cBitCounter;                  ///< bit counter for RD optimization
  TComRdCost              m_cRdCost;                      ///< RD cost computation class
  TEncSbac***             m_pppcRDSbacCoder;              ///< temporal storage for RD computation
  TEncSbac                m_cRDGoOnSbacCoder;             ///< going on SBAC model for RD stage
  
protected:
  Void  xGetNewPicBuffer  ( TComPic*& rpcPic );           ///< get picture buffer which will be processed
  Void  xInitSPS          ();                             ///< initialize SPS from encoder options
  
#if RPS
  Void  xInitRPS();           ///< initialize RPS from encoder options 
#endif

public:
  TEncTop();
  virtual ~TEncTop();
  
  Void      create          ();
  Void      destroy         ();
  Void      init            ();
  Void      deletePicBuffer ();
  
  // -------------------------------------------------------------------------------------------------------------------
  // member access functions
  // -------------------------------------------------------------------------------------------------------------------
  
  TComList<TComPic*>*     getListPic            () { return  &m_cListPic;             }
  TEncSearch*             getPredSearch         () { return  &m_cSearch;              }
  
  TComTrQuant*            getTrQuant            () { return  &m_cTrQuant;             }
  TEncLoopFilter*         getLoopFilter         () { return  &m_cLoopFilter;          }
  
  TEncGOP*                getGOPEncoder         () { return  &m_cGOPEncoder;          }
  TEncPicture*              getPictureEncoder       () { return  &m_cPictureEncoder;        }
  TEncCu*                 getCuEncoder          () { return  &m_cCuEncoder;           }
  TEncEntropy*            getEntropyCoder       () { return  &m_cEntropyCoder;        }
  TEncSbac*               getSbacCoder          () { return  &m_cSbacCoder;           }
  
  TComBitCounter*         getBitCounter         () { return  &m_cBitCounter;          }
  TComRdCost*             getRdCost             () { return  &m_cRdCost;              }
  TEncSbac***             getRDSbacCoder        () { return  m_pppcRDSbacCoder;       }
  TEncSbac*               getRDGoOnSbacCoder    () { return  &m_cRDGoOnSbacCoder;     }
  
  TComSPS*                getSPS                () { return  &m_cSPS;                 }
#if !AVS3_DELETE_PPS_SYC
  TComPPS*                getPPS                () { return  &m_cPPS;                 }
#endif
#if RPS
#if B_RPS_BUG
  Void selectReferencePictureSet(TComPicHeader* pcPicHeader, Int POCCurr, Int GOPid, Int POCLast);
#else
  Void selectReferencePictureSet(TComPicHeader* pcPicHeader, Int POCCurr, Int GOPid);
#endif
#endif
  // -------------------------------------------------------------------------------------------------------------------
  // encoder function
  // -------------------------------------------------------------------------------------------------------------------
  
  /// encode several number of pictures until end-of-sequence
  Void encode( bool bEos, TComPicYuv* pcPicYuvOrg, TComList<TComPicYuv*>& rcListPicYuvRecOut,
              TComList<TComBitstream*>& rcListBitstreamOut, Int& iNumEncoded );
  
};


#endif // __TENCTOP__

