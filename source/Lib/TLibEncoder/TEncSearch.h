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

/** \file     TEncSearch.h
    \brief    encoder search class (header)
*/

#ifndef __TENCSEARCH__
#define __TENCSEARCH__

// Include files
#include "../TLibCommon/TComYuv.h"
#include "../TLibCommon/TComMotionInfo.h"
#include "../TLibCommon/TComPattern.h"
#include "../TLibCommon/TComPredFilter.h"
#include "../TLibCommon/TComPrediction.h"
#include "../TLibCommon/TComTrQuant.h"
#include "../TLibCommon/TComPic.h"
#include "TEncEntropy.h"
#include "TEncSbac.h"
#include "TEncCfg.h"

class TEncCu;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// encoder search class
class TEncSearch : public TComPrediction
{
private:
  TCoeff*         m_pcQTTempCoeffY;
  TCoeff*         m_pcQTTempCoeffCb;
  TCoeff*         m_pcQTTempCoeffCr;
  UChar*          m_puhQTTempTrIdx;
  UChar*          m_puhQTTempCbf[3];
#if wlq_FME
  Int			  m_BestX;
  Int			  m_BestY;
  UInt			  m_BestSad;
  Int**			  m_McostState;

  Double*		  m_Bsize;
  Double*		  m_AlphaSec;
  Double*		  m_AlphaThird;

  Int*			  m_spiral_search_x;
  Int*			  m_spiral_search_y;

  Int			  m_PicCodedNum;

  Int			  m_pred_SAD_space;
  Int			  m_pred_SAD_ref;
  Int			  m_pred_SAD_uplayer;
  Int			  m_pred_MV_time[2];
  Int 			  m_pred_MV_ref[2];
  Int 			  m_pred_MV_uplayer[2];
#endif

#if wlq_Mv_check
  Int  Min_V_MV;
  Int  Max_V_MV;
  Int  Min_H_MV;
  Int  Max_H_MV;
#endif

protected:
  // interface to option
  TEncCfg*        m_pcEncCfg;
  
  // interface to classes
  TComTrQuant*    m_pcTrQuant;
  TComRdCost*     m_pcRdCost;
  TEncEntropy*    m_pcEntropyCoder;
  
  // ME parameters
  Int             m_iSearchRange;
  Int             m_iFastSearch;
#if RPS
  Int             m_aiAdaptSR[2][33];//ZP
#else
  Int             m_aiAdaptSR[2];
#endif
  TComMv          m_cSrchRngLT;
  TComMv          m_cSrchRngRB;

  // RD computation
  TEncSbac***     m_pppcRDSbacCoder;
  TEncSbac*       m_pcRDGoOnSbacCoder;
  DistParam       m_cDistParam;

  // Misc.
  Pel*            m_pTempPel;
  UInt*           m_puiDFilter;
  Int             m_iMaxDeltaQP;


public:
  TEncSearch();
  virtual ~TEncSearch();
  
  Void init(  TEncCfg*      pcEncCfg,
            TComTrQuant*  pcTrQuant,
            Int           iSearchRange,
            Int           iFastSearch,
            Int           iMaxDeltaQP,
            TEncEntropy*  pcEntropyCoder,
            TComRdCost*   pcRdCost,
            TEncSbac***   pppcRDSbacCoder,
            TEncSbac*     pcRDGoOnSbacCoder );
#if wlq_FME
  Void initFME();
  Void DefineThreshold();
  Void AddPicCodedNum(Int PicCodedNum)	{ m_PicCodedNum += PicCodedNum; }
  Void InitPicCodedNum()	{ m_PicCodedNum = -1; }
#endif
  
#if wlq_Mv_check
  Void DecideMvRange();
#endif  

protected:
  
