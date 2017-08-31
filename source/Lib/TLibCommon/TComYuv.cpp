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

/** \file     TComYuv.cpp
    \brief    general YUV buffer class
    \todo     this should be merged with TComPicYuv
*/

#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <math.h>

#include "CommonDef.h"
#include "TComYuv.h"
#if ZHANGYI_INTRA_SDIP
#include "TComDataCU.h"
#endif

TComYuv::TComYuv()
{
  m_apiBufY = NULL;
  m_apiBufU = NULL;
  m_apiBufV = NULL;
}

TComYuv::~TComYuv()
{
}

Void TComYuv::create( UInt iLog2CUSize, UInt uiExt )
{
#if ZHANGYI_INTRA
	UInt uiLumaSize   = (2*(1<<iLog2CUSize)+uiExt)*(2*(1<<iLog2CUSize)+uiExt);
	UInt uiChromaSize = ((1<<(iLog2CUSize))+uiExt)*((1<<(iLog2CUSize))+uiExt);
#else
  UInt uiLumaSize   = ((1<<iLog2CUSize)+uiExt)*((1<<iLog2CUSize)+uiExt);
  UInt uiChromaSize = ((1<<(iLog2CUSize-1))+uiExt)*((1<<(iLog2CUSize-1))+uiExt);
#endif

  m_uiExt = uiExt;

  // memory allocation
  m_apiBufY  = (Pel*)xMalloc( Pel, uiLumaSize    );
  m_apiBufU  = (Pel*)xMalloc( Pel, uiChromaSize );
  m_apiBufV  = (Pel*)xMalloc( Pel, uiChromaSize );
  
  // set width and height
#if DCTIF
	m_iWidth = 1 << iLog2CUSize;
	m_iHeight = 1 << iLog2CUSize;
	m_iCWidth = 1 << (iLog2CUSize - 1);
	m_iCHeight = 1 << (iLog2CUSize - 1);
#endif

#if ZHANGYI_INTRA
  m_iSize  = 2*(1<<iLog2CUSize) + uiExt;
  m_iCSize = (1<<(iLog2CUSize)) + uiExt;
#else
  m_iSize  = (1<<iLog2CUSize) + uiExt;
  m_iCSize = (1<<(iLog2CUSize-1)) + uiExt;
#endif

  m_apiBufY += (uiExt + uiExt*m_iSize);
  m_apiBufU += (uiExt + uiExt*m_iCSize);
  m_apiBufV += (uiExt + uiExt*m_iCSize);
}


Void TComYuv::destroy()
{
  m_apiBufY -= (m_uiExt + m_uiExt*m_iSize);
  m_apiBufU -= (m_uiExt + m_uiExt*m_iCSize);
  m_apiBufV -= (m_uiExt + m_uiExt*m_iCSize);

  // memory free
  xFree( m_apiBufY ); m_apiBufY = NULL;
  xFree( m_apiBufU ); m_apiBufU = NULL;
  xFree( m_apiBufV ); m_apiBufV = NULL;
}

Void TComYuv::clear( UInt uiPartIdx, UInt uiSize )
{
  Pel* pY = getLumaAddr(uiPartIdx);
  Pel* pU = getCbAddr(uiPartIdx);
  Pel* pV = getCrAddr(uiPartIdx);
  UInt uiLSize = uiSize*sizeof(Pel);
  for(int i = 0; i < uiSize; i++)
  {
    memset( pY, 0, uiLSize);
    pY += getStride();
  }
  uiLSize >>= 1;
  uiSize  >>= 1;
  for(int i = 0; i < uiSize; i++)
  {
    memset( pU, 0, uiLSize);
    memset( pV, 0, uiLSize);
    pU += getCStride();
    pV += getCStride();
  }
}
Void TComYuv::clearY( UInt uiPartIdx, UInt uiSize )
{
  Pel* pY = getLumaAddr(uiPartIdx);
  UInt uiLSize = uiSize*sizeof(Pel);
  for(int i = 0; i < uiSize; i++)
  {
    memset( pY, 0, uiLSize);
    pY += getStride();
  }
}

