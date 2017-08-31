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

/** \file     TEncCU.h
    \brief    CU encoder class (header)
*/

#ifndef __TENCCU__
#define __TENCCU__

// Include files
#include "../TLibCommon/CommonDef.h"
#include "../TLibCommon/TComYuv.h"
#include "../TLibCommon/TComPrediction.h"
#include "../TLibCommon/TComTrQuant.h"
#include "../TLibCommon/TComBitCounter.h"
#include "../TLibCommon/TComDataCU.h"

#include "TEncEntropy.h"
#include "TEncSearch.h"

class TEncTop;
class TEncSbac;
class TEncPicture;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// CU encoder class
class TEncCu
{
private:

  TComDataCU**            m_ppcBestCU;      ///< Best CUs in each depth
  TComDataCU**            m_ppcTempCU;      ///< Temporary CUs in each depth
  UChar                   m_uhTotalDepth;
  
  TComYuv*                m_pcResiYuvBest; ///< Best Residual Yuv for each depth
  TComYuv*                m_pcPredYuvTemp; ///< Temporary Prediction Yuv for each depth
  TComYuv*                m_pcResiYuvTemp; ///< Temporary Residual Yuv for each depth
  TComYuv*                m_pcRecoYuvTemp; ///< Temporary Reconstruction Yuv for each depth
  TComYuv*                m_pcOrigYuv;     ///< Original Yuv for each depth
  
  //  Access channel
  TEncCfg*                m_pcEncCfg;
  TComPrediction*         m_pcPrediction;
  TEncSearch*             m_pcPredSearch;
  TComTrQuant*            m_pcTrQuant;
  TComBitCounter*         m_pcBitCounter;
  TComRdCost*             m_pcRdCost;
  
  TEncEntropy*            m_pcEntropyCoder;
  TEncSbac*               m_pcSbacCoder;
  
  // SBAC RD
  TEncSbac***             m_pppcRDSbacCoder;
  TEncSbac*               m_pcRDGoOnSbacCoder;

public:
  /// copy parameters from encoder class
  Void  init                ( TEncTop* pcEncTop );
  
  /// create internal buffers
  Void  create              ( UChar uhTotalDepth, UInt uiLog2MaxCUSize );
  
  /// destroy internal buffers
  Void  destroy             ();
  
  /// CU analysis function
  Void  compressCU          ( TComDataCU*&  rpcCU );
  
  /// CU encoding function
  Void  encodeCU            ( TComDataCU*    pcCU );
  
  /// set QP value
  Void  setQp               ( Int iQp, Int iCQp ) { m_pcTrQuant->setQp(iQp, iCQp); }
  
protected:
#if WLQ_CUSplitFlag 
#if WLQ_rdcost
	Double  xCompressCU(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiDepth, Double split_rd_cost);
#else
	Void  xCompressCU(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiDepth, Double split_rd_cost);
#endif
#else
  Void  xCompressCU         ( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiDepth        );
#endif
  Void  xEncodeCU           ( TComDataCU*  pcCU, UInt uiAbsPartIdx,           UInt uiDepth        );
  
#if inter_direct_skip_bug
  Double  xCheckRDCostSkip(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double minRdCostD, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag);
#else
	  Void  xCheckRDCostSkip(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiSplitBits);
#endif
#if RD_DIRECT
	Void  xCheckRDCostDirect(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiSplitBits);
#endif
#if inter_direct_skip_bug
	Double	xCheckRDCostSkip_NO_QT(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double minRdCostD, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag);
#endif

#if B_MHBSKIP_SYC
#if F_MHPSKIP_SYC
	Double  xCheckRDCostMHSkip(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double minRdCostD, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag);
	Double	xCheckRDCostMHSkip_NO_QT(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double minRdCostD, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag);
#endif
#else
#if F_MHPSKIP_SYC
	Double  xCheckRDCostMHPSkip(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double minRdCostD, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag);
	Double	xCheckRDCostMHPSkip_NO_QT(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double minRdCostD, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag);
#endif
#endif
#if WLQ_CUSplitFlag_inter
#if WLQ_rdcost
		Double  xCheckRDCostInter(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double minRdCostD, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag);
#else
	Void  xCheckRDCostInter(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag);
#endif
#else
  Void  xCheckRDCostInter   ( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, UInt uiSplitBits );
#endif
#if WLQ_CUSplitFlag
#if WLQ_rdcost
  Double  xCheckRDCostIntra(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double minRdCostD, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag);
#else
  Void  xCheckRDCostIntra(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag);
#endif
#else
  Void  xCheckRDCostIntra   ( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, UInt uiSplitBits );
#endif
#if WLQ_rdcost
  Double  xCheckBestMode(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, Double splitRdCost, UInt uiDepth, Bool bSplitMode = false);
#else
  Void  xCheckBestMode      ( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiDepth, Bool bSplitMode = false );
#endif
  Void  xCopyYuv2Pic        ( TComPic* rpcPic, UInt uiCUAddr, UInt uiAbsZorderIdx, UInt uiDepth );
  Void  xCopyYuv2Tmp        ( UInt uhPartUnitIdx, UInt uiDepth );
#if ZHANGYI_INTRA_SDIP
  Void xCopyYuv2PicIntra(TComPic* rpcPic, UInt uiCUAddr, UInt uiAbsZorderIdx, UInt uiDepth);
#endif

};


#endif // __TENCMB__