  /// sub-function for motion vector refinement used in fractional-pel accuracy
#if DCTIF
	UInt  xPatternRefinement( TComPattern* pcPatternKey,
		TComMv baseRefMv,
		Int iFrac, TComMv& rcMvFrac );
#else
  UInt  xPatternRefinement( TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, Int iFrac, TComMv& rcMvFrac );
#endif

#if RD_PMVR
	UInt  xPatternRefinementQ(TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, Int iMvX, Int iMvY, TComMv& rcMvFrac);
#endif

  typedef struct
  {
    Pel*  piRefY;
    Int   iYStride;
    Int   iBestX;
    Int   iBestY;
    UInt  uiBestRound;
    UInt  uiBestDistance;
    UInt  uiBestSad;
    UChar ucPointNr;
  } IntTZSearchStruct;
  
  // sub-functions for ME
  __inline Void xTZSearchHelp         ( TComPattern* pcPatternKey, IntTZSearchStruct& rcStruct, const Int iSearchX, const Int iSearchY, const UChar ucPointNr, const UInt uiDistance );
  __inline Void xTZ2PointSearch       ( TComPattern* pcPatternKey, IntTZSearchStruct& rcStrukt, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB );
  __inline Void xTZ8PointSquareSearch ( TComPattern* pcPatternKey, IntTZSearchStruct& rcStrukt, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB, const Int iStartX, const Int iStartY, const Int iDist );
  __inline Void xTZ8PointDiamondSearch( TComPattern* pcPatternKey, IntTZSearchStruct& rcStrukt, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB, const Int iStartX, const Int iStartY, const Int iDist );
  
public:

  /// encoder estimation - intra prediction
  Void predIntraSearch          ( TComDataCU* pcCU, 
                                  TComYuv*    pcOrgYuv, 
                                  TComYuv*    pcPredYuv, 
                                  TComYuv*    pcResiYuv,
                                  TComYuv*    pcRecoYuv );
#if ZHANGYI_INTRA
  Void xUpdateCandList( UInt uiMode, double uiCost, Int uiFullCandNum, Int *CandModeList, double *CandCostList );
  UInt calcHAD4x4(Pel*pi, UInt uiStride, int iWidth, int iHeight);
  UInt calcHADs(Pel *pi, UInt uiStride, int iWidth, int iHeight);
  #if YQH_INTRA_LAMDA
 Void estIntraPredChromaQT(UInt uiPUBestBits, UInt uiPUBestDistortion, UInt uiPartOffset, TComDataCU* pcCU, TComYuv*    pcOrgYuv, TComYuv*    pcPredYuv, TComYuv*    pcResiYuv,TComYuv*    pcRecoYuv, UInt* uiBitsChroma, UInt* uiDistortionChroma, Double* dCostChroma);
#else
  Void estIntraPredChromaQT( UInt uiPartOffset, TComDataCU* pcCU, TComYuv*    pcOrgYuv, TComYuv*    pcPredYuv, TComYuv*    pcResiYuv,TComYuv*    pcRecoYuv, UInt* uiBitsChroma, UInt* uiDistortionChroma, Double* dCostChroma);
 #endif
  UInt xIntraSearchChroma( ComponentID compID, TComDataCU* pcCU, TComYuv* pcOrgYuv, TComYuv* pcPredYuv, TComYuv* pcResiYuv, TComYuv* pcRecoYuv, UInt uiDir, UInt uiPartOffset , Int* p_avail);
#endif

  Void xInitIntraSearch(TComDataCU* pcCU, TComYuv*    pcRecoYuv);

  /// encoder estimation - skip prediction
  Void predSkipSearch           ( TComDataCU* pcCU,
                                  TComYuv*    pcOrgYuv,
                                  TComYuv*&   rpcRecoYuv );

  /// encoder estimation - inter prediction (non-skip)
  Void predInterSearch          ( TComDataCU* pcCU,
                                  TComYuv*    pcOrgYuv,
                                  TComYuv*&   rpcPredYuv );
#if DMH
	Bool checkDMHMode             (TComDataCU* pcCU,
		                             UInt uiAbsPartIdx);

