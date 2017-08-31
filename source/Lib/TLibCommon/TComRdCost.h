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

/** \file     TComRdCost.h
    \brief    RD cost computation classes (header)
*/

#ifndef __TCOMRDCOST__
#define __TCOMRDCOST__


#include "CommonDef.h"
#include "TComPattern.h"
#include "TComMv.h"

class DistParam;
class TComPattern;

// ====================================================================================================================
// Type definition
// ====================================================================================================================

// for function pointer
typedef UInt (*FpDistFunc) (DistParam*);

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// distortion parameter class
class DistParam
{
public:
  Pel*  pOrg;
  Pel*  pCur;
#if rd_bipred
  Pel*  pBwCur;
  Bool  biPred;
  Int   iStrideBwCur;
#endif
  Int   iStrideOrg;
  Int   iStrideCur;

  Int   iRows;
  Int   iCols;

  Int   iStep;
  FpDistFunc DistFunc;
#if wlq_FME
  UInt BestSad;
  Bool key;     //与对比最佳Sad
  Bool get_ref_line;
  Int cand_x, cand_y;
  Int height, width;
  Int RefStride;
#endif
#if DCTIF || chazhi_bug
	Int   bitDepth;
	UInt  uiComp; // uiComp = 0 (luma Y), 1 (chroma U), 2 (chroma V)
#endif

#if FULLSEARCHFAST
  UInt             m_uiMvCost;
  UInt             m_uiSadBest;

#endif
  // (vertical) subsampling shift (for reducing complexity)
  // - 0 = no subsampling, 1 = even rows, 2 = every 4th, etc.
  Int   iSubShift;
  
  DistParam()
  {
    pOrg = NULL;
    pCur = NULL;
#if rd_bipred
	pBwCur = NULL;
	biPred = false;
	iStrideBwCur = 0;
#endif
    iStrideOrg = 0;
    iStrideCur = 0;

    iRows = 0;
    iCols = 0;

    iStep = 1;
    DistFunc = NULL;
    iSubShift = 0;
#if wlq_FME
	BestSad = MAX_UINT;
	key = false;
	get_ref_line = false;
	cand_x = 0;
	cand_y = 0;
	height = 0;
	width = 0;
#endif
#if FULLSEARCHFAST
    m_uiSadBest = MAX_UINT;
    m_uiMvCost =0;
#endif
  }
};

/// RD cost computation class
class TComRdCost
{
private:
  // for distortion
  Int                     m_iBlkWidth;
  Int                     m_iBlkHeight;
  
  FpDistFunc              m_afpDistortFunc[36]; // [eDFunc]
  
  Double                  m_dLambda;
  UInt                    m_uiLambdaMotionSAD;
  UInt                    m_uiLambdaMotionSSE;
  Double                  m_dFrameLambda;
  Double                  m_dChromaWeight;
#if LAMBDA
  Double                  m_dLambda_motion;
  Double                  m_dlambda_motion_factor;
#endif
  // for motion cost
  UInt*                   m_puiComponentCostOriginP;
  UInt*                   m_puiComponentCost;
  UInt*                   m_puiVerCost;
  UInt*                   m_puiHorCost;
  UInt                    m_uiCost;
  Int                     m_iCostScale;
  Int                     m_iSearchLimit;
#if INTER614
  UInt*                   m_puiRefCost;
#endif
public:
  TComRdCost();
  virtual ~TComRdCost();

#if wlq_FME
  Int*                    m_mvbits;
  Int*                    m_mvbitsbuf;
#endif

  Double  calcRdCost  ( UInt   uiBits, UInt   uiDistortion, Bool bFlag = false, DFunc eDFunc = DF_DEFAULT );
  Double  calcRdCost64( UInt64 uiBits, UInt64 uiDistortion, Bool bFlag = false, DFunc eDFunc = DF_DEFAULT );

  Void    setLambda      ( Double dLambda );
  Void    setFrameLambda ( Double dLambda ) { m_dFrameLambda = dLambda; }
  Void    setChromaWeight( Double dWeight ) { m_dChromaWeight = dWeight; }
  Double  getChromaWeight() { return m_dChromaWeight; }
#if LAMBDA
  Double getLambdaMotion(){ return m_dLambda_motion; }
  Double getLambdaMotionFactor() { return m_dlambda_motion_factor; }
#endif

  // Distortion Functions
  Void    init();
#if wlq_FME
  Void initMVbits(Int MaxMvBits);
#endif
  Void    setDistParam( UInt uiBlkWidth, UInt uiBlkHeight, DFunc eDFunc, DistParam& rcDistParam );
  Void    setDistParam( TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride,            DistParam& rcDistParam );
  Void    setDistParam( TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, Int iStep, DistParam& rcDistParam, Bool bHADME=false );
#if rd_bipred
  Void    setDistParam(TComPattern* pcPatternKey, Pel* piRefY, Pel* piBwRefY, Int iRefStride, Int iStrideBwCur, Int iStep, DistParam& rcDistParam, Bool bHADME = false);
#endif
  UInt    calcHAD     ( Pel* pi0, Int iStride0, Pel* pi1, Int iStride1, Int iWidth, Int iHeight );
  
