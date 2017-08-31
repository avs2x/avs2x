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

/** \file     TComPicture.cpp
    \brief    picture header and SPS class
*/

#include "CommonDef.h"
#include "TComPicture.h"
#include "TComPic.h"

TComPicture::TComPicture()
{
  m_iPOC                = 0;
  m_ePictureType          = I_PICTURE;
  m_iPictureQp            = 0;
  m_bLoopFilterDisable  = false;
  
  m_bDRBFlag            = true;
  
  m_iDepth              = 0;
  
  m_pcPic               = NULL;
  m_bRefenced           = false;
}

TComPicture::~TComPicture()
{
}


Void TComPicture::initPicture()
{
  m_bDRBFlag            = true;
}

Void  TComPicture::sortPicList        (TComList<TComPic*>& rcListPic)
{
  TComPic*    pcPicExtract;
  TComPic*    pcPicInsert;
  
  TComList<TComPic*>::iterator    iterPicExtract;
  TComList<TComPic*>::iterator    iterPicExtract_1;
  TComList<TComPic*>::iterator    iterPicInsert;
  
  for (Int i = 1; i < (Int)(rcListPic.size()); i++)
  {
    iterPicExtract = rcListPic.begin();
    for (Int j = 0; j < i; j++) iterPicExtract++;
    pcPicExtract = *(iterPicExtract);
    
    iterPicInsert = rcListPic.begin();
    while (iterPicInsert != iterPicExtract)
    {
      pcPicInsert = *(iterPicInsert);
      if (pcPicInsert->getPOC() >= pcPicExtract->getPOC())
      {
        break;
      }
      
      iterPicInsert++;
    }
    
    iterPicExtract_1 = iterPicExtract;    iterPicExtract_1++;
    
    //  swap iterPicExtract and iterPicInsert, iterPicExtract = curr. / iterPicInsert = insertion position
    rcListPic.insert (iterPicInsert, iterPicExtract, iterPicExtract_1);
    rcListPic.erase  (iterPicExtract);
  }
}

TComPic* TComPicture::xGetRefPic(TComList<TComPic*>& rcListPic,
                                Bool                bDRBFlag,
                                UInt                uiPOCCurr,
                                RefPic              eRefPic,
                                UInt                uiNthRefPic)
{
  //  find current position
  TComList<TComPic*>::iterator  iterPic = rcListPic.begin();
  TComPic*                      pcRefPic   = NULL;
  
  TComPic*                      pcPic = *(iterPic);
  while ( (pcPic->getPOC() != (Int)uiPOCCurr) && (iterPic != rcListPic.end()) )
  {
    iterPic++;
    pcPic = *(iterPic);
  }
  assert (pcPic->getPOC() == (Int)uiPOCCurr);
  
  //  find n-th reference picture with bDRBFlag and eERBIndex
  UInt  uiCount = 0;
  
  if( eRefPic == REF_PIC_0 )
  {
    while(1)
    {
      if (iterPic == rcListPic.begin())
        break;
      
      iterPic--;
      pcPic = *(iterPic);
      if( ( !pcPic->getReconMark()                        ) ||
         ( bDRBFlag  != pcPic->getPicture()->getDRBFlag()  ) )
        continue;
      
      if( !pcPic->getPicture()->isReferenced() )
        continue;
      
      uiCount++;
      if (uiCount == uiNthRefPic)
      {
        pcRefPic = pcPic;
        return  pcRefPic;
      }
    }
    
    if ( !m_pcSPS->getUseLDC() )
    {
      iterPic = rcListPic.begin();
      pcPic = *(iterPic);
      while ( (pcPic->getPOC() != (Int)uiPOCCurr) && (iterPic != rcListPic.end()) )
      {
        iterPic++;
        pcPic = *(iterPic);
      }
      assert (pcPic->getPOC() == (Int)uiPOCCurr);
      
      while(1)
      {
        iterPic++;
        if (iterPic == rcListPic.end())
          break;
        
        pcPic = *(iterPic);
        if( ( !pcPic->getReconMark()                        ) ||
           ( bDRBFlag  != pcPic->getPicture()->getDRBFlag()  ) )
          continue;
        
        if( !pcPic->getPicture()->isReferenced() )
          continue;
        
        uiCount++;
        if (uiCount == uiNthRefPic)
        {
          pcRefPic = pcPic;
          return  pcRefPic;
        }
      }
    }
  }
  else
  {
    while(1)
    {
      iterPic++;
      if (iterPic == rcListPic.end())
        break;
      
      pcPic = *(iterPic);
      if( ( !pcPic->getReconMark()                        ) ||
         ( bDRBFlag  != pcPic->getPicture()->getDRBFlag()  ) )
        continue;
      
      if( !pcPic->getPicture()->isReferenced() )
        continue;
      
      uiCount++;
      if (uiCount == uiNthRefPic)
      {
        pcRefPic = pcPic;
        return  pcRefPic;
      }
    }
    
    iterPic = rcListPic.begin();
    pcPic = *(iterPic);
    while ( (pcPic->getPOC() != (Int)uiPOCCurr) && (iterPic != rcListPic.end()) )
    {
      iterPic++;
      pcPic = *(iterPic);
    }
    assert (pcPic->getPOC() == (Int)uiPOCCurr);
    
    while(1)
    {
      if (iterPic == rcListPic.begin())
        break;
      
      iterPic--;
      pcPic = *(iterPic);
      if( ( !pcPic->getReconMark()                        ) ||
         ( bDRBFlag  != pcPic->getPicture()->getDRBFlag()  ) )
        continue;
      
      if( !pcPic->getPicture()->isReferenced() )
        continue;
      
      uiCount++;
      if (uiCount == uiNthRefPic)
      {
        pcRefPic = pcPic;
        return  pcRefPic;
      }
    }
  }
  
  return  pcRefPic;
}
#if !RPS
Void TComPicture::setRefPOCList       ()
{
  for (Int iDir = 0; iDir < 2; iDir++)
  {
    if( m_apcRefPic[iDir] != NULL )
      m_aiRefPOC[iDir] = m_apcRefPic[iDir]->getPOC();
  }
}

