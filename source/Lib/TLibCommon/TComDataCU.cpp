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

/** \file     TComDataCU.cpp
    \brief    CU data structure
    \todo     not all entities are documented
*/

#include "TComDataCU.h"
#include "TComPic.h"
#if ZHANGYI_INTRA_SDIP
#include "TComYuv.h"
#endif
// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TComDataCU::TComDataCU()
{
  m_pcPic              = NULL;
  m_pcPicture          = NULL;
  m_puhDepth           = NULL;
  
  m_pePartSize         = NULL;

  m_pePredMode         = NULL;
  m_puhLog2CUSize      = NULL;

  m_puhWidth           = NULL;
  m_puhHeight          = NULL;

  m_puhInterDir        = NULL;
  m_puhTrIdx           = NULL;

  m_puhIntraDir        = NULL;

#if inter_direct_skip_bug1
  m_pcInterSkipmode = NULL;
#endif

#if ZHANGYI_INTRA
  m_puhIntraDirCb = NULL;
  m_puhIntraDirCr = NULL;
  m_puhIntraDirValue = NULL;
  m_puhIntraDirCbValue = NULL;
  m_puhIntraDirCrValue = NULL;
#endif
#if ZHANGYI_INTRA_SDIP

 
  m_pSDIPFlag = NULL;
  m_pSDIPDirection = NULL;

#endif

  m_puhCbf[0]          = NULL;
  m_puhCbf[1]          = NULL;
  m_puhCbf[2]          = NULL;
  m_pcTrCoeffY         = NULL;
  m_pcTrCoeffCb        = NULL;
  m_pcTrCoeffCr        = NULL;
  
  m_pcPattern          = NULL;
  
  m_pcCUAboveLeft      = NULL;
  m_pcCUAboveRight     = NULL;
  m_pcCUAbove          = NULL;
  m_pcCULeft           = NULL;
  
  m_bDecSubCu          = false;

  m_pauhDBKIdx[0]      = NULL;
  m_pauhDBKIdx[1]      = NULL;
}

TComDataCU::~TComDataCU()
{
}

Void TComDataCU::create(UInt uiNumPartition, UInt uiLog2CUSize, Bool bDecSubCu)
{
  UInt uiCoeffSize = 1<<(uiLog2CUSize<<1);
  m_bDecSubCu = bDecSubCu;
  
  m_pcPic              = NULL;
  m_pcPicture            = NULL;
  m_uiNumPartition     = uiNumPartition;
  
  if ( !bDecSubCu )
  {
    m_puhDepth           = (UChar*    )xMalloc(UChar,    uiNumPartition);

    m_puhWidth           = (UChar*    )xMalloc(UChar,    uiNumPartition);
    m_puhHeight          = (UChar*    )xMalloc(UChar,    uiNumPartition);

    m_puhLog2CUSize      = (UChar*    )xMalloc(UChar,    uiNumPartition);

    m_pePartSize         = (PartSize* )xMalloc(PartSize, uiNumPartition);
#if DMH
		m_iDMHMode = (Int*)xMalloc(Int, uiNumPartition);
#endif

#if INTER_GROUP
		m_iPartitionSizeIndex = (Int*)xMalloc(Int, uiNumPartition);
#endif
    m_puhTrIdx           = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_pePredMode         = (PredMode* )xMalloc(PredMode, uiNumPartition);
    
    m_puhInterDir        = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhIntraDir        = (UChar* )xMalloc(UChar,  uiNumPartition);


#if inter_direct_skip_bug1
	m_pcInterSkipmode = (UChar*)xMalloc(UChar, uiNumPartition);
#endif

#if ZHANGYI_INTRA
	m_puhIntraDirValue        = (Char* )xMalloc(UChar,  uiNumPartition);
	m_puhIntraDirCbValue = (Char* )xMalloc(UChar,  uiNumPartition);
	m_puhIntraDirCrValue = (Char* )xMalloc(UChar,  uiNumPartition);
	m_puhIntraDirCb = (UChar* )xMalloc(UChar,  uiNumPartition);
	m_puhIntraDirCr = (UChar* )xMalloc(UChar,  uiNumPartition);
#endif
#if ZHANGYI_INTRA_SDIP
	m_pSDIPFlag = (UChar*)xMalloc(UChar, uiNumPartition);
	m_pSDIPDirection = (UChar*)xMalloc(UChar, uiNumPartition);
#endif
    m_puhCbf[0]          = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhCbf[1]          = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhCbf[2]          = (UChar* )xMalloc(UChar,  uiNumPartition);
    
    m_pcTrCoeffY         = (TCoeff*)xMalloc(TCoeff, uiCoeffSize);
    m_pcTrCoeffCb        = (TCoeff*)xMalloc(TCoeff, uiCoeffSize>>2);
    m_pcTrCoeffCr        = (TCoeff*)xMalloc(TCoeff, uiCoeffSize>>2);
    
    m_acCUMvField[REF_PIC_0].create( uiNumPartition );
    m_acCUMvField[REF_PIC_1].create( uiNumPartition );
    
    m_pauhDBKIdx[0]      = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_pauhDBKIdx[1]      = (UChar* )xMalloc(UChar,  uiNumPartition);
  }
  else
  {
    m_acCUMvField[REF_PIC_0].setNumPartition(uiNumPartition );
    m_acCUMvField[REF_PIC_1].setNumPartition(uiNumPartition );
  }
  
#if CODEFFCODER_LDW_WLQ_YQH// 
  m_CoeffCodingParam.DCT_CGFlag =(UChar* )xMalloc(UChar,  uiCoeffSize>>2);//yuquanhe@hisilicon.com
  m_CoeffCodingParam.DCT_CGLastRun =(UChar* )xMalloc(UChar,  uiCoeffSize>>2);//yuquanhe@hisilicon.com
  m_CoeffCodingParam.DCT_PairsInCG =(UChar* )xMalloc(UChar,  uiCoeffSize>>2);//yuquanhe@hisilicon.com
  m_CoeffCodingParam.level =(int* )xMalloc(int,  uiCoeffSize);//yuquanhe@hisilicon.com
  m_CoeffCodingParam.run =(int* )xMalloc(int,  uiCoeffSize);//yuquanhe@hisilicon.com
#endif

  // create pattern memory
  m_pcPattern            = (TComPattern*)xMalloc(TComPattern, 1);
  
  // create motion vector fields
  
  m_pcCUAboveLeft      = NULL;
  m_pcCUAboveRight     = NULL;
  m_pcCUAbove          = NULL;
  m_pcCULeft           = NULL;
}

Void TComDataCU::destroy()
{
  m_pcPic              = NULL;
  m_pcPicture            = NULL;
  
  if ( m_pcPattern )
  { 
    xFree(m_pcPattern);
    m_pcPattern = NULL;
  }
  
  // encoder-side buffer free
  if ( !m_bDecSubCu )
  {
    if ( m_puhDepth           ) { xFree(m_puhDepth);            m_puhDepth          = NULL; }
    if ( m_puhWidth           ) { xFree(m_puhWidth);            m_puhWidth          = NULL; }
    if ( m_puhHeight          ) { xFree(m_puhHeight);           m_puhHeight         = NULL; }
    if ( m_puhLog2CUSize      ) { xFree(m_puhLog2CUSize);       m_puhLog2CUSize     = NULL; }
    if ( m_pePartSize         ) { xFree(m_pePartSize);          m_pePartSize        = NULL; }
#if INTER_GROUP
		if (m_iPartitionSizeIndex)  { xFree(m_iPartitionSizeIndex); m_iPartitionSizeIndex = NULL; }
#endif
#if DMH
		if (m_iDMHMode            ) { xFree(m_iDMHMode);            m_iDMHMode          = NULL; }
#endif
		if ( m_puhTrIdx           ) { xFree(m_puhTrIdx);            m_puhTrIdx					= NULL; }
    if ( m_pePredMode         ) { xFree(m_pePredMode);          m_pePredMode        = NULL; }
    if ( m_puhCbf[0]          ) { xFree(m_puhCbf[0]);           m_puhCbf[0]         = NULL; }
    if ( m_puhCbf[1]          ) { xFree(m_puhCbf[1]);           m_puhCbf[1]         = NULL; }
    if ( m_puhCbf[2]          ) { xFree(m_puhCbf[2]);           m_puhCbf[2]         = NULL; }
    if ( m_puhInterDir        ) { xFree(m_puhInterDir);         m_puhInterDir       = NULL; }
    if ( m_puhIntraDir        ) { xFree(m_puhIntraDir);         m_puhIntraDir       = NULL; }

#if inter_direct_skip_bug1
	if (m_pcInterSkipmode)      { xFree(m_pcInterSkipmode);         m_pcInterSkipmode = NULL; }
#endif


#if ZHANGYI_INTRA
	if ( m_puhIntraDirValue        ) { xFree(m_puhIntraDirValue);         m_puhIntraDirValue       = NULL; }
	if ( m_puhIntraDirCbValue)  { xFree(m_puhIntraDirCbValue);     m_puhIntraDirCbValue = NULL;}
	if ( m_puhIntraDirCrValue)  { xFree(m_puhIntraDirCrValue);     m_puhIntraDirCrValue = NULL;}
	if ( m_puhIntraDirCb)  { xFree(m_puhIntraDirCb);     m_puhIntraDirCb = NULL;}
	if ( m_puhIntraDirCr)  { xFree(m_puhIntraDirCr);     m_puhIntraDirCr = NULL;}
#endif

#if ZHANGYI_INTRA_SDIP
	if (m_pSDIPFlag) { xFree(m_pSDIPFlag);           m_pSDIPFlag = NULL; }
	if (m_pSDIPDirection) { xFree(m_pSDIPDirection);      m_pSDIPDirection = NULL; }
#endif

    if ( m_pcTrCoeffY         ) { xFree(m_pcTrCoeffY);          m_pcTrCoeffY        = NULL; }
    if ( m_pcTrCoeffCb        ) { xFree(m_pcTrCoeffCb);         m_pcTrCoeffCb       = NULL; }
    if ( m_pcTrCoeffCr        ) { xFree(m_pcTrCoeffCr);         m_pcTrCoeffCr       = NULL; }
    if ( m_pauhDBKIdx[0]      ) { xFree(m_pauhDBKIdx[0]);       m_pauhDBKIdx[0]     = NULL; }
    if ( m_pauhDBKIdx[1]      ) { xFree(m_pauhDBKIdx[1]);       m_pauhDBKIdx[1]     = NULL; }

#if CODEFFCODER_LDW_WLQ_YQH// 
	if ( m_CoeffCodingParam.DCT_CGFlag    ) { xFree(m_CoeffCodingParam.DCT_CGFlag);             m_CoeffCodingParam.DCT_CGFlag     = NULL; }//yuquanhe@hisilicon.com
	if ( m_CoeffCodingParam.DCT_CGLastRun    ) { xFree(m_CoeffCodingParam.DCT_CGLastRun);       m_CoeffCodingParam.DCT_CGLastRun     = NULL; }//yuquanhe@hisilicon.com
	if ( m_CoeffCodingParam.DCT_PairsInCG    ) { xFree(m_CoeffCodingParam.DCT_PairsInCG);       m_CoeffCodingParam.DCT_PairsInCG     = NULL; }//yuquanhe@hisilicon.com
	if ( m_CoeffCodingParam.level    ) { xFree(m_CoeffCodingParam.level);                       m_CoeffCodingParam.level     = NULL; }//yuquanhe@hisilicon.com
	if ( m_CoeffCodingParam.run    ) { xFree(m_CoeffCodingParam.run );                          m_CoeffCodingParam.run      = NULL; }//yuquanhe@hisilicon.com

#endif


    m_acCUMvField[REF_PIC_0].destroy();
    m_acCUMvField[REF_PIC_1].destroy();
    
  }
  
  m_pcCUAboveLeft       = NULL;
  m_pcCUAboveRight      = NULL;
  m_pcCUAbove           = NULL;
  m_pcCULeft            = NULL;
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

// --------------------------------------------------------------------------------------------------------------------
// Initialization
// --------------------------------------------------------------------------------------------------------------------

/**
 - initialize top-level CU
 - internal buffers are already created
 - set values before encoding a CU
 .
 \param  pcPic     picture (TComPic) class pointer
 \param  iCUAddr   CU address
 */
Void TComDataCU::initCU( TComPic* pcPic, UInt iCUAddr )
{
  m_pcPic              = pcPic;
  m_pcPicture          = pcPic->getPicture();
  m_uiCUAddr           = iCUAddr;
  m_uiCUPelX           = ( iCUAddr % pcPic->getFrameWidthInCU() ) << g_uiLog2MaxCUSize;
  m_uiCUPelY           = ( iCUAddr / pcPic->getFrameWidthInCU() ) << g_uiLog2MaxCUSize;
  m_uiAbsIdxInLCU      = 0;
  
  m_dTotalCost         = MAX_DOUBLE;
  m_uiTotalDistortion  = 0;
  m_uiTotalBits        = 0;
  m_uiNumPartition     = pcPic->getNumPartInCU();
  
  Int iSizeInUchar = sizeof( UChar ) * m_uiNumPartition;
  
  memset( m_puhInterDir,        0, iSizeInUchar );
  memset( m_puhIntraDir,        0, iSizeInUchar );

#if inter_direct_skip_bug1
  memset(m_pcInterSkipmode, 0, iSizeInUchar);
#endif


#if ZHANGYI_INTRA
  memset( m_puhIntraDirValue,        0, iSizeInUchar );
  memset( m_puhIntraDirCbValue,        0, iSizeInUchar );
  memset( m_puhIntraDirCrValue,        0, iSizeInUchar );
  memset( m_puhIntraDirCb,        0, iSizeInUchar );
  memset( m_puhIntraDirCr,        0, iSizeInUchar );
#endif
#if ZHANGYI_INTRA_SDIP
  memset(m_pSDIPFlag, 0, iSizeInUchar);
  memset(m_pSDIPDirection, 0, iSizeInUchar);
#endif
  memset( m_puhCbf[0],          0, iSizeInUchar );
  memset( m_puhCbf[1],          0, iSizeInUchar );
  memset( m_puhCbf[2],          0, iSizeInUchar );
  memset( m_puhDepth,           0, iSizeInUchar );
  memset( m_pauhDBKIdx[0],      0, iSizeInUchar );
  memset( m_pauhDBKIdx[1],      0, iSizeInUchar );
  memset( m_puhLog2CUSize, (UChar)g_uiLog2MaxCUSize,  iSizeInUchar );
  memset( m_puhTrIdx,           0, iSizeInUchar );
  
  UChar uhWidth  = (1<<g_uiLog2MaxCUSize);
  UChar uhHeight = (1<<g_uiLog2MaxCUSize);
  memset( m_puhWidth,          uhWidth,  iSizeInUchar );
  memset( m_puhHeight,         uhHeight, iSizeInUchar );

  for (UInt ui = 0; ui < m_uiNumPartition; ui++)
  {
    m_pePartSize[ui] = SIZE_NONE;
    m_pePredMode[ui] = MODE_NONE;
#if INTER_GROUP
		m_iPartitionSizeIndex[ui] = 0;
#endif
#if DMH
		m_iDMHMode[ui] = 0;
#endif
  }
  
  m_acCUMvField[REF_PIC_0].clearMvField();
  m_acCUMvField[REF_PIC_1].clearMvField();
  
  UInt uiTmp = m_puhWidth[0]*m_puhHeight[0];
  memset( m_pcTrCoeffY , 0, sizeof( TCoeff ) * uiTmp );
  
  uiTmp  >>= 2;
  memset( m_pcTrCoeffCb, 0, sizeof( TCoeff ) * uiTmp );
  memset( m_pcTrCoeffCr, 0, sizeof( TCoeff ) * uiTmp );
  

  // setting neighbor CU
  m_pcCULeft        = NULL;
  m_pcCUAbove       = NULL;
  m_pcCUAboveLeft   = NULL;
  m_pcCUAboveRight  = NULL;
  
  UInt uiWidthInCU = pcPic->getFrameWidthInCU();
  if ( m_uiCUAddr % uiWidthInCU )
  {
    m_pcCULeft = pcPic->getCU( m_uiCUAddr - 1 );
  }
  
  if ( m_uiCUAddr / uiWidthInCU )
  {
    m_pcCUAbove = pcPic->getCU( m_uiCUAddr - uiWidthInCU );
  }
  
  if ( m_pcCULeft && m_pcCUAbove )
  {
    m_pcCUAboveLeft = pcPic->getCU( m_uiCUAddr - uiWidthInCU - 1 );
  }
  
  if ( m_pcCUAbove && ( (m_uiCUAddr%uiWidthInCU) < (uiWidthInCU-1) )  )
  {
    m_pcCUAboveRight = pcPic->getCU( m_uiCUAddr - uiWidthInCU + 1 );
  }
}

// initialize prediction data
Void TComDataCU::initEstData( UInt uiDepth )
{
  m_dTotalCost         = MAX_DOUBLE;
  m_uiTotalDistortion  = 0;
  m_uiTotalBits        = 0;
  
  Int iSizeInUchar = sizeof( UChar  ) * m_uiNumPartition;

  memset( m_puhInterDir,        0, iSizeInUchar );
  memset( m_puhIntraDir,        0, iSizeInUchar );

#if inter_direct_skip_bug1
  memset(m_pcInterSkipmode, 0, iSizeInUchar);
#endif


#if ZHANGYI_INTRA
  memset( m_puhIntraDirValue,        0, iSizeInUchar );
  memset( m_puhIntraDirCbValue,        0, iSizeInUchar );
  memset( m_puhIntraDirCrValue,        0, iSizeInUchar );
  memset( m_puhIntraDirCb,        0, iSizeInUchar );
  memset( m_puhIntraDirCr,        0, iSizeInUchar );
#endif

#if ZHANGYI_INTRA_SDIP
  memset(m_pSDIPFlag, 0, iSizeInUchar);
  memset(m_pSDIPDirection, 0, iSizeInUchar);
#endif

  memset( m_puhCbf[0],          0, iSizeInUchar );
  memset( m_puhCbf[1],          0, iSizeInUchar );
  memset( m_puhCbf[2],          0, iSizeInUchar );
  memset( m_puhTrIdx,           0, iSizeInUchar );
  memset( m_pauhDBKIdx[0],      0, iSizeInUchar );
  memset( m_pauhDBKIdx[1],      0, iSizeInUchar );
  memset( m_puhDepth,     uiDepth, iSizeInUchar );
  memset( m_puhLog2CUSize,  g_uiLog2MaxCUSize - uiDepth, iSizeInUchar );

  UChar uhWidth  = 1<<(g_uiLog2MaxCUSize-uiDepth);
  UChar uhHeight = 1<<(g_uiLog2MaxCUSize-uiDepth);
  memset( m_puhWidth,          uhWidth,  iSizeInUchar );
  memset( m_puhHeight,         uhHeight, iSizeInUchar );

  for (UInt ui = 0; ui < m_uiNumPartition; ui++)
  {
    m_pePartSize[ui] = SIZE_NONE;
    m_pePredMode[ui] = MODE_NONE;
#if DMH
		m_iDMHMode[ui] = 0;
#endif
#if INTER_GROUP
		m_iPartitionSizeIndex[ui] = 0;
#endif
  }
  
  UInt uiTmp = m_puhWidth[0]*m_puhHeight[0];
  memset( m_pcTrCoeffY , 0, sizeof(TCoeff)*uiTmp );
  
  uiTmp >>= 2;
  memset( m_pcTrCoeffCb, 0, sizeof(TCoeff)*uiTmp );
  memset( m_pcTrCoeffCr, 0, sizeof(TCoeff)*uiTmp );
  
  m_acCUMvField[REF_PIC_0].clearMvField();
  m_acCUMvField[REF_PIC_1].clearMvField();
}

// initialize Sub partition
Void TComDataCU::initSubCU( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth )
{
  assert( uiPartUnitIdx<4 );
  
  UInt uiPartOffset = ( pcCU->getTotalNumPart()>>2 )*uiPartUnitIdx;
  
  m_pcPic           = pcCU->getPic();
  m_pcPicture       = pcCU->getPicture();
  m_uiCUAddr        = pcCU->getAddr();
  m_uiAbsIdxInLCU   = pcCU->getZorderIdxInCU() + uiPartOffset;
  m_uiCUPelX        = pcCU->getCUPelX() + ( ( uiPartUnitIdx &  1 )<<(pcCU->getLog2CUSize(0) - 1) );
  m_uiCUPelY        = pcCU->getCUPelY() + ( ( uiPartUnitIdx >> 1 )<<(pcCU->getLog2CUSize(0) - 1) );
  
  m_dTotalCost         = MAX_DOUBLE;
  m_uiTotalDistortion  = 0;
  m_uiTotalBits        = 0;
  
  m_uiNumPartition     = pcCU->getTotalNumPart() >> 2;
  
  Int iSizeInUchar = sizeof( UChar  ) * m_uiNumPartition;
  
  memset( m_puhInterDir,        0, iSizeInUchar );
  memset( m_puhIntraDir,        0, iSizeInUchar );

#if inter_direct_skip_bug1
  memset(m_pcInterSkipmode, 0, iSizeInUchar);
#endif



#if ZHANGYI_INTRA
  memset( m_puhIntraDirValue,        0, iSizeInUchar );
  memset( m_puhIntraDirCbValue,        0, iSizeInUchar );
  memset( m_puhIntraDirCrValue,        0, iSizeInUchar );
  memset( m_puhIntraDirCb,        0, iSizeInUchar );
  memset( m_puhIntraDirCr,        0, iSizeInUchar );
#endif
#if ZHANGYI_INTRA_SDIP
  memset(m_pSDIPFlag, 0, iSizeInUchar);
  memset(m_pSDIPDirection, 0, iSizeInUchar);
#endif
  memset( m_puhCbf[0],          0, iSizeInUchar );
  memset( m_puhCbf[1],          0, iSizeInUchar );
  memset( m_puhCbf[2],          0, iSizeInUchar );
  memset( m_puhDepth,     uiDepth, iSizeInUchar );
  memset( m_puhTrIdx,           0, iSizeInUchar );
  memset( m_pauhDBKIdx[0],      0, iSizeInUchar );
  memset( m_pauhDBKIdx[1],      0, iSizeInUchar );

  memset( m_puhLog2CUSize, (UChar)(g_uiLog2MaxCUSize-uiDepth),  iSizeInUchar );
  
  UChar uhWidth  = 1<<(g_uiLog2MaxCUSize-uiDepth);
  UChar uhHeight = 1<<(g_uiLog2MaxCUSize-uiDepth);
  memset( m_puhWidth,          uhWidth,  iSizeInUchar );
  memset( m_puhHeight,         uhHeight, iSizeInUchar );

  for (UInt ui = 0; ui < m_uiNumPartition; ui++)
  {
    m_pePartSize[ui] = SIZE_NONE;
    m_pePredMode[ui] = MODE_NONE;
#if DMH
		m_iDMHMode[ui] = 0;
#endif
#if INTER_GROUP
		m_iPartitionSizeIndex[ui] = 0;
#endif
  }
  
  UInt uiTmp = m_puhWidth[0]*m_puhHeight[0];
  memset( m_pcTrCoeffY , 0, sizeof(TCoeff)*uiTmp );
  
  uiTmp >>= 2;
  memset( m_pcTrCoeffCb, 0, sizeof(TCoeff)*uiTmp );
  memset( m_pcTrCoeffCr, 0, sizeof(TCoeff)*uiTmp );
  
  m_pcCULeft        = pcCU->getCULeft();
  m_pcCUAbove       = pcCU->getCUAbove();
  m_pcCUAboveLeft   = pcCU->getCUAboveLeft();
  m_pcCUAboveRight  = pcCU->getCUAboveRight();
  
  m_acCUMvField[REF_PIC_0].clearMvField();
  m_acCUMvField[REF_PIC_1].clearMvField();
}

// --------------------------------------------------------------------------------------------------------------------
// Copy
// --------------------------------------------------------------------------------------------------------------------

Void TComDataCU::copySubCU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiPart = uiAbsPartIdx;
  
  m_pcPic              = pcCU->getPic();
  m_pcPicture          = pcCU->getPicture();
  m_uiCUAddr           = pcCU->getAddr();
  m_uiAbsIdxInLCU      = uiAbsPartIdx;
  
  m_uiCUPelX           = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  m_uiCUPelY           = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  
  m_pePartSize=pcCU->getPartitionSize()   + uiPart;
#if INTER_GROUP
	m_iPartitionSizeIndex = pcCU->getPartitionSizeIndex() + uiPart;
#endif
#if DMH
	m_iDMHMode = pcCU->getDMHMode() + uiPart;
#endif
	m_pePredMode=pcCU->getPredictionMode()  + uiPart;
  m_puhTrIdx=pcCU->getTransformIdx()			+ uiPart; 
  
  m_puhInterDir         = pcCU->getInterDir()         + uiPart;
  m_puhIntraDir         = pcCU->getIntraDir()         + uiPart;

#if inter_direct_skip_bug1
  m_pcInterSkipmode = pcCU->getInterSkipmode() + uiPart;
#endif



#if ZHANGYI_INTRA
  m_puhIntraDirValue         = pcCU->getIntraDirValue()         + uiPart;
  m_puhIntraDirCbValue         = pcCU->getIntraDirCbValue() + uiPart;
  m_puhIntraDirCrValue         = pcCU->getIntraDirCrValue() + uiPart;
  m_puhIntraDirCb         = pcCU->getIntraDirCb() + uiPart;
  m_puhIntraDirCr         = pcCU->getIntraDirCr() + uiPart;
#endif
#if ZHANGYI_INTRA_SDIP
  m_pSDIPFlag = pcCU->getSDIPFlag() + uiPart;
  m_pSDIPDirection = pcCU->getSDIPDirection() + uiPart;
#endif
  m_puhCbf[0]= pcCU->getCbf(TEXT_LUMA)            + uiPart;
  m_puhCbf[1]= pcCU->getCbf(TEXT_CHROMA_U)        + uiPart;
  m_puhCbf[2]= pcCU->getCbf(TEXT_CHROMA_V)        + uiPart;
  
  m_puhDepth      = pcCU->getDepth()              + uiPart;
  m_puhLog2CUSize = pcCU->getLog2CUSize()         + uiPart;

  m_puhWidth      = pcCU->getWidth()              + uiPart;
  m_puhHeight     = pcCU->getHeight()             + uiPart;

  m_pauhDBKIdx[0] = pcCU->getDBKIdx(0) + uiPart;
  m_pauhDBKIdx[1] = pcCU->getDBKIdx(1) + uiPart;

  m_pcCUAboveLeft      = pcCU->getCUAboveLeft();
  m_pcCUAboveRight     = pcCU->getCUAboveRight();
  m_pcCUAbove          = pcCU->getCUAbove();
  m_pcCULeft           = pcCU->getCULeft();
  
  UInt uiTmp = 1<<(m_puhLog2CUSize[0]<<1);
  UInt uiLog2MaxCuSize=pcCU->getPicture()->getSPS()->getLog2MaxCUSize();
  
  UInt uiCoffOffset = (1<<(uiLog2MaxCuSize<<1))*uiAbsPartIdx/pcCU->getPic()->getNumPartInCU();
  
  m_pcTrCoeffY=pcCU->getCoeffY() + uiCoffOffset;
  
  uiTmp >>= 2;
  uiCoffOffset >>=2;
  m_pcTrCoeffCb=pcCU->getCoeffCb() + uiCoffOffset;
  m_pcTrCoeffCr=pcCU->getCoeffCr() + uiCoffOffset;
  
  m_acCUMvField[REF_PIC_0].setMvPtr(pcCU->getCUMvField (REF_PIC_0)->getMv()     + uiPart);
  m_acCUMvField[REF_PIC_0].setMvdPtr(pcCU->getCUMvField(REF_PIC_0)->getMvd()    + uiPart);
  m_acCUMvField[REF_PIC_1].setMvPtr(pcCU->getCUMvField (REF_PIC_1)->getMv()     + uiPart);
  m_acCUMvField[REF_PIC_1].setMvdPtr(pcCU->getCUMvField(REF_PIC_1)->getMvd()    + uiPart);
#if RPS
  m_acCUMvField[REF_PIC_0].setRefPtr(pcCU->getCUMvField(REF_PIC_0)->getRefIdx() + uiPart);
  m_acCUMvField[REF_PIC_1].setRefPtr(pcCU->getCUMvField(REF_PIC_1)->getRefIdx() + uiPart);
#endif
}