Void TComYuv::clearU( UInt uiPartIdx, UInt uiSize )
{
  Pel* pU = getCbAddr(uiPartIdx);
  UInt uiLSize = uiSize*sizeof(Pel);
  for(int i = 0; i < uiSize; i++)
  {
    memset( pU, 0, uiLSize);
    pU += getCStride();
  }
}

Void TComYuv::clearV( UInt uiPartIdx, UInt uiSize )
{
  Pel* pV = getCrAddr(uiPartIdx);
  UInt uiLSize = uiSize*sizeof(Pel);
  for(int i = 0; i < uiSize; i++)
  {
    memset( pV, 0, uiLSize);
    pV += getCStride();
  }
}

Void TComYuv::copyToPicYuv   ( TComPicYuv* pcPicYuvDst, UInt iCuAddr, UInt uiAbsZorderIdx, UInt uiPartDepth, UInt uiPartIdx )
{
  copyToPicLuma  ( pcPicYuvDst, iCuAddr, uiAbsZorderIdx, uiPartDepth, uiPartIdx );
  copyToPicChroma( pcPicYuvDst, iCuAddr, uiAbsZorderIdx, uiPartDepth, uiPartIdx );
}

Void TComYuv::copyToPicLuma  ( TComPicYuv* pcPicYuvDst, UInt iCuAddr, UInt uiAbsZorderIdx, UInt uiPartDepth, UInt uiPartIdx )
{
  Int  y, iSize;

  iSize  = (m_iSize - m_uiExt) >>uiPartDepth;
  
  Pel* pSrc     = getLumaAddr(uiPartIdx);
  Pel* pDst     = pcPicYuvDst->getLumaAddr ( iCuAddr, uiAbsZorderIdx );
  
  UInt  iSrcStride  = getStride();
  UInt  iDstStride  = pcPicYuvDst->getStride();
  
  for ( y = iSize; y != 0; y-- )
  {
    ::memcpy( pDst, pSrc, sizeof(Pel)*iSize);
    pDst += iDstStride;
    pSrc += iSrcStride;
  }
}

Void TComYuv::copyToPicChroma( TComPicYuv* pcPicYuvDst, UInt iCuAddr, UInt uiAbsZorderIdx, UInt uiPartDepth, UInt uiPartIdx )
{
  Int  y, iSize;
#if BFIX
  iSize  = (m_iCSize - m_uiExt)>>uiPartDepth;
#else
  iSize  = m_iCSize >>uiPartDepth;
#endif
  
  Pel* pSrcU      = getCbAddr(uiPartIdx);
  Pel* pSrcV      = getCrAddr(uiPartIdx);
  Pel* pDstU      = pcPicYuvDst->getCbAddr( iCuAddr, uiAbsZorderIdx );
  Pel* pDstV      = pcPicYuvDst->getCrAddr( iCuAddr, uiAbsZorderIdx );
  
  UInt  iSrcStride = getCStride();
  UInt  iDstStride = pcPicYuvDst->getCStride();
  for ( y = iSize; y != 0; y-- )
  {
    ::memcpy( pDstU, pSrcU, sizeof(Pel)*(iSize) );
    ::memcpy( pDstV, pSrcV, sizeof(Pel)*(iSize) );
    pSrcU += iSrcStride;
    pSrcV += iSrcStride;
    pDstU += iDstStride;
    pDstV += iDstStride;
  }
}

Void TComYuv::copyFromPicYuv   ( TComPicYuv* pcPicYuvSrc, UInt iCuAddr, UInt uiAbsZorderIdx, UInt uiSize )
{
  copyFromPicLuma  ( pcPicYuvSrc, iCuAddr, uiAbsZorderIdx, uiSize );
  copyFromPicChroma( pcPicYuvSrc, iCuAddr, uiAbsZorderIdx, uiSize >> 1 );
}

Void TComYuv::copyFromPicLuma  ( TComPicYuv* pcPicYuvSrc, UInt iCuAddr, UInt uiAbsZorderIdx, UInt uiSize )
{
  Int  y;

  Pel* pDst     = this->getLumaAddr( uiAbsZorderIdx );
  Pel* pSrc     = pcPicYuvSrc->getLumaAddr ( iCuAddr, uiAbsZorderIdx );

  UInt  iDstStride  = getStride();
  UInt  iSrcStride  = pcPicYuvSrc->getStride();
  for ( y = uiSize; y != 0; y-- )
  {
    ::memcpy( pDst, pSrc, sizeof(Pel)*uiSize);
    pDst += iDstStride;
    pSrc += iSrcStride;
  }
}

