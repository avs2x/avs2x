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

/** \file     TEncTop.cpp
    \brief    encoder class
*/

#include "../TLibCommon/CommonDef.h"
#include "TEncTop.h"

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncTop::TEncTop()
{
  m_iPOCLast          = -1;
  m_iNumPicRcvd       =  0;
  m_uiNumAllPicCoded  =  0;
  m_pppcRDSbacCoder   =  NULL;
}

TEncTop::~TEncTop()
{
}

Void TEncTop::create ()
{
  // initialize global variables
  initROM();
  
  // create processing unit classes
  m_cGOPEncoder.        create();
  m_cPictureEncoder.    create( getSourceWidth(), getSourceHeight(), g_uiLog2MaxCUSize, g_uiMaxCUDepth );
  m_cCuEncoder.         create( g_uiMaxCUDepth, g_uiLog2MaxCUSize );
  m_cLoopFilter.        create( g_uiMaxCUDepth );
  
  m_pppcRDSbacCoder = new TEncSbac** [g_uiMaxCUDepth+1];
    
  for ( Int iDepth = 0; iDepth < g_uiMaxCUDepth+1; iDepth++ )
  {
    m_pppcRDSbacCoder[iDepth] = new TEncSbac* [CI_NUM];
      
    for (Int iCIIdx = 0; iCIIdx < CI_NUM; iCIIdx ++ )
    {
      m_pppcRDSbacCoder[iDepth][iCIIdx] = new TEncSbac;
    }
  }
}

Void TEncTop::destroy ()
{
  // destroy processing unit classes
  m_cGOPEncoder.        destroy();
  m_cPictureEncoder.      destroy();
  m_cCuEncoder.         destroy();
  m_cLoopFilter.        destroy();
  
  // SBAC RD
  Int iDepth;
  for ( iDepth = 0; iDepth < g_uiMaxCUDepth+1; iDepth++ )
  {
    for (Int iCIIdx = 0; iCIIdx < CI_NUM; iCIIdx ++ )
    {
      delete m_pppcRDSbacCoder[iDepth][iCIIdx];
    }
  }
    
  for ( iDepth = 0; iDepth < g_uiMaxCUDepth+1; iDepth++ )
  {
    delete [] m_pppcRDSbacCoder[iDepth];
  }
    
  delete [] m_pppcRDSbacCoder;
  
  // destroy ROM
  destroyROM();
  
  return;
}