	Char dmhInterPred             (TComDataCU* pcCU,
		                             TComYuv*    pcOrgYuv,
		                             TComYuv*&   rpcPredYuv,
		                             Int         dmh_mode);
#endif
  /// encode residual and compute rd-cost for inter mode
  Void encodeResAndCalcRdInterCU( TComDataCU* pcCU,
                                  TComYuv*    pcYuvOrg,
                                  TComYuv*    pcYuvPred,
                                  TComYuv*&   rpcYuvResi,
                                  TComYuv*&   rpcYuvResiBest,
                                  TComYuv*&   rpcYuvRec,
                                  Bool        bSkipRes );

#if inter_direct_skip_bug
  Void encodeResAndCalcRdInterCU_NO_QT(TComDataCU* pcCU,
	  TComYuv*    pcYuvOrg,
	  TComYuv*    pcYuvPred,
	  TComYuv*&   rpcYuvResi,
	  TComYuv*&   rpcYuvResiBest,
	  TComYuv*&   rpcYuvRec,
	  Bool        bSkipRes);

#endif
#if RD_DIRECT
	Void encodeResAndCalcRdInterCUDirect(TComDataCU* pcCU,
		TComYuv*    pcYuvOrg,
		TComYuv*    pcYuvPred,
		TComYuv*&   rpcYuvResi,
		TComYuv*&   rpcYuvResiBest,
		TComYuv*&   rpcYuvRec);
#endif
  /// set ME search range
#if RPS
  Void setAdaptiveSearchRange(Int iDir, Int iRefIdx, Int iSearchRange) { m_aiAdaptSR[iDir][iRefIdx] = iSearchRange; }
#else

  Void setAdaptiveSearchRange(Int iDir, Int iSearchRange) { m_aiAdaptSR[iDir] = iSearchRange; }
#endif
  
protected:
  
  // -------------------------------------------------------------------------------------------------------------------
  // Inter search
  // -------------------------------------------------------------------------------------------------------------------

  Void xGetBlkBits                ( PartSize ePartSize, Bool bPPicture, Int iPartIdx,  UInt uiLastMode, UInt uiBlkBit[3]);
  // -------------------------------------------------------------------------------------------------------------------
  // motion estimation
  // -------------------------------------------------------------------------------------------------------------------
#ifdef RPS
  Void CalculateSAD(TComDataCU* pcCU, TComYuv* pcYuvOrg, RefPic eRefPic, Int iRefIdxPred, UInt& ruiCost);
#endif

#if PSKIP
  Void scalingMV(TComMv &cMvPredL0, TComMv tempmv, Int curT, Int colT);
#endif

#if BSKIP
  Void scalingDirectMvHor(Int mv_x, Int dist2, Int dist4, Int dist5, TComMv* cMvPredL0, TComMv* cMvPredL1);
  Int scaleMvDirect(Int mv, Int dist_dst, Int dist_src);
  Void scalingDirectMvVer(Int mv_y, Int dist1, Int dist2, Int dist3, Int dist4, Int dist5, TComMv* cMvPredL0, TComMv* cMvPredL1);
#endif
#if rd_bipred
  Void xMotionEstimationBi(TComDataCU*   pcCU,
	  TComYuv*      pcYuvOrg,
	  Int           iPartIdx,
	  RefPic        eRefPic,
	  TComMv*       pcMvPred,
	  TComMv*       pcBwMvPred,
	  Int           iRefIdxPred,
	  TComMv&       rcMv,
	  TComMv&       rcBwMv,
	  UInt&         ruiBits,
	  UInt&         ruiCost,
	  Bool          bBi = false);
#endif

#if rd_sym
  Void xMotionEstimationSym(TComDataCU*   pcCU,
	  TComYuv*      pcYuvOrg,
	  Int           iPartIdx,
	  RefPic        eRefPic,
	  TComMv*       pcMvPred,
	  TComMv*       pcBwMvPred,
	  Int           iRefIdxPred,
	  TComMv&       rcMv,
	  TComMv&       rcBwMv,
	  UInt&         ruiBits,
	  UInt&         ruiBiCost,
	  Bool          bBi = false);

