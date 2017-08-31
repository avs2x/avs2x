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

/** \file     TDecSbac.cpp
    \brief    Context-adaptive entropy decoder class
*/

#include "TDecSbac.h"

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TDecSbac::TDecSbac() 
// new structure here
: m_pcBitstream               ( 0 )
, m_cCUSplitFlagSCModel       ( 1,             1,               NUM_SPLIT_FLAG_CTX            )
, m_cCUPredModeSCModel        ( 1,             1,               NUM_PRED_MODE_CTX             )
#if INTER_GROUP
, m_cCUShapeOfPartitionSCModel(1, 1, NUM_SHAPE_OF_PARTITION_CTX)
#endif
, m_cCUPartSizeSCModel        ( 1,             1,               NUM_PART_SIZE_CTX             )
, m_cCUInterDirSCModel        ( 1,             1,               NUM_INTER_DIR_CTX             )
#if WRITE_INTERDIR
, m_cCUMinInterDirSCModel     (1,               1,              NUM_MIN_INTER_DIR_CTX         )
#endif
#if inter_direct_skip_bug1
, m_cCUInterWSMSCModel        (1,              1,               NUM_INTER_WSM_CTX             )
#endif
#if F_MHPSKIP_SYC
, m_cCUInterMHPSKIPSCModel   (1,               1,               NUM_INTER_MHPSKIP_CTX         )
#endif
#if B_MHBSKIP_SYC
, m_cCUInterMHBSKIPSCModel   (1,               1,               NUM_INTER_MHPSKIP_CTX         )
#endif
#if F_DHP_SYC
, m_cCUInterDHPSCModel        (1,              1,               NUM_INTER_DHP_CTX             )
, m_cCUInterDHPNXNSCModel     (1,              1,               NUM_INTER_DHP_NXN_CTX         )
#endif
#if YQH_INTRA
, m_cCUIntraDirSCModel        ( 1,             1,               NUM_INTRA_DIR_CTX               )//yuquanhe@hisilicon.com
#else
, m_cCUIntraDirSCModel        ( 1,             1,               1             )
#endif
#if RPS
, m_cCURefPicSCModel(1, 1, NUM_REF_NO_CTX)
#endif
#if DMH
, m_cCUDMHSCModel(1, 1, NUM_DMH_MODE_CTX)
#endif
, m_cCUMvdSCModel             ( 1,             2,               NUM_MV_RES_CTX                )
, m_cCUQtRootCbfSCModel       ( 1,             1,               NUM_QT_ROOT_CBF_CTX           )
, m_cCUQtCbfSCModel           ( 1,             3,               NUM_QT_CBF_CTX                )
, m_cCUTransIdxSCModel        ( 1,             1,               1                             )
, m_cCURunSCModel             ( 1,             2,               NUM_RUN_CTX                   )
, m_cCULastSCModel            ( 1,             2,               NUM_LAST_FLAG_CTX             )
, m_cCUAbsSCModel             ( 1,             2,               NUM_ABS_CTX                   )
, m_cDBKIdxSCModel            ( 1,             1,               NUM_DBK_IDX_CTX               )
, m_cCUXPosiSCModel           ( 1,						 1,								NUM_CU_X_POS_CTX      )
, m_cCUYPosiSCModel           ( 1,						 1,								NUM_CU_Y_POS_CTX      )
#if YQH_INTRA
, m_cTUSplitFlagSCModel       ( 1,             1,               NUM_TRANSSPLIT_FLAG_CTX          )//yuquanhe@hisilicon.com
, m_cPUIntraTypeSCModel       ( 1,             1,               NUM_TNTRA_PU_TYPE_CTX            )//yuquanhe@hisilicon.com
#endif

#if CODEFFCODER_LDW_WLQ_YQH

, m_cTULastCGSCModel          ( 1,             2,               NUM_LAST_CG_CTX               ) //yuquanhe@hisilicon.com
, m_cTUSigCGSCModel           ( 1,             2,               NUM_SIGCG_CTX                 ) //yuquanhe@hisilicon.com
, m_cTULastPosInCGSCModel     ( 1,             2,               NUM_LAST_POS_INCG_CTX         ) //yuquanhe@hisilicon.com
, m_cTULevelRunSCModel        ( 1,             2,               NUM_LEVEL_RUN_INCG_CTX         ) //yuquanhe@hisilicon.com
#endif

{
}