// Copy inter prediction info from the biggest CU
Void TComDataCU::copyInterPredInfoFrom    ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPic eRefPic )
{
  m_pcPic              = pcCU->getPic();
  m_pcPicture          = pcCU->getPicture();
  m_uiCUAddr           = pcCU->getAddr();
  m_uiAbsIdxInLCU      = uiAbsPartIdx;
  
  Int iRastPartIdx     = g_auiZscanToRaster[uiAbsPartIdx];
  m_uiCUPelX           = pcCU->getCUPelX() + (( iRastPartIdx % m_pcPic->getNumPartInWidth() )<<m_pcPic->getLog2MinCUSize());
  m_uiCUPelY           = pcCU->getCUPelY() + (( iRastPartIdx / m_pcPic->getNumPartInWidth() )<<m_pcPic->getLog2MinCUSize());
  
  m_pcCUAboveLeft      = pcCU->getCUAboveLeft();
  m_pcCUAboveRight     = pcCU->getCUAboveRight();
  m_pcCUAbove          = pcCU->getCUAbove();
  m_pcCULeft           = pcCU->getCULeft();
  

  m_pePartSize         = pcCU->getPartitionSize ()        + uiAbsPartIdx;
#if INTER_GROUP
	m_iPartitionSizeIndex = pcCU->getPartitionSizeIndex() + uiAbsPartIdx;
#endif
#if DMH
	m_iDMHMode = pcCU->getDMHMode() + uiAbsPartIdx;
#endif
	m_pePredMode         = pcCU->getPredictionMode()        + uiAbsPartIdx;
  m_puhInterDir        = pcCU->getInterDir      ()        + uiAbsPartIdx;
  m_puhIntraDir        = pcCU->getIntraDir      ()        + uiAbsPartIdx;

#if inter_direct_skip_bug1
  m_pcInterSkipmode = pcCU->getInterSkipmode() + uiAbsPartIdx;
#endif



#if ZHANGYI_INTRA
  m_puhIntraDirValue        = pcCU->getIntraDirValue()       + uiAbsPartIdx;
  m_puhIntraDirCbValue         = pcCU->getIntraDirCbValue()       + uiAbsPartIdx;
  m_puhIntraDirCrValue         = pcCU->getIntraDirCrValue()       + uiAbsPartIdx;
  m_puhIntraDirCb         = pcCU->getIntraDirCb()       + uiAbsPartIdx;
  m_puhIntraDirCr         = pcCU->getIntraDirCr()       + uiAbsPartIdx;
#endif
#if ZHANGYI_INTRA_SDIP
  m_pSDIPFlag = pcCU->getSDIPFlag() + uiAbsPartIdx;
  m_pSDIPDirection = pcCU->getSDIPDirection() + uiAbsPartIdx;
#endif
  m_puhDepth           = pcCU->getDepth ()                + uiAbsPartIdx;
  m_puhLog2CUSize      = pcCU->getLog2CUSize()            + uiAbsPartIdx;

  m_puhWidth           = pcCU->getWidth ()                + uiAbsPartIdx;
  m_puhHeight          = pcCU->getHeight()                + uiAbsPartIdx;

  m_acCUMvField[eRefPic].setMvPtr(pcCU->getCUMvField(eRefPic)->getMv()   + uiAbsPartIdx);
  m_acCUMvField[eRefPic].setMvdPtr(pcCU->getCUMvField(eRefPic)->getMvd() + uiAbsPartIdx);
#if RPS
  m_acCUMvField[eRefPic].setRefPtr(pcCU->getCUMvField(eRefPic)->getRefIdx() + uiAbsPartIdx);
#endif
  m_pauhDBKIdx[0] = pcCU->getDBKIdx(0) + uiAbsPartIdx;
  m_pauhDBKIdx[1] = pcCU->getDBKIdx(1) + uiAbsPartIdx;
}

// Copy small CU to bigger CU.
// One of quarter parts overwritten by predicted sub part.
Void TComDataCU::copyPartFrom( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth )
{
  assert( uiPartUnitIdx<4 );
  
  m_dTotalCost         += pcCU->getTotalCost();
  m_uiTotalDistortion  += pcCU->getTotalDistortion();
  m_uiTotalBits        += pcCU->getTotalBits();

  UInt uiOffset         = pcCU->getTotalNumPart()*uiPartUnitIdx;
  
  UInt uiNumPartition = pcCU->getTotalNumPart();
  Int iSizeInUchar  = sizeof( UChar ) * uiNumPartition;
  

  memcpy( m_pePartSize + uiOffset, pcCU->getPartitionSize(),  sizeof( PartSize ) * uiNumPartition );
#if INTER_GROUP
	memcpy(m_iPartitionSizeIndex + uiOffset, pcCU->getPartitionSizeIndex(), sizeof(Int) * uiNumPartition);
#endif
#if DMH
	memcpy(m_iDMHMode + uiOffset, pcCU->getDMHMode(), sizeof(Int) * uiNumPartition);
#endif
	memcpy( m_pePredMode + uiOffset, pcCU->getPredictionMode(), sizeof( PredMode ) * uiNumPartition );
  memcpy( m_puhTrIdx          + uiOffset, pcCU->getTransformIdx(),   iSizeInUchar );
  memcpy( m_puhInterDir         + uiOffset, pcCU->getInterDir(),          iSizeInUchar );
  memcpy( m_puhIntraDir         + uiOffset, pcCU->getIntraDir(),          iSizeInUchar );
#if ZHANGYI_INTRA
  memcpy( m_puhIntraDirValue         + uiOffset, pcCU->getIntraDirValue(),          iSizeInUchar );
  memcpy( m_puhIntraDirCbValue         + uiOffset , pcCU->getIntraDirCbValue(),          iSizeInUchar );
  memcpy( m_puhIntraDirCrValue         + uiOffset , pcCU->getIntraDirCrValue(),          iSizeInUchar );
  memcpy( m_puhIntraDirCb         + uiOffset , pcCU->getIntraDirCb(),          iSizeInUchar );
  memcpy( m_puhIntraDirCr         + uiOffset , pcCU->getIntraDirCr(),          iSizeInUchar );
#endif
#if ZHANGYI_INTRA_SDIP
  memcpy(m_pSDIPFlag + uiOffset, pcCU->getSDIPFlag(), iSizeInUchar);
  memcpy(m_pSDIPDirection + uiOffset, pcCU->getSDIPDirection(), iSizeInUchar);
#endif
#if inter_direct_skip_bug1
  memcpy(m_pcInterSkipmode + uiOffset, pcCU->getInterSkipmode(), iSizeInUchar);
#endif



  memcpy( m_puhCbf[0] + uiOffset, pcCU->getCbf(TEXT_LUMA)    , iSizeInUchar );
  memcpy( m_puhCbf[1] + uiOffset, pcCU->getCbf(TEXT_CHROMA_U), iSizeInUchar );
  memcpy( m_puhCbf[2] + uiOffset, pcCU->getCbf(TEXT_CHROMA_V), iSizeInUchar );
  
  memcpy( m_puhDepth      + uiOffset, pcCU->getDepth(),      iSizeInUchar );
  memcpy( m_puhLog2CUSize + uiOffset, pcCU->getLog2CUSize(), iSizeInUchar );
  memcpy( m_puhWidth  + uiOffset, pcCU->getWidth(),  iSizeInUchar );
  memcpy( m_puhHeight + uiOffset, pcCU->getHeight(), iSizeInUchar );

  memcpy( m_pauhDBKIdx[0] + uiOffset, pcCU->getDBKIdx(0), iSizeInUchar );
  memcpy( m_pauhDBKIdx[1] + uiOffset, pcCU->getDBKIdx(1), iSizeInUchar );

  m_pcCUAboveLeft      = pcCU->getCUAboveLeft();
  m_pcCUAboveRight     = pcCU->getCUAboveRight();
  m_pcCUAbove          = pcCU->getCUAbove();
  m_pcCULeft           = pcCU->getCULeft();
  
  m_acCUMvField[REF_PIC_0].copyFrom( pcCU->getCUMvField( REF_PIC_0 ), pcCU->getTotalNumPart(), uiOffset );
  m_acCUMvField[REF_PIC_1].copyFrom( pcCU->getCUMvField( REF_PIC_1 ), pcCU->getTotalNumPart(), uiOffset );
  
  UInt uiTmp  = 1<<((g_uiLog2MaxCUSize - uiDepth)<<1);
  UInt uiTmp2 = uiPartUnitIdx*uiTmp;
  memcpy( m_pcTrCoeffY  + uiTmp2, pcCU->getCoeffY(),  sizeof(TCoeff)*uiTmp );
  
  uiTmp >>= 2; uiTmp2>>= 2;
  memcpy( m_pcTrCoeffCb + uiTmp2, pcCU->getCoeffCb(), sizeof(TCoeff)*uiTmp );
  memcpy( m_pcTrCoeffCr + uiTmp2, pcCU->getCoeffCr(), sizeof(TCoeff)*uiTmp );
}

// Copy current predicted part to a CU in picture.
// It is used to predict for next part
Void TComDataCU::copyToPic( UChar uhDepth )
{
  TComDataCU*& rpcCU = m_pcPic->getCU( m_uiCUAddr );
  
  rpcCU->getTotalCost()       = m_dTotalCost;
  rpcCU->getTotalDistortion() = m_uiTotalDistortion;
  rpcCU->getTotalBits()       = m_uiTotalBits;



  Int iSizeInUchar  = sizeof( UChar ) * m_uiNumPartition;
#if inter_direct_skip_bug1
  memcpy(rpcCU->getInterSkipmode() + m_uiAbsIdxInLCU, m_pcInterSkipmode, iSizeInUchar);
#endif
  memcpy( rpcCU->getPartitionSize()  + m_uiAbsIdxInLCU, m_pePartSize, sizeof( PartSize ) * m_uiNumPartition );
#if INTER_GROUP
	memcpy(rpcCU->getPartitionSizeIndex() + m_uiAbsIdxInLCU, m_iPartitionSizeIndex, sizeof(Int) * m_uiNumPartition);
#endif  
#if DMH
	memcpy(rpcCU->getDMHMode() + m_uiAbsIdxInLCU, m_iDMHMode, sizeof(Int) * m_uiNumPartition);
#endif
	memcpy( rpcCU->getPredictionMode() + m_uiAbsIdxInLCU, m_pePredMode, sizeof( PredMode ) * m_uiNumPartition );
  memcpy( rpcCU->getTransformIdx()   + m_uiAbsIdxInLCU, m_puhTrIdx,          iSizeInUchar );
  memcpy( rpcCU->getInterDir()          + m_uiAbsIdxInLCU, m_puhInterDir,         iSizeInUchar );
  memcpy( rpcCU->getIntraDir()          + m_uiAbsIdxInLCU, m_puhIntraDir,         iSizeInUchar );
#if ZHANGYI_INTRA
  memcpy( rpcCU->getIntraDirValue()          + m_uiAbsIdxInLCU, m_puhIntraDirValue,         iSizeInUchar );
  memcpy( rpcCU->getIntraDirCbValue()          + m_uiAbsIdxInLCU, m_puhIntraDirCbValue,         iSizeInUchar );
  memcpy( rpcCU->getIntraDirCrValue()          + m_uiAbsIdxInLCU, m_puhIntraDirCrValue,        iSizeInUchar );
  memcpy( rpcCU->getIntraDirCb()          + m_uiAbsIdxInLCU, m_puhIntraDirCb,         iSizeInUchar );
  memcpy( rpcCU->getIntraDirCr()          + m_uiAbsIdxInLCU, m_puhIntraDirCr,        iSizeInUchar );
#endif
#if ZHANGYI_INTRA_SDIP
  memcpy(rpcCU->getSDIPFlag() + m_uiAbsIdxInLCU, m_pSDIPFlag, iSizeInUchar);
  memcpy(rpcCU->getSDIPDirection() + m_uiAbsIdxInLCU, m_pSDIPDirection, iSizeInUchar);
#endif
  memcpy( rpcCU->getCbf(TEXT_LUMA)     + m_uiAbsIdxInLCU, m_puhCbf[0], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_U) + m_uiAbsIdxInLCU, m_puhCbf[1], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_V) + m_uiAbsIdxInLCU, m_puhCbf[2], iSizeInUchar );
  
  memcpy( rpcCU->getDepth()      + m_uiAbsIdxInLCU, m_puhDepth,      iSizeInUchar );
  memcpy( rpcCU->getLog2CUSize() + m_uiAbsIdxInLCU, m_puhLog2CUSize, iSizeInUchar );
  memcpy( rpcCU->getWidth()  + m_uiAbsIdxInLCU, m_puhWidth,  iSizeInUchar );
  memcpy( rpcCU->getHeight() + m_uiAbsIdxInLCU, m_puhHeight, iSizeInUchar );

  memcpy( rpcCU->getDBKIdx(0) + m_uiAbsIdxInLCU, m_pauhDBKIdx[0], iSizeInUchar );
  memcpy( rpcCU->getDBKIdx(1) + m_uiAbsIdxInLCU, m_pauhDBKIdx[1], iSizeInUchar );

  m_acCUMvField[REF_PIC_0].copyTo( rpcCU->getCUMvField( REF_PIC_0 ), m_uiAbsIdxInLCU );
  m_acCUMvField[REF_PIC_1].copyTo( rpcCU->getCUMvField( REF_PIC_1 ), m_uiAbsIdxInLCU );
  
  UInt uiTmp  = 1<<((g_uiLog2MaxCUSize - uhDepth)<<1);
  UInt uiTmp2 = m_uiAbsIdxInLCU<<(m_pcPic->getLog2MinCUSize()<<1);
  memcpy( rpcCU->getCoeffY()  + uiTmp2, m_pcTrCoeffY,  sizeof(TCoeff)*uiTmp  );
  
  uiTmp >>= 2; uiTmp2 >>= 2;
  memcpy( rpcCU->getCoeffCb() + uiTmp2, m_pcTrCoeffCb, sizeof(TCoeff)*uiTmp  );
  memcpy( rpcCU->getCoeffCr() + uiTmp2, m_pcTrCoeffCr, sizeof(TCoeff)*uiTmp  );

}