Void TComPicture::setRefPic       ( TComList<TComPic*>& rcListPic )
{
  if (m_ePictureType == I_PICTURE)
  {
    m_apcRefPic[0] = NULL;
    m_apcRefPic[1] = NULL;
    return;
  }

  sortPicList(rcListPic);
  
  TComPic*  pcRefPic = NULL;
  for (Int n = 0; n < 2; n++)
  {
    RefPic  eRefPic = (RefPic)n;
    
    UInt  uiOrderDRB  = 1;
    
    if ( m_ePictureType == P_PICTURE && eRefPic == REF_PIC_1 )
    {
      m_apcRefPic[eRefPic] = NULL;
      break;
    }
    
    //  First DRB
    pcRefPic = xGetRefPic(rcListPic, true, m_iPOC, eRefPic, uiOrderDRB);
    if (pcRefPic != NULL)
    {
      m_apcRefPic[eRefPic] = pcRefPic;
      
      pcRefPic->getPicYuvRec()->extendPicBorder();
      continue;
    }
  }
}
#endif
#if RPS
//通过读取参考集合中的POC，计算DeltaPOC,然后与cfg的参考列表中的deltapoc进行比较，来决定那些别用做参考，以及参考完成后，被移除的参考帧
Void TComReferencePictureSet::setRefDeltaPOC(Int bufferNum, Int RefDeltaPOC)
{
	m_iRefDeltaPOC[bufferNum] = RefDeltaPOC;
}

Int TComReferencePictureSet::getRefDeltaPOC(Int bufferNum) const
{
	return m_iRefDeltaPOC[bufferNum];
}
//移出管理
Void TComReferencePictureSet::setRemDeltaPOC(Int bufferNum, Int RemDeltaPOC)
{
	m_iRemDeltaPOC[bufferNum] = RemDeltaPOC;
}

Int TComReferencePictureSet::getRemDeltaPOC(Int bufferNum) const
{
	return m_iRemDeltaPOC[bufferNum];
}

//参考帧数
Void TComReferencePictureSet::setNumberOfRefPictures(Int numberOfRefPictures)
{
	m_iNumberOfRefPictures = numberOfRefPictures;
}

Int TComReferencePictureSet::getNumberOfRefPictures() const
{
	return m_iNumberOfRefPictures;
}
//前向参考帧数
Void TComReferencePictureSet::setNumberOfNegativeRefPictures(Int numberOfNegativeRefPictures)
{
	m_iNumberOfNegativeRefPictures = numberOfNegativeRefPictures;
}

Int TComReferencePictureSet::getNumberOfNegativeRefPictures() const
{
	return m_iNumberOfNegativeRefPictures;
}

//后向参考帧数
Void TComReferencePictureSet::setNumberOfPositiveRefPictures(Int numberOfPositiveRefPictures)
{
	m_iNumberOfPositiveRefPictures = numberOfPositiveRefPictures;
}

Int TComReferencePictureSet::getNumberOfPositiveRefPictures() const
{
	return m_iNumberOfPositiveRefPictures;
}


//移出 暂时没写前向移出
Void TComReferencePictureSet::setNumberOfRemPictures(Int numberOfRemPictures)
{
	m_iNumberOfRemPictures = numberOfRemPictures;
}