TDecSbac::~TDecSbac()
{
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TDecSbac::resetEntropy          (TComPicture* pcPicture)
{
  // Initilization of the context models to equi-probable state    
  m_cCUSplitFlagSCModel.initBuffer       ();
  m_cCUPredModeSCModel.initBuffer        ();
  m_cCUPartSizeSCModel.initBuffer        ();
#if INTER_GROUP
	m_cCUShapeOfPartitionSCModel.initBuffer();
#endif
#if WRITE_INTERDIR
	m_cCUMinInterDirSCModel.initBuffer();
#endif
#if F_DHP_SYC
	m_cCUInterDHPSCModel.initBuffer();
	m_cCUInterDHPNXNSCModel.initBuffer();
#endif
#if F_MHPSKIP_SYC
	m_cCUInterMHPSKIPSCModel.initBuffer();
#endif
#if B_MHBSKIP_SYC
	m_cCUInterMHBSKIPSCModel.initBuffer();
#endif
	m_cCUInterDirSCModel.initBuffer        ();
#if inter_direct_skip_bug1
	m_cCUInterWSMSCModel.initBuffer();
#endif
  m_cCUIntraDirSCModel.initBuffer        ();
#if RPS
  m_cCURefPicSCModel.initBuffer();
#endif
#if DMH
	m_cCUDMHSCModel.initBuffer();
#endif
  m_cCUMvdSCModel.initBuffer             ();
  m_cCUQtCbfSCModel.initBuffer           ();
  m_cCUQtRootCbfSCModel.initBuffer       ();
  m_cCUTransIdxSCModel.initBuffer        ();
  m_cCURunSCModel.initBuffer             ();
  m_cCULastSCModel.initBuffer            ();
  m_cCUAbsSCModel.initBuffer             ();
  m_cDBKIdxSCModel.initBuffer            ();
  m_cCUXPosiSCModel.initBuffer           ();
  m_cCUYPosiSCModel.initBuffer           ();
  m_pcBitstream->setModeSbac();
#if CODEFFCODER_LDW_WLQ_YQH
  m_cTULastCGSCModel.initBuffer           ();//yuquanhe@hisilicon.com
  m_cTUSigCGSCModel.initBuffer           ();//yuquanhe@hisilicon.com
  m_cTULastPosInCGSCModel.initBuffer           ();//yuquanhe@hisilicon.com
  m_cTULevelRunSCModel.initBuffer           ();//yuquanhe@hisilicon.com
#endif
#if YQH_INTRA
  m_cTUSplitFlagSCModel.initBuffer       ();//yuquanhe@hisilicon.com
  m_cPUIntraTypeSCModel.initBuffer       ();//yuquanhe@hisilicon.com
#endif
#if wlq_AEC
  {
	  xReadBit();
  }
#else
  m_uiRange    = 0x10000;
  m_uiValue    = 0;
  for( UInt ui = 0; ui < 16; ui++ )
  {
    xReadBit( m_uiValue );
  }
#endif
}
#if !AVS3_DELETE_PPS_SYC
Void TDecSbac::parsePPS(TComPPS* pcPPS)
{
  UInt  uiCode;
  
  xReadCodeVlc ( 2, uiCode ); //NalRefIdc
  xReadCodeVlc ( 1, uiCode ); assert( 0 == uiCode); // zero bit
  xReadCodeVlc ( 5, uiCode ); assert( NAL_UNIT_PPS == uiCode);//NalUnitType
  return;
}
#endif
#if RPS
	Void TDecSbac::parseRefPicSet(TComSPS* pcSPS, TComReferencePictureSet* pcRPS, Int idx)
{
	UInt  uiCode;
	Int DeltaPoc;
	TComReferencePictureSet*   rps = pcSPS->getRPSList()->getReferencePictureSet(idx);
	xReadFlagVlc(uiCode);
	rps->setRefered((uiCode == 1) ? true : false);
	Int NumberOfNegativeRefPictures = 0;
	Int NumberOfPositiveRefPictures = 0;
	xReadCodeVlc(3, uiCode);
	rps->setNumberOfRefPictures(uiCode);
	for (Int i = 0; i < rps->getNumberOfRefPictures(); i++)
	{
		xReadCodeVlc(6, uiCode);
		assert(uiCode != 0);
		DeltaPoc = 0 - uiCode;
		rps->setRefDeltaPOC(i, DeltaPoc);
		if (DeltaPoc < 0)
		{
			NumberOfNegativeRefPictures++;
		}
		else
			NumberOfPositiveRefPictures++;
	}
	xReadCodeVlc(3, uiCode);
	rps->setNumberOfRemPictures(uiCode);
	rps->setNumberOfNegativeRefPictures(NumberOfNegativeRefPictures);
	rps->setNumberOfPositiveRefPictures(NumberOfPositiveRefPictures);
	for (Int j = 0; j < rps->getNumberOfRemPictures(); j++)
	{
		xReadCodeVlc(6, uiCode);
		assert(uiCode != 0);
		DeltaPoc = 0 - uiCode;
		rps->setRemDeltaPOC(j, DeltaPoc);
	}
	xReadFlagVlc(uiCode);//marker
}
#endif


#if AVS3_PIC_HEADER_ZL
Void TDecSbac::parsePicHeader(TComPicHeader*& rpcPicHeader
#if POC_256_BUG
	, Int prevTid0POC
#endif
	)
{
  UInt  uiCode;

  xReadCodeVlc( 32, uiCode);
  Bool bIntraFlag;
  if ( uiCode == 0x1B3 )
  {
    rpcPicHeader->setIntraPicStartCode( uiCode );
    bIntraFlag = true;
  }
  else
  {
    rpcPicHeader->setInterPicStartCode( uiCode );
    bIntraFlag = false;
  }

  xReadCodeVlc( 32, uiCode ); rpcPicHeader->setBbvDelay( uiCode );

  //intra
  if ( bIntraFlag )
  {
    xReadFlagVlc( uiCode ); rpcPicHeader->setTimeCodeFlag( uiCode == 0? false : true );
    if ( rpcPicHeader->getTimeCodeFlag() )
    {
      xReadCodeVlc( 24, uiCode ); rpcPicHeader->setTimeCode( uiCode );
    }
    if ( !rpcPicHeader->getSeqHeader()->getScenePictureDisableFlag() )
    {
      xReadFlagVlc( uiCode ); rpcPicHeader->setScenePicFlag( uiCode == 0? false : true );
      if ( rpcPicHeader->getScenePicFlag() )                          ///< 背景帧默认关闭2017-1-10
      {
        //PictureTypeB=background
      }
      else
      {
        //PicTypeB=0
      }
    }
    //if(PicTypeB==background) xReadFlagVlc(uiCide);setScenePicOutputFlag 
  }
  //inter
  else
  {
#if RPS_BUG
	  xReadCodeVlc(2, uiCode); rpcPicHeader->setPictureType((PictureType)uiCode);
#else
	  xReadCodeVlc(2, uiCode); rpcPicHeader->setPicCodingType(uiCode);
#endif
	  
	  if ( !rpcPicHeader->getSeqHeader()->getScenePictureDisableFlag() && (rpcPicHeader->getPicCodingType() == 1 || rpcPicHeader->getPicCodingType() ==3 ) )
    {
      if ( rpcPicHeader->getPicCodingType() == 1 )
      {
        xReadFlagVlc( uiCode ); rpcPicHeader->setScenePredFlag( uiCode == 0? false : true );
      }
      else
      {
        rpcPicHeader->setScenePredFlag( false );
      }
      if ( !rpcPicHeader->getScenePredFlag() )
      {
        xReadFlagVlc( uiCode ); rpcPicHeader->setSceneRefEnableFlag( uiCode == 0? false : true );
      }
      else
      {
        rpcPicHeader->setSceneRefEnableFlag( false );
      }
    }
    else
    {
      rpcPicHeader->setScenePredFlag( false );
      rpcPicHeader->setSceneRefEnableFlag( false );
    }

    if ( rpcPicHeader->getPicCodingType() == 1 )                    ///< 设置PictureType
    {
      //pictureType = P_Frame;
    }
    else if ( rpcPicHeader->getPicCodingType() == 3 )
    {
      //pictureType = F_Frame; 
    }
    else
    {
      //pictureType = B_Frame;
    }

    if( rpcPicHeader->getPicCodingType() == 1 && rpcPicHeader->getScenePredFlag() )  
    {
      //pictureTypeB = BP_Frame;                               ///< 设置PictureTypeB
    }
    else
    {
      //pictureTypeB =0;
    }
  }
#if RPS_BUG
#if BUG_816
#if POC_256_BUG
  xReadCodeVlc(8, uiCode);
  Int iPOCLSB = uiCode;
  Int iPOCMSB = 0;
  Int maxPOCoff = 1 << 8;
  Int prevPOCLSB = prevTid0POC & (maxPOCoff - 1);
  Int prevPOCMSB = prevTid0POC - prevPOCLSB;
  if (iPOCLSB < prevPOCLSB && abs(iPOCLSB - prevPOCLSB) >= maxPOCoff / 2)
  {
	  iPOCMSB = prevPOCMSB + maxPOCoff;
  }
  else if (iPOCLSB > prevPOCLSB && abs(iPOCLSB - prevPOCLSB) > maxPOCoff / 2)
  {
	  iPOCMSB = prevPOCMSB - maxPOCoff;
  }
  else
  {
	  iPOCMSB = prevPOCMSB;
  }
  rpcPicHeader->setPOC(iPOCMSB + iPOCLSB);
#else
  xReadCodeVlc(10, uiCode); rpcPicHeader->setPOC(uiCode);
#endif
#else
  xReadCodeVlc( 8, uiCode ); rpcPicHeader->setPOC( uiCode );
#endif
#else
  xReadCodeVlc(8, uiCode); rpcPicHeader->setDecOrderIndex(uiCode);
#endif

  if ( rpcPicHeader->getSeqHeader()->getTemporalIdEnableFlag() )
  {
    xReadCodeVlc( 3, uiCode ); rpcPicHeader->setTemporalId( uiCode );
  }
  //intra
#if SPS_BUG_YU
  if (!rpcPicHeader->getSeqHeader()->getLowDelay() && !(!rpcPicHeader->getSeqHeader()->getScenePictureDisableFlag() && !rpcPicHeader->getScenePicOutputFlag()))
  {
	  xReadUvlc(uiCode); rpcPicHeader->setPicOutputDelay(uiCode);
  }
#else
  if ( bIntraFlag )
  {
    // 背景帧暂时关闭2017-1-10
    // if(!rpcPicHeader->getSeqHeader()->getLowDelay() && (!rpcPicHeader->getScenePicFlag() || rpcPicHeader->getScenePicOutputFlag() ))   
    // xReadUvlc(uiCode); rpcPicHeader->setPicOutputDelay(uiCode);
  }
  //inter
  else
  {
    if ( !rpcPicHeader->getSeqHeader()->getLowDelay() )
    {
      xReadUvlc( uiCode ); rpcPicHeader->setPicOutputDelay( uiCode );
    }
  }
#endif

#if RPS
  Int DeltaPOC;
  xReadFlagVlc(uiCode); rpcPicHeader->setUseRcsFlag(uiCode == 0 ? false : true);
  if (rpcPicHeader->getUseRcsFlag())
  {
	  xReadCodeVlc(5, uiCode); rpcPicHeader->setRcsIndex(uiCode);
	  const TComReferencePictureSet *rps = rpcPicHeader->getSeqHeader()->getRPSList()->getReferencePictureSet(uiCode);
#if B_RPS_BUG_818
	  if (rpcPicHeader->getPictureType() == B_PICTURE)
	  {
		  TComReferencePictureSet *Localrps;
		  Localrps = rpcPicHeader->getLocalRPS();
		  (*Localrps) = TComReferencePictureSet();
		  Localrps->setRefered(rps->getRefered());
		  assert(rps->getNumberOfRefPictures() == 2);
		  Localrps->setNumberOfRefPictures(rps->getNumberOfRefPictures());
		  Localrps->setNumberOfRemPictures(rps->getNumberOfRemPictures());
		  for (Int j = 0; j < Localrps->getNumberOfRemPictures(); j++)
		  {
			  Localrps->setRemDeltaPOC(j, rps->getRemDeltaPOC(j));
		  }
		  Localrps->setNumberOfNegativeRefPictures(1);
		  Localrps->setNumberOfPositiveRefPictures(1);
		  Int i = 0;
		  for (i = 0; i < Localrps->getNumberOfNegativeRefPictures(); i++)
		  {
			  Localrps->setRefDeltaPOC(i, rps->getRefDeltaPOC(i));
		  }
		  for (i = Localrps->getNumberOfNegativeRefPictures(); i < Localrps->getNumberOfRefPictures(); i++)
		  {
			  Localrps->setRefDeltaPOC(i, (0 - rps->getRefDeltaPOC(i)));
		  }
		  rpcPicHeader->setRPS(Localrps);
	  }
	  else
	  {
		  rpcPicHeader->setRPS(rps);
	  }
#else
	  rpcPicHeader->setRPS(rps);
#endif
  }
  else
  {
	  //参考配置集解析 2017-3-20 ZP
	  TComReferencePictureSet *rps;
	  rps = rpcPicHeader->getLocalRPS();
	  (*rps) = TComReferencePictureSet();
	  rpcPicHeader->setRPS(rps);
	  xReadFlagVlc(uiCode);
	  rps->setRefered(uiCode == 0 ? false : true);
	  xReadCodeVlc(3, uiCode);//参考帧数
	  rps->setNumberOfRefPictures(uiCode);
	  UInt NumberOfNegativeRefPictures = 0;
	  UInt NumberOfPositiveRefPictures = 0;//暂无
	  for (Int i = 0; i < rps->getNumberOfRefPictures(); i++)
	  {
		  xReadCodeVlc(6, uiCode);
		  DeltaPOC = 0 - uiCode;
		  rps->setRefDeltaPOC(i, DeltaPOC);
#if B_RPS_BUG_818
		  if (rpcPicHeader->getPictureType() == B_PICTURE)
		  {
			  assert(rps->getNumberOfRefPictures() == 2);
			  if (i == 1) //PositiveRefPictures
			  {
				  DeltaPOC = 0 - DeltaPOC;
				  rps->setRefDeltaPOC(i, DeltaPOC);
			  }
		  }
#endif
		  if (DeltaPOC < 0)
			  NumberOfNegativeRefPictures++;
		  else
			  NumberOfPositiveRefPictures++;
	  }
	  rps->setNumberOfNegativeRefPictures(NumberOfNegativeRefPictures);
	  rps->setNumberOfPositiveRefPictures(NumberOfPositiveRefPictures);
	  xReadCodeVlc(3, uiCode);
	  rps->setNumberOfRemPictures(uiCode);
	  for (Int j = 0; j < rps->getNumberOfRemPictures(); j++)
	  {
		  xReadCodeVlc(6, uiCode);
		  assert(uiCode != 0);
		  DeltaPOC = 0 - uiCode;
		  rps->setRemDeltaPOC(j, DeltaPOC);
	  }
	  rpcPicHeader->setRPS(rps);
	  xReadFlagVlc(uiCode);//marker
  }

#else

  xReadFlagVlc( uiCode ); rpcPicHeader->setUseRcsFlag( uiCode == 0? false : true );
  if ( rpcPicHeader->getUseRcsFlag() )
  {
    xReadCodeVlc( 5, uiCode ); rpcPicHeader->setRcsIndex( uiCode );
  }
  else
  {
    //参考配置集解析 2017-1-10
  }
 #endif
  if ( rpcPicHeader->getSeqHeader()->getLowDelay() )
  {
    xReadUvlc( uiCode ); rpcPicHeader->setBbvCheckTimes( uiCode );
  }

  xReadFlagVlc( uiCode ); rpcPicHeader->setProgressiveFrame( uiCode == 0? false : true );

  if ( !rpcPicHeader->getProgressiveFrame())
  {
    xReadFlagVlc( uiCode ); rpcPicHeader->setPicStructure( uiCode? PS_BLEND:PS_SUCCESSIVELY );
  }

  xReadFlagVlc( uiCode ); rpcPicHeader->setTopFieldFirst( uiCode == 0? false : true );
  xReadFlagVlc( uiCode ); rpcPicHeader->setRepeatFirstField( uiCode == 0? false : true );

  if ( rpcPicHeader->getSeqHeader()->getFieldCodedSequence() )
  {
    xReadFlagVlc( uiCode ); rpcPicHeader->setTopFieldPicFlag( uiCode == 0? false : true );
    xReadBit( uiCode ); //reserved_bits r(1)
  }

  xReadFlagVlc( uiCode ); rpcPicHeader->setFixedPicQP( uiCode == 0? false : true );
  xReadCodeVlc( 7, uiCode ); rpcPicHeader->setPicQP( uiCode );

  //inter
  if ( !bIntraFlag )
  {
    if ( !(rpcPicHeader->getPicCodingType() == 2 && rpcPicHeader->getPicStructure()) )
    {
      xReadBit( uiCode ); //reserved_bits r(1)
    }
    xReadFlagVlc( uiCode ); rpcPicHeader->setRADecodableFlag( uiCode == 0? false : true );
  }

  xReadFlagVlc( uiCode ); rpcPicHeader->setLoopFilterDisableFlag( uiCode == 0? false : true );
  if ( !rpcPicHeader->getLoopFilterDisableFlag() )
  {
    xReadFlagVlc( uiCode ); rpcPicHeader->getLoopFilterSyntax()->m_bLoppFilterParamFlag = ( uiCode == 0? false : true );
    if ( rpcPicHeader->getLoopFilterSyntax()->m_bLoppFilterParamFlag )
    {
      xReadUvlc( uiCode ); rpcPicHeader->getLoopFilterSyntax()->m_iAlphaCOffset = uiCode;
      xReadUvlc( uiCode ); rpcPicHeader->getLoopFilterSyntax()->m_iBetaOffset = uiCode;
    }
    else
    {
      rpcPicHeader->getLoopFilterSyntax()->m_iAlphaCOffset = 0;
      rpcPicHeader->getLoopFilterSyntax()->m_iBetaOffset = 0;
    }
  }

  xReadFlagVlc( uiCode ); rpcPicHeader->setChromaQuantParamDisableFlag( uiCode == 0? false : true );
  if ( !rpcPicHeader->getChromaQuantParamDisableFlag())
  {
    xReadUvlc( uiCode ); rpcPicHeader->setChromaQuantParamDeltaCb( uiCode );
    xReadUvlc( uiCode ); rpcPicHeader->setChromaQuantParamDeltaCr( uiCode );
  }
  else
  {
    rpcPicHeader->setChromaQuantParamDeltaCb( 0 );
    rpcPicHeader->setChromaQuantParamDeltaCr( 0 );
  }

  if ( rpcPicHeader->getSeqHeader()->getWeightQuantEnableFlag() )
  {
    xReadFlagVlc( uiCode ); rpcPicHeader->setPicWeightQuantEnableFlag( uiCode == 0? false : true );
    if ( rpcPicHeader->getPicWeightQuantEnableFlag() )
    {
      xReadCodeVlc( 2, uiCode ); rpcPicHeader->getWeightQuantSyntax()->m_uiPicWeightQuantDataIndex = uiCode;
      if ( rpcPicHeader->getWeightQuantSyntax()->m_uiPicWeightQuantDataIndex == 1 )
      {
        xReadBit( uiCode );
        xReadCodeVlc( 2, uiCode ); rpcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantParamIndex = uiCode;
        xReadCodeVlc( 2, uiCode ); rpcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantMode = uiCode;
        if ( rpcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantParamIndex == 1 )
        {
          for ( int i = 0; i < 6; i++ )
          {
            xReadUvlc( uiCode ); rpcPicHeader->getWeightQuantSyntax()->m_iWeightQuantParamDelta1[i] = uiCode;
            //加权量化Default和unDetail数组未加入   2017-1-10
            //quant_param_undetail[i] = rpcPicHeader->getWeightQuantSyntax()->m_iWeightQuantParamDelta1[i] + wq_param_default[UNDETAILED][i]
          }
        }
        if ( rpcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantParamIndex == 2 )
        {
          for ( int i = 0; i < 6; i++ )
          {
            xReadUvlc( uiCode ); rpcPicHeader->getWeightQuantSyntax()->m_iWeightQuantParamDelta2[i] = uiCode;
            //加权量化Default和Detail数组未加入    2017-1-10
            //quant_param_detail[i] = rpcPicHeader->getWeightQuantSyntax()->m_iWeightQuantParamDelta2[i] + wq_param_default[DETAILED][i]
          }
        }
      }//PicWeightQuantDataIndex == 1
      else if ( rpcPicHeader->getWeightQuantSyntax()->m_uiPicWeightQuantDataIndex == 2 )
      {
        //weight_quant_matrix()
        for ( int iSizeId = 0; iSizeId < 2; iSizeId++ )
        {
          UInt WQMSize = 1 << (iSizeId + 2);
          for ( int i = 0; i < WQMSize; i++ )
          {
            for ( int j = 0; j < WQMSize; j++ )
            {
              xReadUvlc( uiCode );
              if ( iSizeId == 0 )
              {
                rpcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantCoeff4x4[i][j] = uiCode;
              }
              else
              {
                rpcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantCoeff8x8[i][j] = uiCode;
              }
            }//for(int j = 0; j < WQMSize; j++)
          }//for(int i = 0; i < WQMSize; i++)
        }//for(int iSizeId = 0; iSizeId < 2; iSizeId++)
      }//else if ( rpcPicHeader->getWeightQuantSyntax()->m_uiPicWeightQuantDataIndex == 2 )
    }//if(rpcPicHeader->getPicWeightQuantEnableFlag())
  }//if(rpcPicHeader->getSeqHeader()->getWeightQuantEnableFlag())

  if ( rpcPicHeader->getSeqHeader()->getAdaptiveLoopFilterEnableFlag() )
  {
    for ( int i = 0; i< 3; i++ )
    {
      xReadFlagVlc( uiCode ); rpcPicHeader->setPicAlfEnableFlag( (uiCode == 0? false : true), i );
    }
    if( rpcPicHeader->getPicAlfEnableFlag(0) || rpcPicHeader->getPicAlfEnableFlag(1) || rpcPicHeader->getPicAlfEnableFlag(2) )
    {
      if ( rpcPicHeader->getPicAlfEnableFlag(0) )
      {
        xReadUvlc( uiCode ); rpcPicHeader->getAlfFilterSyntax()->m_uiAlfFilterNumMinus1 = uiCode;       
        for ( int i = 0; i < (rpcPicHeader->getAlfFilterSyntax()->m_uiAlfFilterNumMinus1 + 1); i++ )
        {
          if ( i > 0 && rpcPicHeader->getAlfFilterSyntax()->m_uiAlfFilterNumMinus1 != 15 )
          {
            xReadUvlc( uiCode ); rpcPicHeader->getAlfFilterSyntax()->m_uiAlfRegionDistance[i] = uiCode;
          }
          for ( int j = 0; j < 9; j++ )
          {
            xReadUvlc( uiCode ); rpcPicHeader->getAlfFilterSyntax()->m_iAlfCoeffLuma[i][j] = uiCode;
          }
        }
      }//if(rpcPicHeader->getPicAlfEnableFlag(0))
      if ( rpcPicHeader->getPicAlfEnableFlag(1) )
      {
        for ( int j = 0; j < 9; j++ )
        {
          xReadUvlc( uiCode );rpcPicHeader->getAlfFilterSyntax()->m_iAlfCoeffChroma[0][j] = uiCode;
        }
      }
      if ( rpcPicHeader->getPicAlfEnableFlag(2) )
      {
        for ( int j = 0; j < 9; j++ )
        {
          xReadUvlc( uiCode );rpcPicHeader->getAlfFilterSyntax()->m_iAlfCoeffChroma[1][j] = uiCode;
        }
      }
    }//if( rpcPicHeader->getPicAlfEnableFlag(0) || rpcPicHeader->getPicAlfEnableFlag(1) || rpcPicHeader->getPicAlfEnableFlag(2) )
  }//if(rpcPicHeader->getSeqHeader()->getAdaptiveLoopFilterEnableFlag())
  return;
}
#endif



Void TDecSbac::parseSPS(TComSPS* pcSPS)
{
  UInt  uiCode;
  xReadCodeVlc ( 2, uiCode ); //NalRefIdc
  xReadCodeVlc ( 1, uiCode ); assert( 0 == uiCode); // zero bit
  xReadCodeVlc ( 5, uiCode ); assert( NAL_UNIT_SPS == uiCode);//NalUnitType

  // Structure
  xReadUvlc ( uiCode ); pcSPS->setWidth       ( uiCode    );
  xReadUvlc ( uiCode ); pcSPS->setHeight      ( uiCode    );
  xReadUvlc ( uiCode ); pcSPS->setPadX        ( uiCode    );
  xReadUvlc ( uiCode ); pcSPS->setPadY        ( uiCode    );
  
  xReadUvlc ( uiCode );
  uiCode = uiCode + 2;
  pcSPS->setLog2MaxCUSize( uiCode    ); g_uiLog2MaxCUSize  = uiCode;
  
  xReadUvlc ( uiCode ); 
  pcSPS->setMaxCUDepth  ( uiCode+1  ); g_uiMaxCUDepth  = uiCode + 1;

  xReadUvlc( uiCode ); pcSPS->setLog2MaxPhTrSize( uiCode + 2 );
  
  // Tool on/off
  xReadFlagVlc( uiCode ); pcSPS->setUseDQP ( uiCode ? true : false );
  xReadFlagVlc( uiCode ); pcSPS->setUseLDC ( uiCode ? true : false );
  
  // Bit-depth information
  xReadUvlc( uiCode ); pcSPS->setBitDepth     ( uiCode+8 ); g_uiBitDepth     = uiCode + 8;
  
  g_uiBASE_MAX  = ((1<<(g_uiBitDepth))-1);

#if AVS3_SEQ_HEADER_SYC
  xReadCodeVlc( 32,  uiCode);pcSPS->setVideoSequenceStartCode(uiCode);//f(32), "video_sequence_start_code"
  xReadCodeVlc( 8,  uiCode);pcSPS->setProfileId(Profile::Name(uiCode));//u(8), "profile_id"
  xReadCodeVlc( 8,  uiCode);pcSPS->setLevelId(Level::Name(uiCode));//u(8), "level_id"
  xReadFlagVlc( uiCode);pcSPS->setProgressiveSequence(uiCode? true:false);//u(1), "progressive_sequence"
  xReadFlagVlc( uiCode);pcSPS->setFieldCodedSequence(uiCode? FIELD_SEQUENCE:PTS_FRAME_SEQUENCE);//u(1), "field_coded_sequence"
  xReadCodeVlc( 14,  uiCode);pcSPS->setHorizontalSize(uiCode);//u(14), "horizontal_size"
  xReadCodeVlc( 14,  uiCode);pcSPS->setVerticalSize(uiCode);//u(14), "vertical_size"
  xReadCodeVlc( 2,  uiCode);pcSPS->setChromaFormat(ChromaFormat (uiCode));//u(2), "chroma_format"
  xReadCodeVlc(3,  uiCode);pcSPS->setSamplePrecision(uiCode==1? 8:10);//u(3), "sample_precision"
  if (pcSPS->getProfileId() == Profile::BASELINE10_PROFILE) 
  { 
	  xReadCodeVlc(3,  uiCode);pcSPS->setEncodingPrecision(uiCode==1? 8:10);//u(3), "encoding_precision"
  } 
  xReadCodeVlc(4,  uiCode);pcSPS->setAspectRatio(uiCode);//u(4), "aspect_ratio"
  xReadCodeVlc(4,  uiCode);pcSPS->setFrameRateCode(uiCode);//u(4), "frame_rate_code"
  xReadCodeVlc(18,  uiCode);pcSPS->setBitRateLower(uiCode);//u(18), "bit_rate_lower"
  xReadBit(uiCode);//f(1), "marker_bit"
  xReadCodeVlc(12,  uiCode);pcSPS->setBitRateUpper(uiCode);//u(12), "bit_rate_upper"
  xReadFlagVlc(uiCode);pcSPS->setLowDelay(uiCode? true:false);//u(1), "low_delay"
  xReadBit(uiCode);//f(1),"marker_bit"
  xReadFlagVlc(uiCode);pcSPS->setTemporalIdEnableFlag(uiCode? true:false);//u(1), "temporal_id_enable_flag"
  xReadCodeVlc(18,  uiCode);pcSPS->setBbvBufferSize(uiCode*16*1024);//u(18), "bbv_buffer_size"
  xReadCodeVlc(3,  uiCode);pcSPS->setLCUSize(uiCode == 6? 64:16);//u(3), "lcu_size"
  xReadFlagVlc(uiCode);pcSPS->setWeightQuantEnableFlag(uiCode? true:false);//u(1), "weight_quant_enable_flag"
  if (pcSPS->getWeightQuantEnableFlag()) 
  {
	  xReadFlagVlc(uiCode);pcSPS->setbLoadSeqWeightQuantDataFlag(uiCode? true:false);//u(1), "load_seq_weight_quant_data_flag"
	  if(pcSPS->getbLoadSeqWeightQuantDataFlag())
	  {
		  Int x, y, sizeId;
		  UInt uiWqMSize;
		  for (sizeId = 0; sizeId < 2; sizeId++) 
		  {
			  uiWqMSize = min(1 << (sizeId + 2), 8);
			  for (y = 0; y < uiWqMSize; y++) 
			  {
				  for (x = 0; x < uiWqMSize; x++) 
				  {
					  if(sizeId == 0)
					  {
						  xReadUvlc(uiCode);//,  "weight_quant_coeff"
						  pcSPS->setWeightQuantCoeff4x4(y,x,uiCode);///<20170107 Cfg需要返工,4x4数据 m_uiWeightQuantCoeff
					  }
					  else
					  {
						  xReadUvlc(uiCode);//,  "weight_quant_coeff"
						  pcSPS->setWeightQuantCoeff8x8(y,x,uiCode);///<20170107 Cfg需要返工,8x8数据 m_uiWeightQuantCoeff
					  }
				  }
			  }
		  }
	  }
	  else
	  {
		  ///《20170108l加权量化初始值赋值
	  }
  }
  xReadFlagVlc(uiCode);pcSPS->setScenePictureDisableFlag(uiCode? true:false);//u(1), "scene_picture_disable_flag"
  xReadFlagVlc(uiCode);pcSPS->setMultiHypothesisSkipEnableFlag(uiCode? true:false);//u(1), "multi_hypothesis_skip_enable_flag"
  xReadFlagVlc(uiCode);pcSPS->setDualHypothesisPredictionEnableFlag(uiCode? true:false);//u(1), "dual_hypothesis_prediction_enable_flag"
  xReadFlagVlc(uiCode);pcSPS->setWeightedSkipEnableFlag(uiCode? true:false);//u(1), "weighted_skip_enable_flag"
  xReadFlagVlc(uiCode);pcSPS->setAsymmetricMotionPartitionsEnableFlag(uiCode? true:false);//u(1), "asymmetric_motion_partitions_enable_flag"
  xReadFlagVlc(uiCode);pcSPS->setNonsquareQuadtreeTansformEnableFlag(uiCode? true:false);//u(1), "nonsquare_quadtree_transform_enable_flag"
  xReadFlagVlc(uiCode);pcSPS->setNonsquareIntraPredictionEnableFlag(uiCode? true:false);//u(1), "nonsquare_intra_prediction_enable_flag"
  xReadFlagVlc(uiCode);pcSPS->setSecondaryTransformEnableFlag(uiCode? true:false);//u(1), "secondary_transform_enable_flag"
  xReadFlagVlc(uiCode);pcSPS->setSampleAdaptiveOffsetEnableFlag(uiCode? true:false);//u(1), "sample_adaptive_offset_enable_flag"
  xReadFlagVlc(uiCode);pcSPS->setAdaptiveLoopFilterEnableFlag(uiCode? true:false);//u(1), "adaptive_loop_filter_enable_flag"
  xReadFlagVlc(uiCode);pcSPS->setPmvrEnableFlag(uiCode? true:false);//u(1), "pmvr_enable_flag"
  xReadFlagVlc(uiCode);//f(1), "marker_bit"
  //xReadCodeVlc(6,  uiCode);pcSPS->setNumOfRcs(uiCode);//u(6), "num_of_rcs"
  // xReadFlagVlc(uiCode);//u(1), "refered_by_others_flag"          ///<参考帧管理
  // xReadCodeVlc(3,  uiCode);//u(3), "num_of_reference_picture"
  // xReadCodeVlc(6,  uiCode);//u(6), "delta_doi_of_reference_picture"
  // xReadCodeVlc(3,  uiCode);//u(3), "num_of_removed_picture"
  // READ_CODE(6,  uiCode, "delta_doi_of_removed_picture");//f(6)
  xReadFlagVlc(uiCode);//f(1), "marker_bit"
#if RPS
	  xReadCodeVlc(6, uiCode);//listlenght  numofRPS
  pcSPS->createRPSList(uiCode);
  TComRPSList* rpsList = pcSPS->getRPSList();
  TComReferencePictureSet* rps;
  for (UInt i = 0; i< rpsList->getNumberOfReferencePictureSets(); i++)
  {
	  rps = rpsList->getReferencePictureSet(i);
	  parseRefPicSet(pcSPS, rps, i);
  }
#endif
  if (pcSPS->getLowDelay()) 
  {
	  xReadCodeVlc(5,  uiCode);pcSPS->setOutputReorderDelay(uiCode);//u(5), "output_reorder_delay"
  }
  xReadFlagVlc(uiCode);pcSPS->setCrossSliceLoopfilterEnableFlag(uiCode? true:false);//u(1), "cross_slice_loopfilter_enable_flag"
  xReadFlagVlc(uiCode);
  xReadFlagVlc(uiCode);//r(2), "reserved_bits"
#endif
  return;
}
#if AVS3_EXTENSION_LWJ
Void  TDecSbac::parseUserData(TComUserData* pcUserData)
{
  UInt  uiCode1, uiCode2, uiCode3, uiCode=0;
  pcUserData->setUserDataStartCode(0x1B2);	//f(32),"user_data_start_code",
#if AVS3_EXTENSION_DEBUG_SYC
  xNextNBit(24, uiCode);
  uiCode3 = uiCode & 31;
  uiCode >>= 8;
  uiCode2 = uiCode & 31;
  uiCode >>= 8;
  uiCode1 = uiCode & 31;
  while (!((uiCode1 == 0) && (uiCode2 == 0) && (uiCode3 == 1)))
  {
    xReadCodeVlc(8, uiCode);//读出真正的语法元素，暂时不存储。
    uiCode1 = uiCode2;
    uiCode2 = uiCode3;
    xNextNBit(24, uiCode);
    uiCode3 = uiCode & 31;
  }
  //xReadCodeVlc(24, uiCode);

#else
  xReadCodeVlc(8, uiCode1); xReadCodeVlc(8, uiCode2); xReadCodeVlc(8, uiCode3);
  while (!( (uiCode1 == 0) && (uiCode2 == 0) && (uiCode3 == 1)))
  {
    uiCode = uiCode * 256 + uiCode1;
    uiCode1 = uiCode2;
    uiCode2 = uiCode3;
    xReadCodeVlc(8, uiCode3);
  }
  pcUserData->setUserData(uiCode);				//b(8),"user_data",
#endif
}
Void  TDecSbac::parseExtSeqDisplay(TComSPS* pcSPS)
{
  UInt  uiCode;

#if AVS3_EXTENSION_DEBUG_SYC
  xReadCodeVlc(4, uiCode);
  pcSPS->getExtSeqDisplay()->setExtId(uiCode);					//f(4), "extension_id", 
#else
  pcSPS->getExtSeqDisplay()->setExtId(2);					//f(4), "extension_id", 
#endif
  xReadCodeVlc(3, uiCode); pcSPS->getExtSeqDisplay()->setVideoFormat(uiCode);				//u(3), "video_format",
  xReadFlagVlc(uiCode); pcSPS->getExtSeqDisplay()->setSampleRange(uiCode ? true : false);			//u(1), "sample_range",
  xReadFlagVlc(uiCode); pcSPS->getExtSeqDisplay()->setColourDescription(uiCode ? true : false);	//u(1), "colour_description",
  if (pcSPS->getExtSeqDisplay()->getColourDescription())
  {
    xReadCodeVlc(8, uiCode); pcSPS->getExtSeqDisplay()->setColourPrimaries(uiCode);		//u(8), "colour_primaries",
    xReadCodeVlc(8, uiCode); pcSPS->getExtSeqDisplay()->setTransferCharacteristics(uiCode);			//u(8), "transfer_characteristics",
    xReadCodeVlc(8, uiCode); pcSPS->getExtSeqDisplay()->setMatrixCoeff(uiCode);			//u(8), "matrix_coeff",
  }
  xReadCodeVlc(14, uiCode); pcSPS->getExtSeqDisplay()->setDisplayHorSize(uiCode);			//u(14), "display_horizontal_size",
  xReadCodeVlc(1, uiCode);																	//f(1), "marker_bit", 
  xReadCodeVlc(14, uiCode); pcSPS->getExtSeqDisplay()->setDisplayVerSize(uiCode);			//u(14), "display_vertical_size",
  xReadFlagVlc(uiCode); pcSPS->getExtSeqDisplay()->setTdModeFlag(uiCode ? true : false);	//u(1), "td_mode_flag",
  if (pcSPS->getExtSeqDisplay()->getTdModeFlag())
  {
    xReadCodeVlc(8, uiCode); pcSPS->getExtSeqDisplay()->setTdPackingMode(uiCode);		//u(8), "td_packing_mode",
	xReadFlagVlc(uiCode); pcSPS->getExtSeqDisplay()->setViewReverseFlag(uiCode ? true : false);		//u(1), "view_reverse_flag",
  }
}
Void  TDecSbac::parseExtScalability(TComSPS* pcSPS)
{
  UInt  uiCode;
#if AVS3_EXTENSION_DEBUG_SYC
  xReadCodeVlc(4, uiCode);
  pcSPS->getExtScalability()->setExtId(uiCode);					//f(4), "extension_id", 
#else
  pcSPS->getExtScalability()->setExtId(3);					//f(4), "extension_id", 
#endif

  xReadCodeVlc(3, uiCode); pcSPS->getExtScalability()->setNumOfTemporalLevelMinus1(uiCode);//u(3), "num_of_temporal_level_minus1",
  for (int i = 0; i < pcSPS->getExtScalability()->getNumOfTemporalLevelMinus1(); i++)
  {
    xReadCodeVlc(4, uiCode); pcSPS->getExtScalability()->setTemporalFrameRateCode(i, uiCode);	//u(4), "temporal_frame_rate_code[i]", 
    xReadCodeVlc(18, uiCode); pcSPS->getExtScalability()->setTemporalBitRateLower(i, uiCode);	//u(18), "temporal_bit_rate_lower[i]", 
    xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit", 
    xReadCodeVlc(12, uiCode); pcSPS->getExtScalability()->setTemporalBitRateUpper(i, uiCode);	//u(12), "temporal_bit_rate_upper[i]", 
  }
}
#endif
#if AVS3_EXTENSION_CYH
Void  TDecSbac::parseExtCopyright(TComExtCopyright* pcExtCopyright)
{
  UInt  uiCode;

#if AVS3_EXTENSION_DEBUG_SYC
  xReadCodeVlc(4, uiCode);
  pcExtCopyright->setExtId(uiCode);					//f(4), "extension_id", 
#else
  pcExtCopyright->setExtId(4);					//f(4), "extension_id", 
#endif

  xReadFlagVlc(uiCode);	 pcExtCopyright->setCopyrightFlag(uiCode ? true : false);			//u(1), "copyright_flag",
  xReadCodeVlc(8, uiCode); pcExtCopyright->setCopyrightId(uiCode);				//u(8), "copyright_id", 
  xReadFlagVlc(uiCode);	 pcExtCopyright->setOriginalOrCopy(uiCode ? true : false);			//u(1), "OriginalOrCopy",
  xReadCodeVlc(7, uiCode);
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit", 
  xReadCodeVlc(20, uiCode); pcExtCopyright->setCopyrightNum1(uiCode);			//u(20),"copyright_number_1", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit", 
  xReadCodeVlc(22, uiCode); pcExtCopyright->setCopyrightNum2(uiCode);			//u(22), "copyright_number_2", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit", 
  xReadCodeVlc(22, uiCode); pcExtCopyright->setCopyrightNum3(uiCode);			//u(22), "copyright_number_3", 
}
Void  TDecSbac::parseExtCameraPra(TComExtCameraPra* pcExtCameraPra)
{
  UInt  uiCode;
#if AVS3_EXTENSION_DEBUG_SYC
  xReadCodeVlc(4, uiCode);
  pcExtCameraPra->setExtId(uiCode);					//f(4), "extension_id", 
#else
  pcExtCameraPra->setExtId(11);					//f(4), "extension_id", 
#endif
  xReadCodeVlc(1, uiCode);
  xReadCodeVlc(7, uiCode); pcExtCameraPra->setCameraId(uiCode);					//f(4), "camera_id", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit", 
  xReadCodeVlc(22, uiCode); pcExtCameraPra->setHeightOfImageDevice(uiCode);		//u(22), "height_of_image_device", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit", 
  xReadCodeVlc(22, uiCode); pcExtCameraPra->setFocalLenth(uiCode);				//u(22), "focal_length", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit", 
  xReadCodeVlc(22, uiCode); pcExtCameraPra->setFNum(uiCode);					//u(22), "f_number", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit", 
  xReadCodeVlc(22, uiCode); pcExtCameraPra->setVerAngleOfView(uiCode);			//u(22), "vertical_angle_of_view", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit", 
  xReadCodeVlc(16, uiCode); pcExtCameraPra->setCameraPosXUpper(uiCode);			//u(16), "camera_position_x_upper", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit", 
  xReadCodeVlc(16, uiCode); pcExtCameraPra->setCameraPosXLower(uiCode);			//u(16), "camera_position_x_lower", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit", 
  xReadCodeVlc(16, uiCode); pcExtCameraPra->setCameraPosYUpper(uiCode);			//u(16), "camera_position_y_upper", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit",
  xReadCodeVlc(16, uiCode); pcExtCameraPra->setCameraPosYLower(uiCode);			//u(16), "camera_position_y_lower", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit",
  xReadCodeVlc(16, uiCode); pcExtCameraPra->setCameraPosZUpper(uiCode);			//u(16), "camera_position_z_upper", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit",
  xReadCodeVlc(16, uiCode); pcExtCameraPra->setCameraPosZLower(uiCode);			//u(16), "camera_position_z_lower", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit",
  xReadCodeVlc(22, uiCode); pcExtCameraPra->setCameraDirX(uiCode);			    //u(22), "camera_direction_x", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit", 
  xReadCodeVlc(22, uiCode); pcExtCameraPra->setCameraDirY(uiCode);				//u(22), "camera_direction_y", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit", 
  xReadCodeVlc(22, uiCode); pcExtCameraPra->setCameraDirZ(uiCode);				//u(22), "camera_direction_z", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit",
  xReadCodeVlc(22, uiCode); pcExtCameraPra->setImagePlaneVerX(uiCode);			//u(22), "image_plane_vertical_x", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit",
  xReadCodeVlc(22, uiCode); pcExtCameraPra->setImagePlaneVerY(uiCode);			//u(22), "image_plane_vertical_y", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit",
  xReadCodeVlc(22, uiCode); pcExtCameraPra->setImagePlaneVerZ(uiCode);			//u(22), "image_plane_vertical_z", 
  xReadCodeVlc(1, uiCode);																		//f(1), "marker_bit",
  xReadCodeVlc(16, uiCode);
}
#endif
#if AVS3_EXTENSION_LYT
Void TDecSbac::parseExtMetadata(TComExtMetadata*& pcExtMetadata)
{
  UInt uiCode;
  Int c;
#if AVS3_EXTENSION_DEBUG_SYC
  xReadCodeVlc(4, uiCode);
  pcExtMetadata->setExtId(uiCode);					//f(4), "extension_id", 
#else
  pcExtMetadata->setExtId(10);
#endif

  for (c = 0; c < 3; c++)
  {
    xReadCodeVlc(16, uiCode); pcExtMetadata->setDisplayPrimariesX(c, uiCode);
    xReadBit(uiCode);
    xReadCodeVlc(16, uiCode); pcExtMetadata->setDisplayPrimariesY(c, uiCode);
    xReadBit(uiCode);
  }
  xReadCodeVlc(16, uiCode); pcExtMetadata->setWhitePointX(uiCode);
  xReadBit(uiCode);
  xReadCodeVlc(16, uiCode); pcExtMetadata->setWhitePointY(uiCode);
  xReadBit(uiCode);
  xReadCodeVlc(16, uiCode); pcExtMetadata->setMaxDisplayMasteringLum(uiCode);
  xReadBit(uiCode);
  xReadCodeVlc(16, uiCode); pcExtMetadata->setMinDisplayMasteringLum(uiCode);
  xReadBit(uiCode);
  xReadCodeVlc(16, uiCode); pcExtMetadata->setMaxContentLightLevel(uiCode);
  xReadBit(uiCode);
  xReadCodeVlc(16, uiCode); pcExtMetadata->setMaxPicAverageLightLevel(uiCode);
  xReadBit(uiCode);
  xReadCodeVlc(16, uiCode);
}

Void TDecSbac::parseExtRoiPra(TComPicHeader*& pcPicHeader)
{
  UInt uiCode;
#if AVS3_EXTENSION_DEBUG_SYC
  xReadCodeVlc(4, uiCode);
  pcPicHeader->getExtRoiPra()->setExtId(uiCode);					//f(4), "extension_id", 
#else
  pcPicHeader->getExtRoiPra()->setExtId(12);
#endif

  xReadCodeVlc(8, uiCode); pcPicHeader->getExtRoiPra()->setCurrPicRoiNum(uiCode);
  Int roiIndex = 0, i, j, iCode;
  if (pcPicHeader->getSeqHeader()->getFieldCodedSequence() != 0)
  {
    xReadCodeVlc(8, uiCode); pcPicHeader->getExtRoiPra()->setPrevPicRoiNum(uiCode);
    for (i = 0; i < pcPicHeader->getExtRoiPra()->getPrevPicRoiNum(); i++)
    {
      xReadUvlc(uiCode); pcPicHeader->getExtRoiPra()->setRoiSkipRun(uiCode);
      if (pcPicHeader->getExtRoiPra()->getRoiSkipRun() != '0')
      {
        for (j = 0; j < pcPicHeader->getExtRoiPra()->getRoiSkipRun(); j++)
        {
          xReadCodeVlc(1, uiCode); pcPicHeader->getExtRoiPra()->setSkipRoiMode(uiCode, i + j);
          if (j % 22 == 0)
          {
            xReadBit(uiCode);
            if (*(pcPicHeader->getExtRoiPra()->getSkipRoiMode() + i + j) == '1')
            {
              (pcPicHeader->getExtRoiPra()->getROIInfo() + roiIndex)->asisx = (pcPicHeader->getExtRoiPra()->getPrevROIInfo() + i + j)->asisx;
              (pcPicHeader->getExtRoiPra()->getROIInfo() + roiIndex)->asisy = (pcPicHeader->getExtRoiPra()->getPrevROIInfo() + i + j)->asisy;
              (pcPicHeader->getExtRoiPra()->getROIInfo() + roiIndex)->width = (pcPicHeader->getExtRoiPra()->getPrevROIInfo() + i + j)->width;
              (pcPicHeader->getExtRoiPra()->getROIInfo() + roiIndex)->height = (pcPicHeader->getExtRoiPra()->getPrevROIInfo() + i + j)->height;
              roiIndex++;
            }
          }
          i += j;
          xReadBit(uiCode);
        }
      }
      else
      {
        xReadSvlc(iCode); pcPicHeader->getExtRoiPra()->setRoiAxisxDelta(iCode);
        xReadBit(uiCode);
        xReadSvlc(iCode); pcPicHeader->getExtRoiPra()->setRoiAxisyDelta(iCode);
        xReadBit(uiCode);
        xReadSvlc(iCode); pcPicHeader->getExtRoiPra()->setRoiWidthDelta(iCode);
        xReadBit(uiCode);
        xReadSvlc(iCode); pcPicHeader->getExtRoiPra()->setRoiHeightDelta(iCode);
        xReadBit(uiCode);
        (pcPicHeader->getExtRoiPra()->getROIInfo() + roiIndex)->asisx = (pcPicHeader->getExtRoiPra()->getPrevROIInfo() + i + j)->asisx + pcPicHeader->getExtRoiPra()->getRoiAxisxDelta();
        (pcPicHeader->getExtRoiPra()->getROIInfo() + roiIndex)->asisy = (pcPicHeader->getExtRoiPra()->getPrevROIInfo() + i + j)->asisy + pcPicHeader->getExtRoiPra()->getRoiAxisyDelta();
        (pcPicHeader->getExtRoiPra()->getROIInfo() + roiIndex)->width = (pcPicHeader->getExtRoiPra()->getPrevROIInfo() + i + j)->width + pcPicHeader->getExtRoiPra()->getRoiWidthDelta();
        (pcPicHeader->getExtRoiPra()->getROIInfo() + roiIndex)->height = (pcPicHeader->getExtRoiPra()->getPrevROIInfo() + i + j)->height + pcPicHeader->getExtRoiPra()->getRoiHeightDelta();
        roiIndex++;
      }
    }
  }
  for (i = roiIndex; i < pcPicHeader->getExtRoiPra()->getCurrPicRoiNum(); i++)
  {
    xReadCodeVlc(6, uiCode); pcPicHeader->getExtRoiPra()->setRoiAxisx(uiCode);
    xReadBit(uiCode);
    xReadCodeVlc(6, uiCode); pcPicHeader->getExtRoiPra()->setRoiAxisy(uiCode);
    xReadBit(uiCode);
    xReadCodeVlc(6, uiCode); pcPicHeader->getExtRoiPra()->setRoiWidth(uiCode);
    xReadBit(uiCode);
    xReadCodeVlc(6, uiCode); pcPicHeader->getExtRoiPra()->setRoiHeight(uiCode);
    xReadBit(uiCode);
    (pcPicHeader->getExtRoiPra()->getROIInfo() + roiIndex)->asisx = pcPicHeader->getExtRoiPra()->getRoiAxisx();
    (pcPicHeader->getExtRoiPra()->getROIInfo() + roiIndex)->asisy = pcPicHeader->getExtRoiPra()->getRoiAxisy();
    (pcPicHeader->getExtRoiPra()->getROIInfo() + roiIndex)->width = pcPicHeader->getExtRoiPra()->getRoiWidth();
    (pcPicHeader->getExtRoiPra()->getROIInfo() + roiIndex)->height = pcPicHeader->getExtRoiPra()->getRoiHeight();
    roiIndex++;
  }
  for (i = 0; i < roiIndex; i++)
  {
    (pcPicHeader->getExtRoiPra()->getPrevROIInfo() + i)->asisx = (pcPicHeader->getExtRoiPra()->getROIInfo() + i)->asisx;
    (pcPicHeader->getExtRoiPra()->getPrevROIInfo() + i)->asisy = (pcPicHeader->getExtRoiPra()->getROIInfo() + i)->asisy;
    (pcPicHeader->getExtRoiPra()->getPrevROIInfo() + i)->width = (pcPicHeader->getExtRoiPra()->getROIInfo() + i)->width;
    (pcPicHeader->getExtRoiPra()->getPrevROIInfo() + i)->height = (pcPicHeader->getExtRoiPra()->getROIInfo() + i)->height;
  }
}

Void TDecSbac::parseExtPicDisplay(TComPicHeader*& rpcPicHeader)
{
  UInt uiCode;
  Int i, NumberOfFrameCentreOffsets;
#if AVS3_EXTENSION_DEBUG_SYC
  xReadCodeVlc(4, uiCode);
  rpcPicHeader->getExtPicDisplay()->setExtId(uiCode);					//f(4), "extension_id", 
#else
  rpcPicHeader->getExtPicDisplay()->setExtId(7);
#endif

  if (rpcPicHeader->getSeqHeader()->getProgressiveSequence() == 1)
  {
    if (rpcPicHeader->getRepeatFirstField() == 1)
    {
      if (rpcPicHeader->getTopFieldFirst() == 1)
      {
        NumberOfFrameCentreOffsets = 3;
      }
      else
      {
        NumberOfFrameCentreOffsets = 2;
      }
    }
    else
    {
      NumberOfFrameCentreOffsets = 1;
    }
  }
  else
  {
    if (rpcPicHeader->getRepeatFirstField() == 1)
    {
      NumberOfFrameCentreOffsets = 3;
    }
    else
    {
      NumberOfFrameCentreOffsets = 2;
    }
  }
  for (i = 0; i < NumberOfFrameCentreOffsets; i++)
  {
    xReadCodeVlc(16, uiCode); rpcPicHeader->getExtPicDisplay()->setPicCentreHorOffset(uiCode);
    xReadBit(uiCode);
    xReadCodeVlc(16, uiCode); rpcPicHeader->getExtPicDisplay()->setPicCentreVerOffset(uiCode);
    xReadBit(uiCode);
  }
}
#endif

#if AVS3_SLICE_HEADER_SYC
Void TDecSbac::parsePictureHeader (TComPicture*& rpcPicture, TComPicHeader*& rpcPicHeader)
#else
Void TDecSbac::parsePictureHeader (TComPicture*& rpcPicture)
#endif
{
  UInt  uiCode;
  Int   iCode, iCode2;
#if AVS3_SLICE_HEADER_SYC
  xReadCodeVlc (32, uiCode);  rpcPicture->setSliceStartCode   (uiCode);
  if (rpcPicture->getSPS()->getVerticalSize() > (144 * rpcPicture->getSPS()->getLCUSize()))
  {
    xReadCodeVlc (3, uiCode);  rpcPicture->setSliceVerPosExtension   (uiCode);
  }
  xReadCodeVlc (8, uiCode);  rpcPicture->setSliceHorPos   (uiCode);
  if (rpcPicture->getSPS()->getHorizontalSize() > (255 * rpcPicture->getSPS()->getLCUSize()))
  {
    xReadCodeVlc (2, uiCode);  rpcPicture->setSliceHorPosExtension   (uiCode);
  }
  if(rpcPicHeader->getFixedPicQP() == false)///
  {
    xReadFlagVlc (uiCode);   rpcPicture->setFixedSliceQP   (uiCode ? true: false);
    xReadCodeVlc (7, uiCode);  rpcPicture->setSliceQP   (uiCode);
  }
  if (rpcPicture->getSPS()->getSampleAdaptiveOffsetEnableFlag())
  {
    for (UInt uicompIdx = 0; uicompIdx < 3; uicompIdx ++)
    {
      xReadFlagVlc (uiCode);   rpcPicture->setSliceSaoEnableFlag   (uicompIdx, uiCode ? true: false);
    }
  }
#endif
  xReadCodeVlc ( 2, uiCode ); //NalRefIdc
  xReadCodeVlc ( 1, uiCode ); assert( 0 == uiCode); // zero bit
  xReadCodeVlc ( 5, uiCode ); assert( NAL_UNIT_CODED_PICTURE == uiCode);//NalUnitType
 


#if RPS_BUG
  rpcPicture->setPOC(rpcPicHeader->getPOC());
  rpcPicture->setPictureType(rpcPicHeader->getPictureType());
#else
  xReadCodeVlc (10, uiCode);  rpcPicture->setPOC              (uiCode);             // 9 == SPS->Log2MaxFrameNum()
  xReadUvlc (   uiCode);  rpcPicture->setPictureType        ((PictureType)uiCode);
#endif
  xReadSvlc (iCode); xReadSvlc(iCode2);
  rpcPicture->setPictureQp(iCode, iCode - iCode2);
  
  if (!rpcPicture->isIntra())
    xReadFlagVlc (   uiCode);
  else
    uiCode = 1;
  
  rpcPicture->setReferenced       (uiCode ? true : false);
  
  xReadFlagVlc (uiCode); rpcPicture->setLoopFilterDisable(uiCode ? 1 : 0);
  xReadFlagVlc (uiCode); rpcPicture->setDRBFlag          (uiCode ? 1 : 0);

  return;
}

Void TDecSbac::parseTerminatingBit( UInt& ruiSymbol )
{
#if wlq_AEC
	xDecodeBinF(ruiSymbol);
#else

  m_uiRange--;

  if ( m_uiValue < m_uiRange ) 
  {
    ruiSymbol = 0;
    if ( m_uiRange >= 0x8000 ) 
    {
      return;
    }
  }
  else 
  {
    ruiSymbol = 1;
    m_uiValue -= m_uiRange;
    m_uiRange = 1;
    return;
  }

  do 
  {
    m_uiRange <<= 1;
    xReadBit(m_uiValue);
  } while ( m_uiRange < 0x8000 );
 #endif
}

Void TDecSbac::parseSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
#if !YQH_SPLIGFLAG_BUG
  if( uiDepth == g_uiMaxCUDepth - 1 )
  {
    pcCU->setDepthSubParts( uiDepth, uiAbsPartIdx );
    return;
  }
#endif 
  UInt uiSymbol;
#if YQH_SPLIGFLAG_BUG

#if YQH_SPLIGFLAG_DEC
  UInt SizeInBit = g_uiLog2MaxCUSize - uiDepth;
  xDecodeBin(uiSymbol, m_cCUSplitFlagSCModel.get(0, 0, SizeInBit));//yuquanhe@hisilicon.com
  pcCU->setDepthSubParts( uiDepth + uiSymbol, uiAbsPartIdx );

  /* FILE *ResiUV = fopen("AVS3_DEC.txt", "at+");
  fprintf(ResiUV,"sym=%d %d \n ",uiSymbol,SizeInBit);
  fclose(ResiUV);*/
#else
  UInt SizeInBit = pcCU->getLog2CUSize(uiAbsPartIdx);
  xDecodeBin(uiSymbol, m_cCUSplitFlagSCModel.get(0, 0, SizeInBit));//yuquanhe@hisilicon.com
#endif



#else

 
  #if YQH_INTRA
  xDecodeBin( uiSymbol, m_cCUSplitFlagSCModel.get( 0, 0, uiDepth ) );//yuquanhe@hisilicon.com
#else
   xDecodeBin( uiSymbol, m_cCUSplitFlagSCModel.get( 0, 0, 0 ) );
#endif
  pcCU->setDepthSubParts( uiDepth + uiSymbol, uiAbsPartIdx );
#endif  
  return;
}