Void TComYuv::copyFromPicChroma( TComPicYuv* pcPicYuvSrc, UInt iCuAddr, UInt uiAbsZorderIdx, UInt uiSize )
{
  Int  y;

  Pel* pDstU      = this->getCbAddr( uiAbsZorderIdx );
  Pel* pDstV      = this->getCrAddr( uiAbsZorderIdx );
  Pel* pSrcU      = pcPicYuvSrc->getCbAddr( iCuAddr, uiAbsZorderIdx );
  Pel* pSrcV      = pcPicYuvSrc->getCrAddr( iCuAddr, uiAbsZorderIdx );

  UInt  iDstStride = getCStride();
  UInt  iSrcStride = pcPicYuvSrc->getCStride();
  for ( y = uiSize; y != 0; y-- )
  {
    ::memcpy( pDstU, pSrcU, sizeof(Pel)*uiSize );
    ::memcpy( pDstV, pSrcV, sizeof(Pel)*uiSize );
    pSrcU += iSrcStride;
    pSrcV += iSrcStride;
    pDstU += iDstStride;
    pDstV += iDstStride;
  }
}

Void TComYuv::copyToPartYuv( TComYuv* pcYuvDst, UInt uiDstPartIdx )
{
  copyToPartLuma  ( pcYuvDst, uiDstPartIdx );
  copyToPartChroma( pcYuvDst, uiDstPartIdx );
}

Void TComYuv::copyToPartLuma( TComYuv* pcYuvDst, UInt uiDstPartIdx )
{
  Int  y;
  
  Pel* pSrc     = m_apiBufY;
  Pel* pDst     = pcYuvDst->getLumaAddr( uiDstPartIdx );
  
  UInt  iSrcStride  = getStride();
  UInt  iDstStride  = pcYuvDst->getStride();
  for ( y = m_iSize; y != 0; y-- )
  {
    ::memcpy( pDst, pSrc, sizeof(Pel)*m_iSize);
    pDst += iDstStride;
    pSrc += iSrcStride;
  }
}

Void TComYuv::copyToPartChroma( TComYuv* pcYuvDst, UInt uiDstPartIdx )
{
  Int  y;
  
  Pel* pSrcU      = m_apiBufU;
  Pel* pSrcV      = m_apiBufV;
  Pel* pDstU      = pcYuvDst->getCbAddr( uiDstPartIdx );
  Pel* pDstV      = pcYuvDst->getCrAddr( uiDstPartIdx );
  
  UInt  iSrcStride = getCStride();
  UInt  iDstStride = pcYuvDst->getCStride();
  for ( y = m_iCSize; y != 0; y-- )
  {
    ::memcpy( pDstU, pSrcU, sizeof(Pel)*(m_iCSize) );
    ::memcpy( pDstV, pSrcV, sizeof(Pel)*(m_iCSize) );
    pSrcU += iSrcStride;
    pSrcV += iSrcStride;
    pDstU += iDstStride;
    pDstV += iDstStride;
  }
}