Int TComReferencePictureSet::getNumberOfRemPictures() const
{
	return m_iNumberOfRemPictures;
}
//参考
Int TComReferencePictureSet::getRefPOC(Int bufferNum) const
{
	return m_iRefPOC[bufferNum];
}
Void TComReferencePictureSet::setRefPOC(Int bufferNum, Int POC)
{
	m_iRefPOC[bufferNum] = POC;
}
//移出
Int TComReferencePictureSet::getRemPOC(Int bufferNum) const
{
	return m_iRemPOC[bufferNum];
}
Void TComReferencePictureSet::setRemPOC(Int bufferNum, Int POC)
{
	m_iRemPOC[bufferNum] = POC;
}
Void  TComSPS::createRPSList(Int numRPS)
{
	m_RPSList.destroy();
	m_RPSList.create(numRPS);
}

TComReferencePictureSet::TComReferencePictureSet()
	: m_iNumberOfRefPictures(0)
	, m_iNumberOfRemPictures(0)
	, m_iNumberOfNegativeRefPictures(0)
	, m_iNumberOfPositiveRefPictures(0)
	, m_bRefered(true)
{
	::memset(m_iRefDeltaPOC, 0, sizeof(m_iRefDeltaPOC));
	::memset(m_iRefPOC, 0, sizeof(m_iRefPOC));
	::memset(m_iRemDeltaPOC, 0, sizeof(m_iRemDeltaPOC));
	::memset(m_iRemPOC, 0, sizeof(m_iRemPOC));
}
TComReferencePictureSet::~TComReferencePictureSet()
{

}
#endif
// ------------------------------------------------------------------------------------------------
// Sequence parameter set (SPS)
// ------------------------------------------------------------------------------------------------

TComSPS::TComSPS()
{
  // Structure
  m_uiWidth           = 352;
  m_uiHeight          = 288;
  m_uiLog2MaxCUSize   = 6;
  m_uiMaxCUDepth      = 5;
  m_uiLog2MaxPhTrSize = 5;
  
  // Tool list
  m_bUseDQP       = false;


#if AVS3_EXTENSION_LWJ
  m_pcUserData = new TComUserData;
  m_pcExtSeqDisplay = new TComExtSeqDisplay;
  m_pcExtScalability = new TComExtScalability;
#endif
#if AVS3_EXTENSION_CYH
  m_pcExtCopyright = new TComExtCopyright;
  m_pcExtCameraPra = new TComExtCameraPra;
#endif
#if AVS3_EXTENSION_LYT
  m_pcExtMetadata = new TComExtMetadata;
#endif
#if LAMBDA_BUG
  m_iPictureCodingOrder = -1;
#endif

}

TComSPS::~TComSPS()
{
#if RPS
  m_RPSList.destroy();
#endif
}
#if !AVS3_DELETE_PPS_SYC
TComPPS::TComPPS()
{
}

TComPPS::~TComPPS()
{
}
#endif

