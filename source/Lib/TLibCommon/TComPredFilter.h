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

/** \file     TComPredFilter.h
    \brief    interpolation filter class (header)
*/

#ifndef __TCOMPREDFILTER__
#define __TCOMPREDFILTER__

// Include files
#include <limits.h>
#include "TComPic.h"
#include "TComMotionInfo.h"
#if DCTIF || chazhi_bug
#include "TComTrQuant.h"
#include "TComInterpolationFilter.h"
#endif

// ====================================================================================================================
// Constants
// ====================================================================================================================

// Local type definitions
#define HAL_IDX   1
#define QU0_IDX   0
#define QU1_IDX   2

// ====================================================================================================================
// Class definition
// ====================================================================================================================

#define ARGS_LUMA void* piSrc, \
                  void* piDst, \
                  const int iSrcStride, \
                  const int iDstStride, \
                  const int iSize

typedef void (*fnLumaFilter) ( ARGS_LUMA, const int iWidth, const int iHeight );

/// interpolation filter class
class TComPredFilter
{
protected:
  
#if DCTIF || chazhi_bug
	TComYuv m_filteredBlock[4][4];
	TComYuv m_filteredBlockTmp[4];

	TComInterpolationFilter m_if;
#endif

  // filter description (luma)
  static const int sc_iLumaTaps2;
  static const int sc_iLumaRightMargin;
  static const int sc_iLumaLeftMargin, sc_iLumaTaps, sc_iLumaHalfTaps;
  
  static const int sc_iKernels[2][8];
  static const fnLumaFilter sc_afnLumaFilter[3][2];

  template<int FRAC_INDEX, class IN, class OUT, int ROUNDING_CONST, int ROUNDING_SHIFT,
      int LEFT_LIMIT, int RIGHT_LIMIT, int SOURCE_STEP, int DESTINATION_STEP>
  static inline void lumaFilter( ARGS_LUMA, const int iWidth, const int iHeight )
  {
    OUT* piDst1 = (OUT*) piDst;
    IN* piSrcTmp;
    IN* piSrc1 = (IN*) piSrc;

    Int iSum = 0;

    for ( int y = iHeight; y != 0; y-- )
    {
      piSrcTmp = piSrc1;
      for ( int x = 0; x < iWidth + (8 & (LEFT_LIMIT >> 31)); x++ )
      {
        iSum = (*piSrcTmp) * sc_iKernels[FRAC_INDEX][0];

        for (int k = 1; k < sc_iLumaTaps; k++)
          iSum += (*(piSrcTmp + k*iSize)) * sc_iKernels[FRAC_INDEX][k];

        piDst1[x*DESTINATION_STEP] = Clip3( LEFT_LIMIT, RIGHT_LIMIT, (iSum + ROUNDING_CONST) >> ROUNDING_SHIFT );
        piSrcTmp += SOURCE_STEP;
      }
      piSrc1 += iSrcStride;
      piDst1 += iDstStride;
    }
  };

  /* in estimation:
   *
   * - buffer is updated always pair-wise in both quarter pels
   * - destination step is always 4 (todo: make different planes)
   *
   * TODO: next optimization is possible with usage of intermediately
   * computed values
   * Actually, only 1 template may be left here in common class
   * of interpolation filter for encoder and decoder interface
   * Next optimization may be achieved by re-factoring of TEncSearch calls
   * of filter functions, and changing buffer structures etc.
   * Left as you see it due to the lack of time.
   * //kolya
   *
   * TODO: unify direction (vertical/horizontal for destination and source)
   */

  template<class IN, class OUT, int ROUNDING_CONST, int ROUNDING_SHIFT,
      int LEFT_LIMIT, int RIGHT_LIMIT, int SOURCE_STEP>
  static inline void lumaQuarterFilters4Estimation( ARGS_LUMA, const int iSrcOffset, const int iDstOffset, const int iWidth, const int iHeight )
  {
    OUT* piDst1 = (OUT*) piDst;
    OUT* piDst2 = piDst1 + 2*iDstOffset;

    IN* piSrcTmp1;
    IN* piSrcTmp2;

    IN* piSrc1 = (IN*) piSrc;
    IN* piSrc2 = piSrc1 + iSrcOffset;

    Int iSum1 = 0;
    Int iSum2 = 0;

    for ( int y = iHeight; y != 0; y-- )
    {
      piSrcTmp1 = piSrc1;
      piSrcTmp2 = piSrc2;

      for ( int x = 0; x < iWidth; x++ )
      {
        iSum1 =  (* piSrcTmp1)              * sc_iKernels[1][0];
        iSum2 =  (* piSrcTmp2)              * sc_iKernels[1][0];

        for ( int k = 1; k < sc_iLumaTaps; k++)
        {
          iSum1 += (*(piSrcTmp1 + k*iSize)) * sc_iKernels[1][k];
          iSum2 += (*(piSrcTmp2 - k*iSize)) * sc_iKernels[1][k];
        }

        piDst1[x*4] = Clip3( LEFT_LIMIT, RIGHT_LIMIT, (iSum1 + ROUNDING_CONST) >> ROUNDING_SHIFT );
        piDst2[x*4] = Clip3( LEFT_LIMIT, RIGHT_LIMIT, (iSum2 + ROUNDING_CONST) >> ROUNDING_SHIFT );

        piSrcTmp1 += SOURCE_STEP;
        piSrcTmp2 += SOURCE_STEP;
      }
      piSrc1 += iSrcStride;
      piDst1 += iDstStride;

      piSrc2 += iSrcStride;
      piDst2 += iDstStride;
    }
  };

public:
  TComPredFilter();
  
  static
  inline Void xCTI_FilterHalfVer (
      Pel* piSrc, Int iSrcStride, Int iSrcStep,
      Int iWidth, Int iHeight,
      Int iDstStride, Int iDstStep, Int*& rpiDst,
      Int iDstStridePel, Pel*& rpiDstPel )
  {

    Int*  piDst = rpiDst;
    Pel*  piDstPel = rpiDstPel;
    Int   iSum = 0;

    Pel*  piSrcSaved = piSrc - sc_iLumaLeftMargin*iSrcStride;
    Pel*  piSrcTmp;

    for ( Int y = iHeight; y != 0; y-- )
    {
      piSrcTmp = piSrcSaved;
      for ( Int x = 0; x < iWidth; x++ )
      {
        iSum = sc_iKernels[0][0]*piSrcTmp[0];

        for (int k=1; k < sc_iLumaTaps; k++)
          iSum += sc_iKernels[0][k]*piSrcTmp[k*iSrcStride];

        piDst[x * iDstStep]    = iSum;
        piDstPel[x * iDstStep] = Clip( (iSum + 32) >> 6 );
        piSrcTmp += iSrcStep;
      }
      piSrcSaved += iSrcStride;
      piDst += iDstStride;
      piDstPel += iDstStridePel;
    }
  }

};

#endif // __TCOMPREDFILTER__