Void TDecSbac::parseTransformIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  xDecodeBin( uiSymbol, m_cCUTransIdxSCModel.get( 0, 0, 0 ) );

  if( uiSymbol > 0 && pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxnU && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N )
  {
	  //test
    //uiSymbol = 2;
  }

  pcCU->setTrIdxSubParts( uiSymbol, uiAbsPartIdx, uiDepth );
}

#if F_DHP_SYC
Void TDecSbac::parseInterDHP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)//FPuTypeIndex
{
	UInt uiTempPartIdx = uiAbsPartIdx;
	UInt uiPartOffset = (pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1)) >> 2;
	//开辟pdir空间
	switch (pcCU->getPartitionSize(uiAbsPartIdx))
	{
	case SIZE_2Nx2N:
	{
		uiTempPartIdx = uiAbsPartIdx;
		break;
	}
	case SIZE_2NxN:
	{
		uiTempPartIdx = uiAbsPartIdx + (uiPartOffset << 1);
		break;
	}
	case SIZE_Nx2N:
	{
		uiTempPartIdx = uiAbsPartIdx + uiPartOffset;
		break;
	}
	case SIZE_2NxnU:
	{
		uiTempPartIdx = uiAbsPartIdx + (uiPartOffset >> 1);
		break;
	}
	case SIZE_2NxnD:
	{
		uiTempPartIdx = uiAbsPartIdx + (uiPartOffset << 1) + (uiPartOffset >> 1);
		break;
	}
	case SIZE_nLx2N:
	{
		uiTempPartIdx = uiAbsPartIdx + (uiPartOffset >> 2);
		break;
	}
	case SIZE_nRx2N:
	{
		uiTempPartIdx = uiAbsPartIdx + uiPartOffset + (uiPartOffset >> 2);
		break;
	}
	case SIZE_NxN:
	{
		//no NXN
		assert(0);
		break;
	}
	default:
		assert(0);
		break;
	}
	if (pcCU->getLog2CUSize(uiAbsPartIdx) == B8X8_IN_BIT && pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxN && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N)
	{
		pcCU->setInterDirSubParts(INTER_FORWARD, uiAbsPartIdx, uiDepth, 0);
		pcCU->setInterDirSubParts(INTER_FORWARD, uiTempPartIdx, uiDepth, 1);
		//pcCU->setFPuTypeIndex(0,1);
		return;
	}
	UInt act_ctx;
	UInt act_sym;
	UInt symbol;
	UInt pdir0, pdir1;
	static const UInt dir2offset[2][2] = { { 0, 1 }, { 2, 3 } };

	//BiContextTypePtr pCTX = (img->currentSlice)->syn_ctx->pdir_dhp_contexts;

	int counter = 0;
	int terminate_bit = 0;
	act_ctx = 0;
	act_sym = 0;
	if (pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N) {
		//act_sym = biari_decode_symbol(dep_dp, pCTX + act_ctx);
		xDecodeBin(act_sym, m_cCUInterDHPSCModel.get(0, 0, act_ctx));

		pdir0 = act_sym;
		//se->value1 = act_sym;
		pcCU->setInterDirSubParts(act_sym, uiAbsPartIdx, uiDepth, 0);//此处DHM尚未判断
		//pcCU->setInterDir(0, act_sym);
	}
	else if (pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxN || pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N) {
		counter = 0;
		terminate_bit = 0;
		act_ctx = 0;
		act_sym = 0;

		//act_sym = biari_decode_symbol(dep_dp, pCTX + act_ctx + 1);
		xDecodeBin(act_sym, m_cCUInterDHPSCModel.get(0, 0, act_ctx + 1));
		pdir0 = act_sym;

		//symbol = biari_decode_symbol(dep_dp, pCTX + act_ctx + 2);
		xDecodeBin(symbol, m_cCUInterDHPSCModel.get(0, 0, act_ctx + 2));
		if (symbol) {
			pdir1 = pdir0;
		}
		else {
			pdir1 = 1 - pdir0;
		}
		//se->value1 = dir2offset[pdir0][pdir1];

		switch (dir2offset[pdir0][pdir1])
		{
		case 0:
			pcCU->setInterDirSubParts(INTER_DUAL, uiAbsPartIdx, uiDepth, 0);
			pcCU->setInterDirSubParts(INTER_DUAL, uiTempPartIdx, uiDepth, 1);
			break;
		case 1:
			pcCU->setInterDirSubParts(INTER_DUAL, uiAbsPartIdx, uiDepth, 0);
			pcCU->setInterDirSubParts(INTER_FORWARD, uiTempPartIdx, uiDepth, 1);
			break;
		case 2:
			pcCU->setInterDirSubParts(INTER_FORWARD, uiAbsPartIdx, uiDepth, 0);
			pcCU->setInterDirSubParts(INTER_DUAL, uiTempPartIdx, uiDepth, 1);
			break;
		case 3:
			pcCU->setInterDirSubParts(INTER_FORWARD, uiAbsPartIdx, uiDepth, 0);
			pcCU->setInterDirSubParts(INTER_FORWARD, uiTempPartIdx, uiDepth, 1);
			break;
		default:
			break;
		}
	}
	return;
}
Void TDecSbac::parseInterDHP_FNXN(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)//FPuTypeIndex2
{
	UInt act_sym = 0;

	//BiContextTypePtr pCTX = (img->currentSlice)->syn_ctx->b8_type_dhp_contexts;
	for (Int i = 0; i < 4; i++)
	{
		xDecodeBin(act_sym, m_cCUInterDHPNXNSCModel.get(0, 0, 0));
		UInt uiPartOffset = (pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1)) >> 2;
		if (act_sym == 1)
		{
			pcCU->setInterDirSubParts(INTER_DUAL, uiAbsPartIdx, uiDepth, 0);
		}
		else
		{
			pcCU->setInterDirSubParts(INTER_FORWARD, uiAbsPartIdx, uiDepth, 0);
		}
	}
	//act_sym = biari_decode_symbol(dep_dp, pCTX);
	//se->value1 = act_sym;
	return;
}
#endif
#if	inter_direct_skip_bug2
Void TDecSbac::parseInterWSM(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	
	UInt binIdx = 0;
	UInt act_sym = 0;
	UInt uiSymbol=0;
	while (act_sym < pcCU->getPicture()->getPicHeader()->getInterSkipmodeNumber() - 1)
	{
		xDecodeBin(uiSymbol, m_cCUInterWSMSCModel.get(0, 0, binIdx));
		
		if (uiSymbol)
			break;
		act_sym++;
		binIdx = binIdx > 2 ? 2 : binIdx + 1;
	
	}

	pcCU->setInterSkipmodeSubParts(act_sym, uiAbsPartIdx, uiDepth);
	return;
}
#endif
#if	F_MHPSKIP_SYC
Void TDecSbac::parseInterMHPSKIP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	//BiContextTypePtr pCTX = (img->currentSlice)->syn_ctx->p_skip_mode_contexts;
	UInt act_ctx = 0;
	UInt act_sym = 0;
	UInt counter = 0;
	UInt terminate_bit = 0;

	xDecodeBin(terminate_bit, m_cCUInterMHPSKIPSCModel.get(0, 0, act_ctx));
	while (counter < MH_PSKIP_NUM - 1 && terminate_bit == 0) {
		act_sym++;
		act_ctx++;
		counter++;
		if (counter < MH_PSKIP_NUM - 1)
			xDecodeBin(terminate_bit, m_cCUInterMHPSKIPSCModel.get(0, 0, act_ctx));
	}

	UInt temp;
	if (!terminate_bit) {
		xDecodeBin(temp, m_cCUInterMHPSKIPSCModel.get(0, 0, act_ctx));
		act_sym += (!temp);
	}

	//se->value1 = act_sym;
	if (act_sym == 0) {
		pcCU->setInterSkipmodeSubParts(act_sym, uiAbsPartIdx, uiDepth);
	}
	else
	{
		pcCU->setInterSkipmodeSubParts(3 + act_sym, uiAbsPartIdx, uiDepth);
	}

	return;
}
#endif
#if	B_MHBSKIP_SYC
Void TDecSbac::parseInterMHBSKIP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	//BiContextTypePtr pCTX = (img->currentSlice)->syn_ctx->p_skip_mode_contexts;
	UInt act_ctx = 0;
	UInt act_sym = 0;
	UInt counter = 0;
	UInt terminate_bit = 0;

	xDecodeBin(terminate_bit, m_cCUInterMHBSKIPSCModel.get(0, 0, act_ctx));
	while (counter < DIRECTION - 1 && terminate_bit == 0) {
		act_sym++;
		act_ctx++;
		counter++;
		if (counter < DIRECTION - 1)
			xDecodeBin(terminate_bit, m_cCUInterMHBSKIPSCModel.get(0, 0, act_ctx));
	}

	UInt temp;
	if (!terminate_bit) {
		xDecodeBin(temp, m_cCUInterMHBSKIPSCModel.get(0, 0, act_ctx));
		act_sym += (!temp);
	}

	//se->value1 = act_sym;
	if (act_sym == 0) {
		pcCU->setInterSkipmodeSubParts(act_sym, uiAbsPartIdx, uiDepth);
	}
	else
	{
		pcCU->setInterSkipmodeSubParts(3 + act_sym, uiAbsPartIdx, uiDepth);
	}

	return;
}
#endif
Void TDecSbac::parsePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  pcCU->setSizeSubParts( 1<<(g_uiLog2MaxCUSize - uiDepth), 1<<(g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth );

  if( pcCU->getPicture()->isIntra() )
  {
    pcCU->setPredModeSubParts( MODE_INTRA, uiAbsPartIdx, uiDepth );
    return;
  }
#if  inter_intra_1
  return;
#endif
  UInt uiSymbol;
  Int  iPredMode = MODE_INTER;

  xDecodeBin( uiSymbol, m_cCUPredModeSCModel.get( 0, 0, 0 ) );

  if ( uiSymbol )
  {
    iPredMode = MODE_SKIP;
    pcCU->setTrIdxSubParts( 0, uiAbsPartIdx, uiDepth );
  }
  else
  {
    xDecodeBin( uiSymbol, m_cCUPredModeSCModel.get( 0, 0, 1 ) );
    iPredMode = ( uiSymbol ? MODE_INTRA : MODE_INTER );
  }
  
  pcCU->setPredModeSubParts( (PredMode)iPredMode, uiAbsPartIdx, uiDepth );
}
#if YQH_INTRA
PartSize TDecSbac::parseIntraPuTypeIdx(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)//yuquanhe@hisilicon.com
{
	UInt uiSymbol, uiMode = 0;
	PartSize eMode = pcCU->getPartitionSize(uiAbsPartIdx);

	// for test
	UInt uiPocTest = pcCU->getPicture()->getPOC();

	if (pcCU->isIntra(uiAbsPartIdx))
	{
		PictureType cuPicType = pcCU->getPicture()->getPictureType();
		if ((pcCU->isSkip(uiAbsPartIdx)) && ((cuPicType == P_PICTURE) || (cuPicType == B_PICTURE) || (cuPicType == F_PICTURE))) //zhangyiCheck: influence inter or not??
		{
			return SIZE_2Nx2N;
		}
		UInt SizeInBit = pcCU->getLog2CUSize(uiAbsPartIdx);
		eMode = pcCU->getPartitionSize(uiAbsPartIdx);
		if ((((pcCU->getPicture()->getSPS()->getNonsquareIntraPredictionEnableFlag() == 1) && ((SizeInBit > 3) && (SizeInBit < 6)))) && eMode != SIZE_2Nx2N)
		{

			xDecodeBin(uiSymbol, m_cPUIntraTypeSCModel.get(0, 0, 0));
			eMode = uiSymbol ? SIZE_2NxhN : SIZE_hNx2N;
#if !ZHANGYI_INTRA_SDIP
			printf("error\n");
#endif

		}
		if (eMode != SIZE_2Nx2N)
		{
			pcCU->setTrIdxSubParts(1, uiAbsPartIdx, uiDepth);
		}
		else
		{
			pcCU->setTrIdxSubParts(0, uiAbsPartIdx, uiDepth);
		}


	}
	return eMode;
}


PartSize TDecSbac::parseTransSplitFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)//yuquanhe@hisilicon.com
{
	if (pcCU->isSkip(uiAbsPartIdx))
	{
		pcCU->setPartSizeSubParts(SIZE_2Nx2N, uiAbsPartIdx, uiDepth);
		pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
		return SIZE_2Nx2N;
	}

	UInt uiSymbol, uiMode = 0;
	PartSize eMode = SIZE_2Nx2N;

	// for test
	UInt uiPocTest = pcCU->getPicture()->getPOC();

	if (pcCU->isIntra(uiAbsPartIdx))
	{
#if ZHANGYI_INTRA
		PictureType cuPicType = pcCU->getPicture()->getPictureType();
		if ((pcCU->isSkip(uiAbsPartIdx)) && ((cuPicType == P_PICTURE) || (cuPicType == B_PICTURE) || (cuPicType == F_PICTURE))) //zhangyiCheck: influence inter or not??
		{
			return SIZE_2Nx2N;
		}
		UInt SizeInBit = pcCU->getLog2CUSize(uiAbsPartIdx);
		if ((SizeInBit == 3) || (((pcCU->getPicture()->getSPS()->getNonsquareIntraPredictionEnableFlag() == 1) && ((SizeInBit > 3) && (SizeInBit < 6)))))
		{
#endif


			PartSize eSize = SIZE_2Nx2N;
#if ZHANGYI_MODIFY_CTX_OF_SPLITFLAG
			UInt act_ctx = (eSize == SIZE_2Nx2N || eSize == SIZE_NxN || eSize == SIZE_2NxhN || eSize == SIZE_hNx2N) ? 1 : 0;
#else
			UInt act_ctx = (eSize == SIZE_2Nx2N || eSize == SIZE_NxN || eSize == SIZE_2NxhN || eSize == SIZE_hNx2N) ? 0 : 1;
#endif
			if ((SizeInBit == 4) || (SizeInBit == 5))
				act_ctx++;
			assert(act_ctx<3);
#if ZHANGYI_MODIFY_CTX_OF_SPLITFLAG
			xDecodeBin(uiSymbol, m_cTUSplitFlagSCModel.get(0, 0, act_ctx));
#else
			xDecodeBin(uiSymbol, m_cTUSplitFlagSCModel.get(0, 0, 0));
#endif
			eMode = uiSymbol ? SIZE_NxN : SIZE_2Nx2N;

			if (eMode == SIZE_NxN)
			{
				pcCU->setTrIdxSubParts(1, uiAbsPartIdx, uiDepth);
			}
			else
			{
				pcCU->setTrIdxSubParts(0, uiAbsPartIdx, uiDepth);
			}
#if ZHANGYI_INTRA
		}
		else
		{
			eMode = SIZE_2Nx2N; //zhangyiCheck: 
			pcCU->setTrIdxSubParts(0, uiAbsPartIdx, uiDepth);
		}
		pcCU->setPartSizeSubParts(eMode, uiAbsPartIdx, uiDepth);
		return eMode;
#endif
	}
	return eMode;
}

#endif

#if INTER_GROUP
Void TDecSbac::parseInterCUTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
#if YQH_SPLIGFLAG_DEC_BUG2

	pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
#if	!inter_intra_1
	if (pcCU->getPicture()->isIntra())
	{
		pcCU->setPredModeSubParts(MODE_INTRA, uiAbsPartIdx, uiDepth);
		return;
	}

#endif
#endif

#if inter_intra_1

	int act_ctx;
	int act_sym;
	int curr_cuType;


	//const int MapPCUType[7] = {-1,0, 9, 1, 2, PNXN, 3};
	const int MapPCUType[7] = { -1, 0, 1, 2, 3, 8, 9 };

	//    const int MapBCUType[7] = {-1, 1, 2, 3, PNXN, 0, 9}; //{0, 1, 2, 3, 4, 10, 9};
	const int MapBCUType[7] = { -1, 0, 1, 2, 3, 8, 9 }; //{0, 1, 2, 3, 4, 10, 9};

	const int MapPCUTypeMin[6] = { -1, 0, 1, 2, 3, 9 };
	const int MapBCUTypeMin[6] = { -1, 0, 1, 2, 3, 9 };




	int  binidx = 0;
	UInt symbol;


	int max_bit = 0;

	//if (img->type != I_IMG) 
	if (pcCU->getPicture()->getPictureType() != I_PICTURE)
	{   // INTRA-frame
		act_ctx = 0;
		act_sym = 0;


		if (pcCU->getLog2CUSize(uiAbsPartIdx) == B8X8_IN_BIT) {
			max_bit = 5;
		}
		else {
			max_bit = 6;
		}



		while (1) {
			if ((binidx == 5) && (pcCU->getLog2CUSize(uiAbsPartIdx) != MIN_CU_SIZE_IN_BIT)) {
				//symbol = biari_decode_final(dep_dp);
				xDecodeBinF(symbol);
			}
			else {
				//symbol = biari_decode_symbol(dep_dp, pCTX + act_ctx);
				xDecodeBin(symbol, m_cCUPartSizeSCModel.get(0, 0, act_ctx));
			}
			binidx++;



			if (symbol == 0) {
				act_sym++;
				act_ctx++;

				if (act_ctx >= 5) {
					act_ctx = 5;
				}

			}
			else {
				if (binidx == 1)
				{
					pcCU->setPredModeSubParts(MODE_SKIP, uiAbsPartIdx, uiDepth);
					pcCU->setPartSizeSubParts(SIZE_2Nx2N, uiAbsPartIdx, uiDepth);
					pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
					return;
				}
				break;
			}
			if (act_sym >= max_bit) {
				break;
			}
		}
		//if (((img->type == F_IMG)) || ((img->type == P_IMG))) 
		if ((pcCU->getPicture()->getPictureType() == F_PICTURE) || (pcCU->getPicture()->getPictureType() == P_PICTURE))

		{
			if (pcCU->getLog2CUSize(uiAbsPartIdx) == B8X8_IN_BIT) {
				act_sym = MapPCUTypeMin[act_sym];
			}
			else {
				act_sym = MapPCUType[act_sym];
			}
		}
		else {
			if (pcCU->getLog2CUSize(uiAbsPartIdx) == B8X8_IN_BIT) {
				act_sym = MapBCUTypeMin[act_sym];
			}
			else {
				act_sym = MapBCUType[act_sym];
			}
		}

		curr_cuType = act_sym;

		//for AMP


		if (pcCU->getLog2CUSize(uiAbsPartIdx) >= B16X16_IN_BIT)
		{
			//	if (pcCU->getPicture()->getSPS()->getAsymmetricMotionPartitionsEnableFlag()) 
			{
				if (curr_cuType == 2 || curr_cuType == 3)
				{
					xDecodeBin(symbol, m_cCUPartSizeSCModel.get(0, 0, 8));
					if (!symbol) {
						UInt symboltem;
						xDecodeBin(symboltem, m_cCUPartSizeSCModel.get(0, 0, 9));
						curr_cuType = curr_cuType * 2 + (!symboltem);
					}
				}
			}
		}
#if    !inter_intra_2
		curr_cuType = curr_cuType + (pcCU->getPicture()->getPictureType() == B_PICTURE ? 1 : 0);


		if ((pcCU->getPicture()->getPictureType() == F_PICTURE) || (pcCU->getPicture()->getPictureType() == P_PICTURE))
		{
			curr_cuType++;
		}

		curr_cuType--;
#endif


	}
	else {
		curr_cuType = 9;
	}



	//UInt partindx = eMode == SIZE_2Nx2N ? 1 : eMode == SIZE_2NxN ? 2 : eMode == SIZE_Nx2N ? 3 : eMode == SIZE_2NxnU ? 4 : eMode == SIZE_2NxnD ? 5 : eMode == SIZE_nLx2N ? 6 : eMode == SIZE_nRx2N ? 7 : 0;
	PartSize eMode = curr_cuType == 1 ? SIZE_2Nx2N : curr_cuType == 2 ? SIZE_2NxN : curr_cuType == 3 ? SIZE_Nx2N : curr_cuType == 4 ? SIZE_2NxnU : curr_cuType == 5 ? SIZE_2NxnD : curr_cuType == 6 ? SIZE_nLx2N : curr_cuType == 7 ? SIZE_nRx2N : SIZE_NONE;

	if (curr_cuType == 0)
	{
		pcCU->setPredModeSubParts(MODE_DIRECT, uiAbsPartIdx, uiDepth);
		pcCU->setPartSizeSubParts(SIZE_2Nx2N, uiAbsPartIdx, uiDepth);
		pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
	}
	else if (curr_cuType == 10)
	{

		pcCU->setPredModeSubParts(MODE_INTRA, uiAbsPartIdx, uiDepth);
		pcCU->setPartSizeSubParts(SIZE_2Nx2N, uiAbsPartIdx, uiDepth);

	}
	else if (curr_cuType == 9)
	{

		pcCU->setPredModeSubParts(MODE_INTRA, uiAbsPartIdx, uiDepth);
		pcCU->setPartSizeSubParts(SIZE_NxN, uiAbsPartIdx, uiDepth);

	}
	else
	{
		pcCU->setPredModeSubParts(MODE_INTER, uiAbsPartIdx, uiDepth);
		pcCU->setPartSizeSubParts(eMode, uiAbsPartIdx, uiDepth);
		pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);

	}

	pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
	return;
#else