#if AVS3_PIC_HEADER_ZL
Void TComPicHeader::initPicHeader()
{
}
TComPicHeader::TComPicHeader()
{
#if AVS3_EXTENSION_LWJ
  m_pcUserData = new TComUserData;
#endif
#if AVS3_EXTENSION_CYH
  m_pcExtCopyright = new TComExtCopyright;
  m_pcExtCameraPra = new TComExtCameraPra;
#endif
#if AVS3_EXTENSION_LYT
  m_pcExtPicDisplay = new TComExtPicDisplay;
  m_pcExtRoiPra = new TComExtRoiPra;
#endif
  m_uiIntraPicStartCode      = 0x1B3;
  m_uiInterPicStartCode      = 0x1B6;
  m_iBbvDelay                = 0xFFFF;

  m_uiPicCodingType          = 1;             ///< 确定imageType之后需要重新set

#if inter_direct_skip_bug1
  m_ucInterSkipmodeNumber = 1;
#endif

#if RPS
  m_iPOC = 0;
  m_ePictureType = I_PICTURE;
  m_LocalRPS;
  m_iRPSIdx = 0;
  m_pRPS = NULL;
  for (int i = 0; i < NUM_REF_PIC_LIST_01; i++)
  {
	  for (int j = 0; j < MAX_NUM_REF_PICS + 1; j++)
	  {
		  m_apcRefPicList[i][j + 1] = NULL;
		  m_aiRefPOCList[i][j + 1] = 0;
	
	  }
  }
  for (int n = 0; n < NUM_REF_PIC_LIST_01; n++)
  {
	  m_iNumRefIdx[n] = 0;
  }
#endif
  //scene picture
  m_bScenePredFlag           = false;
  m_bSceneRefEnableFlag      = false;

  m_bTimeCodeFlag            = false;         ///< 默认关掉
  m_uiTimeCode               = 0;

  //scene picture I frame
  m_bScenePicFlag            = false;
  m_bScenePicOutputFlag      = false;

  m_uiDecOrderIndex          = false;
  m_uiTemporalId             = 0;
  m_uiPicOutputDelay         = 0;

  m_bUseRcsFlag              = false;
  m_uiRcsIndex               = 0;

  m_uiBbvCheckTimes          = 0;

  m_bProgressiveFrame        = false;
  m_ePicStructure            = PS_SUCCESSIVELY;

  m_bTopFieldFirst           = false;
  m_bRepeatFirstField        = false;
  m_bTopFieldPicFlag         = false;

  m_bFixedPicQP              = false;
  m_uiPicQP                  = 0;

  m_bRADecodableFlag         = false;

  //loop filter
  m_bLoopFilterDisableFlag   = false;
  m_stLoopFilterSyntax.m_bLoppFilterParamFlag  = false;
  m_stLoopFilterSyntax.m_iAlphaCOffset         = 0;
  m_stLoopFilterSyntax.m_iBetaOffset           = 0;

  m_bChromaQuantParamDisableFlag               = false;
  m_uiChromaQuantParamDeltaCb                  = 0;
  m_uiChromaQuantParamDeltaCr                  = 0;

  //WeightQuant
  m_bPicWeightQuantEnableFlag                        = false;
  m_stWeightQuantSyntax.m_uiPicWeightQuantDataIndex  = 0;
  m_stWeightQuantSyntax.m_uiWeightQuantParamIndex    = 0;
  for( int i = 0; i < 6; i++ )
  {
    m_stWeightQuantSyntax.m_iWeightQuantParamDelta1[i] = 0;
    m_stWeightQuantSyntax.m_iWeightQuantParamDelta2[i] = 0;
  }
  //weight_quant_matrix()
  for(int iSizeId = 0; iSizeId < 2; iSizeId++)
  {
    UInt WQMSize = 1 << (iSizeId + 2);
    for(int i = 0; i < WQMSize; i++)
    {
      for(int j = 0; j < WQMSize; j++)
      {
        if(iSizeId == 0)
        {
          m_stWeightQuantSyntax.m_uiWeightQuantCoeff4x4[i][j] = 0;
        }
        else
        {
          m_stWeightQuantSyntax.m_uiWeightQuantCoeff8x8[i][j] = 0;
        }
      }
    }
  }
  //ALF
  for( int i = 0; i < 3; i++ )
  {
    m_bPicAlfEnableFlag[i] = false;
  }
  m_stAlfFilterSyntax.m_uiAlfFilterNumMinus1         = 0;
  for( int i = 0; i < 16; i++ )
  {
    m_stAlfFilterSyntax.m_uiAlfRegionDistance[i]     = 0;
    for( int j = 0; j < 9; j++ )
    {
      m_stAlfFilterSyntax.m_iAlfCoeffLuma[i][j]      = 0;
    }
  }
  for( UInt i = 0; i < 2; i++ )
  {
    for( int j = 0; j < 9; j++ )
    {
      m_stAlfFilterSyntax.m_iAlfCoeffChroma[i][j]    = 0;               
    }
  }
}

TComPicHeader::~TComPicHeader()
{
}
#endif
#if AVS3_EXTENSION_LWJ
TComUserData::TComUserData()
{
  m_uiUserDataStartCode	= 0x1B2;
  m_uiUserData			= 1;
}

TComUserData::~TComUserData()
{
}

TComExtSeqDisplay::TComExtSeqDisplay()
{
  m_uiExtId					= 2;	//位串‘0010’
  m_uiVideoFormat				= 1;	//PAL
  m_uiSampleRange				= 1;
  m_uiColourDescription		= 0;
  m_uiColourPrimaries			= 0;
  m_uiTransferCharacteristics = 0;
  m_uiMatrixCoeff				= 0;
  m_uiDisplayHorSize			= 1920;
  m_uiDisplayVerSize			= 1280;
  m_bTdModeFlag				= 0;
  m_uiTdPackingMode			= 0;
  m_bViewReverseFlag			= 0;
}

TComExtSeqDisplay::~TComExtSeqDisplay()
{
}

TComExtScalability::TComExtScalability()
{
  m_uiExtId						= 3;	//位串‘0011’
  m_uiNumOfTemporalLevelMinus1 = MAX_TEMPORAL_ID -1;
  for (int i = 0; i < MAX_TEMPORAL_ID; i++)
  {
    m_uiTemporalFrameRateCode[i]= 1;
    m_uiTemporalBitRateLower[i]	= 0;
    m_uiTemporalBitRateUpper[i] = 1;
  }
}

TComExtScalability::~TComExtScalability()
{
}
#endif
#if AVS3_EXTENSION_CYH
TComExtCopyright::TComExtCopyright()
{
  m_uiExtId = 4;  //位串‘0100’
  m_bCopyrightFlag = 0;
  m_uiCopyrightId = 0;
  m_bOriginalOrCopy = 0;
  m_uiCopyrightNum1 = 0;
  m_uiCopyrightNum2 = 0;
  m_uiCopyrightNum3 = 0;
}

TComExtCopyright::~TComExtCopyright()
{
}