Void TComDataCU::copyToPic( UChar uhDepth, UInt uiPartIdx, UInt uiPartDepth )
{
  TComDataCU*&  rpcCU       = m_pcPic->getCU( m_uiCUAddr );
  UInt          uiQNumPart  = m_uiNumPartition>>(uiPartDepth<<1);
  
  UInt uiPartStart          = uiPartIdx*uiQNumPart;
  UInt uiPartOffset         = m_uiAbsIdxInLCU + uiPartStart;
  
  rpcCU->getTotalCost()       = m_dTotalCost;
  rpcCU->getTotalDistortion() = m_uiTotalDistortion;
  rpcCU->getTotalBits()       = m_uiTotalBits;

  Int iSizeInUchar  = sizeof( UChar  ) * uiQNumPart;

#if inter_direct_skip_bug1
  memcpy(rpcCU->getInterSkipmode() + uiPartOffset, m_pcInterSkipmode, iSizeInUchar);
#endif

  memcpy( rpcCU->getPartitionSize()  + uiPartOffset, m_pePartSize, sizeof( PartSize ) * uiQNumPart );
#if INTER_GROUP
	memcpy(rpcCU->getPartitionSizeIndex() + uiPartOffset, m_iPartitionSizeIndex, sizeof(Int) * uiQNumPart);
#endif
#if DMH
	memcpy(rpcCU->getDMHMode() + uiPartOffset, m_iDMHMode, sizeof(Int) * uiQNumPart);
#endif
	memcpy( rpcCU->getPredictionMode() + uiPartOffset, m_pePredMode, sizeof( PredMode ) * uiQNumPart );
  memcpy( rpcCU->getTransformIdx()    + uiPartOffset, m_puhTrIdx,          iSizeInUchar );
  memcpy( rpcCU->getInterDir()          + uiPartOffset, m_puhInterDir,         iSizeInUchar );
  memcpy( rpcCU->getIntraDir()          + uiPartOffset, m_puhIntraDir,         iSizeInUchar );
#if ZHANGYI_INTRA
  memcpy( rpcCU->getIntraDirValue()          + uiPartOffset, m_puhIntraDirValue,         iSizeInUchar );
  memcpy( rpcCU->getIntraDirCbValue()           + uiPartOffset, m_puhIntraDirCbValue,         iSizeInUchar );
  memcpy( rpcCU->getIntraDirCrValue()           + uiPartOffset, m_puhIntraDirCrValue,         iSizeInUchar );
  memcpy( rpcCU->getIntraDirCb()           + uiPartOffset, m_puhIntraDirCb,         iSizeInUchar );
  memcpy( rpcCU->getIntraDirCr()           + uiPartOffset, m_puhIntraDirCr,         iSizeInUchar );
#endif
#if ZHANGYI_INTRA_SDIP
  memcpy(rpcCU->getSDIPFlag() + uiPartOffset, m_pSDIPFlag, iSizeInUchar);
  memcpy(rpcCU->getSDIPDirection() + uiPartOffset, m_pSDIPDirection, iSizeInUchar);
#endif
  memcpy( rpcCU->getCbf(TEXT_LUMA)     + uiPartOffset, m_puhCbf[0], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_U) + uiPartOffset, m_puhCbf[1], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_V) + uiPartOffset, m_puhCbf[2], iSizeInUchar );
  
  memcpy( rpcCU->getDepth()      + uiPartOffset, m_puhDepth,      iSizeInUchar );
  memcpy( rpcCU->getLog2CUSize() + uiPartOffset, m_puhLog2CUSize, iSizeInUchar );
  
  memcpy( rpcCU->getWidth()  + uiPartOffset, m_puhWidth,  iSizeInUchar );
  memcpy( rpcCU->getHeight() + uiPartOffset, m_puhHeight, iSizeInUchar );

  m_acCUMvField[REF_PIC_0].copyTo( rpcCU->getCUMvField( REF_PIC_0 ), m_uiAbsIdxInLCU, uiPartStart, uiQNumPart );
  m_acCUMvField[REF_PIC_1].copyTo( rpcCU->getCUMvField( REF_PIC_1 ), m_uiAbsIdxInLCU, uiPartStart, uiQNumPart );
  
  memcpy( rpcCU->getDBKIdx(0) + uiPartOffset, m_pauhDBKIdx[0], iSizeInUchar );
  memcpy( rpcCU->getDBKIdx(1) + uiPartOffset, m_pauhDBKIdx[1], iSizeInUchar );

  UInt uiTmp  = 1<<((g_uiLog2MaxCUSize - uhDepth - uiPartDepth)<<1);
  UInt uiTmp2 = uiPartOffset<<(m_pcPic->getLog2MinCUSize()<<1);
  memcpy( rpcCU->getCoeffY()  + uiTmp2, m_pcTrCoeffY,  sizeof(TCoeff)*uiTmp  );
  
  uiTmp >>= 2; uiTmp2 >>= 2;
  memcpy( rpcCU->getCoeffCb() + uiTmp2, m_pcTrCoeffCb, sizeof(TCoeff)*uiTmp  );
  memcpy( rpcCU->getCoeffCr() + uiTmp2, m_pcTrCoeffCr, sizeof(TCoeff)*uiTmp  );
}

// --------------------------------------------------------------------------------------------------------------------
// Other public functions
// --------------------------------------------------------------------------------------------------------------------
#if INTER611
UInt m_puiBlockSize[20][2] = {
	{ MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE * 2 },
	{ MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE * 2 },
	{ MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE },
	{ MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE },
	{ MIN_BLOCK_SIZE, MIN_BLOCK_SIZE * 2 },
	{ MIN_BLOCK_SIZE, MIN_BLOCK_SIZE * 2 },
	{ MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE / 2 },
	{ MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE * 3 / 2 },
	{ MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE * 3 / 2 },
	{ MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE / 2 },
	{ MIN_BLOCK_SIZE / 2, MIN_BLOCK_SIZE * 2 },
	{ MIN_BLOCK_SIZE * 3 / 2, MIN_BLOCK_SIZE * 2 },
	{ MIN_BLOCK_SIZE * 3 / 2, MIN_BLOCK_SIZE * 2 },
	{ MIN_BLOCK_SIZE / 2, MIN_BLOCK_SIZE * 2 },
	{ MIN_BLOCK_SIZE, MIN_BLOCK_SIZE },
	{ MIN_BLOCK_SIZE, MIN_BLOCK_SIZE },
	{ MIN_BLOCK_SIZE, MIN_BLOCK_SIZE },
	{ MIN_BLOCK_SIZE, MIN_BLOCK_SIZE }
};
#endif
#if INTER612
//block的像素起始位置 block8X8
UInt m_puiBlockPosition[20][2] = {
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 4 },
	{ 0, 0 },
	{ 4, 0 },
	{ 0, 0 },
	{ 0, 2 },
	{ 0, 0 },
	{ 0, 6 },
	{ 0, 0 },
	{ 2, 0 },
	{ 0, 0 },
	{ 6, 0 },
	{ 0, 0 },
	{ 4, 0 },
	{ 0, 4 },
	{ 4, 4 }
};

#endif

#if BSKIP ||PSKIP
TComDataCU* TComDataCU::getPULeft1(UInt& uiL1PartUnitIdx, UInt uiCurrPartUnitIdx, UInt shift)
{
	UInt uiAbsPartIdx = g_auiZscanToRaster[uiCurrPartUnitIdx];

	UInt uiAbsZorderCUIdx = g_auiZscanToRaster[m_uiAbsIdxInLCU];
	UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();

	if (uiAbsPartIdx % uiNumPartInCUWidth)
	{
		uiL1PartUnitIdx = g_auiRasterToZscan[uiAbsPartIdx - 1 + uiNumPartInCUWidth * (shift - 1)];
		if (uiAbsPartIdx % uiNumPartInCUWidth == uiAbsZorderCUIdx % uiNumPartInCUWidth)
		{
			return m_pcPic->getCU(getAddr());
		}
		else
		{
			uiL1PartUnitIdx -= m_uiAbsIdxInLCU;
			return this;
		}
	}
	uiL1PartUnitIdx = g_auiRasterToZscan[uiAbsPartIdx + uiNumPartInCUWidth - 1 + uiNumPartInCUWidth * (shift - 1)];
	return m_pcCULeft;
}
TComDataCU* TComDataCU::getPUAbove1(UInt& uiA1PartUnitIdx, UInt uiCurrPartUnitIdx, UInt shift)
{
	UInt uiAbsPartIdxRT = g_auiZscanToRaster[uiCurrPartUnitIdx];

	UInt uiAbsZorderCUIdx = g_auiZscanToRaster[m_uiAbsIdxInLCU] + m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1;
	UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();

	if ((m_pcPic->getCU(m_uiCUAddr)->getCUPelX() + g_auiRasterToPelX[uiAbsPartIdxRT] + (1 << m_pcPic->getLog2MinCUSize())) >= m_pcPicture->getSPS()->getWidth())
	{
		uiA1PartUnitIdx = MAX_UINT;
		return NULL;
	}

	if (uiAbsPartIdxRT % uiNumPartInCUWidth < uiNumPartInCUWidth - 1)
	{
		if (uiAbsPartIdxRT / uiNumPartInCUWidth)
		{
			if (uiCurrPartUnitIdx > g_auiRasterToZscan[uiAbsPartIdxRT - uiNumPartInCUWidth + 1 + (shift - 2)])
			{
				uiA1PartUnitIdx = g_auiRasterToZscan[uiAbsPartIdxRT - uiNumPartInCUWidth + 1 + (shift - 2)];
				if ((uiAbsPartIdxRT % uiNumPartInCUWidth == uiAbsZorderCUIdx % uiNumPartInCUWidth) || (uiAbsPartIdxRT / uiNumPartInCUWidth == uiAbsZorderCUIdx / uiNumPartInCUWidth))
				{
					return m_pcPic->getCU(getAddr());
				}
				else
				{
					uiA1PartUnitIdx -= m_uiAbsIdxInLCU;
					return this;
				}
			}
			uiA1PartUnitIdx = MAX_UINT;
			return NULL;
		}
		uiA1PartUnitIdx = g_auiRasterToZscan[uiAbsPartIdxRT + m_pcPic->getNumPartInCU() - uiNumPartInCUWidth + 1 + (shift - 2)];
		return m_pcCUAbove;
	}

	if (uiAbsPartIdxRT / uiNumPartInCUWidth)
	{
		uiA1PartUnitIdx = MAX_UINT;
		return NULL;
	}

	uiA1PartUnitIdx = g_auiRasterToZscan[m_pcPic->getNumPartInCU() - uiNumPartInCUWidth + (shift - 2)];
	return m_pcCUAboveRight;
}




#endif

#if ZHANGYI_INTRA_SDIP
TComDataCU* TComDataCU::getPULeftSDIP(UInt& uiLPartUnitIdx, UInt uiCurrPartUnitIdx)
{
	UInt uiAbsPartIdx = g_auiZscanToRaster[uiCurrPartUnitIdx];
	UInt uiLNumPart = getPic()->getNumPartInCU() >> (getDepth(uiCurrPartUnitIdx) << 1);
	UInt uiPartOffset = uiCurrPartUnitIdx % uiLNumPart;
	UInt uiZorder = uiCurrPartUnitIdx - uiPartOffset;
	UInt uiAbsZorderCUIdx = g_auiZscanToRaster[uiZorder];
	UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();

	TComDataCU* pcTempCU;
	UInt uiPartUnitIdx = uiCurrPartUnitIdx - m_uiAbsIdxInLCU;  //the Idx in "this" CU
	UInt uiUnitSize = (1 << g_uiLog2MaxCUSize) >> g_uiMaxCUDepth;
	UInt uiLine = 0;
	if (/* !bEnforceSquareUnit &&*/ getSDIPFlag(uiPartUnitIdx))
	{
		UInt uiNumPart = getPic()->getNumPartInCU() >> (getDepth(uiPartUnitIdx) << 1);
		if (getSDIPDirection(uiPartUnitIdx) == 0 && uiPartUnitIdx % uiNumPart != 0)//hNx2N
		{
			uiLPartUnitIdx = uiPartUnitIdx - 1;
			return this;
		}
		else
		{
			uiAbsZorderCUIdx = g_auiZscanToRaster[uiCurrPartUnitIdx - uiCurrPartUnitIdx % uiNumPart];
			uiLine = convertNonSquareUnitToLine(uiPartUnitIdx);
			uiAbsPartIdx = uiAbsZorderCUIdx + (uiLine / uiUnitSize) * uiNumPartInCUWidth;
		}
	}

	if (uiAbsPartIdx % uiNumPartInCUWidth) //不在一个LCU的最左边
	{
		uiLPartUnitIdx = g_auiRasterToZscan[uiAbsPartIdx - 1];
		if (uiAbsPartIdx % uiNumPartInCUWidth == uiAbsZorderCUIdx % uiNumPartInCUWidth) //是否跨CU
		{
			pcTempCU = m_pcPic->getCU(getAddr());
		}
		else
		{
			uiLPartUnitIdx -= m_uiAbsIdxInLCU;
			return this;
		}
	}
	else
	{
		pcTempCU = m_pcCULeft;
		uiLPartUnitIdx = g_auiRasterToZscan[uiAbsPartIdx + uiNumPartInCUWidth - 1];
		/*if ( (bEnforceSliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getAddr() < m_uiSliceStartCU)) ||
		(bEnforceEntropySliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getAddr() < m_uiEntropySliceStartCU)) )
		{
		return NULL;
		}*/
	}
	if (/*!bEnforceSquareUnit && */pcTempCU && pcTempCU->getSDIPFlag(uiLPartUnitIdx))
	{
		UInt uiLNumPart = getPic()->getNumPartInCU() >> (pcTempCU->getDepth(uiLPartUnitIdx) << 1);

		if (pcTempCU->getSDIPDirection(uiLPartUnitIdx) == 0)
			uiLPartUnitIdx = uiLPartUnitIdx - (uiLPartUnitIdx % uiLNumPart) + (uiLNumPart - 1);          // the last part
		else
		{
			UInt uiLLine = g_auiRasterToPelY[g_auiZscanToRaster[uiLPartUnitIdx % uiLNumPart]] + uiLine % uiUnitSize;
			uiLPartUnitIdx = pcTempCU->convertLineToNonSquareUnit(uiLPartUnitIdx, uiLLine);
		}
	}
	return pcTempCU;
}
TComDataCU* TComDataCU::getPUAboveSDIP(UInt& uiAPartUnitIdx, UInt uiCurrPartUnitIdx)
{
	UInt uiAbsPartIdx = g_auiZscanToRaster[uiCurrPartUnitIdx];//uiCurrPartUnitIdx：当前PU在LCU内的idx
	UInt uiLNumPart = getPic()->getNumPartInCU() >> (getDepth(uiCurrPartUnitIdx) << 1);
	UInt uiPartOffset = uiCurrPartUnitIdx % uiLNumPart;
	UInt uiZorder = uiCurrPartUnitIdx - uiPartOffset;
	UInt uiAbsZorderCUIdx = g_auiZscanToRaster[uiZorder];
	UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();


	TComDataCU* pcTempCU;
	UInt uiPartUnitIdx = uiCurrPartUnitIdx - m_uiAbsIdxInLCU;  //the Idx in "this" CU
	UInt uiUnitSize = (1 << g_uiLog2MaxCUSize) >> g_uiMaxCUDepth;
	UInt uiLine = 0;
	if (/*!bEnforceSquareUnit && */getSDIPFlag(uiPartUnitIdx))
	{
		UInt uiNumPart = getPic()->getNumPartInCU() >> (getDepth(uiPartUnitIdx) << 1);

		if (getSDIPDirection(uiPartUnitIdx) == 1 && uiPartUnitIdx % uiNumPart != 0)//2NxhN
		{
			uiAPartUnitIdx = uiPartUnitIdx - 1;
			return this;
		}
		else
		{
			uiAbsZorderCUIdx = g_auiZscanToRaster[uiCurrPartUnitIdx - uiCurrPartUnitIdx % uiNumPart];
			uiLine = convertNonSquareUnitToLine(uiPartUnitIdx);
			uiAbsPartIdx = uiAbsZorderCUIdx + uiLine / uiUnitSize;
		}
	}

	if (uiAbsPartIdx / uiNumPartInCUWidth) //不是一个LCU的最上面一行
	{
		uiAPartUnitIdx = g_auiRasterToZscan[uiAbsPartIdx - uiNumPartInCUWidth];
		if (uiAbsPartIdx / uiNumPartInCUWidth == uiAbsZorderCUIdx / uiNumPartInCUWidth)
		{//上边块跨CU
			pcTempCU = m_pcPic->getCU(getAddr());
		}
		else
		{
			uiAPartUnitIdx -= m_uiAbsIdxInLCU;
			return this;
		}
	}
	else
	{
		pcTempCU = m_pcCUAbove;
		uiAPartUnitIdx = g_auiRasterToZscan[uiAbsPartIdx + m_pcPic->getNumPartInCU() - uiNumPartInCUWidth];
		/*if ( (bEnforceSliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getAddr() < m_uiSliceStartCU)) ||
		(bEnforceEntropySliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getAddr() < m_uiEntropySliceStartCU)) )
		{
		return NULL;
		}*/
	}
	if (/*!bEnforceSquareUnit && */pcTempCU && pcTempCU->getSDIPFlag(uiAPartUnitIdx))
	{
		UInt uiANumPart = getPic()->getNumPartInCU() >> (pcTempCU->getDepth(uiAPartUnitIdx) << 1);

		if (pcTempCU->getSDIPDirection(uiAPartUnitIdx) == 1)
			uiAPartUnitIdx = uiAPartUnitIdx - (uiAPartUnitIdx % uiANumPart) + (uiANumPart - 1);          // the last part
		else
		{
			UInt uiALine = g_auiRasterToPelX[g_auiZscanToRaster[uiAPartUnitIdx % uiANumPart]] + uiLine % uiUnitSize;
			uiAPartUnitIdx = pcTempCU->convertLineToNonSquareUnit(uiAPartUnitIdx, uiALine);
		}
	}
	return pcTempCU;
}
#endif
TComDataCU* TComDataCU::getPULeft( UInt& uiLPartUnitIdx, UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdx       = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[m_uiAbsIdxInLCU];
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
#if ZHANGYI_INTRA_SDIP
  TComDataCU* pcTempCU;
  UInt uiPartUnitIdx = uiCurrPartUnitIdx - m_uiAbsIdxInLCU;  //the Idx in "this" CU
  UInt uiUnitSize = (1 << g_uiLog2MaxCUSize) >> g_uiMaxCUDepth;
  UInt uiLine = 0;
  if (/* !bEnforceSquareUnit &&*/ getSDIPFlag(uiPartUnitIdx))
  {
	  UInt uiNumPart = getPic()->getNumPartInCU() >> (getDepth(uiPartUnitIdx) << 1);
	  if (getSDIPDirection(uiPartUnitIdx) == 0 && uiPartUnitIdx % uiNumPart != 0)//hNx2N
	  {
		  uiLPartUnitIdx = uiPartUnitIdx - 1;
		  return this;
	  }
	  else
	  {
		  uiAbsZorderCUIdx = g_auiZscanToRaster[uiCurrPartUnitIdx - uiCurrPartUnitIdx % uiNumPart];
		  uiLine = convertNonSquareUnitToLine(uiPartUnitIdx);
		  uiAbsPartIdx = uiAbsZorderCUIdx + (uiLine / uiUnitSize) * uiNumPartInCUWidth;
	  }
  }
#endif


  if( uiAbsPartIdx % uiNumPartInCUWidth )
  {
    uiLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx - 1 ];
    if ( uiAbsPartIdx % uiNumPartInCUWidth == uiAbsZorderCUIdx % uiNumPartInCUWidth )
    {
#if ZHANGYI_INTRA_SDIP
		pcTempCU = m_pcPic->getCU(getAddr());
#else
		return m_pcPic->getCU(getAddr());
#endif
    }
    else
    {
      uiLPartUnitIdx -= m_uiAbsIdxInLCU;
      return this;
    }
  }
  
#if ZHANGYI_INTRA_SDIP
  else
  {
	  pcTempCU = m_pcCULeft;
	  uiLPartUnitIdx = g_auiRasterToZscan[uiAbsPartIdx + uiNumPartInCUWidth - 1];
	  /*if ( (bEnforceSliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getAddr() < m_uiSliceStartCU)) ||
	  (bEnforceEntropySliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getAddr() < m_uiEntropySliceStartCU)) )
	  {
	  return NULL;
	  }*/
  }
  if (/*!bEnforceSquareUnit && */pcTempCU && pcTempCU->getSDIPFlag(uiLPartUnitIdx))
  {
	  UInt uiLNumPart = getPic()->getNumPartInCU() >> (pcTempCU->getDepth(uiLPartUnitIdx) << 1);

	  if (pcTempCU->getSDIPDirection(uiLPartUnitIdx) == 0)
		  uiLPartUnitIdx = uiLPartUnitIdx - (uiLPartUnitIdx % uiLNumPart) + (uiLNumPart - 1);          // the last part
	  else
	  {
		  UInt uiLLine = g_auiRasterToPelY[g_auiZscanToRaster[uiLPartUnitIdx % uiLNumPart]] + uiLine % uiUnitSize;
		  uiLPartUnitIdx = pcTempCU->convertLineToNonSquareUnit(uiLPartUnitIdx, uiLLine);
	  }
  }
  return pcTempCU;
#else
  uiLPartUnitIdx = g_auiRasterToZscan[uiAbsPartIdx + uiNumPartInCUWidth - 1];
  return m_pcCULeft;
#endif
}

TComDataCU* TComDataCU::getPUAbove( UInt& uiAPartUnitIdx, UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdx       = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[m_uiAbsIdxInLCU];
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
#if ZHANGYI_INTRA_SDIP
  TComDataCU* pcTempCU;
  UInt uiPartUnitIdx = uiCurrPartUnitIdx - m_uiAbsIdxInLCU;  //the Idx in "this" CU
  UInt uiUnitSize = (1 << g_uiLog2MaxCUSize) >> g_uiMaxCUDepth;
  UInt uiLine = 0;
  if (/*!bEnforceSquareUnit && */getSDIPFlag(uiPartUnitIdx))
  {
	  UInt uiNumPart = getPic()->getNumPartInCU() >> (getDepth(uiPartUnitIdx) << 1);

	  if (getSDIPDirection(uiPartUnitIdx) == 1 && uiPartUnitIdx % uiNumPart != 0)//2NxhN
	  {
		  uiAPartUnitIdx = uiPartUnitIdx - 1;
		  return this;
	  }
	  else
	  {
		  uiAbsZorderCUIdx = g_auiZscanToRaster[uiCurrPartUnitIdx - uiCurrPartUnitIdx % uiNumPart];
		  uiLine = convertNonSquareUnitToLine(uiPartUnitIdx);
		  uiAbsPartIdx = uiAbsZorderCUIdx + uiLine / uiUnitSize;
	  }
  }
#endif


  if( uiAbsPartIdx / uiNumPartInCUWidth )
  {
    uiAPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx - uiNumPartInCUWidth ];
    if ( uiAbsPartIdx / uiNumPartInCUWidth == uiAbsZorderCUIdx / uiNumPartInCUWidth )
    {
#if ZHANGYI_INTRA_SDIP
		pcTempCU = m_pcPic->getCU(getAddr());
#else
		return m_pcPic->getCU(getAddr()); //取的是整个LCU
#endif
    }
    else
    {
      uiAPartUnitIdx -= m_uiAbsIdxInLCU;
      return this;
    }
  }
#if ZHANGYI_INTRA_SDIP
  else
  {
	  pcTempCU = m_pcCUAbove;
	  uiAPartUnitIdx = g_auiRasterToZscan[uiAbsPartIdx + m_pcPic->getNumPartInCU() - uiNumPartInCUWidth];
	  /*if ( (bEnforceSliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getAddr() < m_uiSliceStartCU)) ||
	  (bEnforceEntropySliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getAddr() < m_uiEntropySliceStartCU)) )
	  {
	  return NULL;
	  }*/
  }
  if (/*!bEnforceSquareUnit && */pcTempCU && pcTempCU->getSDIPFlag(uiAPartUnitIdx))
  {
	  UInt uiANumPart = getPic()->getNumPartInCU() >> (pcTempCU->getDepth(uiAPartUnitIdx) << 1);

	  if (pcTempCU->getSDIPDirection(uiAPartUnitIdx) == 1)
		  uiAPartUnitIdx = uiAPartUnitIdx - (uiAPartUnitIdx % uiANumPart) + (uiANumPart - 1);          // the last part
	  else
	  {
		  UInt uiALine = g_auiRasterToPelX[g_auiZscanToRaster[uiAPartUnitIdx % uiANumPart]] + uiLine % uiUnitSize;
		  uiAPartUnitIdx = pcTempCU->convertLineToNonSquareUnit(uiAPartUnitIdx, uiALine);
	  }
  }
  return pcTempCU;