#if niu_cutype_dec

	int act_ctx;
	int act_sym;
	int curr_cuType;

	//const int MapPCUType[7] = {-1,0, 9, 1, 2, PNXN, 3};
	const int MapPCUType[7] = { -1, 0, 1, 2, 3, 8, 9 };

	//    const int MapBCUType[7] = {-1, 1, 2, 3, PNXN, 0, 9}; //{0, 1, 2, 3, 4, 10, 9};
	const int MapBCUType[7] = { -1, 0, 1, 2, 3, 8, 9 }; //{0, 1, 2, 3, 4, 10, 9};

	const int MapPCUTypeMin[6] = { -1, 0, 1, 2, 3, 9 };
	const int MapBCUTypeMin[6] = { -1, 0, 1, 2, 3, 9 };

	int  binidx = 0;
	UInt symbol;

	UInt symbol1, symbol2, symbol3;
	int max_bit = 0;

	if (pcCU->getPicture()->getPictureType() != I_PICTURE) 
	{   


	//	pcCU->setPredModeSubParts(MODE_INTER, uiAbsPartIdx, uiDepth);
	//	pcCU->setPartSizeSubParts(SIZE_2NxN, uiAbsPartIdx, uiDepth);

	//	pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);

	//	return;

		PartSize curr_cuType_temp;
		xDecodeBin(symbol, m_cCUPartSizeSCModel.get(0, 0, 0));
		symbol1 = symbol;
		if (symbol == 1)
		{
			pcCU->setPredModeSubParts(MODE_SKIP, uiAbsPartIdx, uiDepth);
			#if	RPS_BUG_YQH
			pcCU->setPartSizeSubParts(SIZE_2Nx2N, uiAbsPartIdx, uiDepth);
#else
			pcCU->setPartSizeSubParts(SIZE_NONE, uiAbsPartIdx, uiDepth);
#endif
			pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
		//	printf("error MODE_SKIP");
			return;
		}
		else
		{
			xDecodeBin(symbol, m_cCUPartSizeSCModel.get(0, 0, 1));
			symbol2 = symbol; 
			if (symbol)
			{
				
				pcCU->setPredModeSubParts(MODE_DIRECT, uiAbsPartIdx, uiDepth);
#if	RPS_BUG_YQH
				pcCU->setPartSizeSubParts(SIZE_2Nx2N, uiAbsPartIdx, uiDepth);
#else
				pcCU->setPartSizeSubParts(SIZE_NONE, uiAbsPartIdx, uiDepth);
#endif
				pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
			//	printf("error MODE_DIRECT");
				return;
			}
		}

		xDecodeBin(symbol3, m_cCUPartSizeSCModel.get(0, 0, 2));

		if (symbol1 == 0 && symbol2 == 0 && symbol3 == 1)
		{
			curr_cuType_temp = SIZE_2Nx2N;
		}
		else if (symbol1 == 0 && symbol2 == 0 && symbol3 == 0)
		{
			xDecodeBin(symbol, m_cCUPartSizeSCModel.get(0, 0, 3));
			if (symbol) //SIZE_2NxN SIZE_2NxnU SIZE_2NxnD
			{
				curr_cuType_temp = SIZE_2NxN;
				//if (pcCU->getLog2CUSize(uiAbsPartIdx) >= B16X16_IN_BIT)
				if(	pcCU->getWidth(uiAbsPartIdx) >= 16)
				{
					xDecodeBin(symbol, m_cCUShapeOfPartitionSCModel.get(0, 0, 0));
					if (symbol)
					{
						curr_cuType_temp = SIZE_2NxN;
					}
					else
					{
						xDecodeBin(symbol, m_cCUShapeOfPartitionSCModel.get(0, 0, 1));
						if (symbol)
						{
							curr_cuType_temp = SIZE_2NxnU;
						}
						else
						{
							curr_cuType_temp = SIZE_2NxnD;
						}

					}

				}
			}
			else //SIZE_Nx2N  SIZE_nLx2N SIZE_nRx2N
			{
				xDecodeBin(symbol, m_cCUPartSizeSCModel.get(0, 0, 4));
				if (symbol)
				{
					curr_cuType_temp = SIZE_Nx2N;
					//if (pcCU->getLog2CUSize(uiAbsPartIdx) >= B16X16_IN_BIT)
					if ((pcCU->getHeight(uiAbsPartIdx) >= 16))
					{
						xDecodeBin(symbol, m_cCUShapeOfPartitionSCModel.get(0, 0, 0));
						if (symbol)
						{
							curr_cuType_temp = SIZE_Nx2N;
						}
						else
						{
							xDecodeBin(symbol, m_cCUShapeOfPartitionSCModel.get(0, 0, 1));
							if (symbol)
							{
								curr_cuType_temp = SIZE_nLx2N;
							}
							else
							{
								curr_cuType_temp = SIZE_nRx2N;
							}

						}

					}

				}
				else
				{
					xDecodeBin(symbol, m_cCUPartSizeSCModel.get(0, 0, 5));
					if (symbol)
						curr_cuType_temp = SIZE_NxN;
					else
						curr_cuType_temp = SIZE_NONE;
				}
			}
		}

		else
		{
			printf("error cutype \n");
		}


		pcCU->setPredModeSubParts(MODE_INTER, uiAbsPartIdx, uiDepth);
		pcCU->setPartSizeSubParts(curr_cuType_temp, uiAbsPartIdx, uiDepth);
		if (curr_cuType_temp == SIZE_2NxhN)
			printf("error,SIZE_2NxhN");

		pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);

		return;

		act_ctx = 0;
		act_sym = 0;

		//test
		//pcCU->getWigth(0) == pcCU->getLog2CUSize(0)  ??
		if (pcCU->getLog2CUSize(0) == B8X8_IN_BIT) 
		{
			max_bit = 5;
		}
		else
		{
			max_bit = 6;
		}
		while (1) 
		{
			if ((binidx == 5) && (pcCU->getLog2CUSize(0) != MIN_CU_SIZE_IN_BIT)) 
			{
				xDecodeBinF(symbol);
			}
			else
			{
				xDecodeBin(symbol, m_cCUPartSizeSCModel.get(0, 0, act_ctx));
			}
			binidx++;

			if (symbol == 0) 
			{
				act_sym++;
				act_ctx++;
				if (act_ctx >= 5) 
				{
					act_ctx = 5;
				}
			}
			else 
			{
				break;
			}
			if (act_sym >= max_bit) 
			{
				break;
			}
		}
		if (((pcCU->getPicture()->getPictureType() == F_PICTURE)) || ((pcCU->getPicture()->getPictureType() == P_PICTURE)))
		{
			if (pcCU->getLog2CUSize(0) == B8X8_IN_BIT)
			{
				act_sym = MapPCUTypeMin[act_sym];
			}
			else 
			{
				act_sym = MapPCUType[act_sym];
			}
		}
		else 
		{
			if (pcCU->getLog2CUSize(0) == B8X8_IN_BIT)
			{
				act_sym = MapBCUTypeMin[act_sym];
			}
			else {
				act_sym = MapBCUType[act_sym];
			}
		}

		curr_cuType = act_sym;

		//for AMP
		if (pcCU->getLog2CUSize(0) >= B16X16_IN_BIT)
		{
			//test
			//if (pcCU->getPicture()->getSPS()->getAsymmetricMotionPartitionsEnableFlag())
			{
				if (curr_cuType == 2 || curr_cuType == 3)
				{
					xDecodeBin(symbol, m_cCUShapeOfPartitionSCModel.get(0, 0, 0));
					if (!symbol)
					{
						xDecodeBin(symbol, m_cCUShapeOfPartitionSCModel.get(0, 0, 1));
						curr_cuType = curr_cuType * 2 + (!symbol);
						//test  
						//curr_cuType = curr_cuType - 1;
					}
				}
			}
		}
		//test   两边的索引偏差为1
		curr_cuType = curr_cuType - 1;

		curr_cuType = curr_cuType + (pcCU->getPicture()->getPictureType() == B_PICTURE);
		pcCU->setPredModeSubParts(MODE_INTER, uiAbsPartIdx, uiDepth);
		pcCU->setPartSizeSubParts(PartSize(curr_cuType), uiAbsPartIdx, uiDepth);
		if (PartSize(curr_cuType) == SIZE_2NxhN)
			printf("error,SIZE_2NxhN");

		pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);

		return;
	}
	





	//test     ???
	/*
	if (curr_cuType == -1)
	{
		pcCU->setPredModeSubParts(MODE_SKIP, uiAbsPartIdx, uiDepth);
		pcCU->setTrIdxSubParts(0, uiAbsPartIdx, uiDepth);
		pcCU->setPartSizeSubParts(SIZE_2Nx2N, uiAbsPartIdx, uiDepth);
		pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
		return;
	}
	else if (curr_cuType == 0)
	{
		pcCU->setPartSizeSubParts(SIZE_2Nx2N, uiAbsPartIdx, uiDepth);
		pcCU->setPredModeSubParts(MODE_DIRECT, uiAbsPartIdx, uiDepth);
		pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
		return;
	}
	else
	{
		pcCU->setPartSizeSubParts(PartSize(curr_cuType - 1), uiAbsPartIdx, uiDepth);
		pcCU->setPredModeSubParts(MODE_INTER, uiAbsPartIdx, uiDepth);
		pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
		return;
	}
	*/
#else

	UInt uiSymbol, uiMode = 0;
	//PartSize eMode;

	xDecodeBin(uiSymbol, m_cCUPartSizeSCModel.get(0, 0, 0));

	if (uiSymbol == 1) //skip mode
	{
		//pcCU->setPredictionMode(uiAbsPartIdx, MODE_SKIP);
		pcCU->setPredModeSubParts(MODE_SKIP, uiAbsPartIdx, uiDepth);
		pcCU->setTrIdxSubParts(0, uiAbsPartIdx, uiDepth);
		pcCU->setPartSizeSubParts(SIZE_2Nx2N, uiAbsPartIdx, uiDepth);
		pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
		return;
	}
	else
	{
		xDecodeBin(uiSymbol, m_cCUPartSizeSCModel.get(0, 0, 1));
		if (uiSymbol == 1) //direct mode
		{
			//pcCU->setPredictionMode(uiAbsPartIdx, MODE_DIRECT);
			pcCU->setPartSizeSubParts(SIZE_2Nx2N, uiAbsPartIdx, uiDepth);
			pcCU->setPredModeSubParts(MODE_DIRECT, uiAbsPartIdx, uiDepth);
			pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
			return;
		}
		else
		{
			xDecodeBin(uiSymbol, m_cCUPartSizeSCModel.get(0, 0, 2));
			if (uiSymbol == 1)
			{
				//eMode == SIZE_2Nx2N;
				pcCU->setPartitionSizeIndex(uiAbsPartIdx, 2);
				pcCU->setPartSizeSubParts(SIZE_2Nx2N, uiAbsPartIdx, uiDepth);
				pcCU->setPredModeSubParts(MODE_INTER, uiAbsPartIdx, uiDepth);
				pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
				return;
			}
			else
			{
				xDecodeBin(uiSymbol, m_cCUPartSizeSCModel.get(0, 0, 3));
				if (uiSymbol == 1)
				{
					pcCU->setPartitionSizeIndex(uiAbsPartIdx, 3);
					pcCU->setPredModeSubParts(MODE_INTER, uiAbsPartIdx, uiDepth);
					xDecodeBin(uiSymbol, m_cCUShapeOfPartitionSCModel.get(0, 0, 0));
					if (uiSymbol == 1)
					{
						pcCU->setPartSizeSubParts(SIZE_2NxN, uiAbsPartIdx, uiDepth);
						pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
						return;
					}
					else
					{
						xDecodeBin(uiSymbol, m_cCUShapeOfPartitionSCModel.get(0, 0, 1));
						if (uiSymbol == 1)
						{
							pcCU->setPartSizeSubParts(SIZE_2NxnU, uiAbsPartIdx, uiDepth);
							pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
							return;
						}
						else
						{
							pcCU->setPartSizeSubParts(SIZE_2NxnD, uiAbsPartIdx, uiDepth);
							pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
							return;
						}
					}
					return;
				}
				else
				{
					xDecodeBin(uiSymbol, m_cCUPartSizeSCModel.get(0, 0, 4));
					if (uiSymbol == 1)
					{
						pcCU->setPartitionSizeIndex(uiAbsPartIdx, 4);
						pcCU->setPredModeSubParts(MODE_INTER, uiAbsPartIdx, uiDepth);

						xDecodeBin(uiSymbol, m_cCUShapeOfPartitionSCModel.get(0, 0, 0));
						if (uiSymbol == 1)
						{
							pcCU->setPartSizeSubParts(SIZE_Nx2N, uiAbsPartIdx, uiDepth);
							pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
							return;
						}
						else
						{
							xDecodeBin(uiSymbol, m_cCUShapeOfPartitionSCModel.get(0, 0, 1));
							if (uiSymbol == 1)
							{
								pcCU->setPartSizeSubParts(SIZE_nLx2N, uiAbsPartIdx, uiDepth);
								pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
								return;
							}
							else
							{
								pcCU->setPartSizeSubParts(SIZE_nRx2N, uiAbsPartIdx, uiDepth);
								pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
								return;
							}
						}
					}
					else
					{
						if (pcCU->getLog2CUSize(uiAbsPartIdx) > 3)
						{
							xDecodeBin(uiSymbol, m_cCUPartSizeSCModel.get(0, 0, 5));
							if (uiSymbol)
							{
								pcCU->setPartitionSizeIndex(uiAbsPartIdx, 5);
								pcCU->setPartSizeSubParts(SIZE_NxN, uiAbsPartIdx, uiDepth);
								pcCU->setPredModeSubParts(MODE_INTER, uiAbsPartIdx, uiDepth);
								pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
							}
							else
							{
								//pcCU->setPredictionMode(uiAbsPartIdx, MODE_INTRA);
								pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
								pcCU->setPredModeSubParts(MODE_INTRA, uiAbsPartIdx, uiDepth);
								pcCU->setPartitionSizeIndex(uiAbsPartIdx, 6);
								parsePartSize(pcCU, uiAbsPartIdx, uiDepth);
							}
							return;
						}
						else
						{
							xDecodeBin(uiSymbol, m_cCUPartSizeSCModel.get(0, 0, 5));
							//pcCU->setPredictionMode(uiAbsPartIdx, MODE_INTRA);
							pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
							pcCU->setPredModeSubParts(MODE_INTRA, uiAbsPartIdx, uiDepth);
							pcCU->setPartitionSizeIndex(uiAbsPartIdx, 5);
							parsePartSize(pcCU, uiAbsPartIdx, uiDepth);
							//eMode == MODE_INTRA;
							return;
						}
					}
				}
			}
		}
	}
#endif
#endif
}

Void TDecSbac::parseShapeOfPartitionIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	UInt uiSymbol;

	xDecodeBin(uiSymbol, m_cCUShapeOfPartitionSCModel.get(0, 0, 0));
	if (uiSymbol == 1)
	{
		if (pcCU->getPartitionSizeIndex(uiAbsPartIdx) == 3)
		{
			pcCU->setPartSizeSubParts(SIZE_2NxN, uiAbsPartIdx, uiDepth);
			pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
			return;
		}
		else if (pcCU->getPartitionSizeIndex(uiAbsPartIdx) == 4)
		{
			pcCU->setPartSizeSubParts(SIZE_Nx2N, uiAbsPartIdx, uiDepth);
			pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
			return;
		}
	}
	else
	{
		xDecodeBin(uiSymbol, m_cCUShapeOfPartitionSCModel.get(0, 0, 1));
		if (uiSymbol == 1)
		{
			if (pcCU->getPartitionSizeIndex(uiAbsPartIdx) == 3)
			{
				pcCU->setPartSizeSubParts(SIZE_2NxnU, uiAbsPartIdx, uiDepth);
				pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
				return;
			}
			else if (pcCU->getPartitionSizeIndex(uiAbsPartIdx) == 4)
			{
				pcCU->setPartSizeSubParts(SIZE_nLx2N, uiAbsPartIdx, uiDepth);
				pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
				return;
			}
		}
		else
		{
			if (pcCU->getPartitionSizeIndex(uiAbsPartIdx) == 3)
			{
				pcCU->setPartSizeSubParts(SIZE_2NxnD, uiAbsPartIdx, uiDepth);
				pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
				return;
			}
			else if (pcCU->getPartitionSizeIndex(uiAbsPartIdx) == 4)
			{
				pcCU->setPartSizeSubParts(SIZE_nRx2N, uiAbsPartIdx, uiDepth);
				pcCU->setSizeSubParts(1 << (g_uiLog2MaxCUSize - uiDepth), 1 << (g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth);
				return;
			}
		}
	}
}

Void TDecSbac::parseB2Nx2NInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{

}

Void TDecSbac::parseB2NxnNInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{}

Void TDecSbac::parseF2Nx2NInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{}

Void TDecSbac::parseF2NxnNInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{}

Void TDecSbac::parseBNxNInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{}

Void TDecSbac::parseFNxNInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{}

Void TDecSbac::parseDMHMode(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
#if DMH
	if (!pcCU->getPicture()->isInterF())
	{
		return;
  }
#if FF_ENABLE
	if (pcCU->getLog2CUSize(uiAbsPartIdx) == 3 && pcCU->getPartitionSize(uiAbsPartIdx) != 0
		&& pcCU->getPicture()->isInterF())
#else
	if (pcCU->getLog2CUSize(uiAbsPartIdx) == 3 && pcCU->getPartitionSize(uiAbsPartIdx) != 0
		&& (pcCU->getPicture()->isInterF() || pcCU->getPicture()->isInterP()))
#endif
	{
		return;
	}
	UInt uiInterDir0 = pcCU->getInterDir(uiAbsPartIdx);  ///< uiIntraDir 1 to 4
	UInt uiTempPartIdx;
	if (pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N)
	{
		if (uiInterDir0 == INTER_FORWARD)
		{
			readDMHMode(pcCU, uiAbsPartIdx, uiDepth);
		}
		else
		{
			return;
		}
	}
	else if ((pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxN && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_Nx2N) ||
		(pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxnU && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N))
	{
		UInt uiInterDir1 = 0;
		UInt uiPartOffset = (pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1)) >> 2;
		switch (pcCU->getPartitionSize(uiAbsPartIdx))
		{
		case SIZE_2NxN:
		{
			uiTempPartIdx = uiAbsPartIdx + (uiPartOffset << 1);
			uiInterDir1 = pcCU->getInterDir(uiTempPartIdx);
			break;
		}
		case SIZE_Nx2N:
		{
			uiTempPartIdx = uiAbsPartIdx + uiPartOffset;
			uiInterDir1 = pcCU->getInterDir(uiTempPartIdx);
			break;
		}
		case SIZE_2NxnU:
		{
			uiTempPartIdx = uiAbsPartIdx + (uiPartOffset >> 1);
			uiInterDir1 = pcCU->getInterDir(uiTempPartIdx);
			break;
		}
		case SIZE_2NxnD:
		{
			uiTempPartIdx = uiAbsPartIdx + (uiPartOffset << 1) + (uiPartOffset >> 1);
			uiInterDir1 = pcCU->getInterDir(uiTempPartIdx);
			break;
		}
		case SIZE_nLx2N:
		{
			uiTempPartIdx = uiAbsPartIdx + (uiPartOffset >> 2);
			uiInterDir1 = pcCU->getInterDir(uiTempPartIdx);
			break;
		}
		case SIZE_nRx2N:
		{
			uiTempPartIdx = uiAbsPartIdx + uiPartOffset + (uiPartOffset >> 2);
			uiInterDir1 = pcCU->getInterDir(uiTempPartIdx);
			break;
		}
		default:
			break;
		}
		if ((uiInterDir0 == INTER_FORWARD) && (uiInterDir1 == INTER_FORWARD))
		{
			readDMHMode(pcCU, uiAbsPartIdx, uiDepth);
		}
		else
		{
			return;
		}
	}
	else if (pcCU->getPartitionSize(uiAbsPartIdx == SIZE_NxN))
	{
		if (uiInterDir0 == INTER_FORWARD)
		{
			readDMHMode(pcCU, uiAbsPartIdx, uiDepth);
		}
		else
		{
			return;
		}
	}
	return;
#endif
}
#endif

#if DMH
Void TDecSbac::readDMHMode(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	const Int iDecMapTab[9] = { 0, 3, 4, 7, 8, 1, 2, 5, 6 };
	UInt tempS1, tempS2, tempS3;
	UInt iSymbol = 0;
	UChar index = (pcCU->getLog2CUSize(uiAbsPartIdx) - 3) * 4;

	xDecodeBin(tempS1, m_cCUDMHSCModel.get(0, 0, index));
	//xDecodeBin(tempS1, m_cCUDMHSCModel.get(0, 0, 0));
	//xDecodeBinEP(tempS1);
	//assert(tempS1 == 0);
	if (tempS1 == 0)
	{
		iSymbol = 0;
	}
	else
	{
		xDecodeBin(tempS2, m_cCUDMHSCModel.get(0, 0, index + 1));
		if (tempS2 == 0)
		{
			xDecodeBinEP(tempS3);
			iSymbol = tempS3 + 1;
		}
		else
		{
			xDecodeBin(tempS2, m_cCUDMHSCModel.get(0, 0, index + 2));
			if (tempS2 == 0)
			{
				xDecodeBinEP(tempS3);
				iSymbol = tempS3 + 3;
			}
			else
			{
				xDecodeBinEP(tempS2);
				xDecodeBinEP(tempS3);
				iSymbol = 5 + (tempS2 << 1) + tempS3;
			}
		}
	}
	pcCU->setDMHModeSubPart(iDecMapTab[iSymbol], uiAbsPartIdx, uiDepth);

}
#endif

Void TDecSbac::parsePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if ( pcCU->isSkip( uiAbsPartIdx ) )
  {
    pcCU->setPartSizeSubParts( SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
    pcCU->setSizeSubParts( 1<<(g_uiLog2MaxCUSize - uiDepth), 1<<(g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth );
    return;
  }

  UInt uiSymbol, uiMode = 0;
  PartSize eMode;

  // for test
  UInt uiPocTest = pcCU->getPicture()->getPOC();

  if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
#if YQH_INTRA
	  eMode=parseTransSplitFlag(pcCU,uiAbsPartIdx,uiDepth);
	  eMode=parseIntraPuTypeIdx(pcCU,uiAbsPartIdx,uiDepth);
#else
#if ZHANGYI_INTRA
	  PictureType cuPicType = pcCU->getPicture()->getPictureType();
	  if ( (pcCU->isSkip( uiAbsPartIdx)) && ( (cuPicType == P_PICTURE) || (cuPicType == B_PICTURE) || (cuPicType == F_PICTURE))) //zhangyiCheck: influence inter or not??
	  {
		  return;
	  }
	  UInt SizeInBit = pcCU->getLog2CUSize(uiAbsPartIdx);
	  if ( ( SizeInBit == 3) || (((pcCU->getPicture()->getSPS()->getNonsquareIntraPredictionEnableFlag() == 1) && ((SizeInBit > 3) && (SizeInBit < 6)))))
	  {
#endif
    xDecodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 0) );
    eMode = uiSymbol ? SIZE_2Nx2N : SIZE_NxN;

#if INTER_GROUP
		pcCU->setPredModeSubParts(MODE_INTRA, uiAbsPartIdx, uiDepth);
#endif

    if( eMode == SIZE_NxN )
    {
      pcCU->setTrIdxSubParts( 1, uiAbsPartIdx, uiDepth );
    }
    else
    {
      pcCU->setTrIdxSubParts( 0, uiAbsPartIdx, uiDepth );
    }
#if ZHANGYI_INTRA
	  }
	  else
	  {
		  eMode = SIZE_2Nx2N; //zhangyiCheck: 
		  pcCU->setTrIdxSubParts(0, uiAbsPartIdx, uiDepth);
	  }
#endif
#endif
  }
  else
  {
    for ( UInt ui = 0; ui < 3; ui++ )
    {
      xDecodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, ui) );
      if ( uiSymbol )
      {
        break;
      }
      uiMode++;
    }
    eMode = (PartSize) uiMode;

    if (pcCU->getPicture()->isInterB() && uiMode == 3)
    {
  		xDecodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 3) );
  		if (uiSymbol == 0)
  		{
  		  pcCU->setPredModeSubParts( MODE_INTRA, uiAbsPartIdx, uiDepth );
    		xDecodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 4) );
    		if (uiSymbol == 0)
    		  eMode = SIZE_2Nx2N;
  		}
    }

    if (  pcCU->getLog2CUSize( uiAbsPartIdx ) > 3  )
    {
      if (eMode == SIZE_2NxN)
      {
        xDecodeBin(uiSymbol, m_cCUYPosiSCModel.get( 0, 0, 0 ));
        if (uiSymbol == 0)
        {
          xDecodeBin(uiSymbol, m_cCUYPosiSCModel.get( 0, 0, 1 ));
					eMode = (uiSymbol == 0? SIZE_2NxnU : SIZE_2NxnD);
        }
      }
      else if (eMode == SIZE_Nx2N)
      {
        xDecodeBin(uiSymbol, m_cCUXPosiSCModel.get( 0, 0, 0 ));
        if (uiSymbol == 0)
        {
          xDecodeBin(uiSymbol, m_cCUXPosiSCModel.get( 0, 0, 1 ));
					eMode = (uiSymbol == 0? SIZE_nLx2N : SIZE_nRx2N);
        }
      }
    } 

  }

  pcCU->setPartSizeSubParts( eMode, uiAbsPartIdx, uiDepth );
  pcCU->setSizeSubParts( 1<<(g_uiLog2MaxCUSize - uiDepth), 1<<(g_uiLog2MaxCUSize - uiDepth), uiAbsPartIdx, uiDepth );
#if ZHANGYI_INTRA_SDIP
  if (pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA)
  {
	  if (eMode == SIZE_2Nx2N || eMode == SIZE_NxN)
	  {
		  pcCU->setSDIPFlagSubParts(0, uiAbsPartIdx, uiDepth);
	  }
	  else if (eMode == SIZE_hNx2N || eMode == SIZE_2NxhN)
	  {
		  pcCU->setSDIPFlagSubParts(1, uiAbsPartIdx, uiDepth);
		  pcCU->setSDIPDirectionSubParts((eMode == SIZE_2NxhN) ? 1 : 0, uiAbsPartIdx, uiDepth);
	  }
  }
#endif
}

