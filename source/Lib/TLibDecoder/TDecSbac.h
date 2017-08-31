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

/** \file     TDecSbac.h
    \brief    SBAC decoder class (header)
*/

#ifndef __TDECSBAC__
#define __TDECSBAC__


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../TLibCommon/CommonDef.h"
#include "../TLibCommon/ContextTables.h"
#include "../TLibCommon/ContextModel.h"
#include "../TLibCommon/ContextModel3DBuffer.h"
#include "../TLibCommon/TComCABACTables.h"
#include "../TLibCommon/TComPicture.h"
#include "../TLibCommon/TComBitStream.h"
#include "../TLibCommon/TComPic.h"
#include "../TLibCommon/TComPrediction.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// SBAC decoder class
class TDecSbac
{
public:
  TDecSbac();
  virtual ~TDecSbac();
  
  Void  resetEntropy              ( TComPicture* pcPicture     );
  Void  setBitstream              ( TComBitstream* p       ) { m_pcBitstream = p; }
  
  Void  parseSPS                  ( TComSPS* pcSPS         );
#if AVS3_EXTENSION_LWJ
  Void  parseUserData			  (  TComUserData* pcUserData		   );
  Void  parseExtSeqDisplay		  ( TComSPS* pcSPS		   );
  Void  parseExtScalability		  ( TComSPS* pcSPS		   );
  Void  xReadCodeVlc			  (UInt uiLength, UInt& ruiCode);
#endif
#if AVS3_EXTENSION_DEBUG_SYC
  Void  xNextNBit       (UInt uiLength, UInt& ruiCode);
#endif
#if AVS3_EXTENSION_CYH
  Void  parseExtCopyright		  (TComExtCopyright* pcExtCopyright);
  Void  parseExtCameraPra		  (TComExtCameraPra* pcExtCameraPra);
#endif
#if AVS3_EXTENSION_LYT
  Void	parseExtMetadata			( TComExtMetadata*& pcExtMetadata );
  Void	parseExtRoiPra(TComPicHeader*& pcPicHeader);
  Void	parseExtPicDisplay(TComPicHeader*& rpcPicHeader);
#endif
#if !AVS3_DELETE_PPS_SYC
  Void  parsePPS                  ( TComPPS* pcPPS         );
#endif
#if AVS3_PIC_HEADER_ZL
  Void  parsePicHeader            ( TComPicHeader*& rpcPicHeader   
#if POC_256_BUG
	  , Int prevTid0POC
#endif  
	  );
#endif
#if AVS3_SLICE_HEADER_SYC
  Void TDecSbac::parsePictureHeader (TComPicture*& rpcPicture, TComPicHeader*& rpcPicHeader);
#else
  Void  parsePictureHeader          ( TComPicture*& rpcPicture   );
#endif
  Void  parseTerminatingBit       ( UInt& ruiBit );
#if RPS
protected:
	Void  parseRefPicSet(TComSPS* pcSPS, TComReferencePictureSet* pcRPS, Int idx);
#endif
private:
  Void  xReadUnarySymbol    ( UInt& ruiSymbol, ContextModel* pcSCModel, Int iOffset );
  Void  xReadUnarySymbolSbac( UInt& ruiSymbol, ContextModel* pcSCModel, UInt uiNumCtx );
  Void  xReadUnaryMaxSymbol ( UInt& ruiSymbol, ContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol );
  Void  xReadEpExGolomb     ( UInt& ruiSymbol, UInt uiCount );
  Void  xReadExGolombLevel  ( UInt& ruiSymbol, ContextModel& rcSCModel  );
  Void  xReadMvd            ( Int& riMvdComp, UInt uiCtx );
#if wlq_AEC
  Void  xReadBit            ();
#endif
  Void  xReadBit            ( UInt& ruiVal );
  Void  xDecodeBin          ( UInt& ruiBin, ContextModel& rcCtxModel );
  Void  xDecodeBinEP        ( UInt& ruiBin                           );
#if wlq_AEC
  Void  xDecodeBinF         ( UInt& ruiBin                           );
#endif
#if !AVS3_EXTENSION_LWJ 
  Void  xReadCodeVlc        (UInt uiLength, UInt& ruiCode);
#endif
  Void  xReadUvlc           (UInt& ruiVal);
  Void  xReadSvlc           (Int& riVal);
#if wlq_vlc
  Void  xGetVLCSymbol       (Int* len,Int* info);
  Void  xLinfoUe			(Int len, Int info, UInt* ruiVal);
  Void  xLinfoSe			(Int len, Int info, Int* riVal);
#endif
  Void  xReadFlagVlc        (UInt& ruiCode);
  
#if CODEFFCODER_LDW_WLQ_YQH
  Void getCGLastAndposXAndposY(Int riWidth, Int riHeight,Int &CGLast, Int& CGX, Int& CGY  );
  Void  parseCoeffCGLastXAndCGLastY(TextType eTType,Int riWidth, Int riHeight,Int uiIntraModeIdx,Int& CGLast, Int& CGLastX, Int& CGLastY  );//yuquanhe@hisilicon.com
  Void parseCoeffinCGLastXAndCGLastY(Int& pos,TextType eTType,Int rank, Int numOfCG, Int ctxmode,Int iCG, Int firstCG, Int CGx, Int  CGy  );//yuquanhe@hisilicon.com
  Void parseCoeffAbsLevel( TextType eTType,int pairsInCG, UInt rank,  UInt firstCG, int pos ,Int& AbsLevel );//yuquanhe@hisilicon.com
  Void getCGposXAndposY(Int riWidth, Int riHeight,Int iCG,Int CGSacnIndex , Int& CGX, Int& CGY  );
  Void parseCoeffRun(Int absLevel,Int& baseRun, TextType eTType, int pairs,int numOfCoeff,int pairsInCG, Int ctxmode, UInt rank, UInt firstCG, int pos ,CoeffCodingParam  m_CoeffCodingParam);//yuquanhe@hisilicon.com

