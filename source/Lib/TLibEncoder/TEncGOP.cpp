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

/** \file     TEncGOP.cpp
    \brief    GOP encoder class
*/

#include "TEncTop.h"
#include "TEncGOP.h"
#include "TEncAnalyze.h"

#include <time.h>

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TEncGOP::TEncGOP()
{
  m_iHrchDepth          = 0;
  m_iGopSize            = 0;
  m_iNumPicCoded        = 0; //Niko
  m_bFirst              = true;
  
  m_pcCfg               = NULL;
  m_pcPictureEncoder      = NULL;
  m_pcListPic           = NULL;
  
  m_pcEntropyCoder      = NULL;
  m_pcSbacCoder         = NULL;
  
  m_bSeqFirst           = true;
  
#if B_RPS_BUG
  m_bSeqEnd = false;
#endif

  return;
}

TEncGOP::~TEncGOP()
{
}

Void  TEncGOP::create()
{
}

Void  TEncGOP::destroy()
{
}

Void TEncGOP::init ( TEncTop* pcTEncTop )
{
  m_pcEncTop     = pcTEncTop;
  m_pcCfg                = pcTEncTop;
  m_pcPictureEncoder       = pcTEncTop->getPictureEncoder();
  m_pcListPic            = pcTEncTop->getListPic();
  
  m_pcEntropyCoder       = pcTEncTop->getEntropyCoder();
  m_pcSbacCoder          = pcTEncTop->getSbacCoder();
  m_pcLoopFilter         = pcTEncTop->getLoopFilter();
  m_pcBitCounter         = pcTEncTop->getBitCounter();
  m_pcLoopFilter->setEntropy( m_pcEntropyCoder );

  // initialize RD classes
  m_pcLoopFilter->setRDCost         ( pcTEncTop->getRdCost()          );
  m_pcLoopFilter->setSbacCoder      ( pcTEncTop->getSbacCoder()       );
  m_pcLoopFilter->setRDSbacCoder    ( pcTEncTop->getRDSbacCoder()     );
  m_pcLoopFilter->setRDGoOnSbacCoder( pcTEncTop->getRDGoOnSbacCoder() );
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

//test
Int write_flag = 0;
Void TEncGOP::compressGOP( Int iPOCLast, Int iNumPicRcvd, TComList<TComPic*>& rcListPic, TComList<TComPicYuv*>& rcListPicYuvRecOut, TComList<TComBitstream*> rcListBitstreamOut )
{
  TComPic*        pcPic;
  TComPicYuv*     pcPicYuvRecOut;
  TComBitstream*  pcBitstreamOut;
  TComPicYuv      cPicOrg;
  //stats
  TComBitstream*  pcOut = new TComBitstream;
  pcOut->create( 500000 );
  
  xInitGOP( iPOCLast, iNumPicRcvd, rcListPic, rcListPicYuvRecOut );
  
  m_iNumPicCoded = 0;

#if RPS
  Int iTimeOffset;
  Int uiPOCCurr;
  Int iGOPid;
   for (iGOPid = 0; iGOPid < m_iGopSize; iGOPid++)
  {
#if B_RPS
	  if (iPOCLast == 0) //case first frame or first top field
	  {
		  uiPOCCurr = 0;
		  iTimeOffset = 1;
	  }
#if B_RPS_BUG
#if !B_RPS_BUG_814
	  else if (iNumPicRcvd == m_iGopSize)
	  {
		  uiPOCCurr = iPOCLast - iNumPicRcvd + m_pcCfg->getGOPEntry(iGOPid).m_iPOC;   //
		  iTimeOffset = m_pcCfg->getGOPEntry(iGOPid).m_iPOC;
	  }
#endif 
	  else if (iNumPicRcvd < m_iGopSize && m_pcCfg->getIntraPeriod() > 1) //ra结构下，存在末尾的帧数不够一个GOP时的特殊情况 RD的操作规则
	  {
		  m_bSeqEnd = true;
		  m_uiSeqEndGOPPicNum = iNumPicRcvd;
		  if (iGOPid == 0)
		  {
			  uiPOCCurr = iPOCLast - iNumPicRcvd + m_uiSeqEndGOPPicNum;   //
			  iTimeOffset = m_uiSeqEndGOPPicNum;
		  }
		  else
		  {
			  while (m_pcCfg->getGOPEntry(iGOPid).m_iPOC >iNumPicRcvd || m_pcCfg->getGOPEntry(iGOPid).m_iPOC == iNumPicRcvd)
			  {
				  iGOPid++;
			  }
			  uiPOCCurr = iPOCLast - iNumPicRcvd + m_pcCfg->getGOPEntry(iGOPid).m_iPOC;   //
			  iTimeOffset = m_pcCfg->getGOPEntry(iGOPid).m_iPOC;
		  }
	  }
#if B_RPS_BUG_814
	  else
	  {
		  uiPOCCurr = iPOCLast - iNumPicRcvd + m_pcCfg->getGOPEntry(iGOPid).m_iPOC;   //
		  iTimeOffset = m_pcCfg->getGOPEntry(iGOPid).m_iPOC;
	  }
#endif
#else
	  else
	  {
		  uiPOCCurr = iPOCLast - iNumPicRcvd + m_pcCfg->getGOPEntry(iGOPid).m_iPOC;   //
		  iTimeOffset = m_pcCfg->getGOPEntry(iGOPid).m_iPOC;
	  }
#endif
#else


    uiPOCCurr = iPOCLast - iNumPicRcvd + m_pcCfg->getGOPEntry(iGOPid).m_iPOC;   //
    iTimeOffset = m_pcCfg->getGOPEntry(iGOPid).m_iPOC;
#endif
#else
  for (Int iDepth = 0; iDepth < m_iHrchDepth; iDepth++)
  {
    Int iTimeOffset = (1 << (m_iHrchDepth - 1 - iDepth));
    Int iStep = iTimeOffset << 1;

    // generalized B info.
    if ((m_pcCfg->getHierarchicalCoding() == false) && (iDepth != 0))
    {
      iTimeOffset = 1;
      iStep = 1;
    }

    for (; iTimeOffset <= iNumPicRcvd; iTimeOffset += iStep)
#endif
    {
      //-- For time output for each picture
      long iBeforeTime = clock();
#if wlq_FME
	  getPictureEncoder()->getPredSearch()->AddPicCodedNum(1);
#endif
#if !RPS
      // generalized B info.
      if ((m_pcCfg->getHierarchicalCoding() == false) && (iDepth != 0) && (iTimeOffset == m_iGopSize) && (iPOCLast != 0))
      {
        continue;
      }

      /////////////////////////////////////////////////////////////////////////////////////////////////// Initial to start encoding
      uiPOCCurr = iPOCLast - (iNumPicRcvd - iTimeOffset);

#endif  
      
      xGetBuffer( rcListPic, rcListPicYuvRecOut, rcListBitstreamOut, iNumPicRcvd, iTimeOffset,  pcPic, pcPicYuvRecOut, pcBitstreamOut, uiPOCCurr );
      
      // save original picture
      cPicOrg.create( pcPic->getPicYuvOrg()->getWidth(), pcPic->getPicYuvOrg()->getHeight(), g_uiLog2MaxCUSize, g_uiMaxCUDepth );
      pcPic->getPicYuvOrg()->copyToPic( &cPicOrg );
      
      //  Bitstream reset
      pcBitstreamOut->resetBits();
      pcBitstreamOut->rewindStreamPacket();
      
      //  Picture data initialization
#if AVS3_PIC_HEADER_ZL
      TComPicHeader*  pcPicHeader;
#endif
      TComPicture*      pcPicture;
#if RPS
      m_pcPictureEncoder->initEncPicture(pcPic, iPOCLast, uiPOCCurr, iNumPicRcvd, iTimeOffset, iGOPid, pcPicture, pcPicHeader, (m_pcEncTop->getSPS()));
      if (pcPicture->getPictureType() == B_PICTURE&&m_pcCfg->getGOPEntry(iGOPid).m_iSliceType == 'P')
      {
        pcPicture->setPictureType(P_PICTURE);
	  }
      if (pcPicture->getPictureType() == B_PICTURE&&m_pcCfg->getGOPEntry(iGOPid).m_iSliceType == 'I')
      {
        pcPicture->setPictureType(I_PICTURE);
      }
#if F_RPS
	  if (pcPicture->getPictureType() == B_PICTURE &&( m_pcCfg->getGOPEntry(iGOPid).m_iSliceType == 'F' || 
		  ( m_pcCfg->getGOPEntry(iGOPid).m_iSliceType == 'P' && m_pcCfg->getFFrameEnable())))
	  {
		  pcPicture->setPictureType(F_PICTURE);
	  }
#endif
#else
#if AVS3_PIC_HEADER_ZL
      m_pcPictureEncoder->initEncPicture(pcPic, iPOCLast, uiPOCCurr, iNumPicRcvd, iTimeOffset, iDepth, pcPicture, pcPicHeader, (m_pcEncTop->getSPS()));
#else
      m_pcPictureEncoder->initEncPicture(pcPic, iPOCLast, uiPOCCurr, iNumPicRcvd, iTimeOffset, iDepth, pcPicture);
#endif
#endif

      //  Set SPS
      pcPicture->setSPS( m_pcEncTop->getSPS() );
#if !AVS3_DELETE_PPS_SYC
      pcPicture->setPPS( m_pcEncTop->getPPS() );
#endif
#if RPS
#if B_RPS_BUG
	  m_pcEncTop->selectReferencePictureSet(pcPicHeader, uiPOCCurr, iGOPid, iPOCLast);
#else
	  m_pcEncTop->selectReferencePictureSet(pcPicHeader, uiPOCCurr, iGOPid);
#endif
      //  Set reference list
      //ZP
      pcPicHeader->ApplyReferencePictureSet(rcListPic, pcPicHeader->getRPS());
      pcPicHeader->setNumRefIdx(REF_PIC_0, min(m_pcCfg->getGOPEntry(iGOPid).m_iNumberOfNegativeRefPics, pcPicHeader->getRPS()->getNumberOfNegativeRefPictures()));
      pcPicHeader->setNumRefIdx(REF_PIC_1, min(m_pcCfg->getGOPEntry(iGOPid).m_iNumberOfPositiveRefPics, pcPicHeader->getRPS()->getNumberOfPositiveRefPictures()));


#endif 
      //  Set reference list
#if RPS
      pcPicHeader->setRefPic(rcListPic);
#else
      pcPicture->setRefPic(rcListPic);

#endif
      
      //  Picture info. refinement
#if RPS
      if ((pcPicture->getPictureType() == B_PICTURE) && pcPicHeader->getRPS()->getNumberOfPositiveRefPictures() == 0) //当帧类型为B帧时，如果list1的参考帧数为0则作为P帧类型进行参考
      {
        pcPicture->setPictureType(P_PICTURE);
        pcPicHeader->setPictureType(P_PICTURE);
#if F_RPS
		if (m_pcCfg->getFFrameEnable())
		{
			pcPicture->setPictureType(F_PICTURE);
			pcPicHeader->setPictureType(F_PICTURE);
		}
#endif
        pcPicture->setDRBFlag(true);
      }
#else
      if ((pcPicture->getPictureType() == B_PICTURE) && pcPicture->getRefPic(REF_PIC_1) == NULL)
      {
        pcPicture->setPictureType(P_PICTURE);
        pcPicture->setDRBFlag(true);
      }
#endif
      
      // Generalized B
#if RPS
      if (m_pcCfg->getUseGPB())
      {
#if F_RPS
		  if (pcPicture->getPictureType() == P_PICTURE || pcPicture->getPictureType() == F_PICTURE)
#else
        if (pcPicture->getPictureType() == P_PICTURE)
#endif
        {
          pcPicture->setPictureType(B_PICTURE); // Change picture type by force
#if RPS_BUG
          pcPicHeader->setPictureType(B_PICTURE);
#endif
          Int iNumRefIdx = pcPicHeader->getNumRefIdx(REF_PIC_0);
          pcPicHeader->setNumRefIdx(REF_PIC_1, iNumRefIdx);
          for (Int iRefIdx = 0; iRefIdx < iNumRefIdx; iRefIdx++)
          {
            pcPicHeader->setRefPic(pcPicHeader->getRefPic(REF_PIC_0, iRefIdx), REF_PIC_1, iRefIdx);
          }
        }
      }
#else
      if (m_pcCfg->getUseGPB())
      {
        if (pcPicture->getPictureType() == P_PICTURE)
        {
          pcPicture->setPictureType(B_PICTURE); // Change picture type by force
          pcPicture->setRefPic(pcPicture->getRefPic(REF_PIC_0), REF_PIC_1);
        }
      }
#endif
      
      //-------------------------------------------------------------
#if RPS
	  #if  inter_direct_skip_bug1
	//下面的P帧要改为F帧  pcCU->getPicture()->getPicHeader()->geInterSkipmodeNumber()
#if inter_direct_skip_bug3
	  pcPicHeader->setInterSkipmodeNumber(pcPicHeader->getPictureType() == F_PICTURE&&pcPicHeader->getSeqHeader()->getWeightedSkipEnableFlag() == true ? pcPicHeader->getNumRefIdx((RefPic)0) : 1);
#else
	  pcPicHeader->setInterSkipmodeNumber(pcPicHeader->getPictureType() == P_PICTURE&&pcPicHeader->getSeqHeader()->getWeightedSkipEnableFlag() == true ? pcPicHeader->getNumRefIdx((RefPic)0) : 1);
#endif
#endif
      pcPicHeader->setRefPOCList();
      pcPicture->setPicHeader(pcPicHeader);
      pcPicture->getPicHeader();
#else
      pcPicture->setRefPOCList();
#endif

#if wlq_WQ
	  if (pcPicture->getSPS()->getWeightQuantEnableFlag() && m_pcCfg->getPicWQEnable())
	  {
		  getPictureEncoder()->getTrQuant()->setWeightQuantEnableFlag(true);
		  InitFrameQuantParam(pcPicture->getPicHeader()->getWeightQuantSyntax());
		  FrameUpdateWQMatrix(pcPicture->getPicHeader()->getWeightQuantSyntax());
	  }
	  else
		  getPictureEncoder()->getTrQuant()->setWeightQuantEnableFlag(false);
#endif
      /////////////////////////////////////////////////////////////////////////////////////////////////// Compress a picture
      //  Picture compression
      if (m_pcCfg->getUseASR())
      {
        m_pcPictureEncoder->setSearchRange(pcPicture);
      }
//#if INTERTEST
//      pcPicHeader->setMVtestEnable(false);
//#endif

            m_pcPictureEncoder->compressPicture   ( pcPic );
//#if INTERTEST
//      pcPicHeader->setMVtestEnable(true);
//#endif
      //-- Loop filter
      m_pcLoopFilter->setCfg(pcPicture->getLoopFilterDisable());

      // set entropy coder
#if AVS3_PIC_HEADER_ZL
      m_pcEntropyCoder->setEntropyCoder   ( m_pcSbacCoder, pcPicture, pcPicHeader );
#else
      m_pcEntropyCoder->setEntropyCoder   ( m_pcSbacCoder, pcPicture );
#endif
      m_pcEntropyCoder->resetEntropy      ();

      // estimate & perform loop filter
      m_pcLoopFilter->loopFilterPic( pcPic );
      
      /////////////////////////////////////////////////////////////////////////////////////////////////// File writing
      // Set entropy coder
#if AVS3_PIC_HEADER_ZL
      m_pcEntropyCoder->setEntropyCoder   ( m_pcSbacCoder, pcPicture, pcPicHeader );
#else
      m_pcEntropyCoder->setEntropyCoder   ( m_pcSbacCoder, pcPicture );
#endif
      m_pcEntropyCoder->setBitstream      ( pcBitstreamOut          );
      
      // write SPS
      if ( m_bSeqFirst )
      {
        m_pcEntropyCoder->encodeSPS( pcPicture->getSPS() );
        pcBitstreamOut->write( 1, 1 );
        pcBitstreamOut->writeAlignZero();
        
#if !AVS3_DELETE_PPS_SYC
        // generate start code
        pcBitstreamOut->write( 1, 32);
        m_pcEntropyCoder->encodePPS( pcPicture->getPPS() );
        pcBitstreamOut->write( 1, 1 );
        pcBitstreamOut->writeAlignZero();
#endif
        
        // generate start code
#if !YQH_AVS2_BITSTREAM
        pcBitstreamOut->write( 1, 32);
#endif
        m_bSeqFirst = false;
      }
      
      UInt uiPosBefore = pcBitstreamOut->getNumberOfWrittenBits()>>3;
#if AVS3_PIC_HEADER_ZL
      if ( pcPicture->getPictureType() == I_PICTURE )
      {
        m_pcEntropyCoder->encodeIPicHeader( pcPicHeader);
#if YQH_AVS2_BITSTREAM
		pcBitstreamOut->write(1, 1);  //yuquanhe@hisilicon.com
		pcBitstreamOut->writeAlignZero();//yuquanhe@hisilicon.com
#endif
#if AVS3_EXTENSION_DEBUG_SYC
        // write SliceHeader
        m_pcEntropyCoder->encodePictureHeader ( pcPicture );
#endif
      }
      else
      {
        m_pcEntropyCoder->encodePBPicHeader( pcPicHeader);
#if YQH_AVS2_BITSTREAM
		pcBitstreamOut->write(1, 1);  //yuquanhe@hisilicon.com
		pcBitstreamOut->writeAlignZero();//yuquanhe@hisilicon.com
#endif
#if AVS3_EXTENSION_DEBUG_SYC
        // write SliceHeader
        m_pcEntropyCoder->encodePictureHeader ( pcPicture );
#endif
      }
#endif
#if !AVS3_EXTENSION_DEBUG_SYC
      // write PictureHeader
      m_pcEntropyCoder->encodePictureHeader ( pcPicture );
#endif
      
      // is it needed?
#if AVS3_PIC_HEADER_ZL
      m_pcEntropyCoder->setEntropyCoder ( m_pcSbacCoder, pcPic->getPicture(), pcPic->getPicHeader() );
#else
      m_pcEntropyCoder->setEntropyCoder ( m_pcSbacCoder, pcPic->getPicture() );
#endif
      
      m_pcEntropyCoder->resetEntropy    ();
      
      /////////////////////////////////////////////////////////////////////////////////////////////////// Reconstructed image output

	  // File writing
	  //test
	  write_flag = 1;
      m_pcPictureEncoder->encodePicture( pcPic, pcBitstreamOut );
	  //test
	  write_flag = 0;

      //  End of bitstream & byte align
      pcBitstreamOut->write( 1, 1 );
      pcBitstreamOut->writeAlignZero();
#if   YQH_AVS2_BITSTREAM
	  pcBitstreamOut->write(0x01b1, 32);//end code
#endif      
      pcBitstreamOut->flushBuffer();
#if !YQH_AVS2_BITSTREAM
      pcBitstreamOut->convertRBSPToPayload( uiPosBefore );
#endif

      // save original picture
      cPicOrg.copyToPic( pcPic->getPicYuvOrg() );


#if DPB

	  pcPic->compressMotion();
#endif
      //-- For time output for each picture
      Double dEncTime = (double)(clock()-iBeforeTime) / CLOCKS_PER_SEC;
      
      xCalculateAddPSNR( pcPic, pcPic->getPicYuvRec(), pcBitstreamOut->getNumberOfWrittenBits(), dEncTime );
      
      // free original picture
      cPicOrg.destroy();
      
      //  Reconstruction buffer update
      pcPic->getPicYuvRec()->copyToPic(pcPicYuvRecOut);

      pcPic->setReconMark   ( true );
      
      m_bFirst = false;
      m_iNumPicCoded++;
    }
    
#if RPS
    //当结尾时判定跳出
    if (m_iNumPicCoded == iNumPicRcvd)
    {
      break;
    }
#else
    // generalized B info.
    if (m_pcCfg->getHierarchicalCoding() == false && iDepth != 0)
      break;
#endif
  }
  
  pcOut->destroy();
  delete pcOut;
  
  assert ( m_iNumPicCoded == iNumPicRcvd );
}

#if wlq_WQ
Void TEncGOP::InitFrameQuantParam(WeightQuantSyntax* WeightQuantSyntax)
{
	int i, j, k;
	int uiWQMSizeId, uiBlockSize, uiWMQId;
	Int *cur_wq_matrix;
	Short* wq_matrix;

	if ( WeightQuantSyntax->m_uiPicWeightQuantDataIndex == 1 ) {
		// Patch the Weighting Parameters
		// Use default weighted parameters, input->WQParam==0
		for (j = 0; j < 6; j++) {
			WeightQuantSyntax->m_iWeightQuantParamDelta1[j] = 1 << WQ_FLATBASE_INBIT_AVS2;
			WeightQuantSyntax->m_iWeightQuantParamDelta2[j] = 1 << WQ_FLATBASE_INBIT_AVS2;
		}

		// if input->WQParam!=0, update wq_param
		if ( WeightQuantSyntax->m_uiWeightQuantParamIndex == 0 ) {
			// default Param - Detailed
			for (i = 0; i < 6; i++)
				WeightQuantSyntax->m_iWeightQuantParamDelta2[i] = g_wq_param_default[DETAILED_AVS2][i];
		}
		else if ( WeightQuantSyntax->m_uiWeightQuantParamIndex == 1 ) {
			// 7.2.3.1 UnDetailed Param User Defined
			// Load user defined weighted parameters
#if !wlq_WQ_q
			GetUserDefParam(input->WeightParamUnDetailed, 0);
#endif
		}
		else if ( WeightQuantSyntax->m_uiWeightQuantParamIndex == 2) {
			// 7.2.3.1 Detailed Param User Defined
			// Load user defined weighted parameters
#if !wlq_WQ_q
			GetUserDefParam(input->WeightParamDetailed, 1);
#endif
		}

		// Reconstruct the Weighting matrix
#if !wlq_WQ_q
		wq_model = input->WQModel;
#endif
		wq_matrix = getPictureEncoder()->getTrQuant()->getWqMatrix(1, 0);
		for (j = 0; j < 8; j++)
			for (i = 0; i < 8; i++) {
				wq_matrix[j * 8 + i] = (WeightQuantSyntax->m_iWeightQuantParamDelta1[g_WeightQuantModel8x8[WeightQuantSyntax->m_uiWeightQuantMode][j * 8 + i]]);
			}

		wq_matrix = getPictureEncoder()->getTrQuant()->getWqMatrix(1, 1);
		for (j = 0; j < 8; j++)
			for (i = 0; i < 8; i++) {
				wq_matrix[j * 8 + i] = (WeightQuantSyntax->m_iWeightQuantParamDelta2[g_WeightQuantModel8x8[WeightQuantSyntax->m_uiWeightQuantMode][j * 8 + i]]);
			}
		// afw_4x4  Jianhua Zheng 200906

		wq_matrix = getPictureEncoder()->getTrQuant()->getWqMatrix(0, 0);
		for (j = 0; j < 4; j++)
			for (i = 0; i < 4; i++) {
				wq_matrix[j * 4 + i] = (WeightQuantSyntax->m_iWeightQuantParamDelta1[g_WeightQuantModel4x4[WeightQuantSyntax->m_uiWeightQuantMode][j * 4 + i]]);
			}

		wq_matrix = getPictureEncoder()->getTrQuant()->getWqMatrix(0, 1);
		for (j = 0; j < 4; j++)
			for (i = 0; i < 4; i++) {
				wq_matrix[j * 4 + i] = (WeightQuantSyntax->m_iWeightQuantParamDelta2[g_WeightQuantModel4x4[WeightQuantSyntax->m_uiWeightQuantMode][j * 4 + i]]);
			}
	}//input->PicWQDataIndex==1
	else if ( WeightQuantSyntax->m_uiPicWeightQuantDataIndex == 2 ) {
		for (uiWQMSizeId = 0; uiWQMSizeId < 2; uiWQMSizeId++) {
			uiBlockSize = min(1 << (uiWQMSizeId + 2), 8);
			uiWMQId = (uiWQMSizeId < 2) ? uiWQMSizeId : 1;
#if !wlq_WQ_q
			GetUserDefWQM(input->PicWQFile, uiWMQId, Pic_WQM);//yuquanhe@hisilicon.com
			for (i = 0; i < (uiBlockSize * uiBlockSize); i++) {
				pic_user_wq_matrix[uiWQMSizeId][i] = Pic_WQM[i];
			}
#endif
		}
	}//input->PicWQDataIndex==2

	for (uiWQMSizeId = 0; uiWQMSizeId < SCALING_LIST_SIZE_NUM; uiWQMSizeId++)
	{
		cur_wq_matrix = getPictureEncoder()->getTrQuant()->getDequantCoeff(uiWQMSizeId);
		for (i = 0; i < 64; i++) {
			cur_wq_matrix[i] = 1 << WQ_FLATBASE_INBIT_AVS2;
		}
	}
}

Void TEncGOP::FrameUpdateWQMatrix(WeightQuantSyntax* WeightQuantSyntax)
{
	int i,j;
	int uiWQMSizeId, uiWMQId;
	int uiBlockSize;
	Int *cur_wq_matrix;
	Short* wq_matrix;

	for (uiWQMSizeId = 0; uiWQMSizeId < SCALING_LIST_SIZE_NUM; uiWQMSizeId++)
	{
		cur_wq_matrix = getPictureEncoder()->getTrQuant()->getDequantCoeff(uiWQMSizeId);
		uiBlockSize = min(1 << (uiWQMSizeId + 2), 8);
		uiWMQId = (uiWQMSizeId < 2) ? uiWQMSizeId : 1;
		if (WeightQuantSyntax->m_uiPicWeightQuantDataIndex == 0) {
			if (uiWMQId == 0)
			{
				for (j = 0; j < uiBlockSize; j++)
					for (i = 0; i < uiBlockSize; i++)
						cur_wq_matrix[j*uiBlockSize + i] = WeightQuantSyntax->m_uiWeightQuantCoeff4x4[j][i];
			}
			else
			{
				for (j = 0; j < uiBlockSize; j++)
					for (i = 0; i < uiBlockSize; i++)
						cur_wq_matrix[j*uiBlockSize + i] = WeightQuantSyntax->m_uiWeightQuantCoeff8x8[j][i];
			}
		}
		else if (WeightQuantSyntax->m_uiPicWeightQuantDataIndex == 1) {
			if (WeightQuantSyntax->m_uiWeightQuantParamIndex == 0) {
				wq_matrix = getPictureEncoder()->getTrQuant()->getWqMatrix(uiWMQId, DETAILED_AVS2);
				for (i = 0; i < (uiBlockSize * uiBlockSize); i++) {
					cur_wq_matrix[i] = wq_matrix[i];
				}
			}
			else if (WeightQuantSyntax->m_uiWeightQuantParamIndex == 1) {
				wq_matrix = getPictureEncoder()->getTrQuant()->getWqMatrix(uiWMQId, 0);
				for (i = 0; i < (uiBlockSize * uiBlockSize); i++) {
					cur_wq_matrix[i] = wq_matrix[i];
				}
			}
			else if (WeightQuantSyntax->m_uiWeightQuantParamIndex == 2) {
				wq_matrix = getPictureEncoder()->getTrQuant()->getWqMatrix(uiWMQId, 1);
				for (i = 0; i < (uiBlockSize * uiBlockSize); i++) {
					cur_wq_matrix[i] = wq_matrix[i];
				}
			}
		}
		else if (WeightQuantSyntax->m_uiPicWeightQuantDataIndex == 2) {
#if wlq_WQ_q
			if (uiWMQId == 0)
			{
				for (j = 0; j < uiBlockSize; j++)
					for (i = 0; i < uiBlockSize; i++)
						cur_wq_matrix[j*uiBlockSize + i] = WeightQuantSyntax->m_uiWeightQuantCoeff4x4[j][i];
			}
			else
			{
				for (j = 0; j < uiBlockSize; j++)
					for (i = 0; i < uiBlockSize; i++)
						cur_wq_matrix[j*uiBlockSize + i] = WeightQuantSyntax->m_uiWeightQuantCoeff8x8[j][i];
			}
#endif
		}
	}

	for (uiWQMSizeId = 0; uiWQMSizeId < SCALING_LIST_SIZE_NUM; uiWQMSizeId++)
			CalculateQuantParam(uiWQMSizeId);
}

Void  TEncGOP::CalculateQuantParam(UChar uiWQMSizeId)  //  uiWQMSizeId   //0: 4x4  1:8x8  2: 16x16  3:32x32
{
	Int i, j;
	Int *LevelScaleNxN = getPictureEncoder()->getTrQuant()->getQuantCoeff(uiWQMSizeId);  //不区分Intra和Inter, RD虽区分, 但数据结结构一致
	UChar UseDefaultScalingMatrixFlag[2];
	Int uiBlockSize = 1 << (uiWQMSizeId + 2);
	Int *cur_wq_matrix = getPictureEncoder()->getTrQuant()->getDequantCoeff(uiWQMSizeId);

	for (j = 0; j < uiBlockSize; j++)
		for (i = 0; i < uiBlockSize; i++) {
			if ((uiWQMSizeId == 0) || (uiWQMSizeId == 1)) {
				LevelScaleNxN[j*uiBlockSize + i] = (Int)((Float)(g_scaleM[j & 3][i & 3] << WQ_FLATBASE_INBIT_AVS2) / cur_wq_matrix[j *
					uiBlockSize + i]);
			}
			else if (uiWQMSizeId == 2) {
				LevelScaleNxN[j*uiBlockSize + i] = (Int)((Float)(g_scaleM[j & 3][i & 3] << WQ_FLATBASE_INBIT_AVS2) / cur_wq_matrix[(j >>
					1) * (uiBlockSize >> 1) + (i >> 1)]);
			}
			else if (uiWQMSizeId == 3) {
				LevelScaleNxN[j*uiBlockSize + i] = (Int)((Float)(g_scaleM[j & 3][i & 3] << WQ_FLATBASE_INBIT_AVS2) / cur_wq_matrix[(j >>
					2) * (uiBlockSize >> 2) + (i >> 2)]);
			}
		}
}
#endif

#if wlq_FME
Void TEncGOP::InitRdCostMvBits()
{
	Int search_range = m_pcCfg->getSearchRange();
	Int number_of_subpel_positions = 4 * (2 * search_range + 3);
	Int MaxMvBits = 3 + 2 * (int)ceil(log(number_of_subpel_positions + 1) / log(2) + 1e-10);
	getPictureEncoder()->getRdCost()->initMVbits(MaxMvBits); 
}
#endif

Void TEncGOP::printOutSummary(UInt uiNumAllPicCoded)
{
  assert (uiNumAllPicCoded == m_gcAnalyzeAll.getNumPic());
  
  //--CFG_KDY
  m_gcAnalyzeAll.setFrmRate( m_pcCfg->getFrameRate() );
  m_gcAnalyzeI.setFrmRate( m_pcCfg->getFrameRate() );
#if F_RPS
  m_gcAnalyzeF.setFrmRate(m_pcCfg->getFrameRate());
#endif
  m_gcAnalyzeP.setFrmRate( m_pcCfg->getFrameRate() );
  m_gcAnalyzeB.setFrmRate( m_pcCfg->getFrameRate() );
  
  //-- all
  printf( "\n\nSUMMARY --------------------------------------------------------\n" );
  m_gcAnalyzeAll.printOut('a');
  
  printf( "\n\nI Pictures--------------------------------------------------------\n" );
  m_gcAnalyzeI.printOut('i');
  
#if F_RPS
  printf("\n\nF Pictures--------------------------------------------------------\n");
  m_gcAnalyzeF.printOut('f');
#endif

  printf( "\n\nP Pictures--------------------------------------------------------\n" );
  m_gcAnalyzeP.printOut('p');
  
  printf( "\n\nB Pictures--------------------------------------------------------\n" );
  m_gcAnalyzeB.printOut('b');
  
#if _SUMMARY_OUT_
  m_gcAnalyzeAll.printSummaryOut();
#endif
#if _SUMMARY_PIC_
  m_gcAnalyzeI.printSummary('I');
  m_gcAnalyzeP.printSummary('P');
  m_gcAnalyzeB.printSummary('B');
#endif
}

Void TEncGOP::preLoopFilterPicAll( TComPic* pcPic, UInt64& ruiDist, UInt64& ruiBits )
{
  TComPicture* pcPicture = pcPic->getPicture();
  Bool bCalcDist = false;
  
  m_pcLoopFilter->setCfg(pcPicture->getLoopFilterDisable());
  m_pcLoopFilter->loopFilterPic( pcPic );
  
#if AVS3_PIC_HEADER_ZL
  TComPicHeader* pcPicHeader = pcPic->getPicHeader();
  m_pcEntropyCoder->setEntropyCoder ( m_pcEncTop->getRDGoOnSbacCoder(), pcPicture, pcPicHeader );
#else
  m_pcEntropyCoder->setEntropyCoder ( m_pcEncTop->getRDGoOnSbacCoder(), pcPicture );
#endif
  m_pcEntropyCoder->resetEntropy    ();
  m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
  
  m_pcEntropyCoder->resetEntropy    ();
  ruiBits += m_pcEntropyCoder->getNumberOfWrittenBits();
  
  if (!bCalcDist)
    ruiDist = xFindDistortionFrame(pcPic->getPicYuvOrg(), pcPic->getPicYuvRec());
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TEncGOP::xInitGOP( Int iPOCLast, Int iNumPicRcvd, TComList<TComPic*>& rcListPic, TComList<TComPicYuv*>& rcListPicYuvRecOut )
{
  assert( iNumPicRcvd > 0 );
  Int i;
  
  //  Set hierarchical B info.
  m_iGopSize    = m_pcCfg->getGOPSize();
  for( i=1 ; ; i++)
  {
    m_iHrchDepth = i;
    if((m_iGopSize >> i)==0)
    {
      break;
    }
  }
  
  //  Exception for the first frame
  if ( iPOCLast == 0 )
  {
    m_iGopSize    = 1;
    m_iHrchDepth  = 1;
  }
  
  if (m_iGopSize == 0)
  {
    m_iHrchDepth = 1;
  }
  
  return;
}

Void TEncGOP::xGetBuffer( TComList<TComPic*>&       rcListPic,
                         TComList<TComPicYuv*>&    rcListPicYuvRecOut,
                         TComList<TComBitstream*>& rcListBitstreamOut,
                         Int                       iNumPicRcvd,
                         Int                       iTimeOffset,
                         TComPic*&                 rpcPic,
                         TComPicYuv*&              rpcPicYuvRecOut,
                         TComBitstream*&           rpcBitstreamOut,
                         UInt                      uiPOCCurr )
{
  Int i;
  //  Rec. output
  TComList<TComPicYuv*>::iterator     iterPicYuvRec = rcListPicYuvRecOut.end();
  for ( i = 0; i < iNumPicRcvd - iTimeOffset + 1; i++ )
  {
    iterPicYuvRec--;
  }
  
  rpcPicYuvRecOut = *(iterPicYuvRec);
  
  //  Bitstream output
  TComList<TComBitstream*>::iterator  iterBitstream = rcListBitstreamOut.begin();
  for ( i = 0; i < m_iNumPicCoded; i++ )
  {
    iterBitstream++;
  }
  rpcBitstreamOut = *(iterBitstream);
  
  //  Current pic.
  TComList<TComPic*>::iterator        iterPic       = rcListPic.begin();
  while (iterPic != rcListPic.end())
  {
    rpcPic = *(iterPic);
    if (rpcPic->getPOC() == (Int)uiPOCCurr)
    {
      break;
    }
    iterPic++;
  }
  
  assert (rpcPic->getPOC() == (Int)uiPOCCurr);
  
  return;
}

UInt64 TEncGOP::xFindDistortionFrame (TComPicYuv* pcPic0, TComPicYuv* pcPic1)
{
  Int     x, y;
  Pel*  pSrc0   = pcPic0 ->getLumaAddr();
  Pel*  pSrc1   = pcPic1 ->getLumaAddr();
  Int   iTemp;
  
  Int   iStride = pcPic0->getStride();
  Int   iWidth  = pcPic0->getWidth();
  Int   iHeight = pcPic0->getHeight();
  
  UInt64  uiTotalDiff = 0;
  
  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      iTemp = pSrc0[x] - pSrc1[x]; uiTotalDiff += (iTemp*iTemp);
    }
    pSrc0 += iStride;
    pSrc1 += iStride;
  }
  
  iHeight >>= 1;
  iWidth  >>= 1;
  iStride >>= 1;
  
  pSrc0  = pcPic0->getCbAddr();
  pSrc1  = pcPic1->getCbAddr();
  
  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      iTemp = pSrc0[x] - pSrc1[x]; uiTotalDiff += (iTemp*iTemp);
    }
    pSrc0 += iStride;
    pSrc1 += iStride;
  }
  
  pSrc0  = pcPic0->getCrAddr();
  pSrc1  = pcPic1->getCrAddr();
  
  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      iTemp = pSrc0[x] - pSrc1[x]; uiTotalDiff += (iTemp*iTemp);
    }
    pSrc0 += iStride;
    pSrc1 += iStride;
  }
  
  return uiTotalDiff;
}

