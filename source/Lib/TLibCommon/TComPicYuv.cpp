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

/** \file     TComPicYuv.cpp
    \brief    picture YUV buffer class
*/

#include <cstdlib>
#include <assert.h>
#include <memory.h>

#ifdef __APPLE__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

#include "TComPicYuv.h"

TComPicYuv::TComPicYuv()
{
  m_apiPicBufY      = NULL;   // Buffer (including margin)
  m_apiPicBufU      = NULL;
  m_apiPicBufV      = NULL;
  
  m_piPicOrgY       = NULL;    // m_apiPicBufY + m_iMarginLuma*getStride() + m_iMarginLuma
  m_piPicOrgU       = NULL;
  m_piPicOrgV       = NULL;
  
  m_bIsBorderExtended = false;
}

TComPicYuv::~TComPicYuv()
{
}

Void TComPicYuv::create( Int iPicWidth, Int iPicHeight, UInt uiLog2MaxCUSize, UInt uiMaxCUDepth )
{
  m_iPicWidth       = iPicWidth;
  m_iPicHeight      = iPicHeight;
  
  // --> After config finished!
  m_iLog2CuSize     = uiLog2MaxCUSize;

  m_iNumCuInWidth   = m_iPicWidth >> m_iLog2CuSize;
  m_iNumCuInWidth  += ( m_iPicWidth & ((1<<uiLog2MaxCUSize)-1) ) ? 1 : 0;
  
  m_iLog2BaseUnitSize  = uiLog2MaxCUSize - uiMaxCUDepth;
#if wlq_FME
  m_iLumaMargin     = 64; // up to 8-tap
  m_iChromaMargin   = 64 >> 1;
#else
  m_iLumaMargin     = (1<<g_uiLog2MaxCUSize)  + 8; // up to 8-tap
  m_iChromaMargin   = m_iLumaMargin>>1;
#endif
  
#if YUVMargin_debug
  m_iLumaMargin =64 + 8; // up to 8-tap
  m_iChromaMargin = m_iLumaMargin >> 1; 
#else
  m_iLumaMargin = (1 << g_uiLog2MaxCUSize) + 8; // up to 8-tap
  m_iChromaMargin = m_iLumaMargin >> 1;


#endif

  m_apiPicBufY      = (Pel*)xMalloc( Pel, ( m_iPicWidth       + (m_iLumaMargin  <<1)) * ( m_iPicHeight       + (m_iLumaMargin  <<1)));
  m_apiPicBufU      = (Pel*)xMalloc( Pel, ((m_iPicWidth >> 1) + (m_iChromaMargin<<1)) * ((m_iPicHeight >> 1) + (m_iChromaMargin<<1)) + 1);
  m_apiPicBufV      = (Pel*)xMalloc( Pel, ((m_iPicWidth >> 1) + (m_iChromaMargin<<1)) * ((m_iPicHeight >> 1) + (m_iChromaMargin<<1)) + 1);

  m_piPicOrgY       = m_apiPicBufY + m_iLumaMargin   * getStride()  + m_iLumaMargin;
  m_piPicOrgU       = m_apiPicBufU + m_iChromaMargin * getCStride() + m_iChromaMargin;
  m_piPicOrgV       = m_apiPicBufV + m_iChromaMargin * getCStride() + m_iChromaMargin;
  
  m_bIsBorderExtended = false;

  return;
}

Void TComPicYuv::reset()
{
  Pel* p = m_apiPicBufY;
  UInt uiX, uiY;
  for( uiY = 0; uiY < (m_iPicHeight+(m_iLumaMargin<<1)); uiY++ )
  {
    for( uiX = 0; uiX < (m_iPicWidth+(m_iLumaMargin<<1)); uiX++ )
    {
      p[uiX] = 128;
    }
    p += (m_iPicWidth+(m_iLumaMargin<<1));
  }
  Pel* p1 = m_apiPicBufU;
  Pel* p2 = m_apiPicBufV;
  for( uiY = 0; uiY < ((m_iPicHeight >> 1) + (m_iChromaMargin<<1)); uiY++ )
  {
    for( uiX = 0; uiX < ((m_iPicWidth >> 1) + (m_iChromaMargin<<1)); uiX++ )
    {
      p1[uiX] = 128;
      p2[uiX] = 128;
    }
    p1 += ((m_iPicWidth >> 1) + (m_iChromaMargin<<1));
    p2 += ((m_iPicWidth >> 1) + (m_iChromaMargin<<1));
  }
  p1[0] = 128;
  p2[0] = 128;
}