Void TComYuv::copyPartToLuma( TComYuv* pcYuvDst, UInt uiSrcPartIdx, UInt uiSize )
{
  Int  y;

  Pel* pSrc     = getLumaAddr(uiSrcPartIdx);
  Pel* pDst     = pcYuvDst->getLumaAddr( uiSrcPartIdx );

  UInt  iSrcStride  = getStride();
  UInt  iDstStride  = pcYuvDst->getStride();

  for ( y = uiSize; y != 0; y-- )
  {
    ::memcpy( pDst, pSrc, sizeof(Pel)*uiSize);
    pDst += iDstStride;
    pSrc += iSrcStride;
  }
}
Void TComYuv::copyPartToYuv( TComYuv* pcYuvDst, UInt uiSrcPartIdx, UInt uiSize )
{
  Int  y;

  Pel* pSrcY = getLumaAddr(uiSrcPartIdx);
  Pel* pDstY = pcYuvDst->getLumaAddr( uiSrcPartIdx );

  Pel* pSrcCb = getCbAddr(uiSrcPartIdx);
  Pel* pDstCb = pcYuvDst->getCbAddr( uiSrcPartIdx );
  Pel* pSrcCr = getCrAddr(uiSrcPartIdx);
  Pel* pDstCr = pcYuvDst->getCrAddr( uiSrcPartIdx );

  UInt  iSrcStride  = getStride();
  UInt  iDstStride  = pcYuvDst->getStride();

  UInt uiLSize = sizeof(Pel)*uiSize;
  for ( y = uiSize; y != 0; y-- )
  {
    ::memcpy( pDstY, pSrcY, uiLSize);
    pDstY += iDstStride;
    pSrcY += iSrcStride;
  }

  uiLSize >>= 1;
  uiSize  >>= 1;
  iSrcStride = getCStride();
  iDstStride = pcYuvDst->getCStride();
  for ( y = uiSize; y != 0; y-- )
  {
    ::memcpy( pDstCb, pSrcCb, uiLSize);
    ::memcpy( pDstCr, pSrcCr, uiLSize);
    pDstCb += iDstStride;
    pDstCr += iDstStride;
    pSrcCb += iSrcStride;
    pSrcCr += iSrcStride;
  }
}

Void TComYuv::copyPartToPartYuv( TComYuv* pcYuvDst, UInt uiDstPartIdx, UInt uiPartIdx, Int iWidth, Int iHeight )
{
  copyPartToPartLuma   (pcYuvDst, uiDstPartIdx, uiPartIdx, iWidth, iHeight );
  copyPartToPartChroma (pcYuvDst, uiDstPartIdx, uiPartIdx, iWidth>>1, iHeight>>1 );
}

Void TComYuv::copyPartToPartLuma  ( TComYuv* pcYuvDst, UInt uiDstPartIdx, UInt uiPartIdx, Int iWidth, Int iHeight )
{
  Pel* pSrc =           getLumaAddr(uiPartIdx);
  Pel* pDst = pcYuvDst->getLumaAddr(uiDstPartIdx);
  if( pSrc == pDst )
  {
    //th not a good idea
    //th best would be to fix the caller 
    return ;
  }
  
  UInt  iSrcStride = getStride();
  UInt  iDstStride = pcYuvDst->getStride();

  UInt  uiPelSize  = iWidth * sizeof(Pel);

  for ( UInt y = iHeight; y != 0; y-- )
  {
    ::memcpy( pDst, pSrc, uiPelSize );
    pSrc += iSrcStride;
    pDst += iDstStride;
  }
}

Void TComYuv::copyPartToPartChroma( TComYuv* pcYuvDst, UInt uiDstPartIdx, UInt uiPartIdx, Int iWidth, Int iHeight )
{
  Pel*  pSrcU =           getCbAddr(uiPartIdx);
  Pel*  pSrcV =           getCrAddr(uiPartIdx);
  Pel*  pDstU = pcYuvDst->getCbAddr(uiDstPartIdx);
  Pel*  pDstV = pcYuvDst->getCrAddr(uiDstPartIdx);
  
  if( pSrcU == pDstU && pSrcV == pDstV)
  {
    //th not a good idea
    //th best would be to fix the caller 
    return ;
  }
  
  UInt   iSrcStride = getCStride();
  UInt   iDstStride = pcYuvDst->getCStride();

  UInt   uiPelSize  = iWidth * sizeof(Pel);

  for ( UInt y = iHeight; y != 0; y-- )
  {
    ::memcpy( pDstU, pSrcU, uiPelSize );
    ::memcpy( pDstV, pSrcV, uiPelSize );
    pSrcU += iSrcStride;
    pSrcV += iSrcStride;
    pDstU += iDstStride;
    pDstV += iDstStride;
  }
}
#if inter_direct_skip_bug
Void TComYuv::addClipSkip(TComYuv* pcYuvSrc0, UInt uiTrUnitIdx, UInt uiPartSize)
{
	addClipLumaSkip(pcYuvSrc0, uiTrUnitIdx, uiPartSize);
	addClipChromaSkip(pcYuvSrc0, uiTrUnitIdx, uiPartSize >> 1);
}