TComExtCameraPra::TComExtCameraPra()
{
  m_uiExtId = 11;   //位串1011
  m_uiCameraId = 0;
  m_uiHeightOfImageDevice = 0;
  m_uiFocalLenth = 0;
  m_uiFNum = 0;
  m_uiVerAngleOfView = 0;
  m_uiCameraPosXUpper = 0;
  m_uiCameraPosXLower = 0;
  m_uiCameraPosYUpper = 0;
  m_uiCameraPosYLower = 0;
  m_uiCameraPosZUpper = 0;
  m_uiCameraPosZLower = 0;
  m_uiCameraDirX = 0;
  m_uiCameraDirY = 0;
  m_uiCameraDirZ = 0;
  m_uiImagePlaneVerX = 0;
  m_uiImagePlaneVerY = 0;
  m_uiImagePlaneVerZ = 0;
}

TComExtCameraPra::~TComExtCameraPra()
{
}
#endif
#if AVS3_EXTENSION_LYT

TComExtMetadata::TComExtMetadata()
{
  Int c;
  m_uiExtId = 10;
  for (c = 0; c < 3; c++)
  {
    m_uiDisplayPrimariesX[c] = 0;
    m_uiDisplayPrimariesY[c] = 0;
  }
  m_uiWhitePointX = 0;
  m_uiWhitePointY = 0;
  m_uiMaxDisplayMasteringLum = 1000;
  m_uiMinDisplayMasteringLum = 10;
  m_uiMaxContentLightLevel = 4000;
  m_uiMaxPicAverageLightLevel = 1000;
}

TComExtMetadata::~TComExtMetadata()
{
}

TComExtRoiPra::TComExtRoiPra()
{
  m_uiExtId = 12;
  m_uiCurrPicRoiNum = 0;
  m_uiPrevPicRoiNum = 0;
  m_uiRoiSkipRun = 0;
  m_puiSkipRoiMode = NULL;
  m_uiRoiAxisxDelta = 0;
  m_uiRoiAxisyDelta = 0;
  m_uiRoiWidthDelta = 0;
  m_uiRoiHeightDelta = 0;
  m_uiRoiAxisx = 0;
  m_uiRoiAxisy = 0;
  m_uiRoiWidth = 0;
  m_uiRoiHeight = 0;
  ROIInfo = NULL;
  PrevROIInfo = NULL;
}

TComExtRoiPra::~TComExtRoiPra()
{
}

TComExtPicDisplay::TComExtPicDisplay()
{
  m_uiExtId = 7;
  m_uiPicCentreHorOffset = 0;
  m_uiPicCentreVerOffset = 0;
}

TComExtPicDisplay::~TComExtPicDisplay()
{
}

#endif
#if BSKIP
Void TComPicHeader::setRefDistance()
{
	m_iTemporalReference = m_iPOC;
	m_iTemporalReferenceForwardDistance = m_iTemporalReference;
	//
	if (m_ePictureType != B_PICTURE)
	{
		for (Int i = 0; i < m_iNumRefIdx[REF_PIC_0]; i++)
		{
			m_apcRefPicList[REF_PIC_0][i]->getPicHeader()->setTemporalReferenceForwardDistance(m_aiRefPOCList[REF_PIC_0][i]);
		}
	}
	else
	{
		m_apcRefPicList[REF_PIC_0][0]->getPicHeader()->setTemporalReferenceForwardDistance(m_aiRefPOCList[REF_PIC_1][0]);
		m_apcRefPicList[REF_PIC_1][0]->getPicHeader()->setTemporalReferenceForwardDistance(m_aiRefPOCList[REF_PIC_0][0]);
	}
	//
	m_iTemporalReferenceNextP = (m_ePictureType == B_PICTURE) ? m_apcRefPicList[REF_PIC_0][0]->getPicHeader()->getTemporalReferenceForwardDistance()
		: m_iTemporalReference;
	m_iPictureDistance = m_iTemporalReference;
	if (m_ePictureType == B_PICTURE)
	{
		m_apcRefPicList[REF_PIC_0][0]->getPicHeader()->setTemporalReferenceForwardDistance(m_apcRefPicList[REF_PIC_1][0]->getPicHeader()->getTemporalReferenceForwardDistance());
	}
}
#endif

#if RPS
Void TComPicHeader::setRefPOCList()
{
  for (Int iDir = 0; iDir < 2; iDir++)
  {
    for (Int iNumRefIdx = 0; iNumRefIdx < m_iNumRefIdx[iDir]; iNumRefIdx++)

    {
      if (m_apcRefPicList[iDir][iNumRefIdx] != NULL)
        m_aiRefPOCList[iDir][iNumRefIdx] = m_apcRefPicList[iDir][iNumRefIdx]->getPOC();
    }
  }
#if REFLISTBUG
  if (this->getPictureType() == F_PICTURE)
  {
	  for (Int iNumRefIdx = 0; iNumRefIdx < m_iNumRefIdx[0]; iNumRefIdx++)  //F帧是不能有后向参考帧数的，用前向的来代替
	  {
		  if (m_apcRefPicList[1][iNumRefIdx] != NULL)
			  m_aiRefPOCList[1][iNumRefIdx] = m_apcRefPicList[1][iNumRefIdx]->getPOC();
	  }
  }
#endif
}