Void TComPicYuv::destroy()
{
  m_piPicOrgY       = NULL;
  m_piPicOrgU       = NULL;
  m_piPicOrgV       = NULL;
  
  if( m_apiPicBufY ){ xFree( m_apiPicBufY );    m_apiPicBufY = NULL; }
  if( m_apiPicBufU ){ xFree( m_apiPicBufU );    m_apiPicBufU = NULL; }
  if( m_apiPicBufV ){ xFree( m_apiPicBufV );    m_apiPicBufV = NULL; }
}

Void TComPicYuv::createLuma( Int iPicWidth, Int iPicHeight, UInt uiLog2MaxCUSize, UInt uiMaxCUDepth )
{
  m_iPicWidth       = iPicWidth;
  m_iPicHeight      = iPicHeight;
  
  // --> After config finished!
  m_iLog2CuSize     = uiLog2MaxCUSize;
  
  m_iNumCuInWidth   = m_iPicWidth >> m_iLog2CuSize;
  m_iNumCuInWidth  += ( m_iPicWidth & ((1<<m_iLog2CuSize)-1) ) ? 1 : 0;
  
  m_iLog2BaseUnitSize  = uiLog2MaxCUSize - uiMaxCUDepth;
  m_iLumaMargin    = (1<<g_uiLog2MaxCUSize)  + 8; // up to 8-tap
  
  m_apiPicBufY      = (Pel*)xMalloc( Pel, ( m_iPicWidth + (m_iLumaMargin  <<1)) * ( m_iPicHeight + (m_iLumaMargin  <<1)));
  m_piPicOrgY       = m_apiPicBufY + m_iLumaMargin   * getStride()  + m_iLumaMargin;
  
  return;
}

Void TComPicYuv::destroyLuma()
{
  m_piPicOrgY       = NULL;
  
  if( m_apiPicBufY ){ xFree( m_apiPicBufY );    m_apiPicBufY = NULL; }
}

Pel*  TComPicYuv::getLumaAddr( int iCuAddr )
{
  Int iCuX = iCuAddr % m_iNumCuInWidth;
  Int iCuY = iCuAddr / m_iNumCuInWidth;
  
  return ( m_piPicOrgY + ((iCuY*getStride() + iCuX)<<m_iLog2CuSize) );
}

Pel*  TComPicYuv::getLumaAddr( Int iCuAddr, Int uiAbsZorderIdx )
{
  Int iCuX           = iCuAddr % m_iNumCuInWidth;
  Int iCuY           = iCuAddr / m_iNumCuInWidth;
  Int iOffsetCu      = (iCuY*getStride() + iCuX)<<m_iLog2CuSize;
  
  Int iLog2CuSizeInBases = m_iLog2CuSize - m_iLog2BaseUnitSize;
  Int iRastPartIdx   = g_auiZscanToRaster[uiAbsZorderIdx];
  Int iBaseX         = iRastPartIdx & ((1<<iLog2CuSizeInBases)-1);
  Int iBaseY         = iRastPartIdx >> iLog2CuSizeInBases;
  Int iOffsetBase    = (iBaseY*getStride() + iBaseX)<<m_iLog2BaseUnitSize;
  
  return (m_piPicOrgY + iOffsetCu + iOffsetBase);
}

#if ZHANGYI_INTRA_SDIP
Pel*  TComPicYuv::getLineLumaAddr(Int iCuAddr, Int uiAbsZorderIdx, UInt uiLine, UInt uiDirection)
{
	Int iCuX = iCuAddr % m_iNumCuInWidth;
	Int iCuY = iCuAddr / m_iNumCuInWidth;
	Int iOffsetCu = iCuY*(1 << m_iLog2CuSize)*getStride() + iCuX*(1 << m_iLog2CuSize);

	Int iCuSizeInBases = (1 << m_iLog2CuSize) / (1 << m_iLog2BaseUnitSize);  //zhengyiCheckSDIP
	Int iRastPartIdx = g_auiZscanToRaster[uiAbsZorderIdx];
	Int iBaseX = iRastPartIdx % iCuSizeInBases;
	Int iBaseY = iRastPartIdx / iCuSizeInBases;
	//Int iOffsetBase    = iBaseY*m_iBaseUnitHeight*getStride() + iBaseX*m_iBaseUnitWidth;
	Int iOffsetBase = iBaseY*(1 << m_iLog2BaseUnitSize)*getStride() + iBaseX*(1 << m_iLog2BaseUnitSize);//zhengyiCheckSDIP: (1<<minCuSize)
	Int iOffsetLine = uiDirection ? uiLine*getStride() : uiLine;

	return (m_piPicOrgY + iOffsetCu + iOffsetBase + iOffsetLine);

}
#endif