Void TComYuv::addClipLumaSkip(TComYuv* pcYuvSrc0, UInt uiTrUnitIdx, UInt uiPartSize)
{
	Int x, y;

	Pel* pSrc0 = pcYuvSrc0->getLumaAddr(uiTrUnitIdx);
	
	Pel* pDst = getLumaAddr(uiTrUnitIdx);
	UInt iSrc0Stride = pcYuvSrc0->getStride();

	UInt iDstStride = getStride();
	for (y = uiPartSize - 1; y >= 0; y--)
	{
		for (x = uiPartSize - 1; x >= 0; x--)
		{
			pDst[x] = xClip(pSrc0[x] );
		}
		pSrc0 += iSrc0Stride;
		pDst += iDstStride;
	}
}

Void TComYuv::addClipChromaSkip(TComYuv* pcYuvSrc0, UInt uiTrUnitIdx, UInt uiPartSize)
{
	Int x, y;

	Pel* pSrcU0 = pcYuvSrc0->getCbAddr(uiTrUnitIdx);

	Pel* pSrcV0 = pcYuvSrc0->getCrAddr(uiTrUnitIdx);

	Pel* pDstU = getCbAddr(uiTrUnitIdx);
	Pel* pDstV = getCrAddr(uiTrUnitIdx);
	UInt  iSrc0Stride = pcYuvSrc0->getCStride();
	
	UInt  iDstStride = getCStride();
	for (y = uiPartSize - 1; y >= 0; y--)
	{
		for (x = uiPartSize - 1; x >= 0; x--)
		{
			pDstU[x] = xClip(pSrcU0[x] );
			pDstV[x] = xClip(pSrcV0[x] );
		}

		pSrcU0 += iSrc0Stride;
		pSrcV0 += iSrc0Stride;
		pDstU += iDstStride;
		pDstV += iDstStride;
	}
}
#endif

Void TComYuv::addClip( TComYuv* pcYuvSrc0, TComYuv* pcYuvSrc1, UInt uiTrUnitIdx, UInt uiPartSize )
{
  addClipLuma   ( pcYuvSrc0, pcYuvSrc1, uiTrUnitIdx, uiPartSize     );
  addClipChroma ( pcYuvSrc0, pcYuvSrc1, uiTrUnitIdx, uiPartSize>>1  );
}

Void TComYuv::addClipLuma( TComYuv* pcYuvSrc0, TComYuv* pcYuvSrc1, UInt uiTrUnitIdx, UInt uiPartSize )
{
  Int x, y;
  
  Pel* pSrc0 = pcYuvSrc0->getLumaAddr( uiTrUnitIdx );
  Pel* pSrc1 = pcYuvSrc1->getLumaAddr( uiTrUnitIdx );
  Pel* pDst  = getLumaAddr( uiTrUnitIdx );
  UInt iSrc0Stride = pcYuvSrc0->getStride();
  UInt iSrc1Stride = pcYuvSrc1->getStride();
  UInt iDstStride  = getStride();
  for ( y = uiPartSize-1; y >= 0; y-- )
  {
    for ( x = uiPartSize-1; x >= 0; x-- )
    {
      pDst[x] = xClip( pSrc0[x] + pSrc1[x] );
    }
    pSrc0 += iSrc0Stride;
    pSrc1 += iSrc1Stride;
    pDst  += iDstStride;
  }
}

Void TComYuv::addClipChroma( TComYuv* pcYuvSrc0, TComYuv* pcYuvSrc1, UInt uiTrUnitIdx, UInt uiPartSize )
{
  Int x, y;
  
  Pel* pSrcU0 = pcYuvSrc0->getCbAddr( uiTrUnitIdx );
  Pel* pSrcU1 = pcYuvSrc1->getCbAddr( uiTrUnitIdx );
  Pel* pSrcV0 = pcYuvSrc0->getCrAddr( uiTrUnitIdx );
  Pel* pSrcV1 = pcYuvSrc1->getCrAddr( uiTrUnitIdx );
  Pel* pDstU = getCbAddr( uiTrUnitIdx );
  Pel* pDstV = getCrAddr( uiTrUnitIdx );
  UInt  iSrc0Stride = pcYuvSrc0->getCStride();
  UInt  iSrc1Stride = pcYuvSrc1->getCStride();
  UInt  iDstStride  = getCStride();
  for ( y = uiPartSize-1; y >= 0; y-- )
  {
    for ( x = uiPartSize-1; x >= 0; x-- )
    {
      pDstU[x] = xClip( pSrcU0[x] + pSrcU1[x] );
      pDstV[x] = xClip( pSrcV0[x] + pSrcV1[x] );
    }
    
    pSrcU0 += iSrc0Stride;
    pSrcU1 += iSrc1Stride;
    pSrcV0 += iSrc0Stride;
    pSrcV1 += iSrc1Stride;
    pDstU  += iDstStride;
    pDstV  += iDstStride;
  }
}

