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

/** \file     TEncLoopFilter.h
    \brief    deblocking filter (header)
*/

#ifndef __TCOMLOOPFILTER__
#define __TCOMLOOPFILTER__

#include "../TLibCommon/CommonDef.h"
#include "../TLibCommon/TComPic.h"

#include "../TLibCommon/TComRdCost.h"
#include "TEncEntropy.h"
#include "TEncSbac.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// deblocking filter class
class TEncLoopFilter
{
private:
  UInt         m_uiDisableDeblockingFilterIdc; ///< deblocking filter idc

  TEncEntropy* m_pcEntropyCoder;
  TComRdCost*  m_pcRDCost;
  TEncSbac*    m_pcSbacCoder;                  ///< SBAC encoder
  TEncSbac***  m_pppcRDSbacCoder;              ///< temporal storage for RD computation
  TEncSbac*    m_pcRDGoOnSbacCoder;            ///< going on SBAC model for RD stage

  Pel*         m_puiPelY;
  Pel*         m_puiPelCb;
  Pel*         m_puiPelCr;
protected:
  /// CU-level deblocking function
  Void xDeblockCU             ( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth );  
  Void xEdgeFilterLuma        ( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdge );
  Void xEdgeFilterChroma      ( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdge );

  __inline Void xPelFilterLuma  ( Pel* piSrc, Int iOffset );
  __inline Void xPelFilterChroma( Pel* piSrc, Int iOffset );
  
public:
  TEncLoopFilter();
  virtual ~TEncLoopFilter();
  
  Void  create                    ( UInt uiMaxCUDepth );
  Void  destroy                   ();

  Void  setEntropy(TEncEntropy* pcEntropy) { m_pcEntropyCoder = pcEntropy; }
  Void  setRDCost( TComRdCost* p ) { m_pcRDCost = p; }
  Void  setSbacCoder          (TEncSbac* p)   { m_pcSbacCoder   = p; }
  Void  setRDSbacCoder        (TEncSbac*** p) { m_pppcRDSbacCoder   = p; }
  Void  setRDGoOnSbacCoder    (TEncSbac*   p) { m_pcRDGoOnSbacCoder = p; }

  /// set configuration
  Void setCfg( UInt uiDisableDblkIdc );
  
  /// picture-level deblocking filter
  Void loopFilterPic( TComPic* pcPic );
};

#endif