Void TEncTop::init()
{
  // initialize SPS
  xInitSPS();
#if RPS
  ///RPS初始化
  xInitRPS();
#endif

  // initialize processing unit classes
  m_cGOPEncoder.  init( this );
  m_cPictureEncoder.init( this );
  m_cCuEncoder.   init( this );

  // initialize DIF
//  m_cSearch.setChromaFilter();
  
  // initialize transform & quantization class
  m_cTrQuant.init( 1 << m_uiLog2MaxPhTrSize, m_bUseRDOQ, true );
  
  // initialize encoder search class
#if wlq_FME 
  Int FME = m_bFME ? 1 : 0;
  m_cSearch.init( this, &m_cTrQuant, m_iSearchRange, FME, 0, &m_cEntropyCoder, &m_cRdCost, getRDSbacCoder(), getRDGoOnSbacCoder() );
  m_cGOPEncoder.InitRdCostMvBits();
#else
  m_cSearch.init( this, &m_cTrQuant, m_iSearchRange, m_iFastSearch, 0, &m_cEntropyCoder, &m_cRdCost, getRDSbacCoder(), getRDGoOnSbacCoder() );
#endif
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncTop::deletePicBuffer()
{
  TComList<TComPic*>::iterator iterPic = m_cListPic.begin();
  Int iSize = Int( m_cListPic.size() );
  
  for ( Int i = 0; i < iSize; i++ )
  {
    TComPic* pcPic = *(iterPic++);
    
    pcPic->destroy();
    delete pcPic;
    pcPic = NULL;
  }
}

/**
 - Application has picture buffer list with size of GOP + 1
 - Picture buffer list acts like as ring buffer
 - End of the list has the latest picture
 .
 \param   bEos                true if end-of-sequence is reached
 \param   pcPicYuvOrg         original YUV picture
 \retval  rcListPicYuvRecOut  list of reconstruction YUV pictures
 \retval  rcListBitstreamOut  list of output bitstreams
 \retval  iNumEncoded         number of encoded pictures
 */
Void TEncTop::encode( bool bEos, TComPicYuv* pcPicYuvOrg, TComList<TComPicYuv*>& rcListPicYuvRecOut, TComList<TComBitstream*>& rcListBitstreamOut, Int& iNumEncoded )
{
  TComPic* pcPicCurr = NULL;
  
  // get original YUV
  xGetNewPicBuffer( pcPicCurr );
  //#if B_RPS
  //  int a[25];
  //  int i = 0;
  //  for (TComList<TComPic*>::iterator it = m_cListPic.begin(); it != m_cListPic.end(); it++)
  //  {
  //    a[i] = (*it)->getPOC();
  //    printf("%d\t", a[i]);
  //    i++;
  //  }
  //  printf("\n");
  //#endif
  pcPicYuvOrg->copyToPic( pcPicCurr->getPicYuvOrg() );
  
  if ( m_iPOCLast != 0 && ( m_iNumPicRcvd != m_iGOPSize && m_iGOPSize ) && !bEos )
  {
    iNumEncoded = 0;
    return;
  }
  
  // compress GOP
  m_cGOPEncoder.compressGOP( m_iPOCLast, m_iNumPicRcvd, m_cListPic, rcListPicYuvRecOut, rcListBitstreamOut );
  
  iNumEncoded         = m_iNumPicRcvd;
  m_iNumPicRcvd       = 0;
  m_uiNumAllPicCoded += iNumEncoded;
  
  if (bEos)
  {
    m_cGOPEncoder.printOutSummary (m_uiNumAllPicCoded);
  }
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

/**
 - Application has picture buffer list with size of GOP + 1
 - Picture buffer list acts like as ring buffer
 - End of the list has the latest picture
 .
 \retval rpcPic obtained picture buffer
 */
Void TEncTop::xGetNewPicBuffer ( TComPic*& rpcPic )
{
  TComPicture::sortPicList(m_cListPic);
  
#if RPS
  if (m_cListPic.size() >= (UInt)(m_iGOPSize + m_GOPList[0].m_iNumRefPics + 4))
  {
    TComList<TComPic*>::iterator iterPic = m_cListPic.begin();
    Int iSize = Int(m_cListPic.size());
    for (Int i = 0; i < iSize; i++)
    {
      rpcPic = *(iterPic++);
      if (rpcPic->getPicture()->isReferenced() == false)                      ///<遍历m_cListPic buffer 筛选出需要移出的重构图像 
      {
        break;
      }
    }
  }
  else
  {
    rpcPic = new TComPic;
    rpcPic->create(m_iSourceWidth, m_iSourceHeight, g_uiLog2MaxCUSize, g_uiMaxCUDepth, false); //creat 一个新的空间
    m_cListPic.pushBack(rpcPic);
  }
  rpcPic->setReconMark(false);

  m_iPOCLast++;
  m_iNumPicRcvd++;
  rpcPic->getPicture()->setPOC(m_iPOCLast);
  rpcPic->getPicture()->setReferenced(true);
  rpcPic->getPicYuvRec()->setBorderExtension(false);
#else
  if (m_cListPic.size() >= (UInt)(m_iGOPSize + 3))
  {
    rpcPic = m_cListPic.popFront();
  }
  else
  {
    rpcPic = new TComPic;
    rpcPic->create(m_iSourceWidth, m_iSourceHeight, g_uiLog2MaxCUSize, g_uiMaxCUDepth);
  }

  m_cListPic.pushBack(rpcPic);
  rpcPic->setReconMark(false);

  m_iPOCLast++;
  m_iNumPicRcvd++;

  rpcPic->getPicture()->setPOC(m_iPOCLast);

  // mark it should be extended
  rpcPic->getPicYuvRec()->setBorderExtension(false);
#endif
}

Void TEncTop::xInitSPS()
{
#if AVS3_SEQ_HEADER_SYC
  m_cSPS.setVideoSequenceStartCode(0x1B0);
  m_cSPS.setHorizontalSize(m_iSourceWidth);
  m_cSPS.setVerticalSize(m_iSourceHeight);
  m_cSPS.setSamplePrecision(m_uiInputSampleBitDepth == 8?1:2);
  m_cSPS.setEncodingPrecision(m_uiBitDepth == 8? 1:2);
  m_cSPS.setLCUSize(m_uiMaxCUSize);
  m_cSPS.setProfileId(Profile::Name(m_iProfileId));
  m_cSPS.setLevelId(Level::Name(m_iLevelId));
  m_cSPS.setDualHypothesisPredictionEnableFlag(m_bDHPEnable);
  m_cSPS.setProgressiveSequence(m_bProgressiveSequence);
  if (m_bProgressiveSequence)
  {
   m_cSPS.setFieldCodedSequence(PTS_FRAME_SEQUENCE);    ///<20170107 Cfg可能需要返工，When m_iProgressiveSequence != 1
  }
  m_cSPS.setTemporalIdEnableFlag(false);                 ///<20170107 Cfg可能需要返工,RD默认是false
  m_cSPS.setPmvrEnableFlag(m_bPMVREnable);
  m_cSPS.setNonsquareQuadtreeTansformEnableFlag(m_bNSQT);
  m_cSPS.setNonsquareIntraPredictionEnableFlag(m_bSDIP);
  m_cSPS.setSecondaryTransformEnableFlag(m_bSECTEnable);
  m_cSPS.setCrossSliceLoopfilterEnableFlag(m_bCrossSliceLoopFilter);
  m_cSPS.setSampleAdaptiveOffsetEnableFlag(m_bSAOEnable);
  m_cSPS.setAdaptiveLoopFilterEnableFlag(m_bALFEnable);
  m_cSPS.setAspectRatio(1);                    ///<20170107 Cfg可能需要返工
  m_cSPS.setFrameRateCode(m_iFrameRate);
  m_cSPS.setBitRateLower(0);                   
  m_cSPS.setBitRateUpper(0);  
#if RPS_BUG
  m_cSPS.setGOPSize(m_iGOPSize);
#endif
#if INTER614
  m_cSPS.setNumberReferenceFrames(m_iNumberReferenceFrames);
#endif

  switch (m_cSPS.getLevelId()) 
  {
  case 0x10:
    m_cSPS.setBitRateLower(1875);
    m_cSPS.setBbvBufferSize(1507328);
    break;
  case 0x12:
    m_cSPS.setBitRateLower(2500);
    m_cSPS.setBbvBufferSize(2014232);
    break;
  case 0x14:
    m_cSPS.setBitRateLower(3125);
    m_cSPS.setBbvBufferSize(2506752);
    break;
  case 0x20:
    m_cSPS.setBitRateLower(12500);
    m_cSPS.setBbvBufferSize(6012928);
    break;
  case 0x22:
    m_cSPS.setBitRateLower(25000);
    m_cSPS.setBbvBufferSize(10010624);
    break;
  case 0x40:
    m_cSPS.setBitRateLower(31250);
    m_cSPS.setBbvBufferSize(12009472);
    break;
  case 0x42:
    m_cSPS.setBitRateLower(125000);
    m_cSPS.setBbvBufferSize(30015488);
    break;
  case 0x44:
    m_cSPS.setBitRateLower(50000);
    m_cSPS.setBbvBufferSize(20004864);
    break;
  case 0x46:
    m_cSPS.setBitRateLower(200000);
    m_cSPS.setBbvBufferSize(50003968);
    break;
  case 0x48:
    m_cSPS.setBitRateLower(75000);
    m_cSPS.setBbvBufferSize(25001984);
    break;
  case 0x4A:
    m_cSPS.setBitRateLower(37856);
    m_cSPS.setBitRateUpper(1); 
    m_cSPS.setBbvBufferSize(100007936);
    break;
  case 0x50:
    m_cSPS.setBitRateLower(50000);
    m_cSPS.setBbvBufferSize(25001984);
    break;
  case 0x52:
    m_cSPS.setBitRateLower(200000);
    m_cSPS.setBbvBufferSize(100007936);
    break;
  case 0x54:
    m_cSPS.setBitRateLower(75000);
    m_cSPS.setBbvBufferSize(40009728);
    break;
  case 0x56:
    m_cSPS.setBitRateLower(37856);
    m_cSPS.setBitRateUpper(1);
    m_cSPS.setBbvBufferSize(160006144);
    break;
  case 0x58:
    m_cSPS.setBitRateLower(150000);
    m_cSPS.setBbvBufferSize(60014592);
    break;
  case 0x5A:
    m_cSPS.setBitRateLower(75712);
    m_cSPS.setBitRateUpper(2);
    m_cSPS.setBbvBufferSize(240009216);
    break;
  case 0x60:
    m_cSPS.setBitRateLower(75000);
    m_cSPS.setBbvBufferSize(60014592);
    break;
  case 0x62:
    m_cSPS.setBitRateLower(37856);
    m_cSPS.setBitRateUpper(1);
    m_cSPS.setBbvBufferSize(240009216);
    break;
  case 0x64:
    m_cSPS.setBitRateLower(150000);
    m_cSPS.setBbvBufferSize(120012800);
    break;
  case 0x66:
    m_cSPS.setBitRateLower(75712);
    m_cSPS.setBitRateUpper(2);
    m_cSPS.setBbvBufferSize(480002048);
    break;
  case 0x68:
    m_cSPS.setBitRateLower(37856);
    m_cSPS.setBitRateUpper(1);
    m_cSPS.setBbvBufferSize(240009216);
    break;
  case 0x6A:
    m_cSPS.setBitRateLower(213568);
    m_cSPS.setBitRateUpper(3);
    m_cSPS.setBbvBufferSize(800014336);
    break;
  default:
    m_cSPS.setBitRateLower(1);
  }

  if (m_iNumberBFrames == 0) 
  {
    m_cSPS.setLowDelay(true);
  } 
  else 
  {
    m_cSPS.setLowDelay(false);
  }
  if(m_cSPS.getLowDelay())
  {
    m_cSPS.setOutputReorderDelay(0);
  }
  else
  {
    ///<20170107参考帧管理
  }
  /*<20170107参考帧管理
 m_cSPS.setNumOfRcs                                 ( b );
 m_cSPS.setReferedByOtherFlag                       ( Idx, b );
 m_cSPS.setNumOfReferencePicture                    ( Idx, ui );
 m_cSPS.setDeltaDoiOfReferencePicture               ( Idx1, Idx2, ui );
 m_cSPS.setNumOfRemovedPicture                      ( Idx, ui );
 m_cSPS.setDeltaDoiOfRemovedPicture                 ( Idx1, Idx2, ui );
  */
 m_cSPS.setScenePictureDisableFlag(m_bBackgroundEnable? false:true);
 m_cSPS.setChromaFormat(ChromaFormat (m_iChromaFormat));
 m_cSPS.setWeightQuantEnableFlag(m_bWQEnable);
  
 m_cSPS.setMultiHypothesisSkipEnableFlag(m_bMHPSkipEnable);
 m_cSPS.setWeightedSkipEnableFlag(m_bWSMEnable);
 m_cSPS.setAsymmetricMotionPartitionsEnableFlag(m_bInterSearchAMP);
#if rd_sym
 m_cSPS.setBackgroundEnableFlag(m_bBackgroundEnable);
#endif
  if (m_bWQEnable) 
  {
   m_cSPS.setbLoadSeqWeightQuantDataFlag(m_bSeqWQM);
    if (m_bSeqWQM) 
    {
      Int x, y, sizeId, uiWqMSize;
      for (sizeId = 0; sizeId < 2; sizeId++) 
      {
        uiWqMSize = min(1 << (sizeId + 2), 8);
        for (y = 0; y < uiWqMSize; y++) 
        {
          for (x = 0; x < uiWqMSize; x++) 
          {
            if(sizeId == 0)
            {
             m_cSPS.setWeightQuantCoeff4x4(y,x,0);///<20170107 Cfg需要返工,4x4数据 m_uiWeightQuantCoeff
            }
            else
            {
             m_cSPS.setWeightQuantCoeff8x8(y,x,0);///<20170107 Cfg需要返工,8x8数据 m_uiWeightQuantCoeff
            }
          }
        }
      }
    }
  }


#endif
  m_cSPS.setWidth           ( m_iSourceWidth      );
  m_cSPS.setHeight          ( m_iSourceHeight     );
  m_cSPS.setPad             ( m_aiPad             );
  m_cSPS.setLog2MaxCUSize   ( g_uiLog2MaxCUSize   );
  m_cSPS.setMaxCUDepth      ( g_uiMaxCUDepth      );
  m_cSPS.setLog2MaxPhTrSize ( m_uiLog2MaxPhTrSize );
  
  m_cSPS.setUseDQP          ( m_iMaxDeltaQP != 0  );
  m_cSPS.setUseLDC          ( m_bUseLDC           );
  m_cSPS.setUsePAD          ( m_bUsePAD           );
  
  m_cSPS.setBitDepth        ( g_uiBitDepth        );
}

#if RPS
Void TEncTop::xInitRPS() ///<实现功能 RD代码 prepare_RefInfo部分
{
	TComReferencePictureSet*      rps;
	Int TempNegativeRefNumber = 0;
	Int TempPositiveRefNumber = 0;
	m_cSPS.createRPSList(getGOPSize() + m_iExtraRPSs);
	TComRPSList* rpsList = m_cSPS.getRPSList();
	for (Int i = 0; i < getGOPSize() + m_iExtraRPSs; i++)
	{
		TempNegativeRefNumber = 0;
		TempPositiveRefNumber = 0;
		GOPEntry ge = getGOPEntry(i);
		rps = rpsList->getReferencePictureSet(i);
		rps->setNumberOfRefPictures(ge.m_iNumRefPics);
		rps->setNumberOfRemPictures(ge.m_iNumRemPics);
		rps->setRefered(ge.m_bRefered);
		for (int j = 0; j < ge.m_iNumRefPics; j++)
		{
			rps->setRefDeltaPOC(j, ge.m_iReferencePics[j]);
			if (ge.m_iReferencePics[j] > 0)
			{
				TempPositiveRefNumber++;
			}
			else if (ge.m_iReferencePics[j] < 0)
			{
				TempNegativeRefNumber++;
			}
		}
		rps->setNumberOfNegativeRefPictures(TempNegativeRefNumber);
		rps->setNumberOfPositiveRefPictures(TempPositiveRefNumber);
		for (int m = 0; m < ge.m_iNumRemPics; m++)
		{
			rps->setRemDeltaPOC(m, ge.m_iRemovePics[m]);
		}
		rps->setNumberOfRefPictures(ge.m_iNumRefPics);
		rps->setNumberOfRemPictures(ge.m_iNumRemPics);
	}
}

#if B_RPS_BUG
Void TEncTop::selectReferencePictureSet(TComPicHeader* pcPicHeader, Int POCCurr, Int GOPid, Int POCLast)
#else
Void TEncTop::selectReferencePictureSet(TComPicHeader* pcPicHeader, Int POCCurr, Int GOPid)
#endif
{
	pcPicHeader->setRPSidx(GOPid);  //定位RPS链表位置  GOPid = 0～3 RPS链表长度 m_iExtraRPSs + m_iGOPSize
	//扩展GOPLIST，直到参考管理稳定 m_iExtraRPSs=10
	for (Int extraNum = m_iGOPSize; extraNum < m_iExtraRPSs + m_iGOPSize; extraNum++)
	{
#if B_RPS_BUG
		if (POCCurr == m_GOPList[extraNum].m_iPOC && !m_cGOPEncoder.getSeqEnd())
#else
		if (POCCurr == m_GOPList[extraNum].m_iPOC)
#endif

		{
			pcPicHeader->setRPSidx(extraNum);
			break;
		}
	}
	const  TComReferencePictureSet *rps = (pcPicHeader->getSeqHeader()->getRPSList()->getReferencePictureSet(pcPicHeader->getRPSidx()));
	pcPicHeader->setRPS(rps);

#if RPS_BUG_815
	Int swapFlag1 = 0;
	Int swapFlag2 = 0;
#if B_RPS_BUG_819
	if (!getNumberBFrames())
	{
#endif
		for (Int reflist0Idx = 1; reflist0Idx < rps->getNumberOfNegativeRefPictures(); reflist0Idx++)
		{
			for (Int i = 1; i <= rps->getNumberOfNegativeRefPictures() - reflist0Idx - 1; i++)
			{
				if ((pcPicHeader->getPOC() + rps->getRefDeltaPOC(i)) % getGOPSize() != 0 &&
					((pcPicHeader->getPOC() + rps->getRefDeltaPOC(i + 1)) % getGOPSize() == 0 && (i + 1) < rps->getNumberOfNegativeRefPictures()))
				{
					swapFlag1 = 1;
				}
			}
		}//符合RD代码的参考关系判定
		Int KeyFrameNum = 0;
		for (Int i = 0; i < rps->getNumberOfNegativeRefPictures(); i++)
		{
			if ((pcPicHeader->getPOC() + rps->getRefDeltaPOC(i)) % pcPicHeader->getGOPSize() == 0)
			{
				KeyFrameNum++;
			}
		}
		if (KeyFrameNum == getGOPSize())
		{
			swapFlag2 = 1;
		}
#if  B_RPS_BUG_819
	}
#endif
	if (swapFlag1 || swapFlag2)
	{
		pcPicHeader->setRPSidx(-1);
		TComReferencePictureSet* pLocalRPS = pcPicHeader->getLocalRPS();
		pLocalRPS->setNumberOfNegativeRefPictures(rps->getNumberOfNegativeRefPictures());
		pLocalRPS->setNumberOfPositiveRefPictures(rps->getNumberOfPositiveRefPictures());
		pLocalRPS->setNumberOfRefPictures(rps->getNumberOfRefPictures());
		pLocalRPS->setNumberOfRemPictures(rps->getNumberOfRemPictures());
		pLocalRPS->setRefered(rps->getRefered());
		for (Int i = 0; i < pLocalRPS->getNumberOfRefPictures(); i++)
		{
			pLocalRPS->setRefDeltaPOC(i, rps->getRefDeltaPOC(i));
		}
		for (Int i = 0; i < pLocalRPS->getNumberOfRefPictures(); i++)//暂时没用到
		{
			pLocalRPS->setRefPOC(i, rps->getRefPOC(i));
		}
		for (Int i = 0; i < pLocalRPS->getNumberOfRemPictures(); i++)
		{
			pLocalRPS->setRemDeltaPOC(i, rps->getRemDeltaPOC(i));
		}
		for (Int i = 0; i < pLocalRPS->getNumberOfRemPictures(); i++)
		{
			pLocalRPS->setRemPOC(i, rps->getRemPOC(i));
		}
		if (swapFlag1)
		{
			Int TempDeltaPOC;
			for (Int reflist0Idx = 1; reflist0Idx < pLocalRPS->getNumberOfNegativeRefPictures(); reflist0Idx++)
			{
				for (Int i = 1; i <= pLocalRPS->getNumberOfNegativeRefPictures() - reflist0Idx - 1; i++)
				{
					if ((pcPicHeader->getPOC() + pLocalRPS->getRefDeltaPOC(i)) % getGOPSize() != 0 &&
						((pcPicHeader->getPOC() + pLocalRPS->getRefDeltaPOC(i + 1)) % getGOPSize() == 0 && (i + 1) < pLocalRPS->getNumberOfNegativeRefPictures()))
					{
						TempDeltaPOC = pLocalRPS->getRefDeltaPOC(i + 1);
						pLocalRPS->setRefDeltaPOC(i + 1, pLocalRPS->getRefDeltaPOC(i));
						pLocalRPS->setRefDeltaPOC(i, TempDeltaPOC);
					}
				}
			}//符合RD代码的参考关系判定
#if BUG_815 
			if (pcPicHeader->getPOC() == 4 && pLocalRPS->getNumberOfNegativeRefPictures() == getGOPSize())
			{
				pLocalRPS->setNumberOfNegativeRefPictures(pLocalRPS->getNumberOfNegativeRefPictures() - 1);
				pLocalRPS->setNumberOfRefPictures(pLocalRPS->getNumberOfRefPictures() - 1);
			}
#endif
			if (pcPicHeader->getPOC() == 6)
			{
				pLocalRPS->setRefDeltaPOC(getGOPSize() - 1, -4);  //针对RD代码中参考管理的特殊处理  
				pLocalRPS->setRefPOC(getGOPSize() - 1, pcPicHeader->getPOC() - 4);
#if BUG_815 
				pLocalRPS->setRemDeltaPOC(pLocalRPS->getNumberOfRemPictures() - 1, -3);
#endif
			}
#if BUG_815 
			if (pcPicHeader->getPOC() == 7)
			{
				pLocalRPS->setRemDeltaPOC(pLocalRPS->getNumberOfRemPictures() - 1, -5);
			}
#endif

#if RPS_BUG_816
			if (pcPicHeader->getPOC() == 4)
			{
				pLocalRPS->setNumberOfNegativeRefPictures(rps->getNumberOfNegativeRefPictures() - 1);
			}
#endif
		}
		else if (swapFlag2)
		{
			pLocalRPS->setRefDeltaPOC(getGOPSize() - 1, -2);  //针对RD代码中参考管理的特殊处理
			pLocalRPS->setRefPOC(getGOPSize() - 1, pcPicHeader->getPOC() - 2);
		}
		pcPicHeader->setRPS(pLocalRPS);
	}
#endif

#if B_RPS_BUG
	if (m_cGOPEncoder.getSeqEnd())//RA结构下存在末尾特殊情况，
	{
		//m_uiSeqEndGOPPicNum
#if RPS_BUG_815
		pcPicHeader->setRPSidx(-1);
#endif
		TComReferencePictureSet* pLocalRPS = pcPicHeader->getLocalRPS();
		pLocalRPS->setNumberOfNegativeRefPictures(rps->getNumberOfNegativeRefPictures());
		pLocalRPS->setNumberOfPositiveRefPictures(rps->getNumberOfPositiveRefPictures());
		pLocalRPS->setNumberOfRefPictures(rps->getNumberOfRefPictures());
		pLocalRPS->setNumberOfRemPictures(rps->getNumberOfRemPictures());
		pLocalRPS->setRefered(rps->getRefered());
		if (GOPid == 0 && POCCurr <= getGOPSize())
		{
			pLocalRPS->setNumberOfRefPictures(1);
			pLocalRPS->setNumberOfNegativeRefPictures(1);
			pLocalRPS->setNumberOfRemPictures(0);
		}
		for (Int i = 0; i < pLocalRPS->getNumberOfRefPictures(); i++)
		{
			pLocalRPS->setRefDeltaPOC(i, rps->getRefDeltaPOC(i));
		}
		for (Int i = 0; i < pLocalRPS->getNumberOfRefPictures(); i++)//暂时没用到
		{
			pLocalRPS->setRefPOC(i, rps->getRefPOC(i));
		}

		for (Int i = 0; i < pLocalRPS->getNumberOfRemPictures(); i++)
		{
			pLocalRPS->setRemDeltaPOC(i, rps->getRemDeltaPOC(i));
		}
		for (Int i = 0; i < pLocalRPS->getNumberOfRemPictures(); i++)
		{
			pLocalRPS->setRemPOC(i, rps->getRemPOC(i));
		}
		if (GOPid == 0)//F_PICTURE P_PICTURE
		{
			for (Int i = 0; i < pLocalRPS->getNumberOfNegativeRefPictures(); i++)
			{
				pLocalRPS->setRefDeltaPOC(i, rps->getRefDeltaPOC(i) + (getGOPSize() - m_cGOPEncoder.getSeqEndGOPPicNum()));
			}
		}
		else//B_PICTURE
		{
			for (Int i = rps->getNumberOfNegativeRefPictures(); i < rps->getNumberOfRefPictures(); i++)
			{
				if (POCCurr + pLocalRPS->getRefDeltaPOC(i) > POCLast)
				{
					pLocalRPS->setRefDeltaPOC(i, POCLast - POCCurr);
				}
			}
		}
		pcPicHeader->setRPS(pLocalRPS);
	}


#endif
}
#endif
#if RPS_BUG_816

#endif