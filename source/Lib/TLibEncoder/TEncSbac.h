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

/** \file     TEncSbac.h
    \brief    Context-adaptive entropy encoder class (header)
*/

#ifndef __TENCSBAC__
#define __TENCSBAC__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../TLibCommon/TComBitStream.h"
#include "../TLibCommon/ContextTables.h"
#include "../TLibCommon/ContextModel.h"
#include "../TLibCommon/ContextModel3DBuffer.h"
#include "../TLibCommon/TComCABACTables.h"
#include "TEncEntropy.h"

class TEncTop;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// SBAC encoder class
class TEncSbac
{
public:
  TEncSbac();
  virtual ~TEncSbac();

  Void  resetEntropy           ();
  Void  setBitstream           ( TComBitIf* p )  { m_pcBitIf = p; }
#if wlq_AEC
  TComBitIf*  getBitstream           ()  { return m_pcBitIf; }
  UInt*       getEbits_to_follow     ()  {return &m_Ebits_to_follow;}
  Int         getElow                ()  {return m_Elow;}
#endif
  
  Void  setPicture               ( TComPicture* p )  { m_pcPicture = p; }
#if AVS3_PIC_HEADER_ZL
  Void  setPicHeader               ( TComPicHeader* p )  { m_pcPicHeader = p; }
#endif
  
  Void  resetCoeffCost         ()                { m_uiCoeffCost = 0;  }
  UInt  getCoeffCost           ()                { return  m_uiCoeffCost;  }
  Void  load                   ( TEncSbac* pScr  );
  Void  store                  ( TEncSbac* pDest );
  Void  resetBits              ();
  UInt  getNumberOfWrittenBits ();
  
  Void  codeSPS                 ( TComSPS* pcSPS     );
#if AVS3_EXTENSION_LWJ
  Void  codeUserData			( TComUserData* pcUserData);
  Void  codeExtSeqDisplay		( TComSPS* pcSPS	);
  Void  codeExtScalability		( TComSPS* pcSPS	);
  Void  codeReservedExt		    ( );
#endif
#if AVS3_EXTENSION_CYH
  Void  codeExtCopyright		(TComExtCopyright* pcExtCopyright);
  Void  codeExtCameraPra		(TComExtCameraPra* pcExtCameraPra);
#endif
#if AVS3_EXTENSION_LYT
  Void	codeExtMetadata			( TComExtMetadata*		pcExtMetadata );
  Void	codeExtRoiPra			(TComPicHeader* pcPicHeader);
  Void	codeExtPicDisplay		(TComPicHeader* pcPicHeader);
  Void  xWriteCodeVlc           ( UInt uiCode, UInt uiLength );
#endif
#if !AVS3_DELETE_PPS_SYC
  Void  codePPS                 ( TComPPS* pcPPS     );
#endif
#if AVS3_PIC_HEADER_ZL
  Void  codeIPicHeader                ( TComPicHeader* pcPicHeader     );
  Void  codePBPicHeader               ( TComPicHeader* pcPicHeader     );
#endif

  Void  codePictureHeader       ( TComPicture* pcPicture );
  Void  codeTerminatingBit      ( UInt uilsLast      );
  Void  codePictureFinish       ();
  Void  codeDBKIdx              ( TComDataCU* pcCU, UInt uiAbsPartIdx );

private:
  Void  xWriteUnarySymbol     ( UInt uiSymbol, ContextModel* pcSCModel, Int iOffset );
  Void  xWriteUnarySymbolSbac ( UInt uiSymbol, ContextModel* pcSCModel, UInt uiNumCtx );
  Void  xWriteExGolomb        ( UInt uiSymbol, ContextModel* pcSCModel );
  Void  xWriteUnaryMaxSymbol  ( UInt uiSymbol, ContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol );
  Void  xWriteEpExGolomb      ( UInt uiSymbol, UInt uiCount );
  Void  xWriteTerminatingBit  ( UInt uiBit );
  Void  xCheckCoeff           ( TCoeff* pcCoef, UInt uiSize, UInt uiDepth, UInt& uiNumofCoeff, UInt& uiPart );
  Void  xWriteMvd             ( Int iMvd, UInt uiCtx );
  Void  xCopyFrom             ( TEncSbac* pSrc );

  // From VLC for SPS, PPS, picture header coding
#if !AVS3_EXTENSION_LWJ  
  Void  xWriteCodeVlc         ( UInt uiCode, UInt uiLength );
#endif
  Void  xWriteUvlc            ( UInt uiCode );
  Void  xWriteSvlc            ( Int iCode   );
#if wlq_vlc
  Void  xUeLinfo			  ( UInt uiCode ,Int* len, Int* info );
  Void  xSeLinfo			  ( Int iCode ,Int* len, Int* info );
  Void  xSymbol2Uvlc          ( Int len, Int info, UInt* bitpattern);
#endif
  Void  xWriteFlagVlc         ( UInt uiCode );
  UInt  xConvertToUInt        ( Int iValue ) {  return ( iValue <= 0) ? -iValue<<1 : (iValue<<1)-1; }

