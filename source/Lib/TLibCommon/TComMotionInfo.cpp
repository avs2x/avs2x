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

/** \file     TComMotionInfo.cpp
    \brief    motion information handling classes
*/

#include <memory.h>
#include "TComMotionInfo.h"
#include "assert.h"
#include <stdlib.h>

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

// --------------------------------------------------------------------------------------------------------------------
// Create / destroy
// --------------------------------------------------------------------------------------------------------------------

Void TComCUMvField::create( UInt uiNumPartition )
{
  m_pcMv     = ( TComMv* )xMalloc( TComMv, uiNumPartition );
  m_pcMvd    = ( TComMv* )xMalloc( TComMv, uiNumPartition );
#if RPS
  m_piRefIdx = new Int[uiNumPartition];
#endif
  m_uiNumPartition = uiNumPartition;


}

Void TComCUMvField::destroy()
{
	if (m_pcMv)
  {
    xFree( m_pcMv );     m_pcMv     = NULL;
  }
	if (m_pcMvd)
  {
    xFree( m_pcMvd );    m_pcMvd    = NULL;
  }
#if RPS
	if (m_piRefIdx )
  {
    delete[] m_piRefIdx; m_piRefIdx = NULL;  //释放ref数组
  }
#endif
}

// --------------------------------------------------------------------------------------------------------------------
// Clear / copy
// --------------------------------------------------------------------------------------------------------------------

Void TComCUMvField::clearMv( Int iPartAddr, UInt uiDepth )
{
  Int iNumPartition = m_uiNumPartition >> (uiDepth<<1);
  
  for ( Int i = iNumPartition - 1; i >= 0; i-- )
  {
    m_pcMv[ i ].setZero();
  }
}

Void TComCUMvField::clearMvd( Int iPartAddr, UInt uiDepth )
{
  Int iNumPartition = m_uiNumPartition >> (uiDepth<<1);
  
  for ( Int i = iNumPartition - 1; i >= 0; i-- )
  {
    m_pcMvd[ i ].setZero();
  }
}

Void TComCUMvField::clearMvField()
{
  for ( Int i = m_uiNumPartition - 1; i >= 0; i-- )
  {
    m_pcMv    [ i ].setZero();
    m_pcMvd   [ i ].setZero();
  }
#if RPS
  //assert(sizeof(*m_piRefIdx) == 1);
  memset(m_piRefIdx, NOT_VALID, m_uiNumPartition * sizeof(*m_piRefIdx));
#endif
}

Void TComCUMvField::copyFrom( TComCUMvField* pcCUMvFieldSrc, Int iNumPartSrc, Int iPartAddrDst )
{
  Int iSizeInTComMv = sizeof( TComMv ) * iNumPartSrc;
  
  memcpy( m_pcMv     + iPartAddrDst, pcCUMvFieldSrc->getMv(),     iSizeInTComMv );
  memcpy( m_pcMvd    + iPartAddrDst, pcCUMvFieldSrc->getMvd(),    iSizeInTComMv );
#if RPS
  memcpy(m_piRefIdx + iPartAddrDst, pcCUMvFieldSrc->getRefIdx(), sizeof(*m_piRefIdx)*iNumPartSrc);
#endif
}

Void TComCUMvField::copyTo( TComCUMvField* pcCUMvFieldDst, Int iPartAddrDst )
{
  Int iSizeInTComMv = sizeof( TComMv ) * m_uiNumPartition;
  
  memcpy( pcCUMvFieldDst->getMv()     + iPartAddrDst, m_pcMv,     iSizeInTComMv );
  memcpy( pcCUMvFieldDst->getMvd()    + iPartAddrDst, m_pcMvd,    iSizeInTComMv );
#if RPS
  memcpy(pcCUMvFieldDst->getRefIdx() + iPartAddrDst, m_piRefIdx, sizeof(*m_piRefIdx)*m_uiNumPartition);
#endif
}

Void TComCUMvField::copyTo( TComCUMvField* pcCUMvFieldDst, Int iPartAddrDst, UInt uiOffset, UInt uiNumPart )
{
  Int iSizeInTComMv = sizeof( TComMv ) * uiNumPart;
  Int iOffset = uiOffset + iPartAddrDst;
  
  memcpy( pcCUMvFieldDst->getMv()     + iOffset, m_pcMv     + uiOffset, iSizeInTComMv );
  memcpy( pcCUMvFieldDst->getMvd()    + iOffset, m_pcMvd    + uiOffset, iSizeInTComMv );
#if RPS
  memcpy(pcCUMvFieldDst->getRefIdx() + iOffset, m_piRefIdx + uiOffset, sizeof(*m_piRefIdx) * uiNumPart);
#endif
}