  Int scaleMv(Int val, Int disDst, Int disSrc);
#endif

#if RPS
  Void xMotionEstimation(TComDataCU*   pcCU,
    TComYuv*      pcYuvOrg,
    Int           iPartIdx,
    RefPic    eRefPic,
    TComMv*       pcMvPred,
    Int          iRefIdxPred,
    TComMv&       rcMv,
    UInt&         ruiBits,
    UInt&         ruiCost,
    Bool          bBi = false
#if F_DHP_SYC
	,Bool         bDHP = false
#endif
	);
#else
  Void xMotionEstimation(TComDataCU*   pcCU,
    TComYuv*      pcYuvOrg,
    Int           iPartIdx,
    RefPic    eRefPic,
    TComMv*       pcMvPred,
    TComMv&       rcMv,
    UInt&         ruiBits,
    UInt&         ruiCost,
    Bool          bBi = false);
#endif

#if rd_bipred
  UInt  xPatternRefinementBi(TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Pel* piBwRef, Int iBwRefStride, Int iIntStep, Int iFrac, TComMv& rcMvFrac);
  UInt  xPatternRefinementQBi(TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Pel* piBwRef, Int iBwRefStride, Int iIntStep, Int iMvX, Int iMvY, TComMv& rcMvFrac);
#endif
#if rd_sym
  UInt  xPatternRefinementSym(TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Pel* piBwRef, Int iBwRefStride, Int iIntStep, Int DistBw, Int DistFw, Int iFrac, TComMv& rcMvFrac);
  UInt  xPatternRefinementQSym(TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Pel* piBwRef, Int iBwRefStride, Int iIntStep, Int DistBw, Int DistFw, Int iMvX, Int iMvY, TComMv& rcMvFrac);
#endif

#if wlq_Mv_check
  Char check_mv_range(UInt uiBitSize, Int mv_x, Int mv_y, Int pix_x, Int pix_y, Int blocktype, Int dmh_x, Int dmh_y);
  Char check_mvd(Int mvd_x, Int mvd_y);
#endif

#if !wlq_FME
  Void xTZSearch                  ( TComDataCU*   pcCU,
                                    TComPattern*  pcPatternKey,
                                    Pel*          piRefY,
                                    Int           iRefStride,
                                    TComMv*       pcMvSrchRngLT,
                                    TComMv*       pcMvSrchRngRB,
                                    TComMv&       rcMv,
                                    UInt&         ruiSAD );
#endif
  
  Void xSetSearchRange            ( TComDataCU*   pcCU,
                                    TComMv&       cMvPred,
                                    Int           iSrchRng,
                                    TComMv&       rcMvSrchRngLT,
                                    TComMv&       rcMvSrchRngRB );
  
#if wlq_FME
  Void xSearchOnePixel(Int cand_x, Int center_x, Int cand_y, Int center_y, Pel* piRefY, Int iRefStride, Int pred_x, Int pred_y, UInt   mvshift, Int pic_pix_x, Int pic_pix_y, UInt	BestSad, Bool get_ref_line, Int height, Int width);
  Void xSearchOnePixel2(Int value_iAbort, Int& iAbort, Int cand_x, Int center_x, Int cand_y, Int center_y, Pel* piRefY, Int iRefStride, Int pred_x, Int pred_y, UInt   mvshift, Int pic_pix_x, Int pic_pix_y, UInt	BestSad, Bool get_ref_line, Int height, Int width);
  Void xPreSearchFast			  (TComDataCU* pcCU, Int iRefIdxPred);
#endif