  Void parseCoeff( TComDataCU* pcCU,UInt uiAbsPartIdx,Int CGSacnIndex ,Int iWidth, Int iHeight,TextType eTType, CoeffCodingParam&  m_CoeffCodingParam ,Int& DCTCGNum ,Int& DCT_Pairs);//yuquanhe@hisilicon.com
  Void  getTUSize( UInt uiPartIdx,  Int& riWidth, Int& riHeight,Int uiLog2Size,  PartSize m_pePartSize  );//yuquanhe@hisilicon.coms
  Void  getCoeffScanAndCGSacn(Int& CoeffScanIndex, Int& CGSacnIndex , Int riWidth, Int riHeight );//yuquanhe@hisilicon.com
#endif


private:
  TComBitstream*    m_pcBitstream;
  
public:
  Void parseSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parsePredMode      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#if INTER_GROUP
	Void read(Int value){ return; } // todo
	Void parseShapeOfPartitionIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
	Void parseB2Nx2NInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);  ///< no  split
	Void parseB2NxnNInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);  ///< two split
	Void parseF2Nx2NInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);  ///< no  split
	Void parseF2NxnNInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);  ///< two split
	Void parseBNxNInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);  ///< four split
	Void parseFNxNInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);  ///< four split
	Void parseDMHMode(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
#if DMH
	Void readDMHMode(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
#endif
	Void parseInterCUTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
#endif
	Void parsePartSize      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseInterDir      ( TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseIntraDir      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#if WRITE_INTERDIR
  Void parseInterDirRD(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
#endif
#if RPS
  Void parseRefIdx(TComDataCU* pcCU, Int& iRefIdx, RefPic eRefList);
#endif
  Void parseMvd           ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth, RefPic eRefPic );
  Void parseAllCbf        ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt& uiIsCoeff );
#if niu_CbpBit
  UInt readCbpbit(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt b8);
#endif
#if niu_write_cbf
  Void parseCbfY(TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth);
#endif
  Void parseCbf           ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth );
  Void parseBlockCbf      ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth, UInt uiQPartNum ) {}
  Void parseCoeffNxN      ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiLog2Size, UInt uiDepth, TextType eTType );
  Void parseDBKIdx        ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void parseTransformIdx  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#if ZHANGYI_INTRA
  Void parseIntraPuTypeIndex( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseIntraDirCb( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseIntraDirCr( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#endif
#if YQH_INTRA
  PartSize parseIntraPuTypeIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );//yuquanhe@hisilicon.com
  PartSize parseTransSplitFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );//yuquanhe@hisilicon.com
#endif
#if	F_DHP_SYC
  Void  parseInterDHP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
  Void  parseInterDHP_FNXN(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
#endif
#if	inter_direct_skip_bug1
  Void  parseInterWSM(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
#endif
#if	F_MHPSKIP_SYC
  Void  parseInterMHPSKIP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
#endif
#if	B_MHBSKIP_SYC
  Void  parseInterMHBSKIP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
#endif
private:
  
  ContextModel3DBuffer m_cCUSplitFlagSCModel;
  ContextModel3DBuffer m_cCUPredModeSCModel;
  ContextModel3DBuffer m_cCUPartSizeSCModel;
#if INTER_GROUP
	ContextModel3DBuffer m_cCUShapeOfPartitionSCModel;
#endif
	ContextModel3DBuffer m_cCUInterDirSCModel;
#if WRITE_INTERDIR
	ContextModel3DBuffer m_cCUMinInterDirSCModel;
#endif
#if inter_direct_skip_bug1
	ContextModel3DBuffer m_cCUInterWSMSCModel;
#endif
#if F_MHPSKIP_SYC
	ContextModel3DBuffer m_cCUInterMHPSKIPSCModel;
#endif
#if B_MHBSKIP_SYC
	ContextModel3DBuffer m_cCUInterMHBSKIPSCModel;
#endif
#if F_DHP_SYC
	ContextModel3DBuffer m_cCUInterDHPSCModel;
	ContextModel3DBuffer m_cCUInterDHPNXNSCModel;
#endif
  ContextModel3DBuffer m_cCUIntraDirSCModel;
#if RPS
  ContextModel3DBuffer m_cCURefPicSCModel;
#endif
#if DMH
	ContextModel3DBuffer m_cCUDMHSCModel;
#endif
  ContextModel3DBuffer m_cCUMvdSCModel;
  ContextModel3DBuffer m_cCUQtRootCbfSCModel;
  ContextModel3DBuffer m_cCUQtCbfSCModel;
  ContextModel3DBuffer m_cCUTransIdxSCModel;
  ContextModel3DBuffer m_cCURunSCModel;
  ContextModel3DBuffer m_cCULastSCModel;
  ContextModel3DBuffer m_cCUAbsSCModel;
  ContextModel3DBuffer m_cDBKIdxSCModel;
  ContextModel3DBuffer m_cCUXPosiSCModel;
  ContextModel3DBuffer m_cCUYPosiSCModel;

#if CODEFFCODER_LDW_WLQ_YQH
  ContextModel3DBuffer  m_cTULastCGSCModel; //yuquanhe@hisilicon.com
  ContextModel3DBuffer  m_cTUSigCGSCModel; //yuquanhe@hisilicon.com
  ContextModel3DBuffer  m_cTULastPosInCGSCModel; //yuquanhe@hisilicon.com
  ContextModel3DBuffer  m_cTULevelRunSCModel; //yuquanhe@hisilicon.com
#endif
#if YQH_INTRA
  ContextModel3DBuffer m_cTUSplitFlagSCModel;//yuquanhe@hisilicon.com
  ContextModel3DBuffer m_cPUIntraTypeSCModel;//yuquanhe@hisilicon.com
#endif
#if wlq_AEC
  UInt  t1, value_t;
  UChar s1, value_s;
  UChar value_s_bound;
  UChar is_value_domain;//  is value in R domain 1 is R domain 0 is LG domain
  UChar is_value_bound;
  UChar cFlag;
#else

  UInt m_uiRange;
  UInt m_uiValue;
#endif
};

#endif // !defined(AFX_TDECSBAC_H__CFCAAA19_8110_47F4_9A16_810C4B5499D5__INCLUDED_)