#else
  uiAPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx + m_pcPic->getNumPartInCU() - uiNumPartInCUWidth ];
  return m_pcCUAbove;
#endif
}

TComDataCU* TComDataCU::getPUAboveLeft( UInt& uiALPartUnitIdx, UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdx       = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[m_uiAbsIdxInLCU];
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if( uiAbsPartIdx % uiNumPartInCUWidth )
  {
    if( uiAbsPartIdx / uiNumPartInCUWidth )
    {
      uiALPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx - uiNumPartInCUWidth - 1 ];
      if ( ( uiAbsPartIdx % uiNumPartInCUWidth == uiAbsZorderCUIdx % uiNumPartInCUWidth ) || ( uiAbsPartIdx / uiNumPartInCUWidth == uiAbsZorderCUIdx / uiNumPartInCUWidth ) )
      {
        return m_pcPic->getCU( getAddr() );
      }
      else
      {
        uiALPartUnitIdx -= m_uiAbsIdxInLCU;
        return this;
      }
    }
    uiALPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx + getPic()->getNumPartInCU() - uiNumPartInCUWidth - 1 ];
    return m_pcCUAbove;
  }
  
  if( uiAbsPartIdx / uiNumPartInCUWidth )
  {
    uiALPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx - 1 ];
    return m_pcCULeft;
  }
  
  uiALPartUnitIdx = g_auiRasterToZscan[ m_pcPic->getNumPartInCU() - 1 ];
  return m_pcCUAboveLeft;
}

#if INTER611
TComDataCU* TComDataCU::getPUAboveRight(UInt& uiARPartUnitIdx, UInt uiCurrPartUnitIdx, UInt shift)
#else
TComDataCU* TComDataCU::getPUAboveRight(UInt& uiARPartUnitIdx, UInt uiCurrPartUnitIdx)
#endif
{
  UInt uiAbsPartIdxRT     = g_auiZscanToRaster[uiCurrPartUnitIdx];
#if INTER611
  uiAbsPartIdxRT += (shift - 1);
#endif
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1;
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelX() + g_auiRasterToPelX[uiAbsPartIdxRT] + (1<<m_pcPic->getLog2MinCUSize()) ) >= m_pcPicture->getSPS()->getWidth() )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }

  if ( uiAbsPartIdxRT % uiNumPartInCUWidth < uiNumPartInCUWidth - 1 )
  {
    if ( uiAbsPartIdxRT / uiNumPartInCUWidth )
    {
      if ( uiCurrPartUnitIdx > g_auiRasterToZscan[ uiAbsPartIdxRT - uiNumPartInCUWidth + 1 ] )
      {
        uiARPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxRT - uiNumPartInCUWidth + 1 ];
        if ( ( uiAbsPartIdxRT % uiNumPartInCUWidth == uiAbsZorderCUIdx % uiNumPartInCUWidth ) || ( uiAbsPartIdxRT / uiNumPartInCUWidth == uiAbsZorderCUIdx / uiNumPartInCUWidth ) )
        {
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiARPartUnitIdx -= m_uiAbsIdxInLCU;
          return this;
        }
      }
      uiARPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiARPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxRT + m_pcPic->getNumPartInCU() - uiNumPartInCUWidth + 1 ];
    return m_pcCUAbove;
  }
  
  if ( uiAbsPartIdxRT / uiNumPartInCUWidth )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }
  uiARPartUnitIdx = g_auiRasterToZscan[ m_pcPic->getNumPartInCU() - uiNumPartInCUWidth ];
  return m_pcCUAboveRight;
}

TComDataCU* TComDataCU::getPUBelowLeft( UInt& uiBLPartUnitIdx, UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdxLB     = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  UInt uiAbsZorderCUIdxLB = g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + ((1<<(m_puhLog2CUSize[0] - m_pcPic->getLog2MinCUSize())) - 1)*uiNumPartInCUWidth;
  
  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelY() + g_auiRasterToPelY[uiAbsPartIdxLB] + (1<<m_pcPic->getLog2MinCUSize()) ) >= m_pcPicture->getSPS()->getHeight() )
  {
    uiBLPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  if ( uiAbsPartIdxLB / uiNumPartInCUWidth < uiNumPartInCUWidth - 1 )
  {
    if ( uiAbsPartIdxLB % uiNumPartInCUWidth )
    {
      if ( uiCurrPartUnitIdx > g_auiRasterToZscan[ uiAbsPartIdxLB + uiNumPartInCUWidth - 1 ] )
      {
        uiBLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxLB + uiNumPartInCUWidth - 1 ];
        if ( ( (uiAbsPartIdxLB % uiNumPartInCUWidth) == (uiAbsZorderCUIdxLB % uiNumPartInCUWidth) ) || ( (uiAbsPartIdxLB / uiNumPartInCUWidth) == (uiAbsZorderCUIdxLB / uiNumPartInCUWidth) ) )
        {
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiBLPartUnitIdx -= m_uiAbsIdxInLCU;
          return this;
        }
      }
      uiBLPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiBLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxLB + uiNumPartInCUWidth*2 - 1 ];
    return m_pcCULeft;
  }
  
  uiBLPartUnitIdx = MAX_UINT;
  return NULL;
}

#if ZHANGYI_INTRA_SDIP
Void TComDataCU::copyToPicLuma(TComDataCU* pcCU, TComPicYuv* pcPicYuvDst, TComYuv* pcRecoYuvSrc, UInt iCuAddr, UInt uiAbsZorderIdx, UInt uiDepth, UInt uiPartIdx)
{ //uiAbsZorderIdx: current PU's index in the current CU
	Int  y, iSize;

	//iSize  = (m_iSize - m_uiExt) >>uiPartDepth;
#if ZHANGYI_INTRA_SDIP
	UInt    uiPartIdxInCU = uiPartIdx - pcCU->getZorderIdxInCU();
	UInt    uiSdipFlag = pcCU->getSDIPFlag(uiPartIdxInCU); //zhangyiModify: 0609
	UInt    uiSdipDir = pcCU->getSDIPDirection(uiPartIdxInCU); //zhangyiModify: 0609
	UInt    uiPartDepth = (pcCU->getPartitionSize(uiPartIdxInCU) == SIZE_2Nx2N) ? 0 : 1;//zhangyiModify: 0609
	iSize = 1 << (g_uiLog2MaxCUSize - uiDepth);  //zhangyiModify: 0609
	UInt    uiWidth = uiSdipFlag ? (uiSdipDir ? (iSize << 1) : (iSize >> 1)) : iSize;
	UInt    uiHeight = uiSdipFlag ? (uiSdipDir ? (iSize >> 1) : (iSize << 1)) : iSize;
	UInt    uiLine = pcCU->convertNonSquareUnitToLine(uiPartIdxInCU);

	Pel* pSrc = uiSdipFlag ? (pcRecoYuvSrc->getLumaAddr(pcCU->getZorderIdxInCU()) + (uiSdipDir ? (uiLine * pcRecoYuvSrc->getStride()) : uiLine)) : pcRecoYuvSrc->getLumaAddr(uiAbsZorderIdx);
	Pel* pDst = uiSdipFlag ? (pcPicYuvDst->getLineLumaAddr(iCuAddr, pcCU->getZorderIdxInCU(), uiLine, uiSdipDir)) : (pcPicYuvDst->getLumaAddr(iCuAddr, uiAbsZorderIdx));
#else
	Pel* pSrc = getLumaAddr(uiPartIdx);
	Pel* pDst = pcPicYuvDst->getLumaAddr(iCuAddr, uiAbsZorderIdx);
#endif

	UInt  iSrcStride = pcRecoYuvSrc->getStride();
	UInt  iDstStride = pcPicYuvDst->getStride();

	for (y = uiHeight; y != 0; y--)
	{
		::memcpy(pDst, pSrc, sizeof(Pel)*uiWidth);
		pDst += iDstStride;
		pSrc += iSrcStride;
	}
}
#endif


TComDataCU* TComDataCU::getPUBelowLeftAdi(UInt& uiBLPartUnitIdx, UInt uiPuHeight,  UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdxLB     = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  UInt uiAbsZorderCUIdxLB = g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + ((1<<(m_puhLog2CUSize[0] - m_pcPic->getLog2MinCUSize())) - 1)*uiNumPartInCUWidth;
  
  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelY() + g_auiRasterToPelY[uiAbsPartIdxLB] + uiPuHeight ) >= m_pcPicture->getSPS()->getHeight() )
  {
    uiBLPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  if ( uiAbsPartIdxLB / uiNumPartInCUWidth < uiNumPartInCUWidth - 1 )
  {
    if ( uiAbsPartIdxLB % uiNumPartInCUWidth )
    {
      if ( uiCurrPartUnitIdx > g_auiRasterToZscan[ uiAbsPartIdxLB + uiNumPartInCUWidth - 1 ] )
      {
        uiBLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxLB + uiNumPartInCUWidth - 1 ];
        if ( ( (uiAbsPartIdxLB % uiNumPartInCUWidth) == (uiAbsZorderCUIdxLB % uiNumPartInCUWidth) ) || ( (uiAbsPartIdxLB / uiNumPartInCUWidth) == (uiAbsZorderCUIdxLB / uiNumPartInCUWidth) ) )
        {
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiBLPartUnitIdx -= m_uiAbsIdxInLCU;
          return this;
        }
      }
      uiBLPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiBLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxLB + uiNumPartInCUWidth*2 - 1 ];
    return m_pcCULeft;
  }
  
  uiBLPartUnitIdx = MAX_UINT;
  return NULL;
}

#if INTER_GROUP
Int TComDataCU::getFFourTypeIndex()
{
	for (int i = 0; i < 4; i++)
		if (getFPuTypeIndex(i) != 0)
			return 1;
	return 0;
}
#endif

TComDataCU* TComDataCU::getPUAboveRightAdi(UInt&  uiARPartUnitIdx, UInt uiPuWidth, UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdxRT     = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + (m_puhWidth[0] / m_pcPic->getMinCUWidth()) - 1;
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelX() + g_auiRasterToPelX[uiAbsPartIdxRT] + uiPuWidth ) >= m_pcPicture->getSPS()->getWidth() )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  if ( uiAbsPartIdxRT % uiNumPartInCUWidth < uiNumPartInCUWidth - 1 )
  {
    if ( uiAbsPartIdxRT / uiNumPartInCUWidth )
    {
      if ( uiCurrPartUnitIdx > g_auiRasterToZscan[ uiAbsPartIdxRT - uiNumPartInCUWidth + 1 ] )
      {
        uiARPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxRT - uiNumPartInCUWidth + 1 ];
        if ( ( uiAbsPartIdxRT % uiNumPartInCUWidth == uiAbsZorderCUIdx % uiNumPartInCUWidth ) || ( uiAbsPartIdxRT / uiNumPartInCUWidth == uiAbsZorderCUIdx / uiNumPartInCUWidth ) )
        {
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiARPartUnitIdx -= m_uiAbsIdxInLCU;
          return this;
        }
      }
      uiARPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiARPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxRT + m_pcPic->getNumPartInCU() - uiNumPartInCUWidth + 1 ];
    return m_pcCUAbove;
  }
  
  if ( uiAbsPartIdxRT / uiNumPartInCUWidth )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  uiARPartUnitIdx = g_auiRasterToZscan[ m_pcPic->getNumPartInCU() - uiNumPartInCUWidth ];
  return m_pcCUAboveRight;
}

UInt TComDataCU::getCtxSplitFlag( UInt uiAbsPartIdx, UInt uiDepth )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  UInt        uiCtx;
  // Get left split flag
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx  = ( pcTempCU ) ? ( ( pcTempCU->getDepth( uiTempPartIdx ) > uiDepth ) ? 1 : 0 ) : 0;
  
  // Get above split flag
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx += ( pcTempCU ) ? ( ( pcTempCU->getDepth( uiTempPartIdx ) > uiDepth ) ? 1 : 0 ) : 0;
  
  return uiCtx;
}

UInt TComDataCU::getCtxQtCbf( UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  if( getPredictionMode( uiAbsPartIdx ) != MODE_INTRA && eType != TEXT_LUMA )
  {
    return uiTrDepth;
  }
  UInt uiCtx;
  const UInt uiDepth = getDepth( uiAbsPartIdx );
  const UInt uiLog2TrafoSize = getPicture()->getSPS()->getLog2MaxCUSize() - uiDepth - uiTrDepth;
  
  if( uiTrDepth == 0 || uiLog2TrafoSize == getPicture()->getSPS()->getLog2MaxPhTrSize() )
  {
    uiCtx = 1;
  }
  else
  {
    uiCtx = 0;
  }

  return uiCtx;
}

UInt TComDataCU::getCtxQtRootCbf( UInt uiAbsPartIdx )
{
  UInt uiCtx = 0;
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  
  // Get RootCbf of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  if ( pcTempCU )
  {
    uiCtx = pcTempCU->getQtRootCbf( uiTempPartIdx );
  }
  
  // Get RootCbf of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  if ( pcTempCU )
  {
    uiCtx += pcTempCU->getQtRootCbf( uiTempPartIdx ) << 1;
  }
  
  return uiCtx;
}

UInt TComDataCU::getCtxPredMode( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  UInt        uiCtx = 0;
  
  // Get BCBP of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx    = ( pcTempCU ) ? ( pcTempCU->getPredictionMode( uiTempPartIdx ) > 1 ? 1 : 0 ) : 0;
  
  // Get BCBP of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx    = ( pcTempCU ) ? ( pcTempCU->getPredictionMode( uiTempPartIdx ) > 1 ? 1 : 0 ) : 0;
  
  return uiCtx;
}

UInt TComDataCU::getCtxInterDir( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  UInt        uiCtx = 0;
  
  // Get BCBP of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx += ( pcTempCU ) ? ( ( pcTempCU->getInterDir( uiTempPartIdx ) % 3 ) ? 0 : 1 ) : 0;
  
  // Get BCBP of Above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx += ( pcTempCU ) ? ( ( pcTempCU->getInterDir( uiTempPartIdx ) % 3 ) ? 0 : 1 ) : 0;
  
  return uiCtx;
}

Void TComDataCU::setCbfSubParts( UInt uiCbfY, UInt uiCbfU, UInt uiCbfV, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  memset( m_puhCbf[0] + uiAbsPartIdx, uiCbfY, sizeof( UChar ) * uiCurrPartNumb );
  memset( m_puhCbf[1] + uiAbsPartIdx, uiCbfU, sizeof( UChar ) * uiCurrPartNumb );
  memset( m_puhCbf[2] + uiAbsPartIdx, uiCbfV, sizeof( UChar ) * uiCurrPartNumb );
}

Void TComDataCU::setCbfSubParts( UInt uiCbf, TextType eTType, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  memset( m_puhCbf[g_aucConvertTxtTypeToIdx[eTType]] + uiAbsPartIdx, uiCbf, sizeof( UChar ) * uiCurrPartNumb );
}

Void TComDataCU::setDepthSubParts( UInt uiDepth, UInt uiAbsPartIdx )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  memset( m_puhDepth + uiAbsPartIdx, uiDepth, sizeof(UChar)*uiCurrPartNumb );
}

Bool TComDataCU::isFirstAbsZorderIdxInDepth (UInt uiAbsPartIdx, UInt uiDepth)
{
  UInt uiPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  return (((m_uiAbsIdxInLCU + uiAbsPartIdx)% uiPartNumb) == 0);
}

Void TComDataCU::setPartSizeSubParts( PartSize eMode, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  for (UInt ui = 0; ui < uiCurrPartNumb; ui++ )
  {
    m_pePartSize[uiAbsPartIdx + ui] = eMode;
  }
}

Void TComDataCU::setPredModeSubParts( PredMode eMode, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  for (UInt ui = 0; ui < uiCurrPartNumb; ui++ )
  {
    m_pePredMode[uiAbsPartIdx + ui] = eMode;
  }
}

#if DMH
Void TComDataCU::setDMHModeSubPart(Int dmh_mode, UInt uiAbsPartIdx, UInt uiDepth)
{
	UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);

	for (UInt ui = 0; ui < uiCurrPartNumb; ui++)
	{
		m_iDMHMode[uiAbsPartIdx + ui] = dmh_mode;
	}
}
#endif

Void TComDataCU::setInterDirSubParts( UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx )
{
  UInt uiCurrPartNumQ = (m_pcPic->getNumPartInCU() >> (uiDepth << 1)) >> 2;
  switch ( m_pePartSize[ uiAbsPartIdx ] )
  {
    case SIZE_2Nx2N:
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumQ << 2 );                      break;
    case SIZE_2NxN:
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumQ << 1 );                      break;
    case SIZE_Nx2N:
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumQ );
      memset( m_puhInterDir + uiAbsPartIdx + ( uiCurrPartNumQ << 1 ), uiDir, sizeof(UChar)*uiCurrPartNumQ ); break;
    case SIZE_NxN:
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumQ );                           break;
  case SIZE_2NxnU:
    {
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*(uiCurrPartNumQ>>1) );
      if( uiPartIdx == 0 )
        memset( m_puhInterDir + uiAbsPartIdx + uiCurrPartNumQ, uiDir, sizeof(UChar)*(uiCurrPartNumQ>>1) );
      else
        memset( m_puhInterDir + uiAbsPartIdx + uiCurrPartNumQ, uiDir, sizeof(UChar)*( (uiCurrPartNumQ>>1) + (uiCurrPartNumQ<<1) ) );
      break;
    }
  case SIZE_2NxnD:
    {
      if( uiPartIdx == 0 )
      {
        memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*( (uiCurrPartNumQ>>1) + (uiCurrPartNumQ<<1) ) );
        memset( m_puhInterDir + uiAbsPartIdx + ( uiCurrPartNumQ + (uiCurrPartNumQ<<1) ), uiDir, sizeof(UChar)*(uiCurrPartNumQ>>1) );
      }
      else
      {
        memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*(uiCurrPartNumQ>>1) );
        memset( m_puhInterDir + uiAbsPartIdx + uiCurrPartNumQ, uiDir, sizeof(UChar)*(uiCurrPartNumQ>>1) );
      }
      break;
    }
  case SIZE_nLx2N:
    {
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
      memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ<<1), uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
      if( uiPartIdx == 0 )
      {
        memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ>>1), uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
        memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1), uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
      }
      else
      {
        memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ>>1), uiDir, sizeof(UChar)*( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ) );
        memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1), uiDir, sizeof(UChar)*( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ) );
      }
      break;
    }
  case SIZE_nRx2N:
    {
      if( uiPartIdx == 0 )
      {
        memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ) );
        memset( m_puhInterDir + uiAbsPartIdx + uiCurrPartNumQ + (uiCurrPartNumQ>>1), uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
        memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ<<1), uiDir, sizeof(UChar)*( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ) );
        memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ<<1) + uiCurrPartNumQ + (uiCurrPartNumQ>>1), uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
      }
      else
      {
        memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
        memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ>>1), uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
        memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ<<1), uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
        memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1), uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
      }
      break;
    }
    default:
      assert( 0 );
  }
}
#if wlq_FME
Void TComDataCU::getInterPUxy(Int iPartIdx, Int& x, Int& y)
{
	switch(m_pePartSize[0])
	{
		case SIZE_2Nx2N:
			x = m_uiCUPelX; 
			y = m_uiCUPelY;															break;
		case SIZE_2NxN:
			x = m_uiCUPelX; 
			y = m_uiCUPelY + iPartIdx*((1 << m_puhLog2CUSize[0]) >> 1);				break;
		case SIZE_Nx2N:
			x = m_uiCUPelX + iPartIdx*((1 << m_puhLog2CUSize[0]) >> 1); 
			y = m_uiCUPelY;															break;
		case SIZE_NxN:
			x = m_uiCUPelX + (iPartIdx & 1)*((1 << m_puhLog2CUSize[0]) >> 1);   
			y = m_uiCUPelY + (((iPartIdx)>>1)&1)*((1 << m_puhLog2CUSize[0]) >> 1);	break;
		case SIZE_2NxnU:
			x = m_uiCUPelX;
			y = m_uiCUPelY + iPartIdx*((1 << m_puhLog2CUSize[0]) >> 2);				break;
		case SIZE_2NxnD:
			x = m_uiCUPelX;
			y = m_uiCUPelY + iPartIdx*((1 << m_puhLog2CUSize[0]) >> 2)*3;			break;
		case SIZE_nLx2N:
			x = m_uiCUPelX + iPartIdx*((1 << m_puhLog2CUSize[0]) >> 2);
			y = m_uiCUPelY;															break;
		case SIZE_nRx2N:
			x = m_uiCUPelX + iPartIdx*((1 << m_puhLog2CUSize[0]) >> 2)*3;
			y = m_uiCUPelY;															break;
		default:
			assert(0);
	}
}
#endif

