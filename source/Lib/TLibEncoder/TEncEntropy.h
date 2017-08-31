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

/** \file     TEncEntropy.h
    \brief    entropy encoder class (header)
*/

#ifndef __TENCENTROPY__
#define __TENCENTROPY__

#include "../TLibCommon/TComPicture.h"
#include "../TLibCommon/TComDataCU.h"
#include "../TLibCommon/TComBitStream.h"
#include "../TLibCommon/ContextModel.h"
#include "../TLibCommon/TComPic.h"
#include "../TLibCommon/TComTrQuant.h"
#include "TEncSbac.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

class TEncEntropy
{
public:
#if AVS3_PIC_HEADER_ZL
  Void    setEntropyCoder           ( TEncSbac* e, TComPicture* pcPicture, TComPicHeader* pcPicHeader );
#else
  Void    setEntropyCoder           ( TEncSbac* e, TComPicture* pcPicture );
#endif
  Void    setBitstream              ( TComBitIf* p )          { m_pcEntropyCoderIf->setBitstream(p);  }
  Void    resetBits                 ()                        { m_pcEntropyCoderIf->resetBits();      }
  Void    resetCoeffCost            ()                        { m_pcEntropyCoderIf->resetCoeffCost(); }
  UInt    getNumberOfWrittenBits    ()                        { return m_pcEntropyCoderIf->getNumberOfWrittenBits(); }
  UInt    getCoeffCost              ()                        { return  m_pcEntropyCoderIf->getCoeffCost(); }
  Void    resetEntropy              ()                        { m_pcEntropyCoderIf->resetEntropy();  }
#if AVS3_PIC_HEADER_ZL
  Void encodeIPicHeader              ( TComPicHeader* pcPicHeader );
  Void encodePBPicHeader             ( TComPicHeader* pcPicHeader );
#endif
  Void    encodePictureHeader         ( TComPicture* pcPicture );
  Void    encodeTerminatingBit      ( UInt uiIsLast );
  Void    encodePictureFinish         ();
#if wlq_AEC
#if YQH_RDO_BITOURNT_BUG
  TComBitIf*  getBitstream()                      { return m_pcEntropyCoderIf->getBitstream(); }
#endif
  Void    encodeEntropyFinish         ();
#endif   
  TEncSbac*      m_pcEntropyCoderIf;

public:
  // SPS
  Void encodeSPS               ( TComSPS* pcSPS );
#if !AVS3_DELETE_PPS_SYC
  Void encodePPS               ( TComPPS* pcPPS );
#endif

#if WLQ_CUSplitFlag
  Void encodeSplitFlag(TComDataCU* pcCU, UInt value, Bool key, UInt value2);
#else
  Void encodeSplitFlag ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD = false );
#endif

#if INTER_GROUP
	Void encodeShapeOfPartitionIndex(TComDataCU* pcCU, UInt uiAbsPartIdx);
	Void encodeBPUTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false);
	Void encodeFPUTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false);
	Void encodeBPUTypeIndex2(TComDataCU* pcCU, UInt uiAbsPartIdx);
	Void encodeFPUTypeIndex2(TComDataCU* pcCU, UInt uiAbsPartIdx);
	Void encodeDMHMode(TComDataCU* pcCU, UInt uiAbsPartIdx);
	Void encodeWeightedSkipMode(TComDataCU* pcCU, UInt uiAbsPartIdx);
	Void encodeCUSubTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx);
	Void encodeInterCUTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false);
#endif
	Void encodePredMode          ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
  Void encodePartSize          ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );

  Void encodePredInfo          ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
#if WRITE_INTERDIR
  Void encodeInterDirRD(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false);
#else
  Void encodeInterDir          ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
#endif
  Void encodeIntraDir          ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
#if F_DHP_SYC
  Void encodeInterDHP(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD);
  Void encodeInterDHP_FNXN(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD);
#endif
#if inter_direct_skip_bug1
  Void encodeInterWSM          (TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD);
#endif
#if F_MHPSKIP_SYC
  Void encodeInterMHPSKIP(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD);
#endif
#if B_MHBSKIP_SYC
  Void encodeInterMHBSKIP(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD);
#endif
#if ZHANGYI_INTRA
  Void encodeIntraPuTypeIndex (TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
  Void encodeIntraDirForRDO( Int uiIntraDirValue );
  Void  encodeIntraDirCr( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
  Void  encodeIntraDirCb( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false);
  Void encodeIntraDirCbForRDO(TComDataCU* pcCU, UInt uiAbsPartIdx, Int iIntraDirCbValue );
  Void encodeIntraDirCrForRDO(TComDataCU* pcCU, UInt uiAbsPartIdx, Int iIntraDirCbValue );
#endif
#if RPS
  Void encodeRef(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPic eRefPic);
#endif
#if rd_mvd
  Void encodeMvd(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false);
#else
  Void encodeMvd               ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPic eRefPic, Bool bRD = false );
#endif

  Void encodeDBKIdx            ( TComDataCU* pcCU, UInt uiAbsPartIdx );

private:
  Void xEncodeCoeff            ( TComDataCU* pcCU, TCoeff* pcCoeff, UInt uiAbsPartIdx, UInt uiDepth, UInt uiLog2Size, UInt uiTrIdx, UInt uiCurrTrIdx, TextType eType, Bool bRD = false );
public:
  Void encodeTransformIdx      ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );

  Void encodeCbf               ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, Bool bRD = false );
#if niu_write_cbf
  Void encodeCbfY(TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, Bool bRD = false);
  Void encodeCbfUV(TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, Bool bRD = false);
#endif  
  Void encodeCoeff             ( TComDataCU* pcCU, TCoeff* pCoeff, UInt uiAbsPartIdx, UInt uiDepth, UInt uiLog2Size, UInt uiMaxTrMode, UInt uiTrMode, TextType eType, Bool bRD = false );

  Void encodeCoeff             ( TComDataCU* pcCU                 , UInt uiAbsPartIdx, UInt uiDepth, UInt uiLog2Size );
  Void encodeCoeffNxN          ( TComDataCU* pcCU, TCoeff* pcCoeff, UInt uiAbsPartIdx, UInt uiLog2Size, UInt uiDepth, TextType eType, Bool bRD = false );
  
  Void estimateBit             ( estBitsSbacStruct* pcEstBitsSbac, UInt uiSize, TextType eTType);
};// END CLASS DEFINITION TEncEntropy


#endif // __TENCENTROPY__
