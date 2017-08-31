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

/** \file     TEncPicture.cpp
    \brief    picture encoder class
*/

#include "TEncTop.h"
#include "TEncPicture.h"

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncPicture::TEncPicture()
{
  m_apcPicYuvPred = NULL;
  m_apcPicYuvResi = NULL;
  
  m_pdRdPicLambda = NULL;
  m_piRdPicQp     = NULL;
  m_piRdPicChromaQp     = NULL;
  m_pdRdPicChromaLambda = NULL;
}

TEncPicture::~TEncPicture()
{
}

Void TEncPicture::create( Int iWidth, Int iHeight, UInt uiLog2MaxCUSize, UChar uhTotalDepth )
{
  // create prediction picture
  if ( m_apcPicYuvPred == NULL )
  {
    m_apcPicYuvPred  = new TComPicYuv;
    m_apcPicYuvPred->create( iWidth, iHeight, uiLog2MaxCUSize, uhTotalDepth );
  }
  
  // create residual picture
  if( m_apcPicYuvResi == NULL )
  {
    m_apcPicYuvResi  = new TComPicYuv;
    m_apcPicYuvResi->create( iWidth, iHeight, uiLog2MaxCUSize, uhTotalDepth );
  }
}

Void TEncPicture::destroy()
{
  // destroy prediction picture
  if ( m_apcPicYuvPred )
  {
    m_apcPicYuvPred->destroy();
    delete m_apcPicYuvPred;
    m_apcPicYuvPred  = NULL;
  }
  
  // destroy residual picture
  if ( m_apcPicYuvResi )
  {
    m_apcPicYuvResi->destroy();
    delete m_apcPicYuvResi;
    m_apcPicYuvResi  = NULL;
  }
  
  // free lambda and QP arrays
  if ( m_pdRdPicLambda ) { xFree( m_pdRdPicLambda ); m_pdRdPicLambda = NULL; }
  if ( m_piRdPicQp     ) { xFree( m_piRdPicQp     ); m_piRdPicQp     = NULL; }
  if ( m_piRdPicChromaQp) { xFree( m_piRdPicChromaQp); m_piRdPicChromaQp= NULL; }
  if ( m_pdRdPicChromaLambda ) { xFree( m_pdRdPicChromaLambda ); m_pdRdPicChromaLambda = NULL; }
}

Void TEncPicture::init( TEncTop* pcEncTop )
{
  m_pcCfg             = pcEncTop;
  m_pcListPic         = pcEncTop->getListPic();
  
  m_pcGOPEncoder      = pcEncTop->getGOPEncoder();
  m_pcCuEncoder       = pcEncTop->getCuEncoder();
  m_pcPredSearch      = pcEncTop->getPredSearch();
  
  m_pcEntropyCoder    = pcEncTop->getEntropyCoder();
  m_pcSbacCoder       = pcEncTop->getSbacCoder();
  m_pcTrQuant         = pcEncTop->getTrQuant();
  
  m_pcBitCounter      = pcEncTop->getBitCounter();
  m_pcRdCost          = pcEncTop->getRdCost();
  m_pppcRDSbacCoder   = pcEncTop->getRDSbacCoder();
  m_pcRDGoOnSbacCoder = pcEncTop->getRDGoOnSbacCoder();
  
  // create lambda and QP arrays
  m_pdRdPicLambda     = (Double*)xMalloc( Double, 1 );
  m_piRdPicQp         = (Int*   )xMalloc( Int,    1 );
  m_piRdPicChromaQp   = (Int*   )xMalloc( Int,    1 );
  // create lambda and QP arrays
  m_pdRdPicChromaLambda     = (Double*)xMalloc( Double, 1 );
}

/**
 - non-referenced frame marking
 - QP computation based on temporal structure
 - lambda computation based on QP
 .
 \param pcPic         picture class
 \param iPOCLast      POC of last picture
 \param uiPOCCurr     current POC
 \param iNumPicRcvd   number of received pictures
 \param iTimeOffset   POC offset for hierarchical structure
 \param iDepth        temporal layer depth
 \param rpcPicture      picture header class
 */