#if niu_NSQT_dec   // from   TEncSbac
Void TComDataCU::getTUSize(UInt uiPartIdx, Int& riWidth, Int& riHeight, Int uiLog2Size, PartSize m_pePartSize, Bool bNSQT)
{
	switch (m_pePartSize)
	{
	case SIZE_2Nx2N:
		riWidth = (1 << uiLog2Size);      riHeight = (1 << uiLog2Size);
		break;
	case SIZE_2NxN:
#if RD_NSQT_BUG_YQH
		if (bNSQT&&uiLog2Size>2)
#else
		if (bNSQT)
#endif
		{

#if RD_NSQT_BUG_YQH
			if (uiLog2Size == 5)
				uiLog2Size--;
#endif
			riWidth = (1 << uiLog2Size) << 1;      riHeight = (1 << uiLog2Size) >> 1;
		}
		else
		{
			riWidth = (1 << uiLog2Size);      riHeight = (1 << uiLog2Size);
		}
		break;
	case SIZE_2NxhN: //yuquanhe@hisilicon.com
		if (uiLog2Size == 5)
			uiLog2Size--;
		riWidth = (1 << uiLog2Size);      riHeight = (1 << uiLog2Size) >> 2;
		break;
	case SIZE_hNx2N: //yuquanhe@hisilicon.com
		if (uiLog2Size == 5)
			uiLog2Size--;
		riWidth = (1 << uiLog2Size) >> 2;     riHeight = (1 << uiLog2Size);
		break;
	case SIZE_Nx2N:
#if RD_NSQT_BUG_YQH
		if (bNSQT&&uiLog2Size>2)
#else
		if (bNSQT)
#endif
		{
#if RD_NSQT_BUG_YQH
			if (uiLog2Size == 5)
				uiLog2Size--;
#endif
			riWidth = (1 << uiLog2Size) >> 1;     riHeight = (1 << uiLog2Size) << 1;
		}
		else
		{
			riWidth = (1 << uiLog2Size); riHeight = (1 << uiLog2Size);
		}
		break;
	case SIZE_NxN:
		riWidth = (1 << uiLog2Size); riHeight = (1 << uiLog2Size);
		break;
	case SIZE_2NxnU:
#if RD_NSQT_BUG_YQH
		if (bNSQT&&uiLog2Size>2)
#else
		if (bNSQT)
#endif
		{
#if RD_NSQT_BUG_YQH
			if (uiLog2Size == 5)
				uiLog2Size--;
#endif
			riWidth = (1 << uiLog2Size) << 1;      riHeight = (1 << uiLog2Size) >> 1;
		}
		else
		{
			riWidth = (1 << uiLog2Size);      riHeight = (1 << uiLog2Size);
		}
		break;
	case SIZE_2NxnD:
#if RD_NSQT_BUG_YQH
		if (bNSQT&&uiLog2Size>2)
#else
		if (bNSQT)
#endif
		{
#if RD_NSQT_BUG_YQH
			if (uiLog2Size == 5)
				uiLog2Size--;
#endif
			riWidth = (1 << uiLog2Size) << 1;      riHeight = (1 << uiLog2Size) >> 1;
		}
		else
		{
			riWidth = (1 << uiLog2Size);      riHeight = (1 << uiLog2Size);
		}
		break;
	case SIZE_nLx2N:
#if RD_NSQT_BUG_YQH
		if (bNSQT&&uiLog2Size>2)
#else
		if (bNSQT)
#endif
		{
#if RD_NSQT_BUG_YQH
			if (uiLog2Size == 5)
				uiLog2Size--;
#endif
			riWidth = (1 << uiLog2Size) >> 1;     riHeight = (1 << uiLog2Size) << 1;
		}
		else
		{
			riWidth = (1 << uiLog2Size); riHeight = (1 << uiLog2Size);
		}
		break;
	case SIZE_nRx2N:
#if RD_NSQT_BUG_YQH
		if (bNSQT&&uiLog2Size>2)
#else
		if (bNSQT)
#endif
		{
#if RD_NSQT_BUG_YQH
			if (uiLog2Size == 5)
				uiLog2Size--;
#endif
			riWidth = (1 << uiLog2Size) >> 1;     riHeight = (1 << uiLog2Size) << 1;
		}
		else
		{
			riWidth = (1 << uiLog2Size); riHeight = (1 << uiLog2Size);
		}
		break;
	default:
		riWidth = (1 << uiLog2Size);      riHeight = (1 << uiLog2Size);
		//	assert (0);
		break;
	}
}
#endif

#if ZHANGYI_INTRA_SDIP
Void TComDataCU::setSDIPFlagSubParts(UChar SDIPFlag, UInt uiAbsPartIdx, UInt uiDepth)
{
	UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
	memset(m_pSDIPFlag + uiAbsPartIdx, SDIPFlag, sizeof(UChar)*uiCurrPartNumb);
}

Void TComDataCU::setSDIPDirectionSubParts(UChar SDIPDirection, UInt uiAbsPartIdx, UInt uiDepth)
{
	UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
	memset(m_pSDIPDirection + uiAbsPartIdx, SDIPDirection, sizeof(UChar)*uiCurrPartNumb);
}

UInt  TComDataCU::convertNonSquareUnitToLine(UInt uiAbsPartIdx)
{
	UInt uiNumParts = getPic()->getNumPartInCU() >> (getDepth(uiAbsPartIdx) << 1);
	UInt uiPartIdx = uiAbsPartIdx%uiNumParts;
	return getWidth(uiAbsPartIdx) * uiPartIdx / uiNumParts;
}
UInt  TComDataCU::convertLineToNonSquareUnit(UInt uiAbsPartIdx, UInt uiLine)
{
	UInt uiNumParts = getPic()->getNumPartInCU() >> (getDepth(uiAbsPartIdx) << 1);
	return uiAbsPartIdx - uiAbsPartIdx%uiNumParts + uiNumParts*uiLine / getWidth(uiAbsPartIdx);
}
#endif

#if ZHANGYI_INTRA
Void TComDataCU::setIntraDirSubPartsCb( UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth)
{
	UInt uiCurrPartNum = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
	memset( m_puhIntraDirCb + uiAbsPartIdx , uiDir, sizeof(UChar)*uiCurrPartNum );
}

Void TComDataCU::setIntraDirSubPartsCr( UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth)
{
	UInt uiCurrPartNum = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
	memset( m_puhIntraDirCr + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNum );
}
Void TComDataCU::setIntraDirSubPartsCbValue( Int uiDir, UInt uiAbsPartIdx, UInt uiDepth)
{
	UInt uiCurrPartNum = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
	memset( m_puhIntraDirCbValue + uiAbsPartIdx , uiDir, sizeof(UChar)*uiCurrPartNum );
}

Void TComDataCU::setIntraDirSubPartsCrValue( Int uiDir, UInt uiAbsPartIdx, UInt uiDepth)
{
	UInt uiCurrPartNum = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
	memset( m_puhIntraDirCrValue + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNum );
}
Void TComDataCU::setIntraDirSubPartsValue( Int uiDir, UInt uiAbsPartIdx, UInt uiDepth )
{
	UInt uiCurrPartNum = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
	memset( m_puhIntraDirValue + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNum );
}
#endif
Void TComDataCU::setIntraDirSubParts( UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNum = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  memset( m_puhIntraDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNum );
}

#if inter_direct_skip_bug1
Void TComDataCU::setInterSkipmodeSubParts(UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth)
{
	UInt uiCurrPartNum = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
	memset(m_pcInterSkipmode + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNum);
}
#endif

Void TComDataCU::setTrIdxSubParts( UInt uiTrIdx, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);

  memset( m_puhTrIdx + uiAbsPartIdx, uiTrIdx, sizeof(UChar)*uiCurrPartNumb );
}

Void TComDataCU::setSizeSubParts( UInt uiWidth, UInt uiHeight, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  memset( m_puhWidth  + uiAbsPartIdx, uiWidth,  sizeof(UChar)*uiCurrPartNumb );
  memset( m_puhHeight + uiAbsPartIdx, uiHeight, sizeof(UChar)*uiCurrPartNumb );

  // temporal solution (Sunil)
  memset( m_puhLog2CUSize + uiAbsPartIdx, g_uiLog2MaxCUSize - uiDepth, sizeof(UChar)*uiCurrPartNumb );
}

UChar TComDataCU::getNumPartInter()
{
  UChar iNumPart = 0;
  
  switch ( m_pePartSize[0] )
  {
    case SIZE_2Nx2N:    iNumPart = 1; break;
    case SIZE_2NxN:     iNumPart = 2; break;
    case SIZE_Nx2N:     iNumPart = 2; break;
    case SIZE_NxN:      iNumPart = 4; break;
    case SIZE_2NxnU:    iNumPart = 2; break;
    case SIZE_2NxnD:    iNumPart = 2; break;
    case SIZE_nLx2N:    iNumPart = 2; break;
    case SIZE_nRx2N:    iNumPart = 2; break;
#if ZHANGYI_INTRA
	case SIZE_2NxhN:    iNumPart = 4; break; //SDIP
	case SIZE_hNx2N:    iNumPart = 4; break; //SDIP
#endif
    default:            assert (0);   break;
  }
  
  return  iNumPart;
}

Void TComDataCU::getPartIndexAndSize( UInt uiPartIdx, UInt& ruiPartAddr, Int& riWidth, Int& riHeight )
{
  switch ( m_pePartSize[0] )
  {
    case SIZE_2Nx2N:
      riWidth = getWidth(0);      riHeight = getHeight(0);      ruiPartAddr = 0;
      break;
    case SIZE_2NxN:
      riWidth = getWidth(0);      riHeight = getHeight(0) >> 1; ruiPartAddr = ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1;
      break;
    case SIZE_Nx2N:
      riWidth = getWidth(0) >> 1; riHeight = getHeight(0);      ruiPartAddr = ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 2;
      break;
    case SIZE_NxN:
      riWidth = getWidth(0) >> 1; riHeight = getHeight(0) >> 1; ruiPartAddr = ( m_uiNumPartition >> 2 ) * uiPartIdx;
      break;
    case SIZE_2NxnU:
      riWidth     = getWidth(0);
      riHeight    = ( uiPartIdx == 0 ) ?  getHeight(0) >> 2 : ( getHeight(0) >> 2 ) + ( getHeight(0) >> 1 );
      ruiPartAddr = ( uiPartIdx == 0 ) ? 0 : m_uiNumPartition >> 3;
      break;
    case SIZE_2NxnD:
      riWidth     = getWidth(0);
      riHeight    = ( uiPartIdx == 0 ) ?  ( getHeight(0) >> 2 ) + ( getHeight(0) >> 1 ) : getHeight(0) >> 2;
      ruiPartAddr = ( uiPartIdx == 0 ) ? 0 : (m_uiNumPartition >> 1) + (m_uiNumPartition >> 3);
      break;
    case SIZE_nLx2N:
      riWidth     = ( uiPartIdx == 0 ) ? getWidth(0) >> 2 : ( getWidth(0) >> 2 ) + ( getWidth(0) >> 1 );
      riHeight    = getHeight(0);
      ruiPartAddr = ( uiPartIdx == 0 ) ? 0 : m_uiNumPartition >> 4;
      break;
    case SIZE_nRx2N:
      riWidth     = ( uiPartIdx == 0 ) ? ( getWidth(0) >> 2 ) + ( getWidth(0) >> 1 ) : getWidth(0) >> 2;
      riHeight    = getHeight(0);
      ruiPartAddr = ( uiPartIdx == 0 ) ? 0 : (m_uiNumPartition >> 2) + (m_uiNumPartition >> 4);
      break;
    default:
      assert (0);
      break;
  }
}
#if INTER612
#if niu_mvpred_debug
Void TComDataCU::getPartIndexFromAddr(UInt& uiPartIdx, UInt uiAbsIdxInLCU, UInt uiAbsPartIdx)
#else
Void TComDataCU::getPartIndexFromAddr(UInt& uiPartIdx, UInt ruiPartAddr)
#endif
{
	//test
#if niu_mvpred_debug
	UInt NNum = (1 << (m_pcPicture->getSPS()->getLog2MaxCUSize() - m_puhDepth[uiAbsPartIdx])) * (1 << (m_pcPicture->getSPS()->getLog2MaxCUSize() - m_puhDepth[uiAbsPartIdx])) / 4 / 4;
#endif

#if niu_mvpred_debug
	switch (m_pePartSize[uiAbsPartIdx])
#else
	switch (m_pePartSize[0])
#endif	

	{
	case SIZE_2Nx2N:
		uiPartIdx = 0;
		break;

	case SIZE_2NxN:
	case SIZE_Nx2N:
	case SIZE_2NxnU:
	case SIZE_2NxnD:
	case SIZE_nLx2N:
	case SIZE_nRx2N:

#if niu_mvpred_debug
		uiPartIdx = ((uiAbsIdxInLCU + uiAbsPartIdx) % NNum == 0) ? 0 : 1;		
#else
		uiPartIdx = (ruiPartAddr == 0) ? 0 : 1;
#endif
		break;

	case SIZE_NxN:
#if niu_mvpred_debug
		uiPartIdx = (uiAbsIdxInLCU + uiAbsPartIdx) / (m_uiNumPartition >> 2);
#else
		uiPartIdx = ruiPartAddr / (m_uiNumPartition >> 2);
#endif
		break;
	default:
		assert(0);
		break;
	}
}
#endif
Void TComDataCU::resetDBKIdx(UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth)
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  memset( m_pauhDBKIdx[uiDir] + uiAbsPartIdx, 0, sizeof(UChar)*uiCurrPartNumb );
}
Void TComDataCU::setDBKIdx  (UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth, UChar uiIdx)
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  memset( m_pauhDBKIdx[uiDir] + uiAbsPartIdx, uiIdx, sizeof(UChar)*uiCurrPartNumb );
}

Void TComDataCU::getMv( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPic eRefPic, TComMv& rcMv )
{
  if ( pcCU == NULL )  // OUT OF BOUNDARY
  {
    rcMv.set(0,0);
    return;
  }

  TComCUMvField*  pcCUMvField = pcCU->getCUMvField( eRefPic );
  rcMv = pcCUMvField->getMv( uiAbsPartIdx );
}

Void TComDataCU::deriveLeftRightTopIdx ( UInt uiPartIdx, UInt& ruiPartIdxLT, UInt& ruiPartIdxRT )
{
  ruiPartIdxLT = m_uiAbsIdxInLCU;
  ruiPartIdxRT = g_auiRasterToZscan [g_auiZscanToRaster[ ruiPartIdxLT ] + m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1 ];

  switch ( m_pePartSize[0] )
  {
    case SIZE_2Nx2N:                                                                                                                                break;
    case SIZE_2NxN:
      ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1; ruiPartIdxRT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1;
      break;
    case SIZE_Nx2N:
      ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 2; ruiPartIdxRT -= ( uiPartIdx == 1 )? 0 : m_uiNumPartition >> 2;
      break;
    case SIZE_NxN:
      ruiPartIdxLT += ( m_uiNumPartition >> 2 ) * uiPartIdx;         ruiPartIdxRT +=  ( m_uiNumPartition >> 2 ) * ( uiPartIdx - 1 );
      break;
    case SIZE_2NxnU:
      ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 3;
      ruiPartIdxRT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 3;
      break;
    case SIZE_2NxnD:
      ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : ( m_uiNumPartition >> 1 ) + ( m_uiNumPartition >> 3 );
      ruiPartIdxRT += ( uiPartIdx == 0 )? 0 : ( m_uiNumPartition >> 1 ) + ( m_uiNumPartition >> 3 );
      break;
    case SIZE_nLx2N:
      ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 4;
      ruiPartIdxRT -= ( uiPartIdx == 1 )? 0 : ( m_uiNumPartition >> 2 ) + ( m_uiNumPartition >> 4 );
      break;
    case SIZE_nRx2N:
      ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : ( m_uiNumPartition >> 2 ) + ( m_uiNumPartition >> 4 );
      ruiPartIdxRT -= ( uiPartIdx == 1 )? 0 : m_uiNumPartition >> 4;
      break;
    default:
      assert (0);
      break;
  }

}

Void TComDataCU::deriveLeftBottomIdx( UInt  uiPartIdx,      UInt&      ruiPartIdxLB )
{
  ruiPartIdxLB = g_auiRasterToZscan [g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + ( (1<<(m_puhLog2CUSize[0] - m_pcPic->getLog2MinCUSize() - 1)) - 1)*m_pcPic->getNumPartInWidth()];

  switch ( m_pePartSize[0] )
  {
    case SIZE_2Nx2N:
      ruiPartIdxLB += m_uiNumPartition >> 1;
      break;
    case SIZE_2NxN:
      ruiPartIdxLB += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1;
      break;
    case SIZE_Nx2N:
      ruiPartIdxLB += ( uiPartIdx == 0 )? m_uiNumPartition >> 1 : (m_uiNumPartition >> 2)*3;
      break;
    case SIZE_NxN:
      ruiPartIdxLB += ( m_uiNumPartition >> 2 ) * uiPartIdx;
      break;
    case SIZE_2NxnU:
      ruiPartIdxLB += ( uiPartIdx == 0 ) ? -((Int)m_uiNumPartition >> 3) : m_uiNumPartition >> 1;
      break;
    case SIZE_2NxnD:
      ruiPartIdxLB += ( uiPartIdx == 0 ) ? (m_uiNumPartition >> 2) + (m_uiNumPartition >> 3): m_uiNumPartition >> 1;
      break;
    case SIZE_nLx2N:
      ruiPartIdxLB += ( uiPartIdx == 0 ) ? m_uiNumPartition >> 1 : (m_uiNumPartition >> 1) + (m_uiNumPartition >> 4);
      break;
    case SIZE_nRx2N:
      ruiPartIdxLB += ( uiPartIdx == 0 ) ? m_uiNumPartition >> 1 : (m_uiNumPartition >> 1) + (m_uiNumPartition >> 2) + (m_uiNumPartition >> 4);
      break;
    default:
      assert (0);
      break;
  }

}

Void TComDataCU::deriveLeftRightTopIdxAdi ( UInt& ruiPartIdxLT, UInt& ruiPartIdxRT, UInt uiPartOffset, UInt uiPartDepth )
{
  UInt uiNumPartInWidth = (m_puhWidth[0]/m_pcPic->getMinCUWidth())>>uiPartDepth;

  ruiPartIdxLT = m_uiAbsIdxInLCU + uiPartOffset;
  ruiPartIdxRT = g_auiRasterToZscan[ g_auiZscanToRaster[ ruiPartIdxLT ] + uiNumPartInWidth - 1 ];
}

Void TComDataCU::deriveLeftBottomIdxAdi( UInt& ruiPartIdxLB, UInt uiPartOffset, UInt uiPartDepth )
{
  UInt uiAbsIdx;
  UInt uiLog2MinCuSize, uiWidthInMinCus;
  
  uiLog2MinCuSize = getPic()->getLog2MinCUSize();
  uiWidthInMinCus = 1<<(getLog2CUSize(0)-uiLog2MinCuSize-uiPartDepth);
  uiAbsIdx        = getZorderIdxInCU()+uiPartOffset+(m_uiNumPartition>>(uiPartDepth<<1))-1;
  uiAbsIdx        = g_auiZscanToRaster[uiAbsIdx]-(uiWidthInMinCus-1);
  ruiPartIdxLB    = g_auiRasterToZscan[uiAbsIdx];
}


#if 0//ZHANGYI_INTRA_SDIP
TComMv TComDataCU::getMvPredDec(UInt uiAbsPartIdx, RefPic eRefPic)
{
	TComDataCU* pLeftCU;
	TComDataCU* pAboveCU;
	TComDataCU* pCornerCU;
	UInt        uiLeftIdx, uiAboveIdx, uiCornerIdx;
	Bool        bLeft = false, bAbove = false, bCorner = false;

#if INTER612
	UInt partIdx = 0;

	//test
#if niu_mvpred_debug
	getPartIndexFromAddr(partIdx, m_uiAbsIdxInLCU, uiAbsPartIdx);
#else
	getPartIndexFromAddr(partIdx, uiAbsPartIdx);
#endif
	Int Pixel_X = m_puiBlockPosition[(UInt)m_pePartSize[uiAbsPartIdx] * 2 + partIdx][0] <<
		(m_pcPicture->getSPS()->getLog2MaxCUSize() - m_puhDepth[uiAbsPartIdx] - MIN_CU_SIZE_IN_BIT);
	Int Pixel_Y = m_puiBlockPosition[(UInt)m_pePartSize[uiAbsPartIdx] * 2 + partIdx][1] <<
		(m_pcPicture->getSPS()->getLog2MaxCUSize() - m_puhDepth[uiAbsPartIdx] - MIN_CU_SIZE_IN_BIT);
#endif



#if INTER611
	Int blockShape_X = m_puiBlockSize[(UInt)m_pePartSize[uiAbsPartIdx] * 2 + partIdx][0] <<
		(m_pcPicture->getSPS()->getLog2MaxCUSize() - m_puhDepth[uiAbsPartIdx] - MIN_CU_SIZE_IN_BIT);
	Int blockShape_Y = m_puiBlockSize[(UInt)m_pePartSize[uiAbsPartIdx] * 2 + partIdx][1] <<
		(m_pcPicture->getSPS()->getLog2MaxCUSize() - m_puhDepth[uiAbsPartIdx] - MIN_CU_SIZE_IN_BIT);
	blockShape_X = blockShape_X % MIN_BLOCK_SIZE == 0 ? blockShape_X : MIN_BLOCK_SIZE;
	blockShape_Y = blockShape_Y % MIN_BLOCK_SIZE == 0 ? blockShape_Y : MIN_BLOCK_SIZE;
	Int shift = blockShape_X / (1 << (getPicture()->getSPS()->getLog2MaxCUSize() - g_uiMaxCUDepth));

#endif


	pLeftCU = getPULeftSDIP(uiLeftIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);
	pAboveCU = getPUAboveSDIP(uiAboveIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);
#if INTER611
	pCornerCU = getPUAboveRight(uiCornerIdx, m_uiAbsIdxInLCU + uiAbsPartIdx,shift);
#else
	pCornerCU = getPUAboveRight(uiCornerIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);
#endif
	if (!pCornerCU) pCornerCU = getPUAboveLeft(uiCornerIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);

	// check availability
	if (pLeftCU && !pLeftCU->isIntra(uiLeftIdx))
	{
		bLeft = true;
	}
	if (pAboveCU && !pAboveCU->isIntra(uiAboveIdx))
	{
		bAbove = true;
	}
	if (pCornerCU && !pCornerCU->isIntra(uiCornerIdx))
	{
		bCorner = true;
	}

	if (bLeft)
	{
		m_cMvPred[eRefPic] = pLeftCU->getCUMvField(eRefPic)->getMv(uiLeftIdx);
	}
	else if (bAbove)
	{
		m_cMvPred[eRefPic] = pAboveCU->getCUMvField(eRefPic)->getMv(uiAboveIdx);
	}
	else if (bCorner)
	{
		m_cMvPred[eRefPic] = pCornerCU->getCUMvField(eRefPic)->getMv(uiCornerIdx);
	}
	else
	{
		m_cMvPred[eRefPic].setZero();
	}

	return m_cMvPred[eRefPic];
}
#endif

