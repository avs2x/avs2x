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

/** \file     TEncSearch.cpp
 \brief    encoder search class
 */

#include <limits.h>

#include "../TLibCommon/TypeDef.h"
#include "../TLibCommon/TComMotionInfo.h"
#include "TEncSearch.h"
 extern TCoeff*   m_pcQTTempCoeffY1;
static TComMv s_acMvRefineH[9] =
{
  TComMv(  0,  0 ), // 0
  TComMv(  0, -1 ), // 1
  TComMv(  0,  1 ), // 2
  TComMv( -1,  0 ), // 3
  TComMv(  1,  0 ), // 4
  TComMv( -1, -1 ), // 5
  TComMv(  1, -1 ), // 6
  TComMv( -1,  1 ), // 7
  TComMv(  1,  1 )  // 8
};

static TComMv s_acMvRefineQ[9] =
{
  TComMv(  0,  0 ), // 0
  TComMv(  0, -1 ), // 1
  TComMv(  0,  1 ), // 2
  TComMv( -1, -1 ), // 5
  TComMv(  1, -1 ), // 6
  TComMv( -1,  0 ), // 3
  TComMv(  1,  0 ), // 4
  TComMv( -1,  1 ), // 7
  TComMv(  1,  1 )  // 8
};

static UInt s_auiDFilter[9] =
{
  0, 1, 0,
  2, 3, 2,
  0, 1, 0
};

TEncSearch::TEncSearch()
{
  m_pcQTTempCoeffY    = NULL;
  m_pcQTTempCoeffCb   = NULL;
  m_pcQTTempCoeffCr   = NULL;
  m_puhQTTempTrIdx    = NULL;
  m_puhQTTempCbf[0]   = m_puhQTTempCbf[1] = m_puhQTTempCbf[2] = NULL;
  
  m_pcEncCfg        = NULL;
  m_pcEntropyCoder  = NULL;
  m_pTempPel        = NULL;
#if wlq_FME
  m_pred_SAD_space = 0;
  m_pred_SAD_ref   = 0;
  m_pred_SAD_uplayer = 0;
  m_pred_MV_time[2]  = {0};
  m_pred_MV_ref[2]   = {0};
  m_pred_MV_uplayer[2] = {0};
#endif
}

TEncSearch::~TEncSearch()
{
	if ( m_pTempPel )
  {
    delete [] m_pTempPel;
    m_pTempPel = NULL;
  }
  
  delete[] m_pcQTTempCoeffY;
  delete[] m_pcQTTempCoeffCb;
  delete[] m_pcQTTempCoeffCr;
  delete[] m_puhQTTempTrIdx;
  delete[] m_puhQTTempCbf[0];
  delete[] m_puhQTTempCbf[1];
  delete[] m_puhQTTempCbf[2];
#if wlq_FME
  if (m_pcEncCfg->getFME())
  {
	  Int search_range = m_pcEncCfg->getSearchRange();
	  for (Int i = 0; i < (2 * search_range + 1); i++)
	  {
		  if (m_McostState[i])
			  delete[] m_McostState[i];
	  }
	  if (m_McostState)
		  delete[] m_McostState;

	  if (m_Bsize)
		  delete[] m_Bsize;
	  if (m_AlphaSec)
		  delete[] m_AlphaSec;
	  if (m_AlphaThird)
		  delete[] m_AlphaThird;

	  if (m_spiral_search_x)
		  delete[] m_spiral_search_x;
	  if (m_spiral_search_y)
		  delete[] m_spiral_search_y;
  }
#endif
}

void TEncSearch::init(  TEncCfg*      pcEncCfg,
                        TComTrQuant*  pcTrQuant,
                        Int           iSearchRange,
                        Int           iFastSearch,
                        Int           iMaxDeltaQP,
                        TEncEntropy*  pcEntropyCoder,
                        TComRdCost*   pcRdCost,
                        TEncSbac*** pppcRDSbacCoder,
                        TEncSbac*   pcRDGoOnSbacCoder
                     )
{
  m_pcEncCfg             = pcEncCfg;
  m_pcTrQuant            = pcTrQuant;
  m_iSearchRange         = iSearchRange;
  m_iFastSearch          = iFastSearch;
  m_iMaxDeltaQP          = iMaxDeltaQP;
  m_pcEntropyCoder       = pcEntropyCoder;
  m_pcRdCost             = pcRdCost;
  
  m_pppcRDSbacCoder     = pppcRDSbacCoder;
  m_pcRDGoOnSbacCoder   = pcRDGoOnSbacCoder;
  
#if RPS
  for (UInt iDir = 0; iDir < (UInt)NUM_REF_PIC_LIST_01; iDir++)
  {
    for (UInt iRefIdx = 0; iRefIdx < 33; iRefIdx++)   //33 最大参考帧数
    {
      m_aiAdaptSR[iDir][iRefIdx] = iSearchRange;
    }
  }
#else
  for (Int iDir = 0; iDir < 2; iDir++)
  {
    m_aiAdaptSR[iDir] = iSearchRange;
  }
#endif
#if INTER614
  Int maxRefBit = 1 + 2 * (int)floor(log(max(16, m_pcEncCfg->getNumberReferenceFrames() + 1)) / log(2) + 1e-10);
#endif
  m_puiDFilter = s_auiDFilter + 4;
  
  // initialize motion cost
#if INTER614
#if B_RPS
  m_pcRdCost->initRateDistortionModel(m_iSearchRange << 2, maxRefBit);
#else
  Void    initRateDistortionModel(Int iSubPelSearchLimit, Int iMaxRefBits);
#endif

#else
  m_pcRdCost->initRateDistortionModel(m_iSearchRange << 2);
#endif
  initTempBuff();
  
#if YQH_BUFF_RELEASE
#if wlq_mem_bug
  m_pTempPel = new  Pel[1 << (MAX_CU_DEPTH_Pel<< 1)];
#else
    m_pTempPel = new  Pel[1<<(MAX_CU_DEPTH<<1)];
#endif
 
  m_pcQTTempCoeffY   = new TCoeff [1<<(MAX_CU_DEPTH<<1)   ];
  m_pcQTTempCoeffCb  = new TCoeff [(1<<(MAX_CU_DEPTH<<1))>>2];
  m_pcQTTempCoeffCr  = new TCoeff [(1<<(MAX_CU_DEPTH<<1))>>2];
#else
  m_pTempPel = new  Pel[1<<(g_uiLog2MaxCUSize<<1)];
 
  m_pcQTTempCoeffY   = new TCoeff [1<<(g_uiLog2MaxCUSize<<1)   ];
  m_pcQTTempCoeffCb  = new TCoeff [(1<<(g_uiLog2MaxCUSize<<1))>>2];
  m_pcQTTempCoeffCr  = new TCoeff [(1<<(g_uiLog2MaxCUSize<<1))>>2];
#endif  
  const UInt uiNumPartitions = 1<<(g_uiMaxCUDepth<<1);
  m_puhQTTempTrIdx   = new UChar  [uiNumPartitions];
  m_puhQTTempCbf[0]  = new UChar  [uiNumPartitions];
  m_puhQTTempCbf[1]  = new UChar  [uiNumPartitions];
  m_puhQTTempCbf[2]  = new UChar  [uiNumPartitions];

#if wlq_FME
  InitPicCodedNum();
  if (m_pcEncCfg->getFME())
  {
	  Int search_range = m_pcEncCfg->getSearchRange();
	  m_McostState = new Int*[2 * search_range + 1];
	  for (Int i = 0; i < (2 * search_range + 1); i++)
		  m_McostState[i] = new Int[2 * search_range + 1];

	  m_Bsize = new Double[9];
	  m_AlphaSec = new Double[9];
	  m_AlphaThird = new Double[9];
	  DefineThreshold();

	  Int max_search_points = (2 * search_range + 1) * (2 * search_range + 1);
	  m_spiral_search_x = new Int[max_search_points];
	  m_spiral_search_y = new Int[max_search_points];

	  //--- init array: search pattern ---
	  Int i, k, l;
	  m_spiral_search_x[0] = m_spiral_search_y[0] = 0;
	  for (k = 1, l = 1; l <= max(1, search_range); l++)
	  {
		  for (i = -l + 1; i < l; i++)
		  {
			  m_spiral_search_x[k] = i;
			  m_spiral_search_y[k++] = -l;
			  m_spiral_search_x[k] = i;
			  m_spiral_search_y[k++] = l;
		  }

		  for (i = -l; i <= l; i++)
		  {
			  m_spiral_search_x[k] = -l;
			  m_spiral_search_y[k++] = i;
			  m_spiral_search_x[k] = l;
			  m_spiral_search_y[k++] = i;
		  }
	  }
  }
#endif

#if wlq_Mv_check
  DecideMvRange();
#endif
}

#if wlq_FME
Void TEncSearch::initFME()
{
	m_BestX = 0;
	m_BestY = 0;
	m_BestSad = MAX_INT;
	Int search_range = m_pcEncCfg->getSearchRange();
	for (Int i = 0; i < (2 * search_range + 1); i++)
		memset(m_McostState[i], 0, sizeof(Int)*(2 * search_range + 1));
}
Void TEncSearch::DefineThreshold()
{
	m_AlphaSec[1] = 0.01f;
	m_AlphaSec[2] = 0.01f;
	m_AlphaSec[3] = 0.01f;
	m_AlphaSec[4] = 0.02f;
	m_AlphaSec[5] = 0.03f;
	m_AlphaSec[6] = 0.03f;
	m_AlphaSec[7] = 0.04f;
	m_AlphaSec[8] = 0.05f;
	m_AlphaThird[1] = 0.06f;
	m_AlphaThird[2] = 0.07f;
	m_AlphaThird[3] = 0.07f;
	m_AlphaThird[4] = 0.08f;
	m_AlphaThird[5] = 0.12f;
	m_AlphaThird[6] = 0.11f;
	m_AlphaThird[7] = 0.15f;
	m_AlphaThird[8] = 0.16f;
	
	Int qpP = m_pcEncCfg->getQPPFrame();
	Int gb_qp_per = (qpP - MIN_QP) / 6;
	Int gb_qp_rem = (qpP - MIN_QP) % 6;

	Int gb_q_bits = 15 + gb_qp_per;
	Int gb_qp_const, Thresh4x4;
#if wlq_FME_q
	if (1) {
		gb_qp_const = (1 << gb_q_bits) / 3;    // intra
	}
	else {
		gb_qp_const = (1 << gb_q_bits) / 6;    // inter
	}
#endif
	Thresh4x4 = ((1 << gb_q_bits) - gb_qp_const) / quant_coef_AVS2[gb_qp_rem][0][0];
	Double Quantize_step = Thresh4x4 / (4 * 5.61f);
	m_Bsize[8] = (16 * 16) * Quantize_step;
	m_Bsize[7] = (16 * 16) * Quantize_step;
	m_Bsize[6] = m_Bsize[7] * 4;
	m_Bsize[5] = m_Bsize[7] * 4;
	m_Bsize[4] = m_Bsize[5] * 4;
	m_Bsize[3] = m_Bsize[4] * 4;
	m_Bsize[2] = m_Bsize[4] * 4;
	m_Bsize[1] = m_Bsize[2] * 4;
}

#endif

#if wlq_Mv_check
Void TEncSearch::DecideMvRange()
{
	Int profile_id = m_pcEncCfg->getProfileId();
	Int level_id = m_pcEncCfg->getLevelId();
	if (profile_id == BASELINE_PROFILE_AVS2 || profile_id == BASELINE10_PROFILE_AVS2)
	{
		switch (level_id)
		{
		case 0x10:
			Min_V_MV = -512;
			Max_V_MV = 511;
			Min_H_MV = -8192;
			Max_H_MV = 8191;
			break;
		case 0x20:
			Min_V_MV = -1024;
			Max_V_MV = 1023;
			Min_H_MV = -8192;
			Max_H_MV = 8191;
			break;
		case 0x22:
			Min_V_MV = -1024;
			Max_V_MV = 1023;
			Min_H_MV = -8192;
			Max_H_MV = 8191;
			break;
		case 0x40:
			Min_V_MV = -2048;
			Max_V_MV = 2047;
			Min_H_MV = -8192;
			Max_H_MV = 8191;
			break;
		case 0x42:
			Min_V_MV = -2048;
			Max_V_MV = 2047;
			Min_H_MV = -8192;
			Max_H_MV = 8191;
			break;
		}
	}
}
#endif

#if FASTME_SMOOTHER_MV
#define FIRSTSEARCHSTOP     1
#else
#define FIRSTSEARCHSTOP     0
#endif

#define TZ_SEARCH_CONFIGURATION                                                                                 \
const Int  iRaster                  = 3;  /* TZ soll von aussen ?ergeben werden */                            \
const Bool bTestZeroVector          = 1;                                                                      \
const Bool bFirstSearchDiamond      = 1;  /* 1 = xTZ8PointDiamondSearch   0 = xTZ8PointSquareSearch */        \
const Bool bFirstSearchStop         = FIRSTSEARCHSTOP;                                                        \
const UInt uiFirstSearchRounds      = 3;  /* first search stop X rounds after best match (must be >=1) */     \
const Bool bEnableRasterSearch      = 1;                                                                      \
const Bool bAlwaysRasterSearch      = 0;  /* ===== 1: BETTER but factor 2 slower ===== */                     \
const Bool bRasterRefinementDiamond = 0;  /* 1 = xTZ8PointDiamondSearch   0 = xTZ8PointSquareSearch */        \
const Bool bStarRefinementDiamond   = 1;  /* 1 = xTZ8PointDiamondSearch   0 = xTZ8PointSquareSearch */        \
const Bool bStarRefinementStop      = 0;                                                                      \
const UInt uiStarRefinementRounds   = 2;  /* star refinement stop X rounds after best match (must be >=1) */  \


__inline Void TEncSearch::xTZSearchHelp( TComPattern* pcPatternKey, IntTZSearchStruct& rcStruct, const Int iSearchX, const Int iSearchY, const UChar ucPointNr, const UInt uiDistance )
{
  UInt  uiSad;
  
  Pel*  piRefSrch;
  
  piRefSrch = rcStruct.piRefY + iSearchY * rcStruct.iYStride + iSearchX;
  
  //-- jclee for using the SAD function pointer
  m_pcRdCost->setDistParam( pcPatternKey, piRefSrch, rcStruct.iYStride,  m_cDistParam );
  
  // fast encoder decision: use subsampled SAD when rows > 8 for integer ME
  if ( m_pcEncCfg->getUseFastEnc() )
  {
    if ( m_cDistParam.iRows > 8 )
    {
      m_cDistParam.iSubShift = 1;
    }
  }
  
  // distortion
  uiSad = m_cDistParam.DistFunc( &m_cDistParam );
  
  // motion cost
  uiSad += m_pcRdCost->getCost( iSearchX, iSearchY );
  
  if( uiSad < rcStruct.uiBestSad )
  {
    rcStruct.uiBestSad      = uiSad;
    rcStruct.iBestX         = iSearchX;
    rcStruct.iBestY         = iSearchY;
    rcStruct.uiBestDistance = uiDistance;
    rcStruct.uiBestRound    = 0;
    rcStruct.ucPointNr      = ucPointNr;
  }
}

__inline Void TEncSearch::xTZ2PointSearch( TComPattern* pcPatternKey, IntTZSearchStruct& rcStruct, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB )
{
  Int   iSrchRngHorLeft   = pcMvSrchRngLT->getHor();
  Int   iSrchRngHorRight  = pcMvSrchRngRB->getHor();
  Int   iSrchRngVerTop    = pcMvSrchRngLT->getVer();
  Int   iSrchRngVerBottom = pcMvSrchRngRB->getVer();
  
  // 2 point search,                   //   1 2 3
  // check only the 2 untested points  //   4 0 5
  // around the start point            //   6 7 8
  Int iStartX = rcStruct.iBestX;
  Int iStartY = rcStruct.iBestY;
  switch( rcStruct.ucPointNr )
  {
    case 1:
    {
      if ( (iStartX - 1) >= iSrchRngHorLeft )
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX - 1, iStartY, 0, 2 );
      }
      if ( (iStartY - 1) >= iSrchRngVerTop )
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iStartY - 1, 0, 2 );
      }
    }
      break;
    case 2:
    {
      if ( (iStartY - 1) >= iSrchRngVerTop )
      {
        if ( (iStartX - 1) >= iSrchRngHorLeft )
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX - 1, iStartY - 1, 0, 2 );
        }
        if ( (iStartX + 1) <= iSrchRngHorRight )
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX + 1, iStartY - 1, 0, 2 );
        }
      }
    }
      break;
    case 3:
    {
      if ( (iStartY - 1) >= iSrchRngVerTop )
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iStartY - 1, 0, 2 );
      }
      if ( (iStartX + 1) <= iSrchRngHorRight )
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX + 1, iStartY, 0, 2 );
      }
    }
      break;
    case 4:
    {
      if ( (iStartX - 1) >= iSrchRngHorLeft )
      {
        if ( (iStartY + 1) <= iSrchRngVerBottom )
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX - 1, iStartY + 1, 0, 2 );
        }
        if ( (iStartY - 1) >= iSrchRngVerTop )
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX - 1, iStartY - 1, 0, 2 );
        }
      }
    }
      break;
    case 5:
    {
      if ( (iStartX + 1) <= iSrchRngHorRight )
      {
        if ( (iStartY - 1) >= iSrchRngVerTop )
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX + 1, iStartY - 1, 0, 2 );
        }
        if ( (iStartY + 1) <= iSrchRngVerBottom )
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX + 1, iStartY + 1, 0, 2 );
        }
      }
    }
      break;
    case 6:
    {
      if ( (iStartX - 1) >= iSrchRngHorLeft )
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX - 1, iStartY , 0, 2 );
      }
      if ( (iStartY + 1) <= iSrchRngVerBottom )
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iStartY + 1, 0, 2 );
      }
    }
      break;
    case 7:
    {
      if ( (iStartY + 1) <= iSrchRngVerBottom )
      {
        if ( (iStartX - 1) >= iSrchRngHorLeft )
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX - 1, iStartY + 1, 0, 2 );
        }
        if ( (iStartX + 1) <= iSrchRngHorRight )
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX + 1, iStartY + 1, 0, 2 );
        }
      }
    }
      break;
    case 8:
    {
      if ( (iStartX + 1) <= iSrchRngHorRight )
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX + 1, iStartY, 0, 2 );
      }
      if ( (iStartY + 1) <= iSrchRngVerBottom )
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iStartY + 1, 0, 2 );
      }
    }
      break;
    default:
    {
      assert( false );
    }
      break;
  } // switch( rcStruct.ucPointNr )
}

__inline Void TEncSearch::xTZ8PointSquareSearch( TComPattern* pcPatternKey, IntTZSearchStruct& rcStruct, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB, const Int iStartX, const Int iStartY, const Int iDist )
{
  Int   iSrchRngHorLeft   = pcMvSrchRngLT->getHor();
  Int   iSrchRngHorRight  = pcMvSrchRngRB->getHor();
  Int   iSrchRngVerTop    = pcMvSrchRngLT->getVer();
  Int   iSrchRngVerBottom = pcMvSrchRngRB->getVer();
  
  // 8 point search,                   //   1 2 3
  // search around the start point     //   4 0 5
  // with the required  distance       //   6 7 8
  assert( iDist != 0 );
  const Int iTop        = iStartY - iDist;
  const Int iBottom     = iStartY + iDist;
  const Int iLeft       = iStartX - iDist;
  const Int iRight      = iStartX + iDist;
  rcStruct.uiBestRound += 1;
  
  if ( iTop >= iSrchRngVerTop ) // check top
  {
    if ( iLeft >= iSrchRngHorLeft ) // check top left
    {
      xTZSearchHelp( pcPatternKey, rcStruct, iLeft, iTop, 1, iDist );
    }
    // top middle
    xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iTop, 2, iDist );
    
    if ( iRight <= iSrchRngHorRight ) // check top right
    {
      xTZSearchHelp( pcPatternKey, rcStruct, iRight, iTop, 3, iDist );
    }
  } // check top
  if ( iLeft >= iSrchRngHorLeft ) // check middle left
  {
    xTZSearchHelp( pcPatternKey, rcStruct, iLeft, iStartY, 4, iDist );
  }
  if ( iRight <= iSrchRngHorRight ) // check middle right
  {
    xTZSearchHelp( pcPatternKey, rcStruct, iRight, iStartY, 5, iDist );
  }
  if ( iBottom <= iSrchRngVerBottom ) // check bottom
  {
    if ( iLeft >= iSrchRngHorLeft ) // check bottom left
    {
      xTZSearchHelp( pcPatternKey, rcStruct, iLeft, iBottom, 6, iDist );
    }
    // check bottom middle
    xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iBottom, 7, iDist );
    
    if ( iRight <= iSrchRngHorRight ) // check bottom right
    {
      xTZSearchHelp( pcPatternKey, rcStruct, iRight, iBottom, 8, iDist );
    }
  } // check bottom
}

__inline Void TEncSearch::xTZ8PointDiamondSearch( TComPattern* pcPatternKey, IntTZSearchStruct& rcStruct, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB, const Int iStartX, const Int iStartY, const Int iDist )
{
  Int   iSrchRngHorLeft   = pcMvSrchRngLT->getHor();
  Int   iSrchRngHorRight  = pcMvSrchRngRB->getHor();
  Int   iSrchRngVerTop    = pcMvSrchRngLT->getVer();
  Int   iSrchRngVerBottom = pcMvSrchRngRB->getVer();
  
  // 8 point search,                   //   1 2 3
  // search around the start point     //   4 0 5
  // with the required  distance       //   6 7 8
  assert ( iDist != 0 );
  const Int iTop        = iStartY - iDist;
  const Int iBottom     = iStartY + iDist;
  const Int iLeft       = iStartX - iDist;
  const Int iRight      = iStartX + iDist;
  rcStruct.uiBestRound += 1;
  
  if ( iDist == 1 ) // iDist == 1
  {
    if ( iTop >= iSrchRngVerTop ) // check top
    {
      xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iTop, 2, iDist );
    }
    if ( iLeft >= iSrchRngHorLeft ) // check middle left
    {
      xTZSearchHelp( pcPatternKey, rcStruct, iLeft, iStartY, 4, iDist );
    }
    if ( iRight <= iSrchRngHorRight ) // check middle right
    {
      xTZSearchHelp( pcPatternKey, rcStruct, iRight, iStartY, 5, iDist );
    }
    if ( iBottom <= iSrchRngVerBottom ) // check bottom
    {
      xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iBottom, 7, iDist );
    }
  }
  else // if (iDist != 1)
  {
    if ( iDist <= 8 )
    {
      const Int iTop_2      = iStartY - (iDist>>1);
      const Int iBottom_2   = iStartY + (iDist>>1);
      const Int iLeft_2     = iStartX - (iDist>>1);
      const Int iRight_2    = iStartX + (iDist>>1);
      
      if (  iTop >= iSrchRngVerTop && iLeft >= iSrchRngHorLeft &&
          iRight <= iSrchRngHorRight && iBottom <= iSrchRngVerBottom ) // check border
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX,  iTop,      2, iDist    );
        xTZSearchHelp( pcPatternKey, rcStruct, iLeft_2,  iTop_2,    1, iDist>>1 );
        xTZSearchHelp( pcPatternKey, rcStruct, iRight_2, iTop_2,    3, iDist>>1 );
        xTZSearchHelp( pcPatternKey, rcStruct, iLeft,    iStartY,   4, iDist    );
        xTZSearchHelp( pcPatternKey, rcStruct, iRight,   iStartY,   5, iDist    );
        xTZSearchHelp( pcPatternKey, rcStruct, iLeft_2,  iBottom_2, 6, iDist>>1 );
        xTZSearchHelp( pcPatternKey, rcStruct, iRight_2, iBottom_2, 8, iDist>>1 );
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX,  iBottom,   7, iDist    );
      }
      else // check border
      {
        if ( iTop >= iSrchRngVerTop ) // check top
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iTop, 2, iDist );
        }
        if ( iTop_2 >= iSrchRngVerTop ) // check half top
        {
          if ( iLeft_2 >= iSrchRngHorLeft ) // check half left
          {
            xTZSearchHelp( pcPatternKey, rcStruct, iLeft_2, iTop_2, 1, (iDist>>1) );
          }
          if ( iRight_2 <= iSrchRngHorRight ) // check half right
          {
            xTZSearchHelp( pcPatternKey, rcStruct, iRight_2, iTop_2, 3, (iDist>>1) );
          }
        } // check half top
        if ( iLeft >= iSrchRngHorLeft ) // check left
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iLeft, iStartY, 4, iDist );
        }
        if ( iRight <= iSrchRngHorRight ) // check right
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iRight, iStartY, 5, iDist );
        }
        if ( iBottom_2 <= iSrchRngVerBottom ) // check half bottom
        {
          if ( iLeft_2 >= iSrchRngHorLeft ) // check half left
          {
            xTZSearchHelp( pcPatternKey, rcStruct, iLeft_2, iBottom_2, 6, (iDist>>1) );
          }
          if ( iRight_2 <= iSrchRngHorRight ) // check half right
          {
            xTZSearchHelp( pcPatternKey, rcStruct, iRight_2, iBottom_2, 8, (iDist>>1) );
          }
        } // check half bottom
        if ( iBottom <= iSrchRngVerBottom ) // check bottom
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iBottom, 7, iDist );
        }
      } // check border
    }
    else // iDist > 8
    {
      if ( iTop >= iSrchRngVerTop && iLeft >= iSrchRngHorLeft &&
          iRight <= iSrchRngHorRight && iBottom <= iSrchRngVerBottom ) // check border
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iTop,    0, iDist );
        xTZSearchHelp( pcPatternKey, rcStruct, iLeft,   iStartY, 0, iDist );
        xTZSearchHelp( pcPatternKey, rcStruct, iRight,  iStartY, 0, iDist );
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iBottom, 0, iDist );
        for ( Int index = 1; index < 4; index++ )
        {
          Int iPosYT = iTop    + ((iDist>>2) * index);
          Int iPosYB = iBottom - ((iDist>>2) * index);
          Int iPosXL = iStartX - ((iDist>>2) * index);
          Int iPosXR = iStartX + ((iDist>>2) * index);
          xTZSearchHelp( pcPatternKey, rcStruct, iPosXL, iPosYT, 0, iDist );
          xTZSearchHelp( pcPatternKey, rcStruct, iPosXR, iPosYT, 0, iDist );
          xTZSearchHelp( pcPatternKey, rcStruct, iPosXL, iPosYB, 0, iDist );
          xTZSearchHelp( pcPatternKey, rcStruct, iPosXR, iPosYB, 0, iDist );
        }
      }
      else // check border
      {
        if ( iTop >= iSrchRngVerTop ) // check top
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iTop, 0, iDist );
        }
        if ( iLeft >= iSrchRngHorLeft ) // check left
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iLeft, iStartY, 0, iDist );
        }
        if ( iRight <= iSrchRngHorRight ) // check right
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iRight, iStartY, 0, iDist );
        }
        if ( iBottom <= iSrchRngVerBottom ) // check bottom
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iBottom, 0, iDist );
        }
        for ( Int index = 1; index < 4; index++ )
        {
          Int iPosYT = iTop    + ((iDist>>2) * index);
          Int iPosYB = iBottom - ((iDist>>2) * index);
          Int iPosXL = iStartX - ((iDist>>2) * index);
          Int iPosXR = iStartX + ((iDist>>2) * index);
          
          if ( iPosYT >= iSrchRngVerTop ) // check top
          {
            if ( iPosXL >= iSrchRngHorLeft ) // check left
            {
              xTZSearchHelp( pcPatternKey, rcStruct, iPosXL, iPosYT, 0, iDist );
            }
            if ( iPosXR <= iSrchRngHorRight ) // check right
            {
              xTZSearchHelp( pcPatternKey, rcStruct, iPosXR, iPosYT, 0, iDist );
            }
          } // check top
          if ( iPosYB <= iSrchRngVerBottom ) // check bottom
          {
            if ( iPosXL >= iSrchRngHorLeft ) // check left
            {
              xTZSearchHelp( pcPatternKey, rcStruct, iPosXL, iPosYB, 0, iDist );
            }
            if ( iPosXR <= iSrchRngHorRight ) // check right
            {
              xTZSearchHelp( pcPatternKey, rcStruct, iPosXR, iPosYB, 0, iDist );
            }
          } // check bottom
        } // for ...
      } // check border
    } // iDist <= 8
  } // iDist == 1
}

//<--
#if rd_sym
UInt TEncSearch::xPatternRefinementSym(TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Pel* piSymRef, Int iSymRefStride, Int iIntStep, Int DistBW, Int DistFw, Int iFrac, TComMv& rcMvFrac)
{
	UInt  uiDist;
	UInt  uiDistBest = MAX_UINT;
	UInt  uiDirecBest = 0;

	Pel*  piRefPos;
	Pel*  piSymRefPos;
	m_pcRdCost->setDistParam(pcPatternKey, piRef, piSymRef, iRefStride, iSymRefStride, iIntStep, m_cDistParam, m_pcEncCfg->getUseHADME());
	Int iWidth = pcPatternKey->getROIYWidth();
	Int iHeight = pcPatternKey->getROIYHeight();

	TComMv* pcMvRefine = s_acMvRefineQ;

	for (UInt i = 0; i < 9; i++)
	{
		TComMv cMvTest = pcMvRefine[i];
		cMvTest += rcMvFrac;
		piRefPos = piRef + (pcMvRefine[i].getHor() + iRefStride * pcMvRefine[i].getVer()) * iFrac;
		if (pcCU->getPicture()->getPictureType() == B_PICTURE)
		{
			piSymRefPos = piSymRef + (-scaleMv(pcMvRefine[i].getHor(), DistBW, DistFw) - scaleMv(pcMvRefine[i].getVer(), DistBW, DistFw) * iSymRefStride) * iFrac;
		}
		else if (pcCU->getPicture()->getPictureType() == F_PICTURE)
		{
			piSymRefPos = piSymRef + (scaleMv(pcMvRefine[i].getHor(), DistBW, DistFw) + scaleMv(pcMvRefine[i].getVer(), DistBW, DistFw) * iSymRefStride) * iFrac;
		}
		m_cDistParam.pCur = piRefPos;
		m_cDistParam.pBwCur = piSymRefPos;
#if FULLSEARCHFAST

		uiDist = m_pcRdCost->getCost(cMvTest.getHor(), cMvTest.getVer());
		m_cDistParam.m_uiMvCost = uiDist;
		m_cDistParam.m_uiSadBest = uiDistBest;
		uiDist += m_cDistParam.DistFunc(&m_cDistParam);

#else

		uiDist = m_cDistParam.DistFunc(&m_cDistParam);

		uiDist += m_pcRdCost->getCost(cMvTest.getHor(), cMvTest.getVer());
#endif



		if (uiDist < uiDistBest)
		{
			uiDistBest = uiDist;
			uiDirecBest = i;
		}
	}

	rcMvFrac = pcMvRefine[uiDirecBest];

	return uiDistBest;
}

UInt TEncSearch::xPatternRefinementQSym(TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Pel* piSymRef, Int iSymRefStride, Int iIntStep, Int DistBW, Int DistFw, Int iMvX, Int iMvY, TComMv& rcMvFrac)
{
	UInt  uiDist;
	UInt  uiDistBest = MAX_UINT;
	UInt  uiDirecBest = 0;
	Int   iCandMvx, iCandMvy;
	Int iWidth = pcPatternKey->getROIYWidth();
	Int iHeight = pcPatternKey->getROIYHeight();
	Pel*  piRefPos;
	Pel*  piSymRefPos;
	m_pcRdCost->setDistParam(pcPatternKey, piRef, piSymRef, iRefStride, iSymRefStride, iIntStep, m_cDistParam, m_pcEncCfg->getUseHADME());

	TComMv* pcMvRefine = s_acMvRefineQ;

	for (UInt i = 1; i < 9; i++)
	{
		TComMv cMvTest = pcMvRefine[i];
		cMvTest += rcMvFrac;
#if RD_PMVR
		if ((abs(rcMvFrac.getHor() - iMvX) > TH) || (abs(rcMvFrac.getVer() - iMvY) > TH))
		{
			piRefPos = piRef + ((pcMvRefine[i].getHor() << 1) + iRefStride * (pcMvRefine[i].getVer() << 1));
			if (pcCU->getPicture()->getPictureType() == B_PICTURE)
			{
				piSymRefPos = piSymRef + (-scaleMv(pcMvRefine[i].getHor() << 1, DistBW, DistFw) - scaleMv(pcMvRefine[i].getVer() << 1, DistBW, DistFw) * iSymRefStride);
			}
			else if (pcCU->getPicture()->getPictureType() == F_PICTURE)
			{
				piSymRefPos = piSymRef + (scaleMv(pcMvRefine[i].getHor() << 1, DistBW, DistFw) + scaleMv(pcMvRefine[i].getVer() << 1, DistBW, DistFw) * iSymRefStride);
			}
		}
		else
		{
			piRefPos = piRef + (pcMvRefine[i].getHor() + iRefStride * pcMvRefine[i].getVer()) * 1;
			if (pcCU->getPicture()->getPictureType() == B_PICTURE)
			{
				piSymRefPos = piSymRef + (-scaleMv(pcMvRefine[i].getHor(), DistBW, DistFw) - scaleMv(pcMvRefine[i].getVer(), DistBW, DistFw) * iSymRefStride);
			}
			else if (pcCU->getPicture()->getPictureType() == F_PICTURE)
			{
				piSymRefPos = piSymRef + (scaleMv(pcMvRefine[i].getHor(), DistBW, DistFw) + scaleMv(pcMvRefine[i].getVer(), DistBW, DistFw) * iSymRefStride);
			}
		}
#else
		piRefPos = piRef + (pcMvRefine[i].getHor() + iRefStride * pcMvRefine[i].getVer()) * 1;
		if (pcCU->getPicture()->getPictureType() == B_PICTURE)
		{
			piSymRefPos = piSymRef + (-scaleMv(pcMvRefine[i].getHor(), DistBW, DistFw) - scaleMv(pcMvRefine[i].getVer(), DistBW, DistFw) * iSymRefStride);
		}
		else if (pcCU->getPicture()->getPictureType() == F_PICTURE)
		{
			piSymRefPos = piSymRef + (scaleMv(pcMvRefine[i].getHor(), DistBW, DistFw) + scaleMv(pcMvRefine[i].getVer(), DistBW, DistFw) * iSymRefStride);
		}
#endif

		m_cDistParam.pCur = piRefPos;
		m_cDistParam.pBwCur = piSymRefPos;
		uiDist = m_cDistParam.DistFunc(&m_cDistParam);

#if RD_PMVR
		if (m_pcEncCfg->getPMVREnable())
		{
			if (!pmvrAdaptMv(&iCandMvx, &iCandMvy, iMvX, iMvY, rcMvFrac.getHor(), rcMvFrac.getVer(), i))
			{
				continue;
			}
		}
		else
		{
			iCandMvx = cMvTest.getHor();
			iCandMvy = cMvTest.getVer();
		}
#else
		iCandMvx = cMvTest.getHor();
		iCandMvy = cMvTest.getVer();
#endif

		uiDist += m_pcRdCost->getCost(iCandMvx, iCandMvy);

		if (uiDist < uiDistBest)
		{
			uiDistBest = uiDist;
			uiDirecBest = i;
		}
	}

#if RD_PMVR
	{
		if ((m_pcEncCfg->getPMVREnable()))//传参数 m_pcEncCfg->getUsePmvr()
		{
			pmvrAdaptMv(&iCandMvx, &iCandMvy, iMvX, iMvY, rcMvFrac.getHor(), rcMvFrac.getVer(), uiDirecBest);
			rcMvFrac.set(iCandMvx - rcMvFrac.getHor(), iCandMvy - rcMvFrac.getVer());
			//rcMvFrac.set(iCandMvx , iCandMvy);
		}
		else
		{
			rcMvFrac = pcMvRefine[uiDirecBest];
		}
	}
#else
	rcMvFrac = pcMvRefine[uiDirecBest];
#endif

	return uiDistBest;
}
#endif

#if rd_bipred
UInt TEncSearch::xPatternRefinementBi(TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Pel* piBwRefY, Int iBwRefStride, Int iIntStep, Int iFrac, TComMv& rcMvFrac)
{
	UInt  uiDist;
	UInt  uiDistBest = MAX_UINT;
	UInt  uiDirecBest = 0;

	Pel*  piRefPos;
	m_pcRdCost->setDistParam(pcPatternKey, piRef, piBwRefY, iRefStride, iBwRefStride, iIntStep, m_cDistParam, m_pcEncCfg->getUseHADME());
	Int iWidth = pcPatternKey->getROIYWidth();
	Int iHeight = pcPatternKey->getROIYHeight();
	Pel* pos = (Pel*)malloc(sizeof(Int) * 4 * iWidth*iHeight);

	TComMv* pcMvRefine = s_acMvRefineQ;

	for (UInt i = 0; i < 9; i++)
	{
		TComMv cMvTest = pcMvRefine[i];
		cMvTest += rcMvFrac;
		piRefPos = piRef + (pcMvRefine[i].getHor() + iRefStride * pcMvRefine[i].getVer()) * iFrac;
		/*
		Pel* piFwRef = piRefPos;
		Pel* piBwRef = piBwRefY;
		Int iFwRefStride = (iRefStride * iIntStep);
		//printf("\n");
		for (Int j = 0; j < iHeight; j++)
		{
		for (Int i = 0; i < iWidth; i++)
		{
		//printf("%d\t", piFwRef[i * 4]);
		piFwRef[i * 4] = (piFwRef[i * 4] + piBwRef[i] + 1) / 2;
		//printf("%d\t", piBwRef[i]);
		}
		piFwRef += iFwRefStride;
		piBwRef += iBwRefStride;
		//printf("\n");
		}
		*/
		//m_cDistParam.pCur = pos;
		m_cDistParam.pCur = piRefPos;
#if FULLSEARCHFAST

		uiDist = m_pcRdCost->getCost(cMvTest.getHor(), cMvTest.getVer());
		m_cDistParam.m_uiMvCost = uiDist;
		m_cDistParam.m_uiSadBest = uiDistBest;
		uiDist += m_cDistParam.DistFunc(&m_cDistParam);

#else

		uiDist = m_cDistParam.DistFunc(&m_cDistParam);

		uiDist += m_pcRdCost->getCost(cMvTest.getHor(), cMvTest.getVer());
#endif



		if (uiDist < uiDistBest)
		{
			uiDistBest = uiDist;
			uiDirecBest = i;
		}
	}

	rcMvFrac = pcMvRefine[uiDirecBest];

	return uiDistBest;
}

UInt TEncSearch::xPatternRefinementQBi(TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Pel* piBwRefY, Int iBwRefStride, Int iIntStep, Int iMvX, Int iMvY, TComMv& rcMvFrac)
{
	UInt  uiDist;
	UInt  uiDistBest = MAX_UINT;
	UInt  uiDirecBest = 0;
	Int   iCandMvx, iCandMvy;
	Int iWidth = pcPatternKey->getROIYWidth();
	Int iHeight = pcPatternKey->getROIYHeight();
	Pel*  piRefPos;
	m_pcRdCost->setDistParam(pcPatternKey, piRef, piBwRefY, iRefStride, iBwRefStride, iIntStep, m_cDistParam, m_pcEncCfg->getUseHADME());

	TComMv* pcMvRefine = s_acMvRefineQ;

	for (UInt i = 1; i < 9; i++)
	{
		TComMv cMvTest = pcMvRefine[i];
		cMvTest += rcMvFrac;
#if RD_PMVR
		if ((abs(rcMvFrac.getHor() - iMvX) > TH) || (abs(rcMvFrac.getVer() - iMvY) > TH))
		{
			piRefPos = piRef + ((pcMvRefine[i].getHor() << 1) + iRefStride * (pcMvRefine[i].getVer() << 1));
		}
		else
		{
			piRefPos = piRef + (pcMvRefine[i].getHor() + iRefStride * pcMvRefine[i].getVer()) * 1;
		}
#else
		piRefPos = piRef + (pcMvRefine[i].getHor() + iRefStride * pcMvRefine[i].getVer()) * 1;
#endif

		m_cDistParam.pCur = piRefPos;
		uiDist = m_cDistParam.DistFunc(&m_cDistParam);

#if RD_PMVR
		if (m_pcEncCfg->getPMVREnable())
		{
			if (!pmvrAdaptMv(&iCandMvx, &iCandMvy, iMvX, iMvY, rcMvFrac.getHor(), rcMvFrac.getVer(), i))
			{
				continue;
			}
		}
		else
		{
			iCandMvx = cMvTest.getHor();
			iCandMvy = cMvTest.getVer();
		}
#else
		iCandMvx = cMvTest.getHor();
		iCandMvy = cMvTest.getVer();
#endif
		//Int lambda_factor = LAMBDA_FACTOR(sqrt(m_pcRdCost->getLambda()));
		//uiDist += m_pcRdCost->getMvCost(lambda_factor, 0, iCandMvx, iCandMvy, rcMvFrac.getHor(), rcMvFrac.getVer());
		uiDist += m_pcRdCost->getCost(iCandMvx, iCandMvy);

		if (uiDist < uiDistBest)
		{
			uiDistBest = uiDist;
			uiDirecBest = i;
		}
	}

	//if (uiDirecBest)
	{
#if RD_PMVR
		if ((m_pcEncCfg->getPMVREnable()))//传参数 m_pcEncCfg->getUsePmvr()
		{
			pmvrAdaptMv(&iCandMvx, &iCandMvy, iMvX, iMvY, rcMvFrac.getHor(), rcMvFrac.getVer(), uiDirecBest);
			rcMvFrac.set(iCandMvx - rcMvFrac.getHor(), iCandMvy - rcMvFrac.getVer());
			//rcMvFrac.set(iCandMvx , iCandMvy);
		}
		else
		{
			rcMvFrac = pcMvRefine[uiDirecBest];
		}
#else
		rcMvFrac = pcMvRefine[uiDirecBest];
#endif
	}

	return uiDistBest;
}
#endif
#if DCTIF
UInt TEncSearch::xPatternRefinement( TComPattern* pcPatternKey,
	TComMv baseRefMv,
	Int iFrac, TComMv& rcMvFrac )
{
	UInt  uiDist;
	UInt  uiDistBest  = MAX_UINT;
	UInt  uiDirecBest = 0;

	Pel*  piRefPos;
	Int iRefStride = m_filteredBlock[0][0].getRealStride();

	m_pcRdCost->setDistParam( pcPatternKey, m_filteredBlock[0][0].getLumaAddr(), iRefStride, 1, m_cDistParam, m_pcEncCfg->getUseHADME() );

	const TComMv* pcMvRefine = (iFrac == 2 ? s_acMvRefineH : s_acMvRefineQ);

	for (UInt i = 0; i < 9; i++)
	{
		TComMv cMvTest = pcMvRefine[i];
		cMvTest += baseRefMv;

		Int horVal = cMvTest.getHor() * iFrac;
		Int verVal = cMvTest.getVer() * iFrac;
		piRefPos = m_filteredBlock[ verVal & 3 ][ horVal & 3 ].getLumaAddr();
		if ( horVal == 2 && ( verVal & 1 ) == 0 )
			piRefPos += 1;
		if ( ( horVal & 1 ) == 0 && verVal == 2 )
			piRefPos += iRefStride;
		cMvTest = pcMvRefine[i];
		cMvTest += rcMvFrac;

		setDistParamComp(0);  // Y component

		m_cDistParam.pCur = piRefPos;
		m_cDistParam.bitDepth = g_uiBitDepth;
		uiDist = m_cDistParam.DistFunc(&m_cDistParam);
		uiDist += m_pcRdCost->getCost(cMvTest.getHor(), cMvTest.getVer());

		if (uiDist < uiDistBest)
		{
			uiDistBest = uiDist;
			uiDirecBest = i;
		}
	}

	rcMvFrac = pcMvRefine[uiDirecBest];

	return uiDistBest;
}
#else
UInt TEncSearch::xPatternRefinement    ( TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, Int iFrac, TComMv& rcMvFrac )
{
  UInt  uiDist;
  UInt  uiDistBest  = MAX_UINT;
  UInt  uiDirecBest = 0;
  
  Pel*  piRefPos;
  m_pcRdCost->setDistParam( pcPatternKey, piRef, iRefStride, iIntStep, m_cDistParam, m_pcEncCfg->getUseHADME() );
  
#if INTER611
  TComMv* pcMvRefine = s_acMvRefineQ;
#else
  TComMv* pcMvRefine = (iFrac == 2 ? s_acMvRefineH : s_acMvRefineQ);
#endif
  
  for (UInt i = 0; i < 9; i++)
  {
    TComMv cMvTest = pcMvRefine[i];
    cMvTest += rcMvFrac;
    piRefPos = piRef + (pcMvRefine[i].getHor() + iRefStride * pcMvRefine[i].getVer()) * iFrac;
    m_cDistParam.pCur = piRefPos;
#if FULLSEARCHFAST
   
    uiDist = m_pcRdCost->getCost(cMvTest.getHor(), cMvTest.getVer());
    m_cDistParam.m_uiMvCost = uiDist;
    m_cDistParam.m_uiSadBest = uiDistBest;
    uiDist += m_cDistParam.DistFunc(&m_cDistParam);

#else
    uiDist = m_cDistParam.DistFunc( &m_cDistParam );

    uiDist += m_pcRdCost->getCost(cMvTest.getHor(), cMvTest.getVer());
#endif
   

    
    if ( uiDist < uiDistBest )
    {
      uiDistBest  = uiDist;
      uiDirecBest = i;
    }
  }
  
  rcMvFrac = pcMvRefine[uiDirecBest];
  
  return uiDistBest;
}
#endif

#if RD_PMVR
UInt TEncSearch::xPatternRefinementQ(TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, Int iMvX, Int iMvY, TComMv& rcMvFrac)
{
	UInt  uiDist;
	UInt  uiDistBest = MAX_UINT;
	UInt  uiDirecBest = 0;
	Int   iCandMvx, iCandMvy;

	Pel*  piRefPos;
	m_pcRdCost->setDistParam(pcPatternKey, piRef, iRefStride, iIntStep, m_cDistParam, m_pcEncCfg->getUseHADME());

	TComMv* pcMvRefine = s_acMvRefineQ;

	for (UInt i = 0; i < 9; i++)
	{
		TComMv cMvTest = pcMvRefine[i];
		cMvTest += rcMvFrac;
		if ((abs(rcMvFrac.getHor() - iMvX) > TH) || (abs(rcMvFrac.getVer() - iMvY) > TH))
		{
			piRefPos = piRef + ((pcMvRefine[i].getHor() << 1) + iRefStride * (pcMvRefine[i].getVer() << 1));
		}
		else
		{
			piRefPos = piRef + (pcMvRefine[i].getHor() + iRefStride * pcMvRefine[i].getVer()) * 1;
		}
		//piRefPos = piRef + (m_piSpiralSearchX[i] + iRefStride * m_piSpiralSearchY[i]) * 1;
		m_cDistParam.pCur = piRefPos;
		uiDist = m_cDistParam.DistFunc(&m_cDistParam);


		if (m_pcEncCfg->getPMVREnable())
		{
			if (!pmvrAdaptMv(&iCandMvx, &iCandMvy, iMvX, iMvY, rcMvFrac.getHor(), rcMvFrac.getVer(), i))
			{
				continue;
			}
		}
		else
		{
			iCandMvx = cMvTest.getHor();
			iCandMvy = cMvTest.getVer();
		}

		//Int lambda_factor = LAMBDA_FACTOR(sqrt(m_pcRdCost->getLambda()));
		//uiDist += m_pcRdCost->getMvCost(lambda_factor, 0, iCandMvx, iCandMvy, rcMvFrac.getHor(), rcMvFrac.getVer());
		uiDist += m_pcRdCost->getCost(iCandMvx, iCandMvy);

		if (uiDist < uiDistBest)
		{
			uiDistBest = uiDist;
			uiDirecBest = i;
		}
	}

	//if (uiDirecBest)
	{
		if ((m_pcEncCfg->getPMVREnable()))//传参数 m_pcEncCfg->getUsePmvr()
		{
			pmvrAdaptMv(&iCandMvx, &iCandMvy, iMvX, iMvY, rcMvFrac.getHor(), rcMvFrac.getVer(), uiDirecBest);
			rcMvFrac.set(iCandMvx - rcMvFrac.getHor(), iCandMvy - rcMvFrac.getVer());
			//rcMvFrac.set(iCandMvx , iCandMvy);
		}
		else
		{
			rcMvFrac = pcMvRefine[uiDirecBest];
		}
	}

	return uiDistBest;
}
#endif

#if ZHANGYI_INTRA
UInt TEncSearch::xIntraSearchChroma( ComponentID compID, TComDataCU* pcCU, TComYuv* pcOrgYuv, TComYuv* pcPredYuv, TComYuv* pcResiYuv, TComYuv* pcRecoYuv, UInt uiMode, UInt uiPartOffset, Int* p_avail )
{
	int luma_pic_pix_x = pcCU->getCUPelX();
	int luma_pic_pix_y = pcCU->getCUPelY();
	int lumaWidth = pcCU->getWidth(0);
	int lumaHeight = pcCU->getHeight(0);
	int bitDepth = pcCU->getPicture()->getSPS()->getBitDepth();
	UInt    uiAbsZorderIdx    = pcCU     ->getZorderIdxInCU();
	UInt    uiDepth           = pcCU->getDepth     ( 0 );
	UInt    uiPartDepth       = pcCU->getPartitionSize( 0 ) == SIZE_2Nx2N ? 0 : 1;
	UInt    uiQNumParts    = pcCU->getTotalNumPart()>>(uiPartDepth<<1);
	UInt    uiMaxTrDepth = (pcCU->getPartitionSize(0) == SIZE_NxN) ? 1 : 0;
	UInt    uiLog2Size        = pcCU->getLog2CUSize( 0 ) - uiPartDepth; //luma pu
	UInt    uiLog2SizeC     = pcCU->getLog2CUSize(0) - 1; //chroma pu width is the half of the cu width
	UInt    uiSizeC = 1<<uiLog2SizeC;
	UInt    uiStrideC         = pcOrgYuv ->getCStride (); 

	Pel*    piOrgCb           = pcOrgYuv ->getCbAddr( uiAbsZorderIdx + uiPartOffset );
	Pel*    piResiCb          = pcResiYuv->getCbAddr( uiAbsZorderIdx + uiPartOffset );
	Pel*    piRecoCb          = pcRecoYuv->getCbAddr( uiAbsZorderIdx + uiPartOffset );
	Pel*    piPredCb          = pcPredYuv->getCbAddr( uiAbsZorderIdx + uiPartOffset );

	Pel*    piOrgCr           = pcOrgYuv ->getCrAddr( uiAbsZorderIdx + uiPartOffset );
	Pel*    piResiCr          = pcResiYuv->getCrAddr( uiAbsZorderIdx + uiPartOffset );
	Pel*    piRecoCr          = pcRecoYuv->getCrAddr( uiAbsZorderIdx + uiPartOffset );
	Pel*    piPredCr          = pcPredYuv->getCrAddr( uiAbsZorderIdx + uiPartOffset );

	UInt    uiRecStrideC      = pcRecoYuv->getCStride();
	UInt uiMinCoeffSize = 1<<(pcCU->getPic()->getLog2MinCUSize()<<1);
	UInt uiLumaCoeffOffset   = uiMinCoeffSize*(uiPartOffset);
	UInt uiChromaCoeffOffset = uiLumaCoeffOffset>>2;
	TCoeff* pcCoeffCb         = pcCU     ->getCoeffCb () + uiChromaCoeffOffset;
	TCoeff* pcCoeffCr         = pcCU     ->getCoeffCr () + uiChromaCoeffOffset;

	UInt uiAbsSumCb = 0;
	UInt uiAbsSumCr = 0;
	UInt uiDistortionCb, uiDistortionCr;
	Double  dPUBestCostCb    = MAX_DOUBLE;
	Double  dPUBestCostCr    = MAX_DOUBLE;
	UInt    uiNextDepth    = uiDepth + 1;

	Pel *piAdiTempCb   = m_piYuvExt2g[COMPONENT_Cb][PRED_BUF_UNFILTERED];
	Pel *piAdiTempCr    = m_piYuvExt2g[COMPONENT_Cr][PRED_BUF_UNFILTERED];
	//转换为亮度的模式
	UInt uiChFinalMode;
	switch (uiMode)
	{
	case INTRA_DC_PRED_CHROMA: 
		uiChFinalMode = INTRA_DC_IDX;
		break;
	case INTRA_HOR_PRED_CHROMA:
		uiChFinalMode = INTRA_HOR_IDX;
		break;
	case INTRA_VERT_PRED_CHROMA:
		uiChFinalMode = INTRA_VER_IDX;
		break;
	case INTRA_BI_PRED_CHROMA:
		uiChFinalMode = INTRA_BI_IDX;
		break;
	case INTRA_DM_PRED_CHROMA:
		uiChFinalMode = pcCU->getIntraDir(0);//当前CU的第一个亮度PU的模式
		break;
	}

	if (compID == COMPONENT_Cb)
	{
#if ZHANGYI_INTRA_SDIP
		predIntraAngAVS(TEXT_CHROMA_U, uiChFinalMode, uiStrideC, piPredCb, uiStrideC, uiSizeC, uiSizeC, p_avail[NEIGHBOR_INTRA_UP], p_avail[NEIGHBOR_INTRA_LEFT], pcCU->getPicture()->getSPS()->getBitDepth());
#else
		predIntraAngAVS(TEXT_CHROMA_U, uiChFinalMode, uiStrideC, piPredCb, uiStrideC, uiSizeC, p_avail[NEIGHBOR_INTRA_UP], p_avail[NEIGHBOR_INTRA_LEFT], pcCU->getPicture()->getSPS()->getBitDepth());
#endif
	}
	else if (compID == COMPONENT_Cr)
	{
#if ZHANGYI_INTRA_SDIP
		predIntraAngAVS(TEXT_CHROMA_V, uiChFinalMode, uiStrideC, piPredCr, uiStrideC, uiSizeC, uiSizeC, p_avail[NEIGHBOR_INTRA_UP], p_avail[NEIGHBOR_INTRA_LEFT], pcCU->getPicture()->getSPS()->getBitDepth());
#else
		predIntraAngAVS(TEXT_CHROMA_V, uiChFinalMode, uiStrideC, piPredCr, uiStrideC, uiSizeC, p_avail[NEIGHBOR_INTRA_UP], p_avail[NEIGHBOR_INTRA_LEFT], pcCU->getPicture()->getSPS()->getBitDepth());
#endif
	}

	// get residual
	UInt uiY, uiX;
	if (compID == COMPONENT_Cb)
	{
		for(  uiY = 0; uiY < uiSizeC; uiY++ )
		{
			UInt uiY2 = uiY*uiStrideC;
			for( uiX = 0; uiX < uiSizeC; uiX++ )
			{
				UInt ui = uiY2 + uiX;
				piResiCb[ ui ] = piOrgCb[ ui ] - piPredCb[ ui ];
			}
		}
		//--- transform and quantization --- Cb
		m_pcTrQuant->setQPforQuant  ( TEXT_CHROMA );
#if ZHOULULU_QT
		m_pcTrQuant->setBlockSize( pcCU, uiLog2SizeC, TEXT_CHROMA, 0 );
		m_pcTrQuant->transformMxN   ( pcCU, piResiCb, uiStrideC, pcCoeffCb, uiLog2SizeC, uiAbsSumCb, TEXT_CHROMA_U, 0 );
	//	m_pcTrQuant->transformMxN   ( pcCU, piResiCr, uiStrideC, pcCoeffCr, uiLog2SizeC, uiAbsSumCr, TEXT_CHROMA_V, 0 );
#else
		m_pcTrQuant->transformNxN   ( pcCU, piResiCb, uiStrideC, pcCoeffCb, uiLog2SizeC, uiAbsSumCb, TEXT_CHROMA_U, 0 );
#endif
#if ZHOULULU_QT
		m_pcTrQuant->invtransformMxN( pcCU,piResiCb, uiStrideC, pcCoeffCb, uiLog2SizeC, TEXT_CHROMA_U, 0 );
	//	m_pcTrQuant->invtransformMxN( pcCU,piResiCr, uiStrideC, pcCoeffCr, uiLog2SizeC, TEXT_CHROMA_V, 0 );
#else
		m_pcTrQuant->invtransformNxN( piResiCb, uiStrideC, pcCoeffCb, uiLog2SizeC, TEXT_CHROMA_U );
#endif
#if ZHANGYI_INTRA
		//--- set coded block flag ---
		pcCU->setCbfSubParts( ( uiAbsSumCb ? 1 : 0 ) , TEXT_CHROMA_U, uiPartOffset, uiDepth+uiPartDepth );

		if( uiPartDepth == 1 )
		{
			pcCU->setCuCbfChroma(uiPartOffset, uiPartDepth, uiPartDepth);
		}
#else
		//--- set coded block flag ---
		pcCU->setCbfSubParts( ( uiAbsSumCb ? 1 : 0 ) << uiPartDepth, TEXT_CHROMA_U, uiPartOffset, uiDepth+uiPartDepth );

		if( uiPartDepth == 1 )
		{
			pcCU->setCuCbfChroma(uiPartOffset, uiPartDepth, uiPartDepth);
		}
#endif
		//reconstruct
		Pel* pRecoCb = piRecoCb;
		for( uiY = 0; uiY < uiSizeC; uiY++ )
		{
			for( uiX = 0; uiX < uiSizeC; uiX++ )
			{
				pRecoCb[ uiX ] = Clip( piPredCb[ uiX ] + piResiCb[ uiX ] );
			}
			pRecoCb  += uiRecStrideC; 
			piPredCb += uiStrideC; 
			piResiCb += uiStrideC; 
		}
		//calculate distortion for this mode of Cb
		uiDistortionCb = (UInt)( m_pcRdCost->getChromaWeight()*(Double)( m_pcRdCost->getDistPart( piRecoCb, uiRecStrideC, piOrgCb, uiStrideC, 1<<(uiLog2SizeC), 1<<(uiLog2SizeC) ) ) );
		return uiDistortionCb;
	} else if (compID == COMPONENT_Cr)
	{
		for(  uiY = 0; uiY < uiSizeC; uiY++ )
		{
			UInt uiY2 = uiY*uiStrideC;
			for( uiX = 0; uiX < uiSizeC; uiX++ )
			{
				UInt ui = uiY2 + uiX;
				piResiCr[ ui ] = piOrgCr[ ui ] - piPredCr[ ui ];
			}
		}
		//--- transform and quantization --- Cr
		m_pcTrQuant->setQPforQuant  ( TEXT_CHROMA );
#if ZHOULULU_QT
		m_pcTrQuant->setBlockSize( pcCU, uiLog2SizeC, TEXT_CHROMA, 0 );
		//m_pcTrQuant->transformMxN   ( pcCU, piResiCb, uiStrideC, pcCoeffCb, uiLog2SizeC, uiAbsSumCb, TEXT_CHROMA_U, 0 );
		m_pcTrQuant->transformMxN   ( pcCU, piResiCr, uiStrideC, pcCoeffCr, uiLog2SizeC, uiAbsSumCr, TEXT_CHROMA_V, 0 );
#else
		m_pcTrQuant->transformNxN   ( pcCU, piResiCr, uiStrideC, pcCoeffCr, uiLog2SizeC, uiAbsSumCr, TEXT_CHROMA_V, 0 );
#endif
#if ZHOULULU_QT
	//	m_pcTrQuant->invtransformMxN( pcCU,piResiCb, uiStrideC, pcCoeffCb, uiLog2SizeC, TEXT_CHROMA_U, 0 );
		m_pcTrQuant->invtransformMxN( pcCU,piResiCr, uiStrideC, pcCoeffCr, uiLog2SizeC, TEXT_CHROMA_V, 0 );
#else
		m_pcTrQuant->invtransformNxN( piResiCr, uiStrideC, pcCoeffCr, uiLog2SizeC, TEXT_CHROMA_V );
#endif
		//--- set coded block flag ---
		pcCU->setCbfSubParts( ( uiAbsSumCr ? 1 : 0 ) << uiPartDepth, TEXT_CHROMA_V, uiPartOffset, uiDepth+uiPartDepth );

		if( uiPartDepth == 1 )
		{
			pcCU->setCuCbfChroma(uiPartOffset, uiPartDepth, uiPartDepth);
		}
		//reconstruct
		Pel* pRecoCr = piRecoCr;
		for( uiY = 0; uiY < uiSizeC; uiY++ )
		{
			for( uiX = 0; uiX < uiSizeC; uiX++ )
			{
				pRecoCr[ uiX ] = Clip( piPredCr[ uiX ] + piResiCr[ uiX ] );
			}
			pRecoCr  += uiRecStrideC; 
			piPredCr += uiStrideC; 
			piResiCr += uiStrideC; 
		}
		//calculate distortion for this mode of Cr
		uiDistortionCr = (UInt)( m_pcRdCost->getChromaWeight()*(Double)( m_pcRdCost->getDistPart( piRecoCr, uiRecStrideC, piOrgCr, uiStrideC, 1<<(uiLog2SizeC), 1<<(uiLog2SizeC) ) ) );
		return uiDistortionCr;
	}
	else 
	{
		return 0;
	}
}
Void TEncSearch::xUpdateCandList( UInt uiMode, double uiCost, Int uiFullCandNum, Int *CandModeList, double *CandCostList ) {
	int i;
	int shift = 0;

	while (shift < uiFullCandNum && uiCost < CandCostList[ uiFullCandNum - 1 - shift ]) {
		shift++;
	}

	if (shift != 0) {
		for (i = 1; i < shift; i++) {
			CandModeList[ uiFullCandNum - i ] = CandModeList[ uiFullCandNum - 1 - i ];
			CandCostList[ uiFullCandNum - i ] = CandCostList[ uiFullCandNum - 1 - i ];
		}
		CandModeList[ uiFullCandNum - shift ] = uiMode;
		CandCostList[ uiFullCandNum - shift ] = uiCost;
		return ;
	}
	return ;
}

UInt TEncSearch::calcHADs(Pel *pi, UInt uiStride, int iWidth, int iHeight)
{
	int uiSum = 0, sad, sum_sad = 0;
	int x, y, i, j, k;
	int diff[64], m1[8][8], m2[8][8], m3[8][8];

	// Hadamard8x8
	for (y = 0; y < iHeight; y += 8) {
		for (x = 0; x < iWidth; x += 8) {
			sad = 0;

			for (j = 0; j < 8; j++) {
				for (i = 0; i < 8; i++) {
					diff[j * 8 + i] = pi[(y + j) * uiStride + (x + i)];//pi[y + j][x + i];
				}
			}

			//horizontal
			for (j = 0; j < 8; j++) {
				k = j << 3;
				m2[j][0] = diff[k  ] + diff[k + 4];
				m2[j][1] = diff[k + 1] + diff[k + 5];
				m2[j][2] = diff[k + 2] + diff[k + 6];
				m2[j][3] = diff[k + 3] + diff[k + 7];
				m2[j][4] = diff[k  ] - diff[k + 4];
				m2[j][5] = diff[k + 1] - diff[k + 5];
				m2[j][6] = diff[k + 2] - diff[k + 6];
				m2[j][7] = diff[k + 3] - diff[k + 7];

				m1[j][0] = m2[j][0] + m2[j][2];
				m1[j][1] = m2[j][1] + m2[j][3];
				m1[j][2] = m2[j][0] - m2[j][2];
				m1[j][3] = m2[j][1] - m2[j][3];
				m1[j][4] = m2[j][4] + m2[j][6];
				m1[j][5] = m2[j][5] + m2[j][7];
				m1[j][6] = m2[j][4] - m2[j][6];
				m1[j][7] = m2[j][5] - m2[j][7];

				m2[j][0] = m1[j][0] + m1[j][1];
				m2[j][1] = m1[j][0] - m1[j][1];
				m2[j][2] = m1[j][2] + m1[j][3];
				m2[j][3] = m1[j][2] - m1[j][3];
				m2[j][4] = m1[j][4] + m1[j][5];
				m2[j][5] = m1[j][4] - m1[j][5];
				m2[j][6] = m1[j][6] + m1[j][7];
				m2[j][7] = m1[j][6] - m1[j][7];
			}

			//vertical
			for (i = 0; i < 8; i++) {
				m3[0][i] = m2[0][i] + m2[4][i];
				m3[1][i] = m2[1][i] + m2[5][i];
				m3[2][i] = m2[2][i] + m2[6][i];
				m3[3][i] = m2[3][i] + m2[7][i];
				m3[4][i] = m2[0][i] - m2[4][i];
				m3[5][i] = m2[1][i] - m2[5][i];
				m3[6][i] = m2[2][i] - m2[6][i];
				m3[7][i] = m2[3][i] - m2[7][i];

				m1[0][i] = m3[0][i] + m3[2][i];
				m1[1][i] = m3[1][i] + m3[3][i];
				m1[2][i] = m3[0][i] - m3[2][i];
				m1[3][i] = m3[1][i] - m3[3][i];
				m1[4][i] = m3[4][i] + m3[6][i];
				m1[5][i] = m3[5][i] + m3[7][i];
				m1[6][i] = m3[4][i] - m3[6][i];
				m1[7][i] = m3[5][i] - m3[7][i];

				m2[0][i] = m1[0][i] + m1[1][i];
				m2[1][i] = m1[0][i] - m1[1][i];
				m2[2][i] = m1[2][i] + m1[3][i];
				m2[3][i] = m1[2][i] - m1[3][i];
				m2[4][i] = m1[4][i] + m1[5][i];
				m2[5][i] = m1[4][i] - m1[5][i];
				m2[6][i] = m1[6][i] + m1[7][i];
				m2[7][i] = m1[6][i] - m1[7][i];
			}

			for (j = 0; j < 8; j++) {
				for (i = 0; i < 8; i++) {
					sad += (abs(m2[j][i]));
				}
			}

			sad = ((sad + 2) >> 2);

			sum_sad += sad;
		}
	}
	return sum_sad;
}
UInt TEncSearch::calcHAD4x4(Pel*pi, UInt uiStride, int iWidth, int iHeight)
{
	int uiSum = 0, sad, sum_sad = 0;
	int x, y, i, j;
	int  diff[16], m[16], d[16];

	// Hadamard8x8
	for (y = 0; y < iHeight; y += 4) {
		for (x = 0; x < iWidth; x += 4) {
			sad = 0;

			for (j = 0; j < 4; j++) {
				for (i = 0; i < 4; i++) {
					diff[j * 4 + i] = pi[(y + j) * uiStride + (x + i)];//pi[y + j][x + i];
				}
			}
			/*===== hadamard transform =====*/
			m[ 0] = diff[ 0] + diff[12];
			m[ 1] = diff[ 1] + diff[13];
			m[ 2] = diff[ 2] + diff[14];
			m[ 3] = diff[ 3] + diff[15];
			m[ 4] = diff[ 4] + diff[ 8];
			m[ 5] = diff[ 5] + diff[ 9];
			m[ 6] = diff[ 6] + diff[10];
			m[ 7] = diff[ 7] + diff[11];
			m[ 8] = diff[ 4] - diff[ 8];
			m[ 9] = diff[ 5] - diff[ 9];
			m[10] = diff[ 6] - diff[10];
			m[11] = diff[ 7] - diff[11];
			m[12] = diff[ 0] - diff[12];
			m[13] = diff[ 1] - diff[13];
			m[14] = diff[ 2] - diff[14];
			m[15] = diff[ 3] - diff[15];

			d[ 0] = m[ 0] + m[ 4];
			d[ 1] = m[ 1] + m[ 5];
			d[ 2] = m[ 2] + m[ 6];
			d[ 3] = m[ 3] + m[ 7];
			d[ 4] = m[ 8] + m[12];
			d[ 5] = m[ 9] + m[13];
			d[ 6] = m[10] + m[14];
			d[ 7] = m[11] + m[15];
			d[ 8] = m[ 0] - m[ 4];
			d[ 9] = m[ 1] - m[ 5];
			d[10] = m[ 2] - m[ 6];
			d[11] = m[ 3] - m[ 7];
			d[12] = m[12] - m[ 8];
			d[13] = m[13] - m[ 9];
			d[14] = m[14] - m[10];
			d[15] = m[15] - m[11];

			m[ 0] = d[ 0] + d[ 3];
			m[ 1] = d[ 1] + d[ 2];
			m[ 2] = d[ 1] - d[ 2];
			m[ 3] = d[ 0] - d[ 3];
			m[ 4] = d[ 4] + d[ 7];
			m[ 5] = d[ 5] + d[ 6];
			m[ 6] = d[ 5] - d[ 6];
			m[ 7] = d[ 4] - d[ 7];
			m[ 8] = d[ 8] + d[11];
			m[ 9] = d[ 9] + d[10];
			m[10] = d[ 9] - d[10];
			m[11] = d[ 8] - d[11];
			m[12] = d[12] + d[15];
			m[13] = d[13] + d[14];
			m[14] = d[13] - d[14];
			m[15] = d[12] - d[15];

			d[ 0] = m[ 0] + m[ 1];
			d[ 1] = m[ 0] - m[ 1];
			d[ 2] = m[ 2] + m[ 3];
			d[ 3] = m[ 3] - m[ 2];
			d[ 4] = m[ 4] + m[ 5];
			d[ 5] = m[ 4] - m[ 5];
			d[ 6] = m[ 6] + m[ 7];
			d[ 7] = m[ 7] - m[ 6];
			d[ 8] = m[ 8] + m[ 9];
			d[ 9] = m[ 8] - m[ 9];
			d[10] = m[10] + m[11];
			d[11] = m[11] - m[10];
			d[12] = m[12] + m[13];
			d[13] = m[12] - m[13];
			d[14] = m[14] + m[15];
			d[15] = m[15] - m[14];


			for (j = 0; j < 16; j++) {
				sad += (abs(d[j]));
			}

			sad = ((sad + 1) >> 1);

			sum_sad += sad;
		}
	}
	return sum_sad;
}
#endif

#if ZHANGYI_INTRA_SDIP
int calcHADSDIPVer(Pel* pi, UInt uiStride, int iWidth, int iHeight)
{
	int uiSum = 0, sad, sum_sad = 0;
	int x, y, i, j, jj;
	int diff[257], m1[4][16], m2[4][16];

	// Hadamard8x8
	for (y = 0; y < iHeight; y += 16) {
		for (x = 0; x < iWidth; x += 4) {
			sad = 0;

			for (j = 0; j < 16; j++) {
				for (i = 0; i < 4; i++) {
					diff[j * 4 + i] = pi[(y + j) * uiStride + (x + i)]; //pi[y + j][x + i];
				}
			}


			//horizontal
			for (j = 0; j < 4; j++) {
				jj = j << 4;

				m2[j][0] = diff[jj] + diff[jj + 8];
				m2[j][1] = diff[jj + 1] + diff[jj + 9];
				m2[j][2] = diff[jj + 2] + diff[jj + 10];
				m2[j][3] = diff[jj + 3] + diff[jj + 11];
				m2[j][4] = diff[jj + 4] + diff[jj + 12];
				m2[j][5] = diff[jj + 5] + diff[jj + 13];
				m2[j][6] = diff[jj + 6] + diff[jj + 14];
				m2[j][7] = diff[jj + 7] + diff[jj + 15];
				m2[j][8] = diff[jj] - diff[jj + 8];
				m2[j][9] = diff[jj + 1] - diff[jj + 9];
				m2[j][10] = diff[jj + 2] - diff[jj + 10];
				m2[j][11] = diff[jj + 3] - diff[jj + 11];
				m2[j][12] = diff[jj + 4] - diff[jj + 12];
				m2[j][13] = diff[jj + 5] - diff[jj + 13];
				m2[j][14] = diff[jj + 6] - diff[jj + 14];
				m2[j][15] = diff[jj + 7] - diff[jj + 15];

				m1[j][0] = m2[j][0] + m2[j][4];
				m1[j][1] = m2[j][1] + m2[j][5];
				m1[j][2] = m2[j][2] + m2[j][6];
				m1[j][3] = m2[j][3] + m2[j][7];
				m1[j][4] = m2[j][0] - m2[j][4];
				m1[j][5] = m2[j][1] - m2[j][5];
				m1[j][6] = m2[j][2] - m2[j][6];
				m1[j][7] = m2[j][3] - m2[j][7];
				m1[j][8] = m2[j][8] + m2[j][12];
				m1[j][9] = m2[j][9] + m2[j][13];
				m1[j][10] = m2[j][10] + m2[j][14];
				m1[j][11] = m2[j][11] + m2[j][15];
				m1[j][12] = m2[j][8] - m2[j][12];
				m1[j][13] = m2[j][9] - m2[j][13];
				m1[j][14] = m2[j][10] - m2[j][14];
				m1[j][15] = m2[j][11] - m2[j][15];

				m2[j][0] = m1[j][0] + m1[j][2];
				m2[j][1] = m1[j][1] + m1[j][3];
				m2[j][2] = m1[j][0] - m1[j][2];
				m2[j][3] = m1[j][1] - m1[j][3];
				m2[j][4] = m1[j][4] + m1[j][6];
				m2[j][5] = m1[j][5] + m1[j][7];
				m2[j][6] = m1[j][4] - m1[j][6];
				m2[j][7] = m1[j][5] - m1[j][7];
				m2[j][8] = m1[j][8] + m1[j][10];
				m2[j][9] = m1[j][9] + m1[j][11];
				m2[j][10] = m1[j][8] - m1[j][10];
				m2[j][11] = m1[j][9] - m1[j][11];
				m2[j][12] = m1[j][12] + m1[j][14];
				m2[j][13] = m1[j][13] + m1[j][15];
				m2[j][14] = m1[j][12] - m1[j][14];
				m2[j][15] = m1[j][13] - m1[j][15];

				m1[j][0] = m2[j][0] + m2[j][1];
				m1[j][1] = m2[j][0] - m2[j][1];
				m1[j][2] = m2[j][2] + m2[j][3];
				m1[j][3] = m2[j][2] - m2[j][3];
				m1[j][4] = m2[j][4] + m2[j][5];
				m1[j][5] = m2[j][4] - m2[j][5];
				m1[j][6] = m2[j][6] + m2[j][7];
				m1[j][7] = m2[j][6] - m2[j][7];
				m1[j][8] = m2[j][8] + m2[j][9];
				m1[j][9] = m2[j][8] - m2[j][9];
				m1[j][10] = m2[j][10] + m2[j][11];
				m1[j][11] = m2[j][10] - m2[j][11];
				m1[j][12] = m2[j][12] + m2[j][13];
				m1[j][13] = m2[j][12] - m2[j][13];
				m1[j][14] = m2[j][14] + m2[j][15];
				m1[j][15] = m2[j][14] - m2[j][15];
			}

			//vertical
			for (i = 0; i < 16; i++) {
				m2[0][i] = m1[0][i] + m1[2][i];
				m2[1][i] = m1[1][i] + m1[3][i];
				m2[2][i] = m1[0][i] - m1[2][i];
				m2[3][i] = m1[1][i] - m1[3][i];

				m1[0][i] = m2[0][i] + m2[1][i];
				m1[1][i] = m2[0][i] - m2[1][i];
				m1[2][i] = m2[2][i] + m2[3][i];
				m1[3][i] = m2[2][i] - m2[3][i];
			}


			for (j = 0; j < 4; j++) {
				for (i = 0; i < 16; i++) {
					sad += (abs(m1[j][i]));
				}
			}

			sad = ((sad + 2) >> 2);

			sum_sad += sad;
		}
	}
	return sum_sad;
}
int calcHADSDIPHor(Pel* pi, UInt uiStride, int iWidth, int iHeight)
{
	int uiSum = 0, sad, sum_sad = 0;
	int x, y, i, j, jj;
	int diff[257], m1[16][4], m2[16][4], m3[16][4];

	// Hadamard8x8
	for (y = 0; y < iHeight; y += 4) {
		for (x = 0; x < iWidth; x += 16) {
			sad = 0;

			for (j = 0; j < 4; j++) {
				for (i = 0; i < 16; i++) {
					diff[j * 16 + i] = pi[(y + j)*uiStride + (x + i)];//pi[y + j][x + i];
				}
			}

			//horizontal
			for (j = 0; j < 16; j++) {
				jj = j << 2;
				m2[j][0] = diff[jj] + diff[jj + 2];
				m2[j][1] = diff[jj + 1] + diff[jj + 3];
				m2[j][2] = diff[jj] - diff[jj + 2];
				m2[j][3] = diff[jj + 1] - diff[jj + 3];

				m1[j][0] = m2[j][0] + m2[j][1];
				m1[j][1] = m2[j][0] - m2[j][1];
				m1[j][2] = m2[j][2] + m2[j][3];
				m1[j][3] = m2[j][2] - m2[j][3];
			}

			//vertical
			for (i = 0; i < 4; i++) {
				m2[0][i] = m1[0][i] + m1[8][i];
				m2[1][i] = m1[1][i] + m1[9][i];
				m2[2][i] = m1[2][i] + m1[10][i];
				m2[3][i] = m1[3][i] + m1[11][i];
				m2[4][i] = m1[4][i] + m1[12][i];
				m2[5][i] = m1[5][i] + m1[13][i];
				m2[6][i] = m1[6][i] + m1[14][i];
				m2[7][i] = m1[7][i] + m1[15][i];
				m2[8][i] = m1[0][i] - m1[8][i];
				m2[9][i] = m1[1][i] - m1[9][i];
				m2[10][i] = m1[2][i] - m1[10][i];
				m2[11][i] = m1[3][i] - m1[11][i];
				m2[12][i] = m1[4][i] - m1[12][i];
				m2[13][i] = m1[5][i] - m1[13][i];
				m2[14][i] = m1[6][i] - m1[14][i];
				m2[15][i] = m1[7][i] - m1[15][i];

				m3[0][i] = m2[0][i] + m2[4][i];
				m3[1][i] = m2[1][i] + m2[5][i];
				m3[2][i] = m2[2][i] + m2[6][i];
				m3[3][i] = m2[3][i] + m2[7][i];
				m3[4][i] = m2[0][i] - m2[4][i];
				m3[5][i] = m2[1][i] - m2[5][i];
				m3[6][i] = m2[2][i] - m2[6][i];
				m3[7][i] = m2[3][i] - m2[7][i];
				m3[8][i] = m2[8][i] + m2[12][i];
				m3[9][i] = m2[9][i] + m2[13][i];
				m3[10][i] = m2[10][i] + m2[14][i];
				m3[11][i] = m2[11][i] + m2[15][i];
				m3[12][i] = m2[8][i] - m2[12][i];
				m3[13][i] = m2[9][i] - m2[13][i];
				m3[14][i] = m2[10][i] - m2[14][i];
				m3[15][i] = m2[11][i] - m2[15][i];

				m1[0][i] = m3[0][i] + m3[2][i];
				m1[1][i] = m3[1][i] + m3[3][i];
				m1[2][i] = m3[0][i] - m3[2][i];
				m1[3][i] = m3[1][i] - m3[3][i];
				m1[4][i] = m3[4][i] + m3[6][i];
				m1[5][i] = m3[5][i] + m3[7][i];
				m1[6][i] = m3[4][i] - m3[6][i];
				m1[7][i] = m3[5][i] - m3[7][i];
				m1[8][i] = m3[8][i] + m3[10][i];
				m1[9][i] = m3[9][i] + m3[11][i];
				m1[10][i] = m3[8][i] - m3[10][i];
				m1[11][i] = m3[9][i] - m3[11][i];
				m1[12][i] = m3[12][i] + m3[14][i];
				m1[13][i] = m3[13][i] + m3[15][i];
				m1[14][i] = m3[12][i] - m3[14][i];
				m1[15][i] = m3[13][i] - m3[15][i];

				m2[0][i] = m1[0][i] + m1[1][i];
				m2[1][i] = m1[0][i] - m1[1][i];
				m2[2][i] = m1[2][i] + m1[3][i];
				m2[3][i] = m1[2][i] - m1[3][i];
				m2[4][i] = m1[4][i] + m1[5][i];
				m2[5][i] = m1[4][i] - m1[5][i];
				m2[6][i] = m1[6][i] + m1[7][i];
				m2[7][i] = m1[6][i] - m1[7][i];
				m2[8][i] = m1[8][i] + m1[9][i];
				m2[9][i] = m1[8][i] - m1[9][i];
				m2[10][i] = m1[10][i] + m1[11][i];
				m2[11][i] = m1[10][i] - m1[11][i];
				m2[12][i] = m1[12][i] + m1[13][i];
				m2[13][i] = m1[12][i] - m1[13][i];
				m2[14][i] = m1[14][i] + m1[15][i];
				m2[15][i] = m1[14][i] - m1[15][i];
			}


			for (j = 0; j < 16; j++) {
				for (i = 0; i < 4; i++) {
					sad += (abs(m2[j][i]));
				}
			}

			sad = ((sad + 2) >> 2);

			sum_sad += sad;
		}
	}
	return sum_sad;
}
#endif


#if ZHANGYI_INTRA
UInt TEncSearch::xIntraSearch( TComDataCU* pcCU, TComYuv* pcOrgYuv, TComYuv* pcPredYuv, TComYuv* pcResiYuv, TComYuv* pcRecoYuv, UInt uiDir, UInt uiPartOffset, int* p_avail )
#else
UInt TEncSearch::xIntraSearch( TComDataCU* pcCU, TComYuv* pcOrgYuv, TComYuv* pcPredYuv, TComYuv* pcResiYuv, TComYuv* pcRecoYuv, UInt uiDir, UInt uiPartOffset )
#endif
{
  UInt    uiY, uiX;
  UInt    uiDepth           = pcCU->getDepth     ( 0 );
  UInt    uiPartDepth       = pcCU->getPartitionSize( 0 ) == SIZE_2Nx2N ? 0 : 1;
  UInt    uiLog2Size        = pcCU->getLog2CUSize( 0 ) - uiPartDepth;
  UInt    uiLog2SizeC       = uiLog2Size == 2 ? uiLog2Size : (uiLog2Size-1);

  // Luma variables
  UInt    uiSize            = 1<<uiLog2Size;
#if ZHANGYI_INTRA_SDIP
  UInt    uiSdipFlag = pcCU->getSDIPFlag(0); //zhangyiCheckSdip
  UInt    uiSdipDir = pcCU->getSDIPDirection(0); //zhangyiCheckSdip
  UInt    uiWidth = uiSdipFlag ? (uiSdipDir ? (uiSize << 1) : (uiSize >> 1)) : uiSize;
  UInt    uiHeight = uiSdipFlag ? (uiSdipDir ? (uiSize >> 1) : (uiSize << 1)) : uiSize;
#endif
  UInt    uiStride          = pcOrgYuv ->getStride  ();

  UInt    uiAbsZorderIdx    = pcCU     ->getZorderIdxInCU();

  UInt uiMinCoeffSize = 1<<(pcCU->getPic()->getLog2MinCUSize()<<1);
  UInt uiLumaCoeffOffset   = uiMinCoeffSize*(uiPartOffset);
  UInt uiChromaCoeffOffset = uiLumaCoeffOffset>>2;


#if ZHANGYI_INTRA_SDIP
  Pel *piOrgY, *piResiY, *piRecoY, *piPredY;
  if (uiSdipFlag)
  {
	  UInt uiLine = pcCU->convertNonSquareUnitToLine(uiPartOffset);

	  piOrgY = pcOrgYuv->getLumaAddr(uiAbsZorderIdx);
	  piOrgY += (uiSdipDir ? (uiLine * pcOrgYuv->getStride()) : uiLine);
	  piResiY = pcResiYuv->getLumaAddr(uiAbsZorderIdx);
	  piResiY += (uiSdipDir ? (uiLine * pcResiYuv->getStride()) : uiLine);
	  piRecoY = pcRecoYuv->getLumaAddr(uiAbsZorderIdx);
	  piRecoY += (uiSdipDir ? (uiLine * pcRecoYuv->getStride()) : uiLine);
	  piPredY = pcPredYuv->getLumaAddr(uiAbsZorderIdx);
	  piPredY += (uiSdipDir ? (uiLine * pcPredYuv->getStride()) : uiLine);

  }
  else
  {
	  piOrgY = pcOrgYuv->getLumaAddr(uiAbsZorderIdx + uiPartOffset);
	  piResiY = pcResiYuv->getLumaAddr(uiAbsZorderIdx + uiPartOffset);
	  piRecoY = pcRecoYuv->getLumaAddr(uiAbsZorderIdx + uiPartOffset);
	  piPredY = pcPredYuv->getLumaAddr(uiAbsZorderIdx + uiPartOffset);
  }
#else
  Pel*    piOrgY            = pcOrgYuv ->getLumaAddr( uiAbsZorderIdx + uiPartOffset );
  Pel*    piResiY           = pcResiYuv->getLumaAddr( uiAbsZorderIdx + uiPartOffset );
  Pel*    piRecoY           = pcRecoYuv->getLumaAddr( uiAbsZorderIdx + uiPartOffset );
  Pel*    piPredY           = pcPredYuv->getLumaAddr( uiAbsZorderIdx + uiPartOffset );
#endif


  UInt    uiRecStride       = pcRecoYuv->getStride();

  TCoeff* pcCoeffY          = pcCU     ->getCoeffY() + uiLumaCoeffOffset;

  UInt    uiAbsSumY         = 0;

  UInt    uiDistortion;

  // Chroma variables
  UInt    uiSizeC           = 1<<uiLog2SizeC;
  UInt    uiStrideC         = pcOrgYuv ->getCStride ();

  Pel*    piOrgCb           = pcOrgYuv ->getCbAddr( uiAbsZorderIdx + uiPartOffset );
  Pel*    piResiCb          = pcResiYuv->getCbAddr( uiAbsZorderIdx + uiPartOffset );
  Pel*    piRecoCb          = pcRecoYuv->getCbAddr( uiAbsZorderIdx + uiPartOffset );
  Pel*    piPredCb          = pcPredYuv->getCbAddr( uiAbsZorderIdx + uiPartOffset );

  Pel*    piOrgCr           = pcOrgYuv ->getCrAddr( uiAbsZorderIdx + uiPartOffset );
  Pel*    piResiCr          = pcResiYuv->getCrAddr( uiAbsZorderIdx + uiPartOffset );
  Pel*    piRecoCr          = pcRecoYuv->getCrAddr( uiAbsZorderIdx + uiPartOffset );
  Pel*    piPredCr          = pcPredYuv->getCrAddr( uiAbsZorderIdx + uiPartOffset );

  UInt    uiRecStrideC      = pcRecoYuv->getCStride();

  TCoeff* pcCoeffCb         = pcCU     ->getCoeffCb () + uiChromaCoeffOffset;
  TCoeff* pcCoeffCr         = pcCU     ->getCoeffCr () + uiChromaCoeffOffset;

  UInt uiAbsSumCb = 0;
  UInt uiAbsSumCr = 0;

  // ========== Luma ========== //
  // get Luma prediction

#if ZHANGYI_INTRA
#if ZHANGYI_INTRA_SDIP
  predIntraAngAVS(TEXT_LUMA, uiDir, uiStride, piPredY, uiStride, uiWidth, uiHeight, p_avail[NEIGHBOR_INTRA_UP], p_avail[NEIGHBOR_INTRA_LEFT], pcCU->getPicture()->getSPS()->getBitDepth());
#else
  predIntraAngAVS(TEXT_LUMA, uiDir, uiStride, piPredY, uiStride, uiSize, p_avail[NEIGHBOR_INTRA_UP], p_avail[NEIGHBOR_INTRA_LEFT], pcCU->getPicture()->getSPS()->getBitDepth());
#endif
#else
  predIntra( piRecoY,  uiRecStride,  piPredY,  uiStride,  uiSize,  uiDir );
#endif

  // get Luma residual
#if ZHANGYI_INTRA_SDIP
  for (uiY = 0; uiY < uiHeight; uiY++)
#else
  for (uiY = 0; uiY < uiSize; uiY++)
#endif
  {
    UInt uiY2 = uiY*uiStride;
#if ZHANGYI_INTRA_SDIP
	for (uiX = 0; uiX < uiWidth; uiX++)
#else
	for (uiX = 0; uiX < uiSize; uiX++)
#endif
    {
      UInt ui = uiY2 + uiX;
      piResiY[ ui ] = piOrgY[ ui ] - piPredY[ ui ];
    }
  }

  //===== transform and quantization =====
  if( m_pcEncCfg->getUseRDOQ() )
  {
    m_pcEntropyCoder->estimateBit( m_pcTrQuant->m_pcEstBitsSbac, uiSize, TEXT_LUMA );
  }

  //--- transform and quantization --- Luma
  m_pcTrQuant->setQPforQuant  ( TEXT_LUMA );
#if ZHOULULU_QT
  m_pcTrQuant->setBlockSize( pcCU, uiLog2Size, TEXT_LUMA, 0 );
#if ZHOULULU_SEC_TRANS
  m_pcTrQuant->transformMxN ( pcCU, piResiY, uiStride, pcCoeffY, uiLog2Size, uiAbsSumY, TEXT_LUMA, uiPartOffset );
//  m_pcTrQuant->transformMxN ( pcCU, m_pcEntropyCoder->m_pcEntropyCoderIf, piResiY, uiStride, pcCoeffY, uiLog2Size, uiAbsSumY, TEXT_LUMA, uiPartOffset );
#else
  m_pcTrQuant->transformMxN ( pcCU, piResiY, uiStride, pcCoeffY, uiLog2Size, uiAbsSumY, TEXT_LUMA, 0 );
#endif
#else
  m_pcTrQuant->transformNxN   ( pcCU, piResiY, uiStride, pcCoeffY, uiLog2Size, uiAbsSumY, TEXT_LUMA, 0 );
 #endif
 
#if ZHOULULU_QT
#if ZHOULULU_SEC_TRANS
  m_pcTrQuant->invtransformMxN( pcCU, piResiY, uiStride, pcCoeffY, uiLog2Size, TEXT_LUMA, uiPartOffset );
#else
  m_pcTrQuant->invtransformMxN( pcCU, piResiY, uiStride, pcCoeffY, uiLog2Size, TEXT_LUMA, 0 );
#endif
#else
  m_pcTrQuant->invtransformNxN( piResiY, uiStride, pcCoeffY, uiLog2Size, TEXT_LUMA );
#endif
  //--- set coded block flag ---
  pcCU->setCbfSubParts( ( uiAbsSumY ? 1 : 0 ) << uiPartDepth, TEXT_LUMA, uiPartOffset, uiDepth+uiPartDepth );

  if( uiPartDepth == 1 )
  {
    pcCU->setCuCbfLuma(uiPartOffset, uiPartDepth, uiPartDepth);
  }

  //===== reconstruction =====
  Pel* pRecoY  = piRecoY;
#if ZHANGYI_INTRA_SDIP
  for (uiY = 0; uiY < uiHeight; uiY++)
  {
	  for (uiX = 0; uiX < uiWidth; uiX++)
	  {
#else
  for (uiY = 0; uiY < uiSize; uiY++)
  {
	  for (uiX = 0; uiX < uiSize; uiX++)
	  {
#endif
      pRecoY[ uiX ] = Clip( piPredY[ uiX ] + piResiY[ uiX ] );
    }
    pRecoY  += uiRecStride;
    piPredY += uiStride;
    piResiY += uiStride;
  }

  // update distortion
  #if ZHANGYI_INTRA_SDIP_BUG_YQH
  #if ZHANGYI_INTRA_SDIP
  uiDistortion = m_pcRdCost->getDistPart( piRecoY, uiRecStride, piOrgY, uiStride, uiWidth, uiHeight );
#else
  uiDistortion = m_pcRdCost->getDistPart(piRecoY, uiRecStride, piOrgY, uiStride, 1 << uiLog2Size, 1 << uiLog2Size);
#endif
#else
  uiDistortion = m_pcRdCost->getDistPart( piRecoY, uiRecStride, piOrgY, uiStride, 1<<uiLog2Size, 1<<uiLog2Size );
#endif
  // ========== Chroma ========== //
#if !ZHANGYI_INTRA
  if( !(uiLog2Size == 2 && uiPartOffset > 0) ) // only 1 chroma PU is allowed when 8x8 luma CU has SIZE_NxN partition mode
  {
  // get Chroma prediction
  predIntra( piRecoCb,  uiRecStrideC,  piPredCb,  uiStrideC,  uiSizeC,  uiDir );
  predIntra( piRecoCr,  uiRecStrideC,  piPredCr,  uiStrideC,  uiSizeC,  uiDir );

  // get Chroma residual
  for( uiY = 0; uiY < uiSizeC; uiY++ )
  {
    UInt uiY2 = uiY*uiStrideC;
    for( uiX = 0; uiX < uiSizeC; uiX++ )
    {
      UInt ui = uiY2 + uiX;
      piResiCb[ ui ] = piOrgCb[ ui ] - piPredCb[ ui ];
      piResiCr[ ui ] = piOrgCr[ ui ] - piPredCr[ ui ];
    }
  }

  //--- transform and quantization --- Chroma
  m_pcTrQuant->setQPforQuant  ( TEXT_CHROMA );


  ///////////////////////////
  #if ZHOULULU_QT
		m_pcTrQuant->setBlockSize( pcCU, uiLog2SizeC, TEXT_CHROMA, 0 );
		m_pcTrQuant->transformMxN   ( pcCU, piResiCb, uiStrideC, pcCoeffCb, uiLog2SizeC, uiAbsSumCb, TEXT_CHROMA_U, 0 );
		m_pcTrQuant->transformMxN   ( pcCU, piResiCr, uiStrideC, pcCoeffCb, uiLog2SizeC, uiAbsSumCb, TEXT_CHROMA_U, 0 );
		m_pcTrQuant->invtransformMxN( pcCU,piResiCb, uiStrideC, pcCoeffCb, uiLog2SizeC, TEXT_CHROMA_U, 0 );
		m_pcTrQuant->invtransformMxN( pcCU,piResiCr, uiStrideC, pcCoeffCb, uiLog2SizeC, TEXT_CHROMA_U, 0 );
#else
  ///////////////////
  m_pcTrQuant->transformNxN   ( pcCU, piResiCb, uiStrideC, pcCoeffCb, uiLog2SizeC, uiAbsSumCb, TEXT_CHROMA_U, 0 );
  m_pcTrQuant->transformNxN   ( pcCU, piResiCr, uiStrideC, pcCoeffCr, uiLog2SizeC, uiAbsSumCr, TEXT_CHROMA_V, 0 );
  m_pcTrQuant->invtransformNxN( piResiCb, uiStrideC, pcCoeffCb, uiLog2SizeC, TEXT_CHROMA_U );
  m_pcTrQuant->invtransformNxN( piResiCr, uiStrideC, pcCoeffCr, uiLog2SizeC, TEXT_CHROMA_V );
#endif
  //--- set coded block flag ---
  pcCU->setCbfSubParts( ( uiAbsSumCb ? 1 : 0 ) << uiPartDepth, TEXT_CHROMA_U, uiPartOffset, uiDepth+uiPartDepth );
  pcCU->setCbfSubParts( ( uiAbsSumCr ? 1 : 0 ) << uiPartDepth, TEXT_CHROMA_V, uiPartOffset, uiDepth+uiPartDepth );

  if( uiPartDepth == 1 )
  {
    pcCU->setCuCbfChroma(uiPartOffset, uiPartDepth, uiPartDepth);
  }

  Pel* pRecoCb = piRecoCb;
  Pel* pRecoCr = piRecoCr;
  for( uiY = 0; uiY < uiSizeC; uiY++ )
  {
    for( uiX = 0; uiX < uiSizeC; uiX++ )
    {
      pRecoCb[ uiX ] = Clip( piPredCb[ uiX ] + piResiCb[ uiX ] );
      pRecoCr[ uiX ] = Clip( piPredCr[ uiX ] + piResiCr[ uiX ] );
    }
    pRecoCb  += uiRecStrideC; pRecoCr  += uiRecStrideC;
    piPredCb += uiStrideC; piPredCr += uiStrideC;
    piResiCb += uiStrideC; piResiCr += uiStrideC;
  }

  // update distortion
  uiDistortion += (UInt)( m_pcRdCost->getChromaWeight()*(Double)( m_pcRdCost->getDistPart( piRecoCb, uiRecStrideC, piOrgCb, uiStrideC, 1<<(uiLog2SizeC), 1<<(uiLog2SizeC) )
                                                                + m_pcRdCost->getDistPart( piRecoCr, uiRecStrideC, piOrgCr, uiStrideC, 1<<(uiLog2SizeC), 1<<(uiLog2SizeC) ) ) );
  }
#endif
  return uiDistortion;
}

#if ZHANGYI_INTRA
#if !YQH_INTRA_LAMDA
Void TEncSearch::estIntraPredChromaQT( UInt uiPartOffset, TComDataCU* pcCU, TComYuv*    pcOrgYuv, TComYuv*    pcPredYuv, TComYuv*    pcResiYuv,TComYuv*    pcRecoYuv, UInt* uiBitsChroma, UInt* uiDistortionChroma, Double* dCostChroma)
#else
Void TEncSearch::estIntraPredChromaQT(UInt uiPUBestBits, UInt uiPUBestDistortion,UInt uiPartOffset, TComDataCU* pcCU, TComYuv*    pcOrgYuv, TComYuv*    pcPredYuv, TComYuv*    pcResiYuv,TComYuv*    pcRecoYuv, UInt* uiBitsChroma, UInt* uiDistortionChroma, Double* dCostChroma)	
#endif
{
	int p_avail[5];
	int luma_pic_pix_x = pcCU->getCUPelX();
	int luma_pic_pix_y = pcCU->getCUPelY();
	int lumaWidth = pcCU->getWidth(0);
	int lumaHeight = pcCU->getHeight(0);
	int bitDepth = pcCU->getPicture()->getSPS()->getBitDepth();
	UInt    uiAbsZorderIdx    = pcCU     ->getZorderIdxInCU();
	//UInt    uiPartOffset = 0;
	UInt    uiDepth           = pcCU->getDepth     ( 0 );
	UInt    uiPartDepth       = pcCU->getPartitionSize( 0 ) == SIZE_2Nx2N ? 0 : 1;
	UInt    uiQNumParts    = pcCU->getTotalNumPart()>>(uiPartDepth<<1);
	UInt    uiMaxTrDepth = (pcCU->getPartitionSize(0) == SIZE_NxN) ? 1 : 0;
	UInt    uiLog2Size        = pcCU->getLog2CUSize( 0 ) - uiPartDepth;
	UInt    uiLog2SizeC      =  pcCU->getLog2CUSize( 0 ) - 1;
	//UInt    uiLog2SizeC       = uiLog2Size == 2 ? uiLog2Size : (uiLog2Size-1);
	UInt    uiSizeC           = 1<<uiLog2SizeC;
	UInt    uiStrideC         = pcOrgYuv ->getCStride (); 
	UInt    uiRecStrideC   = pcRecoYuv->getCStride();

	Pel*    piOrgCb           = pcOrgYuv ->getCbAddr( uiAbsZorderIdx + uiPartOffset );
	Pel*    piResiCb          = pcResiYuv->getCbAddr( uiAbsZorderIdx + uiPartOffset );
	Pel*    piRecoCb          = pcRecoYuv->getCbAddr( uiAbsZorderIdx + uiPartOffset );
	Pel*    piPredCb          = pcPredYuv->getCbAddr( uiAbsZorderIdx + uiPartOffset );

	Pel*    piOrgCr           = pcOrgYuv ->getCrAddr( uiAbsZorderIdx + uiPartOffset );
	Pel*    piResiCr          = pcResiYuv->getCrAddr( uiAbsZorderIdx + uiPartOffset );
	Pel*    piRecoCr          = pcRecoYuv->getCrAddr( uiAbsZorderIdx + uiPartOffset );
	Pel*    piPredCr          = pcPredYuv->getCrAddr( uiAbsZorderIdx + uiPartOffset );

	Pel*    piPredCbPic    = pcCU->getPic()->getPicYuvRec()->getCbAddr((uiAbsZorderIdx>>2) + uiPartOffset);
	Pel*     piPredCrPic     = pcCU->getPic()->getPicYuvRec()->getCrAddr((uiAbsZorderIdx>>2) + uiPartOffset);
	UInt     uiStrideCPic    = pcCU->getPic()->getPicYuvRec()->getCStride();

	UInt uiMinCoeffSize = 1<<(pcCU->getPic()->getLog2MinCUSize()<<1);
	UInt uiLumaCoeffOffset   = uiMinCoeffSize*(uiPartOffset);
	UInt uiChromaCoeffOffset = uiLumaCoeffOffset>>2;
	TCoeff* pcCoeffCb         = pcCU     ->getCoeffCb () + uiChromaCoeffOffset;
	TCoeff* pcCoeffCr         = pcCU     ->getCoeffCr () + uiChromaCoeffOffset;

	UInt uiAbsSumCb = 0;
	UInt uiAbsSumCr = 0;

	UInt uiDistortionCb, uiDistortionCr;
	UInt uiBitsCb, uiBitsCr;
	Double  dCostCb, dCostCr;
	UInt uiPUBestDirCb;
	UInt uiPUBestBitsCb;

#if niu_ChromaRDO_revise
	UInt TotalBitsBest;
	UInt TotalDistortionBest;
	Double TotalCostBest = MAX_DOUBLE;
#endif

	UInt uiPUBestDistortionCb;
	Double  dPUBestCostCb    = MAX_DOUBLE;
	UInt uiPUBestDirCr = 0;
	UInt uiPUBestBitsCr = 0;
	UInt uiPUBestDistortionCr = 0;
	Double  dPUBestCostCr    = 0;//;yuquanhe@hisilicon.comMAX_DOUBLE;
	UInt    uiNextDepth    = uiDepth + 1;

	Pel *piAdiTempCb   = m_piYuvExt2g[COMPONENT_Cb][PRED_BUF_UNFILTERED];
	Pel *piAdiTempCr    = m_piYuvExt2g[COMPONENT_Cr][PRED_BUF_UNFILTERED];

	Int uiModeTemp;

	UInt LumaMode = pcCU->getIntraDir(0);
	UInt lmode;
	Bool is_redundant = false;
	if (LumaMode == INTRA_DC_IDX || LumaMode == INTRA_BI_IDX || LumaMode == INTRA_VER_IDX || LumaMode == INTRA_HOR_IDX)
	{
		is_redundant = true;
		lmode = LumaMode == INTRA_VER_IDX ? INTRA_VERT_PRED_CHROMA : (LumaMode == INTRA_HOR_IDX ? INTRA_HOR_PRED_CHROMA : (LumaMode == INTRA_DC_IDX ? INTRA_DC_PRED_CHROMA : INTRA_BI_PRED_CHROMA));
	}

	/*Chroma*/
#if niu_intra_pavil
	getIntraNeighborAvailabilities(pcCU, (1 << g_uiLog2MaxCUSize), luma_pic_pix_x, luma_pic_pix_y, lumaWidth, lumaHeight, p_avail);
#else
	getIntraNeighborAvailabilities(pcCU,  (1<<B64X64_IN_BIT), luma_pic_pix_x, luma_pic_pix_y, lumaWidth, lumaHeight, p_avail);
#endif	
	fillReferenceSamples(bitDepth, pcCU, piRecoCb, piAdiTempCb, lumaWidth >> 1, lumaHeight >> 1, uiRecStrideC, p_avail, luma_pic_pix_x >> 1, luma_pic_pix_y >> 1);
#if ZHANGYI_INTRA
	fillReferenceSamples(bitDepth, pcCU, piRecoCr, piAdiTempCr, lumaWidth >> 1, lumaHeight >> 1, uiRecStrideC, p_avail, luma_pic_pix_x >> 1, luma_pic_pix_y >> 1);
#endif
	 for (UInt uiMode = 0; uiMode < INTRA_NUM_CHROMA; uiMode++)
	{




#if niu_set_intra_premode_debug
		if (uiMode != 0)
			continue;
#endif




#if YQH_AVS3_DIR_RDO_BUG
		if (uiMode != INTRA_DM_PRED_CHROMA && (uiMode == lmode) && is_redundant)
			continue;
#endif


#if 0//intra_premode_debug_YQH
		if (uiMode != 1)
			continue;

#endif


		//----- restore context models -----
#if YQH_RDOINTRA_BUG
#if niu_state_test
		m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
#else
		m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiNextDepth][CI_NEXT_BEST]);
#endif
#else
		m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
#endif




		uiDistortionCb = xIntraSearchChroma(COMPONENT_Cb ,pcCU, pcOrgYuv, pcPredYuv, pcResiYuv, pcRecoYuv, uiMode, 0, p_avail);//色度块不划分
#if ZHANGYI_INTRA_MODIFY
		uiDistortionCr = xIntraSearchChroma(COMPONENT_Cr, pcCU, pcOrgYuv, pcPredYuv, pcResiYuv, pcRecoYuv, uiMode, 0, p_avail);
#endif

#if !YQH_AVS3_DIR_RDO_BUG
		if ((uiMode == lmode) && is_redundant)
		{
			uiModeTemp = INTRA_DM_PRED_CHROMA;
		}
		else
		{
			uiModeTemp = uiMode;
		}
#else
		uiModeTemp = uiMode;
#endif


#if !niu_ChromaRDO_revise
		xAddSymbolBitsIntra(COMPONENT_Cb, pcCU, 0, uiQNumParts, 0, 0, 0, uiLog2SizeC + 1, uiBitsCb, uiModeTemp);//色度块不划分
#endif


#if ZHANGYI_INTRA_MODIFY
#if !niu_ChromaRDO_revise
		xAddSymbolBitsIntra(COMPONENT_Cr, pcCU, 0, uiQNumParts, 0, 0, 0, uiLog2SizeC + 1, uiBitsCr, uiModeTemp);
#endif

#if niu_ChromaRDO_revise	
		UInt TotalBits;
		xAddSymbolBitsIntra_coeff(COMPONENT_Y, pcCU, uiPartOffset, uiQNumParts, 0, 0, 0, uiLog2SizeC + 1, TotalBits, uiModeTemp);
#endif

		// dCostCb = m_pcRdCost->calcRdCost( (uiBitsCb + uiBitsCr), (uiDistortionCb + uiDistortionCr));
#if niu_ChromaRDO_revise
		dCostCb = m_pcRdCost->calcRdCost( TotalBits, (uiDistortionCb + uiDistortionCr+uiPUBestDistortion));

#else
		dCostCb = m_pcRdCost->calcRdCost( (uiBitsCb + uiBitsCr+uiPUBestBits), (uiDistortionCb + uiDistortionCr+uiPUBestDistortion));
#endif

#if	AVS3_Intra_print_Chroma
		printf("Bits=%d  Dist=%d  Cost=%f\n", TotalBits, (uiDistortionCb + uiDistortionCr + uiPUBestDistortion), dCostCb);
#endif
#else
		dCostCb = m_pcRdCost->calcRdCost( uiBitsCb, uiDistortionCb );
#endif
		if( dCostCb < dPUBestCostCb )
		{
			uiPUBestDirCb        = uiMode;
#if niu_ChromaRDO_revise
			TotalBitsBest = TotalBits;
			TotalDistortionBest = uiDistortionCb + uiDistortionCr + uiPUBestDistortion;
			TotalCostBest = dCostCb;
			dPUBestCostCb        = dCostCb;
#else
			uiPUBestBitsCb       = uiBitsCb;
			uiPUBestDistortionCb = uiDistortionCb;
#if YQH_AVS3_DIR_RDO_BUG
			uiPUBestBitsCr = uiBitsCr;
			uiPUBestDistortionCr = uiDistortionCr;
#endif
			dPUBestCostCb        = dCostCb;
#endif
#if niu_state_test
#else
			m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[uiNextDepth][CI_TEMP_BEST] );
#endif
		}
	}//end of CHROMA loop
#if !YQH_AVS3_DIR_RDO_BUG
	if ((uiPUBestDirCb == lmode) && is_redundant)
	{
		uiPUBestDirCb = INTRA_DM_PRED_CHROMA;
	}
#endif

#if niu_state_test
	m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiNextDepth][CI_CURR_BEST]);
#endif

	pcCU->setIntraDirSubPartsCb(uiPUBestDirCb, uiPartOffset, uiDepth);
#if ZHANGYI_INTRA_MODIFY
    pcCU->setIntraDirSubPartsCr(uiPUBestDirCb, uiPartOffset, uiDepth);
	xIntraSearchChroma(COMPONENT_Cr, pcCU, pcOrgYuv, pcPredYuv, pcResiYuv, pcRecoYuv, uiPUBestDirCb, 0, p_avail );
#if YQH_RDO_CHROMA_BUG
	xAddSymbolBitsIntra(COMPONENT_Cb, pcCU, 0, uiQNumParts, 0, 0, 0, uiLog2SizeC + 1, uiPUBestBitsCb, uiPUBestDirCb);//色度块不划分
#endif
#endif
	xIntraSearchChroma(COMPONENT_Cb, pcCU, pcOrgYuv, pcPredYuv, pcResiYuv, pcRecoYuv, uiPUBestDirCb, 0, p_avail );
#if YQH_RDO_CHROMA_BUG
	xAddSymbolBitsIntra(COMPONENT_Cr, pcCU, 0, uiQNumParts, 0, 0, 0, uiLog2SizeC + 1, uiPUBestBitsCr, uiPUBestDirCb);
#endif
#if niu_state_test
#else
	m_pppcRDSbacCoder[uiNextDepth][CI_TEMP_BEST]->store( m_pppcRDSbacCoder[uiNextDepth][CI_NEXT_BEST] );
#endif
	/*Cr*/
#if !ZHANGYI_INTRA_MODIFY
	getIntraNeighborAvailabilities(pcCU, (1<<B64X64_IN_BIT), luma_pic_pix_x, luma_pic_pix_y, lumaWidth, lumaHeight, p_avail);
	fillReferenceSamples(bitDepth, pcCU, piRecoCr, piAdiTempCr, lumaWidth >> 1, lumaHeight >> 1, uiRecStrideC, p_avail, luma_pic_pix_x >> 1, luma_pic_pix_y >> 1);
	for (UInt uiMode = 0; uiMode < INTRA_NUM_CHROMA; uiMode++)
	{
		//----- restore context models -----
		m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);

		uiDistortionCr = xIntraSearchChroma(COMPONENT_Cr, pcCU, pcOrgYuv, pcPredYuv, pcResiYuv, pcRecoYuv, uiMode, 0, p_avail);//色度块不划分
		//calculate bitrate for this mode of Cr

		if ((uiMode == lmode) && is_redundant)
		{
			uiModeTemp = INTRA_DM_PRED_CHROMA;
		}
		else
		{
			uiModeTemp = uiMode;
		}
		xAddSymbolBitsIntra(COMPONENT_Cr, pcCU, 0, uiQNumParts, 0, uiMaxTrDepth, 0, uiLog2SizeC + 1, uiBitsCr,uiModeTemp);//色度块不划分
		dCostCr = m_pcRdCost->calcRdCost( uiBitsCr, uiDistortionCr );
		if( dCostCr < dPUBestCostCr )
		{
			uiPUBestDirCr        = uiMode;
			uiPUBestBitsCr       = uiBitsCr;
			uiPUBestDistortionCr = uiDistortionCr;
			dPUBestCostCr        = dCostCr;
			m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[uiNextDepth][CI_TEMP_BEST] );
		}
	}//end of Cr loop
	if ((uiPUBestDirCr == lmode) && is_redundant)
	{
		uiPUBestDirCr = INTRA_DM_PRED_CHROMA;
	}
	pcCU->setIntraDirSubPartsCr(uiPUBestDirCr, uiPartOffset, uiDepth);
	xIntraSearchChroma(COMPONENT_Cr, pcCU, pcOrgYuv, pcPredYuv, pcResiYuv, pcRecoYuv, uiPUBestDirCr, 0, p_avail );
	m_pppcRDSbacCoder[uiNextDepth][CI_TEMP_BEST]->store( m_pppcRDSbacCoder[uiNextDepth][CI_NEXT_BEST] );
#endif

#if niu_ChromaRDO_revise
	(*uiBitsChroma) = TotalBitsBest;
	(*uiDistortionChroma) = TotalDistortionBest;
	(*dCostChroma) = TotalCostBest;
#else
	(*uiBitsChroma) = uiPUBestBitsCr + uiPUBestBitsCb;
	(*uiDistortionChroma ) = uiPUBestDistortionCb + uiPUBestDistortionCr;
	(*dCostChroma) = dPUBestCostCr + dPUBestCostCb;
#endif

}
#endif

Void TEncSearch::predIntraSearch( TComDataCU* pcCU, TComYuv* pcOrgYuv, TComYuv* pcPredYuv, TComYuv* pcResiYuv, TComYuv* pcRecoYuv )
{//pcCU is a sub-cu
  UInt    uiDepth        = pcCU->getDepth(0);
  UInt    uiNextDepth    = uiDepth + 1;
  UInt    uiNumPU        = pcCU->getNumPartInter();
  UInt    uiPU;
  UInt    uiPartDepth    = pcCU->getPartitionSize(0) == SIZE_2Nx2N ? 0 : 1;
  UInt    uiQNumParts    = pcCU->getTotalNumPart()>>(uiPartDepth<<1);

#if ZHANGYI_INTRA_SDIP
  UInt uiSdipFlag = pcCU->getSDIPFlag(0);
  UInt uiSdipDir = pcCU->getSDIPDirection(0);
  UInt    uiWidth = pcCU->getWidth(0) >> (uiSdipFlag ? (uiSdipDir ? 0 : 2 * uiPartDepth) : uiPartDepth); //PU width
  UInt    uiHeight = pcCU->getHeight(0) >> (uiSdipFlag ? (uiSdipDir ? 2 * uiPartDepth : 0) : uiPartDepth);
#else
  UInt    uiWidth = pcCU->getWidth(0) >> uiPartDepth;
  UInt    uiHeight = pcCU->getHeight(0) >> uiPartDepth;
#endif
  UInt    uiCoeffSize    = uiWidth*uiHeight;
  UInt    uiLog2Size     = pcCU->getLog2CUSize(0) - uiPartDepth;

  UInt   uiPartOffset = 0;
  UInt   uiCoeffOffset = 0;

  Double  dBestCost      = 0;
  Double  dPUBestCost    = MAX_DOUBLE;
  Double  dCost;

  UInt    uiDir;
  UInt    uiPUBestDir;

  UInt    uiBits;
  UInt    uiBestBits       = 0;
  UInt    uiPUBestBits     = 0;

  UInt    uiDistortion;
  UInt    uiBestDistortion   = 0;
  UInt    uiPUBestDistortion = 0;

#if ZHANGYI_INTRA_SDIP
  UInt    uiMaxTrDepth = (pcCU->getPartitionSize(0) == SIZE_2Nx2N) ? 0 : 1;
#else
  UInt    uiMaxTrDepth = (pcCU->getPartitionSize(0) == SIZE_NxN) ? 1 : 0;
#endif
#if ZHANGYI_INTRA
  Int upMode;
  Int leftMode;
  int mostProbableMode[2];
  UInt    uiAbsZorderIdx    = (pcCU->getZorderIdxInCU()); //m_uiAbsIdxInLCU

  UInt    uiStride          = (pcOrgYuv ->getStride  ());
  UInt    uiRecStride       = (pcRecoYuv->getStride());
  //UInt    uiPartDepth       = pcCU->getPartitionSize( 0 ) == SIZE_2Nx2N ? 0 : 1;
  //UInt    uiLog2Size        = pcCU->getLog2CUSize( 0 ) - uiPartDepth;
  UInt    uiSize = (1 << uiLog2Size);//PU大小
  DistParam distParam;
  int    CandModeList[ INTRA_NUM_FULL_RD ];
  double CandCostList[ INTRA_NUM_FULL_RD ];
  UInt        LeftPartIdx  = MAX_UINT;
  UInt        AbovePartIdx = MAX_UINT;
  int p_avail[5];
  Int bitDepth = pcCU->getPicture()->getSPS()->getBitDepth();
  Pel *piAdiTemp   = m_piYuvExt2g[COMPONENT_Y][PRED_BUF_UNFILTERED];
#endif
  pcCU->setTrIdxSubParts( uiMaxTrDepth, 0, uiDepth );

  xInitIntraSearch( pcCU, pcRecoYuv );

#if niu_ChromaRDO_revise
  for (int m = 0; m<4; m++)
	  MProbableMode[m] = -2;
#endif

  for( uiPU = 0; uiPU < uiNumPU; uiPU++ )
  {
    dPUBestCost = MAX_DOUBLE;

#if ZHANGYI_INTRA
	/*MPM的构建*/
#if ZHANGYI_INTRA_SDIP
	Pel *piOrgY, *piResiY, *piRecoY, *piPredY;
	UInt uiLine;
	if (uiSdipFlag)
	{
		uiLine = pcCU->convertNonSquareUnitToLine(uiPartOffset);

		piOrgY = pcOrgYuv->getLumaAddr(uiAbsZorderIdx);
		piOrgY += (uiSdipDir ? (uiLine * pcOrgYuv->getStride()) : uiLine);
		piResiY = pcResiYuv->getLumaAddr(uiAbsZorderIdx);
		piResiY += (uiSdipDir ? (uiLine * pcResiYuv->getStride()) : uiLine);
		piRecoY = pcRecoYuv->getLumaAddr(uiAbsZorderIdx);
		piRecoY += (uiSdipDir ? (uiLine * pcRecoYuv->getStride()) : uiLine);
		piPredY = pcPredYuv->getLumaAddr(uiAbsZorderIdx);
		piPredY += (uiSdipDir ? (uiLine * pcPredYuv->getStride()) : uiLine);

	}
	else
	{
		piOrgY = pcOrgYuv->getLumaAddr(uiAbsZorderIdx + uiPartOffset);
		piResiY = pcResiYuv->getLumaAddr(uiAbsZorderIdx + uiPartOffset);
		piRecoY = pcRecoYuv->getLumaAddr(uiAbsZorderIdx + uiPartOffset);
		piPredY = pcPredYuv->getLumaAddr(uiAbsZorderIdx + uiPartOffset);
	}
#else
	Pel*    piOrgY            = pcOrgYuv ->getLumaAddr( uiAbsZorderIdx + uiPartOffset );
	Pel*    piResiY           = pcResiYuv->getLumaAddr( uiAbsZorderIdx + uiPartOffset );
	Pel*    piRecoY           = pcRecoYuv->getLumaAddr( uiAbsZorderIdx + uiPartOffset );
	Pel*    piPredY           = pcPredYuv->getLumaAddr( uiAbsZorderIdx + uiPartOffset );
#endif

#if ZHANGYI_INTRA_SDIP
	Pel*   piRecIPred = pcCU->getPic()->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), uiAbsZorderIdx);
	if (uiSdipFlag)
	{
		piRecIPred += (uiSdipDir ? (uiLine * pcCU->getPic()->getPicYuvRec()->getStride()) : uiLine);
	}
	else {
		piRecIPred = pcCU->getPic()->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), uiAbsZorderIdx + uiPartOffset);
	}

#else
	Pel*    piRecIPred = pcCU->getPic()->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), uiAbsZorderIdx + uiPartOffset);
#endif
	UInt    uiRecIPredStride  = pcCU->getPic()->getPicYuvRec()->getStride  ();

	UInt iModeBits = 0;
	TComDataCU *pcCULeft, *pcCUAbove;

	pcCUAbove = pcCU->getPUAbove(AbovePartIdx, uiAbsZorderIdx + uiPartOffset);
	pcCULeft = pcCU->getPULeft(LeftPartIdx, uiAbsZorderIdx + uiPartOffset);
	upMode = pcCUAbove ? (pcCUAbove->isIntra(AbovePartIdx) ? pcCUAbove->getIntraDir(AbovePartIdx) : INTRA_DC_IDX) : INTRA_DC_IDX;
	leftMode = pcCULeft ? (pcCULeft->isIntra(LeftPartIdx) ? pcCULeft->getIntraDir(LeftPartIdx) : INTRA_DC_IDX) : INTRA_DC_IDX;
	mostProbableMode[0] = min(upMode, leftMode);
	mostProbableMode[1] = max(upMode, leftMode);
	if (mostProbableMode[0] == mostProbableMode[1])
	{
		mostProbableMode[0] = INTRA_DC_IDX;
		mostProbableMode[1] = (mostProbableMode[1] == INTRA_DC_IDX) ? INTRA_BI_IDX : mostProbableMode[1];
	}
	/*luma预测值*/
	//Int numModesAvailable     = 33; //total number of Intra modes
#if ZHANGYI_INTRA_SDIP
	int block_x = (uiPU & 1) << uiLog2Size;
	int block_y = (uiPU >> 1) << uiLog2Size;
	int pic_pix_x, pic_pix_y;
	if (uiSdipFlag)
	{
		pic_pix_x = (uiSdipDir == 1) ? pcCU->getCUPelX() : (pcCU->getCUPelX() + uiPU * (pcCU->getWidth(0) >> 2));
		pic_pix_y = (uiSdipDir == 1) ? (pcCU->getCUPelY() + uiPU * (pcCU->getHeight(0) >> 2)) : pcCU->getCUPelY();
	}
	else
	{
		pic_pix_x = pcCU->getCUPelX() + block_x;
		pic_pix_y = pcCU->getCUPelY() + block_y;
	}
	UInt debugx = pcCU->getCUPelX();
	UInt debugy = pcCU->getCUPelY();
	UInt Addr_test = pcCU->getAddr();
	TComPrediction::getIntraNeighborAvailabilities(pcCU, (1 << g_uiLog2MaxCUSize), pic_pix_x, pic_pix_y, uiWidth, uiHeight, p_avail);
	TComPrediction::fillReferenceSamples(bitDepth, pcCU, piRecoY, piAdiTemp, uiWidth, uiHeight, uiRecStride, p_avail, pic_pix_x, pic_pix_y);
#else
	int block_x = (uiPU & 1) << uiLog2Size;
	int block_y = (uiPU >> 1) << uiLog2Size;
	int pic_pix_x = pcCU->getCUPelX() + block_x;
	int pic_pix_y = pcCU->getCUPelY() + block_y;
#if niu_intra_pavil
	TComPrediction::getIntraNeighborAvailabilities(pcCU, (1 << g_uiLog2MaxCUSize), pic_pix_x, pic_pix_y, uiSize, uiSize, p_avail);
#else
	TComPrediction::getIntraNeighborAvailabilities(pcCU, (1<<B64X64_IN_BIT), pic_pix_x, pic_pix_y, uiSize, uiSize, p_avail);
#endif
	TComPrediction::fillReferenceSamples(bitDepth, pcCU, piRecoY, piAdiTemp, uiSize, uiSize, uiRecStride, p_avail, pic_pix_x, pic_pix_y);
#endif
	for (int i = 0; i < INTRA_NUM_FULL_RD; i++) {
		CandModeList[ i ] = INTRA_DC_IDX;
		CandCostList[ i ] = 1e30;
	}
	 for (Int modeIdx = 0; modeIdx < NUM_INTRA_MODE; modeIdx++ )	
	{
		UInt uiMode = modeIdx;
		UInt uiSad = 0;
#if ZHANGYI_INTRA_SDIP
		predIntraAngAVS(TEXT_LUMA, uiMode, uiStride, piPredY, uiStride, uiWidth, uiHeight, p_avail[NEIGHBOR_INTRA_UP], p_avail[NEIGHBOR_INTRA_LEFT], pcCU->getPicture()->getSPS()->getBitDepth());
#else
		predIntraAngAVS(TEXT_LUMA, uiMode, uiStride, piPredY, uiStride, uiSize, p_avail[NEIGHBOR_INTRA_UP], p_avail[NEIGHBOR_INTRA_LEFT], pcCU->getPicture()->getSPS()->getBitDepth());
#endif
		/*get residual signal*/
		Pel* pOrg = piOrgY;
		Pel* pPred = piPredY;
		Pel* pResi = piResiY;

		for (UInt uiY = 0; uiY < uiHeight; uiY++)
		{
			for (UInt uiX = 0; uiX < uiWidth; uiX++ )
			{
				pResi[uiX] = pOrg[uiX] - pPred[uiX];
			}
			pOrg += uiStride;
			pResi += uiStride;
			pPred += uiStride;
		}
		//use hadamard transform here
		//uiSad += distParam.DistFunc(&distParam);  //HEVC的做法，不懂。。。
		if (uiLog2Size == 2)
		{
			uiSad = calcHAD4x4(piResiY, uiStride, uiSize, uiSize);
		} 
		else 
		{
#if ZHANGYI_INTRA_SDIP
			if (uiSdipFlag)
			{
				if (uiLog2Size == 3) //CU:16x16  PU:16x4 or 4x16
				{
					#if ZHANGYI_INTRA_SDIP_BUG_YQH
					uiSad = (uiSdipDir == 1) ? calcHADSDIPHor(piResiY, uiStride, 16, 4) : calcHADSDIPVer(piResiY, uiStride, 4, 16);
#else
					uiSad = (uiSdipDir == 1) ? calcHADSDIPHor(piPredY, uiStride, 16, 4) : calcHADSDIPVer(piPredY, uiStride, 4, 16);
#endif
				}
				else if (uiLog2Size == 4) //CU:32x32  PU:32x8or 8x32
				{
#if ZHANGYI_INTRA_SDIP_BUG_YQH
					uiSad = (uiSdipDir == 1) ? calcHADSDIPHor(piResiY, uiStride, 32, 8) : calcHADSDIPVer(piResiY, uiStride, 8, 32);
#else
					uiSad = (uiSdipDir == 1) ? calcHADSDIPHor(piPredY, uiStride, 32, 8) : calcHADSDIPVer(piPredY, uiStride, 8, 32);
#endif
				}
			}
			else
			{
				uiSad = calcHADs(piResiY, uiStride, uiWidth, uiHeight);
			}
#else
			uiSad = calcHADs(piResiY, uiStride, uiSize, uiSize);
#endif
		}

		iModeBits = (mostProbableMode[0] == uiMode || mostProbableMode[1] == uiMode) ? 2 : 6;
		Double cost = m_pcRdCost->calcRdCost(iModeBits, uiSad, false, DF_DEFAULT);  //这里用了defalt的lambda
		xUpdateCandList(uiMode, cost, INTRA_NUM_FULL_RD, CandModeList, CandCostList);
	}

#endif

#if ZHANGYI_INTRA
#if niu_set_intra_premode_debug
	 for (UInt imode = 0; imode < 1; imode++)
#else
	 for ( UInt imode = 0; imode < INTRA_NUM_FULL_RD; imode++ )
#endif

	{
		uiDir = CandModeList[imode];
#else
	for( uiDir = 0; uiDir < 3; uiDir++ )
	{
#endif
#if niu_state_test
		m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
#else
      if( uiPU )
        m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiNextDepth][CI_NEXT_BEST]);
      else
        m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
#endif
#if ZHANGYI_INTRA
	  uiDistortion = xIntraSearch( pcCU, pcOrgYuv, pcPredYuv, pcResiYuv, pcRecoYuv, uiDir, uiPartOffset, p_avail ); //每个PU每个模式下的distortion（luma）
	  Int iIntraDirForRDO = (uiDir == mostProbableMode[0]) ? -2 : ( (mostProbableMode[1] == uiDir) ? -1 : ( (uiDir < mostProbableMode[0]) ? uiDir : ((uiDir < mostProbableMode[1]) ? (uiDir - 1) : (uiDir - 2))));
#if	  YQH_INTRA_BUG
	  pcCU->setIntraDirSubParts(uiDir, uiPartOffset, uiPartDepth + uiDepth); //yuquanhe@hisilicon.com 20170525
#endif	 
	  
	  xAddSymbolBitsIntra(COMPONENT_Y, pcCU, uiPU, uiQNumParts, uiPartDepth, uiMaxTrDepth, uiPartDepth, uiLog2Size, uiBits, iIntraDirForRDO );
#else
      uiDistortion = xIntraSearch( pcCU, pcOrgYuv, pcPredYuv, pcResiYuv, pcRecoYuv, uiDir, uiPartOffset );
	  xAddSymbolBitsIntra( pcCU, uiPU, uiQNumParts, uiPartDepth, uiMaxTrDepth, uiPartDepth, uiLog2Size, uiBits );
#endif

      dCost = m_pcRdCost->calcRdCost( uiBits, uiDistortion );
#if AVS3_Intra_print_Luma
	   Addr_test = pcCU->getAddr();
	  printf("Addr=%d Size=%d Dir=%d  Bits=%d  Dist=%d  Cost=%f\n", pcCU->getAddr(), uiHeight, uiDir, uiBits, uiDistortion, dCost);
#endif          
      if( dCost < dPUBestCost )
      {
        uiPUBestDir        = uiDir;
        uiPUBestBits       = uiBits;
        uiPUBestDistortion = uiDistortion;
        dPUBestCost        = dCost;

#if niu_ChromaRDO_revise
		MProbableMode[uiPU] = (uiPUBestDir == mostProbableMode[0]) ? -2 : ((mostProbableMode[1] == uiPUBestDir) ? -1 : ((uiPUBestDir < mostProbableMode[0]) ? uiPUBestDir : ((uiPUBestDir < mostProbableMode[1]) ? (uiPUBestDir - 1) : (uiPUBestDir - 2))));;
#endif

#if niu_state_test
		m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiNextDepth][CI_CURR_BEST]);
#else
        m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[uiNextDepth][CI_TEMP_BEST] );
#endif
      }
    } // end of intra direction loop (0~2)

    uiBestBits += uiPUBestBits;
    uiBestDistortion += uiPUBestDistortion;
    dBestCost += dPUBestCost; 

    pcCU->setIntraDirSubParts(uiPUBestDir, uiPartOffset, uiPartDepth+uiDepth);
#if ZHANGYI_INTRA
	Int uiPUBestDirValue = (uiPUBestDir == mostProbableMode[0]) ? -2 : ( (mostProbableMode[1] == uiPUBestDir) ? -1 : ( (uiPUBestDir < mostProbableMode[0]) ? uiPUBestDir : ((uiPUBestDir < mostProbableMode[1]) ? (uiPUBestDir - 1) : (uiPUBestDir - 2))));
	
	pcCU->setIntraDirSubPartsValue(uiPUBestDirValue, uiPartOffset, uiPartDepth + uiDepth);
	xIntraSearch( pcCU, pcOrgYuv, pcPredYuv, pcResiYuv, pcRecoYuv, uiPUBestDir, uiPartOffset, p_avail );
#else
    xIntraSearch( pcCU, pcOrgYuv, pcPredYuv, pcResiYuv, pcRecoYuv, uiPUBestDir, uiPartOffset );
#endif
#if niu_state_test
#else
    m_pppcRDSbacCoder[uiNextDepth][CI_TEMP_BEST]->store( m_pppcRDSbacCoder[uiNextDepth][CI_NEXT_BEST] );
#endif
    uiPartOffset  += uiQNumParts;
  } // end of PU loop 

#if ZHANGYI_INTRA
  UInt uiBestBitsChroma = 0;
  Double dBestCostChroma = 0;
  UInt uiBestDistortionChroma = 0;
  #if !YQH_INTRA_LAMDA
  estIntraPredChromaQT(uiPartOffset - uiQNumParts * uiNumPU, pcCU, pcOrgYuv, pcPredYuv, pcResiYuv, pcRecoYuv, &uiBestBitsChroma, &uiBestDistortionChroma, &dBestCostChroma);
#else
#if niu_CHRDO_DEBUG
  estIntraPredChromaQT(uiBestBits, uiBestDistortion, uiPartOffset - uiQNumParts * uiNumPU, pcCU, pcOrgYuv, pcPredYuv, pcResiYuv, pcRecoYuv, &uiBestBitsChroma, &uiBestDistortionChroma, &dBestCostChroma);
#else
  estIntraPredChromaQT(uiPUBestBits,uiPUBestDistortion,uiPartOffset - uiQNumParts * uiNumPU, pcCU, pcOrgYuv, pcPredYuv, pcResiYuv, pcRecoYuv, &uiBestBitsChroma, &uiBestDistortionChroma, &dBestCostChroma);
#endif
#endif
#endif
  pcCU->setCuCbfLuma( 0, uiMaxTrDepth);
  pcCU->setCuCbfChroma( 0, uiMaxTrDepth);
  

#if niu_ChromaRDO_revise
  pcCU->getTotalBits() = uiBestBitsChroma;
  pcCU->getTotalCost() = dBestCostChroma;
  pcCU->getTotalDistortion() = uiBestDistortionChroma;
#else
#if YQH_INTRA
  //yuquanhe@hisilicon.com
  uiBestBits += uiBestBitsChroma;
  uiBestDistortion += uiBestDistortionChroma;
  dBestCost = dBestCostChroma; 
#endif

  pcCU->getTotalBits()       = uiBestBits;
  pcCU->getTotalCost()       = dBestCost;
  pcCU->getTotalDistortion() = uiBestDistortion;

#endif
}

Void TEncSearch::xInitIntraSearch(TComDataCU* pcCU, TComYuv* pcRecoYuv)
{
  Pel* uiRecYtmp;
  Pel* uiRecPicYtmp;

  UInt uiCUSize = 1<<pcCU->getLog2CUSize( 0 );
  UInt uiAbsZorderIdx  = pcCU->getZorderIdxInCU();

  // Luma
  Pel* piRecPicY       = pcCU->getPic()->getPicYuvRec()->getLumaAddr( pcCU->getAddr(), uiAbsZorderIdx );
  UInt uiRecPicStride  = pcCU->getPic()->getPicYuvRec()->getStride( );
  
  Pel* piRecY          = pcRecoYuv->getLumaAddr( uiAbsZorderIdx );
  UInt uiRecStride     = pcRecoYuv->getStride();

  uiRecYtmp    = piRecY    - uiRecStride;
  uiRecPicYtmp = piRecPicY - uiRecPicStride;
#if ZHANGYI_INTRA
  for ( Int i = 0; i < uiCUSize * 2; i++ )
#else
  for ( Int i = 0; i < uiCUSize; i++ )
#endif
  {
    uiRecYtmp[i] = uiRecPicYtmp[i];
  }

  uiRecYtmp    = piRecY    - 1 - uiRecStride;
  uiRecPicYtmp = piRecPicY - 1 - uiRecPicStride;
#if ZHANGYI_INTRA
  for ( Int i = 0;i < 2*uiCUSize+1; i++ )
#else
  for ( Int i = 0;i < uiCUSize+1; i++ )
#endif
  {
    uiRecYtmp[0]  = uiRecPicYtmp[0];
    uiRecYtmp    += uiRecStride;
    uiRecPicYtmp += uiRecPicStride;
  }


  // Chroma
  Pel *uiRecCbTmp, *uiRecCrTmp;
  Pel *uiRecPicCbTmp, *uiRecPicCrTmp;

  UInt uiCUSizeC    = 1<<(pcCU->getLog2CUSize( 0 )-1);
  UInt uiRecStrideC = pcRecoYuv->getCStride();

  Pel*    piRecPicCb      = pcCU->getPic()->getPicYuvRec()->getCbAddr  ( pcCU->getAddr(), uiAbsZorderIdx );
  Pel*    piRecPicCr      = pcCU->getPic()->getPicYuvRec()->getCrAddr  ( pcCU->getAddr(), uiAbsZorderIdx );
  UInt    uiRecPicStrideC = pcCU->getPic()->getPicYuvRec()->getCStride ();

  Pel*    piRecCb         = pcRecoYuv->getCbAddr( uiAbsZorderIdx );
  Pel*    piRecCr         = pcRecoYuv->getCrAddr( uiAbsZorderIdx );

  uiRecCbTmp    = piRecCb    - uiRecStrideC;
  uiRecCrTmp    = piRecCr    - uiRecStrideC;
  uiRecPicCbTmp = piRecPicCb - uiRecPicStrideC;
  uiRecPicCrTmp = piRecPicCr - uiRecPicStrideC;
#if ZHANGYI_INTRA
  for ( Int i = 0; i < uiCUSizeC * 2; i++ )
#else
  for ( Int i = 0; i < uiCUSizeC; i++ )
#endif
  {
    uiRecCbTmp[i] = uiRecPicCbTmp[i];
    uiRecCrTmp[i] = uiRecPicCrTmp[i];
  }

  uiRecCbTmp    = piRecCb    - 1 - uiRecStrideC;
  uiRecCrTmp    = piRecCr    - 1 - uiRecStrideC;
  uiRecPicCbTmp = piRecPicCb - 1 - uiRecPicStrideC;
  uiRecPicCrTmp = piRecPicCr - 1 - uiRecPicStrideC;

#if ZHANGYI_INTRA
  for ( Int i = 0;i < 2*uiCUSizeC+1; i++ )
#else
  for ( Int i = 0;i < uiCUSizeC+1; i++ )
#endif
  {
    uiRecCbTmp[0] = uiRecPicCbTmp[0];
    uiRecCrTmp[0] = uiRecPicCrTmp[0];

    uiRecCbTmp    += uiRecStrideC;
    uiRecCrTmp    += uiRecStrideC;
    uiRecPicCbTmp += uiRecPicStrideC;
    uiRecPicCrTmp += uiRecPicStrideC;
  }

}

Void TEncSearch::predSkipSearch(TComDataCU* pcCU, TComYuv* pcOrgYuv, TComYuv*& rpcPredYuv)
{
	UInt uiPartAddr = pcCU->getZorderIdxInCU();
	UInt uiSize = 1 << pcCU->getLog2CUSize(0);
#if RPS
	UInt  uiCostTemp = 0;
	UInt  uiCost = MAX_UINT;
	Int iRefIdx;
#endif
	// clear data
	m_acYuvPred[0].clear(pcCU->getZorderIdxInCU(), uiSize);
	m_acYuvPred[1].clear(pcCU->getZorderIdxInCU(), uiSize);
	m_cYuvPredTemp.clear(pcCU->getZorderIdxInCU(), uiSize);

	rpcPredYuv->clear(pcCU->getZorderIdxInCU(), uiSize);

	// clear motion data
#if RPS
	TComMvField cMvFieldZero;
	pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvFieldZero, pcCU->getPartitionSize(0), 0, 0, 0);
	pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMvFieldZero, pcCU->getPartitionSize(0), 0, 0, 0);
#else
	TComMv cMvZero;
	pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvZero, pcCU->getPartitionSize(0), 0, 0, 0);
	pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMvZero, pcCU->getPartitionSize(0), 0, 0, 0);
#endif

	// set motion data
	if (pcCU->getPicture()->isInterB())
	{
#if RPS
#if B_RPS_BUG_815
		TComMvField cMvFieldPredL0;            //暂时B帧的skip模式还没有进来
		TComMvField cMvFieldPredL1;
		cMvFieldPredL0.setMvField(TComMv(), 0);
		cMvFieldPredL1.setMvField(TComMv(), 0);
#else
		TComMvField cMvFieldPredL0 = pcCU->getMvFieldPred(0, REF_PIC_0, 0);
		TComMvField cMvFieldPredL1 = pcCU->getMvFieldPred(0, REF_PIC_1, 0);
#endif
		pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvFieldPredL0, pcCU->getPartitionSize(0), 0, 0, 0);
		pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMvFieldPredL1, pcCU->getPartitionSize(0), 0, 0, 0);
#else
		TComMv cMvPredL0 = pcCU->getMvPred(0, REF_PIC_0);
		TComMv cMvPredL1 = pcCU->getMvPred(0, REF_PIC_1);

		pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvPredL0, pcCU->getPartitionSize(0), 0, 0, 0);
		pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMvPredL1, pcCU->getPartitionSize(0), 0, 0, 0);
#endif
#if rd_mvd
#if  B_RPS_BUG_820
		pcCU->setInterDirSubParts(1, 0, pcCU->getDepth(0), 0);
#else
		pcCU->setInterDirSubParts(INTER_BID, 0, pcCU->getDepth(0), 0);
#endif
#else
		pcCU->setInterDirSubParts(3, 0, pcCU->getDepth(0), 0);
#endif
	}
	else
	{
#if PSKIP

	{
		TComMv cMvPredL0;
		TComMvField cMvFieldPredL0;
		TComCUMvField* ColCUMvField = pcCU->getPicture()->getPicHeader()->getRefPic(REF_PIC_0, 0)
			->getPicSym()->getDPBPerCtuData(pcCU->getAddr()).getCUMvField(REF_PIC_0); //col_ref col_mv 光删扫描存储方式
		Int ColRef = 0;
		TComMv Colmv;
		Int offset = 0;
		Int curT, colT;
		Int delta1, delta2;
		delta1 = delta2 = 0;
		//uiPartAddr
		//uiBitSize = 
		UInt blockshape_block_x, blockshape_block_y;
		UInt  uiAbsPartIdx = g_auiZscanToRaster[uiPartAddr];
		blockshape_block_x = pcCU->getWidth(0) >> MIN_BLOCK_SIZE_IN_BIT;
		blockshape_block_y = pcCU->getHeight(0) >> MIN_BLOCK_SIZE_IN_BIT;
		Int iPartIdx = 0;
		Int iRoiWidth, iRoiHeight;
		for (Int i = 0; i < 2; i++)
		{
			for (Int j = 0; j < 2; j++)
			{
				pcCU->getPartIndexAndSize(iPartIdx, uiPartAddr, iRoiWidth, iRoiHeight);
				//如何设置offset;
				offset = blockshape_block_x / 2 * i + pcCU->getPic()->getNumPartInWidth() * blockshape_block_y / 2 * j;
				ColRef = ColCUMvField->getRefIdx(uiAbsPartIdx + offset);
				Colmv = ColCUMvField->getMv(uiAbsPartIdx + offset);

				if (ColRef >= 0)
				{
					curT = (2 * (pcCU->getPicture()->getPOC() - pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_0, 0)) + 512) % 512;
					colT = (2 * (pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_0, 0) -
						pcCU->getPicture()->getPicHeader()->getRefPic(REF_PIC_0, 0)->getPicHeader()->getRefPOC(REF_PIC_0, ColRef)) + 512) % 512;
					/* if (0 == img->num_of_references - 1 && he->background_reference_enable) {
					curT = 1;
					colT = 1;
					}
					if (refframe == img->num_of_references - 1 && he->background_reference_enable) {
					colT = 1;
					}*/ // 暂无

					scalingMV(cMvPredL0, Colmv, curT, colT);
					cMvPredL0.setVer(cMvPredL0.getVer() - delta2);
					//test
					cMvFieldPredL0.setMvField(cMvPredL0, 0);
				}
				else
				{
					cMvPredL0.setZero();
					cMvFieldPredL0.setMvField(cMvPredL0, 0);
				}
				pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvFieldPredL0, SIZE_NxN, uiPartAddr, 0, 0);
				iPartIdx++;
			}
		}
	}  //完成函数 PskipMV_COL
	//setPSkipMotionVector
	{
		uiPartAddr = pcCU->getZorderIdxInCU();
		pcCU->setPSkipMvField(0); // 2NX2N
	}

#else
#if RPS
		//TComMvField cMvFieldPredL0 = pcCU->getMvFieldPred(0, REF_PIC_0);
		RefPic iRefPic = REF_PIC_0;
		for (Int iRefIdxTemp = 0; iRefIdxTemp < pcCU->getPicture()->getPicHeader()->getNumRefIdx((RefPic)0); iRefIdxTemp++)
		{

			//search =0 的motion函数
			CalculateSAD(pcCU, pcOrgYuv, iRefPic, iRefIdxTemp, uiCostTemp);
			if (uiCostTemp < uiCost)
			{
				iRefIdx = iRefIdxTemp; //set iRefIdx 
			}
		}
		pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(iRefIdx, pcCU->getPartitionSize(0), 0, 0, 0);
#else
		TComMv cMvPredL0 = pcCU->getMvPred(0, REF_PIC_0);

		pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvPredL0, pcCU->getPartitionSize(0), 0, 0, 0);
#endif

#endif

#if rd_mvd
		pcCU->setInterDirSubParts(INTER_FORWARD, 0, pcCU->getDepth(0), 0);
#else
		pcCU->setInterDirSubParts( 1, 0, pcCU->getDepth(0), 0 );
#endif
	}

#if inter_direct_skip_bug2
#if	inter_direct_skip_bug3
	Int           iNumPredDir = pcCU->getPicture()->isInterB() ? 2 : 1;
#else
	Int           iNumPredDir = pcCU->getPicture()->isInterP() ? 1 : 2;
#endif
	Int           iRoiWidth, iRoiHeight;
	pcCU->getPartIndexAndSize(0, uiPartAddr, iRoiWidth, iRoiHeight);
#if	inter_direct_skip_bug3
	#if	inter_direct_skip_bug4
	if (pcCU->getPic()->getPicture()->getPictureType() != B_PICTURE)
#else
	if (pcCU->getPic()->getPicture()->getPictureType() == F_PICTURE)
#endif
#else
	if (pcCU->getPic()->getPicture()->getPictureType() == P_PICTURE)
#endif
	{
#if F_MHPSKIP_SYC
		if (pcCU->getInterSkipmode(uiPartAddr) > 3)//MHPSKIP
		{
			Int refIdx[2];
			TComMv tempMV[2];
#if F_MHPSKIP_SYC_FIXED_MV
			switch (pcCU->getInterSkipmode(uiPartAddr) - 3)
			{
			case  BID_P_FST:
				refIdx[0] = 0;//pcCU->getTmpFirstMvPred(BID_P_FST).getRefIdx();
				tempMV[0] = TComMv(0, 0);// pcCU->getTmpFirstMvPred(BID_P_FST).getMv();
				refIdx[1] = 0;//pcCU->getTmpSecondMvPred(BID_P_FST).getRefIdx();
				tempMV[1] = TComMv(0, 0);//pcCU->getTmpSecondMvPred(BID_P_FST).getMv();
				break;
			case BID_P_SND:
				refIdx[0] = 0;//pcCU->getTmpFirstMvPred(BID_P_SND).getRefIdx();
				tempMV[0] = TComMv(0, 0);//pcCU->getTmpFirstMvPred(BID_P_SND).getMv();
				refIdx[1] = 0;//pcCU->getTmpSecondMvPred(BID_P_SND).getRefIdx();
				tempMV[1] = TComMv(0, 0);//pcCU->getTmpSecondMvPred(BID_P_SND).getMv();
				break;
			case FW_P_FST:
				refIdx[0] = 0;//pcCU->getTmpFirstMvPred(FW_P_FST).getRefIdx();
				tempMV[0] = TComMv(0, 0);//pcCU->getTmpFirstMvPred(FW_P_FST).getMv();
				break;
			case FW_P_SND:
				refIdx[0] = 0;//pcCU->getTmpFirstMvPred(FW_P_FST).getRefIdx();
				tempMV[0] = TComMv(0, 0);//pcCU->getTmpFirstMvPred(FW_P_FST).getMv();
				break;
			default:
				assert(0);
			}
#else
			switch (pcCU->getInterSkipmode(uiPartAddr) - 3)
			{
			case  BID_P_FST:
				refIdx[0] = pcCU->getTmpFirstMvPred(BID_P_FST).getRefIdx();
				tempMV[0] = pcCU->getTmpFirstMvPred(BID_P_FST).getMv();
				refIdx[1] = pcCU->getTmpSecondMvPred(BID_P_FST).getRefIdx();
				tempMV[1] = pcCU->getTmpSecondMvPred(BID_P_FST).getMv();
				break;
			case BID_P_SND:
				refIdx[0] = pcCU->getTmpFirstMvPred(BID_P_SND).getRefIdx();
				tempMV[0] = pcCU->getTmpFirstMvPred(BID_P_SND).getMv();
				refIdx[1] = pcCU->getTmpSecondMvPred(BID_P_SND).getRefIdx();
				tempMV[1] = pcCU->getTmpSecondMvPred(BID_P_SND).getMv();
				break;
			case FW_P_FST:
				refIdx[0] = pcCU->getTmpFirstMvPred(FW_P_FST).getRefIdx();
				tempMV[0] = pcCU->getTmpFirstMvPred(FW_P_FST).getMv();
				break;
			case FW_P_SND:
				refIdx[0] = pcCU->getTmpFirstMvPred(FW_P_FST).getRefIdx();
				tempMV[0] = pcCU->getTmpFirstMvPred(FW_P_FST).getMv();
				break;
			default:
				assert(0);
			}
#endif


			if (pcCU->getInterSkipmode(uiPartAddr) - 3 < FW_P_FST)
			{
				pcCU->setInterDirSubParts(INTER_FORWARD, uiPartAddr, pcCU->getDepth(0), 0);
				pcCU->getCUMvField(REF_PIC_0)->setAllMv(tempMV[0], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(refIdx[0], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				motionCompensation(pcCU, &m_acMHPSkipYuvPred[REF_PIC_0], REF_PIC_0, 0);

				pcCU->setInterDirSubParts(INTER_FORWARD, uiPartAddr, pcCU->getDepth(0), 0);
				pcCU->getCUMvField(REF_PIC_0)->setAllMv(tempMV[1], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(refIdx[1], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				motionCompensation(pcCU, &m_acMHPSkipYuvPred[REF_PIC_1], REF_PIC_0, 0);

				rpcPredYuv->addAvg(&m_acMHPSkipYuvPred[0], &m_acMHPSkipYuvPred[1], uiPartAddr + pcCU->getZorderIdxInCU(), iRoiWidth, iRoiHeight, uiPartAddr + pcCU->getZorderIdxInCU());
			}
			else
			{
				pcCU->setInterDirSubParts(INTER_FORWARD, uiPartAddr, pcCU->getDepth(0), 0);
				pcCU->getCUMvField(REF_PIC_0)->setAllMv(tempMV[0], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(refIdx[0], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);

				motionCompensation(pcCU, &m_acMHPSkipYuvPred[REF_PIC_0], REF_PIC_0, 0);

				m_acMHPSkipYuvPred[REF_PIC_0].copyPartToPartYuv(rpcPredYuv, uiPartAddr + pcCU->getZorderIdxInCU(), uiPartAddr + pcCU->getZorderIdxInCU(), iRoiWidth, iRoiHeight);
			}

		}
		else //WSM
		{
#endif
			for (Int iRefPic = 0; iRefPic < iNumPredDir; iRefPic++)
			{
				RefPic eRefPic = (iRefPic ? REF_PIC_1 : REF_PIC_0); // llt
				Int w = pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic);
				// pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic)
				for (Int iRefIdxTemp = 0; iRefIdxTemp < 2; iRefIdxTemp++)
				{
					pcCU->setInterDirSubParts(iRefPic + 1, uiPartAddr, pcCU->getDepth(0), 0);
					Int refIdx = (iRefIdxTemp == 0 ? 0 : pcCU->getInterSkipmode(uiPartAddr));

					pcCU->getCUMvField(eRefPic)->setAllRefIdx(refIdx, pcCU->getPartitionSize(0), uiPartAddr, 0, 0);

					TComYuv*  pcYuvPred = &m_acYuvPred[iRefIdxTemp];
					motionCompensation(pcCU, pcYuvPred, eRefPic, 0);

				}//end of refIdxnum
				Int refIdx = 0;
				pcCU->getCUMvField(eRefPic)->setAllRefIdx(refIdx, pcCU->getPartitionSize(0), uiPartAddr, 0, 0);

				if (pcCU->getInterSkipmode(uiPartAddr) != 0)
				{
					rpcPredYuv->addAvg(&m_acYuvPred[0], &m_acYuvPred[1], uiPartAddr + pcCU->getZorderIdxInCU(), iRoiWidth, iRoiHeight, uiPartAddr + pcCU->getZorderIdxInCU());
				}
				else
				{
					TComYuv*  pcYuvPred_t = &m_acYuvPred[0];
					pcYuvPred_t->copyPartToPartYuv(rpcPredYuv, uiPartAddr + pcCU->getZorderIdxInCU(), uiPartAddr + pcCU->getZorderIdxInCU(), iRoiWidth, iRoiHeight);
				}
			}//end of reflist
#if F_MHPSKIP_SYC
		}
#endif
	}
	else
		motionCompensation(pcCU, rpcPredYuv, REF_PIC_X, 0);

#else
	// MC
	motionCompensation ( pcCU, rpcPredYuv, REF_PIC_X, 0 );
#endif
	return;
}
#if RPS
Void TEncSearch::predInterSearch(TComDataCU* pcCU, TComYuv* pcOrgYuv, TComYuv*& rpcPredYuv)
{
  Int uiSize = 1 << pcCU->getLog2CUSize(0);
  m_acYuvPred[REF_PIC_0].clear(pcCU->getZorderIdxInCU(), uiSize);
  m_acYuvPred[REF_PIC_1].clear(pcCU->getZorderIdxInCU(), uiSize);
  m_cYuvPredTemp.clear(pcCU->getZorderIdxInCU(), uiSize);
  rpcPredYuv->clear(pcCU->getZorderIdxInCU(), uiSize);

  TComMv       cMvSrchRngLT;
  TComMv       cMvSrchRngRB;
  TComMv       cMvZero;
  TComMv       TempMv;
  TComMv        cMv[2];
  TComMv       cMvTemp[2][33]; // 中间变量，33为最大参考帧数
  Int           iNumPart = pcCU->getNumPartInter();


#if F_RPS
  Int           iNumPredDir = (pcCU->getPicture()->isInterP() || pcCU->getPicture()->isInterF()) ? 1 : 2;
#else
  Int           iNumPredDir = pcCU->getPicture()->isInterP() ? 1 : 2;
#endif

  TComMv        cMvPred[2][33];  ///  ZP
  Int           iRefIdx[2] = { 0, 0 };
  Int           iRefIdxBi[2];
  UInt          uiPartAddr = pcCU->getZorderIdxInCU();
  Int           iRoiWidth, iRoiHeight;
#if INTER614
  Bool IsEncodeRefIdx = (pcCU->getPicture()->getSPS()->getNumberReferenceFrames() > 1) ? true : false;
  Int maxRef = pcCU->getPicture()->getPicHeader()->getNumRefIdx(REF_PIC_0)
	  + pcCU->getPicture()->getPicHeader()->getNumRefIdx(REF_PIC_1);
  Int adjustRef;
  Int activeRefNumber = maxRef;
  SetRef(pcCU, &adjustRef, &maxRef);
#endif
  UInt          uiMbBits[3] = { 1, 1, 0 };
  UInt          uiLastMode = 0;
  PartSize      ePartSize = pcCU->getPartitionSize(0);
#if DEBUG_ME
  UInt c_cost[10];
  UInt Refnum=0;
#endif
  for (Int iPartIdx = 0; iPartIdx < iNumPart; iPartIdx++)
  {
    UInt          uiCost[2] = { MAX_UINT, MAX_UINT };
    UInt          uiCostTemp;
    UInt          uiBits[3];
    UInt          uiBitsTemp;
    TComPicture*  pcPicture = pcCU->getPicture();
#if rd_mvd
	UInt          uiSymCostTemp = MAX_UINT;
	Int           iRefIdxSym[2];
#endif
    // bGPB 多参考中，一般不会出现这种情况  ZP
    Int NumberOfNegativeRefPictures = pcPicture->getPicHeader()->getRPS()->getNumberOfNegativeRefPictures();
    Int NumberOfPositiveRefPictures = pcPicture->getPicHeader()->getRPS()->getNumberOfPositiveRefPictures();
    Bool          bGPB = false;
    if (NumberOfPositiveRefPictures == 0 || NumberOfNegativeRefPictures == 0)
    {
      bGPB = false;
    }
    else
    {
      for (Int iRefIdx1 = 0; iRefIdx1 < NumberOfPositiveRefPictures; iRefIdx1++)
      {
        for (Int iRefIdx0 = 0; iRefIdx0 < NumberOfPositiveRefPictures; iRefIdx0++)
        {
          bGPB = pcPicture->getPictureType() == B_PICTURE && pcPicture->getPicHeader()->getRefPOC(REF_PIC_0, iRefIdx0) == pcPicture->getPicHeader()->getRefPOC(REF_PIC_1, iRefIdx1);
          if (bGPB == false)
            break;
        }
        if (bGPB == false)
          break;
      }
    }             ///ZP
    TComMv        cMvPredBi[2][33];  //ZP
    TComMv        cMvBi[2];
#if rd_mvd
	TComMv        cMvSym[2];
	TComMv        cMvPredSym[2][33];
#endif
    UInt          uiCostBi = MAX_UINT;
#if F_RPS
	xGetBlkBits(ePartSize, (pcCU->getPicture()->isInterP() || pcCU->getPicture()->isInterF()), iPartIdx, uiLastMode, uiMbBits);
#else
    xGetBlkBits(ePartSize, pcCU->getPicture()->isInterP(), iPartIdx, uiLastMode, uiMbBits);
#endif

    pcCU->getPartIndexAndSize(iPartIdx, uiPartAddr, iRoiWidth, iRoiHeight);

    //单方向预测 
    // Uni-directional prediction
    for (Int iRefPic = 0; iRefPic < iNumPredDir; iRefPic++)
    {
      RefPic eRefPic = (iRefPic ? REF_PIC_1 : REF_PIC_0); // llt
      Int w = pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic);
#if DEBUG_ME
	  Refnum = pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic);
#endif
      for (Int iRefIdxTemp = 0; iRefIdxTemp < pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic); iRefIdxTemp++)
      {



        pcCU->setInterDirSubParts(iRefPic + 1, uiPartAddr, pcCU->getDepth(0), iPartIdx);

#if INTER614
		if (activeRefNumber > 1)
		{
			uiBitsTemp = IsEncodeRefIdx ? m_pcRdCost->getRefCost(iRefIdxTemp) : 0;
		}
		else
		{
			uiBitsTemp = 0;
		}
#else
		uiBitsTemp = uiMbBits[iRefPic];

#endif

        cMvPred[iRefPic][iRefIdxTemp] = pcCU->getMvFieldPred(uiPartAddr, eRefPic, iRefIdxTemp).getMv();
        //GPB 关掉  ZP //重写 ME函数 传入参数  iRefIdxTemp
        if (pcCU->getPicture()->getSPS()->getUseLDC())
        {
          if (iRefPic && bGPB)
          {
            uiCostTemp = MAX_UINT;
          }
          else
          {
            xMotionEstimation(pcCU, pcOrgYuv, iPartIdx, eRefPic, &cMvPred[iRefPic][iRefIdxTemp], iRefIdxTemp, cMvTemp[iRefPic][iRefIdxTemp], uiBitsTemp, uiCostTemp);//函数重写ME
          }
        }
        else
        {
          xMotionEstimation(pcCU, pcOrgYuv, iPartIdx, eRefPic, &cMvPred[iRefPic][iRefIdxTemp], iRefIdxTemp, cMvTemp[iRefPic][iRefIdxTemp], uiBitsTemp, uiCostTemp);
        }
#if DEBUG_ME
		c_cost[iRefIdxTemp] = uiCostTemp;
#endif
        if (uiCostTemp<uiCost[iRefPic])
        {
          uiCost[iRefPic] = uiCostTemp;
          uiBits[iRefPic] = uiBitsTemp; // storing for bi-prediction
          iRefIdx[iRefPic] = iRefIdxTemp;
          // set motion
          cMv[iRefPic] = cMvTemp[iRefPic][iRefIdxTemp];
          //set mv
          pcCU->getCUMvField(eRefPic)->setAllMv(cMv[iRefPic], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);

#if wlq_set_mvd
		  //set mvd
		  Int iHor = cMv[iRefPic].getHor() - cMvPred[iRefPic][iRefIdxTemp].getHor();
		  Int iVer = cMv[iRefPic].getVer() - cMvPred[iRefPic][iRefIdxTemp].getVer();
		  TComMv mvd;
		  mvd.set(iHor, iVer);
		  pcCU->getCUMvField(eRefPic)->setAllMvd(mvd, pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
#endif

          //set iRefIdx
          pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx[iRefPic], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
          // storing list 1 prediction signal for iterative bi-directional prediction
          if (eRefPic == REF_PIC_1)
          {
#if INTERBUG
			  TComYuv*  pcYuvPred = &m_acYuvPred[iRefPic];
#else
            TComYuv*  pcYuvPred = &m_acYuvPred[(RefPic)iRefPic];
#endif
            motionCompensation(pcCU, pcYuvPred, eRefPic, iPartIdx);
          }
#if B_ME_REVISE
		  if (bGPB && eRefPic == REF_PIC_0)
#else
          if (eRefPic == REF_PIC_0)
#endif
          {
            TComYuv*  pcYuvPred = &m_acYuvPred[iRefPic];
            motionCompensation(pcCU, pcYuvPred, eRefPic, iPartIdx);
          }
        }
      }//end of refIdxnum
    }//end of reflist
#if DEBUG_ME
	{
		for (int i = 0; i < Refnum; i++)
		{
			printf("c_cost=%d\t", c_cost[i]);
		}
		printf("BestIdx=%d\n", iRefIdx[0]);
	}
#endif


#if F_DHP_SYC
	//  Dual prediction

	TComMv        cMvDual[2];
	//TComMv        cMvPredDual[2][33];
	UInt          uiCostDual = MAX_UINT;
	Int           iRefIdxDual[2];
	if (pcCU->getPicture()->isInterF() && pcCU->getPicture()->getSPS()->getDualHypothesisPredictionEnableFlag() && pcCU->getPicture()->getPicHeader()->getNumRefIdx(REF_PIC_0)>1
		&& !(pcCU->getLog2CUSize(0) == B8X8_IN_BIT && pcCU->getPartitionSize(0) >= SIZE_2NxN && pcCU->getPartitionSize(0) <= SIZE_nRx2N))
	{
		pcCU->setInterDirSubParts(INTER_DUAL, uiPartAddr, pcCU->getDepth(0), iPartIdx);

		Int iRefPic = 0; RefPic  eRefPic = REF_PIC_0;//F帧两个列表一致

		for (Int iRefIdxTemp = 0; iRefIdxTemp < pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic); iRefIdxTemp++)
		{

			xMotionEstimation(pcCU, pcOrgYuv, iPartIdx, eRefPic, &cMvPred[REF_PIC_0][iRefIdxTemp], iRefIdxTemp, cMvTemp[REF_PIC_0][iRefIdxTemp], uiBitsTemp, uiCostTemp, false, true);
			uiCostTemp += abs(cMvTemp[REF_PIC_0][iRefIdxTemp].getHor() - cMvPred[iRefPic][iRefIdxTemp].getHor()) +
				abs(cMvTemp[REF_PIC_0][iRefIdxTemp].getVer() - cMvPred[iRefPic][iRefIdxTemp].getVer());
			//uiCostTemp = 10;
			if (uiCostTemp < uiCostDual)
			{
				iRefIdxDual[REF_PIC_0] = iRefIdxTemp;
				iRefIdxDual[REF_PIC_1] = (iRefIdxTemp == 0) ? 1 : 0;
				cMvDual[REF_PIC_0] = cMvTemp[REF_PIC_0][iRefIdxTemp];
				pcCU->MV_Scale(pcCU, cMvDual[REF_PIC_1], cMvDual[REF_PIC_0], iRefIdxDual[REF_PIC_0], iRefIdxDual[REF_PIC_1]);

				uiCostDual = uiCostTemp;
				TComYuv* pcYuvPred = &m_acYuvPred[REF_PIC_1];
				motionCompensation(pcCU, pcYuvPred, REF_PIC_0, iPartIdx);
			}
		}

	}// if (F_PICTURE)
#endif
    //  Bi-directional prediction
#if rd_bipred
	if ((pcCU->getPicture()->isInterB()) && (!((pcCU->getLog2CUSize(0) == 3) && (pcCU->getPartitionSize(0) >= SIZE_2NxN) && (pcCU->getPartitionSize(0) <= SIZE_nRx2N))))
	{
#if rd_mvd
		pcCU->setInterDirSubParts(INTER_BID, uiPartAddr, pcCU->getDepth(0), iPartIdx);
#else
		pcCU->setInterDirSubParts(3, uiPartAddr, pcCU->getDepth(0), iPartIdx);
#endif

		cMvBi[0] = cMv[0];            cMvBi[1] = cMv[1];
#if rd_mvd
		cMvSym[0] = cMv[0];            cMvSym[1] = cMv[1];
#endif

		::memcpy(cMvPredBi, cMvPred, sizeof(cMvPred));

		UInt uiMotBits[2] = { uiBits[0] - uiMbBits[0], uiBits[1] - uiMbBits[1] };
		uiBits[2] = uiMbBits[2] + uiMotBits[0] + uiMotBits[1];

		Int iRefPic = 0;

		RefPic  eRefPic = (iRefPic ? REF_PIC_1 : REF_PIC_0);
		Bool bChanged = false;
		for (Int iRefIdxTemp = 0; iRefIdxTemp < pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic); iRefIdxTemp++)
		{
			uiBitsTemp = uiMbBits[2] + uiMotBits[1 - iRefPic];
			if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic) > 1)
			{
				uiBitsTemp += iRefIdxTemp + 1;
				if (iRefIdxTemp == pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic) - 1)
				{
					uiBitsTemp--;
				}
			}
			//MVP暂无，
#if rd_sym
			//test
#if	rd_mvd_yqh

			if (pcCU->getLog2CUSize(0) > B8X8_IN_BIT || (pcCU->getPartitionSize(0) == SIZE_2Nx2N))

			{
				pcCU->setInterDirSubParts(INTER_SYM, uiPartAddr, pcCU->getDepth(0), iPartIdx);
				xMotionEstimationSym(pcCU, pcOrgYuv, iPartIdx, eRefPic, &cMvPredSym[iRefPic][iRefIdxTemp], &cMvPredSym[1][iRefIdxTemp], iRefIdxTemp, cMvTemp[iRefPic][iRefIdxTemp], cMvSym[1], uiBitsTemp, uiCostTemp, true);
			}
			else
			{
				uiCostTemp = MAX_DOUBLE;
			}
#else
			//	if (pcCU->getLog2CUSize(0) > B8X8_IN_BIT || (pcCU->getPartitionSize(0) == SIZE_2Nx2N))
			xMotionEstimationSym(pcCU, pcOrgYuv, iPartIdx, eRefPic, &cMvPredSym[iRefPic][iRefIdxTemp], &cMvPredSym[1][iRefIdxTemp], iRefIdxTemp, cMvTemp[iRefPic][iRefIdxTemp], cMvSym[1], uiBitsTemp, uiCostTemp, true);
			/*else
			uiCostTemp = MAX_DOUBLE;*/
#endif
			if (uiCostTemp < uiSymCostTemp)
			{
				cMvSym[iRefPic] = cMvTemp[iRefPic][iRefIdxTemp];
				iRefIdxSym[iRefPic] = iRefIdxTemp;
				uiSymCostTemp = uiCostTemp;
				uiBits[2] = uiBitsTemp;
			}
#endif
#if	rd_mvd_yqh

			if (pcCU->getLog2CUSize(0) > B8X8_IN_BIT || (pcCU->getPartitionSize(0) == SIZE_2Nx2N))

			{
				pcCU->setInterDirSubParts(INTER_BID, uiPartAddr, pcCU->getDepth(0), iPartIdx);
				xMotionEstimationBi(pcCU, pcOrgYuv, iPartIdx, eRefPic, &cMvPredBi[iRefPic][iRefIdxTemp], &cMvPredBi[1][iRefIdxTemp], iRefIdxTemp, cMvTemp[iRefPic][iRefIdxTemp], cMvBi[1], uiBitsTemp, uiCostTemp, true);
			}
			else
				uiCostTemp = MAX_DOUBLE;
#else
			//	if (pcCU->getLog2CUSize(0) > B8X8_IN_BIT || (pcCU->getPartitionSize(0) == SIZE_2Nx2N))
			xMotionEstimationBi(pcCU, pcOrgYuv, iPartIdx, eRefPic, &cMvPredBi[iRefPic][iRefIdxTemp], &cMvPredBi[1][iRefIdxTemp], iRefIdxTemp, cMvTemp[iRefPic][iRefIdxTemp], cMvBi[1], uiBitsTemp, uiCostTemp, true);
			//	else
			//	uiCostTemp = MAX_DOUBLE;
#endif
			if (uiCostTemp < uiCostBi)
			{
				bChanged = true;

				cMvBi[iRefPic] = cMvTemp[iRefPic][iRefIdxTemp];
				iRefIdxBi[iRefPic] = iRefIdxTemp;
				uiCostBi = uiCostTemp;
				uiMotBits[iRefPic] = uiBitsTemp - uiMbBits[2] - uiMotBits[1 - iRefPic];
				uiBits[2] = uiBitsTemp;

				//  Set motion

				pcCU->getCUMvField(eRefPic)->setAllMv(cMvBi[iRefPic], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
				pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdxBi[iRefPic], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);

				TComYuv* pcYuvPred = &m_acYuvPred[(RefPic)iRefPic];
				motionCompensation(pcCU, pcYuvPred, eRefPic, iPartIdx);
			}
		}
		// call ME
		if (!bChanged)
		{
			break;
		}
	} // if (B_PICTURE)
#endif
    //  Clear Motion Field
    pcCU->getCUMvField(REF_PIC_0)->setAllMvField(TComMvField(), pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
    pcCU->getCUMvField(REF_PIC_1)->setAllMvField(TComMvField(), pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
	#if rd_bipred_YQH
	pcCU->getCUMvField(REF_PIC_0)->setAllMv(cMvZero, pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
	pcCU->getCUMvField(REF_PIC_1)->setAllMv(cMvZero, pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
#endif
    // Set Motion Field_
#if F_DHP_SYC

	if (pcCU->getPicture()->isInterF() && pcCU->getPicture()->getSPS()->getDualHypothesisPredictionEnableFlag() && pcCU->getPicture()->getPicHeader()->getNumRefIdx(REF_PIC_0) > 1
		&& !(pcCU->getLog2CUSize(0) == B8X8_IN_BIT && pcCU->getPartitionSize(0) >= SIZE_2NxN && pcCU->getPartitionSize(0) <= SIZE_nRx2N))
	{

		if (uiCostBi <= uiCost[0] && uiCostBi <= uiCost[1] && uiCostBi <= uiCostDual)
#else
	if (uiCostBi <= uiCost[0] && uiCostBi <= uiCost[1])
#endif
	{
		uiLastMode = 2;

		pcCU->getCUMvField(REF_PIC_0)->setAllMv(cMvBi[0], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(iRefIdxBi[0], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->getCUMvField(REF_PIC_1)->setAllMv(cMvBi[1], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->getCUMvField(REF_PIC_1)->setAllRefIdx(iRefIdxBi[1], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->setInterDirSubParts(3, uiPartAddr, pcCU->getDepth(0), iPartIdx);
	}
#if F_DHP_SYC
	else if (uiCostDual <= uiCost[0] && uiCostDual <= uiCost[1])
	{
		uiLastMode = 3;////////////////////是否写3   20170816

		pcCU->getCUMvField(REF_PIC_0)->setAllMv(cMvDual[0], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(iRefIdxDual[0], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		//pcCU->getCUMvField(REF_PIC_1)->setAllMv(cMvDual[1], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		//pcCU->getCUMvField(REF_PIC_1)->setAllRefIdx(iRefIdxDual[1], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->setInterDirSubParts(INTER_DUAL, uiPartAddr, pcCU->getDepth(0), iPartIdx);
		//pcCU->setInterDir(iPartIdx, INTER_DUAL);
	}
		}
#endif

#if rd_mvd
	 if ((uiCost[0] <= uiCost[1]) && (uiCost[0] <= uiCostBi) && (uiCost[0] <= uiSymCostTemp))
	{
		uiLastMode = 0;

		pcCU->getCUMvField(REF_PIC_0)->setAllMv(cMv[0], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(iRefIdx[0], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->setInterDirSubParts(INTER_FORWARD, uiPartAddr, pcCU->getDepth(0), iPartIdx);
	}
	else if ((uiCost[1] <= uiCost[0]) && (uiCost[1] <= uiCostBi) && (uiCost[1] <= uiSymCostTemp))
	{
		uiLastMode = 1;

		pcCU->getCUMvField(REF_PIC_1)->setAllMv(cMv[1], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->getCUMvField(REF_PIC_1)->setAllRefIdx(iRefIdx[1], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->setInterDirSubParts(INTER_BACKWARD, uiPartAddr, pcCU->getDepth(0), iPartIdx);
	}
	else if ((uiSymCostTemp <= uiCost[0]) && (uiSymCostTemp <= uiCost[1]) && (uiSymCostTemp <= uiCostBi))
	{
		uiLastMode = 2;//?

		pcCU->getCUMvField(REF_PIC_0)->setAllMv(cMvSym[0], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(iRefIdxSym[0], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->getCUMvField(REF_PIC_1)->setAllRefIdx(iRefIdx[1], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->setInterDirSubParts(INTER_SYM, uiPartAddr, pcCU->getDepth(0), iPartIdx);
	}
	else
	{
		uiLastMode = 3;

		pcCU->getCUMvField(REF_PIC_0)->setAllMv(cMvBi[0], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(iRefIdxBi[0], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->getCUMvField(REF_PIC_1)->setAllMv(cMvBi[1], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->getCUMvField(REF_PIC_1)->setAllRefIdx(iRefIdx[1], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->setInterDirSubParts(INTER_BID, uiPartAddr, pcCU->getDepth(0), iPartIdx);
	}
#endif
#if debug_dir
	if (pcCU->getPicture()->getPictureType() == B_PICTURE)
	{
		uiLastMode = 1;

		//pcCU->getCUMvField(REF_PIC_0)->setAllMv(cMvBi[0], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		//pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(iRefIdx[0], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->getCUMvField(REF_PIC_1)->setAllMv(cMvBi[1], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->getCUMvField(REF_PIC_1)->setAllRefIdx(iRefIdx[1], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
		pcCU->setInterDirSubParts(INTER_BACKWARD, uiPartAddr, pcCU->getDepth(0), iPartIdx);
	}
#endif


	#if	!rd_bipred_YQH
    else if (uiCost[0] <= uiCost[1])
    {
      uiLastMode = 0;

      pcCU->getCUMvField(REF_PIC_0)->setAllMv(cMv[0], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
      pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(iRefIdx[0], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
      pcCU->setInterDirSubParts(1, uiPartAddr, pcCU->getDepth(0), iPartIdx);
    }
    else
    {
      uiLastMode = 1;

      pcCU->getCUMvField(REF_PIC_1)->setAllMv(cMv[1], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
      pcCU->getCUMvField(REF_PIC_1)->setAllRefIdx(iRefIdx[1], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
      pcCU->setInterDirSubParts(2, uiPartAddr, pcCU->getDepth(0), iPartIdx);
    }

#endif




    //  MC
    motionCompensation(pcCU, rpcPredYuv, REF_PIC_X, iPartIdx);
  }//end of part
  return;
}
#else
Void TEncSearch::predInterSearch(TComDataCU* pcCU, TComYuv* pcOrgYuv, TComYuv*& rpcPredYuv)
{
  UInt uiSize = 1 << pcCU->getLog2CUSize(0);
  m_acYuvPred[0].clear(pcCU->getZorderIdxInCU(), uiSize);
  m_acYuvPred[1].clear(pcCU->getZorderIdxInCU(), uiSize);
  m_cYuvPredTemp.clear(pcCU->getZorderIdxInCU(), uiSize);
  rpcPredYuv->clear(pcCU->getZorderIdxInCU(), uiSize);

  TComMv        cMvSrchRngLT;
  TComMv        cMvSrchRngRB;

  TComMv        cMvZero;
  TComMv        TempMv;

  TComMv        cMv[2];
  TComMv        cMvTemp[2];

  Int           iNumPart = pcCU->getNumPartInter();
  Int           iNumPredDir = pcCU->getPicture()->isInterP() ? 1 : 2;

  TComMv        cMvPred[2];

  UInt          uiPartAddr = pcCU->getZorderIdxInCU();

  Int           iRoiWidth, iRoiHeight;

  UInt          uiMbBits[3] = { 1, 1, 0 };

  UInt          uiLastMode = 0;

  PartSize      ePartSize = pcCU->getPartitionSize(0);

  for (Int iPartIdx = 0; iPartIdx < iNumPart; iPartIdx++)
  {

    UInt          uiCost[2] = { MAX_UINT, MAX_UINT };
    UInt          uiCostTemp;

    UInt          uiBits[3];
    UInt          uiBitsTemp;
    TComPicture*  pcPicture = pcCU->getPicture();
    Bool          bGPB = pcPicture->getPictureType() == B_PICTURE && pcPicture->getRefPOC(REF_PIC_0) == pcPicture->getRefPOC(REF_PIC_1);

    TComMv        cMvPredBi[2];
    TComMv        cMvBi[2];
    UInt          uiCostBi = MAX_UINT;

    xGetBlkBits(ePartSize, pcCU->getPicture()->isInterP(), iPartIdx, uiLastMode, uiMbBits);

    pcCU->getPartIndexAndSize(iPartIdx, uiPartAddr, iRoiWidth, iRoiHeight);

    // Uni-directional prediction
    for (Int iRefPic = 0; iRefPic < iNumPredDir; iRefPic++)
    {
      pcCU->setInterDirSubParts(iRefPic + 1, uiPartAddr, pcCU->getDepth(0), iPartIdx);

      RefPic  eRefPic = (iRefPic ? REF_PIC_1 : REF_PIC_0);

      if (pcCU->getPicture()->getRefPic(eRefPic) != NULL)
      {
        uiBitsTemp = uiMbBits[iRefPic];

        // get mv predictor
        //
        cMvPred[iRefPic] = pcCU->getMvPred(uiPartAddr, eRefPic);

#if GPB_SIMPLE_UNI
        if (pcCU->getPicture()->getSPS()->getUseLDC())
        {
          if (iRefPic && bGPB)
          {
            uiCostTemp = MAX_UINT;
            if (bGPB)
            {
              cMvTemp[1] = cMvTemp[0];
            }
          }
          else
          {
            xMotionEstimation(pcCU, pcOrgYuv, iPartIdx, eRefPic, &cMvPred[iRefPic], cMvTemp[iRefPic], uiBitsTemp, uiCostTemp);
          }
        }
        else
        {
          if (iRefPic && bGPB)
          {
            uiCostTemp = MAX_UINT;
            cMvTemp[1] = cMvTemp[0];
          }
          else
          {
            xMotionEstimation(pcCU, pcOrgYuv, iPartIdx, eRefPic, &cMvPred[iRefPic], cMvTemp[iRefPic], uiBitsTemp, uiCostTemp);
          }
        }
#else
        xMotionEstimation(pcCU, pcOrgYuv, iPartIdx, eRefPic, &cMvPred[iRefPic], cMvTemp[iRefPic], uiBitsTemp, uiCostTemp);
#endif
        if ((iRefPic == 0 && uiCostTemp < uiCost[iRefPic]) ||
          (iRefPic == 1 && bGPB) ||
          (iRefPic == 1 && !bGPB && uiCostTemp < uiCost[iRefPic]))
        {
          uiCost[iRefPic] = uiCostTemp;
          uiBits[iRefPic] = uiBitsTemp; // storing for bi-prediction

          // set motion
          cMv[iRefPic] = cMvTemp[iRefPic];
          pcCU->getCUMvField(eRefPic)->setAllMvField(cMv[iRefPic], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);

          // storing list 1 prediction signal for iterative bi-directional prediction
          if (eRefPic == REF_PIC_1)
          {
            TComYuv*  pcYuvPred = &m_acYuvPred[iRefPic];
            motionCompensation(pcCU, pcYuvPred, eRefPic, iPartIdx);
          }
          if (bGPB && eRefPic == REF_PIC_0)
          {
            TComYuv*  pcYuvPred = &m_acYuvPred[iRefPic];
            motionCompensation(pcCU, pcYuvPred, eRefPic, iPartIdx);
          }
        }
      }
    }

    //  Bi-directional prediction
    if (pcCU->getPicture()->isInterB())
    {
      pcCU->setInterDirSubParts(3, uiPartAddr, pcCU->getDepth(0), iPartIdx);

      cMvBi[0] = cMv[0];            cMvBi[1] = cMv[1];

      ::memcpy(cMvPredBi, cMvPred, sizeof(cMvPred));

      UInt uiMotBits[2] = { uiBits[0] - uiMbBits[0], uiBits[1] - uiMbBits[1] };
      uiBits[2] = uiMbBits[2] + uiMotBits[0] + uiMotBits[1];

      // 4-times iteration (default)
      Int iNumIter = 4;

      // fast encoder setting: only one iteration
      if (m_pcEncCfg->getUseFastEnc())
      {
        iNumIter = 1;
      }

      for (Int iIter = 0; iIter < iNumIter; iIter++)
      {
        Int iRefPic = iIter % 2;
        if (m_pcEncCfg->getUseFastEnc() && bGPB)
        {
          iRefPic = 1;
        }
        RefPic  eRefPic = (iRefPic ? REF_PIC_1 : REF_PIC_0);

        Bool bChanged = false;

        uiBitsTemp = uiMbBits[2] + uiMotBits[1 - iRefPic];

        // call ME
        xMotionEstimation(pcCU, pcOrgYuv, iPartIdx, eRefPic, &cMvPredBi[iRefPic], cMvTemp[iRefPic], uiBitsTemp, uiCostTemp, true);

        if (uiCostTemp < uiCostBi)
        {
          bChanged = true;

          cMvBi[iRefPic] = cMvTemp[iRefPic];

          uiCostBi = uiCostTemp;
          uiMotBits[iRefPic] = uiBitsTemp - uiMbBits[2] - uiMotBits[1 - iRefPic];
          uiBits[2] = uiBitsTemp;

          //  Set motion
          pcCU->getCUMvField(eRefPic)->setAllMvField(cMvBi[iRefPic], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);

          TComYuv* pcYuvPred = &m_acYuvPred[iRefPic];
          motionCompensation(pcCU, pcYuvPred, eRefPic, iPartIdx);
        }

        if (!bChanged)
        {
          break;
        }
      } // for loop-iter
    } // if (B_PICTURE)

    //  Clear Motion Field
    pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvZero, pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
    pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMvZero, pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);

    // Set Motion Field_
    if (bGPB)
    {
      uiCost[1] = MAX_UINT;
    }

    if (uiCostBi <= uiCost[0] && uiCostBi <= uiCost[1])
    {
      uiLastMode = 2;

      pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvBi[0], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
      pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMvBi[1], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
      pcCU->setInterDirSubParts(3, uiPartAddr, pcCU->getDepth(0), iPartIdx);
    }
    else if (uiCost[0] <= uiCost[1])
    {
      uiLastMode = 0;

      pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMv[0], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
      pcCU->setInterDirSubParts(1, uiPartAddr, pcCU->getDepth(0), iPartIdx);
    }
    else
    {
      uiLastMode = 1;

      pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMv[1], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
      pcCU->setInterDirSubParts(2, uiPartAddr, pcCU->getDepth(0), iPartIdx);
    }

    //  MC
    motionCompensation(pcCU, rpcPredYuv, REF_PIC_X, iPartIdx);

  } //  end of for ( Int iPartIdx = 0; iPartIdx < iNumPart; iPartIdx++ )

  return;
}
#endif


Void TEncSearch::xGetBlkBits( PartSize ePartSize, Bool bPPicture, Int iPartIdx, UInt uiLastMode, UInt uiBlkBit[3])
{
  if ( ePartSize == SIZE_2Nx2N )
  {
    uiBlkBit[0] = (! bPPicture) ? 3 : 1;
    uiBlkBit[1] = 3;
    uiBlkBit[2] = 5;
  }
  else if ( (ePartSize == SIZE_2NxN || ePartSize == SIZE_2NxnU) || ePartSize == SIZE_2NxnD )
  {
    UInt aauiMbBits[2][3][3] = { { {0,0,3}, {0,0,0}, {0,0,0} } , { {5,7,7}, {7,5,7}, {9-3,9-3,9-3} } };
    if ( bPPicture )
    {
      uiBlkBit[0] = 3;
      uiBlkBit[1] = 0;
      uiBlkBit[2] = 0;
    }
    else
    {
      ::memcpy( uiBlkBit, aauiMbBits[iPartIdx][uiLastMode], 3*sizeof(UInt) );
    }
  }
  else if ( (ePartSize == SIZE_Nx2N || ePartSize == SIZE_nLx2N) || ePartSize == SIZE_nRx2N )
  {
    UInt aauiMbBits[2][3][3] = { { {0,2,3}, {0,0,0}, {0,0,0} } , { {5,7,7}, {7-2,7-2,9-2}, {9-3,9-3,9-3} } };
    if ( bPPicture )
    {
      uiBlkBit[0] = 3;
      uiBlkBit[1] = 0;
      uiBlkBit[2] = 0;
    }
    else
    {
      ::memcpy( uiBlkBit, aauiMbBits[iPartIdx][uiLastMode], 3*sizeof(UInt) );
    }
  }
  else if ( ePartSize == SIZE_NxN )
  {
    uiBlkBit[0] = (! bPPicture) ? 3 : 1;
    uiBlkBit[1] = 3;
    uiBlkBit[2] = 5;
  }
  else
  {
    printf("Wrong!\n");
    assert( 0 );
  }
}
#if PSKIP
Void TEncSearch::scalingMV(TComMv &cMvPredL0, TComMv tempmv, Int curT, Int colT)
{
	cMvPredL0.setHor(Clip3(-32768, 32767, ((long long int)(curT)* tempmv.getHor() * (MULTI / colT) + HALF_MULTI) >> OFFSET));
	cMvPredL0.setVer(Clip3(-32768, 32767, ((long long int)(curT)* tempmv.getVer() * (MULTI / colT) + HALF_MULTI) >> OFFSET));
}
#endif
#if BSKIP
Void TEncSearch::scalingDirectMvHor(Int mv_x, Int dist2, Int dist4, Int dist5, TComMv* cMvPredL0, TComMv* cMvPredL1)
{
	if (mv_x < 0) {
		cMvPredL0->setHor(-scaleMvDirect(mv_x, dist2, -dist4));
		cMvPredL1->setHor(scaleMvDirect(mv_x, dist2, -dist5));
	}
	else {
		cMvPredL0->setHor(scaleMvDirect(mv_x, dist2, dist4));
		cMvPredL1->setHor(-scaleMvDirect(mv_x, dist2, dist5));
	}
}
Void TEncSearch::scalingDirectMvVer(int mv_y, int dist1, int dist2, int dist3, int dist4, int dist5, TComMv* cMvPredL0, TComMv* cMvPredL1)
{
	if (mv_y < 0) {
		cMvPredL0->setVer(-scaleMvDirect(mv_y, dist2, -dist4));
		cMvPredL1->setVer(scaleMvDirect(mv_y, dist2, -dist5));
	}
	else {
		cMvPredL0->setVer(scaleMvDirect(mv_y, dist2, dist4));
		cMvPredL1->setVer(-scaleMvDirect(mv_y, dist2, dist5));
	}
}


Int TEncSearch::scaleMvDirect(Int mv, Int dist_dst, Int dist_src)
{
	return ((long long int)(MULTI / dist_dst) * (1 + dist_src * mv) - 1) >> OFFSET;
}

#endif

#if DMH
Bool TEncSearch::checkDMHMode(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
	if (!pcCU->getPicture()->isInterF())
	{
		return 0;
	}

	if (pcCU->getLog2CUSize(uiAbsPartIdx) == 3 && pcCU->getPartitionSize(uiAbsPartIdx) != 0
		&& pcCU->getPicture()->isInterF())
	{
		return 0;
	}
	UInt uiInterDir0 = pcCU->getInterDir(uiAbsPartIdx);  ///< uiIntraDir 1 to 4
	if (pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N)
	{
		if (uiInterDir0 == INTER_FORWARD)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else if ((pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxN && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_Nx2N) ||
		(pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxnU && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N))
	{
		UInt uiInterDir1 = 0;
		UInt uiPartOffset = (pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1)) >> 2;
		switch (pcCU->getPartitionSize(uiAbsPartIdx))
		{
		case SIZE_2NxN:
		{
			uiAbsPartIdx += uiPartOffset << 1;
			uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
			break;
		}
		case SIZE_Nx2N:
		{
			uiAbsPartIdx += uiPartOffset;
			uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
			break;
		}
		case SIZE_2NxnU:
		{
			uiAbsPartIdx += uiPartOffset >> 1;
			uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
			break;
		}
		case SIZE_2NxnD:
		{
			uiAbsPartIdx += (uiPartOffset << 1) + (uiPartOffset >> 1);
			uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
			break;
		}
		case SIZE_nLx2N:
		{
			uiAbsPartIdx += uiPartOffset >> 2;
			uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
			break;
		}
		case SIZE_nRx2N:
		{
			uiAbsPartIdx += uiPartOffset + (uiPartOffset >> 2);
			uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
			break;
		}
		default:
			break;
		}
		if ((uiInterDir0 == INTER_FORWARD) && (uiInterDir1 == INTER_FORWARD))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else if (pcCU->getPartitionSize(uiAbsPartIdx == SIZE_NxN))
	{
		if (uiInterDir0 == INTER_FORWARD)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	return 0;
}

Char TEncSearch::dmhInterPred(TComDataCU* pcCU, TComYuv* rpcOrgYuv, TComYuv*& rpcPredYuv, Int dmh_mode)
{

	Int uiSize = 1 << pcCU->getLog2CUSize(0);
	m_cYuvPredTemp.clear(pcCU->getZorderIdxInCU(), uiSize);
	rpcPredYuv->clear(pcCU->getZorderIdxInCU(), uiSize);

	Int  pic_pix_x, pic_pix_y;
	Char mvRangeFlag = 1;
	Int maxDmhMode = 2 * DMH_MODE_NUM - 1;

	Int           iNumPart = pcCU->getNumPartInter();
	Int           iNumPredDir = pcCU->getPicture()->isInterP() ? 1 : 2;
	UInt          uiPartAddr;
	Int           iRoiWidth, iRoiHeight;

	for (Int uiPartIdx = 0; uiPartIdx < iNumPart; uiPartIdx++)
	{
		m_acYuvPred[REF_PIC_0].clear(pcCU->getZorderIdxInCU(), uiSize);
		m_acYuvPred[REF_PIC_1].clear(pcCU->getZorderIdxInCU(), uiSize);
		pcCU->getPartIndexAndSize(uiPartIdx, uiPartAddr, iRoiWidth, iRoiHeight);
		Int    refPicId = pcCU->getCUMvField(REF_PIC_0)->getRefIdx(uiPartAddr);
		TComMv rcMv = pcCU->getCUMvField(REF_PIC_0)->getMv(uiPartAddr);
		TComMv rcMvPred = pcCU->getMvFieldPred(uiPartAddr, REF_PIC_0, refPicId).getMv();

		pcCU->getInterPUxy(uiPartIdx, pic_pix_x, pic_pix_y);
		/*mvRangeFlag *= check_mv_range(pcCU->getLog2CUSize(0), rcMv.getHor(), rcMv.getVer(), pic_pix_x, pic_pix_y, pcCU->getPartitionSize(0) + 1, dmh_pos[dmh_mode][0][0], dmh_pos[dmh_mode][0][1]);
		mvRangeFlag *= check_mv_range(pcCU->getLog2CUSize(0), rcMv.getHor(), rcMv.getVer(), pic_pix_x, pic_pix_y, pcCU->getPartitionSize(0) + 1, dmh_pos[dmh_mode][1][0], dmh_pos[dmh_mode][1][1]);
		mvRangeFlag *= check_mvd((rcMv.getHor() - rcMvPred.getHor()), (rcMv.getVer() - rcMvPred.getVer()));

		if (mvRangeFlag == 0)
			return mvRangeFlag;
*/
		xPredDmhMode(pcCU, uiPartIdx, uiPartAddr, iRoiWidth, iRoiHeight, REF_PIC_0);

		m_acYuvPred[REF_PIC_0].copyPartToPartYuv(rpcPredYuv, pcCU->getZorderIdxInCU() + uiPartAddr, pcCU->getZorderIdxInCU() + uiPartAddr, iRoiWidth, iRoiHeight);
		//m_acYuvPred[REF_PIC_0].copyToPartYuv(rpcPredYuv, uiPartIdx/*, uiPartIdx, iRoiWidth, iRoiHeight*/);
	}

	return mvRangeFlag;
	//set best dmh mode
}
#endif

#if RPS
Void TEncSearch::CalculateSAD(TComDataCU* pcCU, TComYuv* pcYuvOrg, RefPic eRefPic, Int iRefIdxPred, UInt& ruiCost)
{
  Int           iRoiWidth;
  Int           iRoiHeight;
  TComYuv*      pcYuv = pcYuvOrg;
  UInt        uiPartAddr = pcCU->getZorderIdxInCU();
  //rps
  Pel*        piRefY = pcCU->getPicture()->getPicHeader()->getRefPic(eRefPic, iRefIdxPred)->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), uiPartAddr);
  Int         iRefStride = pcCU->getPicture()->getPicHeader()->getRefPic(eRefPic, iRefIdxPred)->getPicYuvRec()->getStride();
  TComPattern*  pcPatternKey = pcCU->getPattern();
  iRoiWidth = pcCU->getWidth(0);
  iRoiHeight = pcCU->getHeight(0);

  pcPatternKey->initPattern(pcYuv->getLumaAddr(uiPartAddr),
    pcYuv->getCbAddr(uiPartAddr),
    pcYuv->getCrAddr(uiPartAddr),
    iRoiWidth, iRoiHeight,
    pcYuv->getStride(),
    0, 0, 0, 0);
#if FULLSEARCHFAST
  m_cDistParam.m_uiMvCost = 0;
  m_cDistParam.m_uiSadBest = MAX_UINT;
#endif
  m_pcRdCost->setDistParam(pcPatternKey, piRefY, iRefStride, m_cDistParam);

  m_cDistParam.pCur = piRefY;
  ruiCost = m_cDistParam.DistFunc(&m_cDistParam);
}
#endif
#if INTER614
Void TEncSearch::SetRef(TComDataCU* pcCU, Int *adjustRef, Int *maxRef)
{

	*adjustRef = (pcCU->getPicture()->getPictureType() == B_PICTURE ? 1 : 0);
	*adjustRef = min(*adjustRef, *maxRef - 1);

	*maxRef = min(*maxRef, pcCU->getPicture()->getSPS()->getNumberReferenceFrames());
	*adjustRef = 0;

	if (*maxRef > 1 && pcCU->getPicture()->getPictureType() == B_PICTURE)
	{
		*maxRef = 1;
	}
}
#endif
#if F_DHP_SYC
Void TEncSearch::xPatternSearchForDHP(TComDataCU *pcCU, Int uiPartIdx, RefPic iRefPic, TComMv& rcMv, Int iRefIdxPred, UInt& ruiSAD, TComYuv *pcOrgYuv)
{
	UInt  uiSad = 0;
	UInt  uiSadBest = MAX_UINT;
	Int   iBestX = 0;
	Int   iBestY = 0;
	Int   x, y;
	Int iRefIdxDual[2];
	iRefIdxDual[REF_PIC_0] = iRefIdxPred;
	iRefIdxDual[REF_PIC_1] = (iRefIdxPred == 0) ? 1 : 0;
	TComMv cMvDual[2];

#if !F_DHP_SYC_OP
	TComYuv* m_acYuvPredDual[2];
	//pcYuvPred.create(g_uiLog2MaxCUSize);
	m_acYuvPredDual[0] = new TComYuv;
	m_acYuvPredDual[0]->create(g_uiLog2MaxCUSize);
	m_acYuvPredDual[1] = new TComYuv;
	m_acYuvPredDual[1]->create(g_uiLog2MaxCUSize);
#endif
	Int  iWidth;
	Int  iHeight;
	UInt uiPartAddr;
	pcCU->getPartIndexAndSize(uiPartIdx, uiPartAddr, iWidth, iHeight);
	//  Set motion


	for (y = -4; y <= 4; y++)
	{
		for (x = -4; x <= 4; x++)
		{
			//uiSad = m_pcRdCost->getCost(x, y);
			cMvDual[REF_PIC_0].setHor(rcMv.getHor() + x);
			cMvDual[REF_PIC_0].setVer(rcMv.getVer() + y);

			pcCU->MV_Scale(pcCU, cMvDual[REF_PIC_1], cMvDual[REF_PIC_0], iRefIdxDual[REF_PIC_0], iRefIdxDual[REF_PIC_1]);
			//cMvDual[REF_PIC_1].setHor(pcCU->scaleMotionVector_y1(cMvDual[REF_PIC_0].getHor(), REF_PIC_0, iRefIdxDual[REF_PIC_0], iRefIdxDual[REF_PIC_0], iRefIdxDual[REF_PIC_1]));
			//cMvDual[REF_PIC_1].setVer(pcCU->scaleMotionVector_y1(cMvDual[REF_PIC_0].getVer(), REF_PIC_0, iRefIdxDual[REF_PIC_0], iRefIdxDual[REF_PIC_0], iRefIdxDual[REF_PIC_1]));


			pcCU->getCUMvField(REF_PIC_0)->setAllMv(cMvDual[REF_PIC_1], pcCU->getPartitionSize(0), uiPartAddr, uiPartIdx, 0);
			pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(iRefIdxDual[REF_PIC_1], pcCU->getPartitionSize(0), uiPartAddr, uiPartIdx, 0);
			//motionCompensation(pcCU, &pcYuvPredDual[REF_PIC_1], REF_PIC_0, uiPartIdx);
			xPredInterUni(pcCU, uiPartAddr, iWidth, iHeight, REF_PIC_0, m_acYuvPredDual[REF_PIC_1], uiPartIdx);

			pcCU->getCUMvField(REF_PIC_0)->setAllMv(cMvDual[REF_PIC_0], pcCU->getPartitionSize(0), uiPartAddr, uiPartIdx, 0);
			pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(iRefIdxDual[REF_PIC_0], pcCU->getPartitionSize(0), uiPartAddr, uiPartIdx, 0);
			//motionCompensation(pcCU, &pcYuvPredDual[REF_PIC_0], REF_PIC_0, uiPartIdx);
			xPredInterUni(pcCU, uiPartAddr, iWidth, iHeight, REF_PIC_0, m_acYuvPredDual[REF_PIC_0], uiPartIdx);

			//pcYuvPred->addAvg(pcYuvPredDual[0], pcYuvPredDual[1], pcCU->getZorderIdxInCU() + uiPartAddr, iWidth, iHeight, pcCU->getZorderIdxInCU() + uiPartAddr);


			//calculate SAD
			Int x, y;

			Pel* pSrc0 = pcOrgYuv->getLumaAddr(pcCU->getZorderIdxInCU() + uiPartAddr);
			Pel* pSrc1 = m_acYuvPredDual[0]->getLumaAddr(uiPartAddr);
			Pel* pSrc2 = m_acYuvPredDual[1]->getLumaAddr(uiPartAddr);
			Int  iSrc0Stride = pcOrgYuv->getStride();
			Int  iSrc1Stride = m_acYuvPredDual[0]->getStride();
			Int  iSrc2Stride = m_acYuvPredDual[1]->getStride();
			for (y = iHeight - 1; y >= 0; y--)
			{
				for (x = iWidth - 1; x >= 0; x--)
				{
					uiSad += abs(2 * pSrc0[x] - pSrc1[x] - pSrc2[x]);
				}
				pSrc0 += iSrc0Stride;
				pSrc1 += iSrc1Stride;
				pSrc2 += iSrc2Stride;
			}
			if (uiSad < uiSadBest)
			{
				uiSadBest = uiSad;
				iBestX = x;
				iBestY = y;
			}
			uiSad = 0;
		}

	}

#if !F_DHP_SYC_OP
	m_acYuvPredDual[0]->destroy();
	delete m_acYuvPredDual[0];
	m_acYuvPredDual[0] = NULL;
	m_acYuvPredDual[1]->destroy();
	delete m_acYuvPredDual[1];
	m_acYuvPredDual[1] = NULL;
	//pcYuvPred.destroy();
#endif

	rcMv.set(rcMv.getHor() + iBestX, rcMv.getVer() + iBestY);

	ruiSAD = uiSadBest;
	return;
}
#endif

#if rd_sym
Void TEncSearch::xMotionEstimationSym(TComDataCU* pcCU, TComYuv* pcYuvOrg, Int iPartIdx, RefPic eRefPic, TComMv* pcMvPred, TComMv* pcBwMvPred, Int iRefIdxPred, TComMv& rcMv, TComMv& rcBwMv, UInt& ruiBits, UInt& ruiCost, Bool bBi)
{
	UInt          uiPartAddr = pcCU->getZorderIdxInCU();

	Int           iRoiWidth;
	Int           iRoiHeight;

	TComMv        cMvHalf, cMvQter;
	TComMv        cMvSrchRngLT;
	TComMv        cMvSrchRngRB;

	TComYuv*      pcYuv = pcYuvOrg;

	//  ZP   m_iSearchRange 
	m_iSearchRange = m_aiAdaptSR[eRefPic][iRefIdxPred];

	Int           iSrchRng = (bBi ? 8 : m_iSearchRange);
	TComPattern*  pcPatternKey = pcCU->getPattern();

	Double        fWeight = 1.0;

	UInt uiPuPartAddr;
	pcCU->getPartIndexAndSize(iPartIdx, uiPuPartAddr, iRoiWidth, iRoiHeight);
	uiPartAddr += uiPuPartAddr;

	//  Search key pattern initialization
	pcPatternKey->initPattern(pcYuv->getLumaAddr(uiPartAddr),
		pcYuv->getCbAddr(uiPartAddr),
		pcYuv->getCrAddr(uiPartAddr),
		iRoiWidth, iRoiHeight,
		pcYuv->getStride(),
		0, 0, 0, 0);

	//RPS
	Pel*        piRefY = pcCU->getPicture()->getPicHeader()->getRefPic(eRefPic, iRefIdxPred)->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), uiPartAddr);
	Int         iRefStride = pcCU->getPicture()->getPicHeader()->getRefPic(eRefPic, iRefIdxPred)->getPicYuvRec()->getStride();

	Pel*        piSymRef = pcCU->getPicture()->getPicHeader()->getRefPic((RefPic)(1 - (Int)eRefPic), 0)->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), uiPartAddr);
	Int         iSymRefStride = pcCU->getPicture()->getPicHeader()->getRefPic((RefPic)(1 - (Int)eRefPic), 0)->getPicYuvRec()->getStride();

	TComMv      cMvPred = *pcMvPred;
#if DEBUG_ME
	Int width = pcPatternKey->getROIYWidth();
	Int height = pcPatternKey->getROIYHeight();
	printf("%dx%d\t", width, height);
	printf("pred_mv_x =%d\t pred_mv_y= %d\t", cMvPred.getHor(), cMvPred.getVer());
#endif

	if (bBi)  xSetSearchRange(pcCU, rcMv, iSrchRng, cMvSrchRngLT, cMvSrchRngRB);
	else        xSetSearchRange(pcCU, cMvPred, iSrchRng, cMvSrchRngLT, cMvSrchRngRB); //在RD中相当于predmv/4

	m_pcRdCost->getMotionCost(1, 0);

	m_pcRdCost->setPredictor(*pcMvPred);
	m_pcRdCost->setCostScale(2);

	//  Do integer search
#if DIFSym_bug
	if (!m_iFastSearch)
#else
	if (!m_iFastSearch || bBi)
#endif
	{
#if INTER612
		xPatternSearch(pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, cMvPred, ruiCost);
#else
		xPatternSearch(pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, ruiCost);
#endif
	}
	else
	{
#if wlq_FME 
		xPatternSearchFast(pcCU, pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, cMvPred, ruiCost, iRefIdxPred, iPartIdx);
#else
		rcMv = *pcMvPred;
		xPatternSearchFast(pcCU, pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, ruiCost);
#endif
	}
#if DEBUG_FME
	rcMv.set(2, -1);
#endif
#if wlq_Mv_check_temp
	{
		Int tempH = rcMv.getHor() << 2;
		Int tempV = rcMv.getVer() << 2;
		Char mv_range_flag = 1;
		Int pic_pix_x, pic_pix_y;
		pcCU->getInterPUxy(iPartIdx, pic_pix_x, pic_pix_y);
		mv_range_flag = check_mv_range(pcCU->getLog2CUSize(0), tempH, tempV, pic_pix_x, pic_pix_y, pcCU->getPartitionSize(0) + 1, 0, 0);
		mv_range_flag *= check_mvd((tempH - cMvPred.getHor()), (tempV - cMvPred.getVer()));
		if (!mv_range_flag)
		{
			ruiCost = 0x0EEEEEEE;
			ruiCost = ruiCost;
			return;
		}
	}
#endif


	m_pcRdCost->getMotionCost(1, 0);
	m_pcRdCost->setCostScale(1);
	// printf("(%d,%d)\t",rcMv.getHor(),rcMv.getVer());

	Int iMvX = (cMvPred.getHor() >> 1) << 1;
	Int iMvY = (cMvPred.getVer() >> 1) << 1;

	xPatternSearchFracDIFSym(pcCU, pcPatternKey, piRefY, iRefStride, piSymRef, iSymRefStride, iMvX, iMvY, &rcMv, cMvHalf, cMvQter, ruiCost);

	m_pcRdCost->setCostScale(0);
	rcMv <<= 2;
	rcMv += (cMvHalf <<= 1);
	rcMv += cMvQter;

#if !INTER614
	UInt uiMvBits = m_pcRdCost->getBits(rcMv.getHor(), rcMv.getVer());
	ruiBits += uiMvBits;
#endif
#if INTER614
	ruiCost = (UInt)(floor(fWeight * ((Double)ruiCost + ruiBits)));
#else
#if WLQ_CLOSE_REF_COST_FOR_DEBUG
	if (bBi)
	{
		ruiCost = (UInt)(floor(fWeight * ((Double)(ruiCost))));
		//ruiCost = (UInt)(fWeight * ((Double)(ruiCost + m_pcRdCost->getCost(rcMv.getHor(), rcMv.getVer()))));
		m_pcRdCost->setPredictor(*pcBwMvPred);
		ruiCost += m_pcRdCost->getCost(rcBwMv.getHor(), rcBwMv.getVer());
	}
	else
	{
		ruiCost = ruiCost;
	}
#else
	ruiCost = (UInt)(floor(fWeight * ((Double)ruiCost - (Double)m_pcRdCost->getCost(uiMvBits))) + (Double)m_pcRdCost->getCost(ruiBits));
#endif

#endif
#if DEBUG_ME
	printf("cost=%d\t", ruiCost);
	//printf("mv_x =%d\t mv_y= %d\t refIdx=%d\n", rcMv.getHor(), rcMv.getVer(), iRefIdxPred);
	printf("mv_x =%d\t mv_y= %d\n", rcMv.getHor(), rcMv.getVer());
#endif
}
#endif

#if rd_bipred
Void TEncSearch::xMotionEstimationBi(TComDataCU* pcCU, TComYuv* pcYuvOrg, Int iPartIdx, RefPic eRefPic, TComMv* pcMvPred, TComMv* pcBwMvPred, Int iRefIdxPred, TComMv& rcMv, TComMv& rcBwMv, UInt& ruiBits, UInt& ruiCost, Bool bBi)
{
	UInt          uiPartAddr = pcCU->getZorderIdxInCU();

	Int           iRoiWidth;
	Int           iRoiHeight;

	TComMv        cMvHalf, cMvQter;
	TComMv        cMvSrchRngLT;
	TComMv        cMvSrchRngRB;

	TComYuv*      pcYuv = pcYuvOrg;

	//  ZP   m_iSearchRange 
	m_iSearchRange = m_aiAdaptSR[eRefPic][iRefIdxPred];

	Int           iSrchRng = (bBi ? 8 : m_iSearchRange);
	TComPattern*  pcPatternKey = pcCU->getPattern();

	Double        fWeight = 1.0;

	UInt uiPuPartAddr;
	pcCU->getPartIndexAndSize(iPartIdx, uiPuPartAddr, iRoiWidth, iRoiHeight);
	uiPartAddr += uiPuPartAddr;

	//  Search key pattern initialization
	pcPatternKey->initPattern(pcYuv->getLumaAddr(uiPartAddr),
		pcYuv->getCbAddr(uiPartAddr),
		pcYuv->getCrAddr(uiPartAddr),
		iRoiWidth, iRoiHeight,
		pcYuv->getStride(),
		0, 0, 0, 0);

	//RPS
	Pel*        piRefY = pcCU->getPicture()->getPicHeader()->getRefPic(eRefPic, iRefIdxPred)->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), uiPartAddr);
	Int         iRefStride = pcCU->getPicture()->getPicHeader()->getRefPic(eRefPic, iRefIdxPred)->getPicYuvRec()->getStride();

	Pel*        piBwRef = (&m_acYuvPred[(RefPic)(1 - (Int)eRefPic)])->getLumaAddr(uiPartAddr);
	Int         iBwRefStride = (&m_acYuvPred[(RefPic)(1 - (Int)eRefPic)])->getStride();

	TComMv      cMvPred = *pcMvPred;
#if DEBUG_ME
	Int width = pcPatternKey->getROIYWidth();
	Int height = pcPatternKey->getROIYHeight();
	printf("%dx%d\t", width, height);
	printf("pred_mv_x =%d\t pred_mv_y= %d\t", cMvPred.getHor(), cMvPred.getVer());
#endif

	if (bBi)  xSetSearchRange(pcCU, rcMv, iSrchRng, cMvSrchRngLT, cMvSrchRngRB);
	else        xSetSearchRange(pcCU, cMvPred, iSrchRng, cMvSrchRngLT, cMvSrchRngRB); //在RD中相当于predmv/4

	m_pcRdCost->getMotionCost(1, 0);

	m_pcRdCost->setPredictor(*pcMvPred);
	m_pcRdCost->setCostScale(2);

	//  Do integer search
#if DIFSym_bug
	if (!m_iFastSearch)
#else
	if (!m_iFastSearch || bBi)
#endif
	{
#if INTER612
		xPatternSearch(pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, cMvPred, ruiCost);
#else
		xPatternSearch(pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, ruiCost);
#endif
	}
	else
	{
#if wlq_FME 
		xPatternSearchFast(pcCU, pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, cMvPred, ruiCost, iRefIdxPred, iPartIdx);
#else
		rcMv = *pcMvPred;
		xPatternSearchFast(pcCU, pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, ruiCost);
#endif
	}
#if DEBUG_FME
	rcMv.set(2, -1);
#endif

#if wlq_Mv_check_temp
	{
		Int tempH = rcMv.getHor() << 2;
		Int tempV = rcMv.getVer() << 2;
		Char mv_range_flag = 1;
		Int pic_pix_x, pic_pix_y;
		pcCU->getInterPUxy(iPartIdx, pic_pix_x, pic_pix_y);
		mv_range_flag = check_mv_range(pcCU->getLog2CUSize(0), tempH, tempV, pic_pix_x, pic_pix_y, pcCU->getPartitionSize(0) + 1, 0, 0);
		mv_range_flag *= check_mvd((tempH - cMvPred.getHor()), (tempV - cMvPred.getVer()));
		if (!mv_range_flag)
		{
			ruiCost = 0x0EEEEEEE;
			ruiCost = ruiCost;
			return;
		}
	}
#endif


	m_pcRdCost->getMotionCost(1, 0);
	m_pcRdCost->setCostScale(1);
	// printf("(%d,%d)\t",rcMv.getHor(),rcMv.getVer());

#if FME_SUB
	Int iMvX = cMvPred.getHor();
	Int iMvY = cMvPred.getVer();
	if (!m_iFastSearch || bBi)
		xPatternSearchFracDIFBi(pcCU, pcPatternKey, piRefY, iRefStride, piBwRef, iBwRefStride, iMvX, iMvY, &rcMv, cMvHalf, cMvQter, ruiCost);
	else
	{
		if (pcCU->getPartitionSize(uiPartAddr - pcCU->getZorderIdxInCU()) > SIZE_Nx2N)
		{
			//test
			xPatternSearchFastFracDIF(pcCU, pcPatternKey, piRefY, iRefStride, iPartIdx, iMvX, iMvY, &rcMv, cMvHalf, cMvQter, ruiCost);
		}
		else
			xPatternSearchFracDIFBi(pcCU, pcPatternKey, piRefY, iRefStride, piBwRef, iBwRefStride, iMvX, iMvY, &rcMv, cMvHalf, cMvQter, ruiCost);
	}
#else
#if RD_PMVR
	Int iMvX = (cMvPred.getHor() >> 1) << 1;
	Int iMvY = (cMvPred.getVer() >> 1) << 1;
#if b_cost_revise
	xPatternSearchFracDIFBi(pcCU, pcPatternKey, piRefY, iRefStride, piBwRef, iBwRefStride, iMvX, iMvY, &rcMv, cMvHalf, cMvQter, ruiCost);
#else
	xPatternSearchFracDIF(pcCU, pcPatternKey, piRefY, iRefStride, iMvX, iMvY, &rcMv, cMvHalf, cMvQter, ruiCost);
#endif
#else
	xPatternSearchFracDIF(pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost);
#endif


#endif

	m_pcRdCost->setCostScale(0);
	rcMv <<= 2;
	rcMv += (cMvHalf <<= 1);
	rcMv += cMvQter;

#if wlq_Mv_check
	Char mv_range_flag = 1;
	Int pic_pix_x, pic_pix_y;
	pcCU->getInterPUxy(iPartIdx, pic_pix_x, pic_pix_y);
	mv_range_flag = check_mv_range(pcCU->getLog2CUSize(0), rcMv.getHor(), rcMv.getVer(), pic_pix_x, pic_pix_y, pcCU->getPartitionSize(0) + 1, 0, 0);
	mv_range_flag *= check_mvd((rcMv.getHor() - cMvPred.getHor()), (rcMv.getVer() - cMvPred.getVer()));
	if (!mv_range_flag)
	{
		ruiCost = 0x0EEEEEEE;
	}
#endif

	UInt uiMvBits = m_pcRdCost->getBits(rcMv.getHor(), rcMv.getVer());
	ruiBits += uiMvBits;

#if WLQ_CLOSE_REF_COST_FOR_DEBUG
	if (bBi)
	{
		ruiCost = (UInt)(floor(fWeight * ((Double)(ruiCost))));
		//ruiCost = (UInt)(fWeight * ((Double)(ruiCost + m_pcRdCost->getCost(rcMv.getHor(), rcMv.getVer()))));
		m_pcRdCost->setPredictor(*pcBwMvPred);
		ruiCost += m_pcRdCost->getCost(rcBwMv.getHor(), rcBwMv.getVer());
	}
	else
	{
		ruiCost = ruiCost;
	}
	//ruiCost += (UInt)((Double)m_pcRdCost->getCost(ruiBits - uiMvBits));
#else
	ruiCost = (UInt)(floor(fWeight * ((Double)ruiCost - (Double)m_pcRdCost->getCost(uiMvBits))) + (Double)m_pcRdCost->getCost(ruiBits));
#endif
#if DEBUG_ME
	printf("cost=%d\t", ruiCost);
	printf("mv_x=%d\t mv_y=%d\t refIdx=%d\n", rcMv.getHor(), rcMv.getVer(), iRefIdxPred);
#endif
}
#endif

#if RPS
Void TEncSearch::xMotionEstimation(TComDataCU* pcCU, TComYuv* pcYuvOrg, Int iPartIdx, RefPic eRefPic, TComMv* pcMvPred, Int iRefIdxPred, TComMv& rcMv, UInt& ruiBits, UInt& ruiCost, Bool bBi
#if F_DHP_SYC
	, Bool bDHP
#endif
	)
{
  UInt          uiPartAddr = pcCU->getZorderIdxInCU();

  Int           iRoiWidth;
  Int           iRoiHeight;

  TComMv        cMvHalf, cMvQter;
  TComMv        cMvSrchRngLT;
  TComMv        cMvSrchRngRB;

  TComYuv*      pcYuv = pcYuvOrg;

  //  ZP   m_iSearchRange 
  m_iSearchRange = m_aiAdaptSR[eRefPic][iRefIdxPred];

  Int           iSrchRng = (bBi ? 8 : m_iSearchRange);
  TComPattern*  pcPatternKey = pcCU->getPattern();

  Double        fWeight = 1.0;

#if DCTIF
	pcCU->getPartIndexAndSize(iPartIdx, uiPartAddr, iRoiWidth, iRoiHeight);
#else
  UInt uiPuPartAddr;
  pcCU->getPartIndexAndSize(iPartIdx, uiPuPartAddr, iRoiWidth, iRoiHeight);
  uiPartAddr += uiPuPartAddr;
#endif

  if (bBi)
  {
    TComYuv*  pcYuvOther = &m_acYuvPred[(RefPic)(1 - (Int)eRefPic)];
    pcYuv = &m_cYuvPredTemp;

    pcYuvOrg->copyPartToPartYuv(pcYuv, uiPartAddr, uiPartAddr, iRoiWidth, iRoiHeight);

    pcYuv->removeHighFreq(pcYuvOther, uiPartAddr, iRoiWidth, iRoiHeight);

    fWeight = 0.5;
  }

  //  Search key pattern initialization
#if DCTIF
	pcPatternKey->initPattern(pcYuv->getRealLumaAddr(uiPartAddr),
		pcYuv->getRealCbAddr(uiPartAddr),
		pcYuv->getRealCrAddr(uiPartAddr),
		iRoiWidth, iRoiHeight,
		pcYuv->getRealStride(),
		0, 0, 0, 0);
	Pel*        piRefY = pcCU->getPicture()->getPicHeader()->getRefPic(eRefPic, iRefIdxPred)->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), uiPartAddr + pcCU->getZorderIdxInCU());
#else
  pcPatternKey->initPattern(pcYuv->getLumaAddr(uiPartAddr),
    pcYuv->getCbAddr(uiPartAddr),
    pcYuv->getCrAddr(uiPartAddr),
    iRoiWidth, iRoiHeight,
    pcYuv->getStride(),
    0, 0, 0, 0);

  //RPS
  Pel*        piRefY = pcCU->getPicture()->getPicHeader()->getRefPic(eRefPic, iRefIdxPred)->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), uiPartAddr);
#endif
	Int         iRefStride = pcCU->getPicture()->getPicHeader()->getRefPic(eRefPic, iRefIdxPred)->getPicYuvRec()->getStride();

  TComMv      cMvPred = *pcMvPred;
#if DEBUG_ME
	//TComMv cLastMv = rcMv;
	Int width = pcPatternKey->getROIYWidth();
	Int height = pcPatternKey->getROIYHeight();
	printf("Addr=%d %dx%d\t", pcCU->getAddr(), width, height);
	printf("pred_mv_x =%d\t pred_mv_y=%d\t", cMvPred.getHor(), cMvPred.getVer());
#endif
  //printf("%dx%d\t", pcCU->getWidth(0), pcCU->getHeight(0));
//  cMvPred.setHor(64);
//  cMvPred.setVer(64);

#if wlq_pred_clip
	if (!m_iFastSearch)
	{
		if (bBi)  xSetSearchRange(pcCU, rcMv, iSrchRng, cMvSrchRngLT, cMvSrchRngRB);
		else        xSetSearchRange(pcCU, cMvPred, iSrchRng, cMvSrchRngLT, cMvSrchRngRB); //在RD中相当于predmv/4
	}
#else
	if (bBi)  xSetSearchRange(pcCU, rcMv, iSrchRng, cMvSrchRngLT, cMvSrchRngRB);
	else        xSetSearchRange(pcCU, cMvPred, iSrchRng, cMvSrchRngLT, cMvSrchRngRB); //在RD中相当于predmv/4
#endif

  m_pcRdCost->getMotionCost(1, 0);

  m_pcRdCost->setPredictor(*pcMvPred);
  m_pcRdCost->setCostScale(2);

  //  Do integer search

 
#if DIFSym_bug
  if (!m_iFastSearch)
#else
  if (!m_iFastSearch || bBi)
#endif
  {
#if INTER612
    xPatternSearch(pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, cMvPred, ruiCost);
#else
    xPatternSearch(pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, ruiCost);
#endif
  }
#if F_DHP_SYC
  else if (bDHP)
  {
	  rcMv = cMvPred;
	  xPatternSearchForDHP(pcCU, iPartIdx, eRefPic, rcMv, iRefIdxPred, ruiCost, pcYuv);
  }
#endif
  else
  {
#if wlq_FME 
	  xPatternSearchFast(pcCU, pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, cMvPred, ruiCost, iRefIdxPred, iPartIdx);
#else
    rcMv = *pcMvPred;
    xPatternSearchFast(pcCU, pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, ruiCost);
#endif
  }
  //test
  //rcMv.set(0, 0);
#if wlq_Mv_check_temp
  {
	  Int tempH = rcMv.getHor() << 2;
	  Int tempV = rcMv.getVer() << 2;
	  Char mv_range_flag = 1;
	  Int pic_pix_x, pic_pix_y;
	  pcCU->getInterPUxy(iPartIdx, pic_pix_x, pic_pix_y);
	  mv_range_flag = check_mv_range(pcCU->getLog2CUSize(0), tempH, tempV, pic_pix_x, pic_pix_y, pcCU->getPartitionSize(0) + 1, 0, 0);
	  mv_range_flag *= check_mvd((tempH - cMvPred.getHor()), (tempV - cMvPred.getVer()));
	  if (!mv_range_flag)
	  {
		  ruiCost = 0x0EEEEEEE;
		  ruiCost = ruiCost;
		  return;
	  }
  }
#endif
  m_pcRdCost->getMotionCost(1, 0);
  m_pcRdCost->setCostScale(1);
  //printf("(%d,%d)\t",rcMv.getHor(),rcMv.getVer());
#if RD_PMVR
	Int iMvX = (cMvPred.getHor() >> 1) << 1;
	Int iMvY = (cMvPred.getVer() >> 1) << 1;
	xPatternSearchFracDIF(pcCU, pcPatternKey, piRefY, iRefStride, iMvX, iMvY, &rcMv, cMvHalf, cMvQter, ruiCost);
#else

#if !niu_CloseSubPel
#if !niu_CloseFastSubPel
  if (!m_iFastSearch || bBi)
	  xPatternSearchFracDIF(pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost);
  else
  {
	  if (pcCU->getPartitionSize(uiPartAddr - pcCU->getZorderIdxInCU()) > SIZE_Nx2N)
#if FME_SUB
	  {
		  //test
		  Int iMvX = cMvPred.getHor();
		  Int iMvY = cMvPred.getVer();
		  xPatternSearchFastFracDIF(pcCU, pcPatternKey, piRefY, iRefStride, iPartIdx, iMvX, iMvY, &rcMv, cMvHalf, cMvQter, ruiCost);
	  }
#else
		  //xFastPatternSearchFracDIF(pcCU, pcPatternKey, piRefY, iRefStride, iPartIdx, cMvPred, &rcMv, cMvHalf, cMvQter, ruiCost);
#endif
	  else
		  xPatternSearchFracDIF(pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost);
  }
#else
  xPatternSearchFracDIF(pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost);
#endif
#endif

#endif

  m_pcRdCost->setCostScale(0);

#if F_DHP_SYC
  if (!bDHP)
  {
#endif
	  rcMv <<= 2;
	  rcMv += (cMvHalf <<= 1);
	  rcMv += cMvQter;
#if F_DHP_SYC
  }
#endif


//#if !INTER614
//  UInt uiMvBits = m_pcRdCost->getBits(rcMv.getHor(), rcMv.getVer());
//  ruiBits += uiMvBits;
//#endif
//#if INTER614
//  ruiCost = (UInt)(floor(fWeight * ((Double)ruiCost + ruiBits)));
//#else
//  ruiCost = (UInt)(floor(fWeight * ((Double)ruiCost - (Double)m_pcRdCost->getCost(uiMvBits))) + (Double)m_pcRdCost->getCost(ruiBits));
//
//#endif


#if wlq_Mv_check
  Char mv_range_flag = 1;
  Int pic_pix_x, pic_pix_y;
  pcCU->getInterPUxy(iPartIdx, pic_pix_x, pic_pix_y);
  mv_range_flag = check_mv_range(pcCU->getLog2CUSize(0), rcMv.getHor(), rcMv.getVer(), pic_pix_x, pic_pix_y, pcCU->getPartitionSize(0) + 1, 0, 0);
  mv_range_flag *= check_mvd((rcMv.getHor() - cMvPred.getHor()), (rcMv.getVer() - cMvPred.getVer()));
  if (!mv_range_flag)
  {
	  ruiCost = 0x0EEEEEEE;
  }
#endif

  UInt uiMvBits = m_pcRdCost->getBits(rcMv.getHor(), rcMv.getVer());
  ruiBits += uiMvBits;
#if DEBUG_ME
  printf("mv_x=%d\t mv_y=%d\t refIdx=%d\n", rcMv.getHor(), rcMv.getVer(), iRefIdxPred);
	//printf("cost = %d\t", uiSadBest);
#endif
#if WLQ_CLOSE_REF_COST_FOR_DEBUG
  ruiCost = ruiCost;
  //ruiCost += (UInt)((Double)m_pcRdCost->getCost(ruiBits - uiMvBits));
#else
  ruiCost = (UInt)(floor(fWeight * ((Double)ruiCost - (Double)m_pcRdCost->getCost(uiMvBits))) + (Double)m_pcRdCost->getCost(ruiBits));
#endif

}
#else
Void TEncSearch::xMotionEstimation(TComDataCU* pcCU, TComYuv* pcYuvOrg, Int iPartIdx, RefPic eRefPic, TComMv* pcMvPred, TComMv& rcMv, UInt& ruiBits, UInt& ruiCost, Bool bBi)
{
  UInt          uiPartAddr = pcCU->getZorderIdxInCU();

  Int           iRoiWidth;
  Int           iRoiHeight;

  TComMv        cMvHalf, cMvQter;
  TComMv        cMvSrchRngLT;
  TComMv        cMvSrchRngRB;

  TComYuv*      pcYuv = pcYuvOrg;
  m_iSearchRange = m_aiAdaptSR[eRefPic];

  Int           iSrchRng = (bBi ? 8 : m_iSearchRange);
  TComPattern*  pcPatternKey = pcCU->getPattern();

  Double        fWeight = 1.0;

  UInt uiPuPartAddr;
  pcCU->getPartIndexAndSize(iPartIdx, uiPuPartAddr, iRoiWidth, iRoiHeight);
  uiPartAddr += uiPuPartAddr;

  if (bBi)
  {
    TComYuv*  pcYuvOther = &m_acYuvPred[(RefPic)(1 - (Int)eRefPic)];
    pcYuv = &m_cYuvPredTemp;

    pcYuvOrg->copyPartToPartYuv(pcYuv, uiPartAddr, uiPartAddr, iRoiWidth, iRoiHeight);

    pcYuv->removeHighFreq(pcYuvOther, uiPartAddr, iRoiWidth, iRoiHeight);

    fWeight = 0.5;
  }

  //  Search key pattern initialization
  pcPatternKey->initPattern(pcYuv->getLumaAddr(uiPartAddr),
    pcYuv->getCbAddr(uiPartAddr),
    pcYuv->getCrAddr(uiPartAddr),
    iRoiWidth, iRoiHeight,
    pcYuv->getStride(),
    0, 0, 0, 0);

  Pel*        piRefY = pcCU->getPicture()->getRefPic(eRefPic)->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), uiPartAddr);
  Int         iRefStride = pcCU->getPicture()->getRefPic(eRefPic)->getPicYuvRec()->getStride();

  TComMv      cMvPred = *pcMvPred;

  if (bBi)  xSetSearchRange(pcCU, rcMv, iSrchRng, cMvSrchRngLT, cMvSrchRngRB);
  else        xSetSearchRange(pcCU, cMvPred, iSrchRng, cMvSrchRngLT, cMvSrchRngRB);

  m_pcRdCost->getMotionCost(1, 0);

  m_pcRdCost->setPredictor(*pcMvPred);
  m_pcRdCost->setCostScale(2);

  //  Do integer search
  if (!m_iFastSearch || bBi)
  {
    xPatternSearch(pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, ruiCost);
  }
  else
  {
    rcMv = *pcMvPred;
    xPatternSearchFast(pcCU, pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, ruiCost);
  }

  m_pcRdCost->getMotionCost(1, 0);
  m_pcRdCost->setCostScale(1);

  xPatternSearchFracDIF(pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost);

  m_pcRdCost->setCostScale(0);
  rcMv <<= 2;
  rcMv += (cMvHalf <<= 1);
  rcMv += cMvQter;

  UInt uiMvBits = m_pcRdCost->getBits(rcMv.getHor(), rcMv.getVer());

  ruiBits += uiMvBits;

  ruiCost = (UInt)(floor(fWeight * ((Double)ruiCost - (Double)m_pcRdCost->getCost(uiMvBits))) + (Double)m_pcRdCost->getCost(ruiBits));

}
#endif

#if wlq_Mv_check
Char TEncSearch::check_mv_range(UInt uiBitSize, Int mv_x, Int mv_y, Int pix_x, Int pix_y, Int blocktype, Int dmh_x, Int dmh_y)
{
	Int curr_max_x, curr_min_x, curr_max_y, curr_min_y;

	Int ddx, ddy;
	Int pos_xx, pos_yy;
	Int mv_xx, mv_yy;

	Int bx[9] = { MIN_BLOCK_SIZE, MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE, MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE / 2, MIN_BLOCK_SIZE * 3 / 2, MIN_BLOCK_SIZE };
	Int by[9] = { MIN_BLOCK_SIZE, MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE, MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE / 2, MIN_BLOCK_SIZE * 3 / 2, MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE };

	Int is_x_subpel = 0;
	Int is_y_subpel = 0;
	Int x_size = bx[blocktype];
	Int y_size = by[blocktype];

	ddx = (pix_x * 4 + mv_x + dmh_x) & 3;
	ddy = (pix_y * 4 + mv_y + dmh_y) & 3;
	pos_xx = (pix_x * 4 + mv_x + dmh_x - ddx) / 4;
	pos_yy = (pix_y * 4 + mv_y + dmh_y - ddy) / 4;
	mv_xx = pos_xx * 4 - pix_x * 4;
	mv_yy = pos_yy * 4 - pix_y * 4;
	if (ddx != 0)
		is_x_subpel = 1;

	if (ddy != 0)
		is_y_subpel = 1;

	if ((pix_x % (1 << uiBitSize)) != 0 && blocktype == 6)
		x_size = bx[7];
	if ((pix_y % (1 << uiBitSize)) != 0 && blocktype == 4)
		y_size = bx[5];

	curr_max_x = (m_pcEncCfg->getSourceWidth() - (pix_x + x_size * (1 << (uiBitSize - MIN_CU_SIZE_IN_BIT)) + is_x_subpel * 4)) * 4
		+ /*(16<<(uiBitSize-4))*/64 * 4;

	curr_min_x = pix_x * 4 + /*(16<<(uiBitSize-4))*/64 * 4 - is_x_subpel * 3 * 4;

	curr_max_y = (m_pcEncCfg->getSourceHeight() - (pix_y + y_size * (1 << (uiBitSize - MIN_CU_SIZE_IN_BIT)) + is_y_subpel * 4)) * 4
		+ /*(16<<(uiBitSize-4))*/64 * 4;

	curr_min_y = pix_y * 4 + /*(16<<(uiBitSize-4))*/64 * 4 - is_y_subpel * 3 * 4;

	if (mv_xx > curr_max_x || mv_xx < -curr_min_x || mv_xx > Max_H_MV || mv_xx < Min_H_MV) {
		return 0;
	}

	if (mv_yy > curr_max_y || mv_yy < -curr_min_y || mv_yy > Max_V_MV || mv_yy < Min_V_MV) {
		return 0;
	}

	return 1;
}

Char TEncSearch::check_mvd(Int mvd_x, Int mvd_y)
{
	if (mvd_x > 4095 || mvd_x < -4096 || mvd_y > 4095 || mvd_y < -4096) {
		return 0;
	}

	return 1;
}
#endif

Void TEncSearch::xSetSearchRange ( TComDataCU* pcCU, TComMv& cMvPred, Int iSrchRng, TComMv& rcMvSrchRngLT, TComMv& rcMvSrchRngRB )
{
  Int  iMvShift = 2;

  pcCU->clipMv( cMvPred );

  rcMvSrchRngLT.setHor( cMvPred.getHor() - (iSrchRng << iMvShift) );
  rcMvSrchRngLT.setVer( cMvPred.getVer() - (iSrchRng << iMvShift) );
  
  rcMvSrchRngRB.setHor( cMvPred.getHor() + (iSrchRng << iMvShift) );
  rcMvSrchRngRB.setVer( cMvPred.getVer() + (iSrchRng << iMvShift) );
  
  pcCU->clipMv        ( rcMvSrchRngLT );
  pcCU->clipMv        ( rcMvSrchRngRB );
  
  rcMvSrchRngLT >>= iMvShift;
  rcMvSrchRngRB >>= iMvShift;
}

#if INTER612
Void TEncSearch::xPatternSearch(TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB, TComMv& rcMv, TComMv cMvPred, UInt& ruiSAD)
#else
Void TEncSearch::xPatternSearch(TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB, TComMv& rcMv, UInt& ruiSAD)
#endif
{
  Int   iSrchRngHorLeft   = pcMvSrchRngLT->getHor();
  Int   iSrchRngHorRight  = pcMvSrchRngRB->getHor();
  Int   iSrchRngVerTop    = pcMvSrchRngLT->getVer();
  Int   iSrchRngVerBottom = pcMvSrchRngRB->getVer();
  
  UInt  uiSad;
  UInt  uiSadBest         = MAX_UINT;
  Int   iBestX = 0;
  Int   iBestY = 0;
  
  Pel*  piRefSrch;
#if INTER612

  Int temp_X = 0;
  Int temp_Y = 0;
  Int temp_iBestX = 0;
  Int temp_iBestY = 0;

  Int  radius1 = 0;
  Int  radius2 = 0;
  UInt  distance1 = 0;
  UInt  distance2 = 0;
#endif
  //-- jclee for using the SAD function pointer
  m_pcRdCost->setDistParam( pcPatternKey, piRefY, iRefStride,  m_cDistParam );
  
  // fast encoder decision: use subsampled SAD for integer ME
  if ( m_pcEncCfg->getUseFastEnc() )
  {
    if ( m_cDistParam.iRows > 8 )
    {
      m_cDistParam.iSubShift = 1;
    }
  }
  
  piRefY += (iSrchRngVerTop * iRefStride);
  for ( Int y = iSrchRngVerTop; y <= iSrchRngVerBottom; y++ )
  {
    for ( Int x = iSrchRngHorLeft; x <= iSrchRngHorRight; x++ )
    {
      //  find min. distortion position
      piRefSrch = piRefY + x;
      m_cDistParam.pCur = piRefSrch;
#if FULLSEARCHFAST
      uiSad = m_pcRdCost->getCost( x, y );
      m_cDistParam.m_uiMvCost = uiSad;
      m_cDistParam.m_uiSadBest =uiSadBest;
      uiSad += m_cDistParam.DistFunc( &m_cDistParam );
#else
      uiSad = m_cDistParam.DistFunc(&m_cDistParam);

      // motion cost
      uiSad += m_pcRdCost->getCost(x, y);
#endif
   
      
      if ( uiSad < uiSadBest )
      {
        uiSadBest = uiSad;
        iBestX    = x;
        iBestY    = y;
      }
#if INTER612
	  if (uiSad == uiSadBest) //RD的搜索方式不同，导致了先后关系可能出来在uisad相同的情况下，MV_X,MV_Y不同  spiral search
	  {

		  radius1 = abs(x - cMvPred.getHor() / 4) > abs(y - cMvPred.getVer() / 4) ? abs(x - cMvPred.getHor() / 4) : abs(y - cMvPred.getVer() / 4);
		  radius2 = abs(iBestX - cMvPred.getHor() / 4) > abs(iBestY - cMvPred.getVer() / 4) ? abs(iBestX - cMvPred.getHor() / 4) : abs(iBestY - cMvPred.getVer() / 4);

		  temp_X = x - cMvPred.getHor() / 4;
		  temp_Y = y - cMvPred.getVer() / 4;
		  temp_iBestX = iBestX - cMvPred.getHor() / 4;
		  temp_iBestY = iBestY - cMvPred.getVer() / 4;

		  if (radius1 <radius2)
		  {
			  uiSadBest = uiSad;
			  iBestX = x;
			  iBestY = y;
		  }
		  else if (radius1 == radius2)
		  {
#if INTER613
			  if (temp_X <temp_iBestX && temp_X != -radius1)
			  {
				  uiSadBest = uiSad;
				  iBestX = x;
				  iBestY = y;
			  }
			  if (temp_iBestX == -radius1)
			  {
				  if (abs(temp_X)< radius1)
				  {
					  uiSadBest = uiSad;
					  iBestX = x;
					  iBestY = y;
				  }
			  }
#else
			  if (abs(temp_X - radius1) + abs(temp_Y - (-radius1)) <= 2 * radius1)
			  {
				  distance1 = abs(temp_X - (-radius1)) + abs(temp_Y - (-radius1));
			  }
			  else
			  {
				  distance1 = abs(temp_X - radius1) + abs(temp_Y - radius1) + 2 * radius1;
			  }
			  if (abs(temp_iBestX - radius1) + abs(temp_iBestY - (-radius1)) <= 2 * radius1)
			  {
				  distance2 = abs(temp_iBestX - (-radius1)) + abs(temp_iBestY - (-radius1));
			  }
			  else
			  {
				  distance2 = abs(temp_iBestX - radius1) + abs(temp_iBestY - radius1) + 2 * radius1;
			  }
			  if (distance1 < distance2)
			  {
				  uiSadBest = uiSad;
				  iBestX = x;
				  iBestY = y;
			  }
#endif
			  }
		  }
#endif
    }
    piRefY += iRefStride;
  }
  
  rcMv.set( iBestX, iBestY );
#if WLQ_CLOSE_REF_COST_FOR_DEBUG
  ruiSAD = uiSadBest;
#else
  ruiSAD = uiSadBest - m_pcRdCost->getCost( iBestX, iBestY );
#endif
#if DEBUG_ME
	//printf("mv_x =%d\t mv_y= %d\t", rcMv.getHor(), rcMv.getVer());
	printf("cost = %d\t", uiSadBest);
#endif
 // printf("%d\n",uiSadBest);
  return;
}

#if wlq_FME
#define EARLY_TERMINATION  if(iRefIdxPred>0)								\
  {																			\
	  if ((m_BestSad-m_pred_SAD_ref)<m_pred_SAD_ref*betaThird)				\
			goto third_step;												\
	  else if((m_BestSad-m_pred_SAD_ref)<m_pred_SAD_ref*betaSec)			\
			goto sec_step;													\
  }																			\
  else if(blocktype>1)														\
  {																			\
	if ((m_BestSad-m_pred_SAD_uplayer)<m_pred_SAD_uplayer*betaThird)		\
    {																		\
			goto third_step;												\
    }																		\
	else if((m_BestSad-m_pred_SAD_uplayer)<m_pred_SAD_uplayer*betaSec)		\
			goto sec_step;													\
  }																			\
  else																		\
  {																			\
    if ((m_BestSad-m_pred_SAD_space)<m_pred_SAD_space*betaThird)			\
    {																		\
			goto third_step;												\
    }																		\
	else if((m_BestSad-m_pred_SAD_space)<m_pred_SAD_space*betaSec)			\
			goto sec_step;													\
  }
#endif

#if wlq_FME

Void TEncSearch::xPreSearchFast(TComDataCU* pcCU, Int iRefIdxPred)
{
#if !wlq_FME_q
	UInt blocktype = pcCU->getPartitionSize(0);
	Int  refframe = (iRefIdxPred == -1 ? 0 : iRefIdxPred);
	PictureType ImgType = pcCU->getPic()->getPicSym()->getPicHeader()->getPictureType();
#if wlq_FME_q
	Int flag_intra_SAD = 0;
#endif
#if wlq_FME_q
	if (blocktype == SIZE_NxN)
	{
		m_pred_MV_uplayer[0] = allFwMv[b8_y][b8_x][refframe][2][0];
		m_pred_MV_uplayer[1] = allFwMv[b8_y][b8_x][refframe][2][1];
		m_pred_SAD_uplayer = (iRefIdxPred == -1) ? (he->all_bwmincost[(SubMBpix_y >> MIN_BLOCK_SIZE_IN_BIT) + b8_pix_y][(SubMBpix_x >>
			MIN_BLOCK_SIZE_IN_BIT) + b8_pix_x][0][2][0]) : (he->all_mincost[(SubMBpix_y >> MIN_BLOCK_SIZE_IN_BIT) +
			b8_pix_y][(SubMBpix_x >> MIN_BLOCK_SIZE_IN_BIT) + b8_pix_x][refframe][2][0]);
		m_pred_SAD_uplayer /= 2;
	}
	else if (blocktype == SIZE_2NxnU || blocktype == SIZE_2NxnD) {
		m_pred_MV_uplayer[0] = allFwMv[b8_y][b8_x][refframe][2][0];
		m_pred_MV_uplayer[1] = allFwMv[b8_y][b8_x][refframe][2][1];
		m_pred_SAD_uplayer = (iRefIdxPred == -1) ? (he->all_bwmincost[(SubMBpix_y >> MIN_BLOCK_SIZE_IN_BIT) + b8_pix_y][(SubMBpix_x >>
			MIN_BLOCK_SIZE_IN_BIT) + b8_pix_x][0][2][0]) : (he->all_mincost[(SubMBpix_y >> MIN_BLOCK_SIZE_IN_BIT) +
			b8_pix_y][(SubMBpix_x >> MIN_BLOCK_SIZE_IN_BIT) + b8_pix_x][refframe][2][0]);
		m_pred_SAD_uplayer /= 2;

	}
	else if (blocktype == SIZE_nLx2N || blocktype == SIZE_nRx2N) {
		m_pred_MV_uplayer[0] = allFwMv[b8_y][b8_x][refframe][3][0];
		m_pred_MV_uplayer[1] = allFwMv[b8_y][b8_x][refframe][3][1];
		m_pred_SAD_uplayer = (iRefIdxPred == -1) ? (he->all_bwmincost[(SubMBpix_y >> MIN_BLOCK_SIZE_IN_BIT) + b8_pix_y][(SubMBpix_x >>
			MIN_BLOCK_SIZE_IN_BIT) + b8_pix_x][0][3][0]) : (he->all_mincost[(SubMBpix_y >> MIN_BLOCK_SIZE_IN_BIT) +
			b8_pix_y][(SubMBpix_x >> MIN_BLOCK_SIZE_IN_BIT) + b8_pix_x][refframe][3][0]);
		m_pred_SAD_uplayer /= 2;
	}
	else if (blocktype > SIZE_2Nx2N) {
		m_pred_MV_uplayer[0] = allFwMv[b8_y][b8_x][refframe][1][0];
		m_pred_MV_uplayer[1] = allFwMv[b8_y][b8_x][refframe][1][1];
		m_pred_SAD_uplayer = (iRefIdxPred == -1) ? (he->all_bwmincost[(SubMBpix_y >> MIN_BLOCK_SIZE_IN_BIT) + b8_pix_y][(SubMBpix_x >>
			MIN_BLOCK_SIZE_IN_BIT) + b8_pix_x][0][1][0]) : (he->all_mincost[(SubMBpix_y >> MIN_BLOCK_SIZE_IN_BIT) +
			b8_pix_y][(SubMBpix_x >> MIN_BLOCK_SIZE_IN_BIT) + b8_pix_x][refframe][1][0]);
		m_pred_SAD_uplayer /= 2;
	}
#endif

	m_pred_SAD_uplayer = flag_intra_SAD ? 0 : m_pred_SAD_uplayer;// for irregular motion
	//coordinate prediction
	if (m_PicCodedNum > refframe + 1) {
		m_pred_MV_time[0] = he->all_mincost[(SubMBpix_y >> MIN_BLOCK_SIZE_IN_BIT) + b8_pix_y][(SubMBpix_x >>
			MIN_BLOCK_SIZE_IN_BIT) + b8_pix_x][0][0][3];
		m_pred_MV_time[1] = he->all_mincost[(SubMBpix_y >> MIN_BLOCK_SIZE_IN_BIT) + b8_pix_y][(SubMBpix_x >>
			MIN_BLOCK_SIZE_IN_BIT) + b8_pix_x][0][0][4];
	}

#if wlq_FME_q
	if (iRefIdxPred == -1 && hc->Bframe_ctr > 1) {
		m_pred_MV_time[0] = (int)(he->all_bwmincost[(SubMBpix_y >> MIN_BLOCK_SIZE_IN_BIT) + b8_pix_y][(SubMBpix_x >>
			MIN_BLOCK_SIZE_IN_BIT) + b8_pix_x][0][0][3] * ((n_Bframe == 1) ? (N_Bframe) : (N_Bframe - n_Bframe + 1.0) /
			(N_Bframe - n_Bframe + 2.0)));     //should add a factor
		m_pred_MV_time[1] = (int)(he->all_bwmincost[(SubMBpix_y >> MIN_BLOCK_SIZE_IN_BIT) + b8_pix_y][(SubMBpix_x >>
			MIN_BLOCK_SIZE_IN_BIT) + b8_pix_x][0][0][4] * ((n_Bframe == 1) ? (N_Bframe) : (N_Bframe - n_Bframe + 1.0) /
			(N_Bframe - n_Bframe + 2.0)));     //should add a factor
	}
#endif
	{
		if (refframe > 0) {
			m_pred_SAD_ref = he->all_mincost[(SubMBpix_y >> MIN_BLOCK_SIZE_IN_BIT) + b8_pix_y][(SubMBpix_x >> MIN_BLOCK_SIZE_IN_BIT) +
				b8_pix_x][(refframe - 1)][blocktype][0];
			m_pred_SAD_ref = flag_intra_SAD ? 0 : m_pred_SAD_ref;//add this for irregular motion
			m_pred_MV_ref[0] = he->all_mincost[(SubMBpix_y >> MIN_BLOCK_SIZE_IN_BIT) + b8_pix_y][(SubMBpix_x >> MIN_BLOCK_SIZE_IN_BIT)
				+ b8_pix_x][(refframe - 1)][blocktype][1];
			m_pred_MV_ref[0] = (int)(m_pred_MV_ref[0] * (refframe + 1) / (double)(refframe));
			m_pred_MV_ref[1] = he->all_mincost[(SubMBpix_y >> MIN_BLOCK_SIZE_IN_BIT) + b8_pix_y][(SubMBpix_x >> MIN_BLOCK_SIZE_IN_BIT)
				+ b8_pix_x][(refframe - 1)][blocktype][2];
			m_pred_MV_ref[1] = (int)(m_pred_MV_ref[1] * (refframe + 1) / (double)(refframe));
		}

#if wlq_FME_q
		if (ImgType == B_PICTURE && iRefIdxPred == 0) {
			m_pred_SAD_ref = he->all_bwmincost[(SubMBpix_y >> MIN_BLOCK_SIZE_IN_BIT) + b8_pix_y][(SubMBpix_x >> MIN_BLOCK_SIZE_IN_BIT)
				+ b8_pix_x][0][blocktype][0];
			m_pred_SAD_ref = flag_intra_SAD ? 0 : m_pred_SAD_ref;//add this for irregular motion
			m_pred_MV_ref[0] = (int)(he->all_bwmincost[(SubMBpix_y >> MIN_BLOCK_SIZE_IN_BIT) + b8_pix_y][(SubMBpix_x >>
				MIN_BLOCK_SIZE_IN_BIT) + b8_pix_x][0][blocktype][1] * (-n_Bframe) / (N_Bframe - n_Bframe +
				1.0f));   //should add a factor
			m_pred_MV_ref[1] = (int)(he->all_bwmincost[(SubMBpix_y >> MIN_BLOCK_SIZE_IN_BIT) + b8_pix_y][(SubMBpix_x >>
				MIN_BLOCK_SIZE_IN_BIT) + b8_pix_x][0][blocktype][2] * (-n_Bframe) / (N_Bframe - n_Bframe + 1.0f));
		}
#endif
	}
#endif
}

Void TEncSearch::xSearchOnePixel(Int cand_x, Int center_x, Int cand_y, Int center_y, Pel* piRefY, Int iRefStride, Int pred_x, Int pred_y, UInt   mvshift, Int pic_pix_x, Int pic_pix_y, UInt BestSad, Bool get_ref_line, Int height, Int width)
{
	Int search_range = m_pcEncCfg->getSearchRange();
	if (abs(cand_x - center_x) <= search_range && abs(cand_y - center_y) <= search_range)
	{
		if (!m_McostState[cand_y - center_y + search_range][cand_x - center_x + search_range])
		{
			Int mcost = m_pcRdCost->getCostFME((cand_x << mvshift) - pred_x, (cand_y << mvshift) - pred_y);
			m_cDistParam.pCur = piRefY + (cand_y - pic_pix_y)*iRefStride + (cand_x - pic_pix_x);
			m_cDistParam.BestSad = BestSad - mcost;
			m_cDistParam.key = true; m_cDistParam.get_ref_line = get_ref_line;
			if (get_ref_line)
			{
				m_cDistParam.pCur = piRefY + (0 - pic_pix_y)*iRefStride + (0 - pic_pix_x);
				m_cDistParam.cand_x = cand_x; m_cDistParam.cand_y = cand_y;
				m_cDistParam.width = width; m_cDistParam.height = height;
				m_cDistParam.RefStride = iRefStride;
			}
			mcost += m_cDistParam.DistFunc(&m_cDistParam);
			m_cDistParam.key = false; m_cDistParam.get_ref_line = false;
			m_McostState[cand_y - center_y + search_range][cand_x - center_x + search_range] = mcost;
			if (mcost < m_BestSad)
			{
				m_BestX = cand_x;
				m_BestY = cand_y;
				m_BestSad = mcost;
			}
		}
	}
}

Void TEncSearch::xSearchOnePixel2(Int value_iAbort, Int& iAbort, Int cand_x, Int center_x, Int cand_y, Int center_y, Pel* piRefY, Int iRefStride, Int pred_x, Int pred_y, UInt mvshift, Int pic_pix_x, Int pic_pix_y, UInt BestSad, Bool get_ref_line, Int height, Int width)
{
	Int search_range = m_pcEncCfg->getSearchRange();
	if (abs(cand_x - center_x) <= search_range && abs(cand_y - center_y) <= search_range)
	{
		if (!m_McostState[cand_y - center_y + search_range][cand_x - center_x + search_range])
		{
			Int mcost = m_pcRdCost->getCostFME((cand_x << mvshift) - pred_x, (cand_y << mvshift) - pred_y);
			m_cDistParam.pCur = piRefY + (cand_y - pic_pix_y)*iRefStride + (cand_x - pic_pix_x);
			m_cDistParam.BestSad = BestSad - mcost;
			m_cDistParam.key = true; m_cDistParam.get_ref_line = get_ref_line;
			if (get_ref_line)
			{
				m_cDistParam.pCur = piRefY + (0 - pic_pix_y)*iRefStride + (0 - pic_pix_x);
				m_cDistParam.cand_x = cand_x; m_cDistParam.cand_y = cand_y;
				m_cDistParam.width = width; m_cDistParam.height = height;
				m_cDistParam.RefStride = iRefStride;
			}
			mcost += m_cDistParam.DistFunc(&m_cDistParam);
			m_cDistParam.key = false; m_cDistParam.get_ref_line = false;
			m_McostState[cand_y - center_y + search_range][cand_x - center_x + search_range] = mcost;
			if (mcost < m_BestSad)
			{
				m_BestX = cand_x;
				m_BestY = cand_y;
				m_BestSad = mcost;
				iAbort = value_iAbort;
			}
		}
	}
}
#endif

#if wlq_FME
Void TEncSearch::xPatternSearchFast(TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB, TComMv& rcMv, TComMv cMvPred, UInt& ruiSAD, Int iRefIdxPred, Int iPartIdx)
#else
Void TEncSearch::xPatternSearchFast( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB, TComMv& rcMv, UInt& ruiSAD )
#endif
{
#if wlq_FME
	static Int Diamond_x[4] = { -1, 0, 1, 0 };
	static Int Diamond_y[4] = { 0, 1, 0, -1 };
	static Int Hexagon_x[6] = { 2, 1, -1, -2, -1, 1 };
	static Int Hexagon_y[6] = { 0, -2, -2, 0, 2, 2 };
	static Int Big_Hexagon_x[16] = { 0, -2, -4, -4, -4, -4, -4, -2, 0, 2, 4, 4, 4, 4, 4, 2 };
	static Int Big_Hexagon_y[16] = { 4, 3, 2, 1, 0, -1, -2, -3, -4, -3, -2, -1, 0, 1, 2, 3 };

	Int mcost;

	//-- jclee for using the SAD function pointer
	m_pcRdCost->setDistParam(pcPatternKey, piRefY, iRefStride, m_cDistParam);
	Int search_range = m_pcEncCfg->getSearchRange();
	initFME();

	Int pic_pix_x, pic_pix_y;
	pcCU->getInterPUxy(iPartIdx, pic_pix_x, pic_pix_y);

	UInt  mvshift = 2;                  // motion vector shift for getting sub-pel units
	Int   pred_x = (pic_pix_x << mvshift) + cMvPred.getHor();       // predicted position x (in sub-pel units)
	Int   pred_y = (pic_pix_y << mvshift) + cMvPred.getVer();       // predicted position y (in sub-pel units)
	Int   center_x = pic_pix_x + cMvPred.getHor() / 4;       // center position x (in pel units)
	Int   center_y = pic_pix_y + cMvPred.getVer() / 4;       // center position y (in pel units)

	Int   blocktype = pcCU->getPartitionSize(0) + 1;
	Int   bit_size = pcCU->getLog2CUSize(0);

	Int   blocksize_y = g_blk_size_AVS2[blocktype * 2 + iPartIdx][1] << (bit_size - MIN_CU_SIZE_IN_BIT);
	Int   blocksize_x = g_blk_size_AVS2[blocktype * 2 + iPartIdx][0] << (bit_size - MIN_CU_SIZE_IN_BIT);
	Int   blocksize_x4 = blocksize_x >> 2;                         // horizontal block size in 4-pel units

	Int   height = m_pcEncCfg->getSourceHeight();
	Int   width = m_pcEncCfg->getSourceWidth();
	Bool  get_ref_line;
	Double betaSec, betaThird;
	PictureType ImgType = pcCU->getPic()->getPicSym()->getPicHeader()->getPictureType();

	xPreSearchFast(pcCU, iRefIdxPred);

	int   pos, cand_x, cand_y;

	int   search_step, iYMinNow, iXMinNow;
	int   i, m;
	int   iAbort;

	int   best_pos = 0;                                        // position with minimum motion cost
	int   max_pos = (2 * search_range + 1) * (2 * search_range + 1); // number of search positions
	int   best_pos_x, best_pos_y;

	//===== set function for getting reference picture lines =====
	if ((center_x > search_range) && (center_x < width - 1 - search_range - blocksize_x) &&
		(center_y > search_range) && (center_y < height - 1 - search_range - blocksize_y)) {
		get_ref_line = false; //FastLineX;
	}
	else {
		get_ref_line = true; //UMVLineX;
	}

	///////////////////////////////////////////////////////////////
#if wlq_FME_q
	betaSec = 0;
	betaThird = 0;
#else
	if (iRefIdxPred > 0) {
		if (m_pred_SAD_ref != 0) {
			betaSec = m_Bsize[blocktype] / (m_pred_SAD_ref * m_pred_SAD_ref) - m_AlphaSec[blocktype];
			betaThird = m_Bsize[blocktype] / (m_pred_SAD_ref * m_pred_SAD_ref) - m_AlphaThird[blocktype];
		}
		else {
			betaSec = 0;
			betaThird = 0;
		}
	}
	else {
		if (blocktype == 1) {
			if (m_pred_SAD_space != 0) {
				betaSec = m_Bsize[blocktype] / (m_pred_SAD_space * m_pred_SAD_space) - m_AlphaSec[blocktype];
				betaThird = m_Bsize[blocktype] / (m_pred_SAD_space * m_pred_SAD_space) - m_AlphaThird[blocktype];
			}
			else {
				betaSec = 0;
				betaThird = 0;
			}
		}
		else {
			if (m_pred_SAD_uplayer != 0) {
				betaSec = m_Bsize[blocktype] / (m_pred_SAD_uplayer * m_pred_SAD_uplayer) - m_AlphaSec[blocktype];
				betaThird = m_Bsize[blocktype] / (m_pred_SAD_uplayer * m_pred_SAD_uplayer) - m_AlphaThird[blocktype];
			}
			else {
				betaSec = 0;
				betaThird = 0;
			}
		}
}
#endif
	/*****************************/

	////////////search around the predictor and (0,0)
	//check the center median predictor
	cand_x = center_x;
	cand_y = center_y;
	mcost = m_pcRdCost->getCostFME((cand_x << mvshift) - pred_x, (cand_y << 2) - pred_y);
	m_cDistParam.pCur = piRefY + (cand_y - pic_pix_y)*iRefStride + (cand_x - pic_pix_x);
	m_cDistParam.BestSad = m_BestSad;
	m_cDistParam.key = true; m_cDistParam.get_ref_line = get_ref_line;
	if (get_ref_line)
	{
		m_cDistParam.pCur = piRefY + (0 - pic_pix_y)*iRefStride + (0 - pic_pix_x);
		m_cDistParam.cand_x = cand_x; m_cDistParam.cand_y = cand_y;
		m_cDistParam.width = width; m_cDistParam.height = height;
		m_cDistParam.RefStride = iRefStride;
	}
	mcost += m_cDistParam.DistFunc(&m_cDistParam);
	m_cDistParam.key = false; m_cDistParam.get_ref_line = false;
	m_McostState[search_range][search_range] = mcost;

	if (mcost < m_BestSad) {
		m_BestSad = mcost;
		m_BestX = cand_x;
		m_BestY = cand_y;
	}
#if wlq_FME_q
	if (!((ImgType == P_PICTURE || ImgType == BP_IMG_PICTURE) && m_pcEncCfg->getBackgroundEnable())) { //no motion search for S frame
#endif
		iXMinNow = m_BestX;
		iYMinNow = m_BestY;
		best_pos_x = m_BestX;
		best_pos_y = m_BestY;

		for (m = 0; m < 4; m++) {
			cand_x = iXMinNow + Diamond_x[m];
			cand_y = iYMinNow + Diamond_y[m];
			xSearchOnePixel(cand_x, center_x, cand_y, center_y, piRefY, iRefStride, pred_x, pred_y, mvshift, pic_pix_x, pic_pix_y, m_BestSad, get_ref_line, height, width);
		}

		if (center_x != pic_pix_x || center_y != pic_pix_y) {
			cand_x = pic_pix_x;
			cand_y = pic_pix_y;
			xSearchOnePixel(cand_x, center_x, cand_y, center_y, piRefY, iRefStride, pred_x, pred_y, mvshift, pic_pix_x, pic_pix_y, m_BestSad, get_ref_line, height, width);


			if (m_BestX != best_pos_x || m_BestY != best_pos_y) {
				iXMinNow = m_BestX;
				iYMinNow = m_BestY;
				best_pos_x = m_BestX;
				best_pos_y = m_BestY;
				for (m = 0; m < 4; m++) {
					cand_x = iXMinNow + Diamond_x[m];
					cand_y = iYMinNow + Diamond_y[m];
					xSearchOnePixel(cand_x, center_x, cand_y, center_y, piRefY, iRefStride, pred_x, pred_y, mvshift, pic_pix_x, pic_pix_y, m_BestSad, get_ref_line, height, width);
				}
			}
		}

#if !wlq_FME_q
		if (blocktype > 1) {
			cand_x = pic_pix_x + (m_pred_MV_uplayer[0] / 4);
			cand_y = pic_pix_y + (m_pred_MV_uplayer[1] / 4);
			xSearchOnePixel(cand_x, center_x, cand_y, center_y, piRefY, iRefStride, pred_x, pred_y, mvshift, pic_pix_x, pic_pix_y, m_BestSad, get_ref_line, height, width);

			if ((m_BestSad - m_pred_SAD_uplayer) < m_pred_SAD_uplayer * betaThird) {
				goto third_step;
			}
			else if ((m_BestSad - m_pred_SAD_uplayer) < m_pred_SAD_uplayer * betaSec) {
				goto sec_step;
			}
				}
#endif
		//coordinate position prediction
#if ！wlq_FME_q
#if wlq_FME_q
		if (m_PicCodedNum > 1 + iRefIdxPred && iRefIdxPred != -1){
#else
		if ((m_PicCodedNum > 1 + iRefIdxPred && iRefIdxPred != -1) || (iRefIdxPred == -1 && hc->Bframe_ctr > 1)) {
#endif

			cand_x = pic_pix_x + m_pred_MV_time[0] / 4;
			cand_y = pic_pix_y + m_pred_MV_time[1] / 4;
			xSearchOnePixel(cand_x, center_x, cand_y, center_y, piRefY, iRefStride, pred_x, pred_y, mvshift, pic_pix_x, pic_pix_y, m_BestSad, get_ref_line, height, width);
		}

		////prediciton using mV of last ref moiton vector
		if ((iRefIdxPred > 0) || (ImgType == B_PICTURE && iRefIdxPred == 0)) {
			cand_x = pic_pix_x + m_pred_MV_ref[0] / 4;
			cand_y = pic_pix_y + m_pred_MV_ref[1] / 4;
			xSearchOnePixel(cand_x, center_x, cand_y, center_y, piRefY, iRefStride, pred_x, pred_y, mvshift, pic_pix_x, pic_pix_y, m_BestSad, get_ref_line, height, width);
		}
#endif

		if (m_BestX != best_pos_x || m_BestY != best_pos_y) {
			iXMinNow = m_BestX;
			iYMinNow = m_BestY;
			best_pos_x = m_BestX;
			best_pos_y = m_BestY;
			for (m = 0; m < 4; m++) {
				cand_x = iXMinNow + Diamond_x[m];
				cand_y = iYMinNow + Diamond_y[m];
				xSearchOnePixel(cand_x, center_x, cand_y, center_y, piRefY, iRefStride, pred_x, pred_y, mvshift, pic_pix_x, pic_pix_y, m_BestSad, get_ref_line, height, width);
			}
		}
#if !wlq_FME_q
		//early termination algrithm, refer to JVT-D016
		EARLY_TERMINATION
#endif
			if (blocktype > 7) { //PNXN
				goto sec_step;
			}
			else {
				goto first_step;
			}

		first_step:

			//Unsymmetrical-cross search
			iXMinNow = m_BestX;
			iYMinNow = m_BestY;

			for (i = 1; i <= search_range / 2; i++) {
				search_step = 2 * i - 1;
				cand_x = iXMinNow + search_step;
				cand_y = iYMinNow;
				xSearchOnePixel(cand_x, center_x, cand_y, center_y, piRefY, iRefStride, pred_x, pred_y, mvshift, pic_pix_x, pic_pix_y, m_BestSad, get_ref_line, height, width);
				cand_x = iXMinNow - search_step;
				cand_y = iYMinNow;
				xSearchOnePixel(cand_x, center_x, cand_y, center_y, piRefY, iRefStride, pred_x, pred_y, mvshift, pic_pix_x, pic_pix_y, m_BestSad, get_ref_line, height, width);
		}

			for (i = 1; i <= search_range / 4; i++) {
				search_step = 2 * i - 1;
				cand_x = iXMinNow;
				cand_y = iYMinNow + search_step;
				xSearchOnePixel(cand_x, center_x, cand_y, center_y, piRefY, iRefStride, pred_x, pred_y, mvshift, pic_pix_x, pic_pix_y, m_BestSad, get_ref_line, height, width);
				cand_x = iXMinNow;
				cand_y = iYMinNow - search_step;
				xSearchOnePixel(cand_x, center_x, cand_y, center_y, piRefY, iRefStride, pred_x, pred_y, mvshift, pic_pix_x, pic_pix_y, m_BestSad, get_ref_line, height, width);
			}
#if !wlq_FME_q
			//early termination algrithm, refer to JVT-D016

			EARLY_TERMINATION
#endif
				iXMinNow = m_BestX;
			iYMinNow = m_BestY;

			// Uneven Multi-Hexagon-grid Search
			for (pos = 1; pos < 25; pos++) {
				cand_x = iXMinNow + m_spiral_search_x[pos];
				cand_y = iYMinNow + m_spiral_search_y[pos];
				xSearchOnePixel(cand_x, center_x, cand_y, center_y, piRefY, iRefStride, pred_x, pred_y, mvshift, pic_pix_x, pic_pix_y, m_BestSad, get_ref_line, height, width);
		}
#if !wlq_FME_q
			//early termination algrithm, refer to JVT-D016

			EARLY_TERMINATION
#endif
				for (i = 1; i <= search_range / 4; i++) {
					iAbort = 0;

					for (m = 0; m < 16; m++) {
						cand_x = iXMinNow + Big_Hexagon_x[m] * i;
						cand_y = iYMinNow + Big_Hexagon_y[m] * i;
						//SEARCH_ONE_PIXEL1(1)
						xSearchOnePixel2(1, iAbort, cand_x, center_x, cand_y, center_y, piRefY, iRefStride, pred_x, pred_y, mvshift, pic_pix_x, pic_pix_y, m_BestSad, get_ref_line, height, width);
					}
#if !wlq_FME_q
					if (iAbort) {
						EARLY_TERMINATION
					}
#endif
				}

		sec_step:
			//Extended Hexagon-based Search
			iXMinNow = m_BestX;
			iYMinNow = m_BestY;

			for (i = 0; i < search_range; i++) {
				iAbort = 1;

				for (m = 0; m < 6; m++) {
					cand_x = iXMinNow + Hexagon_x[m];
					cand_y = iYMinNow + Hexagon_y[m];
					//SEARCH_ONE_PIXEL1(0)
					xSearchOnePixel2(0, iAbort, cand_x, center_x, cand_y, center_y, piRefY, iRefStride, pred_x, pred_y, mvshift, pic_pix_x, pic_pix_y, m_BestSad, get_ref_line, height, width);
				}

				if (iAbort) {
					break;
				}

				iXMinNow = m_BestX;
				iYMinNow = m_BestY;
			}

		third_step:
			// the third step with a small search pattern
			iXMinNow = m_BestX;
			iYMinNow = m_BestY;

			for (i = 0; i < search_range; i++) {
				iAbort = 1;

				for (m = 0; m < 4; m++) {
					cand_x = iXMinNow + Diamond_x[m];
					cand_y = iYMinNow + Diamond_y[m];
					//SEARCH_ONE_PIXEL1(0)
					xSearchOnePixel2(0, iAbort, cand_x, center_x, cand_y, center_y, piRefY, iRefStride, pred_x, pred_y, mvshift, pic_pix_x, pic_pix_y, m_BestSad, get_ref_line, height, width);
				}

				if (iAbort) {
					break;
				}

				iXMinNow = m_BestX;
				iYMinNow = m_BestY;
			}
		}


	rcMv.set(m_BestX - pic_pix_x, m_BestY - pic_pix_y);
	ruiSAD = m_BestSad;
#if wlq_FME_p
	printf("mvx = %d\t mvy = %d\t pred_mvx = %d\t pred_mvy = %d\t min_mcost: %d\t",
		m_BestX - pic_pix_x, m_BestY - pic_pix_y, cMvPred.getHor(), cMvPred.getVer(), m_BestSad);
#endif
#else
  switch ( m_iFastSearch )
  {
    case 1:
      xTZSearch( pcCU, pcPatternKey, piRefY, iRefStride, pcMvSrchRngLT, pcMvSrchRngRB, rcMv, ruiSAD );
      break;
      
    default:
      break;
  }
#endif
}

#if !wlq_FME
Void TEncSearch::xTZSearch( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB, TComMv& rcMv, UInt& ruiSAD )
{
  Int   iSrchRngHorLeft   = pcMvSrchRngLT->getHor();
  Int   iSrchRngHorRight  = pcMvSrchRngRB->getHor();
  Int   iSrchRngVerTop    = pcMvSrchRngLT->getVer();
  Int   iSrchRngVerBottom = pcMvSrchRngRB->getVer();
  
  TZ_SEARCH_CONFIGURATION
  
  UInt uiSearchRange = m_iSearchRange;
  pcCU->clipMv( rcMv );
  rcMv >>= 2;
  // init TZSearchStruct
  IntTZSearchStruct cStruct;
  cStruct.iYStride    = iRefStride;
  cStruct.piRefY      = piRefY;
  cStruct.uiBestSad   = MAX_UINT;
  
  // set rcMv (Median predictor) as start point and as best point
  xTZSearchHelp( pcPatternKey, cStruct, rcMv.getHor(), rcMv.getVer(), 0, 0 );

  // test whether zero Mv is better start point than Median predictor
  xTZSearchHelp( pcPatternKey, cStruct, 0, 0, 0, 0 );

  // start search
  Int  iDist = 0;
  Int  iStartX = cStruct.iBestX;
  Int  iStartY = cStruct.iBestY;
  
  // first search
  for ( iDist = 1; iDist <= (Int)uiSearchRange; iDist*=2 )
  {
    if ( bFirstSearchDiamond == 1 )
    {
      xTZ8PointDiamondSearch ( pcPatternKey, cStruct, pcMvSrchRngLT, pcMvSrchRngRB, iStartX, iStartY, iDist );
    }
    else
    {
      xTZ8PointSquareSearch  ( pcPatternKey, cStruct, pcMvSrchRngLT, pcMvSrchRngRB, iStartX, iStartY, iDist );
    }
    
    if ( bFirstSearchStop && ( cStruct.uiBestRound >= uiFirstSearchRounds ) ) // stop criterion
    {
      break;
    }
  }
  
  // calculate only 2 missing points instead 8 points if cStruct.uiBestDistance == 1
  if ( cStruct.uiBestDistance == 1 )
  {
    cStruct.uiBestDistance = 0;
    xTZ2PointSearch( pcPatternKey, cStruct, pcMvSrchRngLT, pcMvSrchRngRB );
  }
  
  // raster search if distance is too big
  if ( bEnableRasterSearch && ( ((Int)(cStruct.uiBestDistance) > iRaster) || bAlwaysRasterSearch ) )
  {
    cStruct.uiBestDistance = iRaster;
    for ( iStartY = iSrchRngVerTop; iStartY <= iSrchRngVerBottom; iStartY += iRaster )
    {
      for ( iStartX = iSrchRngHorLeft; iStartX <= iSrchRngHorRight; iStartX += iRaster )
      {
        xTZSearchHelp( pcPatternKey, cStruct, iStartX, iStartY, 0, iRaster );
      }
    }
  }

  // start refinement
  if ( cStruct.uiBestDistance > 0 )
  {
    while ( cStruct.uiBestDistance > 0 )
    {
      iStartX = cStruct.iBestX;
      iStartY = cStruct.iBestY;
      cStruct.uiBestDistance = 0;
      cStruct.ucPointNr = 0;
      for ( iDist = 1; iDist < (Int)uiSearchRange + 1; iDist*=2 )
      {
        if ( bStarRefinementDiamond == 1 )
        {
          xTZ8PointDiamondSearch ( pcPatternKey, cStruct, pcMvSrchRngLT, pcMvSrchRngRB, iStartX, iStartY, iDist );
        }
        else
        {
          xTZ8PointSquareSearch  ( pcPatternKey, cStruct, pcMvSrchRngLT, pcMvSrchRngRB, iStartX, iStartY, iDist );
        }
      }
      
      // calculate only 2 missing points instead 8 points if cStrukt.uiBestDistance == 1
      if ( cStruct.uiBestDistance == 1 )
      {
        cStruct.uiBestDistance = 0;
        if ( cStruct.ucPointNr != 0 )
        {
          xTZ2PointSearch( pcPatternKey, cStruct, pcMvSrchRngLT, pcMvSrchRngRB );
        }
      }
    }
  }
  
  // write out best match
  rcMv.set( cStruct.iBestX, cStruct.iBestY );

  ruiSAD = cStruct.uiBestSad - m_pcRdCost->getCost( cStruct.iBestX, cStruct.iBestY );

}
#endif

#if RD_PMVR
Int TEncSearch::pmvrAdaptMv(Int *piCandMvX, Int *piCandMvY, Int iCtrX, Int iCtrY, Int iMvX, Int iMvY, Int iPos)
{
	//jcma
	TComMv* pcMvRefine = s_acMvRefineQ;
	if ((abs(iMvX - iCtrX) > TH) || (abs(iMvY - iCtrY) > TH))
	{
		*piCandMvX = iMvX + (pcMvRefine[iPos].getHor() << 1);    // quarter-pel units
		*piCandMvY = iMvY + (pcMvRefine[iPos].getVer() << 1);    // quarter-pel units
		return ((abs(*piCandMvX - iCtrX) > TH) || (abs(*piCandMvY - iCtrY) > TH));
	}
	else
	{
		*piCandMvX = iMvX + pcMvRefine[iPos].getHor();    // quarter-pel units
		*piCandMvY = iMvY + pcMvRefine[iPos].getVer();    // quarter-pel units
		return ((abs(*piCandMvX - iCtrX) <= TH) && (abs(*piCandMvY - iCtrY) <= TH));
	}
}
#endif

#if FME_SUB
Int TEncSearch::pmvrAdaptMv2(Int *iCandMvX, Int *iCandMvY, Int iCtrX, Int iCtrY, Int iXMinNow, Int iYMinNow, Int stepX, Int stepY)
{
	if (abs(iXMinNow - iCtrX) > TH || abs(iYMinNow - iCtrY) > TH) 
	{
		*iCandMvX = iXMinNow + stepX * 2;
		*iCandMvY = iYMinNow + stepY * 2;
		return (abs(*iCandMvX - iCtrX) > TH || abs(*iCandMvY - iCtrY) > TH);
	} 
	else 
	{
		*iCandMvX = iXMinNow + stepX;
		*iCandMvY = iYMinNow + stepY;
		return (abs(*iCandMvX - iCtrX) <= TH && abs(*iCandMvY - iCtrY) <= TH);
	}
}
#endif

#if rd_sym
Int TEncSearch::scaleMv(Int val, Int disDst, Int disSrc)
{
	val = Clip3(-32768, 32767, (((long long int)(val)* disDst * (MULTI / disSrc) + HALF_MULTI) >> OFFSET));
	return val;
}

Void TEncSearch::xPatternSearchFracDIFSym(TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, Pel* piSymRef, Int iSymRefStride, Int iMvX, Int iMvY, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiSymCost)
{
	//  Reference pattern initialization (integer scale)
	TComPattern cPatternRoi;
	TComPattern cSymPatternRoi;
	Int deltaP, TRp, DistanceIndexFw, DistanceIndexBw, refframe, deltaPB;
	if (pcCU->getPicture()->getPictureType() == B_PICTURE)//需要确认是否正确
	{
		refframe = 0; //B帧参考帧
		deltaP = 2 * (pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_1, 0) - pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_0, 0));
		deltaP = (deltaP + 512) % 512;

		TRp = (refframe + 1) * deltaP;
		deltaPB = 2 * (pcCU->getPicture()->getPOC() - pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_0, 0));
		TRp = (TRp + 512) % 512;
		deltaPB = (deltaPB + 512) % 512;

		DistanceIndexFw = deltaPB;
		DistanceIndexBw = (TRp - DistanceIndexFw + 512) % 512;

	}
	else if (pcCU->getPicture()->getPictureType() == F_PICTURE)//需要确认是否正确
	{
		DistanceIndexFw = 2 * (pcCU->getPicture()->getPOC() - pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_0, 0));
		DistanceIndexFw = (DistanceIndexFw + 512) % 512;
		DistanceIndexBw = 2 * (pcCU->getPicture()->getPOC() - pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_1, 0));
		DistanceIndexBw = (DistanceIndexBw + 512) % 512;
		if (m_pcEncCfg->getBackgroundEnable())
		{
			DistanceIndexFw = 1;
			DistanceIndexBw = 1;
		}
	}
	TComMv pcBwMvInt;
	pcBwMvInt.set(scaleMv(pcMvInt->getHor(), DistanceIndexBw, DistanceIndexFw), scaleMv(pcMvInt->getVer(), DistanceIndexBw, DistanceIndexFw));
	Int         iOffset = pcMvInt->getHor() + pcMvInt->getVer() * iRefStride;
	Int iSymOffset;
	if (pcCU->getPicture()->getPictureType() == B_PICTURE)
	{
		iSymOffset = -pcBwMvInt.getHor() - pcBwMvInt.getVer() * iSymRefStride;
	}
#if DIFSym_bug
	else if (pcCU->getPicture()->getPictureType() == F_PICTURE)
#else
	else if (pcCU->getPicture()->getPictureType() == B_PICTURE)
#endif
	{
		iSymOffset = pcBwMvInt.getHor() + pcBwMvInt.getVer() * iSymRefStride;
	}

	cPatternRoi.initPattern(piRefY + iOffset,
		NULL,
		NULL,
		pcPatternKey->getROIYWidth(),
		pcPatternKey->getROIYHeight(),
		iRefStride,
		0, 0, 0, 0);

	cSymPatternRoi.initPattern(piSymRef + iSymOffset,
		NULL,
		NULL,
		pcPatternKey->getROIYWidth(),
		pcPatternKey->getROIYHeight(),
		iRefStride,
		0, 0, 0, 0);

	Pel*  piRef;
	iRefStride = m_cYuvExt.getStride();
	iSymRefStride = m_cYuvExtSym.getStride();

	//  Half-pel refinement
	xExtDIFUpSamplingH(&cPatternRoi, &m_cYuvExt);
	piRef = m_cYuvExt.getLumaAddr() + ((iRefStride + sc_iLumaHalfTaps) << 2);

	xExtDIFUpSamplingH(&cSymPatternRoi, &m_cYuvExtSym);
	piSymRef = m_cYuvExt.getLumaAddr() + ((iRefStride + sc_iLumaHalfTaps) << 2);

	rcMvHalf = *pcMvInt;   rcMvHalf <<= 1;    // for mv-cost
	Int ruiCostH = xPatternRefinementSym(pcCU, pcPatternKey, piRef, iRefStride, piSymRef, iSymRefStride, 4, DistanceIndexBw, DistanceIndexFw, 2, rcMvHalf);

	m_pcRdCost->setCostScale(0);

	//  Quater-pel refinement
	Pel*  piSrcPel = cPatternRoi.getROIY() + (rcMvHalf.getHor() >> 1) + cPatternRoi.getPatternLStride() * (rcMvHalf.getVer() >> 1);
	Int*  piSrc = m_piYuvExt + ((m_iYuvExtStride + sc_iLumaHalfTaps) << 2) + (rcMvHalf.getHor() << 1) + m_iYuvExtStride * (rcMvHalf.getVer() << 1);
	piRef += (rcMvHalf.getHor() << 1) + iRefStride * (rcMvHalf.getVer() << 1);

	xExtDIFUpSamplingQ(pcPatternKey, piRef, iRefStride, piSrcPel, cPatternRoi.getPatternLStride(), piSrc, m_iYuvExtStride, m_puiDFilter[rcMvHalf.getHor() + rcMvHalf.getVer() * 3]);

	Pel* piSymSrcPel;
	Int* piSymSrc;
	if (pcCU->getPicture()->getPictureType() == B_PICTURE)
	{
		piSymSrcPel = cSymPatternRoi.getROIY() + (-scaleMv(rcMvHalf.getHor() >> 1, DistanceIndexBw, DistanceIndexFw)) + cPatternRoi.getPatternLStride() * (-scaleMv(rcMvHalf.getVer() >> 1, DistanceIndexBw, DistanceIndexFw));
		piSymSrc = m_piYuvExt + ((m_iYuvExtStride + sc_iLumaHalfTaps) << 2) + (-scaleMv(rcMvHalf.getHor() << 1, DistanceIndexBw, DistanceIndexFw)) + m_iYuvExtStride * (-scaleMv(rcMvHalf.getVer() << 1, DistanceIndexBw, DistanceIndexFw));
		piSymRef += (-scaleMv(rcMvHalf.getHor() << 1, DistanceIndexBw, DistanceIndexFw)) + iRefStride * (-scaleMv(rcMvHalf.getVer() << 1, DistanceIndexBw, DistanceIndexFw));
	}
	else if (pcCU->getPicture()->getPictureType() == F_PICTURE)
	{
		piSymSrcPel = cSymPatternRoi.getROIY() + (scaleMv(rcMvHalf.getHor() >> 1, DistanceIndexBw, DistanceIndexFw)) + cPatternRoi.getPatternLStride() * (scaleMv(rcMvHalf.getVer() >> 1, DistanceIndexBw, DistanceIndexFw));
		piSymSrc = m_piYuvExt + ((m_iYuvExtStride + sc_iLumaHalfTaps) << 2) + (scaleMv(rcMvHalf.getHor() << 1, DistanceIndexBw, DistanceIndexFw)) + m_iYuvExtStride * (scaleMv(rcMvHalf.getVer() << 1, DistanceIndexBw, DistanceIndexFw));
		piSymRef += (scaleMv(rcMvHalf.getHor() << 1, DistanceIndexBw, DistanceIndexFw)) + iRefStride * (scaleMv(rcMvHalf.getVer() << 1, DistanceIndexBw, DistanceIndexFw));
	}
	xExtDIFUpSamplingQ(pcPatternKey, piSymRef, iSymRefStride, piSymSrcPel, cSymPatternRoi.getPatternLStride(), piSymSrc, m_iYuvExtStride, m_puiDFilter[rcMvHalf.getHor() + rcMvHalf.getVer() * 3]);

	rcMvQter = *pcMvInt;   rcMvQter <<= 1;    // for mv-cost
	rcMvQter += rcMvHalf;  rcMvQter <<= 1;

	Int ruiCostQ = xPatternRefinementQSym(pcCU, pcPatternKey, piRef, iRefStride, piSymRef, iSymRefStride, 4, DistanceIndexBw, DistanceIndexFw, iMvX, iMvY, rcMvQter);
	if ((ruiCostH < ruiCostQ) || (ruiCostH == ruiCostQ))
	{
		rcMvQter.set(0, 0);
		ruiSymCost = ruiCostH;
	}
	else
	{
		ruiSymCost = ruiCostQ;
	}

}
#endif

#if rd_bipred
Void TEncSearch::xPatternSearchFracDIFBi(TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, Pel* piBwRef, Int iBwRefStride, Int iMvX, Int iMvY, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost)
{
	//  Reference pattern initialization (integer scale)
	TComPattern cPatternRoi;
	Int         iOffset = pcMvInt->getHor() + pcMvInt->getVer() * iRefStride;
	cPatternRoi.initPattern(piRefY + iOffset,
		NULL,
		NULL,
		pcPatternKey->getROIYWidth(),
		pcPatternKey->getROIYHeight(),
		iRefStride,
		0, 0, 0, 0);
	Pel*  piRef;
	iRefStride = m_cYuvExt.getStride();

	//  Half-pel refinement
	xExtDIFUpSamplingH(&cPatternRoi, &m_cYuvExt);
	piRef = m_cYuvExt.getLumaAddr() + ((iRefStride + sc_iLumaHalfTaps) << 2);

	rcMvHalf = *pcMvInt;   rcMvHalf <<= 1;    // for mv-cost
	Int ruiCostH = xPatternRefinementBi(pcPatternKey, piRef, iRefStride, piBwRef, iBwRefStride, 4, 2, rcMvHalf);

	m_pcRdCost->setCostScale(0);

	//  Quater-pel refinement
	Pel*  piSrcPel = cPatternRoi.getROIY() + (rcMvHalf.getHor() >> 1) + cPatternRoi.getPatternLStride() * (rcMvHalf.getVer() >> 1);
	Int*  piSrc = m_piYuvExt + ((m_iYuvExtStride + sc_iLumaHalfTaps) << 2) + (rcMvHalf.getHor() << 1) + m_iYuvExtStride * (rcMvHalf.getVer() << 1);
	piRef += (rcMvHalf.getHor() << 1) + iRefStride * (rcMvHalf.getVer() << 1);

	xExtDIFUpSamplingQ(pcPatternKey, piRef, iRefStride, piSrcPel, cPatternRoi.getPatternLStride(), piSrc, m_iYuvExtStride, m_puiDFilter[rcMvHalf.getHor() + rcMvHalf.getVer() * 3]);


	rcMvQter = *pcMvInt;   rcMvQter <<= 1;    // for mv-cost
	rcMvQter += rcMvHalf;  rcMvQter <<= 1;

	Int ruiCostQ = xPatternRefinementQBi(pcPatternKey, piRef, iRefStride, piBwRef, iBwRefStride, 4, iMvX, iMvY, rcMvQter);
	if ((ruiCostH < ruiCostQ) || (ruiCostH == ruiCostQ))
	{
		rcMvQter.set(0, 0);
		ruiCost = ruiCostH;
	}
	else
	{
		ruiCost = ruiCostQ;
	}

}
#endif

#if DCTIF
Void TEncSearch::xPatternSearchFracDIF(TComDataCU* pcCU,
	TComPattern* pcPatternKey,
	Pel* piRefY,
	Int iRefStride,
#if RD_PMVR
	Int iMvX, 
	Int iMvY,
#endif
	TComMv* pcMvInt,
	TComMv& rcMvHalf,
	TComMv& rcMvQter,
	UInt& ruiCost
	)
{
	//  Reference pattern initialization (integer scale)
	Bool biPred = false; //双向预测，从前面传过来
	TComPattern cPatternRoi;
	Int         iOffset    = pcMvInt->getHor() + pcMvInt->getVer() * iRefStride;
	cPatternRoi.initPattern( piRefY +  iOffset,
		NULL,
		NULL,
		pcPatternKey->getROIYWidth(),
		pcPatternKey->getROIYHeight(),
		iRefStride,
		0, 0, 0, 0 );

	//  Half-pel refinement
	xExtDIFUpSamplingH ( &cPatternRoi, biPred );

	rcMvHalf = *pcMvInt;   rcMvHalf <<= 1;    // for mv-cost
	TComMv baseRefMv(0, 0);
	ruiCost = xPatternRefinement( pcPatternKey, baseRefMv, 2, rcMvHalf   );

	m_pcRdCost->setCostScale( 0 );

	xExtDIFUpSamplingQ ( &cPatternRoi, rcMvHalf, biPred );
	baseRefMv = rcMvHalf;
	baseRefMv <<= 1;

	rcMvQter = *pcMvInt;   rcMvQter <<= 1;    // for mv-cost
	rcMvQter += rcMvHalf;  rcMvQter <<= 1;
	ruiCost = xPatternRefinement( pcPatternKey, baseRefMv, 1, rcMvQter );
}
#else
#if RD_PMVR
Void TEncSearch::xPatternSearchFracDIF(TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, Int iMvX, Int iMvY, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost)
#else
Void TEncSearch::xPatternSearchFracDIF( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost )
#endif
{
  //  Reference pattern initialization (integer scale)
  TComPattern cPatternRoi;
  Int         iOffset    = pcMvInt->getHor() + pcMvInt->getVer() * iRefStride;
  cPatternRoi.initPattern( piRefY +  iOffset,
                          NULL,
                          NULL,
                          pcPatternKey->getROIYWidth(),
                          pcPatternKey->getROIYHeight(),
                          iRefStride,
                          0, 0, 0, 0 );
  Pel*  piRef;
  iRefStride  = m_cYuvExt.getStride();
  
  //  Half-pel refinement
  xExtDIFUpSamplingH ( &cPatternRoi, &m_cYuvExt );
  piRef = m_cYuvExt.getLumaAddr() + ((iRefStride + sc_iLumaHalfTaps) << 2);
  
  rcMvHalf = *pcMvInt;   rcMvHalf <<= 1;    // for mv-cost
  ruiCost = xPatternRefinement( pcPatternKey, piRef, iRefStride, 4, 2, rcMvHalf   );
  
  m_pcRdCost->setCostScale( 0 );
  
  //  Quater-pel refinement
  Pel*  piSrcPel = cPatternRoi.getROIY() + (rcMvHalf.getHor() >> 1) + cPatternRoi.getPatternLStride() * (rcMvHalf.getVer() >> 1);
  Int*  piSrc    = m_piYuvExt  + ((m_iYuvExtStride + sc_iLumaHalfTaps) << 2) + (rcMvHalf.getHor() << 1) + m_iYuvExtStride * (rcMvHalf.getVer() << 1);
  piRef += (rcMvHalf.getHor() << 1) + iRefStride * (rcMvHalf.getVer() << 1);

  xExtDIFUpSamplingQ ( pcPatternKey, piRef, iRefStride, piSrcPel, cPatternRoi.getPatternLStride(), piSrc, m_iYuvExtStride, m_puiDFilter[rcMvHalf.getHor()+rcMvHalf.getVer()*3] );


  rcMvQter = *pcMvInt;   rcMvQter <<= 1;    // for mv-cost
  rcMvQter += rcMvHalf;  rcMvQter <<= 1;
#if RD_PMVR
	ruiCost = xPatternRefinementQ(pcPatternKey, piRef, iRefStride, 4, iMvX, iMvY, rcMvQter);
#else
  ruiCost = xPatternRefinement( pcPatternKey, piRef, iRefStride, 4, 1, rcMvQter );
#endif
}
#endif

#if FME_SUB
Void TEncSearch::xPatternSearchFastFracDIF(TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, Int iPartIdx, Int iMvX, Int iMvY, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost)
{
	Int DiamondX[4] = { -1, 0, 1, 0 };
	Int DiamondY[4] = { 0, 1, 0, -1 };
	Int iCandMvX, iCandMvY;
	Int iCurrMvX, iCurrMvY;
	Int iXMinNow, iYMinNow;
	Int iSearchRangeDynamic = 3, iAbortSearch, iCurrSearchRange;
	UInt uiDist;
	UInt uiDistBest = MAX_UINT;
	Int iCtrX = (iMvX >> 1) << 1;
	Int iCtrY = (iMvY >> 1) << 1;
	Pel*  piRefPos;
	//UShort **SearchState = NULL;
	vector<vector<UShort>>SearchState(7, vector<UShort>(7));

	//  Reference pattern initialization (integer scale)
	TComPattern cPatternRoi;
	Int         iOffset = pcMvInt->getHor() + pcMvInt->getVer() * iRefStride;
	cPatternRoi.initPattern(piRefY + iOffset,
		NULL,
		NULL,
		pcPatternKey->getROIYWidth(),
		pcPatternKey->getROIYHeight(),
		iRefStride,
		0, 0, 0, 0);
	Pel*  piRef;
	iRefStride = m_cYuvExt.getStride();

	//  Half-pel refinement
	xExtDIFUpSamplingH(&cPatternRoi, &m_cYuvExt);
	piRef = m_cYuvExt.getLumaAddr() + ((iRefStride + sc_iLumaHalfTaps) << 2);

	m_pcRdCost->setCostScale(0);

	//  Quater-pel refinement
	Pel*  piSrcPel;
	Int*  piSrc;
#if FME_SUB_BUG
	for (Int i = 0; i < 9; i++)
	{
		rcMvHalf = s_acMvRefineQ[i];
		piSrcPel = cPatternRoi.getROIY() + (rcMvHalf.getHor() >> 1) + cPatternRoi.getPatternLStride() * (rcMvHalf.getVer() >> 1);
		piSrc = m_piYuvExt + ((m_iYuvExtStride + sc_iLumaHalfTaps) << 2) + (rcMvHalf.getHor() << 1) + m_iYuvExtStride * (rcMvHalf.getVer() << 1);
		xExtDIFUpSamplingQ(pcPatternKey, m_cYuvExt.getLumaAddr() + ((iRefStride + sc_iLumaHalfTaps) << 2) + (rcMvHalf.getHor() << 1) + iRefStride * (rcMvHalf.getVer() << 1), iRefStride, piSrcPel, cPatternRoi.getPatternLStride(), piSrc, m_iYuvExtStride, m_puiDFilter[rcMvHalf.getHor() + rcMvHalf.getVer() * 3]);
	}
	rcMvHalf.set(0, 0);
#endif
	rcMvQter = *pcMvInt;   rcMvQter <<= 1;    // for mv-cost
	rcMvQter += rcMvHalf;  rcMvQter <<= 1;

	Int predFracMvX = (iMvX - rcMvQter.getHor()) % 4;
	Int predFracMvY = (iMvY - rcMvQter.getVer()) % 4;

	for (Int i = 0; i < 7; i++)
	{
		for (Int j = 0; j < 7; j++)
		{
			SearchState[i][j] = 0;
		}
	}

	if (m_pcEncCfg->getUseHADME())
	{
		iCandMvX = rcMvQter.getHor();
		iCandMvY = rcMvQter.getVer();
		m_pcRdCost->setDistParam(pcPatternKey, piRef, iRefStride, 4, m_cDistParam, m_pcEncCfg->getUseHADME());
		piRefPos = piRef;
		m_cDistParam.pCur = piRefPos;
		uiDist = m_cDistParam.DistFunc(&m_cDistParam);
		uiDist += m_pcRdCost->getCost(iCandMvX, iCandMvY);
		SearchState[iSearchRangeDynamic][iSearchRangeDynamic] = 1;
		//test

		Char mv_range_flag = 1;
		Int pic_pix_x, pic_pix_y;
		pcCU->getInterPUxy(iPartIdx, pic_pix_x, pic_pix_y);
		mv_range_flag = check_mv_range(pcCU->getLog2CUSize(0), iCandMvX, iCandMvY, pic_pix_x, pic_pix_y, pcCU->getPartitionSize(0) + 1, 0, 0);
		if (!mv_range_flag)
		{
			uiDist = 0x0EEEEEEE;
			mv_range_flag = 1;
		}
#if print_key
		printf("1111 cost=%d\n", uiDist);
#endif
		if (uiDist < uiDistBest)
		{
			uiDistBest = uiDist;
			iCurrMvX = iCandMvX;
			iCurrMvY = iCandMvY;
		}
	}
	else
	{
		SearchState[iSearchRangeDynamic][iSearchRangeDynamic] = 1;
		iCurrMvX = rcMvQter.getHor();
		iCurrMvY = rcMvQter.getVer();
	}


	if (predFracMvX != 0 || predFracMvY != 0)
	{
		iCandMvX = rcMvQter.getHor() + predFracMvX;
		iCandMvY = rcMvQter.getVer() + predFracMvY;
		if (!m_pcEncCfg->getPMVREnable() || abs(iCandMvX - iCtrX) <= TH && abs(iCandMvY - iCtrY) <= TH)
		{
			m_pcRdCost->setDistParam(pcPatternKey, piRef, iRefStride, 4, m_cDistParam, m_pcEncCfg->getUseHADME());
			piRefPos = piRef + (predFracMvX + iRefStride * predFracMvY);
			m_cDistParam.pCur = piRefPos;
			uiDist = m_cDistParam.DistFunc(&m_cDistParam);
			uiDist += m_pcRdCost->getCost(iCandMvX, iCandMvY);
			SearchState[iCandMvY - rcMvQter.getVer() + iSearchRangeDynamic][iCandMvX - rcMvQter.getHor() + iSearchRangeDynamic] = 1;

			//test
			Char mv_range_flag = 1;
			Int pic_pix_x, pic_pix_y;
			pcCU->getInterPUxy(iPartIdx, pic_pix_x, pic_pix_y);
			mv_range_flag = check_mv_range(pcCU->getLog2CUSize(0), iCandMvX, iCandMvY, pic_pix_x, pic_pix_y, pcCU->getPartitionSize(0) + 1, 0, 0);
			if (!mv_range_flag)
			{
				uiDist = 0x0EEEEEEE;
				mv_range_flag = 1;
			}
#if print_key
			printf("2222 PredX=%d PredY=%d cost=%d\n", predFracMvX, predFracMvY, uiDist);
#endif
			if (uiDist < uiDistBest)
			{
				uiDistBest = uiDist;
				iCurrMvX = iCandMvX;
				iCurrMvY = iCandMvY;
				//piRef += predFracMvX + iRefStride * predFracMvY;
			}
		}
	}

	iXMinNow = iCurrMvX;
	iYMinNow = iCurrMvY;
	iCurrSearchRange = 2 * iSearchRangeDynamic + 1;

	for (Int i = 0; i < iCurrSearchRange; i++)
	{
		iAbortSearch = 1;
		for (Int m = 0; m < 4; m++)
		{
			if (m_pcEncCfg->getPMVREnable())
			{
				if (!pmvrAdaptMv2(&iCandMvX, &iCandMvY, iCtrX, iCtrY, iXMinNow, iYMinNow, DiamondX[m], DiamondY[m]))
				{
					continue;
				}
			}
			else
			{
				iCandMvX = iXMinNow + DiamondX[m];
				iCandMvY = iYMinNow + DiamondY[m];
			}

			//test
			Char mv_range_flag = 1;
			Int pic_pix_x, pic_pix_y;
			pcCU->getInterPUxy(iPartIdx, pic_pix_x, pic_pix_y);
			mv_range_flag = check_mv_range(pcCU->getLog2CUSize(0), iCandMvX, iCandMvY, pic_pix_x, pic_pix_y, pcCU->getPartitionSize(0) + 1, 0, 0);
			if (!mv_range_flag)
			{
				mv_range_flag = 1;
				continue;
			}

			if (abs(iCandMvX - rcMvQter.getHor()) <= iSearchRangeDynamic && abs(iCandMvY - rcMvQter.getVer()) <= iSearchRangeDynamic)
			{
				if (!SearchState[iCandMvY - rcMvQter.getVer() + iSearchRangeDynamic][iCandMvX - rcMvQter.getHor() + iSearchRangeDynamic])
				{
					m_pcRdCost->setDistParam(pcPatternKey, piRef, iRefStride, 4, m_cDistParam, m_pcEncCfg->getUseHADME());
					piRefPos = piRef + (iCandMvX - rcMvQter.getHor()) + (iCandMvY - rcMvQter.getVer()) * iRefStride;

					m_cDistParam.pCur = piRefPos;
					uiDist = m_cDistParam.DistFunc(&m_cDistParam);
					uiDist += m_pcRdCost->getCost(iCandMvX, iCandMvY);
					SearchState[iCandMvY - rcMvQter.getVer() + iSearchRangeDynamic][iCandMvX - rcMvQter.getHor() + iSearchRangeDynamic] = 1;
					//test
#if print_key
					printf("3333__%d cost=%d\n", m, uiDist);
#endif
					if (uiDist < uiDistBest)
					{
						uiDistBest = uiDist;
						iCurrMvX = iCandMvX;
						iCurrMvY = iCandMvY;
						iAbortSearch = 0;
					}
				}
			}
		}

#if print_key
		if ((abs(iCurrMvX) == 2) && (iCurrMvY == 0))
			printf("!!!!_1111: X=%d  Y=%d\n", iCurrMvX, iCurrMvY);
		if ((abs(iCurrMvY) == 2) && (iCurrMvX == 0))
			printf("!!!!_2222: X=%d  Y=%d\n", iCurrMvX, iCurrMvY);
		if ((abs(iCurrMvX) == 1) && (iCurrMvY == 1))
			printf("!!!!_3333: X=%d  Y=%d\n", iCurrMvX, iCurrMvY);
		if ((abs(iCurrMvX) == 2) && (iCurrMvY == 2))
			printf("!!!!_4444: X=%d  Y=%d\n", iCurrMvX, iCurrMvY);
#endif
		iXMinNow = iCurrMvX;
		iYMinNow = iCurrMvY;

		if (iAbortSearch)
		{
			break;
		}
	}

	rcMvQter.set(iCurrMvX - rcMvQter.getHor(), iCurrMvY - rcMvQter.getVer());
	ruiCost = uiDistBest;
}
#endif

#if !niu_CloseFastSubPel
Void TEncSearch::xFastPatternSearchFracDIF(TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, Int iPartIdx, TComMv cMvPred, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost)
{
	//  Reference pattern initialization (integer scale)
	TComPattern cPatternRoi;
	Int         iOffset = pcMvInt->getHor() + pcMvInt->getVer() * iRefStride;
	cPatternRoi.initPattern(piRefY + iOffset,
		NULL,
		NULL,
		pcPatternKey->getROIYWidth(),
		pcPatternKey->getROIYHeight(),
		iRefStride,
		0, 0, 0, 0);
	Pel*  piRef;
	iRefStride = m_cYuvExt.getStride();

	static Int Diamond_x[4] = { -1, 0, 1, 0 };
	static Int Diamond_y[4] = { 0, 1, 0, -1 };

	Int search_range_dynamic = 3;
	Int pred_frac_mv_x = (cMvPred.getHor() - (pcMvInt->getHor() << 2)) % 4;
	Int pred_frac_mv_y = (cMvPred.getVer() - (pcMvInt->getVer() << 2)) % 4;
	TComMv CurrMv;
	unsigned short SearchState[7][7] = { 0 };

	UInt  uiDistBest = MAX_UINT;
	Pel*  piRefPos;
	TComMv cMvTest;  // refine增加量pcMvRefine  (1) 0   (2)pred_frac_mv  (3)Diamond
	Pel*  piSrcPel;
	Int*  piSrc;
	//Pel *piRef_tb;
	TComMv rcMvQter_final, rcMvQter_temp;

	rcMvQter_final.set(0, 0);


	/*Int pic_pix_x, pic_pix_y;
	pcCU->getInterPUxy(iPartIdx, pic_pix_x, pic_pix_y);*/
	////////////////////////////////////////////////////////////////////////

	if (m_pcEncCfg->getUseHADME())
	{
		////  Half-pel refinement
		xExtDIFUpSamplingH(&cPatternRoi, &m_cYuvExt);

		piRef = m_cYuvExt.getLumaAddr() + ((iRefStride + sc_iLumaHalfTaps) << 2);


		//rcMvHalf = *pcMvInt;   rcMvHalf <<= 1;    // for mv-cost
		//ruiCost = xPatternRefinement(pcPatternKey, piRef, iRefStride, 4, 2, rcMvHalf);

		rcMvHalf.set(0, 0);  //pcMvRefine

		m_pcRdCost->setCostScale(0);

		//  Quater-pel refinement
		piSrcPel = cPatternRoi.getROIY() + (rcMvHalf.getHor() >> 1) + cPatternRoi.getPatternLStride() * (rcMvHalf.getVer() >> 1);
		piSrc = m_piYuvExt + ((m_iYuvExtStride + sc_iLumaHalfTaps) << 2) + (rcMvHalf.getHor() << 1) + m_iYuvExtStride * (rcMvHalf.getVer() << 1);
		//piRef += (rcMvHalf.getHor() << 1) + iRefStride * (rcMvHalf.getVer() << 1);
		xExtDIFUpSamplingQ(pcPatternKey, piRef, iRefStride, piSrcPel, cPatternRoi.getPatternLStride(), piSrc, m_iYuvExtStride, m_puiDFilter[0]);

		/*piSrc = m_piYuvExt;
		xExtDIFUpSamplingQ(pcPatternKey, piRefY, iRefStride, piSrcPel, cPatternRoi.getPatternLStride(), piSrc, m_iYuvExtStride, m_puiDFilter[0]);
		piRef = piRefY ;*/


		rcMvQter = *pcMvInt;   rcMvQter <<= 1;    // for mv-cost
		rcMvQter += rcMvHalf;  rcMvQter <<= 1;
		//ruiCost = xPatternRefinement(pcPatternKey, piRef, iRefStride, 4, 1, rcMvQter);

		m_pcRdCost->setDistParam(pcPatternKey, piRef, iRefStride, 4, m_cDistParam, m_pcEncCfg->getUseHADME());
		cMvTest = rcMvQter;  //  (1) 0
		piRefPos = piRef + (0 + iRefStride * 0) * 1;  // iFrac
		m_cDistParam.pCur = piRefPos;
		{
			Pel *orig_pic = m_cDistParam.pOrg;
			Pel * PelY_14 = m_cDistParam.pCur;
			Int iStep = m_cDistParam.iStep;
			printf("\nOrg   Ref  Hor=%d  Ver=%d\n", cMvTest.getHor(), cMvTest.getVer());
			printf("%d    %d\n", orig_pic[0], PelY_14[0 * iStep]);
			printf("%d    %d\n", orig_pic[1], PelY_14[1 * iStep]);
			printf("%d    %d\n", orig_pic[2], PelY_14[2 * iStep]);
			printf("%d    %d\n", orig_pic[3], PelY_14[3 * iStep]);
		}
		ruiCost = m_cDistParam.DistFunc(&m_cDistParam);
		ruiCost += m_pcRdCost->getCost(cMvTest.getHor(), cMvTest.getVer());

		printf("\nCost1 = %d\n", ruiCost);
		if (ruiCost < uiDistBest)
		{
			//piRef_tb = piRefPos;
			uiDistBest = ruiCost;
			rcMvQter_temp.set(0, 0);  //pcMvRefine
			rcMvQter_final += rcMvQter_temp;
			CurrMv = cMvTest;
		}
		SearchState[search_range_dynamic][search_range_dynamic] = 1;

	}
	else
	{
		SearchState[search_range_dynamic][search_range_dynamic] = 1;
		CurrMv = cMvTest;
	}
	////////////////////////////////////////////////////////////////////////

	if (pred_frac_mv_x != 0 || pred_frac_mv_y != 0)
	{
		//xExtDIFUpSamplingH(&cPatternRoi, &m_cYuvExt);
		//piRef = m_cYuvExt.getLumaAddr() + ((iRefStride + sc_iLumaHalfTaps) << 2);

		//rcMvHalf = *pcMvInt;   rcMvHalf <<= 1;    // for mv-cost
		////ruiCost = xPatternRefinement(pcPatternKey, piRef, iRefStride, 4, 2, rcMvHalf);


		////test
		/*Int pred_frac_mv_x_2 = ((cMvPred.getHor() >> 1) - (pcMvInt->getHor() << 1)) % 2;
		Int pred_frac_mv_y_2 = ((cMvPred.getVer() >> 1) - (pcMvInt->getVer() << 1)) % 2;*/

		//m_pcRdCost->setDistParam(pcPatternKey, piRef, iRefStride, 4, m_cDistParam, m_pcEncCfg->getUseHADME());
		//cMvTest.set(pred_frac_mv_x_2, pred_frac_mv_y_2);
		//cMvTest += rcMvHalf;
		//piRefPos = piRef + (pred_frac_mv_x_2 + iRefStride * pred_frac_mv_y_2) * 2;
		//m_cDistParam.pCur = piRefPos;

		//ruiCost = m_cDistParam.DistFunc(&m_cDistParam);
		//ruiCost += m_pcRdCost->getCost(cMvTest.getHor(), cMvTest.getVer());

		//if (ruiCost < uiDistBest)
		//	uiDistBest = ruiCost;
		//rcMvHalf.set(pred_frac_mv_x_2, pred_frac_mv_y_2);  //pcMvRefine
		//rcMvHalf.set(pred_frac_mv_x / 2, pred_frac_mv_y / 2);  //pcMvRefine
		rcMvHalf.set(0, 0);

		//piRef = piRef_tb;
		m_pcRdCost->setCostScale(0);
		//  Quater-pel refinement
		/*piSrcPel = cPatternRoi.getROIY() + (rcMvHalf.getHor() >> 1) + cPatternRoi.getPatternLStride() * (rcMvHalf.getVer() >> 1);
		piSrc = m_piYuvExt + ((m_iYuvExtStride + sc_iLumaHalfTaps) << 2) + (rcMvHalf.getHor() << 1) + m_iYuvExtStride * (rcMvHalf.getVer() << 1);
		piRef += (rcMvHalf.getHor() << 1) + iRefStride * (rcMvHalf.getVer() << 1);
		xExtDIFUpSamplingQ(pcPatternKey, piRef, iRefStride, piSrcPel, cPatternRoi.getPatternLStride(), piSrc, m_iYuvExtStride, m_puiDFilter[0]);*/


		rcMvQter = *pcMvInt;   rcMvQter <<= 1;    // for mv-cost
		rcMvQter += rcMvHalf;  rcMvQter <<= 1;
		//ruiCost = xPatternRefinement(pcPatternKey, piRef, iRefStride, 4, 1, rcMvQter);

		//m_pcRdCost->setDistParam(pcPatternKey, piRef, iRefStride, 4, m_cDistParam, m_pcEncCfg->getUseHADME());

		cMvTest.set(pred_frac_mv_x, pred_frac_mv_y);
		cMvTest += rcMvQter;  //  (2) 
		//piRefPos = piRef + (pred_frac_mv_x + iRefStride * pred_frac_mv_y) * 1;  // iFrac
		//piRefPos = piRef + (pred_frac_mv_x + rcMvQter_final.getHor() + iRefStride * (pred_frac_mv_y + rcMvQter_final.getVer())) * 1;  // iFra
		piRefPos = piRef + (cMvTest.getHor() - (pcMvInt->getHor() << 2) + iRefStride * (cMvTest.getVer() - (pcMvInt->getVer() << 2))) * 1;  // iFrac
		m_cDistParam.pCur = piRefPos;
		{
			Pel *orig_pic = m_cDistParam.pOrg;
			Pel * PelY_14 = m_cDistParam.pCur;
			Int iStep = m_cDistParam.iStep;
			printf("\nOrg   Ref  Hor=%d  Ver=%d\n", cMvTest.getHor(), cMvTest.getVer());
			printf("%d    %d\n", orig_pic[0], PelY_14[0 * iStep]);
			printf("%d    %d\n", orig_pic[1], PelY_14[1 * iStep]);
			printf("%d    %d\n", orig_pic[2], PelY_14[2 * iStep]);
			printf("%d    %d\n", orig_pic[3], PelY_14[3 * iStep]);
		}
		ruiCost = m_cDistParam.DistFunc(&m_cDistParam);
		ruiCost += m_pcRdCost->getCost(cMvTest.getHor(), cMvTest.getVer());

		printf("\nCost2 = %d\n", ruiCost);
		if (ruiCost < uiDistBest)
		{
			//piRef_tb = piRefPos;
			uiDistBest = ruiCost;
			rcMvQter_temp.set(pred_frac_mv_x, pred_frac_mv_y);  //pcMvRefine
			rcMvQter_final += rcMvQter_temp;
			CurrMv = cMvTest;
		}
		SearchState[cMvTest.getVer() - (pcMvInt->getVer() << 2) + search_range_dynamic][cMvTest.getHor() - (pcMvInt->getHor() << 2) + search_range_dynamic] = 1;
	}

	////////////////////////////////////////////////////////////////////////

	TComMv MinNow = CurrMv;

	Int  iCurrSearchRange = 2 * search_range_dynamic + 1;
	Int abort_search;
	//Pel *piRef_temp;
	for (Int i = 0; i < iCurrSearchRange; i++)
	{
		//piRef_temp = piRef_tb;
		abort_search = 1;
		for (Int m = 0; m < 4; m++)
		{
			//  input->b_pmvr_enabled
			cMvTest.set(Diamond_x[m], Diamond_y[m]);
			cMvTest += MinNow;


			if (absm(cMvTest.getHor() - (pcMvInt->getHor() << 2)) <= search_range_dynamic && absm(cMvTest.getVer() - (pcMvInt->getVer() << 2)) <= search_range_dynamic)
			{
				if (!SearchState[cMvTest.getVer() - (pcMvInt->getVer() << 2) + search_range_dynamic][cMvTest.getHor() - (pcMvInt->getHor() << 2) + search_range_dynamic])
				{
					//piRef = piRef_temp;
					rcMvHalf.set(0, 0);
					m_pcRdCost->setCostScale(0);
					//m_pcRdCost->setDistParam(pcPatternKey, piRef, iRefStride, 4, m_cDistParam, m_pcEncCfg->getUseHADME());

					//piRefPos = piRef + (Diamond_x[m] + iRefStride * Diamond_y[m]) * 1;  // iFrac
					//piRefPos = piRef + (Diamond_x[m] + rcMvQter_final.getHor() + iRefStride * (Diamond_y[m] + rcMvQter_final.getVer())) * 1;  // iFrac
					piRefPos = piRef + (cMvTest.getHor() - (pcMvInt->getHor() << 2) + iRefStride * (cMvTest.getVer() - (pcMvInt->getVer() << 2))) * 1;  // iFrac
					m_cDistParam.pCur = piRefPos;

					{
						Pel *orig_pic = m_cDistParam.pOrg;
						Pel * PelY_14 = m_cDistParam.pCur;
						Int iStep = m_cDistParam.iStep;
						printf("\nOrg   Ref  I=%d  M=%d  Hor=%d  Ver=%d\n", i, m, cMvTest.getHor(), cMvTest.getVer());
						printf("%d    %d\n", orig_pic[0], PelY_14[0 * iStep]);
						printf("%d    %d\n", orig_pic[1], PelY_14[1 * iStep]);
						printf("%d    %d\n", orig_pic[2], PelY_14[2 * iStep]);
						printf("%d    %d\n", orig_pic[3], PelY_14[3 * iStep]);
					}
					ruiCost = m_cDistParam.DistFunc(&m_cDistParam);
					ruiCost += m_pcRdCost->getCost(cMvTest.getHor(), cMvTest.getVer());

					printf("CCCost=%d\n", ruiCost);

					if (ruiCost < uiDistBest)
					{
						//piRef_tb = piRefPos;
						uiDistBest = ruiCost;
						rcMvQter_temp.set(Diamond_x[m], Diamond_y[m]);  //pcMvRefine						
						CurrMv = cMvTest;
						abort_search = 0;
					}
					SearchState[cMvTest.getVer() - (pcMvInt->getVer() << 2) + search_range_dynamic][cMvTest.getHor() - (pcMvInt->getHor() << 2) + search_range_dynamic] = 1;
				}
			}
		}

		MinNow = CurrMv;
		if (abort_search)
			break;
		else
			rcMvQter_final += rcMvQter_temp;
	}


	//////////
	rcMvQter = rcMvQter_final;
	ruiCost = uiDistBest;
}
#endif

#if inter_direct_skip_bug
Void TEncSearch::encodeResAndCalcRdInterCU_NO_QT(TComDataCU* pcCU, TComYuv* pcYuvOrg, TComYuv* pcYuvPred, TComYuv*& rpcYuvResi, TComYuv*& rpcYuvResiBest, TComYuv*& rpcYuvRec, Bool bSkipRes)
{
	assert(!pcCU->isIntra(0));


	Bool    bHighPass = pcCU->getPicture()->getDepth() ? true : false;
	UInt    uiBits = 0, uiBitsBest = 0;
	UInt    uiDistortion = 0, uiDistortionBest = 0;
	UInt   uiDistortiontemp = 0;
#if RD_NSQT
	UInt      uiWidth = pcCU->getWidth(0);
	UInt      uiHeight = pcCU->getHeight(0);
#else
	UInt      uiWidth = pcCU->getWidth(0);
	UInt      uiHeight = pcCU->getHeight(0);
#endif

	UInt    uiCUSize = 1 << pcCU->getLog2CUSize(0);
	Double  dCost;

	UInt uiAbsZorderIdx = pcCU->getZorderIdxInCU();
#if wlq_avs2x_debug
	UInt qq, ddy, ddu, ddv, sign;
#endif


	// Non-skipped CU -------------------------------------------------------------------------------------
	UInt uiTrMode, uiBestTrMode = 0;
	Double dCostBest = MAX_DOUBLE;

	///////////////////////////////////////////////////////////////////////////
#if inter_direct_skip_bug
#if wlq_inter_tu_rdo_debug
	//if (pcCU->getCbf(0, TEXT_LUMA, pcCU->getTransformIdx(0)) || pcCU->getCbf(0, TEXT_CHROMA, pcCU->getTransformIdx(0)))
	{
		for (int i = 1; i >= 0; i--)
		{
			if (i == 1)
				uiTrMode = 1;
			else if (i == 0)
				uiTrMode = 0;
			else
				printf("wrong");

			pcCU->setCbfSubParts(0x00, TEXT_LUMA, 0, pcCU->getDepth(0) + uiTrMode);
			pcCU->setCbfSubParts(0x00, TEXT_CHROMA, 0, pcCU->getDepth(0) + uiTrMode);


#else
	for (uiTrMode = 0; uiTrMode <= 1; uiTrMode++)
	{
#endif
#if !WLQ_2NxnU
		if (uiTrMode > 0 && pcCU->getPartitionSize(0) >= SIZE_2NxnU && pcCU->getPartitionSize(0) <= SIZE_nRx2N)
		{
			uiTrMode = 2;
		}
#endif

		pcCU->setTrIdxSubParts(uiTrMode, 0, pcCU->getDepth(0));

		::memset(pcCU->getCoeffY(), 0, sizeof(TCoeff) * uiWidth * uiHeight);

		rpcYuvResi->subtract(pcYuvOrg, pcYuvPred, uiAbsZorderIdx, uiCUSize);
#if wlq_avs2x_debug
		{
			int iii, jjj;
			Pel *xxx;
			sign = 0;
			if (sign)
			{
				printf("\n");
				printf("org\n");
				xxx = pcYuvOrg->getLumaAddr(pcCU->getZorderIdxInCU());
				for (iii = 0; iii < uiHeight; iii++)
				{
					for (jjj = 0; jjj < uiWidth; jjj++)
					{
						printf("%d\t", xxx[jjj]);
					}
					printf("\n");
					xxx += pcYuvOrg->getStride();
				}
				printf("\n");

				printf("\n");
				printf("pred\n");
				xxx = pcYuvPred->getLumaAddr(pcCU->getZorderIdxInCU());
				for (iii = 0; iii < uiHeight; iii++)
				{
					for (jjj = 0; jjj < uiWidth; jjj++)
					{
						printf("%d\t", xxx[jjj]);
					}
					printf("\n");
					xxx += pcYuvPred->getStride();
				}
				printf("\n");

				printf("\n");
				printf("resi_in\n");
				xxx = rpcYuvResi->getLumaAddr(pcCU->getZorderIdxInCU());
				for (iii = 0; iii < uiHeight; iii++)
				{
					for (jjj = 0; jjj < uiWidth; jjj++)
					{
						printf("%d\t", xxx[jjj]);
					}
					printf("\n");
					xxx += rpcYuvResi->getStride();
				}
				printf("\n");
			}

		{
			//Cb
			//org
			Pel uv_org[128][128];
			Pel uv_pred[128][128];
			Pel uv_resi[128][128];
			xxx = pcYuvOrg->getCbAddr(pcCU->getZorderIdxInCU());
			for (iii = 0; iii < uiHeight / 2; iii++)
			{
				for (jjj = 0; jjj < uiWidth / 2; jjj++)
				{
					uv_org[iii][jjj] = xxx[jjj];
				}
				xxx += pcYuvOrg->getCStride();
			}
			//Cb
			//pred
			xxx = pcYuvPred->getCbAddr(pcCU->getZorderIdxInCU());
			for (iii = 0; iii < uiHeight / 2; iii++)
			{
				for (jjj = 0; jjj < uiWidth / 2; jjj++)
				{
					uv_pred[iii][jjj] = xxx[jjj];
				}
				xxx += pcYuvPred->getCStride();
			}
			//Cb
			//resi
			xxx = rpcYuvResi->getCbAddr(pcCU->getZorderIdxInCU());
			for (iii = 0; iii < uiHeight / 2; iii++)
			{
				for (jjj = 0; jjj < uiWidth / 2; jjj++)
				{
					uv_resi[iii][jjj] = xxx[jjj];
				}
				xxx += rpcYuvResi->getCStride();
			}

			//Cr
			//org
			xxx = pcYuvOrg->getCrAddr(pcCU->getZorderIdxInCU());
			for (iii = 0; iii < uiHeight / 2; iii++)
			{
				for (jjj = 0; jjj < uiWidth / 2; jjj++)
				{
					uv_org[iii][jjj] = xxx[jjj];
				}
				xxx += pcYuvOrg->getCStride();
			}
			//Cr
			//pred
			xxx = pcYuvPred->getCrAddr(pcCU->getZorderIdxInCU());
			for (iii = 0; iii < uiHeight / 2; iii++)
			{
				for (jjj = 0; jjj < uiWidth / 2; jjj++)
				{
					uv_pred[iii][jjj] = xxx[jjj];
				}
				xxx += pcYuvPred->getCStride();
			}
			//Cr
			//resi
			xxx = rpcYuvResi->getCrAddr(pcCU->getZorderIdxInCU());
			for (iii = 0; iii < uiHeight / 2; iii++)
			{
				for (jjj = 0; jjj < uiWidth / 2; jjj++)
				{
					uv_resi[iii][jjj] = xxx[jjj];
				}
				xxx += rpcYuvResi->getCStride();
			}
			sign = 0;

		}
		}
#endif

#if inter_direct_skip_bug
#else
		xEncodeInterTexture(pcCU, rpcYuvResi, uiTrMode);
#endif

		rpcYuvRec->addClipSkip(pcYuvPred, pcCU->getZorderIdxInCU(), uiCUSize);
#if wlq_avs2x_debug
		{
			int iii, jjj;
			Pel *xxx;
			sign = 0;
			if (sign)
			{
				printf("\n");
				printf("pred\n");
				xxx = pcYuvPred->getLumaAddr(pcCU->getZorderIdxInCU());
				for (iii = 0; iii < 8; iii++)
				{
					for (jjj = 0; jjj < 8; jjj++)
					{
						printf("%d\t", xxx[jjj]);
					}
					printf("\n");
					xxx += pcYuvPred->getStride();
				}
				printf("\n");

				printf("\n");
				printf("resi_rec\n");
				xxx = rpcYuvResi->getLumaAddr(pcCU->getZorderIdxInCU());
				for (iii = 0; iii < 8; iii++)
				{
					for (jjj = 0; jjj < 8; jjj++)
					{
						printf("%d\t", xxx[jjj]);
					}
					printf("\n");
					xxx += rpcYuvResi->getStride();
				}
				printf("\n");

			}
			sign = 0;
		}
#endif
		// update with clipped distortion and cost
		uiDistortion = m_pcRdCost->getDistPart(rpcYuvRec->getLumaAddr(uiAbsZorderIdx), rpcYuvRec->getStride(), pcYuvOrg->getLumaAddr(uiAbsZorderIdx), pcYuvOrg->getStride(), uiWidth, uiHeight)
			+ (UInt)(m_pcRdCost->getChromaWeight()*(Double)(m_pcRdCost->getDistPart(rpcYuvRec->getCbAddr(uiAbsZorderIdx), rpcYuvRec->getCStride(), pcYuvOrg->getCbAddr(uiAbsZorderIdx), pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1)
			+ m_pcRdCost->getDistPart(rpcYuvRec->getCrAddr(uiAbsZorderIdx), rpcYuvRec->getCStride(), pcYuvOrg->getCrAddr(uiAbsZorderIdx), pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1)));

		uiBits = 0;
		m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_CURR_BEST]);
#if wlq_avs2x_debug
		g_sign = 0;
#endif
		xAddSymbolBitsInter(pcCU, uiBits, bSkipRes);
#if wlq_avs2x_debug
		g_sign = 0;
#endif
		dCost = m_pcRdCost->calcRdCost(uiBits, uiDistortion);
#if wlq_avs2x_debug
		{
			int iii, jjj;
			Pel *xxx = rpcYuvRec->getLumaAddr(uiAbsZorderIdx);
			qq = m_pcRdCost->getChromaWeight();
			ddy = m_pcRdCost->getDistPart(rpcYuvRec->getLumaAddr(uiAbsZorderIdx), rpcYuvRec->getStride(), pcYuvOrg->getLumaAddr(uiAbsZorderIdx), pcYuvOrg->getStride(), uiWidth, uiHeight);
			ddu = m_pcRdCost->getDistPart(rpcYuvRec->getCbAddr(uiAbsZorderIdx), rpcYuvRec->getCStride(), pcYuvOrg->getCbAddr(uiAbsZorderIdx), pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1);
			ddv = m_pcRdCost->getDistPart(rpcYuvRec->getCrAddr(uiAbsZorderIdx), rpcYuvRec->getCStride(), pcYuvOrg->getCrAddr(uiAbsZorderIdx), pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1);
			sign = 0;
			if (sign)
			{
				printf("\n");
				printf("rec\n");
				for (iii = 0; iii < 8; iii++)
				{
					for (jjj = 0; jjj < 8; jjj++)
					{
						printf("%d\t", xxx[jjj]);
					}
					printf("\n");
					xxx += rpcYuvRec->getStride();
				}
				printf("\n");
			}
			sign = 0;
		}
#endif
#if print_rd
		printf("CTU_addr = %d\t", pcCU->getAddr());
		printf("Type=%d\t", (pcCU->getPredictionMode(0) == MODE_DIRECT || pcCU->getPredictionMode(0) == MODE_SKIP) ? 0 : pcCU->getPartitionSize(0) + 1);
		printf("Size=%d\t",g_uiLog2MaxCUSize - pcCU->getDepth(0));
		printf("rate_all=%d\t", uiBits);
		printf("distortion_all=%d\t", uiDistortion);
		printf("rdcost=%f\t\n", dCost);
#endif

		if (dCost < dCostBest)
		{
			uiBitsBest = uiBits;
			uiDistortionBest = uiDistortion;
			dCostBest = dCost;
			uiBestTrMode = uiTrMode;
			m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_TEMP_BEST]);

		}
	}
		}
#endif
///////////////////////////////////////////////////////////////////////////




  rpcYuvResi->subtract( pcYuvOrg, pcYuvPred, uiAbsZorderIdx, uiCUSize );
  m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[ pcCU->getDepth( 0 ) ][ CI_CURR_BEST ] );
 // xEncodeInterTexture( pcCU, rpcYuvResi, uiBestTrMode );
#if WLQ_inter_bug
  m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_TEMP_BEST]);
#endif
 // rpcYuvRec->addClip ( pcYuvPred,                 rpcYuvResi, uiAbsZorderIdx,      uiCUSize  );
  rpcYuvRec->addClipSkip(pcYuvPred, pcCU->getZorderIdxInCU(), uiCUSize);
  pcCU->getTotalBits()       = uiBitsBest;
  pcCU->getTotalDistortion() = uiDistortionBest;
  pcCU->getTotalCost()       = dCostBest;
  pcCU->setTrIdxSubParts( uiBestTrMode, 0, pcCU->getDepth(0) );
	}


#endif

Void TEncSearch::encodeResAndCalcRdInterCU(TComDataCU* pcCU, TComYuv* pcYuvOrg, TComYuv* pcYuvPred, TComYuv*& rpcYuvResi, TComYuv*& rpcYuvResiBest, TComYuv*& rpcYuvRec, Bool bSkipRes)
{
	assert(!pcCU->isIntra(0));


	Bool    bHighPass = pcCU->getPicture()->getDepth() ? true : false;
	UInt    uiBits = 0, uiBitsBest = 0;
	UInt    uiDistortion = 0, uiDistortionBest = 0;
	UInt   uiDistortiontemp = 0;
#if RD_NSQT
	UInt      uiWidth = pcCU->getWidth(0);
	UInt      uiHeight = pcCU->getHeight(0);
#else
	UInt      uiWidth = pcCU->getWidth(0);
	UInt      uiHeight = pcCU->getHeight(0);
#endif

	UInt    uiCUSize = 1 << pcCU->getLog2CUSize(0);
	Double  dCost;

	UInt uiAbsZorderIdx = pcCU->getZorderIdxInCU();
#if wlq_avs2x_debug
	UInt qq, ddy, ddu, ddv, sign;
#endif
	//#if INTERTEST
	//  uiDistortiontemp = m_pcRdCost->getDistPart(pcYuvPred->getLumaAddr(uiAbsZorderIdx), pcYuvPred->getStride(), pcYuvOrg->getLumaAddr(uiAbsZorderIdx), pcYuvOrg->getStride(), uiWidth, uiHeight)
	//    + (UInt)(m_pcRdCost->getChromaWeight()*(Double)(m_pcRdCost->getDistPart(pcYuvPred->getCbAddr(uiAbsZorderIdx), pcYuvPred->getCStride(), pcYuvOrg->getCbAddr(uiAbsZorderIdx), pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1)
	//    + m_pcRdCost->getDistPart(pcYuvPred->getCrAddr(uiAbsZorderIdx), pcYuvPred->getCStride(), pcYuvOrg->getCrAddr(uiAbsZorderIdx), pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1)));
	//  printf("uiDistortion_start :%4d\t", uiDistortiontemp);
	//#endif
	// Skipped CU ----------------------------------------------------------------------------------
	if (0)
		// if ( bSkipRes )
	{
		// clear cbf and tridx
		const UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(0) << 1);
		UInt uiCoeffSize = (1 << (pcCU->getLog2CUSize(0) << 1)) * sizeof(TCoeff);
		rpcYuvResi->clear(uiAbsZorderIdx, uiCUSize);

		::memset(pcCU->getCbf(TEXT_LUMA), 0, uiQPartNum * sizeof(UChar));
		::memset(pcCU->getCbf(TEXT_CHROMA_U), 0, uiQPartNum * sizeof(UChar));
		::memset(pcCU->getCbf(TEXT_CHROMA_V), 0, uiQPartNum * sizeof(UChar));
		::memset(pcCU->getCoeffY(), 0, uiCoeffSize); uiCoeffSize >>= 2;
		::memset(pcCU->getCoeffCb(), 0, uiCoeffSize);
		::memset(pcCU->getCoeffCr(), 0, uiCoeffSize);

		pcCU->setTrIdxSubParts(0, 0, pcCU->getDepth(0));

		// copy prediction to reconstruction
		pcYuvPred->copyPartToPartYuv(rpcYuvRec, pcCU->getZorderIdxInCU(), pcCU->getZorderIdxInCU(), uiWidth, uiHeight);

		// compute number of bits
		m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_CURR_BEST]);

		m_pcEntropyCoder->resetBits();
#if PU_ZY
		m_pcEntropyCoder->encodeInterCUTypeIndex(pcCU, 0, false);
#else
		m_pcEntropyCoder->encodePredMode(pcCU, 0, true);
#endif
#if RPS
		m_pcEntropyCoder->encodePredInfo(pcCU, 0, false);
#endif
		m_pcEntropyCoder->encodeDBKIdx(pcCU, 0);

		uiBits = m_pcEntropyCoder->getNumberOfWrittenBits();
		m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_TEMP_BEST]);

		uiDistortion = m_pcRdCost->getDistPart(rpcYuvRec->getLumaAddr(uiAbsZorderIdx), rpcYuvRec->getStride(), pcYuvOrg->getLumaAddr(uiAbsZorderIdx), pcYuvOrg->getStride(), uiWidth, uiHeight)
			+ (UInt)(m_pcRdCost->getChromaWeight()*(Double)(m_pcRdCost->getDistPart(rpcYuvRec->getCbAddr(uiAbsZorderIdx), rpcYuvRec->getCStride(), pcYuvOrg->getCbAddr(uiAbsZorderIdx), pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1)
			+ m_pcRdCost->getDistPart(rpcYuvRec->getCrAddr(uiAbsZorderIdx), rpcYuvRec->getCStride(), pcYuvOrg->getCrAddr(uiAbsZorderIdx), pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1)));

		dCost = m_pcRdCost->calcRdCost(uiBits, uiDistortion);
		pcCU->getTotalBits() = uiBits;
		pcCU->getTotalDistortion() = uiDistortion;
		pcCU->getTotalCost() = dCost;

		return;
	}
	// ---------------------------------------------------------------------------------- End of skipped CU

	// Non-skipped CU -------------------------------------------------------------------------------------
	UInt uiTrMode, uiBestTrMode = 0;
	Double dCostBest = MAX_DOUBLE;
#if wlq_inter_tu_rdo_debug
	for (int i = 1; i >= 0; i--)
	{
		if (i == 1)
			uiTrMode = 1;
		else if (i == 0)
			uiTrMode = 0;
		else
			printf("wrong");

#else
	for (uiTrMode = 0; uiTrMode<=1; uiTrMode++)
	{
#endif
#if !WLQ_2NxnU
		if( uiTrMode > 0 && pcCU->getPartitionSize(0) >= SIZE_2NxnU && pcCU->getPartitionSize(0) <= SIZE_nRx2N )
		{
			uiTrMode = 2;
		}
#endif

		pcCU->setTrIdxSubParts(uiTrMode, 0, pcCU->getDepth(0));

		::memset(pcCU->getCoeffY(), 0, sizeof(TCoeff) * uiWidth * uiHeight);

		rpcYuvResi->subtract(pcYuvOrg, pcYuvPred, uiAbsZorderIdx, uiCUSize);
#if wlq_avs2x_debug
		{
			int iii, jjj;
			Pel *xxx;
			sign = 0;
			if (sign)
			{
				printf("\n");
				printf("org\n");
				xxx = pcYuvOrg->getLumaAddr(pcCU->getZorderIdxInCU());
				for (iii = 0; iii < uiHeight; iii++)
				{
					for (jjj = 0; jjj < uiWidth; jjj++)
					{
						printf("%d\t", xxx[jjj]);
					}
					printf("\n");
					xxx += pcYuvOrg->getStride();
				}
				printf("\n");

				printf("\n");
				printf("pred\n");
				xxx = pcYuvPred->getLumaAddr(pcCU->getZorderIdxInCU());
				for (iii = 0; iii < uiHeight; iii++)
				{
					for (jjj = 0; jjj < uiWidth; jjj++)
					{
						printf("%d\t", xxx[jjj]);
					}
					printf("\n");
					xxx += pcYuvPred->getStride();
				}
				printf("\n");

				printf("\n");
				printf("resi_in\n");
				xxx = rpcYuvResi->getLumaAddr(pcCU->getZorderIdxInCU());
				for (iii = 0; iii < uiHeight; iii++)
				{
					for (jjj = 0; jjj < uiWidth; jjj++)
					{
						printf("%d\t", xxx[jjj]);
					}
					printf("\n");
					xxx += rpcYuvResi->getStride();
				}
				printf("\n");
			}

		{
			//Cb
			//org
			Pel uv_org[128][128];
			Pel uv_pred[128][128];
			Pel uv_resi[128][128];
			xxx = pcYuvOrg->getCbAddr(pcCU->getZorderIdxInCU());
			for (iii = 0; iii < uiHeight / 2; iii++)
			{
				for (jjj = 0; jjj < uiWidth / 2; jjj++)
				{
					uv_org[iii][jjj] = xxx[jjj];
				}
				xxx += pcYuvOrg->getCStride();
			}
			//Cb
			//pred
			xxx = pcYuvPred->getCbAddr(pcCU->getZorderIdxInCU());
			for (iii = 0; iii < uiHeight / 2; iii++)
			{
				for (jjj = 0; jjj < uiWidth / 2; jjj++)
				{
					uv_pred[iii][jjj] = xxx[jjj];
				}
				xxx += pcYuvPred->getCStride();
			}
			//Cb
			//resi
			xxx = rpcYuvResi->getCbAddr(pcCU->getZorderIdxInCU());
			for (iii = 0; iii < uiHeight / 2; iii++)
			{
				for (jjj = 0; jjj < uiWidth / 2; jjj++)
				{
					uv_resi[iii][jjj] = xxx[jjj];
				}
				xxx += rpcYuvResi->getCStride();
			}

			//Cr
			//org
			xxx = pcYuvOrg->getCrAddr(pcCU->getZorderIdxInCU());
			for (iii = 0; iii < uiHeight / 2; iii++)
			{
				for (jjj = 0; jjj < uiWidth / 2; jjj++)
				{
					uv_org[iii][jjj] = xxx[jjj];
				}
				xxx += pcYuvOrg->getCStride();
			}
			//Cr
			//pred
			xxx = pcYuvPred->getCrAddr(pcCU->getZorderIdxInCU());
			for (iii = 0; iii < uiHeight / 2; iii++)
			{
				for (jjj = 0; jjj < uiWidth / 2; jjj++)
				{
					uv_pred[iii][jjj] = xxx[jjj];
				}
				xxx += pcYuvPred->getCStride();
			}
			//Cr
			//resi
			xxx = rpcYuvResi->getCrAddr(pcCU->getZorderIdxInCU());
			for (iii = 0; iii < uiHeight / 2; iii++)
			{
				for (jjj = 0; jjj < uiWidth / 2; jjj++)
				{
					uv_resi[iii][jjj] = xxx[jjj];
				}
				xxx += rpcYuvResi->getCStride();
			}
			sign = 0;

		}
		}
#endif
		//m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[ pcCU->getDepth( 0 ) ][ CI_CURR_BEST ] );

		xEncodeInterTexture(pcCU, rpcYuvResi, uiTrMode);

		rpcYuvRec->addClip(pcYuvPred, rpcYuvResi, pcCU->getZorderIdxInCU(), uiCUSize);
#if wlq_avs2x_debug
		{
			int iii, jjj;
			Pel *xxx;
			sign = 0;
			if (sign)
			{
				printf("\n");
				printf("pred\n");
				xxx = pcYuvPred->getLumaAddr(pcCU->getZorderIdxInCU());
				for (iii = 0; iii < 8; iii++)
				{
					for (jjj = 0; jjj < 8; jjj++)
					{
						printf("%d\t", xxx[jjj]);
					}
					printf("\n");
					xxx += pcYuvPred->getStride();
				}
				printf("\n");

				printf("\n");
				printf("resi_rec\n");
				xxx = rpcYuvResi->getLumaAddr(pcCU->getZorderIdxInCU());
				for (iii = 0; iii < 8; iii++)
				{
					for (jjj = 0; jjj < 8; jjj++)
					{
						printf("%d\t", xxx[jjj]);
					}
					printf("\n");
					xxx += rpcYuvResi->getStride();
				}
				printf("\n");

			}
			sign = 0;
		}
#endif
		// update with clipped distortion and cost
		uiDistortion = m_pcRdCost->getDistPart(rpcYuvRec->getLumaAddr(uiAbsZorderIdx), rpcYuvRec->getStride(), pcYuvOrg->getLumaAddr(uiAbsZorderIdx), pcYuvOrg->getStride(), uiWidth, uiHeight)
			+ (UInt)(m_pcRdCost->getChromaWeight()*(Double)(m_pcRdCost->getDistPart(rpcYuvRec->getCbAddr(uiAbsZorderIdx), rpcYuvRec->getCStride(), pcYuvOrg->getCbAddr(uiAbsZorderIdx), pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1)
			+ m_pcRdCost->getDistPart(rpcYuvRec->getCrAddr(uiAbsZorderIdx), rpcYuvRec->getCStride(), pcYuvOrg->getCrAddr(uiAbsZorderIdx), pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1)));

		uiBits = 0;
		m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_CURR_BEST]);
#if wlq_avs2x_debug
		g_sign = 0;
#endif
		xAddSymbolBitsInter(pcCU, uiBits, bSkipRes);
#if wlq_avs2x_debug
		g_sign = 0;
#endif
		dCost = m_pcRdCost->calcRdCost(uiBits, uiDistortion);
#if wlq_avs2x_debug
		{
			int iii, jjj;
			Pel *xxx = rpcYuvRec->getLumaAddr(uiAbsZorderIdx);
			qq = m_pcRdCost->getChromaWeight();
			ddy = m_pcRdCost->getDistPart(rpcYuvRec->getLumaAddr(uiAbsZorderIdx), rpcYuvRec->getStride(), pcYuvOrg->getLumaAddr(uiAbsZorderIdx), pcYuvOrg->getStride(), uiWidth, uiHeight);
			ddu = m_pcRdCost->getDistPart(rpcYuvRec->getCbAddr(uiAbsZorderIdx), rpcYuvRec->getCStride(), pcYuvOrg->getCbAddr(uiAbsZorderIdx), pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1);
			ddv = m_pcRdCost->getDistPart(rpcYuvRec->getCrAddr(uiAbsZorderIdx), rpcYuvRec->getCStride(), pcYuvOrg->getCrAddr(uiAbsZorderIdx), pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1);
			sign = 0;
			if (sign)
			{
				printf("\n");
				printf("rec\n");
				for (iii = 0; iii < 8; iii++)
				{
					for (jjj = 0; jjj < 8; jjj++)
					{
						printf("%d\t", xxx[jjj]);
					}
					printf("\n");
					xxx += rpcYuvRec->getStride();
				}
				printf("\n");
			}
			sign = 0;
		}
#endif
#if print_rd
		printf("CTU_addr = %d\t", pcCU->getAddr());
		printf("Type=%d\t", (pcCU->getPredictionMode(0) == MODE_DIRECT || pcCU->getPredictionMode(0) == MODE_SKIP) ? 0 : pcCU->getPartitionSize(0) + 1);
		printf("Size=%d\t", g_uiLog2MaxCUSize - pcCU->getDepth(0));
		printf("rate_all=%d\t", uiBits);
		printf("distortion_all=%d\t", uiDistortion);
		printf("rdcost=%f\t\n", dCost);
#endif

		if (dCost < dCostBest)
		{
			uiBitsBest = uiBits;
			uiDistortionBest = uiDistortion;
			dCostBest = dCost;
			uiBestTrMode = uiTrMode;
			m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_TEMP_BEST]);
		}
	}


	rpcYuvResi->subtract(pcYuvOrg, pcYuvPred, uiAbsZorderIdx, uiCUSize);
	m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_CURR_BEST]);
	xEncodeInterTexture(pcCU, rpcYuvResi, uiBestTrMode);

#if WLQ_inter_bug
	m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_TEMP_BEST]);
#endif
	rpcYuvRec->addClip(pcYuvPred, rpcYuvResi, uiAbsZorderIdx, uiCUSize);

	pcCU->getTotalBits() = uiBitsBest;
	pcCU->getTotalDistortion() = uiDistortionBest;
	pcCU->getTotalCost() = dCostBest;
	pcCU->setTrIdxSubParts(uiBestTrMode, 0, pcCU->getDepth(0));

	



//  rpcYuvResi->subtract( pcYuvOrg, pcYuvPred, uiAbsZorderIdx, uiCUSize );
//  m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[ pcCU->getDepth( 0 ) ][ CI_CURR_BEST ] );
//  xEncodeInterTexture( pcCU, rpcYuvResi, uiBestTrMode );
//#if WLQ_inter_bug
//  m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_TEMP_BEST]);
//#endif
//  rpcYuvRec->addClip ( pcYuvPred,                 rpcYuvResi, uiAbsZorderIdx,      uiCUSize  );
//
//  pcCU->getTotalBits()       = uiBitsBest;
//  pcCU->getTotalDistortion() = uiDistortionBest;
//  pcCU->getTotalCost()       = dCostBest;
//  pcCU->setTrIdxSubParts( uiBestTrMode, 0, pcCU->getDepth(0) );
}

#if RD_DIRECT
Void TEncSearch::encodeResAndCalcRdInterCUDirect(TComDataCU* pcCU, TComYuv* pcYuvOrg, TComYuv* pcYuvPred, TComYuv*& rpcYuvResi, TComYuv*& rpcYuvResiBest, TComYuv*& rpcYuvRec)
{
	assert(!pcCU->isIntra(0));

	Bool    bHighPass = pcCU->getPicture()->getDepth() ? true : false;
	UInt    uiBits = 0, uiBitsBest = 0;
	UInt    uiDistortion = 0, uiDistortionBest = 0;

	UInt      uiWidth = pcCU->getWidth(0);
	UInt      uiHeight = pcCU->getHeight(0);

	UInt    uiCUSize = 1 << pcCU->getLog2CUSize(0);
	Double  dCost;

	UInt uiAbsZorderIdx = pcCU->getZorderIdxInCU();

	// Direct CU -------------------------------------------------------------------------------------
	UInt uiTrMode, uiBestTrMode = 0;
	Double dCostBest = MAX_DOUBLE;
	for (uiTrMode = 0; uiTrMode <= 1; uiTrMode++)
	{
		if (uiTrMode > 0 && pcCU->getPartitionSize(0) >= SIZE_2NxnU && pcCU->getPartitionSize(0) <= SIZE_nRx2N)
		{
			uiTrMode = 2;
		}

		pcCU->setTrIdxSubParts(uiTrMode, 0, pcCU->getDepth(0));

		::memset(pcCU->getCoeffY(), 0, sizeof(TCoeff) * uiWidth * uiHeight);

		rpcYuvResi->subtract(pcYuvOrg, pcYuvPred, uiAbsZorderIdx, uiCUSize);

		m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_CURR_BEST]);

		xEncodeInterTexture(pcCU, rpcYuvResi, uiTrMode);

		rpcYuvRec->addClip(pcYuvPred, rpcYuvResi, pcCU->getZorderIdxInCU(), uiCUSize);

		// update with clipped distortion and cost
		uiDistortion = m_pcRdCost->getDistPart(rpcYuvRec->getLumaAddr(uiAbsZorderIdx), rpcYuvRec->getStride(), pcYuvOrg->getLumaAddr(uiAbsZorderIdx), pcYuvOrg->getStride(), uiWidth, uiHeight)
			+ (UInt)(m_pcRdCost->getChromaWeight()*(Double)(m_pcRdCost->getDistPart(rpcYuvRec->getCbAddr(uiAbsZorderIdx), rpcYuvRec->getCStride(), pcYuvOrg->getCbAddr(uiAbsZorderIdx), pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1)
			+ m_pcRdCost->getDistPart(rpcYuvRec->getCrAddr(uiAbsZorderIdx), rpcYuvRec->getCStride(), pcYuvOrg->getCrAddr(uiAbsZorderIdx), pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1)));

		uiBits = 0;
		m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_CURR_BEST]);

		m_pcEntropyCoder->resetBits();
		m_pcEntropyCoder->encodeInterCUTypeIndex(pcCU, 0, false);
		m_pcEntropyCoder->encodePredInfo(pcCU, 0, false);
		m_pcEntropyCoder->encodeCoeff(pcCU, 0, pcCU->getDepth(0), pcCU->getLog2CUSize(0));
		m_pcEntropyCoder->encodeDBKIdx(pcCU, 0);

		uiBits = m_pcEntropyCoder->getNumberOfWrittenBits();

		dCost = m_pcRdCost->calcRdCost(uiBits, uiDistortion);

		if (dCost < dCostBest)
		{
			uiBitsBest = uiBits;
			uiDistortionBest = uiDistortion;
			dCostBest = dCost;
			uiBestTrMode = uiTrMode;
			m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_TEMP_BEST]);
		}
	}

	rpcYuvResi->subtract(pcYuvOrg, pcYuvPred, uiAbsZorderIdx, uiCUSize);
	m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_CURR_BEST]);
	xEncodeInterTexture(pcCU, rpcYuvResi, uiBestTrMode);

	rpcYuvRec->addClip(pcYuvPred, rpcYuvResi, uiAbsZorderIdx, uiCUSize);

	pcCU->getTotalBits() = uiBitsBest;
	pcCU->getTotalDistortion() = uiDistortionBest;
	pcCU->getTotalCost() = dCostBest;

	pcCU->setTrIdxSubParts(uiBestTrMode, 0, pcCU->getDepth(0));
}
#endif

Void TEncSearch::xRecurTransformNxN( TComDataCU* rpcCU, UInt uiAbsPartIdx, Pel* pcResidual, UInt uiAddr, UInt uiStride, UInt uiLog2Size, UInt uiMaxTrMode, UInt uiTrMode, TCoeff*& rpcCoeff, TextType eType )
{
  if ( uiTrMode == uiMaxTrMode )
  {
    UInt uiAbsSum;
    UInt uiCoeffOffset = 1 << ( uiLog2Size << 1 );

    if (m_pcEncCfg->getUseRDOQ())
      m_pcEntropyCoder->estimateBit(m_pcTrQuant->m_pcEstBitsSbac, (1<<uiLog2Size), eType );

#if ZHOULULU_QT
#if RD_NSQT
	//test AbsPartIdx -> 0
	m_pcTrQuant->setBlockSize(rpcCU, uiLog2Size, eType, uiAbsPartIdx, uiTrMode);
#else
	m_pcTrQuant->setBlockSize( rpcCU, uiLog2Size, eType, uiAbsPartIdx );
#endif
	m_pcTrQuant->transformMxN( rpcCU, pcResidual + uiAddr, uiStride, rpcCoeff, uiLog2Size, uiAbsSum, eType, uiAbsPartIdx );
#else
     // ACS
    m_pcTrQuant->transformNxN( rpcCU, pcResidual + uiAddr, uiStride, rpcCoeff, uiLog2Size, uiAbsSum, eType, uiAbsPartIdx );
#endif
	//test
	//printf("Addr=%d AbsSum=%d Size=%d YUV=%d offset=%d\n", rpcCU->getAddr(), uiAbsSum!=0, uiLog2Size, eType == TEXT_LUMA, uiAbsPartIdx);
    if ( uiAbsSum )     //rd认为 uiAbsSum>必进行量化后系数的编码
    {
      UInt uiBits, uiDist, uiDistCC;
      Double fCost, fCostCC;

      Pel* pcResidualRec = m_pTempPel;
	   m_pcQTTempCoeffY1= m_pcQTTempCoeffY;
      m_pcEntropyCoder->resetBits();
      m_pcEntropyCoder->encodeCoeffNxN( rpcCU, rpcCoeff, uiAbsPartIdx, uiLog2Size, rpcCU->getDepth( 0 ) + uiTrMode, eType, true );
      uiBits = m_pcEntropyCoder->getNumberOfWrittenBits();      
#if ZHOULULU_QT
#if RD_NSQT
	  m_pcTrQuant->setBlockSize(rpcCU, uiLog2Size, eType, uiAbsPartIdx, uiTrMode);
#else
	  m_pcTrQuant->setBlockSize( rpcCU, uiLog2Size, eType, uiAbsPartIdx );
#endif
	  m_pcTrQuant->invtransformMxN( rpcCU, pcResidualRec, uiStride, rpcCoeff, uiLog2Size, eType, uiAbsPartIdx );
#else
      m_pcTrQuant->invtransformNxN( pcResidualRec, uiStride, rpcCoeff, uiLog2Size, eType );
#endif
#if !INTERTEST
      //-- distortion when the coefficients are not cleared
      uiDist   = m_pcRdCost->getDistPart(pcResidualRec, uiStride, pcResidual+uiAddr, uiStride, (1<<uiLog2Size), (1<<uiLog2Size));

      //-- distortion when the coefficients are cleared
      memset(pcResidualRec, 0, sizeof(Pel)*(1<<uiLog2Size)*uiStride);
      uiDistCC = m_pcRdCost->getDistPart(pcResidualRec, uiStride, pcResidual+uiAddr, uiStride, (1<<uiLog2Size), (1<<uiLog2Size));

      fCost   = m_pcRdCost->calcRdCost(uiBits, uiDist);
      fCostCC = m_pcRdCost->calcRdCost(0, uiDistCC);
      if ( fCostCC < fCost )
      {
        uiAbsSum = 0;
        memset(rpcCoeff, 0, sizeof(TCoeff)*uiCoeffOffset);
        rpcCU->setCbfSubParts( 0x00, eType, uiAbsPartIdx, rpcCU->getDepth( 0 ) + uiTrMode );
      }
#endif
    }
    else
    {
      rpcCU->setCbfSubParts( 0x00, eType, uiAbsPartIdx, rpcCU->getDepth( 0 ) + uiTrMode );
      memset(rpcCoeff, 0, sizeof(TCoeff)*uiCoeffOffset);
    }

    rpcCoeff += uiCoeffOffset;
  }
  else
  {
#if RD_NSQT
	  uiTrMode++;
#if 0//RD_NSQT_BUG_YQH
	  UInt uiQPartNum = rpcCU->getTotalNumPart() >> ((rpcCU->getDepth(0) + uiTrMode) << 1);
#else
	  UInt uiQPartNum = rpcCU->getPic()->getNumPartInCU() >> ((rpcCU->getDepth(0) + uiTrMode) << 1);
#endif
	  UInt uiAddrOffset = (1 << (uiLog2Size - 1)) * uiStride;
	  //UInt uiCoefOffset = 1 << ((uiLog2Size - 1) << 1);
	  if (rpcCU->getPicture()->getSPS()->getNonsquareQuadtreeTansformEnableFlag())
	  {
		  PartSize ePartSize = rpcCU->getPartitionSize(uiAbsPartIdx);
		  switch (ePartSize)
		  {
		  case SIZE_2Nx2N:
		  case SIZE_NxN:
			  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
			  uiAbsPartIdx += uiQPartNum;
			  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + (1 << (uiLog2Size - 1)), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
			  uiAbsPartIdx += uiQPartNum;
			  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + uiAddrOffset, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
			  uiAbsPartIdx += uiQPartNum;
			  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + uiAddrOffset + (1 << (uiLog2Size - 1)), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
			 
			  break;
		  case SIZE_2NxN:
		  case SIZE_2NxnU:
		  case SIZE_2NxnD:
		  case SIZE_2NxhN:
#if RD_NSQT_BUG_YQH
			  if (uiLog2Size > 3)
			  {
				  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
				  //uiAbsPartIdx += uiQPartNum >> 1;
				   uiAbsPartIdx += uiQPartNum;
				  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + (uiAddrOffset >> 1), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
				 // uiAbsPartIdx += uiQPartNum + (uiQPartNum >> 1);
				    uiAbsPartIdx += uiQPartNum;
				  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + uiAddrOffset, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
				 // uiAbsPartIdx += uiQPartNum >> 1;
				  uiAbsPartIdx += uiQPartNum;
				  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + uiAddrOffset + (uiAddrOffset >> 1), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);

			  }
	  else
			  {
				  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
				  uiAbsPartIdx += uiQPartNum;
				  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + (1 << (uiLog2Size - 1)), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
				  uiAbsPartIdx += uiQPartNum;
				  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + uiAddrOffset, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
				  uiAbsPartIdx += uiQPartNum;
				  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + uiAddrOffset + (1 << (uiLog2Size - 1)), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
			  }
#else
			 
			  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
			  uiAbsPartIdx += uiQPartNum >> 1;
			  //uiAbsPartIdx += uiQPartNum;
			  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + (uiAddrOffset >> 1), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
			  uiAbsPartIdx += uiQPartNum + (uiQPartNum >> 1);
			  //uiAbsPartIdx += uiQPartNum;
			  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + uiAddrOffset, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
			  uiAbsPartIdx += uiQPartNum >> 1;
			  //uiAbsPartIdx += uiQPartNum;
			  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + uiAddrOffset + (uiAddrOffset >> 1), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
#endif 
			  break;
		  case SIZE_Nx2N:
		  case SIZE_nLx2N:
		  case SIZE_nRx2N:
		  case SIZE_hNx2N:
#if RD_NSQT_BUG_YQH
			  if (uiLog2Size>3)
			  {
				  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
				 // uiAbsPartIdx += uiQPartNum >> 2;
				  uiAbsPartIdx += uiQPartNum;
				  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + ((1 << (uiLog2Size - 1)) >> 1), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
				 // uiAbsPartIdx += (uiQPartNum >> 2) + (uiQPartNum >> 1);
				  uiAbsPartIdx += uiQPartNum;
				  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + (1 << (uiLog2Size - 1)), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
				 // uiAbsPartIdx += uiQPartNum >> 2;
				  uiAbsPartIdx += uiQPartNum;
				  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + (1 << (uiLog2Size - 1)) + ((1 << (uiLog2Size - 1)) >> 1), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
			  }
			  else
			  {
				  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
				  uiAbsPartIdx += uiQPartNum;
				  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + (1 << (uiLog2Size - 1)), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
				  uiAbsPartIdx += uiQPartNum;
				  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + uiAddrOffset, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
				  uiAbsPartIdx += uiQPartNum;
				  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + uiAddrOffset + (1 << (uiLog2Size - 1)), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
			  }
#else
			  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
			  uiAbsPartIdx += uiQPartNum >> 2;
			  //uiAbsPartIdx += uiQPartNum;
			  //rpcCoeff += uiCoefOffset >> 2;
			  //rpcCoeff += uiCoefOffset;
			  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + ((1 << (uiLog2Size - 1)) >> 1), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
			  uiAbsPartIdx += (uiQPartNum >> 2) + (uiQPartNum >> 1);
			  //uiAbsPartIdx += uiQPartNum;
			  //rpcCoeff += (uiCoefOffset >> 2) + (uiCoefOffset >> 1);
			  //rpcCoeff += uiCoefOffset;
			  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + (1 << (uiLog2Size - 1)), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
			  uiAbsPartIdx += uiQPartNum >> 2;
			  //uiAbsPartIdx += uiQPartNum;
			  //rpcCoeff += uiCoefOffset >> 2;
			  //rpcCoeff += uiCoefOffset;
			  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + (1 << (uiLog2Size - 1)) + ((1 << (uiLog2Size - 1)) >> 1), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);

#endif	
			  break;

		  default:
			  break;
		  }
	  }
	  else
	  {
		  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
		  uiAbsPartIdx += uiQPartNum;
		  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + (1 << (uiLog2Size - 1)), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
		  uiAbsPartIdx += uiQPartNum;
		  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + uiAddrOffset, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
		  uiAbsPartIdx += uiQPartNum;
		  xRecurTransformNxN(rpcCU, uiAbsPartIdx, pcResidual, uiAddr + uiAddrOffset + (1 << (uiLog2Size - 1)), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff, eType);
	  }
#else
    uiTrMode++;
    UInt uiQPartNum = rpcCU->getPic()->getNumPartInCU() >> ( ( rpcCU->getDepth(0)+uiTrMode ) << 1 );
    UInt uiAddrOffset = (1<<(uiLog2Size-1)) * uiStride;
    xRecurTransformNxN( rpcCU, uiAbsPartIdx, pcResidual, uiAddr                                     , uiStride, uiLog2Size-1, uiMaxTrMode, uiTrMode, rpcCoeff, eType );
    uiAbsPartIdx += uiQPartNum;
    xRecurTransformNxN( rpcCU, uiAbsPartIdx, pcResidual, uiAddr + (1<<(uiLog2Size-1))               , uiStride, uiLog2Size-1, uiMaxTrMode, uiTrMode, rpcCoeff, eType );
    uiAbsPartIdx += uiQPartNum;
    xRecurTransformNxN( rpcCU, uiAbsPartIdx, pcResidual, uiAddr + uiAddrOffset                      , uiStride, uiLog2Size-1, uiMaxTrMode, uiTrMode, rpcCoeff, eType );
    uiAbsPartIdx += uiQPartNum;
    xRecurTransformNxN( rpcCU, uiAbsPartIdx, pcResidual, uiAddr + uiAddrOffset + (1<<(uiLog2Size-1)), uiStride, uiLog2Size-1, uiMaxTrMode, uiTrMode, rpcCoeff, eType );
#endif
  }
}

Void TEncSearch::xEncodeInterTexture ( TComDataCU*& rpcCU, TComYuv*& rpcYuv, UInt uiTrMode )
{
  UInt    uiWidth, uiHeight, uiCWidth, uiCHeight, uiLumaTrMode, uiChromaTrMode;
  TCoeff* piCoeff = rpcCU->getCoeffY();
  Pel*    pResi;
  UInt    uiLog2Size = rpcCU->getLog2CUSize( 0 );
  UInt    uiLog2SizeC = rpcCU->getLog2CUSize( 0 ) - 1;
  UInt    uiAbsZorderIdx = rpcCU->getZorderIdxInCU();

  uiWidth    = rpcCU->getWidth ( 0 );
  uiHeight   = rpcCU->getHeight( 0 );  
  uiCWidth   = uiWidth >>1;
  uiCHeight  = uiHeight>>1;

  uiLumaTrMode = uiTrMode;
  (uiLog2SizeC - uiTrMode >= 2) ? uiChromaTrMode = uiTrMode : uiChromaTrMode = uiLog2SizeC - 2;
 
  m_pcTrQuant->setQPforQuant( TEXT_LUMA );

  rpcCU->clearCbf(0, TEXT_LUMA,     rpcCU->getTotalNumPart());
  rpcCU->clearCbf(0, TEXT_CHROMA_U, rpcCU->getTotalNumPart());
  rpcCU->clearCbf(0, TEXT_CHROMA_V, rpcCU->getTotalNumPart());

  // Luma   Y
  piCoeff = rpcCU->getCoeffY();

  xRecurTransformNxN( rpcCU, 0, rpcYuv->getLumaAddr(uiAbsZorderIdx), 0, rpcYuv->getStride(), uiLog2Size, uiLumaTrMode, 0, piCoeff, TEXT_LUMA );
  rpcCU->setCuCbfLuma( 0, uiLumaTrMode );
  piCoeff = rpcCU->getCoeffY(); pResi = rpcYuv->getLumaAddr(uiAbsZorderIdx);

  rpcYuv->clearY(uiAbsZorderIdx, uiWidth);
  m_pcTrQuant->invRecurTransformNxN( rpcCU, 0, TEXT_LUMA, pResi, 0, rpcYuv->getStride(), uiLog2Size, uiLumaTrMode, 0, piCoeff );

  // Chroma
  m_pcTrQuant->setQPforQuant( TEXT_CHROMA );
#if WLQ_inter_Chroma_TU
  // Cb
  piCoeff = rpcCU->getCoeffCb();

  xRecurTransformNxN( rpcCU, 0, rpcYuv->getCbAddr(uiAbsZorderIdx), 0, rpcYuv->getCStride(), uiLog2Size-1, 0, 0, piCoeff, TEXT_CHROMA_U );
  rpcCU->setCuCbfChromaUV( 0, 0, TEXT_CHROMA_U );
  piCoeff = rpcCU->getCoeffCb(); pResi = rpcYuv->getCbAddr(uiAbsZorderIdx);

  rpcYuv->clearU(uiAbsZorderIdx, uiCWidth);
  m_pcTrQuant->invRecurTransformNxN( rpcCU, 0, TEXT_CHROMA_U, pResi, 0, rpcYuv->getCStride(), uiLog2Size-1, 0, 0, piCoeff );

  // Cr
  piCoeff = rpcCU->getCoeffCr();

  xRecurTransformNxN( rpcCU, 0, rpcYuv->getCrAddr(uiAbsZorderIdx), 0, rpcYuv->getCStride(), uiLog2Size-1, 0, 0, piCoeff, TEXT_CHROMA_V );

  rpcCU->setCuCbfChromaUV( 0, 0, TEXT_CHROMA_V );
  piCoeff = rpcCU->getCoeffCr(); pResi = rpcYuv->getCrAddr(uiAbsZorderIdx);

  rpcYuv->clearV(uiAbsZorderIdx, uiCWidth);
  m_pcTrQuant->invRecurTransformNxN( rpcCU, 0, TEXT_CHROMA_V, pResi, 0, rpcYuv->getCStride(), uiLog2Size-1, 0, 0, piCoeff );

#else
  // Cb
  piCoeff = rpcCU->getCoeffCb();

  xRecurTransformNxN( rpcCU, 0, rpcYuv->getCbAddr(uiAbsZorderIdx), 0, rpcYuv->getCStride(), uiLog2Size-1, uiChromaTrMode, 0, piCoeff, TEXT_CHROMA_U );
  rpcCU->setCuCbfChromaUV( 0, uiChromaTrMode, TEXT_CHROMA_U );
  piCoeff = rpcCU->getCoeffCb(); pResi = rpcYuv->getCbAddr(uiAbsZorderIdx);

  rpcYuv->clearU(uiAbsZorderIdx, uiCWidth);
  m_pcTrQuant->invRecurTransformNxN( rpcCU, 0, TEXT_CHROMA_U, pResi, 0, rpcYuv->getCStride(), uiLog2Size-1, uiChromaTrMode, 0, piCoeff );

  // Cr
  piCoeff = rpcCU->getCoeffCr();

  xRecurTransformNxN( rpcCU, 0, rpcYuv->getCrAddr(uiAbsZorderIdx), 0, rpcYuv->getCStride(), uiLog2Size-1, uiChromaTrMode, 0, piCoeff, TEXT_CHROMA_V );

  rpcCU->setCuCbfChromaUV( 0, uiChromaTrMode, TEXT_CHROMA_V );
  piCoeff = rpcCU->getCoeffCr(); pResi = rpcYuv->getCrAddr(uiAbsZorderIdx);

  rpcYuv->clearV(uiAbsZorderIdx, uiCWidth);
  m_pcTrQuant->invRecurTransformNxN( rpcCU, 0, TEXT_CHROMA_V, pResi, 0, rpcYuv->getCStride(), uiLog2Size-1, uiChromaTrMode, 0, piCoeff );
#endif
}

#if ZHANGYI_INTRA
Void  TEncSearch::xAddSymbolBitsIntra(ComponentID compID, TComDataCU* pcCU, UInt uiPU, UInt uiQNumPart, UInt uiPartDepth, UInt uiMaxTrDepth, UInt uiTrDepth, UInt uiLog2Size, UInt& ruiBits, Int iIntraDirValue )
#else
Void  TEncSearch::xAddSymbolBitsIntra( TComDataCU* pcCU, UInt uiPU, UInt uiQNumPart, UInt uiPartDepth, UInt uiMaxTrDepth, UInt uiTrDepth, UInt uiLog2Size, UInt& ruiBits )
#endif
{
  UInt uiPartOffset = uiPU*uiQNumPart;

  UInt uiAbsZorderIdx    = pcCU     ->getZorderIdxInCU();

#if niu_coeff
  UInt uiMinCoeffSize = 1 << (pcCU->getPic()->getLog2MinCUSize() << 1);
  UInt uiLumaCoeffOffset = uiMinCoeffSize * (uiPartOffset);
  TCoeff* pcCoeffY = pcCU->getCoeffY() + uiLumaCoeffOffset;
#else
  TCoeff* pcCoeffY          = pcCU     ->getCoeffY();
#endif
  TCoeff* pcCoeffCb         = pcCU     ->getCoeffCb ();
  TCoeff* pcCoeffCr         = pcCU     ->getCoeffCr ();
#if YQH_AVS3_DIR_RDO_BUG
  m_pcEntropyCoder->resetBits();
  ruiBits = m_pcEntropyCoder->getNumberOfWrittenBits();
#else
  m_pcEntropyCoder->resetBits();
#endif
#if ZHANGYI_INTRA
  if (compID == COMPONENT_Y)
  {
#if !niu_write_cbf
	  m_pcEntropyCoder->encodeCbf( pcCU, uiPartOffset, TEXT_LUMA,     uiTrDepth );
#endif
  } else if (compID == COMPONENT_Cb)
  {
#if !niu_write_cbf
	  m_pcEntropyCoder->encodeCbf( pcCU, uiPartOffset, TEXT_CHROMA_U, uiTrDepth );
#endif
  } else if (compID == COMPONENT_Cr)
  {
#if !niu_write_cbf
	  m_pcEntropyCoder->encodeCbf( pcCU, uiPartOffset, TEXT_CHROMA_V, uiTrDepth );
#endif
  }
#else
  m_pcEntropyCoder->encodeCbf( pcCU, uiPartOffset, TEXT_LUMA,     uiTrDepth );
  m_pcEntropyCoder->encodeCbf( pcCU, uiPartOffset, TEXT_CHROMA_U, uiTrDepth );
  m_pcEntropyCoder->encodeCbf( pcCU, uiPartOffset, TEXT_CHROMA_V, uiTrDepth );
#endif

  //ruiBits = m_pcEntropyCoder->getNumberOfWrittenBits();

  UInt uiLog2SizeC = uiLog2Size - 1;
  UInt uiTrIdx = uiTrDepth;
  UInt uiLumaTrMode = uiTrIdx;
  UInt uiChromaTrMode;
  (uiLog2SizeC - uiTrIdx >= 2) ? uiChromaTrMode = uiTrIdx : uiChromaTrMode = uiLog2SizeC - 2;

#if ZHANGYI_INTRA
#if YQH_AVS3_DIR_RDO_BUG
  if (compID != COMPONENT_Y)
	  m_pcEntropyCoder->encodePartSize(pcCU, 0);//for intra currenct
#endif

  if (compID == COMPONENT_Y)
  {
	  m_pcEntropyCoder->encodeIntraDirForRDO( iIntraDirValue );
#if ZY_INTRA_2X2MODIFY
	  m_pcEntropyCoder->encodeCoeff( pcCU, pcCoeffY,  uiPartOffset, pcCU->getDepth(0)+uiPartDepth, uiLog2Size,  uiMaxTrDepth,uiLumaTrMode, TEXT_LUMA );
#else
	  m_pcEntropyCoder->encodeCoeff( pcCU, pcCoeffY,  uiPartOffset, pcCU->getDepth(0)+uiPartDepth, uiLog2Size,  uiLumaTrMode,   0, TEXT_LUMA );
#endif
  } else if (compID == COMPONENT_Cb)
  {
	  m_pcEntropyCoder->encodeIntraDirCbForRDO(pcCU, uiPartOffset, iIntraDirValue );
	  m_pcEntropyCoder->encodeCoeff( pcCU, pcCoeffCb, uiPartOffset, pcCU->getDepth(0)+uiPartDepth, uiLog2SizeC, uiChromaTrMode, 0, TEXT_CHROMA_U );
  } else if (compID == COMPONENT_Cr)
  {
#if !ZHANGYI_INTRA_MODIFY
	  m_pcEntropyCoder->encodeIntraDirCrForRDO(pcCU, uiPartOffset, iIntraDirValue ); //色度块的模式只需要写一个，在Cb进来的时候已经写了，这里就不需要写了
#endif
	  m_pcEntropyCoder->encodeCoeff( pcCU, pcCoeffCr, uiPartOffset, pcCU->getDepth(0)+uiPartDepth, uiLog2SizeC, uiChromaTrMode, 0, TEXT_CHROMA_V );
  }
#else
  m_pcEntropyCoder->encodeCoeff( pcCU, pcCoeffY,  uiPartOffset, pcCU->getDepth(0)+uiPartDepth, uiLog2Size,  uiLumaTrMode,   0, TEXT_LUMA );
  m_pcEntropyCoder->encodeCoeff( pcCU, pcCoeffCb, uiPartOffset, pcCU->getDepth(0)+uiPartDepth, uiLog2SizeC, uiChromaTrMode, 0, TEXT_CHROMA_U );
  m_pcEntropyCoder->encodeCoeff( pcCU, pcCoeffCr, uiPartOffset, pcCU->getDepth(0)+uiPartDepth, uiLog2SizeC, uiChromaTrMode, 0, TEXT_CHROMA_V );
#endif
#if YQH_AVS3_DIR_RDO_BUG

  ruiBits = m_pcEntropyCoder->getNumberOfWrittenBits() - ruiBits;
#else
  ruiBits = m_pcEntropyCoder->getNumberOfWrittenBits();
#endif
}

#if niu_ChromaRDO_revise
Void TEncSearch::xAddSymbolBitsIntra_coeff(ComponentID compID, TComDataCU* pcCU, UInt uiPU, UInt uiQNumPart, UInt uiPartDepth, UInt uiMaxTrDepth, UInt uiTrDepth, UInt uiLog2Size, UInt& ruiBits, Int iIntraDirValue)
{
	UInt    uiNumPU = pcCU->getNumPartInter();
	UInt    uiPPartDepth = pcCU->getPartitionSize(0) == SIZE_2Nx2N ? 0 : 1;
	UInt    uiQNumParts = pcCU->getTotalNumPart() >> (uiPPartDepth << 1);

	UInt uiPartOffset = 0, uiMinCoeffSize, uiLumaCoeffOffset;
	TCoeff* pcCoeffY;
	TCoeff* pcCoeffCb = pcCU->getCoeffCb();
	TCoeff* pcCoeffCr = pcCU->getCoeffCr();

#if YQH_AVS3_DIR_RDO_BUG
	m_pcEntropyCoder->resetBits();
	ruiBits = m_pcEntropyCoder->getNumberOfWrittenBits();
#else
	m_pcEntropyCoder->resetBits();
#endif

	UInt uiLog2SizeC = uiLog2Size - 1;
	UInt uiTrIdx = uiTrDepth;
	UInt uiLumaTrMode = uiTrIdx;
	UInt uiChromaTrMode;
	(uiLog2SizeC - uiTrIdx >= 2) ? uiChromaTrMode = uiTrIdx : uiChromaTrMode = uiLog2SizeC - 2;

	m_pcEntropyCoder->encodePartSize(pcCU, 0);//for intra currenct

	for (Int m = 0; m < uiNumPU; m++)
	{
		m_pcEntropyCoder->encodeIntraDirForRDO(MProbableMode[m]);
	}

	m_pcEntropyCoder->encodeIntraDirCbForRDO(pcCU, uiPartOffset, iIntraDirValue);

	UInt a = m_pcEntropyCoder->getNumberOfWrittenBits() - ruiBits;
#if niu_write_cbf
	m_pcEntropyCoder->encodeCbfY(pcCU, uiPartOffset, TEXT_LUMA, 0, 0);
#else
	m_pcEntropyCoder->encodeCbf(pcCU, uiPartOffset, TEXT_LUMA, 0, 0);
#endif
	UInt b = m_pcEntropyCoder->getNumberOfWrittenBits() - ruiBits;

	for (UInt PUn = 0; PUn < uiNumPU; PUn++)
	{
		uiPartOffset = PUn * uiQNumPart;
		uiMinCoeffSize = 1 << (pcCU->getPic()->getLog2MinCUSize() << 1);
		uiLumaCoeffOffset = uiMinCoeffSize * (uiPartOffset);
		pcCoeffY = pcCU->getCoeffY() + uiLumaCoeffOffset;

		m_pcEntropyCoder->encodeCoeff(pcCU, pcCoeffY, uiPartOffset, pcCU->getDepth(0) + uiPPartDepth, uiLog2Size - uiPPartDepth, uiMaxTrDepth, uiLumaTrMode, TEXT_LUMA);
	}

	m_pcEntropyCoder->encodeCoeff(pcCU, pcCoeffCb, 0, pcCU->getDepth(0) + uiPPartDepth, uiLog2SizeC, uiChromaTrMode, 0, TEXT_CHROMA_U);
	m_pcEntropyCoder->encodeCoeff(pcCU, pcCoeffCr, 0, pcCU->getDepth(0) + uiPPartDepth, uiLog2SizeC, uiChromaTrMode, 0, TEXT_CHROMA_V);


#if YQH_AVS3_DIR_RDO_BUG
	ruiBits = m_pcEntropyCoder->getNumberOfWrittenBits() - ruiBits + (b - a) / 2 + (b - a) / 2 - (b - a);
#else
	ruiBits = m_pcEntropyCoder->getNumberOfWrittenBits();
#endif

}
#endif

Void  TEncSearch::xAddSymbolBitsInter( TComDataCU* pcCU, UInt& ruiBits, Bool bSkipRes )
{
//  if ( bSkipRes )
	if (0)
  {
    return;
  }
  else
  {
    m_pcEntropyCoder->resetBits();  
#if WLQ_BITS_COUNTER_INTER
	ruiBits = m_pcEntropyCoder->getNumberOfWrittenBits();
#endif
#if PU_ZY
   // m_pcEntropyCoder->encodeSplitFlag(pcCU, 0, false);
		m_pcEntropyCoder->encodeInterCUTypeIndex(pcCU, 0, false);
#else
		m_pcEntropyCoder->encodePredMode(pcCU, 0, false);
		m_pcEntropyCoder->encodePartSize(pcCU, 0, false);
#endif
#if DEBUG_BIT
		printf("\n%dx%d\t", pcCU->getWidth(0), pcCU->getHeight(0));
		Int cutypeIndexBits = m_pcEntropyCoder->getNumberOfWrittenBits();
		printf("cutype=%d\t ", cutypeIndexBits);
#endif
// #if ZY_INTRA_CABAC
// 	m_pcEntropyCoder->encodeIntraPuTypeIndex(pcCU, 0, false);
// #endif
    m_pcEntropyCoder->encodePredInfo( pcCU, 0, false );
#if DEBUG_BIT
		Int predInfoBits = m_pcEntropyCoder->getNumberOfWrittenBits();
		printf("preInfoBit=%d\t", predInfoBits - cutypeIndexBits);
#endif

#if niu_intercbf_revise
		UInt a = m_pcEntropyCoder->getNumberOfWrittenBits() - ruiBits;
#if niu_write_cbf
		m_pcEntropyCoder->encodeCbfY(pcCU, 0, TEXT_LUMA, 0, 0);
#else
		m_pcEntropyCoder->encodeCbf(pcCU, 0, TEXT_LUMA, 0, 0);
#endif
		UInt b = m_pcEntropyCoder->getNumberOfWrittenBits() - ruiBits;
#endif

    m_pcEntropyCoder->encodeCoeff   ( pcCU, 0, pcCU->getDepth(0), pcCU->getLog2CUSize(0) );
#if DEBUG_BIT
		Int coeffBits = m_pcEntropyCoder->getNumberOfWrittenBits();
		printf("coefBit=%d\t", coeffBits - predInfoBits);
#endif
    m_pcEntropyCoder->encodeDBKIdx  ( pcCU, 0 );
#if WLQ_BITS_COUNTER_INTER

	#if niu_intercbf_revise
		ruiBits = m_pcEntropyCoder->getNumberOfWrittenBits() - ruiBits + (b-a)/2 + (b-a)/2 - (b-a);
	#else
		ruiBits = m_pcEntropyCoder->getNumberOfWrittenBits() - ruiBits;
	#endif
#else
    ruiBits = m_pcEntropyCoder->getNumberOfWrittenBits();
#endif
  }
}

#if DCTIF
Void TEncSearch::xExtDIFUpSamplingH(TComPattern* pattern, Bool biPred)
{
	Int width      = pattern->getROIYWidth();
	Int height     = pattern->getROIYHeight();
	Int srcStride  = pattern->getPatternLStride();

	Int intStride = m_filteredBlockTmp[0].getRealStride();
	Int dstStride = m_filteredBlock[0][0].getRealStride();
	Short *intPtr;
	Short *dstPtr;
	Int filterSize = NTAPS_LUMA;
	Int halfFilterSize = (filterSize>>1);
	Pel *srcPtr = pattern->getROIY() - halfFilterSize*srcStride - 1;

	m_if.filterHorLuma(srcPtr, srcStride, m_filteredBlockTmp[0].getLumaAddr(), intStride, width+1, height+filterSize, 0, false);
	m_if.filterHorLuma(srcPtr, srcStride, m_filteredBlockTmp[2].getLumaAddr(), intStride, width+1, height+filterSize, 2, false);

	intPtr = m_filteredBlockTmp[0].getLumaAddr() + halfFilterSize * intStride + 1;  
	dstPtr = m_filteredBlock[0][0].getLumaAddr();
	m_if.filterVerLuma(intPtr, intStride, dstPtr, dstStride, width+0, height+0, 0, false, true);

	intPtr = m_filteredBlockTmp[0].getLumaAddr() + (halfFilterSize-1) * intStride + 1;  
	dstPtr = m_filteredBlock[2][0].getLumaAddr();
	m_if.filterVerLuma(intPtr, intStride, dstPtr, dstStride, width+0, height+1, 2, false, true);

	intPtr = m_filteredBlockTmp[2].getLumaAddr() + halfFilterSize * intStride;
	dstPtr = m_filteredBlock[0][2].getLumaAddr();
	m_if.filterVerLuma(intPtr, intStride, dstPtr, dstStride, width+1, height+0, 0, false, true);

	intPtr = m_filteredBlockTmp[2].getLumaAddr() + (halfFilterSize-1) * intStride;
	dstPtr = m_filteredBlock[2][2].getLumaAddr();
	m_if.filterVerLuma(intPtr, intStride, dstPtr, dstStride, width+1, height+1, 2, false, true);
}

/**
* \brief Generate quarter-sample interpolated blocks
*
* \param pattern    Reference picture ROI
* \param halfPelRef Half-pel mv
* \param biPred     Flag indicating whether block is for biprediction
*/
Void TEncSearch::xExtDIFUpSamplingQ( TComPattern* pattern, TComMv halfPelRef, Bool biPred )
{
	Int width      = pattern->getROIYWidth();
	Int height     = pattern->getROIYHeight();
	Int srcStride  = pattern->getPatternLStride();

	Pel *srcPtr;
	Int intStride = m_filteredBlockTmp[0].getRealStride();
	Int dstStride = m_filteredBlock[0][0].getRealStride();
	Short *intPtr;
	Short *dstPtr;
	Int filterSize = NTAPS_LUMA;

	Int halfFilterSize = (filterSize>>1);

	Int extHeight = (halfPelRef.getVer() == 0) ? height + filterSize : height + filterSize-1;

	// Horizontal filter 1/4
	srcPtr = pattern->getROIY() - halfFilterSize * srcStride - 1;
	intPtr = m_filteredBlockTmp[1].getLumaAddr();
	if (halfPelRef.getVer() > 0)
	{
		srcPtr += srcStride;
	}
	if (halfPelRef.getHor() >= 0)
	{
		srcPtr += 1;
	}
	m_if.filterHorLuma(srcPtr, srcStride, intPtr, intStride, width, extHeight, 1, false);

	// Horizontal filter 3/4
	srcPtr = pattern->getROIY() - halfFilterSize*srcStride - 1;
	intPtr = m_filteredBlockTmp[3].getLumaAddr();
	if (halfPelRef.getVer() > 0)
	{
		srcPtr += srcStride;
	}
	if (halfPelRef.getHor() > 0)
	{
		srcPtr += 1;
	}
	m_if.filterHorLuma(srcPtr, srcStride, intPtr, intStride, width, extHeight, 3, false);        

	// Generate @ 1,1
	intPtr = m_filteredBlockTmp[1].getLumaAddr() + (halfFilterSize-1) * intStride;
	dstPtr = m_filteredBlock[1][1].getLumaAddr();
	if (halfPelRef.getVer() == 0)
	{
		intPtr += intStride;
	}
	m_if.filterVerLuma(intPtr, intStride, dstPtr, dstStride, width, height, 1, false, true);

	// Generate @ 3,1
	intPtr = m_filteredBlockTmp[1].getLumaAddr() + (halfFilterSize-1) * intStride;
	dstPtr = m_filteredBlock[3][1].getLumaAddr();
	m_if.filterVerLuma(intPtr, intStride, dstPtr, dstStride, width, height, 3, false, true);

	if (halfPelRef.getVer() != 0)
	{
		// Generate @ 2,1
		intPtr = m_filteredBlockTmp[1].getLumaAddr() + (halfFilterSize-1) * intStride;
		dstPtr = m_filteredBlock[2][1].getLumaAddr();
		if (halfPelRef.getVer() == 0)
		{
			intPtr += intStride;
		}
		m_if.filterVerLuma(intPtr, intStride, dstPtr, dstStride, width, height, 2, false, true);

		// Generate @ 2,3
		intPtr = m_filteredBlockTmp[3].getLumaAddr() + (halfFilterSize-1) * intStride;
		dstPtr = m_filteredBlock[2][3].getLumaAddr();
		if (halfPelRef.getVer() == 0)
		{
			intPtr += intStride;
		}
		m_if.filterVerLuma(intPtr, intStride, dstPtr, dstStride, width, height, 2, false, true);
	}
	else
	{
		// Generate @ 0,1
		intPtr = m_filteredBlockTmp[1].getLumaAddr() + halfFilterSize * intStride;
		dstPtr = m_filteredBlock[0][1].getLumaAddr();
		m_if.filterVerLuma(intPtr, intStride, dstPtr, dstStride, width, height, 0, false, true);

		// Generate @ 0,3
		intPtr = m_filteredBlockTmp[3].getLumaAddr() + halfFilterSize * intStride;
		dstPtr = m_filteredBlock[0][3].getLumaAddr();
		m_if.filterVerLuma(intPtr, intStride, dstPtr, dstStride, width, height, 0, false, true);
	}

	if (halfPelRef.getHor() != 0)
	{
		// Generate @ 1,2
		intPtr = m_filteredBlockTmp[2].getLumaAddr() + (halfFilterSize-1) * intStride;
		dstPtr = m_filteredBlock[1][2].getLumaAddr();
		if (halfPelRef.getHor() > 0)
		{
			intPtr += 1;
		}
		if (halfPelRef.getVer() >= 0)
		{
			intPtr += intStride;
		}
		m_if.filterVerLuma(intPtr, intStride, dstPtr, dstStride, width, height, 1, false, true);

		// Generate @ 3,2
		intPtr = m_filteredBlockTmp[2].getLumaAddr() + (halfFilterSize-1) * intStride;
		dstPtr = m_filteredBlock[3][2].getLumaAddr();
		if (halfPelRef.getHor() > 0)
		{
			intPtr += 1;
		}
		if (halfPelRef.getVer() > 0)
		{
			intPtr += intStride;
		}
		m_if.filterVerLuma(intPtr, intStride, dstPtr, dstStride, width, height, 3, false, true);  
	}
	else
	{
		// Generate @ 1,0
		intPtr = m_filteredBlockTmp[0].getLumaAddr() + (halfFilterSize-1) * intStride + 1;
		dstPtr = m_filteredBlock[1][0].getLumaAddr();
		if (halfPelRef.getVer() >= 0)
		{
			intPtr += intStride;
		}
		m_if.filterVerLuma(intPtr, intStride, dstPtr, dstStride, width, height, 1, false, true);

		// Generate @ 3,0
		intPtr = m_filteredBlockTmp[0].getLumaAddr() + (halfFilterSize-1) * intStride + 1;
		dstPtr = m_filteredBlock[3][0].getLumaAddr();
		if (halfPelRef.getVer() > 0)
		{
			intPtr += intStride;
		}
		m_if.filterVerLuma(intPtr, intStride, dstPtr, dstStride, width, height, 3, false, true);
	}

	// Generate @ 1,3
	intPtr = m_filteredBlockTmp[3].getLumaAddr() + (halfFilterSize-1) * intStride;
	dstPtr = m_filteredBlock[1][3].getLumaAddr();
	if (halfPelRef.getVer() == 0)
	{
		intPtr += intStride;
	}
	m_if.filterVerLuma(intPtr, intStride, dstPtr, dstStride, width, height, 1, false, true);

	// Generate @ 3,3
	intPtr = m_filteredBlockTmp[3].getLumaAddr() + (halfFilterSize-1) * intStride;
	dstPtr = m_filteredBlock[3][3].getLumaAddr();
	m_if.filterVerLuma(intPtr, intStride, dstPtr, dstStride, width, height, 3, false, true);
}
#else
Void TEncSearch::xExtDIFUpSamplingH ( TComPattern* pcPattern, TComYuv* pcYuvExt  )
{
  Int   x, y;
    
  Int   iWidth     = pcPattern->getROIYWidth();
  Int   iHeight    = pcPattern->getROIYHeight();
  
  Int   iPatStride  = pcPattern->getPatternLStride();
  Int   iExtStride  = pcYuvExt ->getStride();
  
  Int*  piSrcY;
  Int*  piDstY;
  Pel*  piDstYPel;
  Pel*  piSrcYPel;
  
  //  Copy integer-pel
  piSrcYPel = pcPattern->getROIY() - sc_iLumaHalfTaps - iPatStride;
  piDstY    = m_piYuvExt;//pcYuvExt->getLumaAddr();
  piDstYPel = pcYuvExt->getLumaAddr();
  for ( y = 0; y < iHeight + 2; y++ )
  {
    for ( x = 0; x < iWidth + sc_iLumaTaps; x++ )
    {
      piDstYPel[x << 2] = piSrcYPel[x];
    }
    piSrcYPel +=  iPatStride;
    piDstY    += (m_iYuvExtStride << 2);
    piDstYPel += (iExtStride      << 2);
  }
  
  //  Half-pel NORM. : vertical
  piSrcYPel = pcPattern->getROIY()    - iPatStride - sc_iLumaHalfTaps;
  piDstY    = m_piYuvExt              + (m_iYuvExtStride<<1);
  piDstYPel = pcYuvExt->getLumaAddr() + (iExtStride<<1);
  
  xCTI_FilterHalfVer
  ( piSrcYPel, iPatStride, 1,
    iWidth + sc_iLumaTaps, iHeight + 1,
    m_iYuvExtStride<<2, 4, piDstY,
    iExtStride<<2, piDstYPel);

  //  Half-pel interpolation : horizontal
  piSrcYPel = pcPattern->getROIY()   -  iPatStride - 1 - sc_iLumaLeftMargin;
  piDstYPel = pcYuvExt->getLumaAddr() + sc_iLumaTaps2 - 2;
  
  lumaFilter<0, Pel, Pel, 32, 6, 0, 255, 1, 4>( piSrcYPel, piDstYPel, iPatStride, iExtStride<<2, 1, iWidth + 1, iHeight + 1 );

  //  Half-pel interpolation : center
  piSrcY    = m_piYuvExt              + (m_iYuvExtStride<<1) + ((sc_iLumaHalfTaps-1) << 2) - (sc_iLumaLeftMargin*4);
  piDstYPel = pcYuvExt->getLumaAddr() + (iExtStride<<1)      + sc_iLumaTaps2 - 2;
  
  lumaFilter<0, Int, Pel, 2048, 12, 0, 255, 4, 4>( piSrcY, piDstYPel, m_iYuvExtStride<<2, iExtStride<<2, 4, iWidth + 1, iHeight + 1 );

}

Void TEncSearch::xExtDIFUpSamplingQ   ( TComPattern* pcPatternKey, Pel* piDst, Int iDstStride, Pel* piSrcPel, Int iSrcPelStride, Int* piSrc, Int iSrcStride, UInt uiFilter )
{
  Int   x, y;
    
  Int   iWidth     = pcPatternKey->getROIYWidth();
  Int   iHeight    = pcPatternKey->getROIYHeight();

  Int*  piSrcY;
  Int*  piDstY;
  Pel*  piDstYPel;
  Pel*  piSrcYPel;
  
  Int iSrcStride4 = (iSrcStride<<2);
  Int iDstStride4 = (iDstStride<<2);
  
  switch (uiFilter)
  {
    case 0:
    {
    //  Quater-pel interpolation : vertical
    piSrcYPel = piSrcPel - sc_iLumaHalfTaps + 1 - sc_iLumaLeftMargin * iSrcPelStride;
    piDstY = piSrc - sc_iLumaTaps2 + 2 - iSrcStride;

    lumaQuarterFilters4Estimation<Pel, Int, 0, 0, INT_MIN, INT_MAX, 1>( piSrcYPel, piDstY, iSrcPelStride,
        iSrcStride4, iSrcPelStride, iSrcPelStride*(sc_iLumaTaps - 1), iSrcStride, iWidth + sc_iLumaTaps - 1, iHeight );

    // Above three pixels
    piSrcY = piSrc - 2 - iSrcStride - sc_iLumaLeftMargin * 4;
    piDstYPel = piDst - 1 - iDstStride;

    lumaQuarterFilters4Estimation<Int, Pel, 2048, 12, 0, 255, 4>( piSrcY, piDstYPel, iSrcStride4,
        iDstStride4, 4,  4*(sc_iLumaTaps - 1), 1, iWidth, iHeight );

    piSrcY = piSrc - 2 - iSrcStride - sc_iLumaLeftMargin*4;
    piDstYPel = piDst - iDstStride;

    lumaFilter<0, Int, Pel, 2048, 12, 0, 255, 4, 4>( piSrcY, piDstYPel, iSrcStride4, iDstStride4, 4, iWidth, iHeight );

    // Middle two pixels
    piSrcY = piSrc - 2 - sc_iLumaLeftMargin * 4;
    piDstYPel = piDst - 1;

    lumaQuarterFilters4Estimation<Int, Pel, 2048, 12, 0, 255, 4>( piSrcY, piDstYPel, iSrcStride4,
        iDstStride4, 4, 4*(sc_iLumaTaps - 1), 1, iWidth, iHeight );

    // Below three pixels
    piSrcY = piSrc - 2 + iSrcStride - sc_iLumaLeftMargin * 4;
    piDstYPel = piDst - 1 + iDstStride;

    lumaQuarterFilters4Estimation<Int, Pel, 2048, 12, 0, 255, 4>( piSrcY, piDstYPel, iSrcStride4,
        iDstStride4, 4, 4*(sc_iLumaTaps - 1), 1, iWidth, iHeight );

    piSrcY = piSrc - 2 + iSrcStride - sc_iLumaLeftMargin*4;
    piDstYPel = piDst + iDstStride;

    lumaFilter<0, Int, Pel, 2048, 12, 0, 255, 4, 4>( piSrcY, piDstYPel, iSrcStride4, iDstStride4, 4, iWidth, iHeight );

    break;
  }
    case 1:
    {
    //  Quater-pel interpolation : vertical
    piSrcYPel = piSrcPel - sc_iLumaHalfTaps - sc_iLumaLeftMargin * iSrcPelStride;
    piDstY = piSrc - sc_iLumaTaps2 - iSrcStride;

    lumaQuarterFilters4Estimation<Pel, Int, 0, 0, INT_MIN, INT_MAX, 1>( piSrcYPel, piDstY, iSrcPelStride,
        iSrcStride4, iSrcPelStride, iSrcPelStride * (sc_iLumaTaps - 1), iSrcStride, iWidth + sc_iLumaTaps,
        iHeight );

    piSrcY = piSrc - iSrcStride - sc_iLumaLeftMargin * 4;
    piDstYPel = piDst + 1 - iDstStride;

    lumaQuarterFilters4Estimation<Int, Pel, 2048, 12, 0, 255, 4>( piSrcY, piDstYPel, iSrcStride4,
        iDstStride4, 4, 4*(sc_iLumaTaps - 2), -1, iWidth, iHeight );

    piSrcY = piSrc - sc_iLumaLeftMargin * 4;
    piDstYPel = piDst + 1;


    lumaQuarterFilters4Estimation<Int, Pel, 2048, 12, 0, 255, 4>( piSrcY, piDstYPel, iSrcStride4,
        iDstStride4, 4, 4*(sc_iLumaTaps - 2), -1, iWidth, iHeight );

    piSrcY = piSrc + iSrcStride - sc_iLumaLeftMargin * 4;
    piDstYPel = piDst + 1 + iDstStride;

    lumaQuarterFilters4Estimation<Int, Pel, 2048, 12, 0, 255, 4>( piSrcY, piDstYPel, iSrcStride4,
        iDstStride4, 4, 4*(sc_iLumaTaps - 2), -1, iWidth, iHeight );

    // Middle two pixels
    piSrcY = piSrc - iSrcStride;
    piDstYPel = piDst - iDstStride;
    Int iSrcStride2 = (iSrcStride << 1);
    Int iDstStride2 = (iDstStride << 1);

    for ( y = 0; y < iHeight * 2; y++ )
    {
      for ( x = 0; x < iWidth; x++ )
      {
        piDstYPel[x * 4] = Clip( (piSrcY[x*4] + 32) >> 6 );
      }
      piSrcY += iSrcStride2;
      piDstYPel += iDstStride2;
    }

    break;
  }
    case 2:
    {
    //  Quater-pel interpolation : vertical

    piSrcYPel = piSrcPel - sc_iLumaHalfTaps + 1 - sc_iLumaLeftMargin * iSrcPelStride;
    piDstY = piSrc - sc_iLumaTaps2 + 2 + iSrcStride;

    lumaQuarterFilters4Estimation<Pel, Int, 0, 0, INT_MIN, INT_MAX, 1>( piSrcYPel, piDstY, iSrcPelStride,
        iSrcStride4, iSrcPelStride, iSrcPelStride * (sc_iLumaTaps - 2), -iSrcStride,
        iWidth + sc_iLumaTaps - 1, iHeight );

    // Above three pixels

    piSrcY = piSrc - 2 - iSrcStride - sc_iLumaLeftMargin * 4;
    piDstYPel = piDst - 1 - iDstStride;

    lumaQuarterFilters4Estimation<Int, Pel, 2048, 12, 0, 255, 4>( piSrcY, piDstYPel, iSrcStride4,
        iDstStride4, 4, 4 * (sc_iLumaTaps - 1), 1, iWidth, iHeight );

    piSrcY = piSrc - 2 - iSrcStride - sc_iLumaLeftMargin*4;
    piDstYPel = piDst - iDstStride;

    lumaFilter<0, Int, Pel, 2048, 12, 0, 255, 4, 4>( piSrcY, piDstYPel, iSrcStride4, iDstStride4, 4, iWidth, iHeight );

    // Middle two pixels

    piDstYPel = piDst - 1;

    lumaQuarterFilters4Estimation<Pel, Pel, 32, 6, 0, 255, 1>( piSrcPel - sc_iLumaLeftMargin, piDstYPel,
        iSrcPelStride, iDstStride4, 1, (sc_iLumaTaps - 1), 1, iWidth, iHeight );

    // Below three pixels

    piSrcY = piSrc-2 + iSrcStride - sc_iLumaLeftMargin * 4;
    piDstYPel = piDst-1 + iDstStride;

    lumaQuarterFilters4Estimation<Int, Pel, 2048, 12, 0, 255, 4>( piSrcY, piDstYPel, iSrcStride4,
    iDstStride4, 4, 4*(sc_iLumaTaps - 1), 1, iWidth, iHeight );

    piSrcY = piSrc - 2 + iSrcStride - sc_iLumaLeftMargin * 4;
    piDstYPel = piDst + iDstStride;

    lumaFilter<0, Int, Pel, 2048, 12, 0, 255, 4, 4>( piSrcY, piDstYPel, iSrcStride4, iDstStride4, 4, iWidth, iHeight );

    break;
  }
    case 3:
    {
    //  Quater-pel interpolation : vertical

    piSrcYPel = piSrcPel - sc_iLumaHalfTaps - sc_iLumaLeftMargin * iSrcPelStride;
    piDstY = piSrc - sc_iLumaTaps2 + iSrcStride;

    lumaQuarterFilters4Estimation<Pel, Int, 0, 0, INT_MIN, INT_MAX, 1>( piSrcYPel, piDstY, iSrcPelStride,
        iSrcStride4, iSrcPelStride, iSrcPelStride * (sc_iLumaTaps - 2), -iSrcStride, iWidth + sc_iLumaTaps,
        iHeight );

    piSrcY = piSrc - iSrcStride - sc_iLumaLeftMargin * 4;
    piDstYPel = piDst + 1 - iDstStride;

    lumaQuarterFilters4Estimation<Int, Pel, 2048, 12, 0, 255, 4>( piSrcY, piDstYPel, iSrcStride4,
        iDstStride4, 4, 4 * (sc_iLumaTaps - 2), -1, iWidth, iHeight );

    piSrcYPel = piSrcPel - sc_iLumaLeftMargin;
    piDstYPel = piDst + 1;

    lumaQuarterFilters4Estimation<Pel, Pel, 32, 6, 0, 255, 1>( piSrcYPel, piDstYPel, iSrcPelStride,
        iDstStride4, 1, sc_iLumaTaps - 2, -1, iWidth, iHeight );

    piSrcY = piSrc + iSrcStride - 4 * sc_iLumaLeftMargin;
    piDstYPel = piDst + 1 + iDstStride;

    lumaQuarterFilters4Estimation<Int, Pel, 2048, 12, 0, 255, 4>( piSrcY, piDstYPel, iSrcStride4,
        iDstStride4, 4, 4 * (sc_iLumaTaps - 2), -1, iWidth, iHeight );

    // Middle two pixels
    piSrcY = piSrc - iSrcStride;
    piDstYPel = piDst - iDstStride;
    Int iSrcStride2 = (iSrcStride << 1);
    Int iDstStride2 = (iDstStride << 1);

    for ( y = 0; y < iHeight * 2; y++ )
    {
      for ( x = 0; x < iWidth; x++ )
      {
        piDstYPel[x * 4] = Clip( (piSrcY[x*4] + 32) >> 6 );
      }
      piSrcY += iSrcStride2;
      piDstYPel += iDstStride2;
    }

    break;
  }
    default:
    {
      assert(0);
    }
  }
}
#endif