Void TComYuv::subtract( TComYuv* pcYuvSrc0, TComYuv* pcYuvSrc1, UInt uiTrUnitIdx, UInt uiPartSize )
{
  subtractLuma  ( pcYuvSrc0, pcYuvSrc1,  uiTrUnitIdx, uiPartSize    );
  subtractChroma( pcYuvSrc0, pcYuvSrc1,  uiTrUnitIdx, uiPartSize>>1 );
}

Void TComYuv::subtractLuma( TComYuv* pcYuvSrc0, TComYuv* pcYuvSrc1, UInt uiTrUnitIdx, UInt uiPartSize )
{
  Int x, y;
  
  Pel* pSrc0 = pcYuvSrc0->getLumaAddr( uiTrUnitIdx );
  Pel* pSrc1 = pcYuvSrc1->getLumaAddr( uiTrUnitIdx );
  Pel* pDst  = getLumaAddr( uiTrUnitIdx );
  Int  iSrc0Stride = pcYuvSrc0->getStride();
  Int  iSrc1Stride = pcYuvSrc1->getStride();
  Int  iDstStride  = getStride();
  for ( y = uiPartSize-1; y >= 0; y-- )
  {
    for ( x = uiPartSize-1; x >= 0; x-- )
    {

      pDst[x] = pSrc0[x] - pSrc1[x];
    }
    pSrc0 += iSrc0Stride;
    pSrc1 += iSrc1Stride;
    pDst  += iDstStride;
  }
}

Void TComYuv::subtractChroma( TComYuv* pcYuvSrc0, TComYuv* pcYuvSrc1, UInt uiTrUnitIdx, UInt uiPartSize )
{
  Int x, y;

  Pel* pSrcU0 = pcYuvSrc0->getCbAddr( uiTrUnitIdx );
  Pel* pSrcU1 = pcYuvSrc1->getCbAddr( uiTrUnitIdx );
  Pel* pSrcV0 = pcYuvSrc0->getCrAddr( uiTrUnitIdx );
  Pel* pSrcV1 = pcYuvSrc1->getCrAddr( uiTrUnitIdx );
  Pel* pDstU  = getCbAddr( uiTrUnitIdx );
  Pel* pDstV  = getCrAddr( uiTrUnitIdx );

  Int  iSrc0Stride = pcYuvSrc0->getCStride();
  Int  iSrc1Stride = pcYuvSrc1->getCStride();
  Int  iDstStride  = getCStride();
  for ( y = uiPartSize-1; y >= 0; y-- )
  {
    for ( x = uiPartSize-1; x >= 0; x-- )
    {
      pDstU[x] = pSrcU0[x] - pSrcU1[x];
      pDstV[x] = pSrcV0[x] - pSrcV1[x];
    }
    pSrcU0 += iSrc0Stride;
    pSrcU1 += iSrc1Stride;
    pSrcV0 += iSrc0Stride;
    pSrcV1 += iSrc1Stride;
    pDstU  += iDstStride;
    pDstV  += iDstStride;
  }
}