Void TComCUMvField::copyMvTo( TComCUMvField* pcCUMvFieldDst, Int iPartAddrDst )
{
  memcpy( pcCUMvFieldDst->getMv() + iPartAddrDst, m_pcMv, sizeof( TComMv ) * m_uiNumPartition );
}

// --------------------------------------------------------------------------------------------------------------------
// Set
// --------------------------------------------------------------------------------------------------------------------

Void TComCUMvField::setAllMv( TComMv& rcMv, PartSize ePartMode, Int iPartAddr, Int iPartIdx, UInt uiDepth )
{
  Int i;
  TComMv* pcMv = m_pcMv + iPartAddr;
  register TComMv cMv = rcMv;
  Int iNumPartition = m_uiNumPartition >> (uiDepth<<1);
  
  switch( ePartMode )
  {
    case SIZE_2Nx2N:
      for ( i = iNumPartition - 1; i >= 0; i-- )
      {
        pcMv[ i ] = cMv;
      }
      break;
    case SIZE_2NxN:
      for ( i = ( iNumPartition >> 1 ) - 1; i >= 0; i-- )
      {
        pcMv[ i ] = cMv;
      }
      break;
    case SIZE_Nx2N:
    {
      UInt uiOffset = iNumPartition >> 1;
      for ( i = ( iNumPartition >> 2 ) - 1; i >= 0; i-- )
      {
        pcMv[ i ] = cMv;
        pcMv[ i + uiOffset ] = cMv;
      }
      break;
    }
    case SIZE_NxN:
      for ( i = ( iNumPartition >> 2 ) - 1; i >= 0; i-- )
      {
        pcMv[ i ] = cMv;
      }
      break;
  case SIZE_2NxnU:
    {
      Int iCurrPartNumQ = iNumPartition>>2;
      if( iPartIdx == 0 )
      {
        TComMv* pi  = pcMv;
        TComMv* pi2 = pcMv + iCurrPartNumQ;
        for (i = 0; i < (iCurrPartNumQ>>1); i++)
        {
          pi [i] = cMv;
          pi2[i] = cMv;
        }
      }
      else
      {
        TComMv* pi  = pcMv;
        for (i = 0; i < (iCurrPartNumQ>>1); i++)
        {
          pi[i] = cMv;
        }

        pi = pcMv + iCurrPartNumQ;
        for (i = 0; i < ( (iCurrPartNumQ>>1) + (iCurrPartNumQ<<1) ); i++)
        {
          pi[i] = cMv;
        }
      }
      break;
    }
  case SIZE_2NxnD:
    {
      Int iCurrPartNumQ = iNumPartition>>2;
      if( iPartIdx == 0 )
      {
        TComMv* pi  = pcMv;
        for (i = 0; i < ( (iCurrPartNumQ>>1) + (iCurrPartNumQ<<1) ); i++)
        {
          pi[i] = cMv;
        }
        pi = pcMv + ( iNumPartition - iCurrPartNumQ );
        for (i = 0; i < (iCurrPartNumQ>>1); i++)
        {
          pi[i] = cMv;
        }
      }
      else
      {
        TComMv* pi  = pcMv;
        TComMv* pi2 = pcMv + iCurrPartNumQ;
        for (i = 0; i < (iCurrPartNumQ>>1); i++)
        {
          pi [i] = cMv;
          pi2[i] = cMv;
        }
      }
      break;
    }
  case SIZE_nLx2N:
    {
      Int iCurrPartNumQ = iNumPartition>>2;
      if( iPartIdx == 0 )
      {
        TComMv* pi  = pcMv;
        TComMv* pi2 = pcMv + (iCurrPartNumQ<<1);
        TComMv* pi3 = pcMv + (iCurrPartNumQ>>1);
        TComMv* pi4 = pcMv + (iCurrPartNumQ<<1) + (iCurrPartNumQ>>1);

        for (i = 0; i < (iCurrPartNumQ>>2); i++)
        {
          pi [i] = cMv;
          pi2[i] = cMv;
          pi3[i] = cMv;
          pi4[i] = cMv;
        }
      }
      else
      {
        TComMv* pi  = pcMv;
        TComMv* pi2 = pcMv + (iCurrPartNumQ<<1);
        for (i = 0; i < (iCurrPartNumQ>>2); i++)
        {
          pi [i] = cMv;
          pi2[i] = cMv;
        }

        pi  = pcMv + (iCurrPartNumQ>>1);
        pi2 = pcMv + (iCurrPartNumQ<<1) + (iCurrPartNumQ>>1);
        for (i = 0; i < ( (iCurrPartNumQ>>2) + iCurrPartNumQ ); i++)
        {
          pi [i] = cMv;
          pi2[i] = cMv;
        }
      }
      break;
    }
  case SIZE_nRx2N:
    {
      Int iCurrPartNumQ = iNumPartition>>2;
      if( iPartIdx == 0 )
      {
        TComMv* pi  = pcMv;
        TComMv* pi2 = pcMv + (iCurrPartNumQ<<1);
        for (i = 0; i < ( (iCurrPartNumQ>>2) + iCurrPartNumQ ); i++)
        {
          pi [i] = cMv;
          pi2[i] = cMv;
        }

        pi  = pcMv + iCurrPartNumQ + (iCurrPartNumQ>>1);
        pi2 = pcMv + iNumPartition - iCurrPartNumQ + (iCurrPartNumQ>>1);
        for (i = 0; i < (iCurrPartNumQ>>2); i++)
        {
          pi [i] = cMv;
          pi2[i] = cMv;
        }
      }
      else
      {
        TComMv* pi  = pcMv;
        TComMv* pi2 = pcMv + (iCurrPartNumQ>>1);
        TComMv* pi3 = pcMv + (iCurrPartNumQ<<1);
        TComMv* pi4 = pcMv + (iCurrPartNumQ<<1) + (iCurrPartNumQ>>1);
        for (i = 0; i < (iCurrPartNumQ>>2); i++)
        {
          pi [i] = cMv;
          pi2[i] = cMv;
          pi3[i] = cMv;
          pi4[i] = cMv;
        }
      }
      break;
    }
    default:
      assert(0);
      break;
  }
}