#if F_DHP_SYC
Void TComDataCU::MV_Scale(TComDataCU *pcCU, TComMv &cMvPred, TComMv tempmv, Int iSrcRef, Int iDstRef)
{
	Int SrcT = (2 * (pcCU->getPicture()->getPOC() - pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_0, iSrcRef)) + 512) % 512;
	Int DstT = (2 * (pcCU->getPicture()->getPOC() - pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_0, iDstRef)) + 512) % 512;
	cMvPred.setHor(Clip3(-32768, 32767, ((long long int)(SrcT)* tempmv.getHor() * (MULTI / DstT) + HALF_MULTI) >> OFFSET));
	cMvPred.setVer(Clip3(-32768, 32767, ((long long int)(SrcT)* tempmv.getVer() * (MULTI / DstT) + HALF_MULTI) >> OFFSET));
}
#endif


Void TComDataCU::clipMv    (TComMv&  rcMv)
{
  Int  iMvShift = 2;
  Int iHorMax = (m_pcPicture->getSPS()->getWidth() - m_uiCUPelX - 1 )<<iMvShift;
  Int iHorMin = (      -(Int)(1<<g_uiLog2MaxCUSize) - (Int)m_uiCUPelX + 1 )<<iMvShift;
  
  Int iVerMax = (m_pcPicture->getSPS()->getHeight() - m_uiCUPelY - 1 )<<iMvShift;
  Int iVerMin = (      -(Int)(1<<g_uiLog2MaxCUSize) - (Int)m_uiCUPelY + 1 )<<iMvShift;
  
  rcMv.setHor( Min (iHorMax, Max (iHorMin, rcMv.getHor())) );
  rcMv.setVer( Min (iVerMax, Max (iVerMin, rcMv.getVer())) );
}

#if RD_MVP

#if HALF_PIXEL_COMPENSATION
Int TComDataCU::getDeltas(Int *piDelta,          //delt for original MV
	Int *piDelta2,         //delt for scaled MV
	Int iOrigPOC, Int iOrigRefPOC, Int iScaledPOC, Int iScaledRefPOC)
{
	Int iFactor = 2;
	*piDelta = 0;
	*piDelta2 = 0;

	if (!m_pcPic->getPicture()->getFieldSequence())
		//if (!m_pcPic->getPicture()->getSeqHeader()->getRepeatFirstField())
	{
		return -1;
	}
	//assert(img->picture_structure == 1 );

	iOrigPOC = (iOrigPOC + 512) % 512;
	iOrigRefPOC = (iOrigRefPOC + 512) % 512;
	iScaledPOC = (iScaledPOC + 512) % 512;
	iScaledRefPOC = (iScaledRefPOC + 512) % 512;
	//assert((OriPOC % factor) + (OriRefPOC % factor) + (ScaledPOC % factor) + (ScaledRefPOC % factor) == 0);

	iOrigPOC /= iFactor;
	iOrigRefPOC /= iFactor;
	iScaledPOC /= iFactor;
	iScaledRefPOC /= iFactor;

	if (m_pcPic->getPicture()->getPicHeader()->getTopFieldPicFlag())
	{
		//Scaled is top field
		*piDelta2 = (iScaledRefPOC % 2) != (iScaledPOC % 2) ? 2 : 0;

		if ((iScaledPOC % 2) == (iOrigPOC % 2))
		{
			//ori is top
			*piDelta = (iOrigRefPOC % 2) != (iOrigPOC % 2) ? 2 : 0;
		}
		else
		{
			*piDelta = (iOrigRefPOC % 2) != (iOrigPOC % 2) ? -2 : 0;
		}
	}
	else
	{
		//Scaled is bottom field
		*piDelta2 = (iScaledRefPOC % 2) != (iScaledPOC % 2) ? -2 : 0;
		if ((iScaledPOC % 2) == (iOrigPOC % 2))
		{
			//ori is bottom
			*piDelta = (iOrigRefPOC % 2) != (iOrigPOC % 2) ? -2 : 0;
		}
		else
		{
			*piDelta = (iOrigRefPOC % 2) != (iOrigPOC % 2) ? 2 : 0;
		}
	}
	return 0;
}
#endif

Int TComDataCU::calculateDistance(RefPic eRefPic, Int eBlkRef, Int iFwBw)    //fw_bw>=: forward prediction.
{
	Int iDistance = 1;

	//if ( img->picture_structure == 1 ) // frame
	{
		if ((m_pcPic->getPicture()->getPictureType() == F_PICTURE) || (m_pcPic->getPicture()->getPictureType() == P_PICTURE))
		{
#if FIX_MAX_REF
			//TComPic* rpcRefPic = m_pcPic->getPicture()->getRefPic(eBlkRef);
			iDistance = m_pcPic->getPOC() * 2 - m_pcPic->getPicture()->getPicHeader()->getRefPOC(eRefPic, eBlkRef) * 2;
			//iDistance = 2;
			//iDistance = abs(m_pcPicture->getPicHeader()->getRPS()->getRefDeltaPOC(eBlkRef));
#else
			if (eBlkRef == REF_PIC_0) {
				TComPic* rpcRefPic = m_pcPic->getPicture()->getRefPic(REF_PIC_0);
				iDistance = m_pcPic->getPicture()->getPicDist() * 2 - rpcRefPic->getPicture()->getFwRefDist() * 2;
			}
			else if (eBlkRef == REF_PIC_1) {
				TComPic* rpcRefPic = m_pcPic->getPicture()->getRefPic(REF_PIC_1);
				iDistance = m_pcPic->getPicture()->getPicDist() * 2 - rpcRefPic->getPicture()->getFwRefDist() * 2;
			}
			else {
				assert(0);    //only two reference pictures for P frame
			}
#endif
		}
		else
		{
			//B_IMG
#if B_ME_REVISE
			if (iFwBw >= 0)
			{
				iDistance = m_pcPic->getPOC() * 2 - m_pcPic->getPicture()->getPicHeader()->getRefPOC(eRefPic, 0) * 2;
			}
			else
			{
				iDistance = m_pcPic->getPicture()->getPicHeader()->getRefPOC(eRefPic, 0) * 2 - m_pcPic->getPOC() * 2;
			}
#else
			if (iFwBw >= 0)
			{
				TComPic* rpcRefPic = m_pcPic->getPicture()->getPicHeader()->getRefPic(REF_PIC_0, eBlkRef);
				iDistance = m_pcPic->getPicture()->getPicDistance() * 2 - rpcRefPic->getPicture()->getFwRefDistance() * 2;
			}
			else
			{
				iDistance = m_pcPic->getPicture()->getPicTrNextP() * 2 - m_pcPic->getPicture()->getPicDistance() * 2;
			}
#endif
		}
	}

	iDistance = (iDistance + 512) % 512;
	return iDistance;
}

Int TComDataCU::scaleMotionVector_y1(Int iMvY, RefPic eRefPic, Int eCurrBlkRef, Int eNbrBlkRef, Int iRef)
{
	Int iSrcDist = calculateDistance(eRefPic, eNbrBlkRef, iRef);
	Int iDstDist = calculateDistance(eRefPic, eCurrBlkRef, iRef);
	//Int iOrigPOC = 2 * m_pcPic->getPicture()->getPicDistance();
	Int iOrigPOC = 2 * (m_pcPic->getPOC() - m_pcPic->getPicHeader()->getRPS()->getRefPOC(eCurrBlkRef));
	Int iOrigRefPOC = iOrigPOC - iSrcDist;
	//Int iScaledPOC = 2 * m_pcPic->getPicture()->getPicDistance();
	Int iScaledPOC = 2 * (m_pcPic->getPOC() - m_pcPic->getPicHeader()->getRPS()->getRefPOC(eCurrBlkRef));
	Int iScaledRefPOC = iScaledPOC - iDstDist;
	Int iDelta, iDelta2;

	getDeltas(&iDelta, &iDelta2, iOrigPOC, iOrigRefPOC, iScaledPOC, iScaledRefPOC);

#if MV_CLIP
	return (Int)(scaleMotionVector(iMvY + iDelta, eRefPic, eCurrBlkRef, eNbrBlkRef, iRef, -iDelta2));
#else
	return (Int)(scaleMotionVector(iMvY + piDelta, iCurrBlkRef, iNbrBlkRef, iRef) - piDelta2);
#endif
}

#if MV_CLIP
Int TComDataCU::scaleMotionVector(Int iMotionVector, RefPic eRefPic, Int eCurrBlkRef, Int eNbrBlkRef, Int iRef, Int iDelta2)
#else
Int TComDataCU::scaleMotionVector(Int rcMotionVector, Int eCurrBlkRef, Int eNbrBlkRef, Int iRef)
#endif
{
	Int iSign = (iMotionVector > 0) ? 1 : -1;
	Int iMultiDistance, iDeviceDistance;

	iMotionVector = abs(iMotionVector);

#if REMOVE_UNUSED
	if (iMotionVector == 0)
	{
		return iDelta2;
	}
#else
	if (iMotionVector == 0)
	{
		return 0;
	}
#endif

	iMultiDistance = calculateDistance(eRefPic, eCurrBlkRef, iRef);
	iDeviceDistance = calculateDistance(eRefPic, eNbrBlkRef, iRef);

#if MV_RANG
	iMotionVector = Clip3(-32768, 32767, (iSign * (((long long int)(iMotionVector)* iMultiDistance * (MULTI / iDeviceDistance) + HALF_MULTI) >> OFFSET)) + iDelta2);
#else
	iMotionVector = iSign * ((iMotionVector * iMultiDistance * (MULTI / iDeviceDistance) + HALF_MULTI) >> OFFSET);
	iMotionVector = Clip3(-32768, 32767, iMotionVector);
#endif
	return iMotionVector;
}
#if PSKIP
Void  TComDataCU::setPSkipMvField(UInt uiAbsPartIdx)
{
	TComDataCU* pLeftCU;
	TComDataCU* pAboveCU;
	TComDataCU* iAboveRightCU; //
	TComDataCU* iAboveLeftCU; //
	TComDataCU* pLeftCU1;
	TComDataCU* pAboveCU1;

	/*TComMv rcLeftMvHv[2];
	TComMv rcAboveMvHv[2];
	TComMv rcAboveRightMvHv[2];
	TComMv rcAboveLeftMvHv[2];
	TComMv rcLeftCU1MvHv[2];
	TComMv rcAboveCU1MvHv[2];*/

	Int rcLeftMvHv[2][2];
	Int rcAboveMvHv[2][2];
	Int rcAboveRightMvHv[2][2];
	Int rcAboveLeftMvHv[2][2];
	Int rcLeftCU1MvHv[2][2];
	Int rcAboveCU1MvHv[2][2];

	UInt uiLeftIdx, uiAboveIdx, iAboveRightIdx, iAboveLeftIdx, uiLeftIdx1, uiAboveIdx1;
	Int iLeftRef[2], iAboveRef[2], iAboveRightRef[3], iAboveLeftRef[2], iLeftRef1[2], iAboveRef1[2];

	Int pRefFrame[2][6];
	Int pmv[2][2][6];
	Int bid_flag = 0, bw_flag = 0, fw_flag = 0, sym_flag = 0, bid2 = 0, fw2 = 0;

	Int shift = 0;

	Int blockShape_X = MIN_BLOCK_SIZE << (m_pcPicture->getSPS()->getLog2MaxCUSize() - m_puhDepth[uiAbsPartIdx] - MIN_BLOCK_SIZE_IN_BIT);
	Int blockShape_Y = MIN_BLOCK_SIZE << (m_pcPicture->getSPS()->getLog2MaxCUSize() - m_puhDepth[uiAbsPartIdx] - MIN_BLOCK_SIZE_IN_BIT);
	blockShape_X = blockShape_X % MIN_BLOCK_SIZE == 0 ? blockShape_X : MIN_BLOCK_SIZE;
	blockShape_Y = blockShape_Y % MIN_BLOCK_SIZE == 0 ? blockShape_Y : MIN_BLOCK_SIZE;
	pLeftCU = getPULeft(uiLeftIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);
	pAboveCU = getPUAbove(uiAboveIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);

	shift = blockShape_X / (1 << (getPicture()->getSPS()->getLog2MaxCUSize() - g_uiMaxCUDepth));
	iAboveRightCU = getPUAboveRight(iAboveRightIdx, m_uiAbsIdxInLCU + uiAbsPartIdx, shift);

	iAboveLeftCU = getPUAboveLeft(iAboveLeftIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);

	shift = blockShape_Y / (1 << (getPicture()->getSPS()->getLog2MaxCUSize() - g_uiMaxCUDepth));
	pLeftCU1 = getPULeft1(uiLeftIdx1, m_uiAbsIdxInLCU + uiAbsPartIdx, shift);
	shift = blockShape_X / (1 << (getPicture()->getSPS()->getLog2MaxCUSize() - g_uiMaxCUDepth));
	pAboveCU1 = getPUAbove1(uiAboveIdx1, m_uiAbsIdxInLCU + uiAbsPartIdx, shift);

	iLeftRef[0] = pLeftCU ? pLeftCU->getCUMvField(REF_PIC_0)->getRefIdx(uiLeftIdx) : -1;
	iAboveRef[0] = pAboveCU ? pAboveCU->getCUMvField(REF_PIC_0)->getRefIdx(uiAboveIdx) : -1;
	iAboveRightRef[0] = iAboveRightCU ? iAboveRightCU->getCUMvField(REF_PIC_0)->getRefIdx(iAboveRightIdx) : -1;
	iAboveLeftRef[0] = iAboveLeftCU ? iAboveLeftCU->getCUMvField(REF_PIC_0)->getRefIdx(iAboveLeftIdx) : -1;

	iLeftRef1[0] = pLeftCU1 ? pLeftCU1->getCUMvField(REF_PIC_0)->getRefIdx(uiLeftIdx1) : -1;
	iAboveRef1[0] = pAboveCU1 ? pAboveCU1->getCUMvField(REF_PIC_0)->getRefIdx(uiAboveIdx1) : -1;

	iLeftRef[1] = pLeftCU ? pLeftCU->getCUMvField(REF_PIC_1)->getRefIdx(uiLeftIdx) : -1;
	iAboveRef[1] = pAboveCU ? pAboveCU->getCUMvField(REF_PIC_1)->getRefIdx(uiAboveIdx) : -1;
	iAboveRightRef[1] = iAboveRightCU ? iAboveRightCU->getCUMvField(REF_PIC_1)->getRefIdx(iAboveRightIdx) : -1;
	iAboveLeftRef[1] = iAboveLeftCU ? iAboveLeftCU->getCUMvField(REF_PIC_1)->getRefIdx(iAboveLeftIdx) : -1;

	iLeftRef1[1] = pLeftCU1 ? pLeftCU1->getCUMvField(REF_PIC_1)->getRefIdx(uiLeftIdx1) : -1;
	iAboveRef1[1] = pAboveCU1 ? pAboveCU1->getCUMvField(REF_PIC_1)->getRefIdx(uiAboveIdx1) : -1;

	for (Int i = 0; i < 2; i++) {
		pRefFrame[i][0] = iAboveLeftRef[i];
		pRefFrame[i][1] = iAboveRef[i];
		pRefFrame[i][2] = iLeftRef[i];
		pRefFrame[i][3] = iAboveRightRef[i];
		pRefFrame[i][4] = iAboveRef1[i];
		pRefFrame[i][5] = iLeftRef1[i];
	}

	for (Int hv = 0; hv < 2; hv++)
	{
		rcLeftMvHv[0][hv] = pLeftCU ? pLeftCU->getCUMvField(REF_PIC_0)->getMv(uiLeftIdx)[hv] : 0;
		rcAboveMvHv[0][hv] = pAboveCU ? pAboveCU->getCUMvField(REF_PIC_0)->getMv(uiAboveIdx)[hv] : 0;
		rcAboveRightMvHv[0][hv] = iAboveRightCU ? iAboveRightCU->getCUMvField(REF_PIC_0)->getMv(iAboveRightIdx)[hv] : 0;
		rcAboveLeftMvHv[0][hv] = iAboveLeftCU ? iAboveLeftCU->getCUMvField(REF_PIC_0)->getMv(iAboveLeftIdx)[hv] : 0;
		rcLeftCU1MvHv[0][hv] = pLeftCU1 ? pLeftCU1->getCUMvField(REF_PIC_0)->getMv(uiLeftIdx1)[hv] : 0;
#if F_MHPSKIP_SYC_DEBUG
		rcAboveCU1MvHv[0][hv] = pAboveCU1 ? pAboveCU1->getCUMvField(REF_PIC_0)->getMv(uiAboveIdx1)[hv] : 0;
#else
		rcAboveCU1MvHv[0][hv] = pAboveCU1 ? pAboveCU1->getCUMvField(REF_PIC_0)->getMv(uiLeftIdx1)[hv] : 0;
#endif

		rcLeftMvHv[1][hv] = pLeftCU ? pLeftCU->getCUMvField(REF_PIC_1)->getMv(uiLeftIdx)[hv] : 0;
		rcAboveMvHv[1][hv] = pAboveCU ? pAboveCU->getCUMvField(REF_PIC_1)->getMv(uiAboveIdx)[hv] : 0;
		rcAboveRightMvHv[1][hv] = iAboveRightCU ? iAboveRightCU->getCUMvField(REF_PIC_1)->getMv(iAboveRightIdx)[hv] : 0;
		rcAboveLeftMvHv[1][hv] = iAboveLeftCU ? iAboveLeftCU->getCUMvField(REF_PIC_1)->getMv(iAboveLeftIdx)[hv] : 0;
		rcLeftCU1MvHv[1][hv] = pLeftCU1 ? pLeftCU1->getCUMvField(REF_PIC_1)->getMv(uiLeftIdx1)[hv] : 0;
#if F_MHPSKIP_SYC_DEBUG
		rcAboveCU1MvHv[1][hv] = pAboveCU1 ? pAboveCU1->getCUMvField(REF_PIC_1)->getMv(uiAboveIdx1)[hv] : 0;
#else
		rcAboveCU1MvHv[1][hv] = pAboveCU1 ? pAboveCU1->getCUMvField(REF_PIC_1)->getMv(uiLeftIdx1)[hv] : 0;
#endif

		for (Int i = 0; i < 2; i++)
		{
			pmv[i][hv][0] = rcAboveLeftMvHv[i][hv];   //i:0-first;1-second, hv-x or y
			pmv[i][hv][1] = rcAboveMvHv[i][hv];
			pmv[i][hv][2] = rcLeftMvHv[i][hv];
			pmv[i][hv][3] = rcAboveRightMvHv[i][hv];
			pmv[i][hv][4] = rcAboveCU1MvHv[i][hv];
			pmv[i][hv][5] = rcLeftCU1MvHv[i][hv];
		}
	}
	for (Int dir = 0; dir < MH_PSKIP_NUM + NUM_OFFSET + 1; dir++)
	{
		m_cTmpFirstMvPred[dir].setMvField(TComMv(), 0);     // 进行RDO决策时 使用 参见RD rdopt.c    cMCParam.dir = dir;
		m_cTmpSecondMvPred[dir].setMvField(TComMv(), 0);    // 进行RDO决策时 使用
	}

	for (Int j = 0; j < 6; j++)
	{
		// bid
		if (pRefFrame[0][j] != -1 && pRefFrame[1][j] != -1)
		{
			m_cTmpFirstMvPred[BID_P_FST].setRefIdx(pRefFrame[0][j]);
			m_cTmpSecondMvPred[BID_P_FST].setRefIdx(pRefFrame[1][j]);
			m_cTmpFirstMvPred[BID_P_FST].setMv(pmv[0][0][j], pmv[0][1][j]);
			m_cTmpSecondMvPred[BID_P_FST].setMv(pmv[1][0][j], pmv[1][1][j]);

			bid_flag++;
			if (bid_flag == 1)
			{
				bid2 = j;
			}
		}
		// fw
		else if (pRefFrame[0][j] != -1 && pRefFrame[1][j] == -1)
		{
			m_cTmpFirstMvPred[FW_P_FST].setRefIdx(pRefFrame[0][j]);
			m_cTmpFirstMvPred[FW_P_FST].setMv(pmv[0][0][j], pmv[0][1][j]);
			fw_flag++;
			if (fw_flag == 1)
			{
				fw2 = j;
			}
		}
	}
	//first bid
	if (bid_flag == 0 && fw_flag > 1)
	{
		m_cTmpFirstMvPred[FW_P_FST].setRefIdx(m_cTmpFirstMvPred[BID_P_FST].getRefIdx());
		m_cTmpSecondMvPred[BID_P_FST].setRefIdx(pRefFrame[0][fw2]);

		m_cTmpFirstMvPred[BID_P_FST].setMv(m_cTmpFirstMvPred[FW_P_FST].getHor(), m_cTmpFirstMvPred[FW_P_FST].getVer());
		m_cTmpSecondMvPred[BID_P_FST].setMv(pmv[0][0][fw2], pmv[0][1][fw2]);

	}

	//second bid
	if (bid_flag > 1)
	{
		m_cTmpFirstMvPred[BID_P_SND].setRefIdx(pRefFrame[0][bid2]);
		m_cTmpSecondMvPred[BID_P_SND].setRefIdx(pRefFrame[1][bid2]);
		m_cTmpFirstMvPred[BID_P_SND].setMv(pmv[0][0][bid2], pmv[0][1][bid2]);
		m_cTmpSecondMvPred[BID_P_SND].setMv(pmv[1][0][bid2], pmv[1][1][bid2]);
	}
	else if (bid_flag == 1 && fw_flag > 1)
	{
		m_cTmpFirstMvPred[BID_P_SND].setRefIdx(m_cTmpFirstMvPred[FW_P_FST].getRefIdx());
		m_cTmpSecondMvPred[BID_P_SND].setRefIdx(pRefFrame[0][fw2]);
		m_cTmpFirstMvPred[BID_P_SND].setMv(m_cTmpFirstMvPred[FW_P_FST].getHor(), m_cTmpFirstMvPred[FW_P_FST].getHor());
		m_cTmpSecondMvPred[BID_P_SND].setMv(pmv[0][0][fw2], pmv[0][1][fw2]);
	}
	//first fw
	if (fw_flag == 0 && bid_flag > 1)
	{
		m_cTmpFirstMvPred[FW_P_FST].setRefIdx(pRefFrame[0][bid2]);
		m_cTmpFirstMvPred[FW_P_FST].setMv(pmv[0][0][bid2], pmv[0][1][bid2]);
	}
	else if (fw_flag == 0 && bid_flag == 1)
	{
		m_cTmpFirstMvPred[FW_P_FST].setRefIdx(m_cTmpFirstMvPred[BID_P_FST].getRefIdx());
		m_cTmpFirstMvPred[FW_P_FST].setMv(m_cTmpFirstMvPred[BID_P_FST].getHor(), m_cTmpFirstMvPred[BID_P_FST].getVer());
	}
	// second fw
	if (fw_flag > 1)
	{
		m_cTmpFirstMvPred[FW_P_SND].setRefIdx(pRefFrame[0][fw2]);
		m_cTmpFirstMvPred[FW_P_SND].setMv(pmv[0][0][fw2], pmv[0][1][fw2]);
	}
	else if (bid_flag > 1)
	{
		m_cTmpFirstMvPred[FW_P_SND].setRefIdx(pRefFrame[1][bid2]);
		m_cTmpFirstMvPred[FW_P_SND].setMv(pmv[1][0][bid2], pmv[1][1][bid2]);
	}
	else if (bid_flag == 1)
	{
		m_cTmpFirstMvPred[FW_P_SND].setRefIdx(m_cTmpSecondMvPred[BID_P_FST].getRefIdx());
		m_cTmpFirstMvPred[FW_P_SND].setMv(m_cTmpSecondMvPred[BID_P_FST].getHor(), m_cTmpSecondMvPred[BID_P_FST].getVer());
	}

}
#endif