Void TEncGOP::xCalculateAddPSNR( TComPic* pcPic, TComPicYuv* pcPicD, UInt uibits, Double dEncTime )
{
  Int     x, y;
  UInt    uiSSDY  = 0;
  UInt    uiSSDU  = 0;
  UInt    uiSSDV  = 0;
  
  Double  dYPSNR  = 0.0;
  Double  dUPSNR  = 0.0;
  Double  dVPSNR  = 0.0;
  
  //===== calculate PSNR =====
  Pel*  pOrg    = pcPic ->getPicYuvOrg()->getLumaAddr();
  Pel*  pRec    = pcPicD->getLumaAddr();
  Int   iStride = pcPicD->getStride();
  
  Int   iWidth;
  Int   iHeight;
  
  iWidth  = pcPicD->getWidth () - m_pcEncTop->getPad(0);
  iHeight = pcPicD->getHeight() - m_pcEncTop->getPad(1);
  
  Int   iSize   = iWidth*iHeight;
  
  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      Int iDiff = (Int)( pOrg[x] - pRec[x] );
      uiSSDY   += iDiff * iDiff;
    }
    pOrg += iStride;
    pRec += iStride;
  }
  
  iHeight >>= 1;
  iWidth  >>= 1;
  iStride >>= 1;
  pOrg  = pcPic ->getPicYuvOrg()->getCbAddr();
  pRec  = pcPicD->getCbAddr();
  
  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      Int iDiff = (Int)( pOrg[x] - pRec[x] );
      uiSSDU   += iDiff * iDiff;
    }
    pOrg += iStride;
    pRec += iStride;
  }
  
  pOrg  = pcPic ->getPicYuvOrg()->getCrAddr();
  pRec  = pcPicD->getCrAddr();
  
  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      Int iDiff = (Int)( pOrg[x] - pRec[x] );
      uiSSDV   += iDiff * iDiff;
    }
    pOrg += iStride;
    pRec += iStride;
  }
  
  Double fRefValueY = 255.0 * 255.0 * (Double)iSize;
  Double fRefValueC = fRefValueY / 4.0;
  dYPSNR            = ( uiSSDY ? 10.0 * log10( fRefValueY / (Double)uiSSDY ) : 99.99 );
  dUPSNR            = ( uiSSDU ? 10.0 * log10( fRefValueC / (Double)uiSSDU ) : 99.99 );
  dVPSNR            = ( uiSSDV ? 10.0 * log10( fRefValueC / (Double)uiSSDV ) : 99.99 );
  
  // fix: total bits should consider picture size bits (32bit)
  uibits += 32;