#if WRITE_INTERDIR
Void TDecSbac::parseInterDirRD(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	Int act_ctx = 0;
	Int act_sym = 0;

	Int pDir0, pDir1;
	Int newPdir[4] = { 3, 1, 0, 2 };

	Int dir2offset[4][4] = { { 0, 2, 4, 9 },
	{ 3, 1, 5, 10 },
	{ 6, 7, 8, 11 },
	{ 12, 13, 14, 15 }
	};

	Int index = 0;
#if rd_mvd1
	Int pdir0[16] = { INTER_FORWARD, INTER_BACKWARD, INTER_FORWARD, INTER_BACKWARD, INTER_FORWARD, INTER_BACKWARD, INTER_BID, INTER_BID, INTER_BID, INTER_FORWARD, INTER_BACKWARD, INTER_BID, INTER_SYM, INTER_SYM, INTER_SYM, INTER_SYM };
	Int pdir1[16] = { INTER_FORWARD, INTER_BACKWARD, INTER_BACKWARD, INTER_FORWARD, INTER_BID, INTER_BID, INTER_FORWARD, INTER_BACKWARD, INTER_BID, INTER_SYM, INTER_SYM, INTER_SYM, INTER_FORWARD, INTER_BACKWARD, INTER_BID, INTER_SYM };
#else
	//Int pdir0[16] = { INTER_FORWARD, INTER_BACKWARD, INTER_FORWARD, INTER_BACKWARD, INTER_FORWARD, INTER_BACKWARD, INTER_SYM, INTER_SYM, INTER_SYM, INTER_FORWARD, INTER_BACKWARD, INTER_SYM, INTER_BID, INTER_BID, INTER_BID, INTER_BID };
	//Int pdir1[16] = { INTER_FORWARD, INTER_BACKWARD, INTER_BACKWARD, INTER_FORWARD, INTER_SYM, INTER_SYM, INTER_FORWARD, INTER_BACKWARD, INTER_SYM, INTER_BID, INTER_BID, INTER_BID, INTER_FORWARD, INTER_BACKWARD, INTER_SYM, INTER_BID };
#endif

	Int pdir0[16] = { 0, 1, 0, 1, 0, 1, 2, 2, 2, 0, 1, 2, 3, 3, 3, 3 };
	Int pdir1[16] = { 0, 1, 1, 0, 2, 2, 0, 1, 2, 3, 3, 3, 0, 1, 2, 3 };
	Int counter = 0;
	Int terminateBit = 0;
	UInt uiSymbol;

	if (pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N)
	{
		xDecodeBin(uiSymbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
		while (counter < 2 && (terminateBit = uiSymbol) == 0)
		{
			act_sym++;
			act_ctx++;
			counter++;
#if WLQ_Interdir_bug
			if (counter < 2)
#endif
				xDecodeBin(uiSymbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
		}
		if (!terminateBit)
		{
			xDecodeBin(uiSymbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
			act_sym += (!uiSymbol);
		}
		pDir0 = act_sym;
		index = act_sym;
	}
	else if (pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxN && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N && pcCU->getLog2CUSize(uiAbsPartIdx) == B8X8_IN_BIT)
	{
		xDecodeBin(uiSymbol, m_cCUMinInterDirSCModel.get(0, 0, act_ctx));
		pDir0 = uiSymbol;
		xDecodeBin(uiSymbol, m_cCUMinInterDirSCModel.get(0, 0, act_ctx + 1));
		pDir1 = uiSymbol;

		if (pDir1 == 1)
		{
			pDir1 = pDir0;
		}
		else
		{
			pDir1 = 1 - pDir0;
		}

		if (pDir0 == 0)
		{ // BW
			pDir0 = 1;
		}
		else
		{
			pDir0 = 0;
		}

		if (pDir1 == 0)
		{ // BW
			pDir1 = 1;
		}
		else
		{
			pDir1 = 0;
		}


#if	YQH_B_INTER
		index = dir2offset[pDir0][pDir1];
#else
		index = dir2offset[pDir0 + 1][pDir1 + 1];
#endif
		//printf("index=%d  \n", index);
	}
	else if (pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxN || pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N)
	{
		counter = 0;
		terminateBit = 0;
		act_ctx = 0;
		act_sym = 0;

		xDecodeBin(uiSymbol, m_cCUInterDirSCModel.get(0, 0, act_ctx + 4));
		while (counter < 2 && (terminateBit = uiSymbol) == 0)
		{
			act_sym++;
			act_ctx++;
			counter++;
#if WLQ_Interdir_bug
			if (counter < 2)
#endif
				xDecodeBin(uiSymbol, m_cCUInterDirSCModel.get(0, 0, act_ctx + 4));
		}

		if (!terminateBit)
		{
			xDecodeBin(uiSymbol, m_cCUInterDirSCModel.get(0, 0, act_ctx + 4));
			act_sym += (!uiSymbol);
		}
		pDir0 = act_sym;

		act_ctx = 8;

		xDecodeBin(uiSymbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
		if (uiSymbol)
		{
			pDir1 = pDir0;
		}
		else
		{
			switch (pDir0)
			{
			case 0:
				act_ctx = 9;
				xDecodeBin(uiSymbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
				if (uiSymbol)
				{
					pDir1 = 1;
				}
				else
				{
					act_ctx = 10;
					xDecodeBin(uiSymbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
					if (uiSymbol)
					{
						pDir1 = 2;
					}
					else
					{
						pDir1 = 3;
					}
				}
				break;
			case 1:
				act_ctx = 11;
				xDecodeBin(uiSymbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
				if (uiSymbol)
				{
					pDir1 = 0;
				}
				else
				{
					act_ctx = 12;
					xDecodeBin(uiSymbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
					if (uiSymbol)
					{
						pDir1 = 2;
					}
					else
					{
						pDir1 = 3;
					}
				}
				break;

			case 2:
				act_ctx = 13;
				xDecodeBin(uiSymbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
				if (uiSymbol)
				{
					pDir1 = 0;
				}
				else
				{
					act_ctx = 14;
					xDecodeBin(uiSymbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
					if (uiSymbol)
					{
						pDir1 = 1;
					}
					else
					{
						pDir1 = 3;
					}
				}
				break;
			case 3:
				act_ctx = 15;
				xDecodeBin(uiSymbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
				if (uiSymbol)
				{
					pDir1 = 0;
				}
				else
				{
					act_ctx = 16;
					xDecodeBin(uiSymbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
					if (uiSymbol)
					{
						pDir1 = 1;
					}
					else
					{
						pDir1 = 2;
					}
				}
				break;
			}
		}
		pDir0 = newPdir[pDir0];
		pDir1 = newPdir[pDir1];
		index = dir2offset[pDir0][pDir1];
	}

	UInt uiPartOffset = (pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1)) >> 2;
#if	YQH_B_INTER
	if ((pcCU->getPartitionSize(uiAbsPartIdx)) == SIZE_2Nx2N)
	{

		if (index == 4)
			printf("error index \n");


		if (index == 0)
			index = 1;
		else if (index == 1)
			index = 2;
		else if (index == 2)
			index = 4;
		else
			index = index;



	}
	else
	{
		if (pdir0[index] == 4)
			printf("error pdir0[index] \n");


		if (pdir0[index] == 0)
			pdir0[index] = 1;
		else if (pdir0[index] == 1)
			pdir0[index] = 2;
		else if (pdir0[index] == 2)
			pdir0[index] = 4;
		else
			pdir0[index] = pdir0[index];

		if (pdir1[index] == 4)
			printf("error pdir1[index] \n");

		if (pdir1[index] == 0)
			pdir1[index] = 1;
		else if (pdir1[index] == 1)
			pdir1[index] = 2;
		else if (pdir1[index] == 2)
			pdir1[index] = 4;
		else
			pdir1[index] = pdir1[index];

	}

	//	printf("pDir0=%d \n", index);
#endif
	switch (pcCU->getPartitionSize(uiAbsPartIdx))
	{
	case SIZE_2Nx2N:
	{
		pcCU->setInterDirSubParts(index, uiAbsPartIdx, uiDepth, 0);
		break;
	}
	case SIZE_2NxN:
	{
		pcCU->setInterDirSubParts(pdir0[index], uiAbsPartIdx, uiDepth, 0);

		uiAbsPartIdx += uiPartOffset << 1;
		pcCU->setInterDirSubParts(pdir1[index], uiAbsPartIdx, uiDepth, 1);
		break;
	}
	case SIZE_Nx2N:
	{
		pcCU->setInterDirSubParts(pdir0[index], uiAbsPartIdx, uiDepth, 0);

		uiAbsPartIdx += uiPartOffset;
		pcCU->setInterDirSubParts(pdir1[index], uiAbsPartIdx, uiDepth, 1);

		break;
	}
	case SIZE_2NxnU:
	{
		pcCU->setInterDirSubParts(pdir0[index], uiAbsPartIdx, uiDepth, 0);

		uiAbsPartIdx += (uiPartOffset >> 1);
		pcCU->setInterDirSubParts(pdir1[index], uiAbsPartIdx, uiDepth, 1);

		break;
	}
	case SIZE_2NxnD:
	{
		pcCU->setInterDirSubParts(pdir0[index], uiAbsPartIdx, uiDepth, 0);

		uiAbsPartIdx += (uiPartOffset << 1) + (uiPartOffset >> 1);
		pcCU->setInterDirSubParts(pdir1[index], uiAbsPartIdx, uiDepth, 1);

		break;
	}
	case SIZE_nLx2N:
	{
		pcCU->setInterDirSubParts(pdir0[index], uiAbsPartIdx, uiDepth, 0);

		uiAbsPartIdx += (uiPartOffset >> 2);
		pcCU->setInterDirSubParts(pdir1[index], uiAbsPartIdx, uiDepth, 1);

		break;
	}
	case SIZE_nRx2N:
	{
		pcCU->setInterDirSubParts(pdir0[index], uiAbsPartIdx, uiDepth, 0);

		uiAbsPartIdx += uiPartOffset + (uiPartOffset >> 2);
		pcCU->setInterDirSubParts(pdir1[index], uiAbsPartIdx, uiDepth, 1);

		break;
	}
	default:
		break;
	}
}
#endif

Void TDecSbac::parseInterDir( TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;

  xDecodeBin( uiSymbol, m_cCUInterDirSCModel.get( 0, 0, 0 ) );

  if ( uiSymbol )
  {
    uiSymbol = 2;
  }
#if RPS
#if B_RPS_BUG_818
  else if (pcCU->getPicture()->getPictureType() == B_PICTURE && pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_0, 0) == pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_1, 0))
#else
  else if (pcCU->getPicture()->getPictureType() == B_PICTURE && pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_0, pcCU->getCUMvField(REF_PIC_0)->getRefIdx(uiAbsPartIdx)) == pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_1, pcCU->getCUMvField(REF_PIC_1)->getRefIdx(uiAbsPartIdx)))
#endif
#else
  else if (pcCU->getPicture()->getPictureType() == B_PICTURE && pcCU->getPicture()->getRefPOC(REF_PIC_0) == pcCU->getPicture()->getRefPOC(REF_PIC_1))
#endif
 
  {
    uiSymbol = 0;
  }
  else
  {
    xDecodeBin( uiSymbol, m_cCUInterDirSCModel.get( 0, 0, 1 ) );
  }
  uiSymbol++;
  ruiInterDir = uiSymbol;
  return;
}
#if ZHANGYI_INTRA
Void TDecSbac::parseIntraPuTypeIndex( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
	UInt uiSymbol;
	xDecodeBin(uiSymbol, m_cCUIntraDirSCModel.get(0,0,0)); //zhangyiCheck: In the future add the real IntraPuTypeIndex and set it into the right place.
}
#endif

#if ZHANGYI_INTRA
Void TDecSbac::parseIntraDirCb( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
	UInt uiSymbol;
	UInt uiDir = 0;
	UInt debug_x = pcCU->getCUPelX();
	UInt debug_y = pcCU->getCUPelY();
	UInt LumaMode = pcCU->getIntraDir(uiAbsPartIdx);
	int lmode = 0;
	Bool is_redundant = false;
	UInt uiSymbol1, uiSymbol2;
	UInt l;
	if (LumaMode == INTRA_VER_IDX || LumaMode == INTRA_HOR_IDX || LumaMode == INTRA_DC_IDX || LumaMode == INTRA_BI_IDX)
	{
		lmode = LumaMode == INTRA_VER_IDX ? INTRA_VERT_PRED_CHROMA : (LumaMode == INTRA_HOR_IDX ? INTRA_HOR_PRED_CHROMA : (LumaMode == INTRA_DC_IDX ? INTRA_DC_PRED_CHROMA : INTRA_BI_PRED_CHROMA));
		is_redundant = true;
	}

	#if YQH_INTRA
	//yuquanhe@hisilicon.com
#if niu_getLeftCU
#if niu_LeftPU_revise
	UInt PULeftPartIdx = MAX_UINT;
	TComDataCU* pcCUAboveLeft = pcCU->getPULeft(PULeftPartIdx, uiAbsPartIdx);
#else
	TComDataCU* pcCUAboveLeft = pcCU->getCULeft();
#endif
#else
	TComDataCU* pcCUAboveLeft=  pcCU->getCUAboveLeft();
#endif
	int act_ctx=0;
	if(pcCUAboveLeft==NULL)
		act_ctx=0;
	else
	{
#if niu_LeftPU_revise
		act_ctx = pcCUAboveLeft->getIntraDirCb(PULeftPartIdx) != 0 ? 1 : 0;
#else
		act_ctx = pcCUAboveLeft->getIntraDirCb(uiAbsPartIdx) != 0 ? 1 : 0;
#endif
	}
#endif
#if	WLQ_intra_Chroma_ctx_BUG
	act_ctx = 0;
#endif
	xDecodeBin(uiSymbol, m_cCUIntraDirSCModel.get(0,0,7+act_ctx)); //the first bin
	uiSymbol = !uiSymbol;
	if (uiSymbol != 0) //dir > 0
	{
		xDecodeBin(uiSymbol1, m_cCUIntraDirSCModel.get(0,0,7+2)); // the second bin
		if (uiSymbol1 == 1)
		{
			uiDir = 0 + 1;
		} else {
			uiSymbol2 = 0;
			do 
			{
				xDecodeBin(l, m_cCUIntraDirSCModel.get(0,0,7+2));
				uiSymbol2++;
			} while ((l != 1) && uiSymbol2 < 2);

			if ((l != 1) && (uiSymbol2 == 2))
			{
				uiSymbol2++;
			}
			uiDir = uiSymbol2 + 1;
		}
		if (is_redundant && uiDir >= lmode)
		{
			if (uiDir == 4)
			{
				printf("\n error in intra_chroma_pred_mode \n");
			}
			uiDir++;
		}
	}
	else //dir = 0
	{
		uiDir = uiSymbol;
	}
	pcCU->setIntraDirSubPartsCb(uiDir, uiAbsPartIdx, uiDepth);
#if ZHANGYI_INTRA_MODIFY
	pcCU->setIntraDirSubPartsCr(uiDir, uiAbsPartIdx, uiDepth);
#endif
	return;
}
Void TDecSbac::parseIntraDirCr( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
	UInt uiSymbol;
	UInt uiDir = 0;
	UInt debug_x = pcCU->getCUPelX();
	UInt debug_y = pcCU->getCUPelY();
	UInt LumaMode = pcCU->getIntraDir(uiAbsPartIdx);
	int lmode = 0;
	Bool is_redundant = false;
	UInt uiSymbol1, uiSymbol2;
	UInt l;
	if (LumaMode == INTRA_VER_IDX || LumaMode == INTRA_HOR_IDX || LumaMode == INTRA_DC_IDX || LumaMode == INTRA_BI_IDX)
	{
		lmode = LumaMode == INTRA_VER_IDX ? INTRA_VERT_PRED_CHROMA : (LumaMode == INTRA_HOR_IDX ? INTRA_HOR_PRED_CHROMA : (LumaMode == INTRA_DC_IDX ? INTRA_DC_PRED_CHROMA : INTRA_BI_PRED_CHROMA));
		is_redundant = true;
	}
	xDecodeBin(uiSymbol, m_cCUIntraDirSCModel.get(0,0,0)); //the first bin
	uiSymbol = !uiSymbol;
	if (uiSymbol != 0) //dir > 0
	{
		xDecodeBin(uiSymbol1, m_cCUIntraDirSCModel.get(0,0,0)); // the second bin
		if (uiSymbol1 == 1)
		{
			uiDir = 0 + 1;
		} else {
			uiSymbol2 = 0;
			do 
			{
				xDecodeBin(l, m_cCUIntraDirSCModel.get(0,0,0));
				uiSymbol2++;
			} while ((l != 1) && uiSymbol2 < 2);

			if ((l != 1) && (uiSymbol2 == 2))
			{
				uiSymbol2++;
			}
			uiDir = uiSymbol2 + 1;
		}
		if (is_redundant && uiDir >= lmode)
		{
			if (uiDir == 4)
			{
				printf("\n error in intra_chroma_pred_mode \n");
			}
			uiDir++;
		}
	}
	else //dir = 0
	{
		uiDir = uiSymbol;
	}
	pcCU->setIntraDirSubPartsCr(uiDir, uiAbsPartIdx, uiDepth);
	return;
}
#endif

  #if CODEFFCODER_LDW_WLQ_YQH

Void TDecSbac:: getCGLastAndposXAndposY(Int riWidth, Int riHeight,Int &CGLast, Int& CGX, Int& CGY  )
{
	//const int raster2ZZ_2x2[] = { 0,  1,  2,  3};
	//const int raster2ZZ_4x4[] = { 0,  1,  5,  6,
	//	2,  4,  7, 12,
	//	3,  8, 11, 13,
	//	9, 10, 14, 15
	//};
	//const int raster2ZZ_8x8[] = { 0,  1,  5,  6, 14, 15, 27, 28,
	//	2,  4,  7, 13, 16, 26, 29, 42,
	//	3,  8, 12, 17, 25, 30, 41, 43,
	//	9, 11, 18, 24, 31, 40, 44, 53,
	//	10, 19, 23, 32, 39, 45, 52, 54,
	//	20, 22, 33, 38, 46, 51, 55, 60,
	//	21, 34, 37, 47, 50, 56, 59, 61,
	//	35, 36, 48, 49, 57, 58, 62, 63
	//};
	//const int raster2ZZ_2x8[] = { 0,  1,  4,  5 , 8,  9, 12, 13,
	//	2,  3,  6,  7, 10, 11, 14, 15
	//};
	//const int raster2ZZ_8x2[] = { 0,  1,
	//	2,  4,
	//	3,  5,
	//	6,  8,
	//	7,  9,
	//	10, 12,
	//	11, 13,
	//	14, 15
	//};

	if(riWidth==4&&riHeight==16)
	{
		CGX=0;
		CGY=CGLast;
		return;
	}
	if(riWidth==16&&riHeight==4)
	{
		CGX=CGLast;
		CGY=0;
		return;
	}

	if((riWidth==8&&riHeight==8))
	{
		CGX = CGLast & 1;
		CGY = CGLast / 2;
		return;
	}
	if((riWidth==16&&riHeight==16))
	{
		const UInt* pucCGScanXY  = g_auiFrameinvScanXY[2];
		CGLast=pucCGScanXY[CGY *4 + CGX];
		//CGLast=raster2ZZ_4x4[CGY *4 + CGX];
		return;
	}
	if((riWidth==32&&riHeight==32))
	{
		const UInt* pucCGScanXY  = g_auiFrameinvScanXY[3];
		CGLast=pucCGScanXY[CGY *8 + CGX];
	//	CGLast=raster2ZZ_8x8[CGY *8 + CGX];
		return;
	}
#if	CODEFFCODER_LDW_WLQ_YQH_BUG2
	if ((riWidth == 8 && riHeight == 32))
	{
		const UInt* pucCGScanXY = g_auiFrameinvScanXY[0];
		CGLast = pucCGScanXY[CGY * 2 + CGX];
		// CGLast=raster2ZZ_8x2[CGY * 8 + CGX];
		return;
	}
	if ((riWidth == 32 && riHeight == 8))
	{
		const UInt* pucCGScanXY = g_auiFrameinvScanXY[1];
		CGLast = pucCGScanXY[CGY * 8 + CGX];
		//CGLast=raster2ZZ_2x8[CGY * 2 + CGX];
		return;
	}
#else
	if((riWidth==8&&riHeight==32))
	{
		const UInt* pucCGScanXY  = g_auiFrameinvScanXY[0];
		CGLast=pucCGScanXY[CGY * 8 + CGX];
		// CGLast=raster2ZZ_8x2[CGY * 8 + CGX];
		return;
	}
	if((riWidth==32&&riHeight==8))
	{
		const UInt* pucCGScanXY  = g_auiFrameinvScanXY[1];
		CGLast=pucCGScanXY[CGY * 2 + CGX];
		//CGLast=raster2ZZ_2x8[CGY * 2 + CGX];
		return;
	}
#endif
}

Void TDecSbac:: parseCoeffCGLastXAndCGLastY(TextType eTType,Int riWidth, Int riHeight,Int uiIntraModeIdx,Int& CGLast, Int& CGLastX, Int& CGLastY  )
{
	Int numOfCoeff=riWidth*riHeight;
	UInt count = 0;
	UInt CGLastBit=0;
	UInt numCGminus1X=(riWidth>>2)-1;
	UInt numCGminus1Y=(riHeight>>2)-1;


	if(numOfCoeff==16)
	{
		CGLast = 0;
		CGLastX = 0;
		CGLastY = 0;
		return;
	}

	if(numOfCoeff==64)
	{
		do {
			xDecodeBin( CGLastBit, m_cTULastCGSCModel.get( 0, eTType != TEXT_LUMA, count) );
			CGLastBit=!CGLastBit;
			CGLast += CGLastBit;
			count++;
		} while (CGLastBit && count < 3);
		getCGLastAndposXAndposY( riWidth, riHeight, CGLast, CGLastX, CGLastY  );
		return;

	}
	if(numOfCoeff==256||numOfCoeff==1024)
	{
		UInt numCGminus1X=(riWidth>>2)-1;
		UInt numCGminus1Y=(riHeight>>2)-1;
		UInt offset;
		if (eTType == TEXT_LUMA && uiIntraModeIdx == INTRA_PRED_DC_DIAG) {
			std:: swap(numCGminus1X,numCGminus1Y);
		}
		offset = (eTType != TEXT_LUMA) ? 3 : 9;
		xDecodeBin(CGLastBit,m_cTULastCGSCModel.get( 0, eTType != TEXT_LUMA, offset ));//
		
		if (CGLastBit == 0) {
			CGLastX = 0;
			CGLastY = 0;
			CGLast  = 0;
			return;
		}

		if(CGLastBit)
		{
			offset = (eTType != TEXT_LUMA) ? 4 : 10;
			do {
				xDecodeBin(CGLastBit,m_cTULastCGSCModel.get( 0, eTType != TEXT_LUMA, offset ));//
				CGLastBit=!CGLastBit;
				CGLastX += CGLastBit;
				count++;
			} while (CGLastBit && count < numCGminus1X);

			count = 0;
			offset =  (eTType != TEXT_LUMA) ? 5 : 11;

			if (CGLastX == 0) {
				if (numCGminus1Y != 1) {
					do {
						xDecodeBin(CGLastBit,m_cTULastCGSCModel.get( 0, eTType != TEXT_LUMA, offset ));//
						CGLastBit=!CGLastBit;
						CGLastY += CGLastBit;
						count++;
					} while (CGLastBit && count + 1 < numCGminus1Y);
				}

				CGLastY ++;
			}  else {
				do {
					xDecodeBin(CGLastBit,m_cTULastCGSCModel.get( 0, eTType != TEXT_LUMA, offset ));//
					CGLastBit=!CGLastBit;
					CGLastY += CGLastBit;
					count++;
				} while (CGLastBit && count < numCGminus1Y);
			}
		}
		if (eTType == TEXT_LUMA && uiIntraModeIdx == INTRA_PRED_DC_DIAG) {
			std:: swap(CGLastX,CGLastY);
		}
		getCGLastAndposXAndposY( riWidth, riHeight, CGLast, CGLastX, CGLastY  );
		return;
	}
		
}


Void TDecSbac:: parseCoeffinCGLastXAndCGLastY(Int& pos,TextType eTType,Int rank, Int numOfCG, Int ctxmode,Int iCG, Int firstCG, Int CGx, Int  CGy  )
{
	int xx, yy,symbol=0;
	//const UInt* pucScan  = g_auiFrameScanXY[2];
	//const UInt* pucScanX  = g_auiFrameScanX[2];
	//const UInt* pucScanY  = g_auiFrameScanY[2];
	int zigzag[4][4] = {{0, 1, 5, 6}, {2, 4, 7, 12}, {3, 8, 11, 13}, {9, 10, 14, 15}};
	symbol = 0;

	UInt offset,ctx=0;
#if	CODEFFCODER_LDW_WLQ_YQH_BUG
	if (eTType == TEXT_LUMA) {
		offset = (numOfCG == 1) ? (ctxmode / 2) * 4 : (((CGx > 0 &&
			CGy > 0) ? 0 : ((ctxmode / 2) * 4 + (firstCG ? 4 : 12))) + 8);
	}
	else {
		offset = (numOfCG == 1) ? 0 : 4;
	}
#else
	if (eTType == TEXT_LUMA) {
		offset = (numOfCG == 4) ? (ctxmode / 2) * 4 : (((CGx > 0 &&
			CGy > 0) ? 0 : ((ctxmode / 2) * 4 + (firstCG ? 4 : 12))) + 8);
	} else {
		offset = (numOfCG == 4) ? 0 : 4;
	}
#endif
	offset += (iCG == 0 ? 0 : ((eTType != TEXT_LUMA) ? NUM_LAST_POS_CTX_CHROMA / 2 : NUM_LAST_POS_CTX_LUMA / 2));
	UInt bit=0;

	xDecodeBin(bit,m_cTULastPosInCGSCModel.get( 0, (eTType != TEXT_LUMA), offset + ctx ));//

	while ( bit== 0) {
		symbol += 1;
		if (symbol == 3) {
			break;
		}

		ctx ++;
		if (ctx >= 2) {
			ctx = 2;
		}
		if (ctx >= 1) {
			ctx = 1;
		}

    xDecodeBin(bit,m_cTULastPosInCGSCModel.get( 0, (eTType != TEXT_LUMA), offset + ctx ));//
	}

	xx = symbol;
	symbol = 0;
	ctx = 0;


	xDecodeBin(bit,m_cTULastPosInCGSCModel.get( 0, eTType != TEXT_LUMA, offset + ctx + 2 ));//

	while ( bit== 0) {
		symbol += 1;
		if (symbol == 3) {
			break;
		}

		ctx ++;
		if (ctx >= 2) {
			ctx = 2;
		}
		if (ctx >= 1) {
			ctx = 1;
		}

		xDecodeBin(bit,m_cTULastPosInCGSCModel.get( 0, eTType != TEXT_LUMA, offset + ctx + 2 ));//
	}
	yy = symbol;

	if ((CGx == 0 && CGy > 0 && ctxmode == 2) /*|| (ctxmode == 1)*/) {
		yy = yy ^ xx;
		xx = yy ^ xx;
		yy = xx ^ yy;
	}
	if (rank != 0) {
		//if(ctxmode!=1)
		{
			xx = 3 - xx;
		}
		if (ctxmode != 0) {
			yy = 3 - yy;
		}
	}
	 pos += 15 - zigzag[yy][xx];

}
Void  TDecSbac:: parseCoeffAbsLevel( TextType eTType,int pairsInCG, UInt rank,  UInt firstCG, int pos ,Int& AbsLevel     )
{
	int symbol = 0;

	int indiv = Min(2, (pairsInCG + 1) / 2);
	// pCTX = Primary[ min(rank, indiv + 2) ];
	int leve_base=Min(rank, indiv + 2)*17;
	int offset = ((firstCG && pos > 12) ? 0 : 3) + indiv + 8;
	if (eTType==TEXT_LUMA) {
		offset += 3;
	}

	UInt band=0;
	UInt golomb_order,binary_symbol;
#if wlq_AEC
	xDecodeBinF(band);
#else
	xDecodeBin(band,m_cTULevelRunSCModel.get( 0, eTType!=TEXT_LUMA, 0 ));//
#endif
	UInt l,sig;
	if (band) {
		golomb_order = 0;
		binary_symbol = 0;
		do {
			 xDecodeBinEP(l);

			if (l == 0) {
				symbol += (1 << golomb_order);
				golomb_order++;
			}
		} while (l != 1);

		while (golomb_order--) {
			//next binary part
			xDecodeBinEP(sig);

			if (sig == 1) {
				binary_symbol |= (1 << golomb_order);
			}
		}

		symbol += binary_symbol;
		symbol += 32;

	}else
	{
		UInt bins = 0;
		xDecodeBin(bins,m_cTULevelRunSCModel.get( 0, eTType!=TEXT_LUMA, leve_base+ offset));//

		while (bins == 0) {
			symbol += 1;
			bins++;
#if ZY_MODIFY_COEFF_CODING
			if (symbol == 31) {
#else
			if (bins == 31) {
#endif
				break;
			}
       xDecodeBin(bins,m_cTULevelRunSCModel.get( 0, eTType!=TEXT_LUMA, leve_base+ offset));//
		}
	}
	
	AbsLevel= (symbol + 1);
	return;
	
}


Void TDecSbac::	parseCoeffRun(Int absLevel,Int& baseRun, TextType eTType, int pairs,int numOfCoeff,int pairsInCG, Int ctxmode, UInt rank, UInt firstCG, int pos ,CoeffCodingParam  m_CoeffCodingParam)
{
	int absSum5 = 0;
	int n = 0,k;

	absSum5 = 0;
	n = 0;
	for (k = pairs - 1; k >= pairs - pairsInCG; k --) {
		n += m_CoeffCodingParam.run[k];
		if (n >= 6) {
			break;
		}
		absSum5 += abs(m_CoeffCodingParam.level[k]);
		n ++;
	}

	//	pCTX = Primary[ min((absSum5 + absLevel) / 2, 2) ];
	Int leve_base=Min((absSum5 + absLevel) / 2, 2)*17;
	int ctxpos = 0,moddiv,offset,px,py;
	const UInt* pucScanX  = g_auiFrameScanX[2];
	const UInt* pucScanY  = g_auiFrameScanY[2];
	int symbol = 0;
	Int numOfCoeffInCG=16;
	if (pos < numOfCoeffInCG - 1) {
		 ctxpos = 0;
	if (15 - pos > 0) {
		px = pucScanX[15 - pos - 1 - ctxpos];
		py = pucScanY[15 - pos - 1 - ctxpos];
		//#if BBRY_CU8/////how to modify
		moddiv = (ctxmode == INTRA_PRED_VER) ? (py >> 1) : (/*(ctxmode == INTRA_PRED_HOR)?(px >> 1):*/(pos + ctxpos <= 9));
#if		CODEFFCODER_LDW_WLQ_YQH_BUG
		offset = (firstCG ? (pos + ctxpos == 14 ? 0 : (1 + moddiv)) : ((eTType == TEXT_LUMA) ? 2 : 3 + moddiv)) +
			(numOfCoeff == 16 ? 0 : 3);
		if (eTType == TEXT_LUMA) {
			moddiv = (ctxmode == INTRA_PRED_VER) ? ((py + 1) / 2) : (/*(ctxmode == INTRA_PRED_HOR)?(((px+1)/2)+3):*/((
				pos + ctxpos) > 11 ? 6 : ((pos + ctxpos) > 4 ? 7 : 8)));
			offset = (firstCG ? (pos + ctxpos == 14 ? 0 : (1 + moddiv % 3)) : (4 + moddiv % 3)) + (numOfCoeff == 16 ? 0 : 4);
		}
#else
		offset = (firstCG ? (pos + ctxpos == 14 ? 0 : (1 + moddiv)) : ((eTType== TEXT_LUMA) ? 2 : 3 + moddiv)) +
			(numOfCoeff == 64 ? 0 : 3);
		if (eTType== TEXT_LUMA) {
			moddiv = (ctxmode == INTRA_PRED_VER) ? ((py + 1) / 2) : (/*(ctxmode == INTRA_PRED_HOR)?(((px+1)/2)+3):*/((
				pos + ctxpos) > 11 ? 6 : ((pos + ctxpos) > 4 ? 7 : 8)));
			offset = (firstCG ? (pos + ctxpos == 14 ? 0 : (1 + moddiv % 3)) : (4 + moddiv % 3)) + (numOfCoeff == 64 ? 0 : 4);
		}
#endif
	}
	UInt bit;

		xDecodeBin(bit,m_cTULevelRunSCModel.get( 0, eTType!=TEXT_LUMA, leve_base+offset ));//

	while (bit == 0) {
		symbol += 1;

		if (symbol == numOfCoeffInCG - 1 - pos) {
			break;
		}

		ctxpos++;
		if ((15 - pos - 1 - ctxpos) >= 0) {

			px = pucScanX[15 - pos - 1 - ctxpos];
			py = pucScanY[15 - pos - 1 - ctxpos];
			moddiv = (ctxmode == INTRA_PRED_VER) ? (py >> 1) : (/*(ctxmode == INTRA_PRED_HOR)?(px >> 1):*/(pos + ctxpos <= 9));
#if		CODEFFCODER_LDW_WLQ_YQH_BUG
			offset = (firstCG ? (pos + ctxpos == 14 ? 0 : (1 + moddiv)) : (eTType == TEXT_LUMA ? 2 : 3 + moddiv)) +
				(numOfCoeff == 16 ? 0 : 3);
			if (eTType == TEXT_LUMA) {
				moddiv = (ctxmode == INTRA_PRED_VER) ? ((py + 1) / 2) : (/*(ctxmode == INTRA_PRED_HOR)?(((px+1)/2)+3):*/((
					pos + ctxpos) > 11 ? 6 : ((pos + ctxpos) > 4 ? 7 : 8)));
				offset = (firstCG ? (pos + ctxpos == 14 ? 0 : (1 + moddiv % 3)) : (4 + moddiv % 3)) + (numOfCoeff == 16 ? 0 : 4);
			}
#else
			offset = (firstCG ? (pos + ctxpos == 14 ? 0 : (1 + moddiv)) : (eTType== TEXT_LUMA ? 2 : 3 + moddiv)) +
				(numOfCoeff == 64 ? 0 : 3);
			if (eTType== TEXT_LUMA) {
				moddiv = (ctxmode == INTRA_PRED_VER) ? ((py + 1) / 2) : (/*(ctxmode == INTRA_PRED_HOR)?(((px+1)/2)+3):*/((
					pos + ctxpos) > 11 ? 6 : ((pos + ctxpos) > 4 ? 7 : 8)));
				offset = (firstCG ? (pos + ctxpos == 14 ? 0 : (1 + moddiv % 3)) : (4 + moddiv % 3)) + (numOfCoeff == 64 ? 0 : 4);
			}
#endif
		}

		xDecodeBin(bit,m_cTULevelRunSCModel.get( 0, eTType!=TEXT_LUMA, leve_base+offset ));//
	}

	}

	   baseRun=symbol;
	   return;

}

Void TDecSbac:: getCGposXAndposY(Int riWidth, Int riHeight,Int iCG,Int CGSacnIndex , Int& CGX, Int& CGY  )
{
	if(riWidth==4&&riHeight==16)
	{
		CGX=0;
		CGY=iCG;
		return;
	}
	if(riWidth==16&&riHeight==4)
	{
		CGX=iCG;
		CGY=0;
		return;
	}

	if((riWidth==8&&riHeight==8))
	{
		CGX = iCG & 1;
		CGY = iCG / 2;
		return;
	}

	const UInt* pucCGScanX  = g_auiFrameScanX[CGSacnIndex];
	const UInt* pucCGScanY  = g_auiFrameScanY[CGSacnIndex];
	{
		CGX=pucCGScanX[iCG];
		CGY=pucCGScanY[iCG];
		return;
	}

}


Void TDecSbac:: parseCoeff( TComDataCU* pcCU,UInt uiAbsPartIdx,Int CGSacnIndex ,Int iWidth, Int iHeight,TextType eTType, CoeffCodingParam&  m_CoeffCodingParam ,Int& DCT_CGNum ,Int& DCT_Pairs)
{
	//if(uiNumofCoeff)
	int Pair_Pos = 0;
	 DCT_Pairs = -1;
	if (DCT_Pairs < 0) {
	
		
		Int sigCGctx,prevCGFlagSum;
		int rank = 0;
		int pairs = 0;
		int Pair_Pos = 0;
		 DCT_CGNum = 1;
		 m_CoeffCodingParam.DCT_CGFlag[ 0 ] = 1;
		Int numOfCG = ((iWidth*iHeight) >> 4);
		Int CGx=0,CGy=0;
		PredMode m_pePreMode = pcCU->getPredictionMode(uiAbsPartIdx);
		Int ctxmode=INTRA_PRED_DC_DIAG;
		if(eTType==TEXT_LUMA&&(m_pePreMode == MODE_INTRA))
		{
			ctxmode = g_auiIntraModeClassified[ pcCU->getIntraDir(uiAbsPartIdx)];
			if (ctxmode == INTRA_PRED_HOR) 
				ctxmode = INTRA_PRED_VER;
		}
		Int CGLast=0;
		Int sigCGFlag = 1, firstCG = 0;
		Int pairs_prev;
		Int numOfCoeffInCG=16;
		const int T_Chr[5] = { 0, 1, 2, 4, 3000};

			    m_CoeffCodingParam.DCT_CGFlag[0]=1;

		for (Int iCG = 0; iCG < numOfCG; iCG ++) {
			m_CoeffCodingParam.DCT_PairsInCG[iCG]=0;
		
			if (rank == 0  && numOfCG>1) 
			{
				// getCGposXAndposY(iWidth, iHeight,iCG,CGSacnIndex ,  CGx, CGy  );
				parseCoeffCGLastXAndCGLastY(eTType, iWidth, iHeight, ctxmode,CGLast,CGx, CGy  );
			}
			if(rank>0)
				getCGposXAndposY(iWidth, iHeight,CGLast,CGSacnIndex , CGx, CGy  );
			firstCG = (CGLast == 0);
			if(iCG!=0)
			 m_CoeffCodingParam.DCT_CGFlag[ iCG ]=0;
			//! Sig CG Flag
			UInt bits=1;
			if (rank > 0) {
				 prevCGFlagSum = (iCG - 1 < 0 ? 0 : m_CoeffCodingParam.DCT_CGFlag[ iCG - 1 ]) * 2 + (iCG - 2 < 0 ? 0 : m_CoeffCodingParam.DCT_CGFlag[ iCG - 2 ]);
				sigCGctx = eTType != TEXT_LUMA ? 0 : ((CGLast == 0) ?  0 : 1);
				
				xDecodeBin(bits,m_cTUSigCGSCModel.get( 0, eTType!=TEXT_LUMA, sigCGctx ));//
			m_CoeffCodingParam.DCT_CGFlag[ iCG ]=bits;
			DCT_CGNum ++;
			}
			sigCGFlag=bits;
			//! (Run, Level)
			 if (sigCGFlag || rank == 0) {
				Int pos  = 0;
				Int pairsInCG = 0;
				pairs_prev = pairs;
				Int baseLevel , baseRun,absLevel;
				  for (int i = 0; i < numOfCoeffInCG; i++, pairs++, pairsInCG++) {
					  if(i==0)
					  {
						   parseCoeffinCGLastXAndCGLastY(pos, eTType, rank, numOfCG, ctxmode,iCG, firstCG, CGx,  CGy  );
					  }


					  if (pos == 16) {
						  break;
					  }

                    parseCoeffAbsLevel(eTType, pairsInCG,  rank,  firstCG,  pos,absLevel);
	
					parseCoeffRun( absLevel, baseRun, eTType, pairs,(numOfCG<<4),pairsInCG, ctxmode, rank, firstCG, pos ,m_CoeffCodingParam);
					
				    m_CoeffCodingParam.level[pairs] = absLevel;
					m_CoeffCodingParam.run[pairs] = baseRun;

					m_CoeffCodingParam.DCT_PairsInCG[ iCG ] ++;

					if (absLevel > T_Chr[rank]) {
						if (absLevel <= 2) {
							rank = absLevel;
						} else if (absLevel <= 4) {
							rank = 3;
						} else {
							rank = 4;
						}
					}
					if (baseRun == numOfCoeffInCG - 1 - pos) {
						pairs ++;
						pairsInCG ++;
						break;
					}

					pos += (baseRun + 1);
				}

				  //Sign of Level
				  pairs = pairs_prev;




				  for (Int i = m_CoeffCodingParam.DCT_PairsInCG[ iCG ]; i > 0; i--, pairs++) {
					  UInt bits=0;
					  xDecodeBinEP(bits);
					  if (bits) {
						  m_CoeffCodingParam.level[pairs] = - m_CoeffCodingParam.level[pairs];
					  } else {
						  m_CoeffCodingParam.level[pairs] =  m_CoeffCodingParam.level[pairs];
					  }
				  }
			}
			  CGLast --;
			  if (firstCG) {
				  break;
			  }

		}

		DCT_Pairs = pairs;
		Pair_Pos = DCT_Pairs;

	}
	(DCT_Pairs--);
	////--- set run and level ---
	//if (DCT_Pairs > 0) {
	///*	se->value1 = DCT_Level[Pair_Pos - 1];
	//	se->value2 = DCT_Run[Pair_Pos - 1];*/
	//	Pair_Pos --;
	//} else {
	//	//--- set run and level (EOB) ---
	//	/*se->value1 = se->value2 = 0;*/
	//}

	////--- decrement coefficient counter and re-set position ---
	//if ((DCT_Pairs--) == 0) {
	//	Pair_Pos = 0;
	//}

	return;
}

Void TDecSbac::getTUSize( UInt uiPartIdx,  Int& riWidth, Int& riHeight,Int uiLog2Size,  PartSize m_pePartSize  )
{
	switch ( m_pePartSize)
	{
	case SIZE_2Nx2N:
		riWidth = (1<<uiLog2Size);      riHeight = (1<<uiLog2Size);      
		break;
		//case SIZE_2NxN:
		//	riWidth = (1<<uiLog2Size);      riHeight = (1<<uiLog2Size);   
		//	break;
#if !niu_NSQT_dec
	case SIZE_2NxhN: //yuquanhe@hisilicon.com
#if ZY_INTRA_MODIFY_TUSIZE
		riWidth = (1 << (uiLog2Size + 1));      riHeight = (1 << (uiLog2Size - 1));
#else
		riWidth = (1 << uiLog2Size);      riHeight = (1 << uiLog2Size) >> 2;
#endif
		break;
	case SIZE_hNx2N: //yuquanhe@hisilicon.com
#if ZY_INTRA_MODIFY_TUSIZE
		riWidth = (1 << (uiLog2Size - 1));      riHeight = (1 << (uiLog2Size + 1));
#else
		riWidth = (1 << uiLog2Size) >> 2;     riHeight = (1 << uiLog2Size);
#endif
		break;
#endif
		//case SIZE_Nx2N:
		//	riWidth = (1<<uiLog2Size) >> 1; riHeight =(1<<uiLog2Size);     
		//	break;
		//case SIZE_NxN:
		//	riWidth = (1<<uiLog2Size)>> 1; riHeight = (1<<uiLog2Size) >> 1; 
		//	break;
		//case SIZE_2NxnU:
		//	riWidth     = (1<<uiLog2Size);
		//	riHeight    = ( uiPartIdx == 0 ) ?  (1<<uiLog2Size) >> 2 : ( (1<<uiLog2Size) >> 2 ) + ( (1<<uiLog2Size) >> 1 );
		//	
		//	break;
		//case SIZE_2NxnD:
		//	riWidth     = (1<<uiLog2Size);
		//	riHeight    = ( uiPartIdx == 0 ) ?  ( (1<<uiLog2Size) >> 2 ) + ( (1<<uiLog2Size)>> 1 ) : (1<<uiLog2Size) >> 2;
		//	
		//	break;
		//case SIZE_nLx2N:
		//	riWidth     = ( uiPartIdx == 0 ) ? (1<<uiLog2Size) >> 2 : ((1<<uiLog2Size) >> 2 ) + ( (1<<uiLog2Size) >> 1 );
		//	riHeight    = (1<<uiLog2Size);
		//	
		//	break;
		//case SIZE_nRx2N:
		//	riWidth     = ( uiPartIdx == 0 ) ? ((1<<uiLog2Size) >> 2 ) + ( (1<<uiLog2Size) >> 1 ) : (1<<uiLog2Size) >> 2;
		//	riHeight    = (1<<uiLog2Size);
		//	break;
#if ZY_INTRA_MODIFY_TUSIZE
	case SIZE_NxN:
		riWidth = (1 << uiLog2Size); riHeight = (1 << uiLog2Size);
		break;
#endif
	default:
		riWidth = (1<<uiLog2Size);      riHeight = (1<<uiLog2Size);    
		//	assert (0);
		break;
	}
}

Void TDecSbac:: getCoeffScanAndCGSacn(Int& CoeffScanIndex, Int& CGSacnIndex , Int riWidth, Int riHeight )
{
	if(riWidth==2&&riHeight==8)
	{
		CoeffScanIndex=8;
		CGSacnIndex=0;
		return;
	}
	if(riWidth==8&&riHeight==2)
	{
		CoeffScanIndex=9;
		CGSacnIndex=0;
		return;
	}

	if(riWidth==4&&riHeight==16)
	{
		CoeffScanIndex=10;
		CGSacnIndex=0;
		return;
	}

	if(riWidth==16&&riHeight==4)
	{
		CoeffScanIndex=11;
		CGSacnIndex=0;
		return;
	}

	if(riWidth==8&&riHeight==32)
	{
		CoeffScanIndex=12;
		CGSacnIndex=8;
		return;
	}

	if(riWidth==32&&riHeight==8)
	{
		CoeffScanIndex=13;
		CGSacnIndex=9;
		return;
	}
	if(riWidth==8&&riHeight==8)
	{
		CoeffScanIndex=14;
		CGSacnIndex=1;
		return;
	}
	if(riWidth==16&&riHeight==16)
	{
		CoeffScanIndex=15;
		CGSacnIndex=2;
		return;
	}
	if(riWidth==32&&riHeight==32)
	{
		CoeffScanIndex=16;
		CGSacnIndex=3;
		return;
	}
	if(riWidth==4&&riHeight==4)
	{
		CoeffScanIndex=2;
		CGSacnIndex=0;
		return;
	}


	//printf("error TU riWidth==%d riHeight=%d",riWidth,riHeight);
}

#endif


Void TDecSbac::parseIntraDir( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  UInt uiDir = 0;
#if ZHANGYI_INTRA
  UInt uiSymbol1, uiSymbol2, uiSymbol3, uiSymbol4, uiSymbol5, uiSymbol6;
  Int uiDirValue;
  xDecodeBin(uiSymbol, m_cCUIntraDirSCModel.get(0,0,0));
  if  ( uiSymbol == 0 ) //not in mpm
  {
	  xDecodeBin(uiSymbol2, m_cCUIntraDirSCModel.get(0,0,0+1));
	  xDecodeBin(uiSymbol3, m_cCUIntraDirSCModel.get(0,0,0+2));
	  xDecodeBin(uiSymbol4, m_cCUIntraDirSCModel.get(0,0,0+3));
	  xDecodeBin(uiSymbol5, m_cCUIntraDirSCModel.get(0,0,0+4));
	  xDecodeBin(uiSymbol6, m_cCUIntraDirSCModel.get(0,0,0+5)); //最低位
	  uiDirValue = (uiSymbol2 << 4) + (uiSymbol3 << 3) + (uiSymbol4 << 2) +  (uiSymbol5 << 1) + uiSymbol6;
  }
  else //in mpm
  {
	  xDecodeBin(uiSymbol1, m_cCUIntraDirSCModel.get(0,0,0+6));
	  uiDirValue = uiSymbol1 - 2;
  }

  //////////////////////////////////////////
  //		{
		//UInt bins = 0 ,symbol=0;
		//xDecodeBin(bins,m_cCUXPosiSCModel.get( 0, 0, 0 ));//

		//while (bins == 0) {
		//	symbol += 1;
		//	bins++;

  //     xDecodeBin(bins,m_cCUXPosiSCModel.get( 0, 0, 0 ));//
		//}
		//uiDirValue=symbol;
	 // }
		
	
  pcCU->setIntraDirSubPartsValue(uiDirValue, uiAbsPartIdx, uiDepth);

  UInt        LeftPartIdx  = MAX_UINT;
  UInt        AbovePartIdx = MAX_UINT;
  UInt        intraPUDir;
  // UInt        uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( pcCU->getDepth(uiAbsPartIdx) << 1)) >> 2;
  //MPM construction
#if ZHANGYI_INTRA_SDIP
  TComDataCU* pcCUAbove = pcCU->getPUAboveSDIP(AbovePartIdx, uiAbsPartIdx);
  TComDataCU* pcCULeft = pcCU->getPULeftSDIP(LeftPartIdx, uiAbsPartIdx);
#else
  TComDataCU* pcCUAbove = pcCU->getPUAbove(AbovePartIdx, uiAbsPartIdx);
  TComDataCU* pcCULeft = pcCU->getPULeft(LeftPartIdx, uiAbsPartIdx);
#endif
  UInt upMode = pcCUAbove ? (pcCUAbove->isIntra(AbovePartIdx) ? pcCUAbove->getIntraDir(AbovePartIdx) : INTRA_DC_IDX) : INTRA_DC_IDX;
  UInt leftMode = pcCULeft ? (pcCULeft->isIntra(LeftPartIdx) ? pcCULeft->getIntraDir(LeftPartIdx) : INTRA_DC_IDX) : INTRA_DC_IDX;
  UInt mostProbableMode[2];
  mostProbableMode[0] = min(upMode, leftMode);
  mostProbableMode[1] = max(upMode, leftMode);
  if (mostProbableMode[0] == mostProbableMode[1])
  {
	  mostProbableMode[0] = INTRA_DC_IDX;
	  mostProbableMode[1] = (mostProbableMode[1] == INTRA_DC_IDX) ? INTRA_BI_IDX : mostProbableMode[1];
  }
  intraPUDir = (uiDirValue < 0) ? mostProbableMode[uiDirValue + 2] :  (uiDirValue + (uiDirValue >= mostProbableMode[0]) + ((uiDirValue + 1) >= mostProbableMode[1]));
  pcCU->setIntraDirSubParts(intraPUDir, uiAbsPartIdx, uiDepth);
  //m_pcPrediction->predIntraAng(TEXT_LUMA, intraPUDir, piorg, uiStride, piPred, uiStride, uiSize, p_avail[NEIGHBOR_INTRA_UP], p_avail[NEIGHBOR_INTRA_LEFT]);

#else
  xDecodeBin( uiSymbol, m_cCUIntraDirSCModel.get( 0, 0, 0 ) );

  if ( uiSymbol )
  {
    uiDir++;
    xDecodeBin( uiSymbol, m_cCUIntraDirSCModel.get( 0, 0, 0 ) );
    uiDir += uiSymbol;
  }

  pcCU->setIntraDirSubParts(uiDir, uiAbsPartIdx, uiDepth);
#endif
  return;
}
#if RPS

Void TDecSbac::parseRefIdx(TComDataCU* pcCU, Int& iRefIdx, RefPic eRefList)
{
  UInt uiSymbol;
  ContextModel *pCtx = m_cCURefPicSCModel.get(0);
  iRefIdx = 0;

 Int iActualCtxIdx;
  Bool bBSlice = (pcCU->getPicture()->getPictureType() == B_PICTURE);

  iActualCtxIdx = 0;
  xDecodeBin(uiSymbol, *pCtx);
  Int poc = pcCU->getPicture()->getPOC();
  if (uiSymbol)
  {
    iRefIdx = 0;
  }
  else
  {
    iRefIdx = 1;
    if (bBSlice == false)
    {
      iActualCtxIdx = 4;

      if ((iRefIdx != pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefList) - 1))
      {
        xDecodeBin(uiSymbol, *(pCtx + iActualCtxIdx));
        if (!uiSymbol)
        {
          iRefIdx++;
          iActualCtxIdx++;
        }
        while ((iRefIdx != pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefList) - 1) && (!uiSymbol))
        {
          xDecodeBin(uiSymbol, *(pCtx + iActualCtxIdx));
          if (!uiSymbol)
          {
            iRefIdx++;
            iActualCtxIdx++;
          }
          if (iActualCtxIdx >= 5)
          {
            iActualCtxIdx = 5;
          }
        }
      }
    }
  }
  return;
}

#endif

#if MVD
Void TDecSbac::parseMvd(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth, RefPic eRefPic)
{
	
#if niu_mvd_dec
	//TComMv cMv(0, 0);
	UInt Sign[2] = {0};
	Int AbsValue[2] = {0};
#if 0//ZHANGYI_INTRA_SDIP
	cMv += pcCU->getMvPredDec(uiAbsPartIdx, eRefPic);
#endif
	for (UInt m = 0; m < 2; m++)
	{
	Int binary_symbol = 0;
	Int golomb_order = 0;
	UInt symbol;
	Int act_sym, act_ctx=0;
		xDecodeBin(symbol, m_cCUMvdSCModel.get(0,m,0));
		if (!symbol)
			act_sym = 0;
		else
		{
			xDecodeBin(symbol, m_cCUMvdSCModel.get(0, m, 3));
			if (!symbol)
				act_sym = 1;
			else
			{
				xDecodeBin(symbol, m_cCUMvdSCModel.get(0, m, 4));
				if (!symbol)
					act_sym = 2;
				else
				{
					xDecodeBinEP(symbol);
					if (!symbol)         //1110
					{
						act_sym = 0;
						do{
							xDecodeBinEP(symbol);
							if (symbol == 0)
							{
								act_sym += (1 << golomb_order);
								golomb_order++;
							}
						} while (symbol != 1);

						while (golomb_order--) {
							//next binary part
							xDecodeBinEP(symbol);

							if (symbol == 1) {
								binary_symbol |= (1 << golomb_order);
							}
						}
						act_sym += binary_symbol;
						act_sym = 3 + act_sym * 2;
					}
					else       //1111
					{
						act_sym = 0;
						do {
							xDecodeBinEP(symbol);
							if (symbol == 0) {
								act_sym += (1 << golomb_order);
								golomb_order++;
							}
						} while (symbol != 1);

						while (golomb_order--) {
							//next binary part
							xDecodeBinEP(symbol);
							if (symbol == 1) {
								binary_symbol |= (1 << golomb_order);
							}
						}
						act_sym += binary_symbol;
						act_sym = 4 + act_sym * 2;
					}
				}
			}
		}

		if (act_sym != 0) {
			xDecodeBinEP(symbol);
			Sign[m] = symbol;
			AbsValue[m] = (symbol == 0) ? act_sym : -act_sym;
		}
	}

//	TComMv cMv(Sign[0] ? -Int(AbsValue[0]) : AbsValue[0], Sign[1] ? -Int(AbsValue[1]) : AbsValue[1]);

	TComMv cMv(AbsValue[0] ,AbsValue[1]);
#else
	UInt uiSymbol;
	UInt uiHorAbs;
	UInt uiVerAbs;
	UInt uiHorSign = 0;
	UInt uiVerSign = 0;
	ContextModel *pCtx = m_cCUMvdSCModel.get(0);

	xDecodeBin(uiHorAbs, *pCtx);
	xDecodeBin(uiVerAbs, *pCtx);

	const Bool bHorAbsGr0 = uiHorAbs != 0;
	const Bool bVerAbsGr0 = uiVerAbs != 0;
	pCtx++;

	if (bHorAbsGr0)
	{
		xDecodeBin(uiSymbol, *pCtx);
		uiHorAbs += uiSymbol;
	}

	if (bVerAbsGr0)
	{
		xDecodeBin(uiSymbol, *pCtx);
		uiVerAbs += uiSymbol;
	}

	if (bHorAbsGr0)
	{
		if (2 == uiHorAbs)
		{
			xReadEpExGolomb(uiSymbol, 1);
			uiHorAbs += uiSymbol;
		}

		xDecodeBinEP(uiHorSign);
	}

	if (bVerAbsGr0)
	{
		if (2 == uiVerAbs)
		{
			xReadEpExGolomb(uiSymbol, 1);
			uiVerAbs += uiSymbol;
		}

		xDecodeBinEP(uiVerSign);
	}

	 TComMv cMv(uiHorSign ? -Int(uiHorAbs) : uiHorAbs, uiVerSign ? -Int(uiVerAbs) : uiVerAbs);
#endif

#if RD_PMVR
	Int iRefIdx = pcCU->getCUMvField(eRefPic)->getRefIdx(uiAbsPartIdx);
	TComMv mvd(0, 0);
	mvd.setHor(uiHorSign ? -Int(uiHorAbs) : uiHorAbs);
	mvd.setVer(uiVerSign ? -Int(uiVerAbs) : uiVerAbs);
	TComMv pMv = pcCU->getMvFieldPred(uiAbsPartIdx, eRefPic, iRefIdx).getMv();
	TComMv pmvrMv(0, 0);
	if (pcCU->getPicture()->getSPS()->getPmvrEnableFlag() == 1)
	{
		pcCU->pmvrMvDerivation(pmvrMv, mvd, pMv);
	}
	else
	{
		pmvrMv.set(mvd.getHor() + pMv.getHor(), mvd.getVer() + pMv.getVer());
	}

	pcCU->getCUMvField(eRefPic)->setAllMv(pmvrMv, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, uiPartIdx, uiDepth);
#else

	 //test
	 //pcCU->getCUMvField(eRefPic)->setAllMvd(cMv, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, uiPartIdx, uiDepth);

#if RD_MVP
	Int iRefIdx = pcCU->getCUMvField(eRefPic)->getRefIdx(uiAbsPartIdx);

	//test  niu 等效
	UChar Flag = pcCU->getPicture()->isIntra();
	UInt cnnt = pcCU->getAddr();
	TComMvField testpred;
	testpred = pcCU->getMvFieldPred(uiAbsPartIdx, eRefPic, iRefIdx);
	//printf("Addr=%d PredMV:%d  %d	MVD:%d  %d\n", pcCU->getAddr(), testpred.getMv().getHor(), testpred.getVer(), cMv.getHor(), cMv.getVer());

	cMv += testpred.getMv();

  //printf("\n%d,%d,%d\t", pcCU->getHeight(uiAbsPartIdx), pcCU->getWidth(uiAbsPartIdx), uiAbsPartIdx);
  //printf("%d,%d\t", cMv.getHor(), cMv.getVer());
	//cMv += pcCU->getMvFieldPred(uiAbsPartIdx, eRefPic, iRefIdx).getMv();

#else
	cMv += pcCU->getMvPred(uiAbsPartIdx, eRefPic);
#endif

	pcCU->getCUMvField(eRefPic)->setAllMv(cMv, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, uiPartIdx, uiDepth);
#endif

	return;
}
#else
#if RD_MVP
Void TDecSbac::parseMvd(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth, RefPic eRefPic)
{
	Int iHor, iVer;

	Int iRefIdx = pcCU->getCUMvField(eRefPic)->getRefIdx(uiAbsPartIdx);
	xReadMvd(iHor, 0);
	xReadMvd(iVer, 1);

	// set mvd
	TComMv cMv(iHor, iVer);

	// add predictor
#if RD_PMVR
	Int mvd[2];
	mvd[0] = iHor;
	mvd[1] = iVer;
	TComMv pmvrMvd(iHor, iVer);
	TComMv pMv = pcCU->getMvFieldPred(uiAbsPartIdx, eRefPic, iRefIdx).getMv();
	TComMv pmvrMv(0, 0);
	if (pcCU->getPicture()->getSPS()->getPmvrEnableFlag() == 1)
	{
		pcCU->pmvrMvDerivation(pmvrMv, pmvrMvd, pMv);
		pcCU->getCUMvField(eRefPic)->setAllMvd(cMv, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, uiPartIdx, uiDepth);
	}
	else
	{
		pmvrMv.set(mvd[0] + pMv.getHor(), mvd[1] + pMv.getVer());
	}

	pcCU->getCUMvField(eRefPic)->setAllMv(pmvrMv, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, uiPartIdx, uiDepth);
#else
	cMv += pcCU->getMvFieldPred(uiAbsPartIdx, eRefPic, iRefIdx).getMv();

	pcCU->getCUMvField(eRefPic)->setAllMv(cMv, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, uiPartIdx, uiDepth);
#endif

	return;
}
#else
Void TDecSbac::parseMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth, RefPic eRefPic )
{
  Int iHor, iVer;

  xReadMvd( iHor, 0 );
  xReadMvd( iVer, 1 );

  // set mvd
  TComMv cMv( iHor, iVer );

  // add predictor

#if RD_PMVR
	Int iRefIdx = pcCU->getCUMvField(eRefPic)->getRefIdx(uiAbsPartIdx);
	Int mvd[2];
	mvd[0] = iHor;
	mvd[1] = iVer;
	TComMv pmvrMvd(iHor, iVer);
	TComMv pMv = pcCU->getMvFieldPred(uiAbsPartIdx, eRefPic, iRefIdx).getMv();
	TComMv pmvrMv(0, 0);
	if (pcCU->getPicture()->getSPS()->getPmvrEnableFlag() == 1)
	{
		pcCU->pmvrMvDerivation(pmvrMv, pmvrMvd, pMv);
		pcCU->getCUMvField(eRefPic)->setAllMvd(cMv, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, uiPartIdx, uiDepth);
	}
	else
	{
		pmvrMv.set(mvd[0] + pMv.getHor(), mvd[1] + pMv.getVer());
	}

	pcCU->getCUMvField(eRefPic)->setAllMv(pmvrMv, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, uiPartIdx, uiDepth);
#else
#if RPS
	cMv += pcCU->getMvFieldPred(uiAbsPartIdx, eRefPic).getMv();
#else
	cMv += pcCU->getMvPred(uiAbsPartIdx, eRefPic);
#endif

	pcCU->getCUMvField(eRefPic)->setAllMv(cMv, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, uiPartIdx, uiDepth);
#endif

  return;
}
#endif
#endif

Void TDecSbac::parseAllCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt& uiIsCoeff )
{
  xDecodeBin( uiIsCoeff , m_cCUQtRootCbfSCModel.get( 0, 0, 0 ) );

  if( uiIsCoeff==0 )
  {
    pcCU->setCbfSubParts( 0, TEXT_LUMA,     uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx) );
    pcCU->setCbfSubParts( 0, TEXT_CHROMA_U, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx) );
    pcCU->setCbfSubParts( 0, TEXT_CHROMA_V, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx) );
  }
}
Void TDecSbac::parseCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth )
{
  UInt uiSymbol;
  const UInt uiCtx = pcCU->getCtxQtCbf( uiAbsPartIdx, eType, uiTrDepth );
  xDecodeBin( uiSymbol , m_cCUQtCbfSCModel.get( 0, eType ? eType - 1: eType, uiCtx ) );

  UInt uiCbf = pcCU->getCbf( uiAbsPartIdx, eType );
  pcCU->setCbfSubParts( uiCbf | (uiSymbol << uiTrDepth), eType, uiAbsPartIdx, uiDepth );
}