#if BSKIP
Void TComDataCU::setSkipMotionVectorPredictor(UInt uiAbsPartIdx)
{
	TComDataCU* pLeftCU;
	TComDataCU* pAboveCU;
	TComDataCU* iAboveRightCU; //
	TComDataCU* iAboveLeftCU; //
	TComDataCU* pLeftCU1;
	TComDataCU* pAboveCU1;
	// Int mva[2][2], mvb[2][2], mvc[2][2];
	// Int mva1[2][2], mvb1[2][2], mve[2][2];

	Int rcLeftMvHv[2][2];
	Int rcAboveMvHv[2][2];
	Int rcAboveRightMvHv[2][2];
	Int rcAboveLeftMvHv[2][2];
	Int rcLeftCU1MvHv[2][2];
	Int rcAboveCU1MvHv[2][2];

	UInt uiLeftIdx, uiAboveIdx, iAboveRightIdx, iAboveLeftIdx, uiLeftIdx1, uiAboveIdx1;
	Int iLeftRef[2], iAboveRef[2], iAboveRightRef[2], iAboveLeftRef[2], iLeftRef1[2], iAboveRef1[2];

	Int bRefFrame[2][6];
	Int pmv[2][2][6];
	Int i, j, dir;
	Int bid_flag = 0, bw_flag = 0, fw_flag = 0, sym_flag = 0, bid2;
	Int modeInfo[6];
	Int shift = 0;
	Int blockShape_X = MIN_BLOCK_SIZE << (m_pcPicture->getSPS()->getLog2MaxCUSize() - m_puhDepth[uiAbsPartIdx] - MIN_CU_SIZE_IN_BIT);
	Int blockShape_Y = MIN_BLOCK_SIZE << (m_pcPicture->getSPS()->getLog2MaxCUSize() - m_puhDepth[uiAbsPartIdx] - MIN_CU_SIZE_IN_BIT);
	blockShape_X = blockShape_X % MIN_BLOCK_SIZE == 0 ? blockShape_X : MIN_BLOCK_SIZE;
	blockShape_Y = blockShape_Y % MIN_BLOCK_SIZE == 0 ? blockShape_Y : MIN_BLOCK_SIZE;

	pLeftCU = getPULeft(uiLeftIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);
	pAboveCU = getPUAbove(uiAboveIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);

	shift = blockShape_X / (1 << (getPicture()->getSPS()->getLog2MaxCUSize() - g_uiMaxCUDepth));
	iAboveRightCU = getPUAboveRight(iAboveRightIdx, m_uiAbsIdxInLCU + uiAbsPartIdx, shift);

	iAboveLeftCU = getPUAboveLeft(iAboveLeftIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);

	shift = blockShape_Y / (1 << (getPicture()->getSPS()->getLog2MaxCUSize() - g_uiMaxCUDepth));
	pLeftCU1 = getPULeft1(uiLeftIdx1, m_uiAbsIdxInLCU + uiAbsPartIdx, shift);
	shift = blockShape_X / (1 << (getPicture()->getSPS()->getLog2MaxCUSize() - g_uiMaxCUDepth));
	pAboveCU1 = getPUAbove1(uiAboveIdx1, m_uiAbsIdxInLCU + uiAbsPartIdx, shift);

	iLeftRef[0] = pLeftCU ? pLeftCU->getCUMvField(REF_PIC_0)->getRefIdx(uiLeftIdx) : -1;
	iAboveRef[0] = pAboveCU ? pAboveCU->getCUMvField(REF_PIC_0)->getRefIdx(uiAboveIdx) : -1;
	iAboveRightRef[0] = iAboveRightCU ? iAboveRightCU->getCUMvField(REF_PIC_0)->getRefIdx(iAboveRightIdx) : -1;
	iAboveLeftRef[0] = iAboveLeftCU ? iAboveLeftCU->getCUMvField(REF_PIC_0)->getRefIdx(iAboveLeftIdx) : -1;

	iLeftRef1[0] = pLeftCU1 ? pLeftCU1->getCUMvField(REF_PIC_0)->getRefIdx(uiLeftIdx1) : -1;
	iAboveRef1[0] = pAboveCU1 ? pAboveCU1->getCUMvField(REF_PIC_0)->getRefIdx(uiAboveIdx1) : -1;

	iLeftRef[1] = pLeftCU ? pLeftCU->getCUMvField(REF_PIC_1)->getRefIdx(uiLeftIdx) : -1;
	iAboveRef[1] = pAboveCU ? pAboveCU->getCUMvField(REF_PIC_1)->getRefIdx(uiAboveIdx) : -1;
	iAboveRightRef[1] = iAboveRightCU ? iAboveRightCU->getCUMvField(REF_PIC_1)->getRefIdx(iAboveRightIdx) : -1;
	iAboveLeftRef[1] = iAboveLeftCU ? iAboveLeftCU->getCUMvField(REF_PIC_1)->getRefIdx(iAboveLeftIdx) : -1;

	iLeftRef1[1] = pLeftCU1 ? pLeftCU1->getCUMvField(REF_PIC_1)->getRefIdx(uiLeftIdx1) : -1;
	iAboveRef1[1] = pAboveCU1 ? pAboveCU1->getCUMvField(REF_PIC_1)->getRefIdx(uiAboveIdx1) : -1;

	for (Int i = 0; i < 2; i++)
	{
		bRefFrame[i][0] = iAboveLeftRef[i];
		bRefFrame[i][1] = iAboveRef[i];
		bRefFrame[i][2] = iLeftRef[i];
		bRefFrame[i][3] = iAboveRightRef[i];
		bRefFrame[i][4] = iAboveRef1[i];
		bRefFrame[i][5] = iLeftRef1[i];
	}
	for (Int hv = 0; hv < 2; hv++)
	{
		rcLeftMvHv[0][hv] = pLeftCU ? pLeftCU->getCUMvField(REF_PIC_0)->getMv(uiLeftIdx)[hv] : 0;
		rcAboveMvHv[0][hv] = pAboveCU ? pAboveCU->getCUMvField(REF_PIC_0)->getMv(uiAboveIdx)[hv] : 0;
		rcAboveRightMvHv[0][hv] = iAboveRightCU ? iAboveRightCU->getCUMvField(REF_PIC_0)->getMv(iAboveRightIdx)[hv] : 0;
		rcAboveLeftMvHv[0][hv] = iAboveLeftCU ? iAboveLeftCU->getCUMvField(REF_PIC_0)->getMv(iAboveLeftIdx)[hv] : 0;
		rcLeftCU1MvHv[0][hv] = pLeftCU1 ? pLeftCU1->getCUMvField(REF_PIC_0)->getMv(uiLeftIdx1)[hv] : 0;
		rcAboveCU1MvHv[0][hv] = pAboveCU1 ? pAboveCU1->getCUMvField(REF_PIC_0)->getMv(uiLeftIdx1)[hv] : 0;

		rcLeftMvHv[1][hv] = pLeftCU ? pLeftCU->getCUMvField(REF_PIC_1)->getMv(uiLeftIdx)[hv] : 0;
		rcAboveMvHv[1][hv] = pAboveCU ? pAboveCU->getCUMvField(REF_PIC_1)->getMv(uiAboveIdx)[hv] : 0;
		rcAboveRightMvHv[1][hv] = iAboveRightCU ? iAboveRightCU->getCUMvField(REF_PIC_1)->getMv(iAboveRightIdx)[hv] : 0;
		rcAboveLeftMvHv[1][hv] = iAboveLeftCU ? iAboveLeftCU->getCUMvField(REF_PIC_1)->getMv(iAboveLeftIdx)[hv] : 0;
		rcLeftCU1MvHv[1][hv] = pLeftCU1 ? pLeftCU1->getCUMvField(REF_PIC_1)->getMv(uiLeftIdx1)[hv] : 0;
		rcAboveCU1MvHv[1][hv] = pAboveCU1 ? pAboveCU1->getCUMvField(REF_PIC_1)->getMv(uiLeftIdx1)[hv] : 0;

		for (Int i = 0; i < 2; i++)
		{
			pmv[i][hv][0] = rcAboveLeftMvHv[i][hv];   //i:0-first;1-second, hv-x or y
			pmv[i][hv][1] = rcAboveMvHv[i][hv];
			pmv[i][hv][2] = rcLeftMvHv[i][hv];
			pmv[i][hv][3] = rcAboveRightMvHv[i][hv];
			pmv[i][hv][4] = rcAboveCU1MvHv[i][hv];
			pmv[i][hv][5] = rcLeftCU1MvHv[i][hv];
		}
	}
	modeInfo[0] = iAboveLeftCU ? iAboveLeftCU->getInterDir(iAboveLeftIdx) : -1;
	modeInfo[1] = pAboveCU ? pAboveCU->getInterDir(uiAboveIdx) : -1;
	modeInfo[2] = pLeftCU ? pLeftCU->getInterDir(uiLeftIdx) : -1;
	modeInfo[3] = iAboveRightCU ? iAboveRightCU->getInterDir(iAboveRightIdx) : -1;
	modeInfo[4] = pAboveCU1 ? pAboveCU1->getInterDir(uiAboveIdx1) : -1;
	modeInfo[5] = pLeftCU1 ? pLeftCU1->getInterDir(uiLeftIdx1) : -1;
	for (Int j = 0; j < 6; j++)
	{

		if (modeInfo[j] == INTER_BID)
		{
			m_cTempForwardBSkipMv[0][INTER_BID] = pmv[0][0][j];
			m_cTempForwardBSkipMv[1][INTER_BID] = pmv[0][1][j];
			m_cTempBackwardBSkipMv[0][INTER_BID] = pmv[1][0][j];
			m_cTempBackwardBSkipMv[1][INTER_BID] = pmv[1][1][j];
			bid_flag++;
			if (bid_flag == 1) {
				bid2 = j;
			}
		}
		else if (modeInfo[j] == INTER_SYM) {
			m_cTempForwardBSkipMv[0][INTER_SYM] = pmv[0][0][j];
			m_cTempForwardBSkipMv[1][INTER_SYM] = pmv[0][1][j];
			m_cTempBackwardBSkipMv[0][INTER_SYM] = pmv[1][0][j];
			m_cTempBackwardBSkipMv[1][INTER_SYM] = pmv[1][1][j];
			sym_flag++;
		}
		else if (modeInfo[j] == INTER_BACKWARD) {
			m_cTempBackwardBSkipMv[0][INTER_BACKWARD] = pmv[0][0][j];
			m_cTempBackwardBSkipMv[1][INTER_BACKWARD] = pmv[0][1][j];
			bw_flag++;
		}
		else if (modeInfo[j] == INTER_FORWARD) {
			m_cTempForwardBSkipMv[0][INTER_FORWARD] = pmv[1][0][j];
			m_cTempForwardBSkipMv[1][INTER_FORWARD] = pmv[1][1][j];
			fw_flag++;
		}
	}
	if (bid_flag == 0 && fw_flag != 0 && bw_flag != 0)
	{
		m_cTempBackwardBSkipMv[0][INTER_BID] = m_cTempBackwardBSkipMv[0][INTER_BACKWARD];
		m_cTempBackwardBSkipMv[1][INTER_BID] = m_cTempBackwardBSkipMv[1][INTER_BACKWARD];
		m_cTempForwardBSkipMv[0][INTER_BID] = m_cTempForwardBSkipMv[0][INTER_FORWARD];
		m_cTempForwardBSkipMv[1][INTER_BID] = m_cTempForwardBSkipMv[1][INTER_FORWARD];
	}

	if (sym_flag == 0 && bid_flag > 1)
	{
		m_cTempBackwardBSkipMv[0][INTER_SYM] = pmv[0][0][bid2];
		m_cTempBackwardBSkipMv[1][INTER_SYM] = pmv[0][1][bid2];
		m_cTempForwardBSkipMv[0][INTER_SYM] = pmv[1][0][bid2];
		m_cTempForwardBSkipMv[1][INTER_SYM] = pmv[1][1][bid2];
	}
	else if (sym_flag == 0 && bw_flag != 0)
	{
		m_cTempBackwardBSkipMv[0][INTER_SYM] = m_cTempBackwardBSkipMv[0][INTER_BACKWARD];
		m_cTempBackwardBSkipMv[1][INTER_SYM] = m_cTempBackwardBSkipMv[1][INTER_BACKWARD];
		m_cTempForwardBSkipMv[0][INTER_SYM] = -m_cTempForwardBSkipMv[0][INTER_BACKWARD];
		m_cTempForwardBSkipMv[1][INTER_SYM] = -m_cTempForwardBSkipMv[1][INTER_BACKWARD];
	}
	else if (sym_flag == 0 && fw_flag != 0)
	{
		m_cTempBackwardBSkipMv[0][INTER_SYM] = -m_cTempBackwardBSkipMv[0][INTER_FORWARD];
		m_cTempBackwardBSkipMv[1][INTER_SYM] = -m_cTempBackwardBSkipMv[1][INTER_FORWARD];
		m_cTempForwardBSkipMv[0][INTER_SYM] = m_cTempForwardBSkipMv[0][INTER_FORWARD];
		m_cTempForwardBSkipMv[1][INTER_SYM] = m_cTempForwardBSkipMv[1][INTER_FORWARD];
	}
	if (bw_flag == 0 && bid_flag > 1)
	{
		m_cTempBackwardBSkipMv[0][INTER_BACKWARD] = pmv[0][0][bid2];
		m_cTempBackwardBSkipMv[1][INTER_BACKWARD] = pmv[0][1][bid2];
	}
	else if (bw_flag == 0 && bid_flag != 0)
	{
		m_cTempBackwardBSkipMv[0][INTER_BACKWARD] = m_cTempBackwardBSkipMv[0][INTER_BID];
		m_cTempBackwardBSkipMv[1][INTER_BACKWARD] = m_cTempBackwardBSkipMv[1][INTER_BID];
	}
	if (fw_flag == 0 && bid_flag > 1)
	{
		m_cTempBackwardBSkipMv[0][INTER_FORWARD] = pmv[1][0][bid2];
		m_cTempBackwardBSkipMv[1][INTER_FORWARD] = pmv[1][1][bid2];
	}
	else if (fw_flag == 0 && bid_flag != 0)
	{
		m_cTempForwardBSkipMv[0][INTER_FORWARD] = m_cTempForwardBSkipMv[0][INTER_BID];
		m_cTempForwardBSkipMv[1][INTER_FORWARD] = m_cTempForwardBSkipMv[1][INTER_BID];
	}
	//m_cTempForwardBSkipMv 现在还没有地方写这个变量的条用函数

}

#endif


