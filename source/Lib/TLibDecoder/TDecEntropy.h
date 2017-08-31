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

/** \file     TDecEntropy.h
    \brief    entropy decoder class (header)
*/

#ifndef __TDECENTROPY__
#define __TDECENTROPY__

#include "../TLibCommon/CommonDef.h"
#include "../TLibCommon/TComBitStream.h"
#include "../TLibCommon/TComPicture.h"
#include "../TLibCommon/TComPic.h"
#include "../TLibCommon/TComPrediction.h"
#include "TDecSbac.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

class TDecEntropy
{
private:
  TDecSbac*  m_pcEntropyDecoderIf;
  TComPrediction* m_pcPrediction;

public:
  Void init (TComPrediction* p) {m_pcPrediction = p;}

  Void    setEntropyDecoder           ( TDecSbac* p );
  Void    setBitstream                ( TComBitstream* p )      { m_pcEntropyDecoderIf->setBitstream(p);                    }
  Void    resetEntropy                ( TComPicture* p)           { m_pcEntropyDecoderIf->resetEntropy(p);                    }
  
  Void    decodeSPS(TComSPS* pcSPS)    
  {
    m_pcEntropyDecoderIf->parseSPS(pcSPS);

#if AVS3_EXTENSION_LWJ
    UInt	uiCode, uiCode1, uiCode2, uiCode3;
#if AVS3_EXTENSION_DEBUG_SYC
    m_pcEntropyDecoderIf->xNextNBit(32, uiCode);
#else
    m_pcEntropyDecoderIf->xReadCodeVlc(32, uiCode);
#endif
    while (uiCode == 0x1B5 || uiCode == 0x1B2)
    {
#if AVS3_EXTENSION_DEBUG_SYC
      m_pcEntropyDecoderIf->xReadCodeVlc(32, uiCode);
#endif
      if (uiCode == 0x1B5)
      {
#if AVS3_EXTENSION_DEBUG_SYC
        m_pcEntropyDecoderIf->xNextNBit(4, uiCode);
#else
        m_pcEntropyDecoderIf->xReadCodeVlc(4, uiCode);
#endif
        if (uiCode == 2)
          m_pcEntropyDecoderIf->parseExtSeqDisplay(pcSPS);
        else if (uiCode == 3)
          m_pcEntropyDecoderIf->parseExtScalability(pcSPS);
        else if (uiCode == 4)
          m_pcEntropyDecoderIf->parseExtCopyright(pcSPS->getExtCopyright());
        else if (uiCode == 10)
        {
          TComExtMetadata* a = pcSPS->getExtMetadata();
          m_pcEntropyDecoderIf->parseExtMetadata(a);
        }
        else if (uiCode == 11)
          m_pcEntropyDecoderIf->parseExtCameraPra(pcSPS->getExtCameraPra());  
        else															//reserved_extension_data_byte
        {
#if AVS3_EXTENSION_DEBUG_SYC
          m_pcEntropyDecoderIf->xNextNBit(24, uiCode);
          uiCode3 = uiCode & 31;
          uiCode >>= 8;
          uiCode2 = uiCode & 31;
          uiCode >>= 8;
          uiCode1 = uiCode & 31;
          while (!((uiCode1 == 0) && (uiCode2 == 0) && (uiCode3 == 1)))
          {
            m_pcEntropyDecoderIf->xReadCodeVlc(8, uiCode);//读出真正的语法元素，暂时不存储。
            uiCode1 = uiCode2;
            uiCode2 = uiCode3;
            m_pcEntropyDecoderIf->xNextNBit(24, uiCode);
            uiCode3 = uiCode & 31;
          }
#else
          m_pcEntropyDecoderIf->xReadCodeVlc(8, uiCode1); 
          m_pcEntropyDecoderIf->xReadCodeVlc(8, uiCode2); 
          m_pcEntropyDecoderIf->xReadCodeVlc(8, uiCode3);
          while (!((uiCode1 == 0) && (uiCode2 == 0) && (uiCode3 == 1)))
          {
            uiCode1 = uiCode2;
            uiCode2 = uiCode3;
            m_pcEntropyDecoderIf->xReadCodeVlc(8, uiCode3);
          }
#endif
        }
      } 
      else
      {
        m_pcEntropyDecoderIf->parseUserData(pcSPS->getUserData());
      }
#if AVS3_EXTENSION_DEBUG_SYC
      m_pcEntropyDecoderIf->xNextNBit(32, uiCode);
#else
      m_pcEntropyDecoderIf->xReadCodeVlc(32, uiCode);
#endif
    }	  
#endif
  }
#if !AVS3_DELETE_PPS_SYC
  Void    decodePPS                   ( TComPPS* pcPPS     )    { m_pcEntropyDecoderIf->parsePPS(pcPPS);                    }
#endif
#if AVS3_PIC_HEADER_ZL
  Void    decodePicHeader             ( TComPicHeader*& rpcPicHeader )  
  { 
    m_pcEntropyDecoderIf->parsePicHeader(rpcPicHeader); 
#if AVS3_EXTENSION_LWJ
    UInt uiCode, uiCode1, uiCode2, uiCode3;
#if AVS3_EXTENSION_DEBUG_SYC
    m_pcEntropyDecoderIf->xNextNBit(32, uiCode);
#else
    m_pcEntropyDecoderIf->xReadCodeVlc(32, uiCode);
#endif
    while (uiCode == 0x1B5 || uiCode == 0x1B2)
    {
#if AVS3_EXTENSION_DEBUG_SYC
      m_pcEntropyDecoderIf->xReadCodeVlc(32, uiCode);
#endif
      if (uiCode == 0x1B5)
      {
#if AVS3_EXTENSION_DEBUG_SYC
        m_pcEntropyDecoderIf->xNextNBit(4, uiCode);
#else
        m_pcEntropyDecoderIf->xReadCodeVlc(4, uiCode);
#endif
        if (uiCode == 4)
          m_pcEntropyDecoderIf->parseExtCopyright(rpcPicHeader->getExtCopyright());
        else if (uiCode == 7)
          m_pcEntropyDecoderIf->parseExtPicDisplay(rpcPicHeader);
        else if (uiCode == 11)
          m_pcEntropyDecoderIf->parseExtCameraPra(rpcPicHeader->getExtCameraPra());
        else if (uiCode == 12)
          m_pcEntropyDecoderIf->parseExtRoiPra(rpcPicHeader);
        else															//reserved_extension_data_byte
        {
#if AVS3_EXTENSION_DEBUG_SYC
          m_pcEntropyDecoderIf->xNextNBit(24, uiCode);
          uiCode3 = uiCode & 31;
          uiCode >>= 8;
          uiCode2 = uiCode & 31;
          uiCode >>= 8;
          uiCode1 = uiCode & 31;
          while (!((uiCode1 == 0) && (uiCode2 == 0) && (uiCode3 == 1)))
          {
            m_pcEntropyDecoderIf->xReadCodeVlc(8, uiCode);//读出真正的语法元素，暂时不存储。
            uiCode1 = uiCode2;
            uiCode2 = uiCode3;
            m_pcEntropyDecoderIf->xNextNBit(24, uiCode);
            uiCode3 = uiCode & 31;
          }
#else
          m_pcEntropyDecoderIf->xReadCodeVlc(8, uiCode1); 
          m_pcEntropyDecoderIf->xReadCodeVlc(8, uiCode2); 
          m_pcEntropyDecoderIf->xReadCodeVlc(8, uiCode3);
          while (!((uiCode1 == 0) && (uiCode2 == 0) && (uiCode3 == 1)))
          {
            uiCode1 = uiCode2;
            uiCode2 = uiCode3;
            m_pcEntropyDecoderIf->xReadCodeVlc(8, uiCode3);
          }
#endif
        }
      }
      else
      {
        m_pcEntropyDecoderIf->parseUserData(rpcPicHeader->getUserData());
      }
#if AVS3_EXTENSION_DEBUG_SYC
      m_pcEntropyDecoderIf->xNextNBit(32, uiCode);
#else
      m_pcEntropyDecoderIf->xReadCodeVlc(32, uiCode);
#endif
    }
#endif
  }
#endif

#if AVS3_SLICE_HEADER_SYC
  Void    decodePictureHeader           ( TComPicture*& rpcPicture, TComPicHeader*& rpcPicHeader )  { m_pcEntropyDecoderIf->parsePictureHeader(rpcPicture, rpcPicHeader);         }
#else
  Void    decodePictureHeader           ( TComPicture*& rpcPicture )  { m_pcEntropyDecoderIf->parsePictureHeader(rpcPicture);         }
#endif
  Void    decodeTerminatingBit        ( UInt& ruiIsLast )       { m_pcEntropyDecoderIf->parseTerminatingBit(ruiIsLast);     }
  
