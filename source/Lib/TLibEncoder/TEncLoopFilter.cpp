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

/** \file     TEncLoopFilter.cpp
    \brief    deblocking filter
*/

#include "TEncLoopFilter.h"
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

TEncLoopFilter::TEncLoopFilter()
{
  m_uiDisableDeblockingFilterIdc = 0;

  m_puiPelY         = NULL;
  m_puiPelCb        = NULL;
  m_puiPelCr        = NULL;
  m_pcEntropyCoder  = NULL;
}

TEncLoopFilter::~TEncLoopFilter()
{
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncLoopFilter::setCfg( UInt uiDisableDblkIdc )
{
  m_uiDisableDeblockingFilterIdc  = uiDisableDblkIdc;
}

Void TEncLoopFilter::create( UInt uiMaxCUDepth )
{
  m_puiPelY  = new Pel[(1<<(g_uiLog2MaxCUSize<<1))<<2];
  m_puiPelCb = new Pel[1<<(g_uiLog2MaxCUSize<<1)];
  m_puiPelCr = new Pel[1<<(g_uiLog2MaxCUSize<<1)];
}

Void TEncLoopFilter::destroy()
{
  delete [] m_puiPelY;  m_puiPelY  = NULL;
  delete [] m_puiPelCb; m_puiPelCb = NULL;
  delete [] m_puiPelCr; m_puiPelCr = NULL;
}

/**
 - call deblocking function for every CU
 .
 \param  pcPic   picture class (TComPic) pointer
 */
Void TEncLoopFilter::loopFilterPic( TComPic* pcPic )
{
  if (m_uiDisableDeblockingFilterIdc == 1)
    return;
  
  // set entropy coder
#if AVS3_PIC_HEADER_ZL
  m_pcEntropyCoder->setEntropyCoder ( m_pcRDGoOnSbacCoder, pcPic->getPicture(), pcPic->getPicHeader() );
#else
  m_pcEntropyCoder->setEntropyCoder ( m_pcRDGoOnSbacCoder, pcPic->getPicture() );
#endif
  m_pcRDGoOnSbacCoder->load( m_pcSbacCoder );

  m_pcEntropyCoder->resetEntropy();

  // for every CU
  for ( UInt uiCUAddr = 0; uiCUAddr < pcPic->getNumCUsInFrame(); uiCUAddr++ )
  {
    TComDataCU* pcCU = pcPic->getCU( uiCUAddr );

    // CU-based deblocking
    xDeblockCU( pcCU, 0, 0 );
  }
}

Void TEncLoopFilter::xDeblockCU( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth )
{
  TComPic* pcPic     = pcCU->getPic();

  if( pcCU->getDepth(uiAbsZorderIdx) > uiDepth )
  {
    UInt uiCurNumParts = pcPic->getNumPartInCU() >> (uiDepth<<1);
    UInt uiQNumParts   = uiCurNumParts>>2;
    for ( UInt uiPartIdx = 0; uiPartIdx < 4; uiPartIdx++, uiAbsZorderIdx+=uiQNumParts )
    {
      UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsZorderIdx] ];
      UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsZorderIdx] ];
      if( ( uiLPelX < pcCU->getPicture()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getPicture()->getSPS()->getHeight() ) )
        xDeblockCU( pcCU, uiAbsZorderIdx, uiDepth+1 );
    }
    return;
  }

  m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);

  UInt uiSizeInPU = pcPic->getNumPartInWidth()>>uiDepth;
  UInt PartIdxIncr = uiSizeInPU;

  UInt uiSize  = 1<<pcCU->getLog2CUSize(uiAbsZorderIdx);

  UInt uiVerPelOffset = uiSize;
  UInt uiHorPelOffset = uiSize;
  Pel* puiRefY  = pcPic->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), uiAbsZorderIdx);
  Pel* puiRefCb = pcPic->getPicYuvRec()->getCbAddr  (pcCU->getAddr(), uiAbsZorderIdx);
  Pel* puiRefCr = pcPic->getPicYuvRec()->getCrAddr  (pcCU->getAddr(), uiAbsZorderIdx);
  UInt uiCurStride  = pcPic->getPicYuvRec()->getStride();
  UInt uiCurCStride = pcPic->getPicYuvRec()->getCStride();

  Pel* puiOrgY  = pcPic->getPicYuvOrg()->getLumaAddr(pcCU->getAddr(), uiAbsZorderIdx);
  Pel* puiOrgCb = pcPic->getPicYuvOrg()->getCbAddr  (pcCU->getAddr(), uiAbsZorderIdx);
  Pel* puiOrgCr = pcPic->getPicYuvOrg()->getCrAddr  (pcCU->getAddr(), uiAbsZorderIdx);
  UInt uiOrgStride  = pcPic->getPicYuvOrg()->getStride();
  UInt uiOrgCStride = pcPic->getPicYuvOrg()->getCStride();

  UInt uiCurOffset = uiVerPelOffset*uiCurStride + uiHorPelOffset;
  Pel* puiCurY = puiRefY - uiCurOffset;

  uiCurOffset = (uiVerPelOffset*uiCurCStride + uiHorPelOffset)>>1;
  Pel* puiCurCb = puiRefCb - uiCurOffset;
  Pel* puiCurCr = puiRefCr - uiCurOffset;

  UInt uiOrgOffset = uiVerPelOffset*uiOrgStride + uiHorPelOffset;
  puiOrgY -= uiOrgOffset;

  uiOrgOffset = (uiVerPelOffset*uiOrgCStride + uiHorPelOffset)>>1;
  puiOrgCb -= uiOrgOffset;
  puiOrgCr -= uiOrgOffset;

  UInt uiW = uiSize + uiHorPelOffset;
  UInt uiH = uiSize + uiVerPelOffset;
  UInt uiHW = uiW>>1;
  UInt uiHH = uiH>>1;
  UInt ui;
  Pel* pTmpY;
  Pel* pTmpCb;
  Pel* pTmpCr;

  Pel* pCurY;
  Pel* pCurCb;
  Pel* pCurCr;

  // Store Current reconstruction
  pTmpY = m_puiPelY; pTmpCb = m_puiPelCb; pTmpCr = m_puiPelCr;
  pCurY = puiCurY;   pCurCb = puiCurCb;   pCurCr = puiCurCr;
  for(ui=0; ui<uiH; ui++)
  {
    memcpy(pTmpY,  pCurY, sizeof(Pel)*uiW);
    pTmpY  += uiW;
    pCurY  += uiCurStride;
  }
  for(ui=0; ui<uiHH; ui++)
  {
    memcpy(pTmpCb, pCurCb, sizeof(Pel)*uiHW);
    memcpy(pTmpCr, pCurCr, sizeof(Pel)*uiHW);
    pTmpCb += uiHW;
    pTmpCr += uiHW;

    pCurCb += uiCurCStride;
    pCurCr += uiCurCStride;
  }

  UInt   uiDist     = MAX_UINT;
  Double uiCost     = MAX_DOUBLE;
  Double uiBestCost = MAX_DOUBLE;

  UInt uiBest[2]  = {0,0};

  for( Int iVerStrength=0; iVerStrength<=1; iVerStrength++)
  {
    pcCU->setDBKIdx(0, uiAbsZorderIdx, uiDepth, iVerStrength);
    for( Int iHorStrength=0; iHorStrength<=1; iHorStrength++ )
    {
      pcCU->setDBKIdx(1, uiAbsZorderIdx, uiDepth, iHorStrength);

      for( Int iDir = EDGE_VER; iDir <= EDGE_HOR; iDir++ )
      {
        if( iDir == EDGE_VER && iVerStrength == 0 ) continue;
        if( iDir == EDGE_HOR && iHorStrength == 0 ) continue;

        for ( UInt iEdge = 0; iEdge < uiSizeInPU ; iEdge+=PartIdxIncr)
        {
          if( (iDir == EDGE_VER) && ( (g_auiRasterToPelX[g_auiZscanToRaster[uiAbsZorderIdx]] + pcCU->getCUPelX())==0 ) && (iEdge==0) )
            continue;
          if( (iDir == EDGE_HOR) && ( (g_auiRasterToPelY[g_auiZscanToRaster[uiAbsZorderIdx]] + pcCU->getCUPelY())==0 ) && (iEdge==0) )
            continue;

          xEdgeFilterLuma  ( pcCU, uiAbsZorderIdx, uiDepth, iDir, iEdge );
          xEdgeFilterChroma( pcCU, uiAbsZorderIdx, uiDepth, iDir, iEdge );
        }
      }

      uiDist = m_pcRDCost->getDistPart(puiCurY,  uiCurStride,  puiOrgY,  uiOrgStride,  1<<(pcCU->getLog2CUSize(uiAbsZorderIdx) + 1), 1<<(pcCU->getLog2CUSize(uiAbsZorderIdx) + 1) );
      UInt uiChromadist = m_pcRDCost->getDistPart(puiCurCb, uiCurCStride, puiOrgCb, uiOrgCStride, 1<<pcCU->getLog2CUSize(uiAbsZorderIdx), 1<<pcCU->getLog2CUSize(uiAbsZorderIdx))
                        + m_pcRDCost->getDistPart(puiCurCr, uiCurCStride, puiOrgCr, uiOrgCStride, 1<<pcCU->getLog2CUSize(uiAbsZorderIdx), 1<<pcCU->getLog2CUSize(uiAbsZorderIdx));
      uiDist += (UInt)((Double)uiChromadist*m_pcRDCost->getChromaWeight());

      // recover reconstruction here
      pTmpY = m_puiPelY; pTmpCb = m_puiPelCb; pTmpCr = m_puiPelCr;
      pCurY = puiCurY;   pCurCb = puiCurCb;   pCurCr = puiCurCr;
      for(ui=0; ui<uiH; ui++)
      {
        memcpy(pCurY, pTmpY, sizeof(Pel)*uiW);
        pTmpY  += uiW;
        pCurY  += uiCurStride;
      }
      for(ui=0; ui<uiHH; ui++)
      {
        memcpy(pCurCb, pTmpCb, sizeof(Pel)*uiHW);
        memcpy(pCurCr, pTmpCr, sizeof(Pel)*uiHW);
        pTmpCb += uiHW;
        pTmpCr += uiHW;

        pCurCb += uiCurCStride;
        pCurCr += uiCurCStride;
      }

      m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);
      m_pcEntropyCoder->resetBits();
      m_pcEntropyCoder->encodeDBKIdx(pcCU, uiAbsZorderIdx);

      UInt uiBits = m_pcEntropyCoder->getNumberOfWrittenBits();

      uiCost = m_pcRDCost->calcRdCost( uiBits, uiDist );

      // check best
      if( uiCost < uiBestCost )
      {
        uiBest[0]  = iVerStrength;
        uiBest[1]  = iHorStrength;
        uiBestCost = uiCost;
      }
    }
  }

  pcCU->setDBKIdx(0, uiAbsZorderIdx, uiDepth, uiBest[0]);
  pcCU->setDBKIdx(1, uiAbsZorderIdx, uiDepth, uiBest[1]);

  for( Int iDir = EDGE_VER; iDir <= EDGE_HOR; iDir++ )
  {
    if( iDir == EDGE_VER && uiBest[0] == 0 ) continue;
    if( iDir == EDGE_HOR && uiBest[1] == 0 ) continue;

    for ( UInt iEdge = 0; iEdge < uiSizeInPU ; iEdge+=PartIdxIncr)
    {
      if( (iDir == EDGE_VER) && ( (g_auiRasterToPelX[g_auiZscanToRaster[uiAbsZorderIdx]] + pcCU->getCUPelX())==0 ) && (iEdge==0) )
        continue;
      if( (iDir == EDGE_HOR) && ( (g_auiRasterToPelY[g_auiZscanToRaster[uiAbsZorderIdx]] + pcCU->getCUPelY())==0 ) && (iEdge==0) )
        continue;

      xEdgeFilterLuma  ( pcCU, uiAbsZorderIdx, uiDepth, iDir, iEdge );
      xEdgeFilterChroma( pcCU, uiAbsZorderIdx, uiDepth, iDir, iEdge );
    }
  }

  m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);
  m_pcEntropyCoder->encodeDBKIdx(pcCU, uiAbsZorderIdx);
}