#if niu_CbpBit

//UInt nx2N_Pos[16] = { 0, 1, 4, 5, 16, 17, 20, 21, 32, 33, 36, 37, 48, 49, 52, 53 };
UInt nxN_Pos[16] = { 0, 1, 4, 5 };
UInt Nxn_Pos[16] = { 0, 2, 8, 10 };
UInt ToRealPos(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
	UInt AbsPartIdx_real;
	PartSize partsize = pcCU->getPartitionSize(uiAbsPartIdx);
	UInt Trmode = pcCU->getTransformIdx(uiAbsPartIdx);
	switch (partsize)
	{
	case SIZE_2Nx2N:
	case SIZE_NxN:
		AbsPartIdx_real = uiAbsPartIdx;
		break;
	case SIZE_Nx2N:
	case SIZE_nLx2N:
	case SIZE_nRx2N:
	case SIZE_hNx2N:
		if (Trmode)
		{
			if (partsize == SIZE_Nx2N && pcCU->getLog2CUSize(uiAbsPartIdx) == 3)
				AbsPartIdx_real = uiAbsPartIdx;
			else
			{
				//AbsPartIdx_real = nx2N_Pos[uiAbsPartIdx / 4];
				UInt uiCurrPartNumb_pcCU = pcCU->getPic()->getNumPartInCU() >> ((pcCU->getDepth(uiAbsPartIdx) + 1) << 1);
				UInt uiCurrPartNumb_all = pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1);
				UInt uiAbsPartIdx_resi = uiAbsPartIdx - uiAbsPartIdx / uiCurrPartNumb_all * uiCurrPartNumb_all;
				AbsPartIdx_real = (uiAbsPartIdx / pcCU->getPic()->getNumPartInCU()) * pcCU->getPic()->getNumPartInCU() + nxN_Pos[uiAbsPartIdx_resi / uiCurrPartNumb_pcCU] * uiCurrPartNumb_pcCU / 4;
			}

			//if (partsize == SIZE_Nx2N && pcCU->getLog2CUSize(uiAbsPartIdx) == 3)
			//	AbsPartIdx_real = uiAbsPartIdx;
			//else
			//{
			//	//AbsPartIdx_real = nx2N_Pos[uiAbsPartIdx / 4];
			//	UInt uiCurrPartNumb_pcCU = pcCU->getPic()->getNumPartInCU() >> ((pcCU->getDepth(uiAbsPartIdx) + 1) << 1);
			//	AbsPartIdx_real = nx2N_Pos[uiAbsPartIdx / uiCurrPartNumb_pcCU] * uiCurrPartNumb_pcCU / 4;
			//}
		}
		else
			AbsPartIdx_real = uiAbsPartIdx;;
		break;
	case SIZE_2NxN:
	case SIZE_2NxnU:
	case SIZE_2NxnD:
	case SIZE_2NxhN:
		if (Trmode)
		{
			if (partsize == SIZE_2NxN && pcCU->getLog2CUSize(uiAbsPartIdx) == 3)
				AbsPartIdx_real = uiAbsPartIdx;
			else
			{
				//AbsPartIdx_real = nx2N_Pos[uiAbsPartIdx / 4];
				UInt uiCurrPartNumb_pcCU = pcCU->getPic()->getNumPartInCU() >> ((pcCU->getDepth(uiAbsPartIdx) + 1) << 1);
				UInt uiCurrPartNumb_all = pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1);
				UInt uiAbsPartIdx_resi = uiAbsPartIdx - uiAbsPartIdx / uiCurrPartNumb_all * uiCurrPartNumb_all;
				AbsPartIdx_real = (uiAbsPartIdx / pcCU->getPic()->getNumPartInCU()) * pcCU->getPic()->getNumPartInCU() + Nxn_Pos[uiAbsPartIdx_resi / uiCurrPartNumb_pcCU] * uiCurrPartNumb_pcCU / 4;
			}
		}
		else
			AbsPartIdx_real = uiAbsPartIdx;
		break;
	default:
		assert(0);
	}

	return AbsPartIdx_real;
}