  // For SBAC coding
  Void xPutByte           ( UChar ucByte );
  Void xCarryPropagate    ( UInt& ruiCode );
  Void xEncodeBin         ( UInt  uiBin, ContextModel& rcCtxModel );
  Void xEncodeBinEP       ( UInt  uiBin                            );
#if wlq_AEC
  Void xEncodeBinF        ( UInt  uiBin                            );
#endif  
protected:
  TComBitIf*    m_pcBitIf;
  TComPicture*    m_pcPicture;
#if AVS3_PIC_HEADER_ZL
  TComPicHeader* m_pcPicHeader;
#endif
  UInt          m_uiCoeffCost;
#if RPS
  Void codeRefPicSetList(const TComReferencePictureSet* pcRPS, Int idx);
#endif
public:

#if 0//YQH_SPLIGFLAG_DEC_BUG2
#if WLQ_CUSplitFlag
	Void codeSplitFlag(TComDataCU* pcCU, UInt value, Bool key, UInt value2);
#endif
	Void codeSplitFlag           ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#else
#if WLQ_CUSplitFlag
	Void codeSplitFlag(TComDataCU* pcCU, UInt value, Bool key, UInt value2);
#else
  Void codeSplitFlag           ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#endif
#endif
  Void codePredMode            ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#if INTER_GROUP
	//AVS3
	Void write(Int value){ return; } // todo
	Void codeShapeOfPartitionIndex(TComDataCU* pcCU, UInt uiAbsPartIdx);
	Void codeB2Nx2NInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx);  ///< no  split
	Void codeB2NxnNInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx);  ///< two split
	Void codeF2Nx2NInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx);  ///< no  split
	Void codeF2NxnNInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx);  ///< two split
	Void codeBNxNInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx);  ///< four split
	Void codeFNxNInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx);  ///< four split
	Void codeDMHMode(TComDataCU* pcCU, UInt uiAbsPartIdx);
	Void codeInterCUTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx);
	//AVS3
#endif
#if DMH
	Void writeDMHMode(TComDataCU *pcCU, UInt uiAbsPartIdx);
#endif
	Void codePartSize            ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeInterDir            ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeIntraDir            ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#if WRITE_INTERDIR
  Void codeInterDirRD(TComDataCU* pcCU, UInt uiAbsPartIdx);
#endif
#if ZHANGYI_INTRA
  Void codeIntraDirForRDO            ( Int iIntraDirForRDO );
  Void codeIntraPuTypeIdx( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeIntraDirCb( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeIntraDirCbForRDO( TComDataCU* pcCU, UInt uiAbsPartIdx,Int iIntraDirCbValue );
  Void codeIntraDirCrForRDO( TComDataCU* pcCU, UInt uiAbsPartIdx,Int iIntraDirCbValue );
  Void codeIntraDirCr( TComDataCU* pcCU, UInt uiAbsPartIdx );
#endif
#if RPS
  Void codeRefIdx(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPic eRefPic);
#endif
  Void codeMvd                 ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPic eRefPic );
  Void codeAllCbfs             ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeCbf                 ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth );
#if niu_CbpBit
  Void TEncSbac::writeCbpbit(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt b8mode, UInt cbp_bit);
#endif
#if niu_write_cbf
  Void codeCbfY(TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth);
  Void codeCbfUV(TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth);
#endif
  Void codeCoeffNxN            ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiLog2Size, UInt uiDepth, TextType eTType, Bool bRD = false );
  Void codeTransformIdx        ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#if	F_DHP_SYC
  Void codeInterDHP(TComDataCU* pcCU, UInt uiAbsPartIdx);
  Void codeInterDHP_FNXN(TComDataCU* pcCU, UInt uiAbsPartIdx);
#endif
#if	inter_direct_skip_bug1
  Void codeInterWSM(TComDataCU* pcCU, UInt uiAbsPartIdx);
#endif
#if	F_MHPSKIP_SYC
  Void codeInterMHPSKIP(TComDataCU* pcCU, UInt uiAbsPartIdx);
#endif
  #if YQH_INTRA
   Void codeTransSplitFlag( TComDataCU* pcCU, UInt uiAbsPartIdx );