Void TComCUMvField::setAllMvd( TComMv& rcMvd, PartSize ePartMode, Int iPartAddr, Int iPartIdx, UInt uiDepth )
{
  Int i;
  TComMv* pcMvd = m_pcMvd + iPartAddr;
  register TComMv cMvd = rcMvd;
  Int iNumPartition = m_uiNumPartition >> (uiDepth<<1);
  
  switch( ePartMode )
  {
    case SIZE_2Nx2N:
      for ( i = iNumPartition - 1; i >= 0; i-- )
      {
        pcMvd[ i ] = cMvd;
      }
      break;
    case SIZE_2NxN:
      for ( i = ( iNumPartition >> 1 ) - 1; i >= 0; i-- )
      {
        pcMvd[ i ] = cMvd;
      }
      break;
    case SIZE_Nx2N:
    {
      UInt uiOffset = iNumPartition >> 1;
      for ( i = ( iNumPartition >> 2 ) - 1; i >= 0; i-- )
      {
        pcMvd[ i ] = cMvd;
        pcMvd[ i + uiOffset ] = cMvd;
      }
      break;
    }
    case SIZE_NxN:
      for ( i = ( iNumPartition >> 2 ) - 1; i >= 0; i-- )
      {
        pcMvd[ i ] = cMvd;
      }
      break;
   case SIZE_2NxnU:
    {
      Int iCurrPartNumQ = iNumPartition>>2;
      if( iPartIdx == 0 )
      {
        TComMv* pi  = pcMvd;
        TComMv* pi2 = pcMvd + iCurrPartNumQ;
        for (i = 0; i < (iCurrPartNumQ>>1); i++)
        {
          pi [i] = cMvd;
          pi2[i] = cMvd;
        }
      }
      else
      {
        TComMv* pi  = pcMvd;
        for (i = 0; i < (iCurrPartNumQ>>1); i++)
        {
          pi[i] = cMvd;
        }

        pi = pcMvd + iCurrPartNumQ;
        for (i = 0; i < ( (iCurrPartNumQ>>1) + (iCurrPartNumQ<<1) ); i++)
        {
          pi[i] = cMvd;
        }
      }
      break;
    }
  case SIZE_2NxnD:
    {
      Int iCurrPartNumQ = iNumPartition>>2;
      if( iPartIdx == 0 )
      {
        TComMv* pi  = pcMvd;
        for (i = 0; i < ( (iCurrPartNumQ>>1) + (iCurrPartNumQ<<1) ); i++)
        {
          pi[i] = cMvd;
        }
        pi = pcMvd + ( iNumPartition - iCurrPartNumQ );
        for (i = 0; i < (iCurrPartNumQ>>1); i++)
        {
          pi[i] = cMvd;
        }
      }
      else
      {
        TComMv* pi  = pcMvd;
        TComMv* pi2 = pcMvd + iCurrPartNumQ;
        for (i = 0; i < (iCurrPartNumQ>>1); i++)
        {
          pi [i] = cMvd;
          pi2[i] = cMvd;
        }
      }
      break;
    }
  case SIZE_nLx2N:
    {
      Int iCurrPartNumQ = iNumPartition>>2;
      if( iPartIdx == 0 )
      {
        TComMv* pi  = pcMvd;
        TComMv* pi2 = pcMvd + (iCurrPartNumQ<<1);
        TComMv* pi3 = pcMvd + (iCurrPartNumQ>>1);
        TComMv* pi4 = pcMvd + (iCurrPartNumQ<<1) + (iCurrPartNumQ>>1);

        for (i = 0; i < (iCurrPartNumQ>>2); i++)
        {
          pi [i] = cMvd;
          pi2[i] = cMvd;
          pi3[i] = cMvd;
          pi4[i] = cMvd;
        }
      }
      else
      {
        TComMv* pi  = pcMvd;
        TComMv* pi2 = pcMvd + (iCurrPartNumQ<<1);
        for (i = 0; i < (iCurrPartNumQ>>2); i++)
        {
          pi [i] = cMvd;
          pi2[i] = cMvd;
        }

        pi  = pcMvd + (iCurrPartNumQ>>1);
        pi2 = pcMvd + (iCurrPartNumQ<<1) + (iCurrPartNumQ>>1);
        for (i = 0; i < ( (iCurrPartNumQ>>2) + iCurrPartNumQ ); i++)
        {
          pi [i] = cMvd;
          pi2[i] = cMvd;
        }
      }
      break;
    }
  case SIZE_nRx2N:
    {
      Int iCurrPartNumQ = iNumPartition>>2;
      if( iPartIdx == 0 )
      {
        TComMv* pi  = pcMvd;
        TComMv* pi2 = pcMvd + (iCurrPartNumQ<<1);
        for (i = 0; i < ( (iCurrPartNumQ>>2) + iCurrPartNumQ ); i++)
        {
          pi [i] = cMvd;
          pi2[i] = cMvd;
        }

        pi  = pcMvd + iCurrPartNumQ + (iCurrPartNumQ>>1);
        pi2 = pcMvd + iNumPartition - iCurrPartNumQ + (iCurrPartNumQ>>1);
        for (i = 0; i < (iCurrPartNumQ>>2); i++)
        {
          pi [i] = cMvd;
          pi2[i] = cMvd;
        }
      }
      else
      {
        TComMv* pi  = pcMvd;
        TComMv* pi2 = pcMvd + (iCurrPartNumQ>>1);
        TComMv* pi3 = pcMvd + (iCurrPartNumQ<<1);
        TComMv* pi4 = pcMvd + (iCurrPartNumQ<<1) + (iCurrPartNumQ>>1);
        for (i = 0; i < (iCurrPartNumQ>>2); i++)
        {
          pi [i] = cMvd;
          pi2[i] = cMvd;
          pi3[i] = cMvd;
          pi4[i] = cMvd;
        }
      }
      break;
    }
    default:
      assert(0);
      break;
  }
}