#if RPS
Void TEncPicture::initEncPicture(TComPic* pcPic, Int iPOCLast, UInt uiPOCCurr, Int iNumPicRcvd, Int iTimeOffset, Int iGOPid, TComPicture*& rpcPicture, TComPicHeader*& pcPicHeader, TComSPS* pcSeqHeader)
#else
#if AVS3_PIC_HEADER_ZL
Void TEncPicture::initEncPicture(TComPic* pcPic, Int iPOCLast, UInt uiPOCCurr, Int iNumPicRcvd, Int iTimeOffset, Int iDepth, TComPicture*& rpcPicture, TComPicHeader*& pcPicHeader, TComSPS* pcSeqHeader)
#else
Void TEncPicture::initEncPicture(TComPic* pcPic, Int iPOCLast, UInt uiPOCCurr, Int iNumPicRcvd, Int iTimeOffset, Int iDepth, TComPicture*& rpcPicture)
#endif
#endif
{
#if !LAMBDA
  Double dQP, dCQP;
  Double dLambda;
  //Int    iChromaQp;
  Double dChromaLambda;
#endif
#if RPS
  Int iDepth = 0;
#endif
  rpcPicture = pcPic->getPicture();
  rpcPicture->setPic( pcPic );
  rpcPicture->initPicture();

#if AVS3_PIC_HEADER_ZL
  pcPicHeader=pcPic->getPicture()->getPic()->getPicHeader();
  pcPicHeader->initPicHeader();
  pcPicHeader->setSeqHeader(pcSeqHeader);

  ///20170214修改要编辑的header信息，然后输出
  ////已在构造函数中初始化
  //pcPicHeader->setIntraPicStartCode(0x1B3);
  //pcPicHeader->setInterPicStartCode(0x1B6);
  //pcPicHeader->setBbvDelay(0xFFFF);
  //pcPicHeader->setTimeCodeFlag(false);
  //if (pcPicHeader->getTimeCodeFlag())
  //{
  //  //pcPicHeader->setTimeCode(0);      ///< RD中TimeCodeFlag默认为0,如果不为0 ，需要计算TimeCode
  //}

  if (pcSeqHeader->getScenePictureDisableFlag())
  {
    pcPicHeader->setScenePicFlag(true);
    pcPicHeader->setScenePredFlag(true);
  }
  else
  {
    pcPicHeader->setScenePicFlag(false);
    pcPicHeader->setScenePredFlag(false);
  }
  //背景帧默认关掉
  //pcPicHeader->setScenePicOutputFlag();
  // pcPicHeader->setSceneRefEnableFlag();

  if (m_pcCfg->getIntraPeriod() ==1)
  {
    pcPicHeader->setUseRcsFlag(true);
  }
  else
  {
    pcPicHeader->setUseRcsFlag(false);
  }

  if ( m_pcCfg->getInterlaceCodingOption() == 0 )
  {
    if (m_pcCfg->getProgressiveFrame())
    {
      pcPicHeader->setProgressiveFrame(true);
      pcPicHeader->setPicStructure(PS_BLEND);
    }
    else
    {
      pcPicHeader->setProgressiveFrame(false);
      pcPicHeader->setPicStructure(PS_BLEND);
    }
  }
  else if ( m_pcCfg->getInterlaceCodingOption() == 3 )
  {
    pcPicHeader->setProgressiveFrame(false);
    pcPicHeader->setPicStructure(PS_SUCCESSIVELY);
  }

  (m_pcCfg->getTopFieldFirst() == 0) ? pcPicHeader->setTopFieldFirst(false) : pcPicHeader->setTopFieldFirst(true);
  (m_pcCfg->getRepeatFirstField() == 0) ? pcPicHeader->setRepeatFirstField( false ) : pcPicHeader->setTopFieldFirst(true);

  (m_pcCfg->getDeltaQP() == 0) ? pcPicHeader->setFixedPicQP(true) : pcPicHeader->setFixedPicQP(false);

  //背景帧默认关掉，所以不需要set GB frame's qp
  pcPicHeader->setPicQP(Clip3(0, 63 + 8 * (m_pcCfg->getBitDepth() - 8),m_pcCfg->getQPIFrame()));

  pcPicHeader->setLoopFilterDisableFlag(m_pcCfg->getLoopFilterDisable());
  pcPicHeader->getLoopFilterSyntax()->m_bLoppFilterParamFlag = m_pcCfg->getLoopFilterParameter();
  pcPicHeader->getLoopFilterSyntax()->m_iAlphaCOffset        = m_pcCfg->getLoopFilterAlphaOffset();
  pcPicHeader->getLoopFilterSyntax()->m_iBetaOffset          = m_pcCfg->getLoopFilterBetaOffset();

  pcPicHeader->setChromaQuantParamDisableFlag(m_pcCfg->getChromaDeltaQPDisable());
  pcPicHeader->setChromaQuantParamDeltaCb(m_pcCfg->getChromaDeltaU());
  pcPicHeader->setChromaQuantParamDeltaCr(m_pcCfg->getChromaDeltaV());

  pcPicHeader->setPicWeightQuantEnableFlag(m_pcCfg->getPicWQEnable());
  pcPicHeader->getWeightQuantSyntax()->m_uiPicWeightQuantDataIndex = m_pcCfg->getPicWQDataIndex();
  pcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantParamIndex   = m_pcCfg->getWQParam();
  pcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantMode         = m_pcCfg->getWQModel();

  //WQparamDefault默认值还未设置，需要赋给m_iWeightQuantParamDelta1、2

  //WeightQuantCoeff4x4,8x8已经在构造函数初始化，设为0   

  //RD中ALF flag需要从alf过程中得出,初始值为false，构造函数中初始化
#endif
#if RPS
  pcPicHeader->setIntraPeriod(m_pcCfg->getIntraPeriod());
#endif
#if AVS3_SLICE_HEADER_SYC
  //set avs slice header
#if YQH_AVS2_BITSTREAM
  rpcPicture->setSliceStartCode(0x100);//yuquanhe
#else
  rpcPicture->setSliceStartCode(0);
#endif
  rpcPicture->setSliceVerPosExtension(0);
  rpcPicture->setSliceHorPos(0);
  rpcPicture->setSliceHorPosExtension(0);
  rpcPicture->setFixedSliceQP(rpcPicture->getPic()->getPicHeader()->getFixedPicQP());
  rpcPicture->setSliceQP(rpcPicture->getPic()->getPicHeader()->getPicQP());
#endif
  
  // depth re-computation based on rate GOP size
  if ( m_pcCfg->getGOPSize() != m_pcCfg->getRateGOPSize() )
  {
    Int i, j;
    Int iPOC = rpcPicture->getPOC()%m_pcCfg->getRateGOPSize();
    if ( iPOC == 0 ) iDepth = 0;
    else
    {
#if RPS
      Int iStep = m_pcCfg->getGOPSize();
      iDepth = 0;
      for (i = iStep >> 1; i >= 1; i >>= 1)
      {
        for (j = i; j<m_pcCfg->getGOPSize(); j += iStep)
        {
          if (j == iPOC)
          {
            i = 0;
            break;
          }
        }
        iStep >>= 1;
        iDepth++;
      }
#else
      Int iStep = m_pcCfg->getRateGOPSize();
      iDepth = 0;
      for (i = iStep >> 1; i >= 1; i >>= 1)
      {

        for (j = i; j<m_pcCfg->getRateGOPSize(); j += iStep)
        {
          if (j == iPOC)
          {
            i = 0;
            break;
          }
        }
        iStep >>= 1;
        iDepth++;
      }
#endif
    }
  }
  
  // picture type
  PictureType ePictureType;
  
#if !HB_LAMBDA_FOR_LDC
  if ( m_pcCfg->getUseLDC() )
  {
    ePictureType = P_PICTURE;
  }
  else
#endif
#if RPS
  if (m_pcCfg->getGOPEntry(iGOPid).m_iSliceType == 'P')
  {
    ePictureType = P_PICTURE;
  }
  else if (m_pcCfg->getGOPEntry(iGOPid).m_iSliceType == 'B')
  {
    ePictureType = B_PICTURE;
  }
#if F_RPS
  if (m_pcCfg->getGOPEntry(iGOPid).m_iSliceType == 'F')
  {
	  ePictureType = F_PICTURE;
  }
#endif

#else
  {
    ePictureType = iDepth > 0 ? B_PICTURE : P_PICTURE;
  }
#endif
#if RPS_BUG
#if WLQ_RPS_debug
#if B_RPS_BUG_821 
  ePictureType = (iPOCLast == 0 || (m_pcCfg->getIntraPeriod() != 0 && uiPOCCurr % m_pcCfg->getIntraPeriod() == 0) || m_pcGOPEncoder->getGOPSize() == 1) ? I_PICTURE : ePictureType;
#else
  ePictureType = (iPOCLast == 0 || m_pcGOPEncoder->getGOPSize() == 1) ? I_PICTURE : ePictureType;
  // ePictureType = (iPOCLast == 0 || (m_pcCfg->getIntraPeriod() != 0 && uiPOCCurr % m_pcCfg->getIntraPeriod() == 0) || m_pcGOPEncoder->getGOPSize() == 1) ? I_PICTURE : ePictureType;
#endif

#else
  ePictureType = (iPOCLast == 0 || m_pcGOPEncoder->getGOPSize() == 0) ? I_PICTURE : ePictureType;
#endif
#if F_RPS
  if (m_pcCfg->getFFrameEnable() && ePictureType == P_PICTURE)
  {
	  ePictureType = F_PICTURE;
  }
  iDepth = (ePictureType == P_PICTURE || ePictureType == F_PICTURE) ? 0 : iDepth;
#else
  iDepth = (ePictureType == P_PICTURE) ? 0 : iDepth;
#endif
#else
  ePictureType = (iPOCLast == 0 || uiPOCCurr % m_pcCfg->getIntraPeriod() == 0 || m_pcGOPEncoder->getGOPSize() == 0) ? I_PICTURE : ePictureType;
#endif 
  rpcPicture->setPOC(iPOCLast - iNumPicRcvd + iTimeOffset);
  rpcPicture->setPictureType    ( ePictureType );

#if RPS
  pcPicHeader->setPictureType(ePictureType);
  pcPicHeader->setPOC(iPOCLast - iNumPicRcvd + iTimeOffset);
  pcPicHeader->setGOPSize(m_pcCfg->getGOPSize());
#endif

#if LAMBDA_BUG
  pcSeqHeader->setPictureCodingOrder(pcSeqHeader->getPictureCodingOrder() + 1);   //hc->coding_order ..rd
#endif

  // ------------------------------------------------------------------------------------------------------------------
  // Non-referenced frame marking
  // ------------------------------------------------------------------------------------------------------------------
#if RPS
  if ( m_pcCfg->getUseNRF() )
  {
    if ( ( m_pcCfg->getRateGOPSize() != 1) && (m_pcCfg->getRateGOPSize() >> (iDepth+1)) == 0 )
    {
      rpcPicture->setReferenced(false);
    }
    else
    {
      rpcPicture->setReferenced(true);
    }
  }
  else
  {
    rpcPicture->setReferenced(true);
  }
#endif
  // ------------------------------------------------------------------------------------------------------------------
  // QP setting
  // ------------------------------------------------------------------------------------------------------------------

#if LAMBDA
  Int qp;                        //  == Clip3(0, MAX_QP, img->qp);  MAX_QP = 63;
  Int iChromaQP;
  Double temp_qp;
  Double dLambda;
  Int RPS_Idx;
#if wlq_WQ
  Bool WeightQuantEnable = (getCfg()->getWQEnable() && getCfg()->getPicWQEnable()) ? true : false;
  int     lambdaQP;     //M2331 2008-04
  double  lambdaF;      //M2331 2008-04
  if (WeightQuantEnable)
  {
	  if (pcPicHeader->getPictureType() == F_PICTURE)
	  {
		  lambdaQP = g_LambdaQPTab[P_PICTURE][WQ_MODE_D_AVS2];
		  lambdaF = g_LambdaFTab[P_PICTURE][WQ_MODE_D_AVS2];
	  }
	  else
	  {
#if wlq_WQ_q    //img->type = INTER_IMG, img->type = INTRA_IMG
		  PictureType PictureType = pcPicHeader->getPictureType();
#endif
		  lambdaQP = g_LambdaQPTab[PictureType][WQ_MODE_D_AVS2];
		  lambdaF = g_LambdaFTab[PictureType][WQ_MODE_D_AVS2];
	  }
  }
#endif
  if (rpcPicture->getPictureType() == I_PICTURE)
  {
    qp = m_pcCfg->getQP();
  }
  else
  {
    qp = m_pcCfg->getQP() + m_pcCfg->getGOPEntry(iGOPid).m_iQPOffset;
  }
  qp = Clip3(0, 63, qp);
  iChromaQP = QP_SCALE_CR[qp]; // 
  ////lambda
#if wlq_WQ
  if (WeightQuantEnable)
		temp_qp = (Double)qp - SHIFT_QP + lambdaQP;
  else
		temp_qp = (Double)qp - SHIFT_QP;
#else
  temp_qp = (Double)qp - SHIFT_QP;
#endif
  if (m_pcCfg->getIntraPeriod() == 1)       //ALL INTRA
  {
    dLambda = 0.85 * pow(2, temp_qp / 4.0) *  LAM_2Level_TU;
  }
  else
  {
#if wlq_WQ
	  if(WeightQuantEnable)
			dLambda = lambdaF * pow(2, temp_qp / 4.0) * (pcPicHeader->getPictureType() == B_PICTURE ? 1.2 : 0.8);
	  else
			dLambda = 0.68 * pow(2, temp_qp / 4.0) * (pcPicHeader->getPictureType() == B_PICTURE ? 1.2 : 0.8);
#else
    dLambda = 0.68 * pow(2, temp_qp / 4.0) * (pcPicHeader->getPictureType() == B_PICTURE ? 1.2 : 0.8);
#endif
  }
#if LAMBDA_BUG
  RPS_Idx = (pcSeqHeader->getPictureCodingOrder() - 1) % m_pcCfg->getGOPSize();
  if (m_pcCfg->getNumberBFrames() > 0)   //RA结构式，且存在B帧
#else

  RPS_Idx = (pcPicHeader->getPOC() - 1) % m_pcCfg->getGOPSize();
  if (pcPicHeader->getPictureType() == B_PICTURE)
#endif
  {
    if (pcPicHeader->getPictureType() != I_PICTURE && RPS_Idx != 0)
    {
      dLambda *= max(2.00, min(4.00, temp_qp / 8.0));
    }
    else if (pcPicHeader->getPictureType() == P_PICTURE || pcPicHeader->getPictureType() == F_PICTURE)
    {
      dLambda *= 1.25;
    }
  }
  else
  {
    if ((RPS_Idx + 1) % m_pcCfg->getGOPSize() != 0)
    {
      dLambda *= max(2.00, min(4.00, temp_qp / 8.0)) * 0.8;
    }
	#if YQH_INTRA_LAMDA_DUG
	if (rpcPicture->getPOC() == 8)
	{
		dLambda *= 1.25;
	}
#endif
  }
//#if INTERTEST
//  if (pcPicHeader->getPictureType() != I_PICTURE)
//  {
//    dLambda *= 0.3;
//  }
//#endif
#if 0//print_rd
  printf("\n dLambda'= %f\n", dLambda);
#endif
  m_pcRdCost->setLambda(dLambda);
  m_pcTrQuant->setLambda(dLambda);
  rpcPicture->setLambda(dLambda);
  m_pcTrQuant->setChromaLambda(dLambda);
  m_pcRdCost->setChromaWeight(dLambda / dLambda);
#else
#if H_QP
  Int iQuantParameter;
  Int iChromaQP;
  if (rpcPicture->getPictureType() != I_PICTURE)
  {
    iQuantParameter = m_pcCfg->getQP() + m_pcCfg->getGOPEntry(iGOPid).m_iQPOffset -3;
    iChromaQP = m_pcCfg->getQP() + m_pcCfg->getGOPEntry(iGOPid).m_iQPOffset - 5;
  }
  else
  {
    iQuantParameter = m_pcCfg->getQP();
    iChromaQP = m_pcCfg->getCQP();
  }
#else
  Int iQuantParameter = m_pcCfg->getQP();
  Int iChromaQP = m_pcCfg->getCQP();
#endif

  //new model
  Double dQstep = log((double)g_uiQuantStep[iQuantParameter]) / log(2.0);
  Double dChromaQstep = log((double)g_uiQuantStep[iChromaQP]) / log(2.0);
  dQP = (double)(6 * (dQstep - 5.3327));
  dCQP = (double)(6 * (dChromaQstep - 5.3327));

  if ((iPOCLast != 0) && ((uiPOCCurr % m_pcCfg->getIntraPeriod()) != 0) && (m_pcGOPEncoder->getGOPSize() != 0)) // P or B-slice
  {
    if (m_pcCfg->getUseLDC() && !m_pcCfg->getUseBQP())
    {
      if (iDepth == 0) { dQP += 1.0; dCQP += 1.0; }
      else
      {
        dQP += iDepth + 3; dCQP += iDepth + 3;
      }
    }
    else
    {
      dQP += iDepth + 1; dCQP += iDepth + 1;
    }
  }

  // modify QP
  Int* pdQPs = m_pcCfg->getdQPs();
  if (pdQPs){ dQP += pdQPs[pcPic->getPicture()->getPOC()]; dCQP += pdQPs[pcPic->getPicture()->getPOC()]; }

  // ------------------------------------------------------------------------------------------------------------------
  // Lambda computation
  // ------------------------------------------------------------------------------------------------------------------

  Int iQP, iCQP;
  Double dOrigQP = dQP;
  Double dCOrigQP = dCQP;

  // pre-compute lambda and QP values for all possible QP candidates
  for (Int iDQpIdx = 0; iDQpIdx < 1; iDQpIdx++)
  {
    // compute QP value
    dQP = dOrigQP + ((iDQpIdx + 1) >> 1)*(iDQpIdx % 2 ? -1 : 1);
    dCQP = dCOrigQP + ((iDQpIdx + 1) >> 1)*(iDQpIdx % 2 ? -1 : 1);

    // compute lambda value
    Int    NumberBFrames = (m_pcCfg->getRateGOPSize() - 1);
    Double dLambda_scale = 1.0 - Clip3(0.0, 0.5, 0.05*(Double)NumberBFrames);

    Double qp_temp = dQP - 12;
    Double qp_chromatemp = dCQP - 12;

    Double dChromaLambda_scale = dLambda_scale;

    // Case #1: I or P-pictures (key-frame)
    if (iDepth == 0)
    {
      if (m_pcCfg->getUseRDOQ() && pcPic->getPicture()->isIntra() && dQP == dOrigQP)
      {
        dLambda = 0.57 * pow(2.0, qp_temp / 3.0);
        dChromaLambda = 0.57 * pow(2.0, qp_chromatemp / 3.0);
      }
      else
      {
        if (NumberBFrames > 0) // HB structure or HP structure
        {
          dLambda = 0.68 * pow(2.0, qp_temp / 3.0);
          dChromaLambda = 0.68 * pow(2.0, qp_chromatemp / 3.0);
        }
        else                     // IPP structure
        {
          dLambda = 0.85 * pow(2.0, qp_temp / 3.0);
          dChromaLambda = 0.85 * pow(2.0, qp_chromatemp / 3.0);
        }
      }
      dLambda *= dLambda_scale;
      dChromaLambda *= dChromaLambda_scale;
    }
    else // P or B pictures for HB or HP structure
    {
      dLambda = 0.68 * pow(2.0, qp_temp / 3.0);
      dChromaLambda = 0.68 * pow(2.0, qp_chromatemp / 3.0);
      if (pcPic->getPicture()->isInterB())
      {
        dLambda *= Clip3(2.00, 4.00, (qp_temp / 6.0)); // (j == B_SLICE && p_cur_frm->layer != 0 )
        dChromaLambda *= Clip3(2.00, 4.00, (qp_chromatemp / 6.0)); // (j == B_SLICE && p_cur_frm->layer != 0 )
        if (rpcPicture->isReferenced()) // HB structure and referenced
        {
          dLambda *= 0.80;
          dLambda *= dLambda_scale;
          dChromaLambda *= 0.80;
          dChromaLambda *= dChromaLambda_scale;
        }
      }
      else
      {
        dLambda *= dLambda_scale;
        dChromaLambda *= dChromaLambda_scale;
      }
    }
    // if hadamard is used in ME process
    if (!m_pcCfg->getUseHADME())
    {
      dLambda *= 0.95;
      dChromaLambda *= 0.95;
    }
#if YQH_INTRA_LAMDA
	dLambda=0.85 * pow(2, (iQuantParameter - 11) / 4.0) *  0.8;
	dChromaLambda = dLambda;
#endif
    // reverse mapping from AVC model to new quantizer

    Double dQstep_temp = (dQP / 6.0) + 5.3327;
    dQstep_temp = pow(2.0, dQstep_temp);

    Double dCQstep_temp = (dCQP / 6.0) + 5.3327;
    dCQstep_temp = pow(2.0, dCQstep_temp);

    Int idx;
    for (idx = 0; idx<128; idx++)
    {
      if ((double)g_uiQuantStep[idx] > dQstep_temp)
      {
        break;
      }
    }
    iQP = idx - 1;

    for (idx = 0; idx<128; idx++)
    {
      if ((double)g_uiQuantStep[idx] > dCQstep_temp)
      {
        break;
      }
    }
    iCQP = idx - 1;

    m_pdRdPicLambda[iDQpIdx] = dLambda;
    m_piRdPicQp[iDQpIdx] = iQP;
    m_piRdPicChromaQp[iDQpIdx] = iCQP;
    m_pdRdPicChromaLambda[iDQpIdx] = dChromaLambda;
  }

  // obtain dQP = 0 case
  dLambda = m_pdRdPicLambda[0];
  iQP = m_piRdPicQp[0];
  iCQP = m_piRdPicChromaQp[0];
  dChromaLambda = m_pdRdPicChromaLambda[0];

  // store lambda
  m_pcRdCost->setLambda(dLambda);
  m_pcTrQuant->setLambda(dLambda);
  rpcPicture->setLambda(dLambda);
  m_pcTrQuant->setChromaLambda(dChromaLambda);
  m_pcRdCost->setChromaWeight(dLambda / dChromaLambda);
#endif
#if RPS
  pcPicHeader->setNumRefIdx(REF_PIC_0, m_pcCfg->getGOPEntry(iGOPid).m_iNumberOfNegativeRefPics); //重写cfg的entryGOP NumberOfNegativeRefPictures
  pcPicHeader->setNumRefIdx(REF_PIC_1, m_pcCfg->getGOPEntry(iGOPid).m_iNumberOfPositiveRefPics);
#endif
#if HB_LAMBDA_FOR_LDC
  // restore original picture type
  if ( m_pcCfg->getUseLDC() )
  {
#if inter_direct_skip_bug3
	  ePictureType = m_pcCfg->getFFrameEnable()==true ? F_PICTURE : P_PICTURE;
#else
    ePictureType = P_PICTURE;
#endif
  }
#if RPS
#if WLQ_RPS_debug
#if B_RPS_BUG_821 
  ePictureType = (iPOCLast == 0 || (m_pcCfg->getIntraPeriod() != 0 && uiPOCCurr % m_pcCfg->getIntraPeriod() == 0) || m_pcGOPEncoder->getGOPSize() == 1) ? I_PICTURE : ePictureType;
#else
  ePictureType = (iPOCLast == 0 || m_pcGOPEncoder->getGOPSize() == 1) ? I_PICTURE : ePictureType;
  // ePictureType = (iPOCLast == 0 || (m_pcCfg->getIntraPeriod() != 0 && uiPOCCurr % m_pcCfg->getIntraPeriod() == 0) || m_pcGOPEncoder->getGOPSize() == 1) ? I_PICTURE : ePictureType;
#endif
#else
  ePictureType = (iPOCLast == 0 || m_pcGOPEncoder->getGOPSize() == 0) ? I_PICTURE : ePictureType;
#endif
  pcPicHeader->setPictureType(ePictureType);
#else
  ePictureType = (iPOCLast == 0 || uiPOCCurr % m_pcCfg->getIntraPeriod() == 0 || m_pcGOPEncoder->getGOPSize() == 0) ? I_PICTURE : ePictureType;

#endif 
  rpcPicture->setPictureType        ( ePictureType );
#endif
  
 #if LAMBDA
  rpcPicture->setPictureQp(qp, iChromaQP);
#else
  rpcPicture->setPictureQp(iQP, iCQP);
#endif
  
  rpcPicture->setLoopFilterDisable( m_pcCfg->getLoopFilterDisable() );
  
  rpcPicture->setDepth            ( iDepth );
  
  // reference picture usage indicator for next frames
  rpcPicture->setDRBFlag          ( true );
  
  // generalized B info. (for non-reference B)
  if ( m_pcCfg->getHierarchicalCoding() == false && iDepth != 0 )
  {
    rpcPicture->setDRBFlag        ( false );
  }
  
  assert( m_apcPicYuvPred );
  assert( m_apcPicYuvResi );
  
  pcPic->setPicYuvPred( m_apcPicYuvPred );
  pcPic->setPicYuvResi( m_apcPicYuvResi );
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncPicture::setSearchRange( TComPicture* pcPicture )
{
  Int iCurrPOC = pcPicture->getPOC();
  Int iRefPOC;
  Int iRateGOPSize = m_pcCfg->getRateGOPSize();
  Int iOffset = (iRateGOPSize >> 1);
  Int iMaxSR = m_pcCfg->getSearchRange();
#if F_RPS
  Int iNumPredDir = (pcPicture->isInterP() || pcPicture->isInterF()) ? 1 : 2;
#else
  Int iNumPredDir = pcPicture->isInterP() ? 1 : 2;
#endif

  for (Int iDir = 0; iDir <= iNumPredDir; iDir++)
  {
#if RPS
    RefPic e = (RefPic)iDir;
    for (Int iNumRefIdx = 0; iNumRefIdx < pcPicture->getPicHeader()->getNumRefIdx(e); iNumRefIdx++)
    {
      iRefPOC = pcPicture->getPicHeader()->getRefPic(e, iNumRefIdx)->getPOC();
      Int iNewSR = Clip3(8, iMaxSR, (iMaxSR*ADAPT_SR_SCALE*abs(iCurrPOC - iRefPOC) + iOffset) / iRateGOPSize);
      m_pcPredSearch->setAdaptiveSearchRange(iDir, iNewSR, iNumRefIdx);
    }
#else
    RefPic e = (RefPic)iDir;
    {
      iRefPOC = pcPicture->getRefPic(e)->getPOC();
      Int iNewSR = Clip3(8, iMaxSR, (iMaxSR*ADAPT_SR_SCALE*abs(iCurrPOC - iRefPOC) + iOffset) / iRateGOPSize);
      m_pcPredSearch->setAdaptiveSearchRange(iDir, iNewSR);
    }
#endif
  }
}

/** \param rpcPic   picture class
 */
Void TEncPicture::compressPicture( TComPic*& rpcPic )
{
  UInt  uiCUAddr;
  
  // initialize cost values
  m_uiPicTotalBits  = 0;
  m_dPicRdCost      = 0;
  m_uiPicDist       = 0;
  
  // set entropy coder
#if AVS3_PIC_HEADER_ZL
  m_pcEntropyCoder->setEntropyCoder   ( m_pcSbacCoder, rpcPic->getPicture(), rpcPic->getPicHeader() );
#else
  m_pcEntropyCoder->setEntropyCoder   ( m_pcSbacCoder, rpcPic->getPicture() );
#endif
  m_pcEntropyCoder->resetEntropy      ();

  m_pppcRDSbacCoder[0][CI_CURR_BEST]->load(m_pcSbacCoder);

  rpcPic->getPicYuvRec()->reset();

#if WLQ_CUSplitFlag
  m_pcRDGoOnSbacCoder->resetEntropy();
#endif

  // set QP
  m_pcCuEncoder->setQp( rpcPic->getPicture()->getPictureQp(), rpcPic->getPicture()->getPictureChromaQp() );
#if YQH_RDO_BITOURNT_BUG   //yuquanhe@hisilicon.com 20170525
  m_pcEntropyCoder->setBitstream(m_pcBitCounter);
  m_pcEntropyCoder->resetBits();
  m_pcEntropyCoder->setEntropyCoder(m_pcRDGoOnSbacCoder, rpcPic->getPicture(), rpcPic->getPicHeader());
#endif
  // for every CU
  for( uiCUAddr = 0; uiCUAddr < rpcPic->getPicSym()->getNumberOfCUsInFrame() ; uiCUAddr++ )
  {
    //printf("#####################################################33333");
    // initialize CU encoder
    TComDataCU*& pcCU = rpcPic->getCU( uiCUAddr );
    pcCU->initCU( rpcPic, uiCUAddr );

    // set go-on entropy coder
#if AVS3_PIC_HEADER_ZL
    m_pcEntropyCoder->setEntropyCoder ( m_pcRDGoOnSbacCoder, rpcPic->getPicture(), rpcPic->getPicHeader() );
#else
    m_pcEntropyCoder->setEntropyCoder ( m_pcRDGoOnSbacCoder, rpcPic->getPicture() );
#endif

    m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
#if WLQ_CUSplitFlag
	m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[0][CI_COPY]);
#endif
    // run CU encoder
	//test
	UInt cnt = pcCU->getAddr();
	UChar flag = pcCU->getPicture()->isIntra();
    m_pcCuEncoder->compressCU( pcCU ); //LCU

	//test   print coeffY
	/*if (!pcCU->isIntra(0))
	{
		Pel *coe = pcCU->getCoeffY() + 16 * 48;
		for (Int m = 0; m < 4; m++)
		{
			for (Int n = 0; n < 16; n++)
			{
				printf("%d ", coe[n]);
			}
			coe = coe + 16;
			printf("\n");
		}
	}*/

#if DEBUG_ME
		//printf("\n###################################################################################\n");
#endif
    // restore entropy coder to an initial stage
#if AVS3_PIC_HEADER_ZL
#if WLQ_CUSplitFlag
	m_pcEntropyCoder->setEntropyCoder ( m_pppcRDSbacCoder[0][CI_COPY], rpcPic->getPicture(), rpcPic->getPicHeader() );
#else
    m_pcEntropyCoder->setEntropyCoder ( m_pppcRDSbacCoder[0][CI_CURR_BEST], rpcPic->getPicture(), rpcPic->getPicHeader() );
#endif
#else
    m_pcEntropyCoder->setEntropyCoder ( m_pppcRDSbacCoder[0][CI_CURR_BEST], rpcPic->getPicture() );
#endif
    m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
      
    m_pcCuEncoder->encodeCU( pcCU );
#if WLQ_CUSplitFlag
	m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[0][CI_COPY]);
#endif
    m_uiPicTotalBits += pcCU->getTotalBits();
    m_dPicRdCost     += pcCU->getTotalCost();
    m_uiPicDist      += pcCU->getTotalDistortion();
  }
}