#endif
#if CODEFFCODER_LDW_WLQ_YQH
  Void  xCheckCoeffcode( TComDataCU* pcCU,TCoeff* pcCoef,Int CoeffScanIndex,Int iWidth, Int iHeight, Bool bReverse, UInt& uiNumofCoeff, CoeffCodingParam&  m_CoeffCodingParam);//yuquanhe@hisilicon.com
  Void  getCoeffScanAndCGSacn(Int& CoeffScanIndex, Int& CGSacnIndex , Int riWidth, Int riHeight );//yuquanhe@hisilicon.com
  Void  codeCoeff( TComDataCU* pcCU,TCoeff* pcCoef,UInt uiAbsPartIdx,Int CGSacnIndex ,Int iWidth, Int iHeight,TextType eTType,  UInt uiNumofCoeff, CoeffCodingParam&  m_CoeffCodingParam);//yuquanhe@hisilicon.com
  Void  codeCoeffCGLastXAndCGLastY(TextType eTType,Int riWidth, Int riHeight, UInt uiAbsPartIdx,Int iCG,Int CGSacnIndex , Int uiIntraModeIdx,Int& CGLastX, Int& CGLastY  );//yuquanhe@hisilicon.com
  Void  codeCoeffinCGLastXAndCGLastY(CoeffCodingParam&  m_CoeffCodingParam,TextType eTType,Int rank, Int numOfCG, Int ctxmode,Int iCG, Int CGx, Int CGy  );//yuquanhe@hisilicon.com
  Void  codeCoeffRun(Int baseRun,Int absLevel,TextType eTType, int pairs,int numOfCoeff,int pairsInCG, Int ctxmode, UInt rank, UInt iCG, int pos ,CoeffCodingParam  m_CoeffCodingParam);//yuquanhe@hisilicon.com
  Void  getCGposXAndposY(Int riWidth, Int riHeight,Int iCG,Int CGSacnIndex , Int& CGX, Int& CGY  );//yuquanhe@hisilicon.com
  Void  codeCoeffAbsLevel( Int absLevel,TextType eTType,int pairsInCG, UInt rank,  UInt iCG, int pos   );//yuquanhe@hisilicon.com
  Void  getTUSize( UInt uiPartIdx,  Int& riWidth, Int& riHeight,Int uiLog2Size,  PartSize m_pePartSize  );//yuquanhe@hisilicon.coms
#endif

#if !niu_NSQT_dec
#if RD_NSQT
  Void  getTUSize(UInt uiPartIdx, Int& riWidth, Int& riHeight, Int uiLog2Size, PartSize m_pePartSize, Bool bNSQT);
#endif
#endif
  // For RDOQ
  Void estBit                        ( estBitsSbacStruct* pcEstBitsSbac, TextType eTType );
  Void estCBFBit                     ( estBitsSbacStruct* pcEstBitsSbac, TextType eTType );
  Void estRunLevelLastBit            ( estBitsSbacStruct* pcEstBitsSbac, TextType eTType );
  __inline Int  biari_no_bits        ( Short symbol, ContextModel& rcSCModel );

private:

  ContextModel3DBuffer m_cCUSplitFlagSCModel;
#if YQH_INTRA
  ContextModel3DBuffer m_cTUSplitFlagSCModel;//yuquanhe@hisilicon.com
  ContextModel3DBuffer m_cPUIntraTypeSCModel;//yuquanhe@hisilicon.com
#endif
  ContextModel3DBuffer m_cCUPredModeSCModel;
  ContextModel3DBuffer m_cCUPartSizeSCModel;
#if INTER_GROUP
	ContextModel3DBuffer m_cCUShapeOfPartitionSCModel;
#endif
  ContextModel3DBuffer m_cCUInterDirSCModel;
#if WRITE_INTERDIR
  ContextModel3DBuffer m_cCUMinInterDirSCModel;
#endif
#if F_DHP_SYC
  ContextModel3DBuffer m_cCUInterDHPSCModel;
  ContextModel3DBuffer m_cCUInterDHPNXNSCModel;
#endif
#if inter_direct_skip_bug1
  ContextModel3DBuffer m_cCUInterWSMSCModel;
#endif
#if F_MHPSKIP_SYC
  ContextModel3DBuffer m_cCUInterMHPSKIPSCModel;
#endif
  ContextModel3DBuffer m_cCUIntraDirSCModel;
#if RPS
  ContextModel3DBuffer m_cCURefPicSCModel;
#endif
#if DMH
	ContextModel3DBuffer m_cCUDmhSCModel;
#endif
  ContextModel3DBuffer m_cCUMvdSCModel;
  ContextModel3DBuffer m_cCUQtCbfSCModel;
  ContextModel3DBuffer m_cCUQtRootCbfSCModel;
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
#if wlq_AEC
  Int  m_Elow;
  Int  m_E_s1;
  Int  m_E_t1;
  UInt m_s2, m_t2;
  UInt m_s1, m_t1;
  UInt m_Ebits_to_follow;
#else
  UInt   m_uiRange;
  UInt   m_uiCode;
 #endif
#if 0//YQH_AEC_RDO_BUG
  UInt m_uiByteCounter;
  UInt m_uiCodeBitsbiggo;
#endif
  UInt   m_uiCodeBits;
  UInt   m_uiStackedFFs;
  UInt   m_uiStackedZeros;
  UChar  m_ucPendingByte;
  Bool   m_bIsPendingByte;
};

#endif // !defined(AFX_TENCSBAC_H__DDA7CDC4_EDE3_4015_9D32_2156249C82AA__INCLUDED_)
