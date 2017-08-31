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

/** \file     TDecTop.cpp
    \brief    decoder class
*/

#include "TDecTop.h"

TDecTop::TDecTop()
{
  m_iGopSize      = 0;
  m_bGopSizeSet   = false;
  m_iMaxRefPicNum = 0;
  m_uiValidPS = 0;
}

TDecTop::~TDecTop()
{
}

Void TDecTop::create()
{
  m_cGopDecoder.create();
  m_apcPicturePilot = new TComPicture;
#if AVS3_PIC_HEADER_ZL
  m_pcPicHeader = new TComPicHeader;
#endif
}

Void TDecTop::destroy()
{
  m_cGopDecoder.destroy();
  
  delete m_apcPicturePilot;
  m_apcPicturePilot = NULL;
#if AVS3_PIC_HEADER_ZL
  delete m_pcPicHeader;
  m_pcPicHeader = NULL;
#endif
  
  m_cPictureDecoder.destroy();
}

Void TDecTop::init()
{
  // initialize ROM
  initROM();
  
  m_cGopDecoder.  init( &m_cEntropyDecoder, &m_cSbacDecoder, &m_cPictureDecoder, &m_cLoopFilter );
  m_cPictureDecoder.init( &m_cEntropyDecoder, &m_cCuDecoder );
  m_cEntropyDecoder.init(&m_cPrediction);
}

Void TDecTop::deletePicBuffer ( )
{
  TComList<TComPic*>::iterator  iterPic   = m_cListPic.begin();
  Int iSize = Int( m_cListPic.size() );
  
  for (Int i = 0; i < iSize; i++ )
  {
    TComPic* pcPic = *(iterPic++);
    pcPic->destroy();
    
    delete pcPic;
    pcPic = NULL;
  }
  
  m_cLoopFilter.        destroy();
  
  // destroy ROM
  destroyROM();
}

Void TDecTop::xUpdateGopSize (TComPicture* pcPicture)
{
  if ( !pcPicture->isIntra() && !m_bGopSizeSet)
  {
    m_iGopSize    = pcPicture->getPOC();
    m_bGopSizeSet = true;
    
    m_cGopDecoder.setGopSize(m_iGopSize);
  }
}

#if RPS
Void TDecTop::xRemovePicBuffer(TComPicHeader* m_pcPicHeader)
{
  TComPic* rpcPic = NULL;
#if !B_RPS_BUG_819
  if (m_pcPicHeader->getRPS()->getNumberOfRemPictures() > 0)
  {
    for (Int i = 0; i < m_pcPicHeader->getRPS()->getNumberOfRemPictures(); i++)
    {
      TComList<TComPic*>::iterator iterPic = m_cListPic.begin();
      while (iterPic != m_cListPic.end())
      {
        rpcPic = *iterPic;
        if (m_pcPicHeader->getRPS()->getRemDeltaPOC(i) + m_pcPicHeader->getPOC() == rpcPic->getPOC())
        {
          m_cListPic.erase(iterPic);
          break;
        }
        *(iterPic++);
      }
    }
  }
#endif
#if B_RPS_BUG_819
  TComList<TComPic*>::iterator iterPic = m_cListPic.begin();
  while (iterPic != m_cListPic.end())
  {
	  rpcPic = *iterPic;
	  if (!rpcPic->getPicture()->isReferenced())
	  {
		  m_cListPic.erase(iterPic);
		  break;
	  }
	  *(iterPic++);
  }
#endif
}
#endif
#if RPS
Void TDecTop::xGetNewPicBuffer(TComPicHeader* m_pcPicHeader, TComPic*& rpcPic)
#else
Void TDecTop::xGetNewPicBuffer(TComPicture* pcPicture, TComPic*& rpcPic)
#endif
{
#if RPS
  //m_iMaxRefPicNum = MAX_NUM_REF_PICS;//暂时认为参考4帧
#if	RPS820
	m_iMaxRefPicNum = (m_pcPicHeader->getRPS()->getNumberOfRefPictures() + 8);
#else
  m_iMaxRefPicNum = (m_pcPicHeader->getRPS()->getNumberOfRefPictures() + 4);
#endif
  assert(m_cListPic.size() < (UInt)m_iMaxRefPicNum);
  if (m_cListPic.size() < (UInt)m_iMaxRefPicNum)
  {
    rpcPic = new TComPic;
    rpcPic->create(m_pcPicHeader->getSeqHeader()->getWidth(), m_pcPicHeader->getSeqHeader()->getHeight(), g_uiLog2MaxCUSize, g_uiMaxCUDepth, true);
    m_cListPic.pushBack(rpcPic);
    return;
  }
#else
  xUpdateGopSize(pcPicture);  //RD 写成定长 7
  m_iMaxRefPicNum = Max(m_iMaxRefPicNum, m_iGopSize / 2 + 3);
  if (m_cListPic.size() < (UInt)m_iMaxRefPicNum)
  {
    rpcPic = new TComPic;

    rpcPic->create(pcPicture->getSPS()->getWidth(), pcPicture->getSPS()->getHeight(), g_uiLog2MaxCUSize, g_uiMaxCUDepth, true);
    m_cListPic.pushBack(rpcPic);

    return;
  }

  Bool bBufferIsAvailable = false;
  TComList<TComPic*>::iterator  iterPic = m_cListPic.begin();
  while (iterPic != m_cListPic.end())
  {
    rpcPic = *(iterPic++);
    if (rpcPic->getReconMark() == false)
    {
      bBufferIsAvailable = true;
      break;
    }
  }

  if (!bBufferIsAvailable)
  {
    pcPicture->sortPicList(m_cListPic);
    iterPic = m_cListPic.begin();
    rpcPic = *(iterPic);
    rpcPic->setReconMark(false);

    // mark it should be extended
    rpcPic->getPicYuvRec()->setBorderExtension(false);
  }
#endif
}

