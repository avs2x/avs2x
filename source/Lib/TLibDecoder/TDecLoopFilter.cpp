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

/** \file     TDecLoopFilter.cpp
    \brief    deblocking filter
*/

#include "TDecLoopFilter.h"
#include "../TLibCommon/TComPicture.h"
#include "../TLibCommon/TComMv.h"

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define   EDGE_VER    0
#define   EDGE_HOR    1

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TDecLoopFilter::TDecLoopFilter()
{
  m_uiDisableDeblockingFilterIdc = 0;
}

TDecLoopFilter::~TDecLoopFilter()
{
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TDecLoopFilter::setCfg( UInt uiDisableDblkIdc )
{
  m_uiDisableDeblockingFilterIdc  = uiDisableDblkIdc;
}

Void TDecLoopFilter::create( UInt uiMaxCUDepth )
{
}

Void TDecLoopFilter::destroy()
{
}

/**
 - call deblocking function for every CU
 .
 \param  pcPic   picture class (TComPic) pointer
 */

Void TDecLoopFilter::loopFilterPic( TComPic* pcPic )
{
  if (m_uiDisableDeblockingFilterIdc == 1)
    return;

  // for every CU
  for ( UInt uiCUAddr = 0; uiCUAddr < pcPic->getNumCUsInFrame(); uiCUAddr++ )
  {
    TComDataCU* pcCU = pcPic->getCU( uiCUAddr );

    // CU-based deblocking
    xDeblockCU( pcCU, 0, 0 );
  }
}

Void TDecLoopFilter::xEdgeFilterLuma( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdge  )
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();
  Pel* piSrc    = pcPicYuvRec->getLumaAddr( pcCU->getAddr(), uiAbsZorderIdx );
  Pel* piTmpSrc = piSrc;

  Int  iStride = pcPicYuvRec->getStride();
  UInt uiNumParts = pcCU->getPic()->getNumPartInWidth()>>uiDepth;

  UInt uiPelsInPart = 1<<(g_uiLog2MaxCUSize - g_uiMaxCUDepth);
  UInt uiTUSize     = 1<<(g_uiLog2MaxCUSize - uiDepth);
  UInt PartIdxIncr  = uiNumParts;

  Int   iOffset, iSrcStep;
  UInt  uiPartIdx, uiPartOffset;
  if (iDir == EDGE_VER)
  {
    iOffset = 1;
    iSrcStep = iStride;
    piTmpSrc += iEdge*uiPelsInPart;
    uiPartOffset = g_auiRasterToZscan[iEdge];
  }
  else  // (iDir == EDGE_HOR)
  {
    iOffset = iStride;
    iSrcStep = 1;
    piTmpSrc += iEdge*uiPelsInPart*iStride;
    uiPartOffset = g_auiRasterToZscan[iEdge*pcCU->getPic()->getNumPartInWidth()];
  }

  for ( UInt iIdx = 0; iIdx < uiNumParts; iIdx+=PartIdxIncr )
  {
    if (iDir == EDGE_VER)
      uiPartIdx = uiPartOffset + g_auiRasterToZscan[iIdx*pcCU->getPic()->getNumPartInWidth()];
    else
      uiPartIdx = uiPartOffset + g_auiRasterToZscan[iIdx];

    for ( UInt i = 0; i < uiTUSize; i++ )
      xPelFilterLuma( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+i), iOffset );
  }
}