Void TComYuv::addAvg( TComYuv* pcYuvSrc0, TComYuv* pcYuvSrc1, UInt iPartUnitIdx, Int iWidth, Int iHeight, UInt uiDstPartIdx )
{
  Int x, y;
  
  Pel* pSrcY0  = pcYuvSrc0->getLumaAddr( iPartUnitIdx );
  Pel* pSrcU0  = pcYuvSrc0->getCbAddr  ( iPartUnitIdx );
  Pel* pSrcV0  = pcYuvSrc0->getCrAddr  ( iPartUnitIdx );
  
  Pel* pSrcY1  = pcYuvSrc1->getLumaAddr( iPartUnitIdx );
  Pel* pSrcU1  = pcYuvSrc1->getCbAddr  ( iPartUnitIdx );
  Pel* pSrcV1  = pcYuvSrc1->getCrAddr  ( iPartUnitIdx );
  
  Pel* pDstY   = getLumaAddr( uiDstPartIdx );
  Pel* pDstU   = getCbAddr  ( uiDstPartIdx );
  Pel* pDstV   = getCrAddr  ( uiDstPartIdx );
  
  UInt  iSrc0Stride = pcYuvSrc0->getStride();
  UInt  iSrc1Stride = pcYuvSrc1->getStride();
  UInt  iDstStride  = getStride();
  
  for ( y = iHeight-1; y >= 0; y-- )
  {
    for ( x = iWidth-1; x >= 0; )
    {
      // note: luma min width is 4
      pDstY[x] = (pSrcY0[x] + pSrcY1[x] + 1) >> 1; x--;
      pDstY[x] = (pSrcY0[x] + pSrcY1[x] + 1) >> 1; x--;
      pDstY[x] = (pSrcY0[x] + pSrcY1[x] + 1) >> 1; x--;
      pDstY[x] = (pSrcY0[x] + pSrcY1[x] + 1) >> 1; x--;
    }
    pSrcY0 += iSrc0Stride;
    pSrcY1 += iSrc1Stride;
    pDstY  += iDstStride;
  }
  
  iSrc0Stride = pcYuvSrc0->getCStride();
  iSrc1Stride = pcYuvSrc1->getCStride();
  iDstStride  = getCStride();
  
  iWidth  >>=1;
  iHeight >>=1;

  for ( y = iHeight-1; y >= 0; y-- )
  {
    for ( x = iWidth-1; x >= 0; )
    {
      // note: chroma min width is 2
      pDstU[x] = (pSrcU0[x] + pSrcU1[x] + 1) >> 1;
      pDstV[x] = (pSrcV0[x] + pSrcV1[x] + 1) >> 1; x--;
      pDstU[x] = (pSrcU0[x] + pSrcU1[x] + 1) >> 1;
      pDstV[x] = (pSrcV0[x] + pSrcV1[x] + 1) >> 1; x--;
    }
    
    pSrcU0 += iSrc0Stride;
    pSrcU1 += iSrc1Stride;
    pSrcV0 += iSrc0Stride;
    pSrcV1 += iSrc1Stride;
    pDstU  += iDstStride;
    pDstV  += iDstStride;
  }
}

Void TComYuv::removeHighFreq( TComYuv* pcYuvSrc, UInt uiPartIdx, UInt uiWidth, UInt uiHeight )
{
  Int x, y;
  
  Pel* pSrc  = pcYuvSrc->getLumaAddr(uiPartIdx);
  Pel* pSrcU = pcYuvSrc->getCbAddr(uiPartIdx);
  Pel* pSrcV = pcYuvSrc->getCrAddr(uiPartIdx);
  
  Pel* pDst  = getLumaAddr(uiPartIdx);
  Pel* pDstU = getCbAddr(uiPartIdx);
  Pel* pDstV = getCrAddr(uiPartIdx);
  
  Int  iSrcStride = pcYuvSrc->getStride();
  Int  iDstStride = getStride();
  
  for ( y = uiHeight-1; y >= 0; y-- )
  {
    for ( x = uiWidth-1; x >= 0; x-- )
    {
      pDst[x ] = xClip( (pDst[x ]<<1) - pSrc[x ] );
    }
    pSrc += iSrcStride;
    pDst += iDstStride;
  }
  
  iSrcStride = pcYuvSrc->getCStride();
  iDstStride = getCStride();
  
  uiWidth >>= 1;
  uiHeight >>= 1;
  
  for ( y = uiHeight-1; y >= 0; y-- )
  {
    for ( x = uiWidth-1; x >= 0; x-- )
    {
      pDstU[x ] = xClip( (pDstU[x ]<<1) - pSrcU[x ] );
      pDstV[x ] = xClip( (pDstV[x ]<<1) - pSrcV[x ] );
    }
    pSrcU += iSrcStride;
    pSrcV += iSrcStride;
    pDstU += iDstStride;
    pDstV += iDstStride;
  }
}