  Void xPatternSearchFast         ( TComDataCU*   pcCU,
                                    TComPattern*  pcPatternKey,
                                    Pel*          piRefY,
                                    Int           iRefStride,
                                    TComMv*       pcMvSrchRngLT,
                                    TComMv*       pcMvSrchRngRB,
                                    TComMv&       rcMv,
#if wlq_FME
									TComMv		  cMvPred,
#endif
                                    UInt&         ruiSAD 
#if wlq_FME
									, Int iRefIdxPred
									, Int iPartIdx
#endif
									);
#if F_DHP_SYC
  Void xPatternSearchForDHP(TComDataCU *pcCU, Int uiPartIdx, RefPic iRefPic, TComMv& rcMv, Int iRefIdxPred, UInt& ruiSAD, TComYuv *pcOrgYuv);
#endif


  Void xPatternSearch             ( TComPattern*  pcPatternKey,
                                    Pel*          piRefY,
                                    Int           iRefStride,
                                    TComMv*       pcMvSrchRngLT,
                                    TComMv*       pcMvSrchRngRB,
                                    TComMv&       rcMv,
#if INTER612
                                    TComMv       cMvPred,
#endif
                                    UInt&         ruiSAD );
#if RD_PMVR
	Void xPatternSearchFracDIF(TComDataCU*   pcCU,
		TComPattern*  pcPatternKey,
		Pel*          piRefY,
		Int           iRefStride,
		Int           iMvX,
		Int           iMvY,
		TComMv*       pcMvInt,
		TComMv&       rcMvHalf,
		TComMv&       rcMvQter,
		UInt&         ruiCost
		);
#else
  Void xPatternSearchFracDIF      ( TComDataCU*   pcCU,
                                    TComPattern*  pcPatternKey,
                                    Pel*          piRefY,
                                    Int           iRefStride,
                                    TComMv*       pcMvInt,
                                    TComMv&       rcMvHalf,
                                    TComMv&       rcMvQter,
                                    UInt&         ruiCost 
                                   );
#endif

#if FME_SUB
  Void xPatternSearchFastFracDIF(TComDataCU*   pcCU,
	  TComPattern*  pcPatternKey,
	  Pel*          piRefY,
	  Int           iRefStride,
	  Int			iPartIdx,
	  Int           iMvX,
	  Int           iMvY,
	  TComMv*       pcMvInt,
	  TComMv&       rcMvHalf,
	  TComMv&       rcMvQter,
	  UInt&         ruiCost
	  );
#endif

#if !niu_CloseFastSubPel
  Void xFastPatternSearchFracDIF	( TComDataCU*   pcCU,
	  TComPattern*  pcPatternKey,
	  Pel*          piRefY,
	  Int           iRefStride,
	  Int			iPartIdx,
	  TComMv		cMvPred,
	  TComMv*       pcMvInt,
	  TComMv&       rcMvHalf,
	  TComMv&       rcMvQter,
	  UInt&         ruiCost
	  );
#endif

#if rd_bipred
  Void xPatternSearchFracDIFBi(TComDataCU*   pcCU,
	  TComPattern*  pcPatternKey,
	  Pel*          piRefY,
	  Int           iRefStride,
	  Pel*          piBwRef,
	  Int           iBwRefStride,
	  Int           iMvX,
	  Int           iMvY,
	  TComMv*       pcMvInt,
	  TComMv&       rcMvHalf,
	  TComMv&       rcMvQter,
	  UInt&         ruiCost
	  );
#endif
#if rd_sym
  Void xPatternSearchFracDIFSym(TComDataCU*   pcCU,
	  TComPattern*  pcPatternKey,
	  Pel*          piRefY,
	  Int           iRefStride,
	  Pel*          piBwRef,
	  Int           iBwRefStride,
	  Int           iMvX,
	  Int           iMvY,
	  TComMv*       pcMvInt,
	  TComMv&       rcMvHalf,
	  TComMv&       rcMvQter,
	  UInt&         ruiCost
	  );
#endif
#if RD_PMVR
	Int   pmvrAdaptMv(Int *piCandMvX, Int *piCandMvY, Int iCtrX, Int iCtrY, Int iMvX, Int iMvY, Int iPos);
#endif

#if FME_SUB
	Int pmvrAdaptMv2(Int *piCandMvX, Int *piCandMvY, Int iCtrX, Int iCtrY, Int iXMinNow, Int iYMinNow, Int stepX, Int stepY);
#endif

#if DCTIF
	Void xExtDIFUpSamplingH( TComPattern* pcPattern, Bool biPred  );
	Void xExtDIFUpSamplingQ( TComPattern* pcPatternKey, TComMv halfPelRef, Bool biPred );
#else
  Void xExtDIFUpSamplingH         ( TComPattern*  pcPattern, TComYuv* pcYuvExt  );
  