UInt ToVirtualPos(TComDataCU* neighborCU, UInt uiAbsPartIdx)
{
	UInt AbsPartIdx_virtual;
	PartSize partsize = neighborCU->getPartitionSize(uiAbsPartIdx);
	UInt Trmode = neighborCU->getTransformIdx(uiAbsPartIdx);
	switch (partsize)
	{
	case SIZE_2Nx2N:
	case SIZE_NxN:
		AbsPartIdx_virtual = uiAbsPartIdx;
		break;
	case SIZE_Nx2N:
	case SIZE_nLx2N:
	case SIZE_nRx2N:
	case SIZE_hNx2N:
		if (Trmode)
		{
			if (partsize == SIZE_Nx2N && neighborCU->getLog2CUSize(uiAbsPartIdx) == 3)
				AbsPartIdx_virtual = uiAbsPartIdx;
			else
			{
				UInt uiCurrPartNumb_pcCU = neighborCU->getPic()->getNumPartInCU() >> ((neighborCU->getDepth(uiAbsPartIdx) + 1) << 1);
				UInt uiCurrPartNumb_all = neighborCU->getPic()->getNumPartInCU() >> (neighborCU->getDepth(uiAbsPartIdx) << 1);
				UInt uiAbsPartIdx_resi = uiAbsPartIdx - uiAbsPartIdx / uiCurrPartNumb_all * uiCurrPartNumb_all;
				UInt Abs = g_auiZscanToRaster[uiAbsPartIdx];
				UInt Abs_int = g_auiZscanToRaster[uiAbsPartIdx / uiCurrPartNumb_all * uiCurrPartNumb_all];
				UInt factor = (uiCurrPartNumb_all == 16) ? 1 : ((uiCurrPartNumb_all == 64) ? 2 : 4);
				AbsPartIdx_virtual = uiAbsPartIdx / uiCurrPartNumb_all * uiCurrPartNumb_all + (Abs - Abs_int) / factor % 4 * uiCurrPartNumb_pcCU;
			}

			//if (partsize == SIZE_Nx2N && pcCU->getLog2CUSize(uiAbsPartIdx) == 3)
			//	AbsPartIdx_virtual = uiAbsPartIdx;
			//else
			//{
			//	UInt tt = g_auiZscanToRaster[uiAbsPartIdx];
			//	//UInt uiCurrPartNumb= neighborCU->getPic()->getNumPartInCU() >> (neighborCU->getDepth(uiAbsPartIdx) << 1);
			//	UInt uiCurrPartNumb = neighborCU->getPic()->getNumPartInWidth();

			//	if (neighborCU->getPic()->getNumPartInCU() == 16)
			//		AbsPartIdx_virtual = (tt % 4) * 4;
			//	else
			//		AbsPartIdx_virtual = (tt / 2 % 4) * 4;
			//}
		}
		else
			AbsPartIdx_virtual = uiAbsPartIdx;;
		break;
	case SIZE_2NxN:
	case SIZE_2NxnU:
	case SIZE_2NxnD:
	case SIZE_2NxhN:
		if (Trmode)
		{
			if (partsize == SIZE_2NxN && neighborCU->getLog2CUSize(uiAbsPartIdx) == 3)
				AbsPartIdx_virtual = uiAbsPartIdx;
			else
			{
				UInt uiCurrPartNumb_pcCU = neighborCU->getPic()->getNumPartInCU() >> ((neighborCU->getDepth(uiAbsPartIdx) + 1) << 1);
				UInt uiCurrPartNumb_all = neighborCU->getPic()->getNumPartInCU() >> (neighborCU->getDepth(uiAbsPartIdx) << 1);
				//UInt uiAbsPartIdx_resi = uiAbsPartIdx - uiAbsPartIdx / uiCurrPartNumb_all * uiCurrPartNumb_all;
				UInt uiZorderIdxInLCU = neighborCU->getZorderIdxInCU();
				UInt Abs = g_auiZscanToRaster[uiZorderIdxInLCU + uiAbsPartIdx];
				UInt Abs_int = g_auiZscanToRaster[(uiAbsPartIdx + uiZorderIdxInLCU) / uiCurrPartNumb_all * uiCurrPartNumb_all];
				//  ??  2NxN 条件下是否应该为 64
				UInt factor = (uiCurrPartNumb_all == 16) ? 1 : ((uiCurrPartNumb_all == 64) ? 2 : 4);
				//AbsPartIdx_virtual = uiAbsPartIdx / uiCurrPartNumb_all * uiCurrPartNumb_all + (Abs - Abs_int) / factor / 4 * uiCurrPartNumb_pcCU;
				UInt NumPartInWidth = (1 << g_uiLog2MaxCUSize) / 4;
				AbsPartIdx_virtual = (uiAbsPartIdx) / uiCurrPartNumb_all * uiCurrPartNumb_all + (Abs - Abs_int) / factor / NumPartInWidth * uiCurrPartNumb_pcCU;
			}
		}
		else
			AbsPartIdx_virtual = uiAbsPartIdx;
		break;
	default:
		assert(0);
	}
	return AbsPartIdx_virtual;
}



typedef struct pix_pos {
	int available;   //ABCD
	int mb_addr;    //MB position
	int x;
	int y;
	int pos_x;     //4x4 x-pos
	int pos_y;
} PixelPos;

UInt TDecSbac::readCbpbit(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt b8)
{
	UInt cbp_bit;
	int a, b;
	int mb_x = b8 % 2;
	int mb_y = b8 / 2;
	PixelPos block_a, block_b;
	TComDataCU *upMB, *leftMB;
	//BiContextTypePtr pCTX;
	int uiBitSize = pcCU->getLog2CUSize(uiAbsPartIdx); // ??
	int width = (1 << uiBitSize);
	int height = (1 << uiBitSize);
	int x, y, sizeOfNeighborBlock;
	TComDataCU *neighborMB;
	int block_a_nsqt_hor = 0, block_a_nsqt_ver = 0;
	int block_b_nsqt_hor = 0, block_b_nsqt_ver = 0;
	PartSize neighborPartSize;
	PartSize PartSize = pcCU->getPartitionSize(uiAbsPartIdx); //??
	UChar Trmode = pcCU->getTransformIdx(uiAbsPartIdx);
	Bool bNSQT = pcCU->getPicture()->getSPS()->getNonsquareQuadtreeTansformEnableFlag();
	//test  || PartSize == InNxNMB
	int currMB_hor = ((PartSize == SIZE_2NxN && uiBitSize > 3 || PartSize == SIZE_2NxnU ||
		PartSize == SIZE_2NxnD) && Trmode == 1 && bNSQT || PartSize == SIZE_2NxhN);
	int currMB_ver = ((PartSize == SIZE_Nx2N && uiBitSize > 3 || PartSize == SIZE_nLx2N ||
		PartSize == SIZE_nRx2N) && Trmode == 1 && bNSQT || PartSize == SIZE_hNx2N);
	if (b8 == 4) {
		mb_x = mb_y = 0;
	}

	mb_x = (mb_x == 0) ? 0 : 1;
	mb_y = (mb_y == 0) ? 0 : 1;

	UInt uiAbsPartIdx_real = ToRealPos(pcCU, uiAbsPartIdx);
	UInt PULeftPartIdx, PULeftPartIdx_real = MAX_UINT;
	UInt PUAbovePartIdx, PUAbovePartIdx_real = MAX_UINT;

	if (currMB_hor) {
		x = -1;
		y = height * (mb_x + mb_y * 2) / 4 + 0;
	}
	else if (currMB_ver) {
		x = width * (mb_x + mb_y * 2) / 4 - 1;
		y = 0;
	}
	else {
		x = width * mb_x / 2 - 1; //qyu night
		y = height * mb_y / 2 + 0;
	}

	// getNeighbour(x, y, 1,  &block_b, uiPosition, uiBitSize, currMB);

	leftMB = pcCU->getPULeft(PULeftPartIdx_real, pcCU->getZorderIdxInCU() + uiAbsPartIdx_real);
	block_a.available = (leftMB != NULL);
	if (leftMB != NULL)
	{
		//PULeftPartIdx = (PULeftPartIdx >> 2) << 2;
		PULeftPartIdx = ToVirtualPos(leftMB, PULeftPartIdx_real);
		//test
		UInt uiCurrPartNumb_left = leftMB->getPic()->getNumPartInCU() >> (leftMB->getDepth(PULeftPartIdx) << 1);
		UInt uiCurrPartNumb_pcCU = pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1);
		UInt AbsPartIdx = g_auiZscanToRaster[(PULeftPartIdx_real / uiCurrPartNumb_left) * uiCurrPartNumb_left];
		UInt AbsPartIdx_pcCU = g_auiZscanToRaster[(uiAbsPartIdx_real / uiCurrPartNumb_pcCU) * uiCurrPartNumb_pcCU];
		UInt uiNumPartInCUWidth = leftMB->getPic()->getNumPartInWidth();
		block_a.x = x + (pcCU->getCUPelX() + (AbsPartIdx_pcCU % uiNumPartInCUWidth) * MIN_BLOCK_SIZE) - (leftMB->getCUPelX() + (AbsPartIdx % uiNumPartInCUWidth)* MIN_BLOCK_SIZE);
		block_a.y = y + (pcCU->getCUPelY() + (AbsPartIdx_pcCU / uiNumPartInCUWidth) * MIN_BLOCK_SIZE) - (leftMB->getCUPelY() + (AbsPartIdx / uiNumPartInCUWidth)* MIN_BLOCK_SIZE);
	}
	else
	{
		block_a.y = 0;
		block_a.x = 0;
	}


	if (currMB_hor) {
		x = 0;
		y = height * (mb_x + mb_y * 2) / 4 - 1;
	}
	else if (currMB_ver) {
		x = width * (mb_x + mb_y * 2) / 4 + 0;
		y = -1;
	}
	else {
		x = width * mb_x / 2 + 0; //qyu night
		y = height * mb_y / 2 - 1;
	}

	//getNeighbour(x, y, 1, &block_b, uiPosition, uiBitSize, currMB);
	// getPUAbove  (Idx)

	upMB = pcCU->getPUAbove(PUAbovePartIdx_real, pcCU->getZorderIdxInCU() + uiAbsPartIdx_real);
	block_b.available = (upMB != NULL);
	if (upMB != NULL)
	{
		PUAbovePartIdx = ToVirtualPos(upMB, PUAbovePartIdx_real);
		//PUAbovePartIdx = (PUAbovePartIdx >> 2) << 2;
		//test
		UInt uiCurrPartNumb_up = upMB->getPic()->getNumPartInCU() >> (upMB->getDepth(PUAbovePartIdx) << 1);
		UInt uiCurrPartNumb_pcCU = pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1);
		UInt AbsPartIdx = g_auiZscanToRaster[(PUAbovePartIdx_real / uiCurrPartNumb_up) * uiCurrPartNumb_up];
		UInt AbsPartIdx_pcCU = g_auiZscanToRaster[(uiAbsPartIdx_real / uiCurrPartNumb_pcCU) * uiCurrPartNumb_pcCU];
		UInt uiNumPartInCUWidth = upMB->getPic()->getNumPartInWidth();
		block_b.x = x + (pcCU->getCUPelX() + (AbsPartIdx_pcCU % uiNumPartInCUWidth) * MIN_BLOCK_SIZE) - (upMB->getCUPelX() + (AbsPartIdx % uiNumPartInCUWidth)* MIN_BLOCK_SIZE);
		block_b.y = y + (pcCU->getCUPelY() + (AbsPartIdx_pcCU / uiNumPartInCUWidth) * MIN_BLOCK_SIZE) - (upMB->getCUPelY() + (AbsPartIdx / uiNumPartInCUWidth)* MIN_BLOCK_SIZE);
	}
	else
	{
		block_b.y = 0;
		block_b.x = 0;
	}

	if (block_a.available) {

		//if (block_a.mb_addr == pcCU->getAddr()) {   // ??  0 / 1
		//	neighborMB = pcCU;
		//}
		//else {
		//	//neighborMB = &img->mb_data[block_a.mb_addr];
		//	neighborMB = pcCU->getPic()->getCU(block_a.mb_addr);
		//}

		sizeOfNeighborBlock = leftMB->getLog2CUSize(PULeftPartIdx);
		neighborPartSize = leftMB->getPartitionSize(PULeftPartIdx);
		UChar neighborTrmode = leftMB->getTransformIdx(PULeftPartIdx);

		//test || neighborMB->cuType == INxnNMB
		block_a_nsqt_hor = ((neighborPartSize == SIZE_2NxN && sizeOfNeighborBlock > 3 || neighborPartSize == SIZE_2NxnU ||
			neighborPartSize == SIZE_2NxnD) && neighborTrmode && bNSQT || PartSize == SIZE_2NxhN);
		block_a_nsqt_ver = ((neighborPartSize == SIZE_Nx2N && sizeOfNeighborBlock > 3 || neighborPartSize == SIZE_nLx2N ||
			neighborPartSize == SIZE_nRx2N) && neighborTrmode && bNSQT || PartSize == SIZE_hNx2N);
		if (block_a_nsqt_hor) {
			block_a.x = 0;
			block_a.y = block_a.y / ((1 << sizeOfNeighborBlock) / 4);
		}
		else if (block_a_nsqt_ver) {
			block_a.x = block_a.x / ((1 << sizeOfNeighborBlock) / 4);
			block_a.y = 0;
		}
		else {
			block_a.x = block_a.x / ((1 << sizeOfNeighborBlock) / 2);
			block_a.y = block_a.y / ((1 << sizeOfNeighborBlock) / 2);
		}
	}

	if (block_b.available) {

		//if (block_b.mb_addr == pcCU->getAddr()) {
		//	neighborMB = pcCU;
		//}
		//else {
		//	//neighborMB = &img->mb_data[block_b.mb_addr];
		//	neighborMB = pcCU->getPic()->getCU(block_b.mb_addr);
		//}

		sizeOfNeighborBlock = upMB->getLog2CUSize(PUAbovePartIdx);
		neighborPartSize = upMB->getPartitionSize(PUAbovePartIdx);
		UChar neighborTrmode = upMB->getTransformIdx(PUAbovePartIdx);

		//test || neighborMB->cuType == INxnNMB
		block_b_nsqt_hor = ((neighborPartSize == SIZE_2NxN && sizeOfNeighborBlock > 3 || neighborPartSize == SIZE_2NxnU ||
			neighborPartSize == SIZE_2NxnD) && neighborTrmode && bNSQT || PartSize == SIZE_2NxhN);
		block_b_nsqt_ver = ((neighborPartSize == SIZE_Nx2N && sizeOfNeighborBlock > 3 || neighborPartSize == SIZE_nLx2N ||
			neighborPartSize == SIZE_nRx2N) && neighborTrmode && bNSQT || PartSize == SIZE_hNx2N);
		if (block_b_nsqt_hor) {
			block_b.x = 0;
			block_b.y = block_b.y / ((1 << sizeOfNeighborBlock) / 4);

		}
		else if (block_b_nsqt_ver) {
			block_b.x = block_b.x / ((1 << sizeOfNeighborBlock) / 4);
			block_b.y = 0;
		}
		else {
			block_b.x = block_b.x / ((1 << sizeOfNeighborBlock) / 2);
			block_b.y = block_b.y / ((1 << sizeOfNeighborBlock) / 2);
		}
	}

	//if (block_b.mb_addr == pcCU->getAddr()) {
	//	upMB = pcCU;
	//}
	//else {
	//	//upMB = &img->mb_data[block_b.mb_addr];
	//	upMB = pcCU->getPic()->getCU(block_b.mb_addr);
	//}

	//if (block_a.mb_addr == pcCU->getAddr()) {
	//	leftMB = pcCU;
	//}
	//else {
	//	//leftMB = &img->mb_data[block_a.mb_addr];
	//	leftMB = pcCU->getPic()->getCU(block_a.mb_addr);
	//}

	UChar Trmode_LU, CbfU, CbfV;
	UChar allYcbf[4] = { 0 };

	if (b8 == 4) {
		xDecodeBin(cbp_bit, m_cCUQtCbfSCModel.get(0, 2, 0));
	}
	else {
		if (block_a.available) {
			// test  ??  0
			//PULeftPartIdx = (PULeftPartIdx >> 2) << 2;
			UInt uiPartDepth = leftMB->getDepth(PULeftPartIdx) + 1;
			UInt uiCurrPartNumb = leftMB->getPic()->getNumPartInCU() >> (uiPartDepth << 1);
			UInt uiCurrPartNumb_left = leftMB->getPic()->getNumPartInCU() >> (leftMB->getDepth(PULeftPartIdx) << 1);
			//test
			CbfU = leftMB->getCbf(((PULeftPartIdx / uiCurrPartNumb_left) * uiCurrPartNumb_left), TEXT_CHROMA_U, 0);
			CbfV = leftMB->getCbf(((PULeftPartIdx / uiCurrPartNumb_left) * uiCurrPartNumb_left), TEXT_CHROMA_V, 0);
			Trmode_LU = leftMB->getTransformIdx(((PULeftPartIdx / uiCurrPartNumb_left) * uiCurrPartNumb_left));
			if (!Trmode_LU)
				allYcbf[0] = leftMB->getCbf(((PULeftPartIdx / uiCurrPartNumb_left) * uiCurrPartNumb_left), TEXT_LUMA, 0);
			else
			{
				for (Int m = 0; m <= 3; m++)
				{

					//test
					//PULeftPartIdx = (PULeftPartIdx >> 2) << 2;
					allYcbf[m] = leftMB->getCbf(((PULeftPartIdx / uiCurrPartNumb_left) * uiCurrPartNumb_left) + m*uiCurrPartNumb, TEXT_LUMA, 1);
				}
			}
			UInt Cbf_CU = 0;
			if (!Trmode_LU)
				Cbf_CU = (CbfV << 5) | (CbfU << 4) | (allYcbf[0] << 3) | (allYcbf[0] << 2) | (allYcbf[0] << 1) | (allYcbf[0]);
			else
				Cbf_CU = (CbfV << 5) | (CbfU << 4) | (allYcbf[3] << 3) | (allYcbf[2] << 2) | (allYcbf[1] << 1) | (allYcbf[0]);

			if (block_a_nsqt_hor) {
				a = (Cbf_CU & (1 << (block_a.y))) != 0;
			}
			else if (block_a_nsqt_ver) {
				a = (Cbf_CU& (1 << (block_a.x))) != 0;
			}
			else {
				a = (Cbf_CU & (1 << (block_a.x + block_a.y * 2))) != 0;
			}
		}
		else {
			a = 0;
		}
		if (block_b.available) {

			// test  ??  0
			//PUAbovePartIdx = (PUAbovePartIdx >> 2) << 2;
			UInt uiPartDepth = upMB->getDepth(PUAbovePartIdx) + 1;
			UInt uiCurrPartNumb = upMB->getPic()->getNumPartInCU() >> (uiPartDepth << 1);
			UInt uiCurrPartNumb_up = upMB->getPic()->getNumPartInCU() >> (upMB->getDepth(PUAbovePartIdx) << 1);

			CbfU = upMB->getCbf(((PUAbovePartIdx / uiCurrPartNumb_up) * uiCurrPartNumb_up), TEXT_CHROMA_U, 0);
			CbfV = upMB->getCbf(((PUAbovePartIdx / uiCurrPartNumb_up) * uiCurrPartNumb_up), TEXT_CHROMA_V, 0);
			Trmode_LU = upMB->getTransformIdx(((PUAbovePartIdx / uiCurrPartNumb_up) * uiCurrPartNumb_up));
			if (!Trmode_LU)
				allYcbf[0] = upMB->getCbf(((PUAbovePartIdx / uiCurrPartNumb_up) * uiCurrPartNumb_up), TEXT_LUMA, 0);
			else
			{
				for (Int m = 0; m <= 3; m++)
				{

					//test
					//PUAbovePartIdx = (PUAbovePartIdx >> 2) << 2;
					allYcbf[m] = upMB->getCbf(((PUAbovePartIdx / uiCurrPartNumb_up) * uiCurrPartNumb_up) + m*uiCurrPartNumb, TEXT_LUMA, 1);
				}
			}

			UInt Cbf_CU = 0;
			if (!Trmode_LU)
				Cbf_CU = (CbfV << 5) | (CbfU << 4) | (allYcbf[0] << 3) | (allYcbf[0] << 2) | (allYcbf[0] << 1) | (allYcbf[0]);
			else
				Cbf_CU = (CbfV << 5) | (CbfU << 4) | (allYcbf[3] << 3) | (allYcbf[2] << 2) | (allYcbf[1] << 1) | (allYcbf[0]);

			if (block_b_nsqt_hor) {
				b = (Cbf_CU & (1 << (block_b.y))) != 0;
			}
			else if (block_b_nsqt_ver) {
				b = (Cbf_CU & (1 << (block_b.x))) != 0;
			}
			else {
				b = (Cbf_CU & (1 << (block_b.x + block_b.y * 2))) != 0;
			}
		}
		else {
			b = 0;
		}

		xDecodeBin(cbp_bit, m_cCUQtCbfSCModel.get(0, 0, a + 2 * b));
	}
	return cbp_bit;
}
#endif

#if niu_write_cbf
Void TDecSbac::parseCbfY(TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth)
{
	UInt uiSymbol;
	//const UInt uiCtx = pcCU->getCtxQtCbf( uiAbsPartIdx, eType, uiTrDepth );
	//xDecodeBin( uiSymbol , m_cCUQtCbfSCModel.get( 0, eType ? eType - 1: eType, uiCtx ) );

	//UInt uiCbf = pcCU->getCbf( uiAbsPartIdx, eType );
	//pcCU->setCbfSubParts( uiCbf | (uiSymbol << uiTrDepth), eType, uiAbsPartIdx, uiDepth );

#if niu_cbf_dec
	//test 
#if	inter_intra_1
	if (!pcCU->isIntra(uiAbsPartIdx))
#else
	if(!pcCU->getPicture()->isIntra())
#endif
	//if (!pcCU->isIntra(0))
	{
		#if	DIRECTSKIP_BUG_YQH
		uiSymbol = 0;
#endif
		if (!((pcCU->getPredictionMode(uiAbsPartIdx) == MODE_DIRECT) || (pcCU->getPredictionMode(uiAbsPartIdx) == MODE_SKIP)))
		{
			//test
			//xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, TEXT_LUMA, 0));
#if niu_CbpBit  //niu_CbpBit_inter
			uiSymbol = readCbpbit(pcCU, uiAbsPartIdx, 4);
#else
			xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, 2, 0));
#endif
		}

		if (uiSymbol == 0)
		{
			//test 已注释掉 uisymbol = 2;
			parseTransformIdx(pcCU, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
			//UInt uiSymbol = pcCU->getTransformIdx(uiAbsPartIdx);
			xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, 1, 0));

			/*if (pcCU->getTransformIdx(uiAbsPartIdx) != 1)
				printf("error pcCU->getTransformIdx(uiAbsPartIdx) != 1 \n");*/

			if (pcCU->getTransformIdx(uiAbsPartIdx) == 0)
			{
				if (uiSymbol == 0)
				{
					/*pcCU->setCbfSubParts(1 | (1 << (uiTrDepth + 1)), TEXT_LUMA, uiAbsPartIdx + 0, pcCU->getDepth(uiAbsPartIdx) + 1);
					pcCU->setCbfSubParts(1 | (1 << (uiTrDepth + 1)), TEXT_LUMA, uiAbsPartIdx + 1, pcCU->getDepth(uiAbsPartIdx) + 1);
					pcCU->setCbfSubParts(1 | (1 << (uiTrDepth + 1)), TEXT_LUMA, uiAbsPartIdx + 2, pcCU->getDepth(uiAbsPartIdx) + 1);
					pcCU->setCbfSubParts(1 | (1 << (uiTrDepth + 1)), TEXT_LUMA, uiAbsPartIdx + 3, pcCU->getDepth(uiAbsPartIdx) + 1);*/
					for (Int m = 0; m <= 3; m++)
					{
#if ZY_MODIFY_CBF
						UInt uiCurrPartNumb = pcCU->getPic()->getNumPartInCU() >> ((uiDepth + 1) << 1);
						UInt uiCbf = pcCU->getCbf(uiAbsPartIdx + m * uiCurrPartNumb, eType);
						pcCU->setCbfSubParts(1 | (1 << (uiTrDepth + 1)), TEXT_LUMA, uiAbsPartIdx + m * uiCurrPartNumb, uiDepth + 1);
#else
						UInt uiCbf = pcCU->getCbf(uiAbsPartIdx + m, eType);
						pcCU->setCbfSubParts(1 | (uiSymbol << (uiTrDepth + 1)), TEXT_LUMA, uiAbsPartIdx + m, uiDepth + 1);
#endif
						//	pcCU->setCbfSubParts( 1 | (uiSymbol << 1), TEXT_LUMA, m, 1 );

					}
					UInt uiCbf = pcCU->getCbf(uiAbsPartIdx, eType);
					pcCU->setCbfSubParts(uiCbf | (1 << uiTrDepth), TEXT_LUMA, uiAbsPartIdx, uiDepth);


				
					uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U);
					pcCU->setCbfSubParts(uiCbf | (0 << uiTrDepth), TEXT_CHROMA_U, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
					uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V);
					pcCU->setCbfSubParts(uiCbf | (0 << uiTrDepth), TEXT_CHROMA_V, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
				}
				else
				{
					//test
					xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, !TEXT_LUMA, 0 + 2));
					//xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, !TEXT_LUMA, 0 + 1));
					if (uiSymbol)
					{
						UInt uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U);
						pcCU->setCbfSubParts(uiCbf | (1 << uiTrDepth), TEXT_CHROMA_U, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
						uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V);
						pcCU->setCbfSubParts(uiCbf | (1 << uiTrDepth), TEXT_CHROMA_V, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
					}
					else
					{
						//test
						xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, !TEXT_LUMA, 0 + 2));
						//xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, !TEXT_LUMA, 0 + 3));
						if (uiSymbol)
						{
							UInt uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U);
							pcCU->setCbfSubParts(uiCbf | (0 << uiTrDepth), TEXT_CHROMA_U, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
							uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V);
							pcCU->setCbfSubParts(uiCbf | (1 << uiTrDepth), TEXT_CHROMA_V, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
						}
						else
						{
							UInt uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U);
							pcCU->setCbfSubParts(uiCbf | (1 << uiTrDepth), TEXT_CHROMA_U, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
							uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V);
							pcCU->setCbfSubParts(uiCbf | (0 << uiTrDepth), TEXT_CHROMA_V, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
						}
					}
#if niu_CbpBit  //niu_CbpBit_inter
					uiSymbol = readCbpbit(pcCU, uiAbsPartIdx, 0);
#else
					xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, 0, 0));
#endif
					UInt uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA);
					//test  uicbf -> 1
					pcCU->setCbfSubParts(uiCbf | (uiSymbol << uiTrDepth), TEXT_LUMA, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));

				}
			}
			else
			{
				if (uiSymbol)
				{
					//test
					xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, 1, 0 + 2));
					//xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, !TEXT_LUMA, 0 + 1));
					if (uiSymbol)
					{
						UInt uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U);
						pcCU->setCbfSubParts(uiCbf | (1 << uiTrDepth), TEXT_CHROMA_U, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
						uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V);
						pcCU->setCbfSubParts(uiCbf | (1 << uiTrDepth), TEXT_CHROMA_V, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
					}
					else
					{
						//test
						xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, 1, 0 + 2));
						//xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, !TEXT_LUMA, 0 + 3));
						if (uiSymbol)
						{
							UInt uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U);
							pcCU->setCbfSubParts(uiCbf | (0 << uiTrDepth), TEXT_CHROMA_U, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
							uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V);
							pcCU->setCbfSubParts(uiCbf | (1 << uiTrDepth), TEXT_CHROMA_V, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
						}
						else
						{
							UInt uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U);
							pcCU->setCbfSubParts(uiCbf | (1 << uiTrDepth), TEXT_CHROMA_U, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
							uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V);
							pcCU->setCbfSubParts(uiCbf | (0 << uiTrDepth), TEXT_CHROMA_V, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
						}
					}
				}

				//for (Int m = 0; m <= 3; m++)
				//{
				//	//test    偏移量问题  set 时是否也需要确认？
				//	UInt uiPartDepth = pcCU->getDepth(uiAbsPartIdx) + 1;
				//	UInt uiCurrPartNumb = pcCU->getPic()->getNumPartInCU() >> (uiPartDepth << 1);
				//	xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, TEXT_LUMA, 0));
				//	UInt uiCbf = pcCU->getCbf(uiAbsPartIdx + m , eType);
				//	
				//	pcCU->setCbfSubParts(1 | (uiSymbol << (uiTrDepth + 1)), TEXT_LUMA, uiAbsPartIdx + m*uiCurrPartNumb, pcCU->getDepth(uiAbsPartIdx) + 1);
				//}


				for (Int m = 0; m <= 3; m++)
				{
#if !niu_CbpBit  //niu_CbpBit_inter
					xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, 0, 0));