#if RPS
Void TComCUMvField::setAllMvField(TComMvField & mvField, PartSize ePartMode, Int iPartAddr, Int iPartIdx, UInt uiDepth)
{
  setAllMv(mvField.getMv(), ePartMode, iPartAddr, iPartIdx, uiDepth);
  setAllRefIdx(mvField.getRefIdx(), ePartMode, iPartAddr, iPartIdx, uiDepth);
  return;
}
#else
Void TComCUMvField::setAllMvField(TComMv& rcMv, PartSize ePartMode, Int iPartAddr, Int iPartIdx, UInt uiDepth)
{
  setAllMv(rcMv, ePartMode, iPartAddr, iPartIdx, uiDepth);
  return;
}
#endif
#if RPS
//初始化PU的MVFIELD参考帧信息
Void TComCUMvField::setAllRefIdx(Int &iRefIdx, PartSize ePartMode, Int iPartAddr, Int iPartIdx, UInt uiDepth)
{
  //setAll(m_piRefIdx, static_cast<Int>(iRefIdx), ePartMode, iPartAddr, iPartIdx, uiDepth);
  Int i;
  Int *piRefIdx = m_piRefIdx + iPartAddr;
  register  Int riRefIdx = iRefIdx;
  Int iNumPartition = m_uiNumPartition >> (uiDepth << 1);

  switch (ePartMode)
  {
  case SIZE_2Nx2N:
    for (i = iNumPartition - 1; i >= 0; i--)
    {
      piRefIdx[i] = riRefIdx;
    }
    break;
  case SIZE_2NxN:
    for (i = (iNumPartition >> 1) - 1; i >= 0; i--)
    {
      piRefIdx[i] = riRefIdx;
    }
    break;
  case SIZE_Nx2N:
  {
    UInt uiOffset = iNumPartition >> 1;
    for (i = (iNumPartition >> 2) - 1; i >= 0; i--)
    {
      piRefIdx[i] = riRefIdx;
      piRefIdx[i + uiOffset] = riRefIdx;
    }
    break;
  }
  case SIZE_NxN:
    for (i = (iNumPartition >> 2) - 1; i >= 0; i--)
    {
      piRefIdx[i] = riRefIdx;
    }
    break;
  case SIZE_2NxnU:
  {
    Int iCurrPartNumQ = iNumPartition >> 2;
    if (iPartIdx == 0)
    {
      Int* pi = piRefIdx;
      Int* pi2 = piRefIdx + iCurrPartNumQ;
      for (i = 0; i < (iCurrPartNumQ >> 1); i++)
      {
        pi[i] = riRefIdx;
        pi2[i] = riRefIdx;
      }
    }
    else
    {
      Int* pi = piRefIdx;
      for (i = 0; i < (iCurrPartNumQ >> 1); i++)
      {
        pi[i] = riRefIdx;
      }

      pi = piRefIdx + iCurrPartNumQ;
      for (i = 0; i < ((iCurrPartNumQ >> 1) + (iCurrPartNumQ << 1)); i++)
      {
        pi[i] = riRefIdx;
      }
    }
    break;
  }
  case SIZE_2NxnD:
  {
    Int iCurrPartNumQ = iNumPartition >> 2;
    if (iPartIdx == 0)
    {
      Int* pi = piRefIdx;
      for (i = 0; i < ((iCurrPartNumQ >> 1) + (iCurrPartNumQ << 1)); i++)
      {
        pi[i] = riRefIdx;
      }
      pi = piRefIdx + (iNumPartition - iCurrPartNumQ);
      for (i = 0; i < (iCurrPartNumQ >> 1); i++)
      {
        pi[i] = riRefIdx;
      }
    }
    else
    {
      Int* pi = piRefIdx;
      Int* pi2 = piRefIdx + iCurrPartNumQ;
      for (i = 0; i < (iCurrPartNumQ >> 1); i++)
      {
        pi[i] = riRefIdx;
        pi2[i] = riRefIdx;
      }
    }
    break;
  }
  case SIZE_nLx2N:
  {
    Int iCurrPartNumQ = iNumPartition >> 2;
    if (iPartIdx == 0)
    {
      Int* pi = piRefIdx;
      Int* pi2 = piRefIdx + (iCurrPartNumQ << 1);
      Int* pi3 = piRefIdx + (iCurrPartNumQ >> 1);
      Int* pi4 = piRefIdx + (iCurrPartNumQ << 1) + (iCurrPartNumQ >> 1);

      for (i = 0; i < (iCurrPartNumQ >> 2); i++)
      {
        pi[i] = riRefIdx;
        pi2[i] = riRefIdx;
        pi3[i] = riRefIdx;
        pi4[i] = riRefIdx;
      }
    }
    else
    {
      Int* pi = piRefIdx;
      Int* pi2 = piRefIdx + (iCurrPartNumQ << 1);
      for (i = 0; i < (iCurrPartNumQ >> 2); i++)
      {
        pi[i] = riRefIdx;
        pi2[i] = riRefIdx;
      }

      pi = piRefIdx + (iCurrPartNumQ >> 1);
      pi2 = piRefIdx + (iCurrPartNumQ << 1) + (iCurrPartNumQ >> 1);
      for (i = 0; i < ((iCurrPartNumQ >> 2) + iCurrPartNumQ); i++)
      {
        pi[i] = riRefIdx;
        pi2[i] = riRefIdx;
      }
    }
    break;
  }
  case SIZE_nRx2N:
  {
    Int iCurrPartNumQ = iNumPartition >> 2;
    if (iPartIdx == 0)
    {
      Int* pi = piRefIdx;
      Int* pi2 = piRefIdx + (iCurrPartNumQ << 1);
      for (i = 0; i < ((iCurrPartNumQ >> 2) + iCurrPartNumQ); i++)
      {
        pi[i] = riRefIdx;
        pi2[i] = riRefIdx;
      }

      pi = piRefIdx + iCurrPartNumQ + (iCurrPartNumQ >> 1);
      pi2 = piRefIdx + iNumPartition - iCurrPartNumQ + (iCurrPartNumQ >> 1);
      for (i = 0; i < (iCurrPartNumQ >> 2); i++)
      {
        pi[i] = riRefIdx;
        pi2[i] = riRefIdx;
      }
    }
    else
    {
      Int* pi = piRefIdx;
      Int* pi2 = piRefIdx + (iCurrPartNumQ >> 1);
      Int* pi3 = piRefIdx + (iCurrPartNumQ << 1);
      Int* pi4 = piRefIdx + (iCurrPartNumQ << 1) + (iCurrPartNumQ >> 1);
      for (i = 0; i < (iCurrPartNumQ >> 2); i++)
      {
        pi[i] = riRefIdx;
        pi2[i] = riRefIdx;
        pi3[i] = riRefIdx;
        pi4[i] = riRefIdx;
      }
    }
    break;
  }
  default:
    assert(0);
    break;
  }
}