#if YQH_AVS2_BITSTREAM
  uibits -= 64;
#endif  
  //===== add PSNR =====
  m_gcAnalyzeAll.addResult (dYPSNR, dUPSNR, dVPSNR, (Double)uibits);
  if (pcPic->getPicture()->isIntra())
  {
    m_gcAnalyzeI.addResult (dYPSNR, dUPSNR, dVPSNR, (Double)uibits);
  }
#if F_RPS
  if (pcPic->getPicture()->isInterF())
  {
	  m_gcAnalyzeF.addResult(dYPSNR, dUPSNR, dVPSNR, (Double)uibits);
  }
#endif
  if (pcPic->getPicture()->isInterP())
  {
    m_gcAnalyzeP.addResult (dYPSNR, dUPSNR, dVPSNR, (Double)uibits);
  }
  if (pcPic->getPicture()->isInterB())
  {
    m_gcAnalyzeB.addResult (dYPSNR, dUPSNR, dVPSNR, (Double)uibits);
  }
  
  //===== output =====
  TComPicture*  pcPicture = pcPic->getPicture();
#if F_RPS
  printf("\nPOC %4d ( %c-PICTURE, QP %d ) %10d bits ",
	  pcPicture->getPOC(),
	  pcPicture->isIntra() ? 'I' : pcPicture->isInterF() ? 'F' : pcPicture->isInterP() ? 'P' : 'B',
	  pcPicture->getPictureQp(),
	  uibits);