#endif
#if ZY_MODIFY_CBF
					UInt uiCurrPartNumb = pcCU->getPic()->getNumPartInCU() >> ((uiDepth + 1) << 1);
#if niu_CbpBit  //niu_CbpBit_inter
					uiSymbol = readCbpbit(pcCU, uiAbsPartIdx + m * uiCurrPartNumb, m);
#endif
					UInt uiCbf = pcCU->getCbf(uiAbsPartIdx + m * uiCurrPartNumb, eType);
					pcCU->setCbfSubParts(1 | (uiSymbol << (uiTrDepth + 1)), TEXT_LUMA, uiAbsPartIdx + m * uiCurrPartNumb, uiDepth + 1);
					

#else
					UInt uiCbf = pcCU->getCbf(uiAbsPartIdx + m, eType);
					pcCU->setCbfSubParts(1 | (uiSymbol << (uiTrDepth + 1)), TEXT_LUMA, uiAbsPartIdx + m, uiDepth + 1);
#endif
					//	pcCU->setCbfSubParts( 1 | (uiSymbol << 1), TEXT_LUMA, m, 1 );

				}



			}
		}
		else
		{
			pcCU->setTrIdxSubParts(1, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
	/*		pcCU->setCbfSubParts(1 | (0 << (uiTrDepth + 1)), TEXT_LUMA, uiAbsPartIdx + 0, uiDepth + 1);
			pcCU->setCbfSubParts(1 | (0 << (uiTrDepth + 1)), TEXT_LUMA, uiAbsPartIdx + 1, uiDepth + 1);
			pcCU->setCbfSubParts(1 | (0 << (uiTrDepth + 1)), TEXT_LUMA, uiAbsPartIdx + 2, uiDepth + 1);
			pcCU->setCbfSubParts(1 | (0 << (uiTrDepth + 1)), TEXT_LUMA, uiAbsPartIdx + 3, uiDepth + 1);
*/
			
			for (Int m = 0; m <= 3; m++)
			{
#if ZY_MODIFY_CBF
				UInt uiCurrPartNumb = pcCU->getPic()->getNumPartInCU() >> ((uiDepth + 1) << 1);
				UInt uiCbf = pcCU->getCbf(uiAbsPartIdx + m * uiCurrPartNumb, eType);
				pcCU->setCbfSubParts(1 | (0 << (uiTrDepth + 1)), TEXT_LUMA, uiAbsPartIdx + m * uiCurrPartNumb, uiDepth + 1);
#else
				UInt uiCbf = pcCU->getCbf(uiAbsPartIdx + m, eType);
				pcCU->setCbfSubParts(1 | (uiSymbol << (uiTrDepth + 1)), TEXT_LUMA, uiAbsPartIdx + m, uiDepth + 1);
#endif
				//	pcCU->setCbfSubParts( 1 | (uiSymbol << 1), TEXT_LUMA, m, 1 );

			}



			pcCU->setCbfSubParts(0, TEXT_LUMA, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
			pcCU->setCbfSubParts(0, TEXT_CHROMA_U, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
			pcCU->setCbfSubParts(0, TEXT_CHROMA_V, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
		}
		return;
	}
	else
	{
#endif
		PartSize eSize = pcCU->getPartitionSize(uiAbsPartIdx);
		if (eSize == SIZE_2Nx2N)
		{
#if niu_CbpBit
			uiSymbol = readCbpbit(pcCU, uiAbsPartIdx, 0);
#else
			xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, eType ? eType - 1 : eType, 0));
#endif
			UInt uiCbf = pcCU->getCbf(uiAbsPartIdx, eType);
			pcCU->setCbfSubParts(uiCbf | (uiSymbol << uiTrDepth), TEXT_LUMA, uiAbsPartIdx, uiDepth);
		}
		else
		{
			UChar CCbf = 0;
			for (Int m = 0; m <= 3; m++)
			{
#if ZY_MODIFY_CBF
				UInt uiCurrPartNumb = pcCU->getPic()->getNumPartInCU() >> ((uiDepth + 1) << 1);
#if niu_CbpBit
				uiSymbol = readCbpbit(pcCU, uiAbsPartIdx + m * uiCurrPartNumb, m);
#else
				xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, eType ? eType - 1 : eType, 0));
#endif
				UInt uiCbf = pcCU->getCbf(uiAbsPartIdx + m * uiCurrPartNumb, eType);
				pcCU->setCbfSubParts(1 | (uiSymbol << (uiTrDepth + 1)), TEXT_LUMA, uiAbsPartIdx + m * uiCurrPartNumb, uiDepth + 1);
#else
				UInt uiCbf = pcCU->getCbf(uiAbsPartIdx + m, eType);
				pcCU->setCbfSubParts(1 | (uiSymbol << (uiTrDepth + 1)), TEXT_LUMA, uiAbsPartIdx + m, uiDepth + 1);
#endif
				//	pcCU->setCbfSubParts( 1 | (uiSymbol << 1), TEXT_LUMA, m, 1 );

			}

		}

#if niu_IntraCbp_revise
		xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, 1, 0 + 1));
#else
		xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, eType ? eType - 1 : eType, 0 + 1));
#endif
		if (uiSymbol)
		{
#if niu_IntraCbp_revise
			xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, 1, 0 + 3));
#else
			xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, eType ? eType - 1 : eType, 0 + 3));
#endif
			if (uiSymbol)
			{
				pcCU->setCbfSubParts(1, TEXT_CHROMA_U, uiAbsPartIdx, uiDepth);
				pcCU->setCbfSubParts(1, TEXT_CHROMA_V, uiAbsPartIdx, uiDepth);
			}
			else
			{
#if niu_IntraCbp_revise
				xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, 1, 0 + 3));
#else
				xDecodeBin(uiSymbol, m_cCUQtCbfSCModel.get(0, eType ? eType - 1 : eType, 0 + 3));
#endif
				if (uiSymbol)
				{
					pcCU->setCbfSubParts(0, TEXT_CHROMA_U, uiAbsPartIdx, uiDepth);
					pcCU->setCbfSubParts(1, TEXT_CHROMA_V, uiAbsPartIdx, uiDepth);
				}
				else
				{
					pcCU->setCbfSubParts(1, TEXT_CHROMA_U, uiAbsPartIdx, uiDepth);
					pcCU->setCbfSubParts(0, TEXT_CHROMA_V, uiAbsPartIdx, uiDepth);
				}
			}
		}
		else
		{
			pcCU->setCbfSubParts(0, TEXT_CHROMA_U, uiAbsPartIdx, uiDepth);
			pcCU->setCbfSubParts(0, TEXT_CHROMA_V, uiAbsPartIdx, uiDepth);
		}
#if niu_cbf_dec
	}
#endif
}
#endif
Void TDecSbac::parseCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiLog2Size, UInt uiDepth, TextType eTType )
{
#if niu_NSQT_dec
	UInt uiLog2Size_org = uiLog2Size;
#endif
  // check LOT
  if ( uiLog2Size > pcCU->getPicture()->getSPS()->getLog2MaxPhTrSize() )
  {
    uiLog2Size = pcCU->getPicture()->getSPS()->getLog2MaxPhTrSize();
  }
  
  UInt uiCTXIdx = 0;
   
  eTType = eTType == TEXT_LUMA ? TEXT_LUMA : ( eTType == TEXT_NONE ? TEXT_NONE : TEXT_CHROMA );
  const UInt uiMaxNumCoeff = 1 << ( uiLog2Size << 1 );

  // Initialization of the scanning pattern
  const UInt* pucScan  = g_auiFrameScanXY[uiLog2Size];

  UInt uiRun, uiAbs, uiSign, uiIndex;
  UInt uiLast = 0;
  UInt uiScanPosOffset = 0;
  UInt uiPrevLevel = 6;

#if CODEFFCODER_LDW_WLQ_YQH
  Int         iWidth;
  Int         iHeight;
  PartSize m_pePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
  CoeffCodingParam   m_CoeffCodingParam = pcCU->getCoeffCodingParam();
#if niu_NSQT_dec
  Bool bNSQT = pcCU->getPicture()->getSPS()->getNonsquareQuadtreeTansformEnableFlag();
  if ((uiLog2Size_org < pcCU->getLog2CUSize(uiAbsPartIdx)) && (eTType == TEXT_LUMA))
  {
	  pcCU->getTUSize(0, iWidth, iHeight, uiLog2Size, m_pePartSize, bNSQT);//这个函数编解码端都会用到，应该放在common里面，TComDataCU.cpp
  }
  else
  {
#if niu_NSQT_dec
	  iWidth = (1 << uiLog2Size);
	  iHeight = (1 << uiLog2Size);
#else
	  getTUSize(0, iWidth, iHeight, uiLog2Size, m_pePartSize);
#endif
  }
#else
  getTUSize(0, iWidth, iHeight, uiLog2Size, m_pePartSize);
#endif

  ///////////////////
#if ZHANGYI_INTRA_SDIP
#if ZHANGYI_INTRA_SDIP_BUG_YQH
 
  UInt uiSdipFlag = pcCU->getSDIPFlag(uiAbsPartIdx); //zhangyiCheckSDIP
  UInt uiSdipDir = pcCU->getSDIPDirection(uiAbsPartIdx); //zhangyiCheckSDIP

  UInt uiWidth, uiHeight;

  if (uiSdipFlag && (eTType == TEXT_LUMA))
  {
	  iWidth = uiSdipDir ? pcCU->getWidth(uiAbsPartIdx) : ((pcCU->getWidth(uiAbsPartIdx)) >> 2);
	  iHeight = uiSdipDir ? (pcCU->getHeight(uiAbsPartIdx) >> 2) : pcCU->getHeight(uiAbsPartIdx);
  }
  else
  {
#if  !niu_NSQT_dec
	  iWidth = (1 << uiLog2Size);
	  iHeight = (1 << uiLog2Size);
#endif
  }
#endif
#endif

  ////////////////////
  //if((iWidth) * (iHeight) >= 16)
	    if((iWidth>=4) && (iHeight>=4) )
  {
	  Int DCT_CGNum=0;
	  Int CoeffScanIndex=0,CGSacnIndex=0;
	  Int DCT_Pairs=0;
	 // if(iWidth==2||iHeight==2)
	//	  printf("error TU size iWidth=%d and iHeight =%d \n", iWidth,iHeight);
	 
	  getCoeffScanAndCGSacn( CoeffScanIndex,  CGSacnIndex , iWidth, iHeight );
	  parseCoeff( pcCU,uiAbsPartIdx,CGSacnIndex ,iWidth, iHeight,eTType, m_CoeffCodingParam,DCT_CGNum, DCT_Pairs);

	  const UInt* pucCGScanXY  = g_auiFrameScanXY[CoeffScanIndex];
	  const UInt* pucCGScanX  = g_auiFrameScanX[CoeffScanIndex];
	  const UInt* pucCGScanY  = g_auiFrameScanY[CoeffScanIndex];
	  Int level    = 1;
	  Int iCG = 0;
	  Int pairs = 0;
	  Int coef_ctr = - 1;
	  Int i,j;
	  Int run;

	  UInt  uiIntraModeIdx = g_auiIntraModeClassified[ static_cast<UInt>(pcCU->getIntraDir(uiAbsPartIdx))];

	  level=m_CoeffCodingParam.level[DCT_Pairs];
	  run =m_CoeffCodingParam.run[DCT_Pairs];
	  for (Int k = 0; (k < (iWidth) * (iHeight) + 1) && (level != 0); k++) {

		  level=m_CoeffCodingParam.level[DCT_Pairs];
		  //============ decode =============
		  if (level != 0) {   /* leave if len=1 */
			  while (m_CoeffCodingParam.DCT_CGFlag[ DCT_CGNum - iCG - 1 ] == 0) {
				  coef_ctr += 16;
				  iCG ++;
			  }
			  pairs ++;
			  coef_ctr += run + 1;

			  i = pucCGScanX[coef_ctr];
			  j =  pucCGScanY[coef_ctr];

			  if(iWidth==iHeight&&pcCU->getPredictionMode(uiAbsPartIdx)==MODE_INTRA&&(eTType == TEXT_LUMA)&&uiIntraModeIdx==INTRA_PRED_HOR)
			  {
				  std:: swap(i, j);
			  }

			  //  m_CoeffCodingParam.level[pairs]  = Clip(0 - (1 << 15), (1 << 15) - 1, m_CoeffCodingParam.level[pairs] );
			  pcCoef[j*iWidth+i] = m_CoeffCodingParam.level[DCT_Pairs] ;


		  }
		  if (pairs == m_CoeffCodingParam.DCT_PairsInCG[DCT_CGNum - iCG - 1]) {
			  coef_ctr |= 0xf;
			  pairs = 0;
			  iCG ++;
		  }

		  DCT_Pairs--;
		  if(DCT_Pairs<0)
		  {
			  level=0;
			  run=0;
		  }
		  else
		  {
			  level=m_CoeffCodingParam.level[DCT_Pairs];
			  run =m_CoeffCodingParam.run[DCT_Pairs];
		  }

	  }


  }
  else
#endif
  {

  while( !uiLast )
  {
    // run parsing
    xReadUnarySymbolSbac( uiRun, &m_cCURunSCModel.get( uiCTXIdx, eTType, Min(uiPrevLevel-1,5)*2 ), 2 );

    for ( UInt uiScanPos = uiScanPosOffset; uiScanPos < uiScanPosOffset + uiRun; uiScanPos++ )
    {
      uiIndex = pucScan[ uiScanPos ];
      pcCoef[ uiIndex ] = 0;
    }
    uiScanPosOffset += uiRun;

    // level parsing 
    xReadUnarySymbolSbac( uiAbs, &m_cCUAbsSCModel.get( uiCTXIdx, eTType, Min(uiPrevLevel-1,5)*2 ), 2 );

    uiAbs++;
    uiPrevLevel = uiAbs;

    xDecodeBinEP( uiSign );
    uiIndex = pucScan[ uiScanPosOffset ];
    pcCoef[ uiIndex ] = ( uiSign ? -(Int)uiAbs : (Int)uiAbs );

    // last_flag does not need to be parsed.
    if( uiScanPosOffset >= uiMaxNumCoeff-1 )
    {
      break;
    }
    uiScanPosOffset++;
    xDecodeBin( uiLast, m_cCULastSCModel.get( uiCTXIdx, eTType, 0 ) );

    if( uiLast )
    {
      break;
    }
  }
    }

  return;


}

Void TDecSbac::parseDBKIdx( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiDBKIdxV = 0;
  UInt uiDBKIdxH = 0;
  UInt uiSymbol;

  if( (g_auiRasterToPelX[g_auiZscanToRaster[uiAbsPartIdx]] + pcCU->getCUPelX()) )
  {
    xDecodeBin(uiSymbol, m_cDBKIdxSCModel.get(0,0,0));
    uiDBKIdxV = uiSymbol;
  }
  if( (g_auiRasterToPelY[g_auiZscanToRaster[uiAbsPartIdx]] + pcCU->getCUPelY()) )
  {
    xDecodeBin(uiSymbol, m_cDBKIdxSCModel.get(0,0,1));
    uiDBKIdxH = uiSymbol;
  }

  pcCU->setDBKIdx(0, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx), uiDBKIdxV);
  pcCU->setDBKIdx(1, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx), uiDBKIdxH);
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TDecSbac::xReadUnaryMaxSymbol( UInt& ruiSymbol, ContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol )
{
  xDecodeBin( ruiSymbol, pcSCModel[0] );
  
  if( ruiSymbol == 0 || uiMaxSymbol == 1 )
  {
    return;
  }
  
  UInt uiSymbol = 0;
  UInt uiCont;
  
  do
  {
    xDecodeBin( uiCont, pcSCModel[ iOffset ] );
    uiSymbol++;
  }
  while( uiCont && ( uiSymbol < uiMaxSymbol - 1 ) );
  
  if( uiCont && ( uiSymbol == uiMaxSymbol - 1 ) )
  {
    uiSymbol++;
  }
  
  ruiSymbol = uiSymbol;
}

Void TDecSbac::xReadMvd( Int& riMvdComp, UInt uiCtx )
{ 
  UInt uiSymbol;
  xReadUnarySymbolSbac( uiSymbol, &m_cCUMvdSCModel.get( 0, uiCtx, 0 ), 5);

  if ( !uiSymbol )
  {
    riMvdComp = 0;
    return;
  }
  
  UInt uiSign;
  xDecodeBinEP( uiSign );
  
  riMvdComp = ( 0 != uiSign ) ? -(Int)uiSymbol : (Int)uiSymbol;
  
  return;
}

Void TDecSbac::xReadEpExGolomb( UInt& ruiSymbol, UInt uiCount )
{
  UInt uiSymbol = 0;
  UInt uiBit = 1;
  
  while( uiBit )
  {
    xDecodeBinEP( uiBit );
    uiSymbol += uiBit << uiCount++;
  }
  
  uiCount--;
  while( uiCount-- )
  {
    xDecodeBinEP( uiBit );
    uiSymbol += uiBit << uiCount;
  }
  
  ruiSymbol = uiSymbol;
  
  return;
}

Void TDecSbac::xReadUnarySymbolSbac( UInt& ruiSymbol, ContextModel* pcSCModel, UInt uiNumCtx )
{
  UInt uiCtxIdx = 0;

  xDecodeBin( ruiSymbol, pcSCModel[uiCtxIdx] );
  
  if( !ruiSymbol )
  {
    return;
  }
  
  UInt uiSymbol = 0;
  UInt uiCont;
  
  do
  {
    if ( uiCtxIdx < uiNumCtx - 1 )
    {
      uiCtxIdx++;
    }
    xDecodeBin( uiCont, pcSCModel[ uiCtxIdx ] );
    uiSymbol++;
  }
  while( uiCont );
  
  ruiSymbol = uiSymbol;
}
 
Void TDecSbac::xReadUnarySymbol( UInt& ruiSymbol, ContextModel* pcSCModel, Int iOffset )
{
  xDecodeBin( ruiSymbol, pcSCModel[0] );
  
  if( !ruiSymbol )
  {
    return;
  }
  
  UInt uiSymbol = 0;
  UInt uiCont;
  
  do
  {
    xDecodeBin( uiCont, pcSCModel[ iOffset ] );
    uiSymbol++;
  }
  while( uiCont );
  
  ruiSymbol = uiSymbol;
}

Void TDecSbac::xReadExGolombLevel( UInt& ruiSymbol, ContextModel& rcSCModel  )
{
  UInt uiSymbol;
  UInt uiCount = 0;
  do
  {
    xDecodeBin( uiSymbol, rcSCModel );
    uiCount++;
  }
  while( uiSymbol && ( uiCount != 13 ) );
  
  ruiSymbol = uiCount - 1;
  
  if( uiSymbol )
  {
    xReadEpExGolomb( uiSymbol, 0 );
    ruiSymbol += uiSymbol + 1;
  }
  
  return;
}

#if wlq_AEC
Void TDecSbac::xReadBit()
{
	UInt i,j;
	value_s_bound = NUN_VALUE_BOUND_AVS2;
	s1 = 0;
	t1 = QUARTER_AVS2 - 1; //0xff
	value_s = 0;
	value_t = 0;
#if wlq_AEC_q
	m_pcBitstream->read(1,j);
#endif
	for (i = 0; i < B_BITS_AVS2 - 1 ; i++) {
		m_pcBitstream->read(1,j);
		value_t = (value_t << 1)  | j;
	}
	is_value_domain = 1;
	cFlag = 1;
}
#endif

Void TDecSbac::xReadBit( UInt& ruiVal )
{
  UInt uiBit = 0;
  m_pcBitstream->read( 1, uiBit );
  ruiVal  = ( ruiVal << 1 ) | uiBit;
  ruiVal &= 0xFFFF;
}

Void TDecSbac::xDecodeBin( UInt& ruiSymbol, ContextModel &rcCtxModel )
{
#if wlq_AEC
	register unsigned char s_flag, is_LPS = 0;
	register unsigned int lg_pmps = rcCtxModel.getLG_PMPS() >> LG_PMPS_SHIFTNO_AVS2;
	register unsigned int t_rlps;
	register unsigned int t2;
	register unsigned char s2;


	ruiSymbol = rcCtxModel.getMPS();

	if (is_value_domain == 1 || (s1 == value_s_bound &&
		is_value_bound == 1)) {  //value_t is in R domain  s1=0 or s1 == value_s_bound
			s1 = 0;
			value_s = 0;

			while (value_t < QUARTER_AVS2 && value_s < value_s_bound) {
#if wlq_AEC_r
				UInt j;
				m_pcBitstream->read( 1, j );
#else
				int j;
				if (--Dbits_to_go < 0) {
					get_byte();
				}
				j = (Dbuffer >> Dbits_to_go) & 0x01;
				// Shift in next ruiSymbol and add to value
#endif
				value_t = (value_t << 1) | j;
				value_s++;
			}
			if (value_t < QUARTER_AVS2) {
				is_value_bound = 1;
			} else {
				is_value_bound = 0;
			}

			value_t = value_t & 0xff;
	}

	if (t1 >=  lg_pmps) {
		s2 = s1;
		t2 = t1 -  lg_pmps ; //8bits
		s_flag = 0;
	} else {
		s2 = s1 + 1;
		t2 = 256 + t1 - lg_pmps ; //8bits
		s_flag = 1;
	}

	if (value_s > value_s_bound) {
		printf("value_s:%d\n", value_s);
		exit(1);
	}

	if ((s2 > value_s || (s2 == value_s && value_t >= t2)) && is_value_bound == 0) {     //LPS
		is_LPS = 1;
		ruiSymbol = !ruiSymbol; //LPS
		is_value_domain = 1;
		t_rlps = (s_flag == 0) ? (lg_pmps)
			: (t1 +  lg_pmps);

		if (s2 == value_s) {
			value_t = (value_t - t2);
		} else {
#if wlq_AEC_r
			UInt j;
			m_pcBitstream->read( 1, j );
			value_t = (value_t << 1) | j;
#else
			if (--Dbits_to_go < 0) {
				get_byte();
			}

			// Shift in next ruiSymbol and add to value
			value_t = (value_t << 1) | ((Dbuffer >> Dbits_to_go) & 0x01);
#endif
			value_t = 256 + value_t - t2;
		}

		//restore range
		while (t_rlps < QUARTER_AVS2) {
			t_rlps = t_rlps << 1;
#if wlq_AEC_r
			UInt j;
			m_pcBitstream->read( 1, j );
			value_t = (value_t << 1) | j;
#else
			if (--Dbits_to_go < 0) {
				get_byte();
			}

			// Shift in next ruiSymbol and add to value
			value_t = (value_t << 1) | ((Dbuffer >> Dbits_to_go) & 0x01);
#endif
		}

		s1 = 0;
		t1 = t_rlps & 0xff;

	} else { //MPS
		s1 = s2;
		t1 = t2;
		is_value_domain = 0;
	}
	if (cFlag) {
		rcCtxModel.update_ctx(is_LPS);
	}
#else
  UInt uiLPS;
  UChar ucNextStateLPS, ucNextStateMPS;

  uiLPS = TComCABACTables::sm_auiLPSTable[rcCtxModel.getState()];
  ucNextStateLPS = uiLPS & 0xFF;
  uiLPS >>= 8;
  ucNextStateMPS = uiLPS & 0xFF;
  uiLPS >>= 8;

  ruiSymbol = rcCtxModel.getMps();
  m_uiRange -= uiLPS;

  if ( m_uiValue < m_uiRange ) 
  {
    if ( m_uiRange >= 0x8000 ) 
    {
      return;
    }
    else 
    {
      if ( m_uiRange < uiLPS ) 
      {
        ruiSymbol = 1 - ruiSymbol;
        rcCtxModel.setStateMps(ucNextStateLPS);
      }
      else 
      {
        rcCtxModel.setStateMps(ucNextStateMPS);
      }
    }
  }
  else 
  {
    m_uiValue -= m_uiRange;
    if ( m_uiRange < uiLPS ) 
    {
      rcCtxModel.setStateMps(ucNextStateMPS);
    }
    else 
    {
      ruiSymbol = 1 - ruiSymbol;
      rcCtxModel.setStateMps(ucNextStateLPS);
    }
    m_uiRange = uiLPS;
  }

  do 
  {
    m_uiRange <<= 1;
    xReadBit(m_uiValue);
  } while ( m_uiRange < 0x8000 );
  #endif
}

Void TDecSbac::xDecodeBinEP( UInt& ruiSymbol )
{
#if wlq_AEC
	ContextModel octx;
	ContextModel* ctx = &octx;
	ctx->setLG_PMPS(QUARTER_AVS2 << LG_PMPS_SHIFTNO_AVS2);
	ctx->setMPS(0);
	cFlag = 0;
	xDecodeBin(ruiSymbol, octx);
	cFlag = 1;
#else
  m_uiRange >>= 1;

  if ( m_uiValue < m_uiRange ) 
  {
    ruiSymbol = 0;
  }
  else 
  {
    m_uiValue -= m_uiRange;
    ruiSymbol = 1;
  }

  m_uiRange <<= 1;
  xReadBit(m_uiValue);
  #endif
}
#if wlq_AEC
Void TDecSbac::xDecodeBinF         ( UInt& ruiSymbol )
{
	ContextModel octx;
	ContextModel* ctx = &octx;
	ctx->setLG_PMPS(1 << LG_PMPS_SHIFTNO_AVS2);
	ctx->setMPS(0);
	cFlag = 0;
	xDecodeBin(ruiSymbol, octx);
	cFlag = 1;
}
#endif
Void TDecSbac::xReadFlagVlc (UInt& ruiCode)
{
  m_pcBitstream->read( 1, ruiCode );
}

Void TDecSbac::xReadUvlc( UInt& ruiVal)
{
#if wlq_vlc
	Int len,info;
	xGetVLCSymbol(&len,&info);
	xLinfoUe(len, info, &ruiVal);
#else
  UInt uiVal = 0;
  UInt uiCode = 0;
  UInt uiLength;
  m_pcBitstream->read( 1, uiCode );
  
  if( 0 == uiCode )
  {
    uiLength = 0;
    
    while( ! ( uiCode & 1 ))
    {
      m_pcBitstream->read( 1, uiCode );
      uiLength++;
    }
    
    m_pcBitstream->read( uiLength, uiVal );
    
    uiVal += (1 << uiLength)-1;
  }
  
  ruiVal = uiVal;
 #endif
}

Void TDecSbac::xReadSvlc( Int& riVal)
{
#if wlq_vlc
	Int len,info;
	xGetVLCSymbol(&len,&info);
	xLinfoSe(len, info, &riVal);
#else
  UInt uiBits = 0;
  m_pcBitstream->read( 1, uiBits );
  if( 0 == uiBits )
  {
    UInt uiLength = 0;
    
    while( ! ( uiBits & 1 ))
    {
      m_pcBitstream->read( 1, uiBits );
      uiLength++;
    }
    
    m_pcBitstream->read( uiLength, uiBits );
    
    uiBits += (1 << uiLength);
    riVal = ( uiBits & 1) ? -(Int)(uiBits>>1) : (Int)(uiBits>>1);
  }
  else
  {
    riVal = 0;
  }
#endif
}
#if wlq_vlc
Void TDecSbac::xGetVLCSymbol (Int* len,Int* info)
{
	UInt ctr_bit = 0;
	Int info_bit;
	Int len_bit = 1;
	*len = 1;
	m_pcBitstream->read (1, ctr_bit);
	while (ctr_bit == 0) {
		len_bit++;
		(*len)++;
		m_pcBitstream->read (1, ctr_bit);
	}
	*info = 0;
	for(info_bit = 0; (info_bit < len_bit-1); info_bit++){
		(*len)++;
		*info = (*info) << 1;
		m_pcBitstream->read (1, ctr_bit);
		if (ctr_bit) {
			(*info) |= 1;
		}
	}
}

Void TDecSbac::xLinfoUe (Int len, Int info, UInt* ruiVal)
{
	*ruiVal = (Int) pow(2.0, (len / 2.0)) + info - 1;        // *value1 = (int)(2<<(len>>1))+info-1;
}

Void TDecSbac::xLinfoSe (Int len, Int info, Int* riVal)
{
	Int n;
	n = (Int) pow(2.0, (len / 2.0)) + info - 1;
	*riVal = (n + 1) / 2;

	if ((n & 0x01) == 0) {                      // lsb is signed bit
		*riVal = -(*riVal);
	}
}
#endif
Void TDecSbac::xReadCodeVlc (UInt uiLength, UInt& ruiCode)
{
  assert ( uiLength > 0 );
  m_pcBitstream->read (uiLength, ruiCode);
}

#if AVS3_EXTENSION_DEBUG_SYC
Void TDecSbac::xNextNBit (UInt uiLength, UInt& ruiCode)
{
  m_pcBitstream->nextnbit (uiLength, ruiCode);
}
#endif