Pel*  TComPicYuv::getCbAddr( int iCuAddr )
{
  Int iCuX = iCuAddr % m_iNumCuInWidth;
  Int iCuY = iCuAddr / m_iNumCuInWidth;
  
  return ( m_piPicOrgU + ( (iCuY*getCStride() + iCuX)<<(m_iLog2CuSize-1) ) );
}

Pel*  TComPicYuv::getCbAddr( Int iCuAddr, Int uiAbsZorderIdx )
{
  Int iCuX           = iCuAddr % m_iNumCuInWidth;
  Int iCuY           = iCuAddr / m_iNumCuInWidth;
  Int iOffsetCu      = (iCuY*getCStride() + iCuX)<<m_iLog2CuSize;
  
  Int iLog2CuSizeInBases = m_iLog2CuSize - m_iLog2BaseUnitSize;
  Int iRastPartIdx   = g_auiZscanToRaster[uiAbsZorderIdx];
  Int iBaseX         = iRastPartIdx & ((1<<iLog2CuSizeInBases)-1);
  Int iBaseY         = iRastPartIdx >> iLog2CuSizeInBases;
  Int iOffsetBase    = (iBaseY*getCStride() + iBaseX)<<m_iLog2BaseUnitSize;
  
  return (m_piPicOrgU + ( (iOffsetCu + iOffsetBase)>>1 ) );
}

Pel*  TComPicYuv::getCrAddr( int iCuAddr )
{
  Int iCuX = iCuAddr % m_iNumCuInWidth;
  Int iCuY = iCuAddr / m_iNumCuInWidth;
  
  return ( m_piPicOrgV + ( (iCuY*getCStride() + iCuX)<<(m_iLog2CuSize-1) ) );
}

Pel*  TComPicYuv::getCrAddr( Int iCuAddr, Int uiAbsZorderIdx )
{
  Int iCuX           = iCuAddr % m_iNumCuInWidth;
  Int iCuY           = iCuAddr / m_iNumCuInWidth;
  Int iOffsetCu      = (iCuY*getCStride() + iCuX)<<m_iLog2CuSize;
  
  Int iLog2CuSizeInBases = m_iLog2CuSize - m_iLog2BaseUnitSize;
  Int iRastPartIdx   = g_auiZscanToRaster[uiAbsZorderIdx];
  Int iBaseX         = iRastPartIdx & ((1<<iLog2CuSizeInBases)-1);
  Int iBaseY         = iRastPartIdx >> iLog2CuSizeInBases;
  Int iOffsetBase    = (iBaseY*getCStride() + iBaseX)<<m_iLog2BaseUnitSize;
  
  return (m_piPicOrgV + ( (iOffsetCu + iOffsetBase)>>1 ) );
}

Void  TComPicYuv::copyToPic (TComPicYuv*  pcPicYuvDst)
{
  assert( m_iPicWidth  == pcPicYuvDst->getWidth()  );
  assert( m_iPicHeight == pcPicYuvDst->getHeight() );
  
  ::memcpy ( pcPicYuvDst->getBufY(), m_apiPicBufY, sizeof (Pel) * ( m_iPicWidth       + (m_iLumaMargin   << 1)) * ( m_iPicHeight       + (m_iLumaMargin   << 1)) );
  ::memcpy ( pcPicYuvDst->getBufU(), m_apiPicBufU, sizeof (Pel) * ((m_iPicWidth >> 1) + (m_iChromaMargin << 1)) * ((m_iPicHeight >> 1) + (m_iChromaMargin << 1)) );
  ::memcpy ( pcPicYuvDst->getBufV(), m_apiPicBufV, sizeof (Pel) * ((m_iPicWidth >> 1) + (m_iChromaMargin << 1)) * ((m_iPicHeight >> 1) + (m_iChromaMargin << 1)) );

  return;
}

Void  TComPicYuv::copyToPicLuma (TComPicYuv*  pcPicYuvDst)
{
  assert( m_iPicWidth  == pcPicYuvDst->getWidth()  );
  assert( m_iPicHeight == pcPicYuvDst->getHeight() );
  
  ::memcpy ( pcPicYuvDst->getBufY(), m_apiPicBufY, sizeof (Pel) * ( m_iPicWidth       + (m_iLumaMargin   << 1)) * ( m_iPicHeight       + (m_iLumaMargin   << 1)) );
  return;
}

Void  TComPicYuv::copyToPicCb (TComPicYuv*  pcPicYuvDst)
{
  assert( m_iPicWidth  == pcPicYuvDst->getWidth()  );
  assert( m_iPicHeight == pcPicYuvDst->getHeight() );
  
  ::memcpy ( pcPicYuvDst->getBufU(), m_apiPicBufU, sizeof (Pel) * ((m_iPicWidth >> 1) + (m_iChromaMargin << 1)) * ((m_iPicHeight >> 1) + (m_iChromaMargin << 1)) );
  return;
}