  // for motion cost
#if INTER614
  Void    initRateDistortionModel(Int iSubPelSearchLimit, Int iMaxRefBits);
#else
  Void    initRateDistortionModel(Int iSubPelSearchLimit);
#endif
  Void    xUninit();
  UInt    xGetComponentBits( Int iVal );
  Void    getMotionCost( Bool bSad, Int iAdd ) { m_uiCost = (bSad ? m_uiLambdaMotionSAD + iAdd : m_uiLambdaMotionSSE + iAdd); }
  Void    setPredictor( TComMv& rcMv )
  {
    m_puiHorCost = m_puiComponentCost - rcMv.getHor();
    m_puiVerCost = m_puiComponentCost - rcMv.getVer();
  }
  Void    setCostScale( Int iCostScale )    { m_iCostScale = iCostScale; }

  __inline UInt getCost(Int x, Int y)
  {
    return ((m_uiCost * (m_puiHorCost[x * (1 << m_iCostScale)] + m_puiVerCost[y * (1 << m_iCostScale)])) >> 16);
  }
#if INTER614
  __inline UInt getRefCost(Int RefIdx)
  {
	  return (Int (m_dlambda_motion_factor) * m_puiRefCost[RefIdx] >> 16);
  }
#endif

#if wlq_FME
  __inline UInt getCostFME(Int x, Int y)
  {
	  return ((m_uiCost * (m_mvbits[x] + m_mvbits[y])) >> 16);
  }
#endif
  UInt    getCost( UInt b )                 { return ( m_uiCost * b ) >> 16; }
  UInt    getBits( Int x, Int y )           { return m_puiHorCost[ x * (1<<m_iCostScale)] + m_puiVerCost[ y * (1<<m_iCostScale) ]; }
  
private:

  static UInt xGetSSE           ( DistParam* pcDtParam );
  static UInt xGetSSE4          ( DistParam* pcDtParam );
  static UInt xGetSSE8          ( DistParam* pcDtParam );
  static UInt xGetSSE16         ( DistParam* pcDtParam );
  static UInt xGetSSE32         ( DistParam* pcDtParam );
  static UInt xGetSSE64         ( DistParam* pcDtParam );
  static UInt xGetSSE16N        ( DistParam* pcDtParam );
  
  static UInt xGetSAD           ( DistParam* pcDtParam );
  static UInt xGetSAD4          ( DistParam* pcDtParam );
  static UInt xGetSAD8          ( DistParam* pcDtParam );
  static UInt xGetSAD16         ( DistParam* pcDtParam );
  static UInt xGetSAD32         ( DistParam* pcDtParam );
  static UInt xGetSAD64         ( DistParam* pcDtParam );
  static UInt xGetSAD16N        ( DistParam* pcDtParam );
  
  static UInt xGetSADs          ( DistParam* pcDtParam );
  static UInt xGetSADs4         ( DistParam* pcDtParam );
  static UInt xGetSADs8         ( DistParam* pcDtParam );
  static UInt xGetSADs16        ( DistParam* pcDtParam );
  static UInt xGetSADs32        ( DistParam* pcDtParam );
  static UInt xGetSADs64        ( DistParam* pcDtParam );
  static UInt xGetSADs16N       ( DistParam* pcDtParam );
  
  static UInt xGetHADs4         ( DistParam* pcDtParam );
  static UInt xGetHADs8         ( DistParam* pcDtParam );
  static UInt xGetHADs          ( DistParam* pcDtParam );
  static UInt xCalcHADs2x2      ( Pel *piOrg, Pel *piCurr, Int iStrideOrg, Int iStrideCur, Int iStep );
  static UInt xCalcHADs4x4      ( Pel *piOrg, Pel *piCurr, Int iStrideOrg, Int iStrideCur, Int iStep );
  static UInt xCalcHADs8x8      ( Pel *piOrg, Pel *piCurr, Int iStrideOrg, Int iStrideCur, Int iStep );
#if rd_bipred
  static UInt xCalcHADs2x2Bi(Pel *piOrg, Pel *piCurr, Pel* piBwCurr, Int iStrideOrg, Int iStrideCur, Int iStrideBwCur, Int iStep);
  static UInt xCalcHADs4x4Bi(Pel *piOrg, Pel *piCurr, Pel* piBwCurr, Int iStrideOrg, Int iStrideCur, Int iStrideBwCur, Int iStep);
  static UInt xCalcHADs8x8Bi(Pel *piOrg, Pel *piCurr, Pel* piBwCurr, Int iStrideOrg, Int iStrideCur, Int iStrideBwCur, Int iStep);
#endif


#if wlq_FME
  static Pel*   expand_ref_line(Int blocksize_x, Pel* ref_pic, Int y, Int x, Int height, Int width, Int iRefStride);
#endif

public:
  UInt   getDistPart( Pel* piCur, Int iCurStride,  Pel* piOrg, Int iOrgStride, UInt uiBlkWidth, UInt uiBlkHeight, DFunc eDFunc = DF_SSE );
};// END CLASS DEFINITION TComRdCost


#endif // __TCOMRDCOST__