/**
 \param  rpcPic        picture class
 \retval rpcBitstream  bitstream class
 */
Void TEncPicture::encodePicture   ( TComPic*& rpcPic, TComBitstream*& rpcBitstream )
{
  UInt        uiCUAddr;
  TComPicture*  pcPicture = rpcPic->getPicture();
#if AVS3_PIC_HEADER_ZL
  TComPicHeader*  pcPicHeader = rpcPic->getPicHeader();
  m_pcEntropyCoder->setEntropyCoder ( m_pcSbacCoder, pcPicture, pcPicHeader );
#else
  m_pcEntropyCoder->setEntropyCoder ( m_pcSbacCoder, pcPicture );
#endif
  
  // set bitstream
  m_pcEntropyCoder->setBitstream( rpcBitstream );

  // set QP
  m_pcCuEncoder->setQp( rpcPic->getPicture()->getPictureQp(), rpcPic->getPicture()->getPictureType() );

  // for every CU
  for( uiCUAddr = 0; uiCUAddr < rpcPic->getPicSym()->getNumberOfCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU*& pcCU = rpcPic->getCU( uiCUAddr );
//#if INTERTEST
//    if (rpcPic->getPicture()->getPictureType() == P_PICTURE &&rpcPic->getPicHeader()->getMVtestEnable())
//    {
      //printf("\n#####################################################################################################\n");
//    }
//#endif
    // encode CU
    m_pcCuEncoder->encodeCU( pcCU );
  }
}
