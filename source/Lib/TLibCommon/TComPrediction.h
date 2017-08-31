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

/** \file     TComPrediction.h
    \brief    prediction class (header)
*/

#ifndef __TCOMPREDICTION__
#define __TCOMPREDICTION__


// Include files
#include "TComPic.h"
#include "TComMotionInfo.h"
#include "TComPattern.h"
#include "TComTrQuant.h"
#include "TComPredFilter.h"

//kolya
#include "TComPredFilterChroma.h"
// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// prediction class
class TComPrediction : public TComPredFilter
                     , public TComPredFilterChroma //kolya
{
protected:
#if ZHANGYI_INTRA
	Pel*    m_piYuvExt2g[MAX_NUM_COMPONENT][NUM_PRED_BUF];//3个通道：yuv，2个buffer：FILTERED  UNFILTERD
#endif
  Int*      m_piYuvExt;
  Int*      m_piYuvExt2; //pointer to second half, for chroma Cr prediction buffer //kolya
  Int       m_iYuvExtStride;
  Int       m_iYuvExtHeight;

  TComYuv   m_acYuvPred[2];
  TComYuv   m_cYuvPredTemp;
#if F_DHP_SYC_OP
  TComYuv*  m_acYuvPredDual[2];
#endif
#if F_MHPSKIP_SYC
  TComYuv  m_acMHPSkipYuvPred[2];
#endif
  TComYuv   m_cYuvExt;
#if rd_sym
  TComYuv   m_cYuvExtSym;
#endif
  // motion compensation functions
#if F_DHP_SYC
  Void xPredInterDual(TComDataCU* pcCU, UInt uiPartAddr, Int iWidth, Int iHeight, TComYuv*& rpcYuvPred, Int iPartIdx);
#endif
  Void xPredInterUni      ( TComDataCU* pcCU,                          UInt uiPartAddr,               Int iWidth, Int iHeight, RefPic eRefPic, TComYuv*& rpcYuvPred, Int iPartIdx          );
  Void xPredInterBi       ( TComDataCU* pcCU,                          UInt uiPartAddr,               Int iWidth, Int iHeight,                 TComYuv*& rpcYuvPred, Int iPartIdx          );
  Void xPredInterLumaBlk  ( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight,                 TComYuv*& rpcYuv                            );
  Void xPredInterChromaBlk( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight,                 TComYuv*& rpcYuv                            );
#if B_RPS_BUG_819
  Void xAverage(TComDataCU* pcCU, TComYuv* pcYuvSrc0, TComYuv* pcYuvSrc1, UInt uiPartAddr, Int iWidth, Int iHeight, TComYuv*& rpcYuvDst, Int PartAddr);
#else
  Void xAverage           ( TComDataCU* pcCU, TComYuv* pcYuvSrc0, TComYuv* pcYuvSrc1, UInt uiPartAddr, Int iWidth, Int iHeight, TComYuv*& rpcYuvDst );
#endif
#if DMH
	Void xPredDmhMode(TComDataCU *pcCU, Int uiPartIdx, UInt uiPartAddr, Int iWidth, Int iHeight, RefPic iRefPic);
#endif
public:
  TComPrediction();
  virtual ~TComPrediction();
  
  Void    initTempBuff();
  
  // inter
  Void motionCompensation         ( TComDataCU*  pcCU, TComYuv* pcYuvPred, RefPic eRefPic = REF_PIC_X, Int iPartIdx = -1 );

  // intra
  Void predIntra                  ( Pel* pSrc, Int iSrcStride, Pel* pDst, Int iDstStride, UInt uiSize, UInt uiDir );
#if ZHANGYI_INTRA
#if ZHANGYI_INTRA_SDIP
  Void predIntraAngAVS(const TextType compID, UInt uiDirMode, UInt uiOrgStride, Pel* piPred, UInt uiStride, UInt uiWidth, UInt uiHeight, Int bAbove, Int bLeft, UInt bitDepth);
#else
  Void predIntraAngAVS(const TextType compID, UInt uiDirMode, UInt uiOrgStride, Pel* piPred, UInt uiStride, UInt uiSize, Int bAbove, Int bLeft, UInt bitDepth);
#endif
  Void xPredIntraVertAdi(Pel* ptrSrc, UInt srcStride, Pel* ptrDst, UInt dstStride, UInt uiWidth, UInt uiHeight );
  Void xPredIntraHorAdi(Pel* ptrSrc, UInt srcStride, Pel* ptrDst, UInt dstStride, UInt uiWidth, UInt uiHeight );
  Void xPredIntraDCAdi(Pel* ptrSrc, UInt srcStride, Pel* ptrDst, UInt dstStride, UInt uiWidth, UInt uiHeight, int bAboveAvail, int bLeftAvail, UInt bitDepth );
  Void xPredIntraPlaneAdi(Pel* ptrSrc, UInt srcStride, Pel* ptrDst, UInt dstStride, UInt uiWidth, UInt uiHeight, UInt bitDepth );
  Void xPredIntraBiAdi(Pel* ptrSrc, UInt srcStride, Pel* ptrDst, UInt dstStride, UInt uiWidth, UInt uiHeight, UInt bitDepth );
  Void xPredIntraAngAdi(Pel* ptrSrc, UInt srcStride, Pel* ptrDst, UInt dstStride, UInt uiDirMode, UInt uiWidth, UInt uiHeight);
  Int getContextPixel(int uiDirMode, int uiXYflag, int iTempD, int *offset);
  Void predIntraAng( const TextType compID, UInt uiDirMode, Pel* piOrg , UInt uiOrgStride, Pel* piPred, UInt uiStride, UInt uiSize, Bool bAbove, Bool bLeft);
  Void getIntraNeighborAvailabilities(TComDataCU* pcCU, int maxCUSizeInBit, int img_x, int img_y, int bsx, int bsy, int* p_avail);
  Void fillReferenceSamples(const Int bitDepth, TComDataCU* pcCU, const Pel* piRoiOrigin, Pel* piAdiTemp, const UInt uiCuWidth, const UInt uiCuHeight, const Int iPicStride, Int* p_avail, Int img_x, Int img_y);
  Void xPredIntraPlanar( const Pel* pSrc, Int srcStride, Pel* rpDst, Int dstStride, UInt width, UInt height);
  Pel predIntraGetPredValDC( const Pel* pSrc, Int iSrcStride, UInt iWidth, UInt iHeight, Bool bAbove, Bool bLeft );
  Void xPredIntraAng( Int bitDepth, const Pel* pSrc, Int srcStride,Pel* pTrueDst, Int dstStrideTrue,UInt uiWidth, UInt uiHeight,UInt dirMode, Bool blkAboveAvailable, Bool blkLeftAvailable);
  Pel* getpiYuvExt2g(ComponentID compID, PRED_BUF buffer) {return m_piYuvExt2g[compID][buffer];}
#endif
  Int* getPredicBuf()             { return m_piYuvExt;      }
  Int  getPredicBufWidth()        { return m_iYuvExtStride; }
  Int  getPredicBufHeight()       { return m_iYuvExtHeight; }
#if DMH
	Void recDmhMode(TComDataCU *pcCU, Int uiPartIdx, RefPic iRefPic, TComYuv *pcYuvRec);
#endif
};


#endif // __TCOMPREDICTION__