Void TDecTop::decode (Bool bEos, TComBitstream* pcBitstream, UInt& ruiPOC, TComList<TComPic*>*& rpcListPic)
{
  rpcListPic = NULL;
  TComPic*    pcPic = NULL;
  
  // Initialize entropy decoder
  m_cEntropyDecoder.setEntropyDecoder (&m_cSbacDecoder);
  m_cEntropyDecoder.setBitstream      (pcBitstream);
  
  // don't feel like adding the whole chain of interface crap just to access the first byte in the buffer
  const UChar* pucBuffer = reinterpret_cast<const UChar*>(pcBitstream->getStartStream());
  const NalUnitType eNalUnitType = NalUnitType(pucBuffer[0]&31); 
  const bool bDecodeSPS   = ( NAL_UNIT_SPS == eNalUnitType );
#if !AVS3_DELETE_PPS_SYC
  const bool bDecodePPS   = ( NAL_UNIT_PPS == eNalUnitType );
  const bool bDecodePicture = ( NAL_UNIT_CODED_PICTURE == eNalUnitType );
#endif
  if( bDecodeSPS )
  {
    m_cEntropyDecoder.decodeSPS( &m_cSPS );
    
    // initialize DIF
//    m_cPrediction.setChromaFilter();

    m_cLoopFilter.create( g_uiMaxCUDepth );
    m_uiValidPS |= 1;
#if AVS3_DELETE_PPS_SYC
    return;
#endif
  }

#if !AVS3_DELETE_PPS_SYC
  if( bDecodePPS )
  {
    m_cEntropyDecoder.decodePPS( &m_cPPS );
    m_uiValidPS |= 2;
  }
  if( false == bDecodePicture )
  {
    return;
  }
  // make sure we already received both parameter sets
  assert( 3 == m_uiValidPS );
#endif  
  
  
  m_apcPicturePilot->initPicture();
  
  //  Read picture header
  m_apcPicturePilot->setSPS( &m_cSPS );
#if !AVS3_DELETE_PPS_SYC
  m_apcPicturePilot->setPPS( &m_cPPS );
#endif

#if AVS3_PIC_HEADER_ZL
  m_pcPicHeader->initPicHeader();
  m_pcPicHeader->setSeqHeader(&m_cSPS);
  m_cEntropyDecoder.decodePicHeader(m_pcPicHeader);
#endif
#if AVS3_SLICE_HEADER_SYC
  m_cEntropyDecoder.decodePictureHeader (m_apcPicturePilot, m_pcPicHeader);
#else
  m_cEntropyDecoder.decodePictureHeader (m_apcPicturePilot);
#endif
  
  // Buffer initialize for prediction.
  m_cPrediction.initTempBuff();
  //  Get a new picture buffer
#if RPS
  m_pcPicHeader->setPOC(m_apcPicturePilot->getPOC());
  m_pcPicHeader->setPictureType(m_apcPicturePilot->getPictureType());
  xRemovePicBuffer(m_pcPicHeader);
#endif
#if RPS
  xGetNewPicBuffer(m_pcPicHeader, pcPic);
#else
  xGetNewPicBuffer(m_apcPicturePilot, pcPic);
#endif
  
  // Recursive structure
  m_cCuDecoder.create ( g_uiMaxCUDepth, g_uiLog2MaxCUSize );
  m_cCuDecoder.init   ( &m_cEntropyDecoder, &m_cTrQuant, &m_cPrediction );
  m_cTrQuant.init     ( 1 << m_apcPicturePilot->getSPS()->getLog2MaxPhTrSize() );
  
  m_cPictureDecoder.create( m_apcPicturePilot, m_apcPicturePilot->getSPS()->getWidth(), m_apcPicturePilot->getSPS()->getHeight(), g_uiLog2MaxCUSize, g_uiMaxCUDepth );
  
  //  Set picture picture pointer
  TComPicture*  pcPicture = m_apcPicturePilot;
  m_apcPicturePilot = pcPic->getPicSym()->getPicture();
#if AVS3_PIC_HEADER_ZL
  TComPicHeader* pcPicHeader = m_pcPicHeader;
   m_pcPicHeader = pcPic->getPicSym()->getPicHeader();
#endif
  pcPic->getPicSym()->setPicture(pcPicture);
#if AVS3_PIC_HEADER_ZL

   pcPic->getPicSym()->setPicHeader( pcPicHeader );
#endif
  // Set reference list
#if RPS
#if B_RPS_BUG_819
   pcPicHeader->ApplyReferencePictureSet(m_cListPic, pcPicHeader->getRPS());
#endif
   pcPicHeader->setNumRefIdx(REF_PIC_0, pcPicHeader->getRPS()->getNumberOfNegativeRefPictures());
   pcPicHeader->setNumRefIdx(REF_PIC_1, pcPicHeader->getRPS()->getNumberOfPositiveRefPictures());
   pcPicHeader->setRefPic(m_cListPic);

#if  inter_direct_skip_bug2
   //下面的P帧要改为F帧  pcCU->getPicture()->getPicHeader()->geInterSkipmodeNumber()
#if inter_direct_skip_bug3
   pcPicHeader->setInterSkipmodeNumber(pcPicHeader->getPictureType() == F_PICTURE&&pcPicHeader->getSeqHeader()->getWeightedSkipEnableFlag() == true ? pcPicHeader->getNumRefIdx((RefPic)0) : 1);
#else
   pcPicHeader->setInterSkipmodeNumber(pcPicHeader->getPictureType() == P_PICTURE&&pcPicHeader->getSeqHeader()->getWeightedSkipEnableFlag() == true ? pcPicHeader->getNumRefIdx((RefPic)0) : 1);
#endif
#endif


#else
   pcPicture->setRefPic(m_cListPic);

#endif
  
  // HierP + GPB case
#if RPS
   if (m_cSPS.getUseLDC() && pcPicture->isInterB())
   {
     for (Int iNumRefIdx = 0; iNumRefIdx < pcPicHeader->getRPS()->getNumberOfNegativeRefPictures(); iNumRefIdx++)
     {
       pcPicHeader->setRefPic(pcPicHeader->getRefPic(REF_PIC_0, iNumRefIdx), REF_PIC_1, iNumRefIdx);
     }
   }
#else
   if (m_cSPS.getUseLDC() && pcPicture->isInterB())
   {
     pcPicture->setRefPic(pcPicture->getRefPic(REF_PIC_0), REF_PIC_1);
   }
#endif

  // For generalized B
  // note: maybe not existed case (always L0 is copied to L1 if L1 is empty)
#if RPS
   if (pcPicture->isInterB() && pcPicHeader->getRPS()->getNumberOfPositiveRefPictures() == 0)
   {
     for (Int iNumRefIdx = 0; iNumRefIdx < pcPicHeader->getRPS()->getNumberOfNegativeRefPictures(); iNumRefIdx++)
     {
       pcPicHeader->setRefPic(pcPicHeader->getRefPic(REF_PIC_0, iNumRefIdx), REF_PIC_1, iNumRefIdx);
     }

   }
#else
   if (pcPicture->isInterB() && pcPicture->getRefPic(REF_PIC_1) == NULL)
   {
     pcPicture->setRefPic(pcPicture->getRefPic(REF_PIC_0), REF_PIC_1);
   }

#endif

  
  //---------------
#if RPS
   pcPicHeader->setRefPOCList();
   pcPicture->setPicHeader(pcPicHeader);
#else
   pcPicture->setRefPOCList();
#endif

  
  //  Decode a picture
  m_cGopDecoder.decompressGop ( bEos, pcBitstream, pcPic );
  
  pcPicture->sortPicList(m_cListPic);       //  sorting for application output
  
  ruiPOC = pcPic->getPicture()->getPOC();
  
  rpcListPic = &m_cListPic;
  
  m_cCuDecoder.destroy();
  
  return;
}