#if DCTIF
Pel* TComYuv::getRealLumaAddr(UInt uiPartUnitIdx)
{
	UInt iBlkX;
	UInt iBlkY;
	iBlkX = g_auiRasterToPelX[g_auiZscanToRaster[uiPartUnitIdx]];
	iBlkY = g_auiRasterToPelY[g_auiZscanToRaster[uiPartUnitIdx]];

	return m_apiBufY + iBlkY*getRealStride() + iBlkX;
}

Pel* TComYuv::getRealCbAddr(UInt uiPartUnitIdx)
{
	UInt iBlkX;
	UInt iBlkY;
	iBlkX = g_auiRasterToPelX[g_auiZscanToRaster[uiPartUnitIdx]];
	iBlkY = g_auiRasterToPelY[g_auiZscanToRaster[uiPartUnitIdx]];

	return m_apiBufU + ((iBlkY*getRealCStride() + iBlkX) >> 1);
}

Pel* TComYuv::getRealCrAddr(UInt uiPartUnitIdx)
{
	UInt iBlkX;
	UInt iBlkY;
	iBlkX = g_auiRasterToPelX[g_auiZscanToRaster[uiPartUnitIdx]];
	iBlkY = g_auiRasterToPelY[g_auiZscanToRaster[uiPartUnitIdx]];

	return m_apiBufV + ((iBlkY*getRealCStride() + iBlkX) >> 1);
}
#endif

Pel* TComYuv::getLumaAddr( UInt uiPartUnitIdx )
{
  UInt iBlkX;
  UInt iBlkY;
  iBlkX = g_auiRasterToPelX[g_auiZscanToRaster[uiPartUnitIdx]];
  iBlkY = g_auiRasterToPelY[g_auiZscanToRaster[uiPartUnitIdx]];
  
  return m_apiBufY + iBlkY*getStride() + iBlkX;
}

Pel* TComYuv::getCbAddr( UInt uiPartUnitIdx )
{
  UInt iBlkX;
  UInt iBlkY;
  iBlkX = g_auiRasterToPelX[g_auiZscanToRaster[uiPartUnitIdx]];
  iBlkY = g_auiRasterToPelY[g_auiZscanToRaster[uiPartUnitIdx]];
  
  return m_apiBufU + ((iBlkY*getCStride() + iBlkX)>>1);
}

Pel* TComYuv::getCrAddr( UInt uiPartUnitIdx )
{
  UInt iBlkX;
  UInt iBlkY;
  iBlkX = g_auiRasterToPelX[g_auiZscanToRaster[uiPartUnitIdx]];
  iBlkY = g_auiRasterToPelY[g_auiZscanToRaster[uiPartUnitIdx]];
  
  return m_apiBufV + ((iBlkY*getCStride() + iBlkX)>>1);
}

Pel* TComYuv::getLumaAddr( UInt iTransUnitIdx, UInt iBlkSize )
{
  UInt uiNumTrInWidth = m_iSize / iBlkSize;
  UInt   iBlkX   = iTransUnitIdx % uiNumTrInWidth;
  UInt   iBlkY   = iTransUnitIdx / uiNumTrInWidth;
  
  return m_apiBufY + (iBlkX + iBlkY * getStride()) * iBlkSize;
}

Pel* TComYuv::getCbAddr( UInt iTransUnitIdx, UInt iBlkSize )
{
  UInt uiNumTrInWidth = m_iCSize / iBlkSize;
  UInt   iBlkX   = iTransUnitIdx % uiNumTrInWidth;
  UInt   iBlkY   = iTransUnitIdx / uiNumTrInWidth;
  
  return m_apiBufU + (iBlkX + iBlkY * getCStride()) * iBlkSize;
}

Pel* TComYuv::getCrAddr( UInt iTransUnitIdx, UInt iBlkSize )
{
  UInt uiNumTrInWidth = m_iCSize / iBlkSize;
  UInt   iBlkX   = iTransUnitIdx % uiNumTrInWidth;
  UInt   iBlkY   = iTransUnitIdx / uiNumTrInWidth;
  
  return m_apiBufV + (iBlkX + iBlkY * getCStride()) * iBlkSize;
}