#else
  printf("\nPOC %4d ( %c-PICTURE, QP %d ) %10d bits ",
         pcPicture->getPOC(),
         pcPicture->isIntra() ? 'I' : pcPicture->isInterP() ? 'P' : 'B',
         pcPicture->getPictureQp(),
         uibits );
#endif

  printf( "[Y %6.4lf dB    U %6.4lf dB    V %6.4lf dB]  ", dYPSNR, dUPSNR, dVPSNR );
  printf ("[ET %5.0f ] ", dEncTime );
  
#if RPS
  for (Int iRefPic = 0; iRefPic < 2; iRefPic++)
  {
    for (Int iNumRefIdx = 0; iNumRefIdx<pcPicture->getPicHeader()->getNumRefIdx(RefPic(iRefPic)); iNumRefIdx++)
    {
      if (pcPicture->getPicHeader()->getRefPic(RefPic(iRefPic), iNumRefIdx))
        printf("[L%d %d]", iRefPic, pcPicture->getPicHeader()->getRefPOC(RefPic(iRefPic), iNumRefIdx));
    }
  }
#else
  for (Int iRefPic = 0; iRefPic < 2; iRefPic++)
  {
    if (pcPicture->getRefPic(RefPic(iRefPic)))
      printf("[L%d %d]", iRefPic, pcPicture->getRefPOC(RefPic(iRefPic)));
  }
#endif
  
  fflush(stdout);
}