TComMvField TComDataCU::getMvFieldPred(UInt uiAbsPartIdx, RefPic eRefPic, Int iRef)
{
	TComDataCU* pLeftCU;
	TComDataCU* pAboveCU;
	TComDataCU* pCornerCU;

	// 自己定义的三个列表
	TComMv rcLeftMvHv;
	TComMv rcAboveMvHv;
	TComMv rcCornerMvHv;
	UInt uiHv;
#if B_ME_REVISE
	Int ref = eRefPic == REF_PIC_1 ? -1 : 0;
#else
	Int ref = 1; //实现B帧的时候需要用到这个值，从前面传过来！！！
#endif
	UInt uiMvPredType;
	Int iLeftRef, iAboveRef, iCornerRef;
	TComMv rcTmpMv;
	Int iLeft, iAbove, iCorner;
	Int iPredVec = 0;
	TComMv mv0;
	mv0.setZero();

	UInt        uiLeftIdx, uiAboveIdx, uiCornerIdx;
	//Bool        bLeft = false, bAbove = false, bCorner = false;
#if INTER612
	UInt partIdx = 0;

	//test
#if niu_mvpred_debug
	getPartIndexFromAddr(partIdx, m_uiAbsIdxInLCU, uiAbsPartIdx);
#else
	getPartIndexFromAddr(partIdx, uiAbsPartIdx);
#endif
	Int Pixel_X = m_puiBlockPosition[(UInt)m_pePartSize[uiAbsPartIdx] * 2 + partIdx][0] <<
		(m_pcPicture->getSPS()->getLog2MaxCUSize() - m_puhDepth[uiAbsPartIdx] - MIN_CU_SIZE_IN_BIT);
	Int Pixel_Y = m_puiBlockPosition[(UInt)m_pePartSize[uiAbsPartIdx] * 2 + partIdx][1] <<
		(m_pcPicture->getSPS()->getLog2MaxCUSize() - m_puhDepth[uiAbsPartIdx] - MIN_CU_SIZE_IN_BIT);
#endif

#if INTER611
  Int blockShape_X = m_puiBlockSize[(UInt)m_pePartSize[uiAbsPartIdx] * 2 + partIdx][0] <<
	  (m_pcPicture->getSPS()->getLog2MaxCUSize() - m_puhDepth[uiAbsPartIdx] - MIN_CU_SIZE_IN_BIT);
  Int blockShape_Y = m_puiBlockSize[(UInt)m_pePartSize[uiAbsPartIdx] * 2 + partIdx][1] <<
	  (m_pcPicture->getSPS()->getLog2MaxCUSize() - m_puhDepth[uiAbsPartIdx] - MIN_CU_SIZE_IN_BIT);
  blockShape_X = blockShape_X % MIN_BLOCK_SIZE == 0 ? blockShape_X : MIN_BLOCK_SIZE;
  blockShape_Y = blockShape_Y % MIN_BLOCK_SIZE == 0 ? blockShape_Y : MIN_BLOCK_SIZE;
  Int shift = blockShape_X / (1 << (getPicture()->getSPS()->getLog2MaxCUSize() - g_uiMaxCUDepth));

#endif
	pLeftCU = getPULeft(uiLeftIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);
	pAboveCU = getPUAbove(uiAboveIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);
#if INTER611
  pCornerCU = getPUAboveRight(uiCornerIdx, m_uiAbsIdxInLCU + uiAbsPartIdx, shift);
#else
  pCornerCU = getPUAboveRight(uiCornerIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);
#endif
	if (!pCornerCU) pCornerCU = getPUAboveLeft(uiCornerIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);

#if XY_MIN_PMV
	uiMvPredType = MVPRED_xy_MIN;
#else
	uiMvPredType = MVPRED_MEDIAN;
#endif
	/*
	// check availability
	if (pLeftCU && !pLeftCU->isIntra(uiLeftIdx))
	{
	bLeft = true;
	}
	if (pAboveCU && !pAboveCU->isIntra(uiAboveIdx))
	{
	bAbove = true;
	}
	if (pCornerCU && !pCornerCU->isIntra(uiCornerIdx))
	{
	bCorner = true;
	}
	*/
	iLeftRef = pLeftCU ? pLeftCU->getCUMvField(eRefPic)->getRefIdx(uiLeftIdx) : -1;
	iAboveRef = pAboveCU ? pAboveCU->getCUMvField(eRefPic)->getRefIdx(uiAboveIdx) : -1;
	iCornerRef = pCornerCU ? pCornerCU->getCUMvField(eRefPic)->getRefIdx(uiCornerIdx) : -1;

	if ((iLeftRef != -1) && (iAboveRef == -1) && (iCornerRef == -1))
	{
		//m_cMvPred[eRefPic].setMvField(pLeftCU->getCUMvField(eRefPic)->getMv(uiLeftIdx), pLeftCU->getCUMvField(eRefPic)->getRefIdx(uiLeftIdx));
		//rcLeftMvHv = pLeftCU->getCUMvField(eRefPic)->getMv(uiLeftIdx);
		uiMvPredType = MVPRED_L;
	}
	else if ((iAboveRef != -1) && (iLeftRef == -1) && (iCornerRef == -1))
	{
		//m_cMvPred[eRefPic].setMvField(pAboveCU->getCUMvField(eRefPic)->getMv(uiAboveIdx), pAboveCU->getCUMvField(eRefPic)->getRefIdx(uiAboveIdx));
		//rcAboveMvHv = pAboveCU->getCUMvField(eRefPic)->getMv(uiAboveIdx);
		uiMvPredType = MVPRED_U;
	}
	else if ((iCornerRef != -1) && (iLeftRef == -1) && (iAboveRef == -1))
	{
		//m_cMvPred[eRefPic].setMvField(pCornerCU->getCUMvField(eRefPic)->getMv(uiCornerIdx), pCornerCU->getCUMvField(eRefPic)->getRefIdx(uiCornerIdx));
		//rcCornerMvHv = pCornerCU->getCUMvField(eRefPic)->getMv(uiCornerIdx);
		uiMvPredType = MVPRED_UR;
	}
#if INTER612
	else if (blockShape_X < blockShape_Y)
	{
		if (Pixel_X == 0)
		{
			if (iLeftRef == iRef)
			{
				uiMvPredType = MVPRED_L;
			}
		}
		else
		{
			if (iCornerRef == iRef)
			{
				uiMvPredType = MVPRED_UR;
			}
		}
	}
	else if (blockShape_X > blockShape_Y)
	{
		if (Pixel_Y == 0)
		{
			if (iAboveRef == iRef)
			{
				uiMvPredType = MVPRED_U;
			}
		}
		else
		{
			if (iLeftRef == iRef)
			{
				uiMvPredType = MVPRED_L;
			}
		}
	}

#endif
	//else if ()
  
	//test
	rcLeftMvHv = pLeftCU ? pLeftCU->getCUMvField(eRefPic)->getMv(uiLeftIdx) : mv0;
	//rcLeftMvHv = mv0;
	rcAboveMvHv = pAboveCU ? pAboveCU->getCUMvField(eRefPic)->getMv(uiAboveIdx) : mv0;
	//rcAboveMvHv = mv0;
	rcCornerMvHv = pCornerCU ? pCornerCU->getCUMvField(eRefPic)->getMv(uiCornerIdx) : mv0;
	//rcCornerMvHv = mv0;
#if DEBUG_ME1
	printf("left_mv_x = %d\t left_mv_y= %d\t, right_mv_x = %d\t right_mv_y= %d\t, corner_mv_x = %d\t corner_mv_y= %d\t\n ",
		rcLeftMvHv.getHor(), rcLeftMvHv.getVer(), rcAboveMvHv.getHor(), rcAboveMvHv.getVer(), rcCornerMvHv.getHor(), rcCornerMvHv.getVer());
	//printf("pred_mv_x =%d\t pred_mv_y= %d\t\n", m_cMvPred[eRefPic].getMv().getHor(), m_cMvPred[eRefPic].getMv().getVer());
#endif
	// 下面是进行MV的 Scale/Clip 操作
	
	for (uiHv = 0; uiHv < 2; uiHv++)
	{
		// 下面针对3个MV做了3次 Scale 操作

		if (((iLeftRef == -1) && ((m_pcPic->getPicture()->getPictureType() == P_PICTURE) || (m_pcPic->getPicture()->getPictureType() == F_PICTURE)) && false) ||
			((iLeftRef == -1) && (m_pcPic->getPicture()->getPictureType() == P_PICTURE) && (m_pcPic->getPicture()->getPictureType() == BP_IMG_PICTURE) && false))
		{
			rcLeftMvHv[uiHv] = 0;
		}
		else
#if HALF_PIXEL_COMPENSATION_PMV
		{
#if MV_SCALE
			if (m_pcPic->getPicture()->getFieldSequence() && uiHv == 1 && (iLeftRef != -1))
			{
				rcLeftMvHv[uiHv] = scaleMotionVector_y1(rcLeftMvHv[uiHv], eRefPic, iRef, iLeftRef, ref);
			}
			else
			{
#if MV_CLIP
				rcLeftMvHv[uiHv] = scaleMotionVector(rcLeftMvHv[uiHv], eRefPic, iRef, iLeftRef, ref, 0);
#endif
			}
#endif
		}
#endif


		if (((iAboveRef == -1) && ((m_pcPic->getPicture()->getPictureType() == P_PICTURE) || (m_pcPic->getPicture()->getPictureType() == F_PICTURE)) && false) ||
			(!(iAboveRef == -1) && (m_pcPic->getPicture()->getPictureType() == P_PICTURE) && (m_pcPic->getPicture()->getPictureType() == BP_IMG_PICTURE) && false))//传参数bgenable
		{
			rcAboveMvHv[uiHv] = 0;
		}
		else
#if HALF_PIXEL_COMPENSATION_PMV
		{
			{
#if MV_SCALE
				if (m_pcPic->getPicture()->getFieldSequence() && uiHv == 1 && (iAboveRef != -1))
				{
					rcAboveMvHv[uiHv] = scaleMotionVector_y1(rcAboveMvHv[uiHv], eRefPic, iRef, iAboveRef, ref);
				}
				else
				{
#if MV_CLIP
					rcAboveMvHv[uiHv] = scaleMotionVector(rcAboveMvHv[uiHv], eRefPic, iRef, iAboveRef, ref, 0);
#endif
				}
#endif
			}
		}
#endif

		if (((iCornerRef == -1) && ((m_pcPic->getPicture()->getPictureType() == P_PICTURE) || (m_pcPic->getPicture()->getPictureType() == F_PICTURE)) && false) ||
			((iCornerRef == -1) && (m_pcPic->getPicture()->getPictureType() == P_PICTURE) && (m_pcPic->getPicture()->getPictureType() == BP_IMG_PICTURE) && false))
		{
			rcCornerMvHv[uiHv] = 0;
		}
		else
#if HALF_PIXEL_COMPENSATION_PMV
		{
#if MV_SCALE
			if (m_pcPic->getPicture()->getFieldSequence() && uiHv == 1 && (iCornerRef != -1))
			{
				rcCornerMvHv[uiHv] = scaleMotionVector_y1(rcCornerMvHv[uiHv], eRefPic, iRef, iCornerRef, ref);
			}
			else
			{
#if MV_CLIP
				rcCornerMvHv[uiHv] = scaleMotionVector(rcCornerMvHv[uiHv], eRefPic, iRef, iCornerRef, ref, 0);
#endif
			}
#endif
		}
#endif

		switch (uiMvPredType)
		{
		case MVPRED_xy_MIN:
			if (uiHv == 1)
			{
				if (((rcLeftMvHv[0] < 0) && (rcAboveMvHv[0] > 0) && (rcCornerMvHv[0] > 0)) ||
					((rcLeftMvHv[0] > 0) && (rcAboveMvHv[0] < 0) && (rcCornerMvHv[0] < 0)))
				{
					rcTmpMv[0] = (rcAboveMvHv[0] + rcCornerMvHv[0]) / 2;
				}
				else if (((rcAboveMvHv[0] < 0) && (rcLeftMvHv[0] > 0) && (rcCornerMvHv[0] > 0)) ||
					((rcAboveMvHv[0] > 0) && (rcLeftMvHv[0] < 0) && (rcCornerMvHv[0] < 0)))
				{
					rcTmpMv[0] = (rcLeftMvHv[0] + rcCornerMvHv[0]) / 2;
				}
				else if (((rcCornerMvHv[0] < 0) && (rcLeftMvHv[0] > 0) && (rcAboveMvHv[0] > 0)) ||
					((rcCornerMvHv[0] > 0) && (rcLeftMvHv[0] < 0) && (rcAboveMvHv[0] < 0)))
				{
					rcTmpMv[0] = (rcLeftMvHv[0] + rcAboveMvHv[0]) / 2;
				}
				else
				{
					iLeft = abs(rcLeftMvHv[0] - rcAboveMvHv[0]);
					iAbove = abs(rcAboveMvHv[0] - rcCornerMvHv[0]);
					iCorner = abs(rcCornerMvHv[0] - rcLeftMvHv[0]);

					iPredVec = min(iLeft, min(iAbove, iCorner));
					if (iPredVec == iLeft)
					{
						rcTmpMv[0] = (rcLeftMvHv[0] + rcAboveMvHv[0]) / 2;
					}
					else if (iPredVec == iAbove)
					{
						rcTmpMv[0] = (rcAboveMvHv[0] + rcCornerMvHv[0]) / 2;
					}
					else
					{
						rcTmpMv[0] = (rcLeftMvHv[0] + rcCornerMvHv[0]) / 2;
					}
				}

				// for y component
				if (((rcLeftMvHv[1] < 0) && (rcAboveMvHv[1] > 0) && (rcCornerMvHv[1] > 0)) ||
					((rcLeftMvHv[1] > 0) && (rcAboveMvHv[1] < 0) && (rcCornerMvHv[1] < 0)))
				{
					rcTmpMv[1] = (rcAboveMvHv[1] + rcCornerMvHv[1]) / 2;
				}
				else if (((rcAboveMvHv[1] < 0) && (rcLeftMvHv[1] > 0) && (rcCornerMvHv[1] > 0)) ||
					((rcAboveMvHv[1] > 0) && (rcLeftMvHv[1] < 0) && (rcCornerMvHv[1] < 0)))
				{
					rcTmpMv[1] = (rcLeftMvHv[1] + rcCornerMvHv[1]) / 2;
				}
				else if (((rcCornerMvHv[1] < 0) && (rcLeftMvHv[1] > 0) && (rcAboveMvHv[1] > 0)) ||
					((rcCornerMvHv[1] > 0) && (rcLeftMvHv[1] < 0) && (rcAboveMvHv[1] < 0)))
				{
					rcTmpMv[1] = (rcLeftMvHv[1] + rcAboveMvHv[1]) / 2;
				}
				else
				{
					iLeft = abs(rcLeftMvHv[1] - rcAboveMvHv[1]);
					iAbove = abs(rcAboveMvHv[1] - rcCornerMvHv[1]);
					iCorner = abs(rcCornerMvHv[1] - rcLeftMvHv[1]);

					iPredVec = min(iLeft, min(iAbove, iCorner));
					if (iPredVec == iLeft)
					{
						rcTmpMv[1] = (rcLeftMvHv[1] + rcAboveMvHv[1]) / 2;
					}
					else if (iPredVec == iAbove)
					{
						rcTmpMv[1] = (rcAboveMvHv[1] + rcCornerMvHv[1]) / 2;
					}
					else
					{
						rcTmpMv[1] = (rcLeftMvHv[1] + rcCornerMvHv[1]) / 2;
					}
				}
			}
			break;
		case MVPRED_L:
			iPredVec = rcLeftMvHv[uiHv];
			break;
		case MVPRED_U:
			iPredVec = rcAboveMvHv[uiHv];
			break;
		case MVPRED_UR:
			iPredVec = rcCornerMvHv[uiHv];
		default:
			break;
		}

		// 下面这一段就是用来判断上面的最后三个case
		// MVPRED_L && MVPRED_U && MVPRED_UR
		// 将结果写入pmv
#if XY_MIN_PMV
		if (uiMvPredType != MVPRED_xy_MIN)
#else
		if (uiMvPredType != MVPRED_MEDIAN)
#endif
		{
			rcTmpMv[uiHv] = iPredVec;
		}
	}
	//if (iLeftRef==iCornerRef||iLeftRef==iAboveRef)
#if DEBUG_MV
	rcTmpMv.set(0, 0);
#endif
#if wlq_FME && !wlq_FME_q
	rcTmpMv.set(0, 0);
#endif

#if wlq_FME_nomvp
	rcTmpMv.set(0, 0);
#endif

	m_cMvPred[eRefPic].setMvField(rcTmpMv, iLeftRef);

	return m_cMvPred[eRefPic];
}
#else
#ifdef RPS
TComMvField TComDataCU::getMvFieldPred(UInt uiAbsPartIdx, RefPic eRefPic)
{
  TComDataCU* pLeftCU;
  TComDataCU* pAboveCU;
  TComDataCU* pCornerCU;
  UInt        uiLeftIdx, uiAboveIdx, uiCornerIdx;
  Bool        bLeft = false, bAbove = false, bCorner = false;

  pLeftCU = getPULeft(uiLeftIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);
  pAboveCU = getPUAbove(uiAboveIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);
  pCornerCU = getPUAboveRight(uiCornerIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);
  if (!pCornerCU) pCornerCU = getPUAboveLeft(uiCornerIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);

  // check availability
  if (pLeftCU && !pLeftCU->isIntra(uiLeftIdx))
  {
    bLeft = true;
  }
  if (pAboveCU && !pAboveCU->isIntra(uiAboveIdx))
  {
    bAbove = true;
  }
  if (pCornerCU && !pCornerCU->isIntra(uiCornerIdx))
  {
    bCorner = true;
  }
  if (bLeft)
  {
    m_cMvPred[eRefPic].setMvField(pLeftCU->getCUMvField(eRefPic)->getMv(uiLeftIdx), pLeftCU->getCUMvField(eRefPic)->getRefIdx(uiLeftIdx));
  }
  else if (bAbove)
  {
    m_cMvPred[eRefPic].setMvField(pAboveCU->getCUMvField(eRefPic)->getMv(uiAboveIdx), pAboveCU->getCUMvField(eRefPic)->getRefIdx(uiAboveIdx));
  }
  else if (bCorner)
  {
    m_cMvPred[eRefPic].setMvField(pCornerCU->getCUMvField(eRefPic)->getMv(uiCornerIdx), pCornerCU->getCUMvField(eRefPic)->getRefIdx(uiCornerIdx));

  }
  else
  {
    TComMv  cZeroMv;
    m_cMvPred[eRefPic].setMvField(cZeroMv, NOT_VALID);
  }

  return m_cMvPred[eRefPic];
}
#else
TComMv TComDataCU::getMvPred(UInt uiAbsPartIdx, RefPic eRefPic)
{
  TComDataCU* pLeftCU;
  TComDataCU* pAboveCU;
  TComDataCU* pCornerCU;
  UInt        uiLeftIdx, uiAboveIdx, uiCornerIdx;
  Bool        bLeft = false, bAbove = false, bCorner = false;

  pLeftCU = getPULeft(uiLeftIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);
  pAboveCU = getPUAbove(uiAboveIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);
  pCornerCU = getPUAboveRight(uiCornerIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);
  if (!pCornerCU) pCornerCU = getPUAboveLeft(uiCornerIdx, m_uiAbsIdxInLCU + uiAbsPartIdx);

  // check availability
  if (pLeftCU && !pLeftCU->isIntra(uiLeftIdx))
  {
    bLeft = true;
  }
  if (pAboveCU && !pAboveCU->isIntra(uiAboveIdx))
  {
    bAbove = true;
  }
  if (pCornerCU && !pCornerCU->isIntra(uiCornerIdx))
  {
    bCorner = true;
  }

  if (bLeft)
  {
    m_cMvPred[eRefPic] = pLeftCU->getCUMvField(eRefPic)->getMv(uiLeftIdx);
  }
  else if (bAbove)
  {
    m_cMvPred[eRefPic] = pAboveCU->getCUMvField(eRefPic)->getMv(uiAboveIdx);
  }
  else if (bCorner)
  {
    m_cMvPred[eRefPic] = pCornerCU->getCUMvField(eRefPic)->getMv(uiCornerIdx);
  }
  else
  {
    m_cMvPred[eRefPic].setZero();
  }

  return m_cMvPred[eRefPic];
}
#endif
#endif

#if RD_PMVR
_inline int TComDataCU::pmvrSign(Int val)
{
	if (val > 0) {
		return 1;
	}
	else if (val < 0) {
		return -1;
	}
	else {
		return 0;
	}
}

Void TComDataCU::pmvrMvdDerivation(TComMv& Mvd, TComMv rcMv, TComMv tmvp)
{
	int ctr[2], mv[2], mvp[2], mvd[2];
	mv[0] = rcMv.getHor();
	mv[1] = rcMv.getVer();
	mvp[0] = tmvp.getHor();
	mvp[1] = tmvp.getVer();

	ctr[0] = (mvp[0] >> 1) << 1;
	ctr[1] = (mvp[1] >> 1) << 1;
	if (abs(mv[0] - ctr[0]) > TH) {
		mvd[0] = (mv[0] + ctr[0] + pmvrSign(mv[0] - ctr[0]) * TH) / 2 - mvp[0];
		mvd[1] = (mv[1] - ctr[1]) >> 1;
	}
	else if (abs(mv[1] - ctr[1]) > TH) {
		mvd[0] = (mv[0] - ctr[0]) >> 1;
		mvd[1] = (mv[1] + ctr[1] + pmvrSign(mv[1] - ctr[1]) * TH) / 2 - mvp[1];
	}
	else {
		mvd[0] = mv[0] - mvp[0];
		mvd[1] = mv[1] - mvp[1];
	}

	Mvd.set(mvd[0], mvd[1]);
}

Void TComDataCU::pmvrMvDerivation(TComMv& rcMv, TComMv Mvd, TComMv tmvp)
{
	int ctrd[2], mv[2], mvp[2], mvd[2];
	mv[0] = rcMv.getHor();
	mv[1] = rcMv.getVer();
	mvp[0] = tmvp.getHor();
	mvp[1] = tmvp.getVer();
	mvd[0] = Mvd.getHor();
	mvd[1] = Mvd.getVer();

	ctrd[0] = ((mvp[0] >> 1) << 1) - mvp[0];
	ctrd[1] = ((mvp[1] >> 1) << 1) - mvp[1];
	if (abs(mvd[0] - ctrd[0]) > TH)
	{
		mv[0] = mvp[0] + (mvd[0] << 1) - ctrd[0] - pmvrSign(mvd[0] - ctrd[0]) * TH;
		mv[1] = mvp[1] + (mvd[1] << 1) + ctrd[1];
	}
	else if (abs(mvd[1] - ctrd[1]) > TH)
	{
		mv[0] = mvp[0] + (mvd[0] << 1) + ctrd[0];
		mv[1] = mvp[1] + (mvd[1] << 1) - ctrd[1] - pmvrSign(mvd[1] - ctrd[1]) * TH;
	}
	else
	{
		mv[0] = mvd[0] + mvp[0];
		mv[1] = mvd[1] + mvp[1];
	}
	mv[0] = Clip3(-32768, 32767, mv[0]);
	mv[1] = Clip3(-32768, 32767, mv[1]);
	rcMv.setHor(mv[0]);
	rcMv.setVer(mv[1]);
}
#endif

Void TComDataCU::clearCbf( UInt uiIdx, TextType eType, UInt uiNumParts )
{
  ::memset( &m_puhCbf[g_aucConvertTxtTypeToIdx[eType]][uiIdx], 0, sizeof(UChar)*uiNumParts);
}

Void TComDataCU::setCuCbfLuma( UInt uiAbsPartIdx, UInt uiLumaTrMode, UInt uiPartDepth )
{
  UInt uiDepth = m_puhDepth[ uiAbsPartIdx ] + uiPartDepth;
  xCalcCuCbf( m_puhCbf[g_aucConvertTxtTypeToIdx[TEXT_LUMA]]     + uiAbsPartIdx, uiLumaTrMode,   0, uiDepth );
}

Void TComDataCU::setCuCbfChromaUV( UInt uiAbsPartIdx, UInt uiChromaTrMode, TextType eTxt,  UInt uiPartDepth )
{
  UInt uiDepth = m_puhDepth[ uiAbsPartIdx ] + uiPartDepth;
  xCalcCuCbf( m_puhCbf[g_aucConvertTxtTypeToIdx[eTxt]] + uiAbsPartIdx, uiChromaTrMode, 0, uiDepth );
}

Void TComDataCU::setCuCbfChroma( UInt uiAbsPartIdx, UInt uiChromaTrMode, UInt uiPartDepth )
{
  UInt uiDepth = m_puhDepth[ uiAbsPartIdx ] + uiPartDepth;
  xCalcCuCbf( m_puhCbf[g_aucConvertTxtTypeToIdx[TEXT_CHROMA_U]] + uiAbsPartIdx, uiChromaTrMode, 0, uiDepth );
  xCalcCuCbf( m_puhCbf[g_aucConvertTxtTypeToIdx[TEXT_CHROMA_V]] + uiAbsPartIdx, uiChromaTrMode, 0, uiDepth );
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

UInt TComDataCU::xGetMvdBits(TComMv cMvd)
{
  return ( xGetComponentBits(cMvd.getHor()) + xGetComponentBits(cMvd.getVer()) );
}

UInt TComDataCU::xGetComponentBits(Int iVal)
{
  UInt uiLength = 1;
  UInt uiTemp   = ( iVal <= 0) ? (-iVal<<1)+1: (iVal<<1);
  
  assert ( uiTemp );
  
  while ( 1 != uiTemp )
  {
    uiTemp >>= 1;
    uiLength += 2;
  }
  
  return uiLength;
}


Int TComDataCU::xGetDistScaleFactor(Int iCurrPOC, Int iCurrRefPOC, Int iColPOC, Int iColRefPOC)
{
  Int iDiffPocD = iColPOC - iColRefPOC;
  Int iDiffPocB = iCurrPOC - iCurrRefPOC;
  
  if( iDiffPocD == iDiffPocB )
  {
    return 1024;
  }
  else
  {
    Int iTDB      = Clip3( -128, 127, iDiffPocB );
    Int iTDD      = Clip3( -128, 127, iDiffPocD );
    Int iX        = (0x4000 + abs(iTDD/2)) / iTDD;
    Int iScale    = Clip3( -1024, 1023, (iTDB * iX + 32) >> 6 );
    return iScale;
  }
}

Void TComDataCU::xCalcCuCbf( UChar* puhCbf, UInt uiTrDepth, UInt uiCbfDepth, UInt uiCuDepth )
{
  if ( uiTrDepth == 0 )
    return;
  
  UInt ui, uiNumSig = 0;
  
  UInt uiNumPart  = m_pcPic->getNumPartInCU() >> ( uiCuDepth << 1 );
  UInt uiQNumPart = uiNumPart >> 2;
  
  UInt uiCbfDepth1 = uiCbfDepth + 1;
  if( uiNumPart == 1 )
  {
    if ( ( puhCbf[0] >> uiCbfDepth1 ) & 0x1 )
    {
      uiNumSig = 1;
    }
    puhCbf[0] |= uiNumSig << uiCbfDepth;
    
    return;
  }
  assert( uiQNumPart );
  
  if ( uiCbfDepth < ( uiTrDepth - 1 ) )
  {
    UChar* puhNextCbf = puhCbf;
    xCalcCuCbf( puhNextCbf, uiTrDepth, uiCbfDepth1, uiCuDepth+1 ); puhNextCbf += uiQNumPart;
    xCalcCuCbf( puhNextCbf, uiTrDepth, uiCbfDepth1, uiCuDepth+1 ); puhNextCbf += uiQNumPart;
    xCalcCuCbf( puhNextCbf, uiTrDepth, uiCbfDepth1, uiCuDepth+1 ); puhNextCbf += uiQNumPart;
    xCalcCuCbf( puhNextCbf, uiTrDepth, uiCbfDepth1, uiCuDepth+1 );
  }
  
  for ( ui = 0; ui < uiNumPart; ui += uiQNumPart )
  {
    if ( ( puhCbf[ui] >> uiCbfDepth1 ) & 0x1 )
    {
      uiNumSig = 1;
      break;
    }
  }
  
  uiNumSig <<= uiCbfDepth;
  for ( ui = 0; ui < uiNumPart; ui++ )
  {
    puhCbf[ui] |= uiNumSig;
  }
}

#if DPB
Void TComDataCU::compressMvAndRef()
{

	TComPicSym &picSym = *(getPic()->getPicSym());
	TComPicSym::DPBPerCtuData &dpbForCtu = picSym.getDPBPerCtuData(getAddr());

	for (UInt i = 0; i<NUM_REF_PIC_LIST_01; i++)
	{
		dpbForCtu.m_CUMvField[i].compress(m_acCUMvField[i], m_pcPic->getNumPartInWidth());
		dpbForCtu.m_pcPicture = getPicture();
	}
}
#endif