template <typename T>
Void TComCUMvField::setAll(T *p, T const & val, PartSize ePartMode, Int iPartAddr, Int iPartIdx, UInt uiDepth)
{
  Int i;
  //p += iPartAddr;
  Int numElements = m_uiNumPartition >> (2 * uiDepth);

  switch (ePartMode)
  {
  case SIZE_2Nx2N:
    for (i = 0; i < numElements; i++)
    {
      p[i] = val;
    }
    break;

  case SIZE_2NxN:
    numElements >>= 1;
    for (i = 0; i < numElements; i++)
    {
      p[i] = val;
    }
    break;

  case SIZE_Nx2N:
    numElements >>= 2;
    for (i = 0; i < numElements; i++)
    {
      p[i] = val;
      p[i + 2 * numElements] = val;
    }
    break;

  case SIZE_NxN:
    numElements >>= 2;
    for (i = 0; i < numElements; i++)
    {
      p[i] = val;
    }
    break;
  case SIZE_2NxnU:
  {
    Int iCurrPartNumQ = numElements >> 2;
    if (iPartIdx == 0)
    {
      T *pT = p;
      T *pT2 = p + iCurrPartNumQ;
      for (i = 0; i < (iCurrPartNumQ >> 1); i++)
      {
        pT[i] = val;
        pT2[i] = val;
      }
    }
    else
    {
      T *pT = p;
      for (i = 0; i < (iCurrPartNumQ >> 1); i++)
      {
        pT[i] = val;
      }

      pT = p + iCurrPartNumQ;
      for (i = 0; i < ((iCurrPartNumQ >> 1) + (iCurrPartNumQ << 1)); i++)
      {
        pT[i] = val;
      }
    }
    break;
  }
  case SIZE_2NxnD:
  {
    Int iCurrPartNumQ = numElements >> 2;
    if (iPartIdx == 0)
    {
      T *pT = p;
      for (i = 0; i < ((iCurrPartNumQ >> 1) + (iCurrPartNumQ << 1)); i++)
      {
        pT[i] = val;
      }
      pT = p + (numElements - iCurrPartNumQ);
      for (i = 0; i < (iCurrPartNumQ >> 1); i++)
      {
        pT[i] = val;
      }
    }
    else
    {
      T *pT = p;
      T *pT2 = p + iCurrPartNumQ;
      for (i = 0; i < (iCurrPartNumQ >> 1); i++)
      {
        pT[i] = val;
        pT2[i] = val;
      }
    }
    break;
  }
  case SIZE_nLx2N:
  {
    Int iCurrPartNumQ = numElements >> 2;
    if (iPartIdx == 0)
    {
      T *pT = p;
      T *pT2 = p + (iCurrPartNumQ << 1);
      T *pT3 = p + (iCurrPartNumQ >> 1);
      T *pT4 = p + (iCurrPartNumQ << 1) + (iCurrPartNumQ >> 1);

      for (i = 0; i < (iCurrPartNumQ >> 2); i++)
      {
        pT[i] = val;
        pT2[i] = val;
        pT3[i] = val;
        pT4[i] = val;
      }
    }
    else
    {
      T *pT = p;
      T *pT2 = p + (iCurrPartNumQ << 1);
      for (i = 0; i < (iCurrPartNumQ >> 2); i++)
      {
        pT[i] = val;
        pT2[i] = val;
      }

      pT = p + (iCurrPartNumQ >> 1);
      pT2 = p + (iCurrPartNumQ << 1) + (iCurrPartNumQ >> 1);
      for (i = 0; i < ((iCurrPartNumQ >> 2) + iCurrPartNumQ); i++)
      {
        pT[i] = val;
        pT2[i] = val;
      }
    }
    break;
  }
  case SIZE_nRx2N:
  {
    Int iCurrPartNumQ = numElements >> 2;
    if (iPartIdx == 0)
    {
      T *pT = p;
      T *pT2 = p + (iCurrPartNumQ << 1);
      for (i = 0; i < ((iCurrPartNumQ >> 2) + iCurrPartNumQ); i++)
      {
        pT[i] = val;
        pT2[i] = val;
      }

      pT = p + iCurrPartNumQ + (iCurrPartNumQ >> 1);
      pT2 = p + numElements - iCurrPartNumQ + (iCurrPartNumQ >> 1);
      for (i = 0; i < (iCurrPartNumQ >> 2); i++)
      {
        pT[i] = val;
        pT2[i] = val;
      }
    }
    else
    {
      T *pT = p;
      T *pT2 = p + (iCurrPartNumQ >> 1);
      T *pT3 = p + (iCurrPartNumQ << 1);
      T *pT4 = p + (iCurrPartNumQ << 1) + (iCurrPartNumQ >> 1);
      for (i = 0; i < (iCurrPartNumQ >> 2); i++)
      {
        pT[i] = val;
        pT2[i] = val;
        pT3[i] = val;
        pT4[i] = val;
      }
    }
    break;
  }
  default:
    assert(0);
    break;
  }
}