Void TDecLoopFilter::xEdgeFilterChroma( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdge )
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();
  Int         iStride     = pcPicYuvRec->getCStride();
  Pel*        piSrcCb     = pcPicYuvRec->getCbAddr( pcCU->getAddr(), uiAbsZorderIdx );
  Pel*        piSrcCr     = pcPicYuvRec->getCrAddr( pcCU->getAddr(), uiAbsZorderIdx );

  UInt  uiPelsInPartChroma = 1<<(g_uiLog2MaxCUSize - g_uiMaxCUDepth - 1);

  Int   iOffset, iSrcStep;

  UInt  uiNumParts = pcCU->getPic()->getNumPartInWidth()>>uiDepth;

  UInt uiTUSize    = 1<<(g_uiLog2MaxCUSize - uiDepth - 1);
  UInt PartIdxIncr = uiNumParts;

  Pel* piTmpSrcCb = piSrcCb;
  Pel* piTmpSrcCr = piSrcCr;

  UInt  uiPartIdx, uiPartOffset;

  if (iDir == EDGE_VER)
  {
    iOffset   = 1;
    iSrcStep  = iStride;
    piTmpSrcCb += iEdge*uiPelsInPartChroma;
    piTmpSrcCr += iEdge*uiPelsInPartChroma;

    uiPartOffset = g_auiRasterToZscan[iEdge];
  }
  else  // (iDir == EDGE_HOR)
  {
    iOffset   = iStride;
    iSrcStep  = 1;
    piTmpSrcCb += iEdge*iStride*uiPelsInPartChroma;
    piTmpSrcCr += iEdge*iStride*uiPelsInPartChroma;

    uiPartOffset = g_auiRasterToZscan[iEdge*pcCU->getPic()->getNumPartInWidth()];
  }

  for ( UInt iIdx = 0; iIdx < uiNumParts; iIdx+=PartIdxIncr )
  {
    if (iDir == EDGE_VER)
      uiPartIdx = uiPartOffset + g_auiRasterToZscan[iIdx*pcCU->getPic()->getNumPartInWidth()];
    else
      uiPartIdx = uiPartOffset + g_auiRasterToZscan[iIdx];

      for ( UInt uiStep = 0; uiStep < uiTUSize; uiStep++ )
      {
        xPelFilterChroma( piTmpSrcCb + iSrcStep*(uiStep+iIdx*uiPelsInPartChroma), iOffset );
        xPelFilterChroma( piTmpSrcCr + iSrcStep*(uiStep+iIdx*uiPelsInPartChroma), iOffset );
      }
    }
  }

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================
Void TDecLoopFilter::xDeblockCU( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth )
{
  TComPic* pcPic     = pcCU->getPic();
  UInt uiCurNumParts = pcPic->getNumPartInCU() >> (uiDepth<<1);
  UInt uiQNumParts   = uiCurNumParts>>2;

  if( pcCU->getDepth(uiAbsZorderIdx) > uiDepth )
  {
    for ( UInt uiPartIdx = 0; uiPartIdx < 4; uiPartIdx++, uiAbsZorderIdx+=uiQNumParts )
    {
      UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsZorderIdx] ];
      UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsZorderIdx] ];
      if( ( uiLPelX < pcCU->getPicture()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getPicture()->getSPS()->getHeight() ) )
        xDeblockCU( pcCU, uiAbsZorderIdx, uiDepth+1 );
    }
    return;
  }

  UInt uiSizeInPU = pcPic->getNumPartInWidth()>>uiDepth;
  UInt PartIdxIncr = uiSizeInPU;

  for ( Int iDir = EDGE_VER; iDir <= EDGE_HOR; iDir++ )
  {
    if( iDir == EDGE_VER && pcCU->getDBKIdx(0, uiAbsZorderIdx) == 0 ) continue;
    if( iDir == EDGE_HOR && pcCU->getDBKIdx(1, uiAbsZorderIdx) == 0 ) continue;

    for ( UInt iEdge = 0; iEdge < uiSizeInPU ; iEdge+=PartIdxIncr )
    {
      if( (iDir == EDGE_VER) && ( (g_auiRasterToPelX[g_auiZscanToRaster[uiAbsZorderIdx]] + pcCU->getCUPelX())==0 ) && (iEdge==0) )
        continue;
      if( (iDir == EDGE_HOR) && ( (g_auiRasterToPelY[g_auiZscanToRaster[uiAbsZorderIdx]] + pcCU->getCUPelY())==0 ) && (iEdge==0) )
        continue;

      xEdgeFilterLuma( pcCU, uiAbsZorderIdx, uiDepth, iDir, iEdge );
      xEdgeFilterChroma( pcCU, uiAbsZorderIdx, uiDepth, iDir, iEdge );
    }
  }
}

__inline Void TDecLoopFilter::xPelFilterLuma( Pel* piSrc, Int iOffset )
{
  Pel m1  = piSrc[-iOffset*3];
  Pel m2  = piSrc[-iOffset*2];
  Pel m3  = piSrc[-iOffset];
  Pel m4  = piSrc[0];
  Pel m5  = piSrc[ iOffset];
  Pel m6  = piSrc[ iOffset*2];

  piSrc[-iOffset*2] = Clip( (m1 + (m2<<1) + m3 + 2)>>2 );
  piSrc[-iOffset  ] = Clip( (m2 + (m3<<1) + m4 + 2)>>2 );
  piSrc[         0] = Clip( (m3 + (m4<<1) + m5 + 2)>>2 );
  piSrc[ iOffset  ] = Clip( (m4 + (m5<<1) + m6 + 2)>>2 );
}

__inline Void TDecLoopFilter::xPelFilterChroma( Pel* piSrc, Int iOffset )
{
  Pel m2  = piSrc[-iOffset*2];
  Pel m3  = piSrc[-iOffset];
  Pel m4  = piSrc[0];
  Pel m5  = piSrc[ iOffset];
  
  piSrc[-iOffset] = Clip( (m2 + (m3<<1) + m4 + 2)>>2 );
  piSrc[       0] = Clip( (m3 + (m4<<1) + m5 + 2)>>2 );
}
