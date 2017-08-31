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

/** \file     TComPattern.cpp
    \brief    neighbouring pixel access classes
*/

#include "TComPic.h"
#include "TComPattern.h"
#include "TComDataCU.h"

// ====================================================================================================================
// Public member functions (TComPatternParam)
// ====================================================================================================================

/** \param  piTexture     pixel data
 \param  iRoiWidth     pattern width
 \param  iRoiHeight    pattern height
 \param  iStride       buffer stride
 \param  iOffsetLeft   neighbour offset (left)
 \param  iOffsetRight  neighbour offset (right)
 \param  iOffsetAbove  neighbour offset (above)
 \param  iOffsetBottom neighbour offset (bottom)
 */
Void TComPatternParam::setPatternParamPel ( Pel* piTexture,
                                           Int iRoiWidth,
                                           Int iRoiHeight,
                                           Int iStride,
                                           Int iOffsetLeft,
                                           Int iOffsetRight,
                                           Int iOffsetAbove,
                                           Int iOffsetBottom )
{
  m_piPatternOrigin = piTexture;
  
  m_iROIWidth       = iRoiWidth;
  m_iROIHeight      = iRoiHeight;

  m_iPatternStride  = iStride;
  m_iOffsetLeft     = iOffsetLeft;
  m_iOffsetAbove    = iOffsetAbove;
  m_iOffsetRight    = iOffsetRight;
  m_iOffsetBottom   = iOffsetBottom;
}

// ====================================================================================================================
// Public member functions (TComPattern)
// ====================================================================================================================
Void TComPattern::initPattern ( Pel* piY,
                               Pel* piCb,
                               Pel* piCr,
                               Int iRoiWidth,
                               Int iRoiHeight,
                               Int iStride,
                               Int iOffsetLeft,
                               Int iOffsetRight,
                               Int iOffsetAbove,
                               Int iOffsetBottom )
{
  m_cPatternY. setPatternParamPel( piY,  iRoiWidth,      iRoiHeight,      iStride,      iOffsetLeft,      iOffsetRight,      iOffsetAbove,      iOffsetBottom );
  m_cPatternCb.setPatternParamPel( piCb, iRoiWidth >> 1, iRoiHeight >> 1, iStride >> 1, iOffsetLeft >> 1, iOffsetRight >> 1, iOffsetAbove >> 1, iOffsetBottom >> 1 );
  m_cPatternCr.setPatternParamPel( piCr, iRoiWidth >> 1, iRoiHeight >> 1, iStride >> 1, iOffsetLeft >> 1, iOffsetRight >> 1, iOffsetAbove >> 1, iOffsetBottom >> 1 );
  
  return;
}