Void  TComPicYuv::copyToPicCr (TComPicYuv*  pcPicYuvDst)
{
  assert( m_iPicWidth  == pcPicYuvDst->getWidth()  );
  assert( m_iPicHeight == pcPicYuvDst->getHeight() );
  
  ::memcpy ( pcPicYuvDst->getBufV(), m_apiPicBufV, sizeof (Pel) * ((m_iPicWidth >> 1) + (m_iChromaMargin << 1)) * ((m_iPicHeight >> 1) + (m_iChromaMargin << 1)) );
  return;
}


Void TComPicYuv::getLumaMinMax( Int *pMin, Int *pMax )
{
  Pel*  piY   = getLumaAddr();
  Int   iMin  = (1<<(g_uiBitDepth))-1;
  Int   iMax  = 0;
  Int   x, y;
  
  for ( y = 0; y < m_iPicHeight; y++ )
  {
    for ( x = 0; x < m_iPicWidth; x++ )
    {
      if ( piY[x] < iMin ) iMin = piY[x];
      if ( piY[x] > iMax ) iMax = piY[x];
    }
    piY += getStride();
  }
  
  *pMin = iMin;
  *pMax = iMax;
}

Void TComPicYuv::extendPicBorder ()
{
  if ( m_bIsBorderExtended ) return;
  
  xExtendPicCompBorder( getLumaAddr(), getStride(),  getWidth(),      getHeight(),      m_iLumaMargin,   m_iLumaMargin   );
  xExtendPicCompBorder( getCbAddr()  , getCStride(), getWidth() >> 1, getHeight() >> 1, m_iChromaMargin, m_iChromaMargin );
  xExtendPicCompBorder( getCrAddr()  , getCStride(), getWidth() >> 1, getHeight() >> 1, m_iChromaMargin, m_iChromaMargin );
  
  m_bIsBorderExtended = true;
}

Void TComPicYuv::xExtendPicCompBorder  (Pel* piTxt, Int iStride, Int iWidth, Int iHeight, Int iMarginX, Int iMarginY)
{
  Int   x, y;
  Pel*  pi;
  
  pi = piTxt;
  for ( y = 0; y < iHeight; y++)
  {
    for ( x = 0; x < iMarginX; x++ )
    {
      pi[ -iMarginX + x ] = pi[0];
      pi[    iWidth + x ] = pi[iWidth-1];
    }
    pi += iStride;
  }
  
  pi -= (iStride + iMarginX);
  for ( y = 0; y < iMarginY; y++ )
  {
    ::memcpy( pi + (y+1)*iStride, pi, sizeof(Pel)*(iWidth + (iMarginX<<1)) );
  }
  
  pi -= ((iHeight-1) * iStride);
  for ( y = 0; y < iMarginY; y++ )
  {
    ::memcpy( pi - (y+1)*iStride, pi, sizeof(Pel)*(iWidth + (iMarginX<<1)) );
  }
}


Void TComPicYuv::dump (char* pFileName, Bool bAdd)
{
  FILE* pFile;
  if (!bAdd)
  {
    pFile = fopen (pFileName, "wb");
  }
  else
  {
    pFile = fopen (pFileName, "ab");
  }
  
  Int   x, y;
  UChar uc;
  
  Pel*  piY   = getLumaAddr();
  Pel*  piCb  = getCbAddr();
  Pel*  piCr  = getCrAddr();
  
  Pel  iMax = ((1<<(g_uiBitDepth))-1);
  
  for ( y = 0; y < m_iPicHeight; y++ )
  {
    for ( x = 0; x < m_iPicWidth; x++ )
    {
      uc = (UChar)Clip3(0, iMax, piY[x]);
      
      fwrite( &uc, sizeof(UChar), 1, pFile );
    }
    piY += getStride();
  }
  
  for ( y = 0; y < m_iPicHeight >> 1; y++ )
  {
    for ( x = 0; x < m_iPicWidth >> 1; x++ )
    {
      uc = (UChar)Clip3(0, iMax, piCb[x]);
      fwrite( &uc, sizeof(UChar), 1, pFile );
    }
    piCb += getCStride();
  }
  
  for ( y = 0; y < m_iPicHeight >> 1; y++ )
  {
    for ( x = 0; x < m_iPicWidth >> 1; x++ )
    {
      uc = (UChar)Clip3(0, iMax, piCr[x]);
      fwrite( &uc, sizeof(UChar), 1, pFile );
    }
    piCr += getCStride();
  }
  
  fclose(pFile);
}