Void TComPicHeader::setRefPic(TComList<TComPic*>& rcListPic)
{
  //I-Picture Return  //将参考置为0
#if RPS
  if (m_ePictureType == I_PICTURE)
  {
    ::memset(m_apcRefPicList, 0, sizeof(m_apcRefPicList));
    ::memset(m_iNumRefIdx, 0, sizeof(m_iNumRefIdx));
    return;
  }
#else
  if (m_ePictureType == I_PICTURE)
  {
    m_apcRefPic[0] = NULL;
    m_apcRefPic[1] = NULL;
    return;
  }
#endif
  //LDP
#if INTERTEST
#if !del_rec_0
  FILE* pf = fopen("rec_0.yuv", "ab+");
  TComPic* temp = NULL;

  temp = xGetRefPicList(rcListPic, 0);
  TComPicYuv *a = temp->getPicYuvRec();

  Pel* bufY = a->getLumaAddr();
  Pel* bufCb = a->getCbAddr();
  Pel* bufCr = a->getCrAddr();
  Int HeightY = a->getHeight();
  Int HeightCb = a->getHeight() / 2;
  Int HeightCr = a->getHeight() / 2;
  Int WidthY = a->getWidth();
  Int WidthCb = a->getWidth() / 2;
  Int WidthCr = a->getWidth() / 2;
  Int YStride = a->getStride();
  Int CbStride = a->getCStride();
  Int CrStride = a->getCStride();
  Pel target;
    for (Int y = 0; y < HeightY; y++)
    {
      for (Int x = 0; x < WidthY; x++)
      {
        fread(&target, 1, 1, pf);
        bufY[x] =(unsigned char) target;
      }
      bufY += YStride;
    }
    for (Int y = 0; y < HeightCb; y++)
    {
      for (Int x = 0; x < WidthCb; x++)
      {
        fread(&target, 1, 1, pf);
        bufCb[x] = (unsigned char)target;
      }
     
      bufCb += CbStride;
    }

    for (Int y = 0; y < HeightCr; y++)
    {
      for (Int x = 0; x < WidthCr; x++)
      {
        fread(&target, 1, 1, pf);
        bufCr[x] = (unsigned char)target;
      }
      bufCr += CrStride;
    }
    fclose(pf);
  ////////////
#endif


#endif

	TComPic* pcRefPic = NULL;
	static const UInt MAX_NUM_NEGATIVE_PICTURES = 16;    // MAX_NUM_NEGATIVE_PICTURES 最多参考帧数 建议使用num=4
	TComPic*  RefPicSetStCurr0[MAX_NUM_NEGATIVE_PICTURES]; //LDP LIST0
	TComPic*  RefPicSetStCurr1[MAX_NUM_NEGATIVE_PICTURES]; //RA  LIST1
	// TComPic*  RefPicSetLtCurr[MAX_NUM_NEGATIVE_PICTURES];  //LONGTERM
	UInt NumPicStCurr0 = 0;
	UInt NumPicStCurr1 = 0;
	//UInt NumPicLtCurr = 0;
	for (int i = 0; i < m_pRPS->getNumberOfRefPictures(); i++)
	{
		pcRefPic = xGetRefPicList(rcListPic, m_pRPS->getRefDeltaPOC(i) + getPOC());
		if (m_pRPS->getRefDeltaPOC(i) < 0)
		{
			pcRefPic->getPicYuvRec()->extendPicBorder();
			RefPicSetStCurr0[NumPicStCurr0] = pcRefPic; //list0
			NumPicStCurr0++;
		}

		else if (m_pRPS->getRefDeltaPOC(i) > 0)   //RA
		{
			pcRefPic->getPicYuvRec()->extendPicBorder();
			RefPicSetStCurr1[NumPicStCurr1] = pcRefPic;  //list1
			NumPicStCurr1++;
		}
	}
	//longterm  暂时没有此功能

	// ref_pic_list_init
	TComPic*  rpsCurrList0[MAX_NUM_REF_PICS + 1];  //MAX_NUM_REF_PICS =7 认为一条链上的参考帧数不能超过7
	TComPic*  rpsCurrList1[MAX_NUM_REF_PICS + 1];
	Int numPicTotalCurr = NumPicStCurr0 + NumPicStCurr1;

	//参考帧入链；
	for (int reflist0Idx = 0; reflist0Idx < NumPicStCurr0; reflist0Idx++)
	{
		rpsCurrList0[reflist0Idx] = RefPicSetStCurr0[reflist0Idx];
	}

	for (int reflist1Idx = 0; reflist1Idx < NumPicStCurr1; reflist1Idx++)
	{
		rpsCurrList1[reflist1Idx] = RefPicSetStCurr1[reflist1Idx];   //RA
	}
	//RA
	for (Int rIdx = 0; rIdx < m_iNumRefIdx[REF_PIC_0]; rIdx++)  //前向参考帧入链
	{
		m_apcRefPicList[REF_PIC_0][rIdx] = rpsCurrList0[rIdx];
	}
	for (Int rIdx = 0; rIdx < m_iNumRefIdx[REF_PIC_1]; rIdx++)  //后向参考帧入链
	{
		m_apcRefPicList[REF_PIC_1][rIdx] = rpsCurrList1[rIdx];
	}
#if REFLISTBUG
	if (this->getPictureType() == F_PICTURE)
	{
		for (int reflist0Idx = 0; reflist0Idx < NumPicStCurr0; reflist0Idx++)
		{
			rpsCurrList1[reflist0Idx] = RefPicSetStCurr0[reflist0Idx];
		}
		for (Int rIdx = 0; rIdx < m_iNumRefIdx[REF_PIC_0]; rIdx++)  //F帧将前向的帧拷贝到后向去
		{
			m_apcRefPicList[REF_PIC_1][rIdx] = rpsCurrList1[rIdx];
		}
	}
#endif
 
}
TComPic* TComPicHeader::xGetRefPicList(TComList<TComPic*>& rcListPic, Int poc)
{
  TComList<TComPic*>::iterator  iterPic = rcListPic.begin();
  TComPic*                      pcPic = *(iterPic);
  while (iterPic != rcListPic.end())
  {
    if (pcPic->getPOC() == poc)
    {
      break;
    }
    iterPic++;
    pcPic = *(iterPic);
  }
  return  pcPic;
}
#if RPS
// remove
Void TComPicHeader::ApplyReferencePictureSet(TComList<TComPic*>& rcListPic, const TComReferencePictureSet *RPSList)
{
  TComPic* rpcPic;
  Int i, isReference;
  TComList<TComPic*>::iterator iterPic = rcListPic.begin();

  while (iterPic != rcListPic.end())
  {
    rpcPic = *(iterPic++);
    //rpcPic->getPicture()->setReferenced(false);
    //isReference = 0;
    //for (i = 0; i<RPSList->getNumberOfPositiveRefPictures() + RPSList->getNumberOfNegativeRefPictures(); i++)
    //{
    //  if (rpcPic->getPicSym()->getPicture()->getPOC() == this->getPOC() + RPSList->getRefDeltaPOC(i))
    //  {
    //    isReference = 1;
    //  }
    //}    
    //if (isReference == 1)
    //{
    //  rpcPic->getPicture()->setReferenced(true);  //  xGetNewPicBuffer  控制 移出  //HM中通过一个GOP的参考关系制定移除规则，不需要传码流
    //}

    //AVS3 中，通过移出管理，写入码流控制移出
    isReference = 1;
    for (i = 0; i<RPSList->getNumberOfRemPictures(); i++)
    {
      if (rpcPic->getPicSym()->getPicture()->getPOC() == this->getPOC() + RPSList->getRemDeltaPOC(i))
      {
        isReference = 0;
      }
    }

#if B_RPS
	if (rpcPic->getPicSym()->getPicture()->getPOC() == this->getPOC() && RPSList->getRefered() == false)//B
	{
		isReference = 0;
	}
#endif
#if B_RPS_BUG_819
	if (rpcPic->getPicSym()->getPicture()->getPOC() == this->getPOC())
	{
		rpcPic->getPicture()->setReferenced(RPSList->getRefered());
	}
#endif
    if (isReference == 0)
    {
      rpcPic->getPicture()->setReferenced(false);  //  xGetNewPicBuffer  控制 移出
    }
  }
}
#endif
#endif
#if IDR
Void TComPicHeader::ApplyIDR(TComList<TComPic*>& rcListPic, Bool bUseIDR, PictureType e)
{
	TComPic* rpcPic;
	TComList<TComPic*>::iterator iterPic = rcListPic.begin();
	Bool IsClearPicList = false;
	Int TempIPicturePOC;
	while (iterPic != rcListPic.end())
	{
		rpcPic = *(iterPic++);
		if (this->getPOC() > rpcPic->getPicSym()->getPicHeader()->getPOC() && rpcPic->getPicSym()->getPicHeader()->getPictureType() == I_PICTURE)
		{
			printf("###");
			IsClearPicList = true;
			TempIPicturePOC = rpcPic->getPicSym()->getPicHeader()->getPOC();
			break;       //  IDR 清空I帧之前的所有参考帧
		}
	}
	if (IsClearPicList)
	{
		TComList<TComPic*>::iterator iterPic = rcListPic.begin();
		while (iterPic != rcListPic.end())
		{
			rpcPic = *(iterPic++);
			if (rpcPic->getPicSym()->getPicHeader()->getPOC() < TempIPicturePOC)
			{
				rpcPic->getPicSym()->getPicture()->setReferenced(false);  //  IDR 清空I帧之前的所有参考帧
			}
		}
	}
	//Bool randomAccessIDREnable = false;
	//Int TempPOC;
	//if (e == I_PICTURE && bUseIDR && this->getIntraPeriod() <= 1)
	//{
	//  while (iterPic != rcListPic.end())
	//  {
	//    rpcPic = *(iterPic++);
	//    if (this->getPOC() != rpcPic->getPOC())
	//    {
	//      rpcPic->getPicture()->setReferenced(false);  //  IDR 清空I帧之前的所有参考帧
	//    }
	//  }
	//}
	//else if (bUseIDR && this->getIntraPeriod() > 1)
	//{
	//  while (iterPic != rcListPic.end())
	//  {
	//    rpcPic = *(iterPic++);
	//    if ((this->getPOC() - this->getGOPSize()) == rpcPic->getPOC() && rpcPic->getPicSym()->getPicHeader()->getPictureType() == I_PICTURE)
	//    {
	//      TempPOC = rpcPic->getPOC();
	//      randomAccessIDREnable = true;
	//    }
	//  }
	//  if (randomAccessIDREnable)
	//  {
	//    TComList<TComPic*>::iterator iterPic = rcListPic.begin();
	//    while (iterPic != rcListPic.end())
	//    {
	//      rpcPic = *(iterPic++);
	//      if (rpcPic->getPOC() < TempPOC)
	//      {
	//        rpcPic->getPicture()->setReferenced(false);  //  IDR 清空I帧之前的所有参考帧
	//      }
	//    }
	//  }
	//}

}
Bool TComPicHeader::checkThatAllRefPicsAreAvailable(TComList<TComPic*>& rcListPic, const TComReferencePictureSet *pReferencePictureSet)
{
	TComPic* rpcPic;
	Bool isAvailable = true;

	for (Int i = 0; i < pReferencePictureSet->getNumberOfRefPictures(); i++)
	{
		TComList<TComPic*>::iterator iterPic = rcListPic.begin();
		while (iterPic != rcListPic.end())
		{
			rpcPic = *(iterPic++);
			if (rpcPic->getPicSym()->getPicture()->getPOC() == (pReferencePictureSet->getRefDeltaPOC(i) + this->getPOC()) && !rpcPic->getPicture()->isReferenced())
			{
				isAvailable = false;
			}
		}
	}
	return isAvailable;
}
Void TComPicHeader::createExplicitReferencePictureSetFromReference(TComList<TComPic*>& rcListPic, const TComReferencePictureSet *pReferencePictureSet)
{
	TComPic* rpcPic;

	Int k = 0;
	Int nrOfNegativePictures = 0;
	Int nrOfPositivePictures = 0;
	TComReferencePictureSet* pAdjustRPS = this->getAdjustRPS();
	(*pAdjustRPS) = TComReferencePictureSet();
	for (Int i = 0; i < pReferencePictureSet->getNumberOfRefPictures(); i++)
	{
		TComList<TComPic*>::iterator iterPic = rcListPic.begin();
		while (iterPic != rcListPic.end())
		{
			rpcPic = *(iterPic++);
			if (rpcPic->getPicSym()->getPicture()->getPOC() == this->getPOC() + pReferencePictureSet->getRefDeltaPOC(i) && rpcPic->getPicture()->isReferenced())
			{
				pAdjustRPS->setRefDeltaPOC(k, pReferencePictureSet->getRefDeltaPOC(i));
				pAdjustRPS->setRefPOC(k, rpcPic->getPicSym()->getPicture()->getPOC());

				if (pAdjustRPS->getRefDeltaPOC(k) < 0)
				{
					nrOfNegativePictures++;
				}
				else
				{
					nrOfPositivePictures++;
				}
				k++;
			}
		}
	}
	pAdjustRPS->setNumberOfNegativeRefPictures(nrOfNegativePictures);
	pAdjustRPS->setNumberOfPositiveRefPictures(nrOfPositivePictures);
	pAdjustRPS->setNumberOfRefPictures(nrOfNegativePictures + nrOfPositivePictures);
	pAdjustRPS->setNumberOfRemPictures(pReferencePictureSet->getNumberOfRemPictures());
	for (Int j = 0; j < pAdjustRPS->getNumberOfRemPictures(); j++)
	{
		pAdjustRPS->setRemDeltaPOC(j, pReferencePictureSet->getRemDeltaPOC(j));
		pAdjustRPS->setRemPOC(j, pReferencePictureSet->getRemPOC(j));
	}
	this->setRPS(pAdjustRPS);
	this->setRPSidx(-1);


}
#endif