Void TEncLoopFilter::xEdgeFilterLuma( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdge  )
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();
  Pel* piSrc    = pcPicYuvRec->getLumaAddr( pcCU->getAddr(), uiAbsZorderIdx );
  Pel* piTmpSrc = piSrc;

  Int  iStride = pcPicYuvRec->getStride();
  UInt uiNumParts = pcCU->getPic()->getNumPartInWidth()>>uiDepth;

  UInt uiPelsInPart = 1<<(g_uiLog2MaxCUSize - g_uiMaxCUDepth);
  UInt uiTUSize     = 1<<(g_uiLog2MaxCUSize - uiDepth);
  UInt PartIdxIncr = uiNumParts;

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

    UInt ui = pcCU->getDBKIdx( iDir, uiAbsZorderIdx+uiPartIdx );

    if ( ui )
    {
      for ( UInt i = 0; i < uiTUSize; i++ )
        xPelFilterLuma( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+i), iOffset );
    }
  }
}

Void TEncLoopFilter::xEdgeFilterChroma( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdge )
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

    if( pcCU->getDBKIdx( iDir, uiAbsZorderIdx + uiPartIdx ) )
    {
      for ( UInt uiStep = 0; uiStep < uiTUSize; uiStep++ )
      {
        xPelFilterChroma( piTmpSrcCb + iSrcStep*(uiStep+iIdx*uiPelsInPartChroma), iOffset );
        xPelFilterChroma( piTmpSrcCr + iSrcStep*(uiStep+iIdx*uiPelsInPartChroma), iOffset );
      }
    }
  }
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

__inline Void TEncLoopFilter::xPelFilterLuma( Pel* piSrc, Int iOffset )
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

__inline Void TEncLoopFilter::xPelFilterChroma( Pel* piSrc, Int iOffset )
{
  Pel m2  = piSrc[-iOffset*2];
  Pel m3  = piSrc[-iOffset];
  Pel m4  = piSrc[0];
  Pel m5  = piSrc[ iOffset];

  piSrc[-iOffset] = Clip( (m2 + (m3<<1) + m4 + 2)>>2 );
  piSrc[       0] = Clip( (m3 + (m4<<1) + m5 + 2)>>2 );
}