  Void xExtDIFUpSamplingQ         ( TComPattern* pcPatternKey,
                                    Pel*          piDst,
                                    Int           iDstStride,
                                    Pel*          piSrcPel,
                                    Int           iSrcPelStride,
                                    Int*          piSrc,
                                    Int           iSrcStride,
                                    UInt          uiFilter  );
#endif
  // -------------------------------------------------------------------------------------------------------------------
  // T & Q & Q-1 & T-1
  // -------------------------------------------------------------------------------------------------------------------
  Void xRecurTransformNxN( TComDataCU* rpcCU, UInt uiAbsPartIdx, Pel* pcResidual, UInt uiAddr, UInt uiStride, UInt uiLog2Size, UInt uiMaxTrMode, UInt uiTrMode, TCoeff*& rpcCoeff, TextType eType );
  Void xEncodeInterTexture ( TComDataCU*& rpcCU, TComYuv*& rpcYuv, UInt uiTrMode );
  Void xAddSymbolBitsInter( TComDataCU* pcCU, UInt& ruiBits, Bool bSkipRes );

#if niu_ChromaRDO_revise
  Void xAddSymbolBitsIntra_coeff(ComponentID compID, TComDataCU* pcCU, UInt uiPU, UInt uiQNumPart, UInt uiPartDepth, UInt uiMaxTrDepth, UInt uiTrDepth, UInt uiLog2Size, UInt& ruiBits, Int iIntraDirValue);
#endif

#if ZHANGYI_INTRA
  UInt xIntraSearch( TComDataCU* pcCU, TComYuv* pcOrgYuv, TComYuv* pcPredYuv, TComYuv* pcResiYuv, TComYuv* pcRecoYuv, UInt uiDir, UInt uiPU, int* p_avail );
  Void xAddSymbolBitsIntra(ComponentID compID, TComDataCU* pcCU, UInt uiPU, UInt uiQNumPart, UInt uiPartDepth, UInt uiMaxTrDepth, UInt uiTrDepth, UInt uiLog2Size, UInt& ruiBits, Int iIntraDirValue);
#else
  UInt xIntraSearch( TComDataCU* pcCU, TComYuv* pcOrgYuv, TComYuv* pcPredYuv, TComYuv* pcResiYuv, TComYuv* pcRecoYuv, UInt uiDir, UInt uiPU );
  Void xAddSymbolBitsIntra( TComDataCU* pcCU, UInt uiPU, UInt uiQNumPart, UInt uiPartDepth, UInt uiMaxTrDepth, UInt uiTrDepth, UInt uiLog2Size, UInt& ruiBits );
#endif

#if DCTIF
	inline  Void  setDistParamComp(UInt uiComp)  { m_cDistParam.uiComp = uiComp; }
#endif
#if INTER614
	Void SetRef(TComDataCU* pcCU, Int *adjustRef, Int *maxRef);
#endif
};// END CLASS DEFINITION TEncSearch


#endif // __TENCSEARCH__