  TDecSbac* getEntropyDecoder() { return m_pcEntropyDecoderIf; }
  
public:
  Void decodeSplitFlag         ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#if INTER_GROUP
	Void decodeInterCUTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
	Void decodeShapeOfPartitionIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
	Void decodeBPUTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
	Void decodeFPUTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
	Void decodeBPUTypeIndex2(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
	Void decodeFPUTypeIndex2(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
	Void decodeDMHMode(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
	Void decodeWeightedSkipMode(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
	Void decodeCUSubTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
#endif
  Void decodePredMode          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#if F_DHP_SYC
  Void decodeInterDHP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
#endif
#if	inter_direct_skip_bug2
  Void decodeInterWSM(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
#endif
#if F_MHPSKIP_SYC
  Void decodeInterMHPSKIP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
#endif
  Void decodePartSize          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );

  Void decodeTransformIdx      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void decodePredInfo          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth/*, TComDataCU* pcSubCU*/ );
#if ZHANGYI_INTRA
  Void decodeIntraPuTypeIndex( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void decodeIntraDirCb( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void decodeIntraDirCr( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#endif


#if WRITE_INTERDIR
  Void decodeInterDirRD(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
#else
  Void decodeInterDir          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#endif

  Void decodeIntraDir          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#if RPS
  Void decodeRef(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, RefPic eRefPic);
#endif
#if rd_mvd
  Void decodeMvd(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
#else
  Void decodeMvd               ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, RefPic eRefPic );
#endif
  Void decodeDBKIdx            ( TComDataCU* pcCU, UInt uiAbsPartIdx );
private:
  Void xDecodeCoeff            ( TComDataCU* pcCU, TCoeff* pcCoeff, UInt uiAbsPartIdx, UInt uiLog2Size, UInt uiHeight, UInt uiTrIdx, UInt uiCurrTrIdx, TextType eType );
public:
  Void decodeCoeff             ( TComDataCU* pcCU                 , UInt uiAbsPartIdx, UInt uiDepth, UInt uiLog2Size );
};// END CLASS DEFINITION TDecEntropy


#endif // __TDECENTROPY__