#endif

#if DPB
Void TComCUMvField::compress(const TComCUMvField &source, Int NumPartInWidth)
{
	assert(source.m_uiNumPartition == m_uiNumPartition);
	for (Int partIdx = 0; partIdx < m_uiNumPartition; partIdx++)
	{
		Int xPos = partIdx % NumPartInWidth;
		Int yPos = partIdx / NumPartInWidth;
		Int centerxPos = xPos / MV_DECIMATION_FACTOR * MV_DECIMATION_FACTOR + 2; //MV_DECIMATION_FACTOR
		Int centeryPos = yPos / MV_DECIMATION_FACTOR * MV_DECIMATION_FACTOR + 2;
		Int stridePartIdx = centeryPos * NumPartInWidth + centerxPos;

		TComMv cMv(0, 0);
		Int iRefIdx = 0;
		cMv = source.m_pcMv[g_auiRasterToZscan[stridePartIdx]];  //Z scan order
		iRefIdx = source.m_piRefIdx[g_auiRasterToZscan[stridePartIdx]]; ////Z scan order

		m_pcMv[partIdx] = cMv;  //Raster order  
		m_piRefIdx[partIdx] = iRefIdx;   //Raster order在存储colmv和colref时，采用光删存储方式，当然也可以转换成Zscan
	}
}
#endif