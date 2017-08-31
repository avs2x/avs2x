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

/** \file     TComPredFilter.cpp
    \brief    interpolation filter class
*/

#include <limits.h>
#include "TComPredFilter.h"

#define C2DVER1 0
#define S2DVER1 0
#define C2DHOR1 2048
#define S2DHOR1 12
#define C1D1    32
#define S1D1    6

// better visual quality, derived from DCT-IF with Hamming window applied
const int TComPredFilter::sc_iLumaTaps     = 8;
const int

#if IF_RD
TComPredFilter::sc_iKernels[2][8] =
{
  { -1, 4, -11, 40, 40, -11, 4, -1 },
  { -1, 4, -10, 57, 19, -7, 3, -1 }
};
#else
TComPredFilter::sc_iKernels[2][8] =
{
  { -1, 4, -10, 39, 39, -10, 4, -1 },
  { -1, 3, -9, 57, 19, -7, 3, -1 }
};
#endif

const int TComPredFilter::sc_iLumaHalfTaps = (sc_iLumaTaps >> 1);
const int TComPredFilter::sc_iLumaTaps2    = (sc_iLumaTaps << 1);
const int TComPredFilter::sc_iLumaLeftMargin  = (sc_iLumaTaps-2)>>1;
const int TComPredFilter::sc_iLumaRightMargin = sc_iLumaTaps-sc_iLumaLeftMargin;

//function pointers
const fnLumaFilter
TComPredFilter::sc_afnLumaFilter[3][2] =
{
    {
            lumaFilter<0, Pel, Pel, C1D1, S1D1, 0, 255, 1, 1>,
            lumaFilter<1, Pel, Pel, C1D1, S1D1, 0, 255, 1, 1>
    },
    {
            lumaFilter<0, Pel, Int, C2DVER1, S2DVER1, INT_MIN, INT_MAX, 1, 1>,
            lumaFilter<1, Pel, Int, C2DVER1, S2DVER1, INT_MIN, INT_MAX, 1, 1>
    },
    {
            lumaFilter<0, Int, Pel, C2DHOR1, S2DHOR1, 0, 255, 1, 1>,
            lumaFilter<1, Int, Pel, C2DHOR1, S2DHOR1, 0, 255, 1, 1>
    }
};

// ====================================================================================================================
// Constructor
// ====================================================================================================================

TComPredFilter::TComPredFilter()
{  
}
