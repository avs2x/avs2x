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

/** \file     TEncSbac.cpp
    \brief    SBAC encoder class
*/

#include "TEncTop.h"
#include "TEncSbac.h"

#include <map>
#if wlq_AEC
UInt LPSbits[256] = {
	2184, 2184, 1928, 1779, 1673, 1591, 1525, 1468, 1419, 1376, 1338,
	1303, 1272, 1243, 1216, 1191, 1167, 1145, 1125, 1105, 1087, 1069,
	1053, 1037, 1022, 1007, 993, 980, 967, 954, 942, 930, 919, 908,
	898, 888, 878, 868, 859, 850, 841, 832, 824, 816, 808, 800, 792, 785,
	777, 770, 763, 756, 750, 743, 737, 730, 724, 718, 712, 707, 701, 695,
	690, 684, 679, 674, 669, 663, 658, 654, 649, 644, 639, 635, 630, 626,
	621, 617, 613, 608, 604, 600, 596, 592, 588, 584, 580, 577, 573, 569,
	566, 562, 558, 555, 551, 548, 545, 541, 538, 535, 531, 528, 525, 522,
	519, 516, 513, 510, 507, 504, 501, 498, 495, 492, 490, 487, 484, 482,
	479, 476, 474, 471, 468, 466, 463, 461, 458, 456, 454, 451, 449, 446,
	444, 442, 439, 437, 435, 433, 430, 428, 426, 424, 422, 420, 418, 415,
	413, 411, 409, 407, 405, 403, 401, 399, 397, 395, 394, 392, 390, 388,
	386, 384, 382, 381, 379, 377, 375, 373, 372, 370, 368, 367, 365, 363,
	362, 360, 358, 357, 355, 353, 352, 350, 349, 347, 346, 344, 342, 341,
	339, 338, 336, 335, 333, 332, 331, 329, 328, 326, 325, 323, 322, 321, 319,
	318, 317, 315, 314, 313, 311, 310, 309, 307, 306, 305, 303, 302, 301,
	300, 298, 297, 296, 295, 293, 292, 291, 290, 289, 287, 286, 285, 284,
	283, 282, 281, 279, 278, 277, 276, 275, 274, 273, 272, 271, 269, 268,
	267, 266, 265, 264, 263, 262, 261, 260, 259, 258, 257
};
#endif
extern UChar  stateMappingTable[113];
extern Int    entropyBits[128];

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncSbac::TEncSbac()
: m_pcBitIf                   ( NULL )
, m_pcPicture                   ( NULL )
, m_uiCoeffCost               ( 0 )
, m_cCUSplitFlagSCModel       ( 1,             1,               NUM_SPLIT_FLAG_CTX            )
, m_cCUPredModeSCModel        ( 1,             1,               NUM_PRED_MODE_CTX             )
, m_cCUPartSizeSCModel        ( 1,             1,               NUM_PART_SIZE_CTX             )
#if INTER_GROUP
, m_cCUShapeOfPartitionSCModel(1, 1, NUM_SHAPE_OF_PARTITION_CTX)
#endif
, m_cCUInterDirSCModel        ( 1,             1,               NUM_INTER_DIR_CTX             )
#if WRITE_INTERDIR
, m_cCUMinInterDirSCModel     (1,              1,                NUM_MIN_INTER_DIR_CTX       )
#endif
#if F_DHP_SYC
, m_cCUInterDHPSCModel        (1,              1,               NUM_INTER_DHP_CTX             )
, m_cCUInterDHPNXNSCModel     (1,              1,               NUM_INTER_DHP_NXN_CTX         )
#endif
#if inter_direct_skip_bug1
, m_cCUInterWSMSCModel        (1,              1,               NUM_INTER_WSM_CTX             )
#endif
#if F_MHPSKIP_SYC
, m_cCUInterMHPSKIPSCModel    (1,              1,               NUM_INTER_MHPSKIP_CTX          )
#endif
#if YQH_INTRA
, m_cCUIntraDirSCModel        ( 1,             1,               NUM_INTRA_DIR_CTX               )//yuquanhe@hisilicon.com
#else
, m_cCUIntraDirSCModel        ( 1,             1,               1                             )
#endif
#if RPS
, m_cCURefPicSCModel(1, 1, NUM_REF_NO_CTX)
#endif
#if DMH
, m_cCUDmhSCModel(1, 1, NUM_DMH_MODE_CTX)
#endif
, m_cCUMvdSCModel             ( 1,             2,               NUM_MV_RES_CTX                )
, m_cCUQtCbfSCModel           ( 1,             3,               NUM_QT_CBF_CTX                )
, m_cCUQtRootCbfSCModel       ( 1,             1,               NUM_QT_ROOT_CBF_CTX           )
, m_cCUTransIdxSCModel        ( 1,             1,               1                             )
, m_cCURunSCModel             ( 1,             2,               NUM_RUN_CTX                   )
, m_cCULastSCModel            ( 1,             2,               NUM_LAST_FLAG_CTX             )
, m_cCUAbsSCModel             ( 1,             2,               NUM_ABS_CTX                   )
, m_cDBKIdxSCModel            ( 1,             1,               NUM_DBK_IDX_CTX               )
, m_cCUXPosiSCModel           ( 1,						 1,								NUM_CU_X_POS_CTX      )
, m_cCUYPosiSCModel           ( 1,						 1,								NUM_CU_Y_POS_CTX      )
#if YQH_INTRA
, m_cTUSplitFlagSCModel       ( 1,             1,               NUM_TRANSSPLIT_FLAG_CTX            )//yuquanhe@hisilicon.com
, m_cPUIntraTypeSCModel       ( 1,             1,               NUM_TNTRA_PU_TYPE_CTX            )//yuquanhe@hisilicon.com
#endif


#if CODEFFCODER_LDW_WLQ_YQH
, m_cTULastCGSCModel            ( 1,             2,               NUM_LAST_CG_CTX            ) //yuquanhe@hisilicon.com
, m_cTUSigCGSCModel             ( 1,             2,                NUM_SIGCG_CTX              ) //yuquanhe@hisilicon.com
, m_cTULevelRunSCModel          ( 1,             2,               NUM_LEVEL_RUN_INCG_CTX      ) //yuquanhe@hisilicon.com
, m_cTULastPosInCGSCModel        ( 1,             2,              NUM_LAST_POS_INCG_CTX        ) //yuquanhe@hisilicon.com
#endif
{
}

TEncSbac::~TEncSbac()
{
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncSbac::resetEntropy           ()
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
	m_cCUInterDirSCModel.initBuffer        ();
#if F_DHP_SYC
	m_cCUInterDHPSCModel.initBuffer();
	m_cCUInterDHPNXNSCModel.initBuffer();
#endif
#if inter_direct_skip_bug1
	m_cCUInterWSMSCModel.initBuffer();
#endif
#if F_MHPSKIP_SYC
	m_cCUInterMHPSKIPSCModel.initBuffer();
#endif
  m_cCUIntraDirSCModel.initBuffer        ();
#if RPS
  m_cCURefPicSCModel.initBuffer();
#endif
#if DMH
	m_cCUDmhSCModel.initBuffer();
#endif
  m_cCUMvdSCModel.initBuffer             ();
  m_cCUQtCbfSCModel.initBuffer           ();
  m_cCUQtRootCbfSCModel.initBuffer       ();
  m_cCUTransIdxSCModel.initBuffer        ();
  m_cCURunSCModel.initBuffer             ();
  m_cCULastSCModel.initBuffer            ();
  m_cCUAbsSCModel.initBuffer             ();
  m_cDBKIdxSCModel.initBuffer            ();
  
#if YQH_INTRA
  m_cTUSplitFlagSCModel.initBuffer       ();//yuquanhe@hisilicon.com
  m_cPUIntraTypeSCModel.initBuffer       ();//yuquanhe@hisilicon.com
#endif

  m_cCUXPosiSCModel.initBuffer           ();
  m_cCUYPosiSCModel.initBuffer           ();
#if CODEFFCODER_LDW_WLQ_YQH
  m_cTULastCGSCModel.initBuffer           ();//yuquanhe@hisilicon.com
  m_cTULevelRunSCModel.initBuffer           ();//yuquanhe@hisilicon.com
  m_cTUSigCGSCModel.initBuffer           ();//yuquanhe@hisilicon.com
  m_cTULastPosInCGSCModel.initBuffer           ();//yuquanhe@hisilicon.com
#endif
  // Initialization of the SBAC variables
#if !wlq_AEC
  m_uiRange = 0x10000;
  m_uiCode = 0;
#endif
  //m_uiRange = 0x10000;
  //m_uiCode = 0;
  m_uiCodeBits = 11;
  m_ucPendingByte = 0;
  m_bIsPendingByte = false;
  m_uiStackedFFs = 0;
  m_uiStackedZeros = 0;
#if wlq_AEC
  m_Elow = 0;
  m_E_s1 = 0;
  m_E_t1 = 0xFF;
  m_s2 = 0;
  m_t2 = 0xff;
  m_Ebits_to_follow = 0;
  m_s1 = 0;
  m_s2 = 0;
#endif

#if 0//YQH_AEC_RDO_BUG
  m_uiByteCounter = 9;
  m_uiCodeBitsbiggo = 9;
#endif

  return;
}

#if RPS
void TEncSbac::codeRefPicSetList(const TComReferencePictureSet* rps, Int idx)
{
#if YQH_AVS2_BITSTREAM
	xWriteCodeVlc(0, 1);//refered  //yuquanhe@hisilicon.com INTRA for 0
#else
	xWriteCodeVlc(rps->getRefered(), 1);//refered//refered//这个在全I帧的情况下需要调一下。
#endif

	xWriteCodeVlc(rps->getNumberOfRefPictures(), 3);
	for (Int i = 0; i<rps->getNumberOfRefPictures(); i++)
	{
		xWriteCodeVlc(abs(rps->getRefDeltaPOC(i)), 6);
	}
#if YQH_AVS2_BITSTREAM
	xWriteCodeVlc(1, 3);//yuquanhe@hisilicon.com
#else
	xWriteCodeVlc(rps->getNumberOfRemPictures(), 3);
#endif
#if YQH_AVS2_BITSTREAM
	for (Int j = 0; j < 1; j++)
	{
		xWriteCodeVlc(1, 6);
	}
#else
	for (Int j = 0; j < rps->getNumberOfRemPictures(); j++)
	{
		xWriteCodeVlc(abs(rps->getRemDeltaPOC(j)), 6);
	}
#endif
	xWriteFlagVlc(1); //marker
}
#endif


Void TEncSbac::codeSPS( TComSPS* pcSPS )
{
#if !YQH_AVS2_BITSTREAM
  // uiFirstByte
  xWriteCodeVlc( NAL_REF_IDC_PRIORITY_HIGHEST, 2);
  xWriteCodeVlc( 0, 1);
  xWriteCodeVlc( NAL_UNIT_SPS, 5);

  // Structure
  xWriteUvlc  ( pcSPS->getWidth () );
  xWriteUvlc  ( pcSPS->getHeight() );
  
  xWriteUvlc  ( pcSPS->getPad (0) );
  xWriteUvlc  ( pcSPS->getPad (1) );
  
  xWriteUvlc  ( pcSPS->getLog2MaxCUSize() - 2 );
  xWriteUvlc  ( pcSPS->getMaxCUDepth() - 1      );
  xWriteUvlc  ( pcSPS->getLog2MaxPhTrSize() - 2 );
  
  // Tools
  xWriteFlagVlc  ( (pcSPS->getUseDQP ()) ? 1 : 0 );
  xWriteFlagVlc  ( (pcSPS->getUseLDC ()) ? 1 : 0 );
  
  // Bit-depth information
  xWriteUvlc( pcSPS->getBitDepth() - 8 );
#endif
#if AVS3_SEQ_HEADER_SYC
  xWriteCodeVlc( pcSPS->getVideoSequenceStartCode(),32);//32,"video_sequence_start_code",
  xWriteCodeVlc((UInt)pcSPS->getProfileId(),8);//8, "profile_id", 
  xWriteCodeVlc((UInt)pcSPS->getLevelId(),8);//8, "level_id", 
  xWriteFlagVlc((pcSPS->getProgressiveSequence()? 1:0));//1, "progressive_sequence", 
  xWriteFlagVlc((UInt)pcSPS->getFieldCodedSequence());//1, "field_coded_sequence",
  xWriteCodeVlc(pcSPS->getHorizontalSize(),14);//14,"horizontal_size",
  xWriteCodeVlc(pcSPS->getVerticalSize(),14);//14,"vertical_size",
  xWriteCodeVlc((UInt)pcSPS->getChromaFormat(),2);//2, "chroma_format",
  xWriteCodeVlc(pcSPS->getSamplePrecision(),3);//3, "sample_precision", 
  if (pcSPS->getProfileId() == Profile::BASELINE10_PROFILE) 
  {
	  xWriteCodeVlc( pcSPS->getEncodingPrecision(),3);//3, "encoding_precision",
  }
  xWriteCodeVlc(pcSPS->getAspectRatio(),4);//4, "aspect_ratio", 
#if  YQH_SYS
  if (pcSPS->getFrameRateCode()>10)
	  xWriteCodeVlc(pcSPS->getFrameRateCode() == 24 ? 2 : pcSPS->getFrameRateCode() == 25 ? 3 : pcSPS->getFrameRateCode() == 30 ? 5 : pcSPS->getFrameRateCode() == 50 ? 6 : pcSPS->getFrameRateCode() == 60 ? 8 : 1, 4);//4, "frame_rate_code",
  else
	  xWriteCodeVlc(pcSPS->getFrameRateCode(), 4);//4, "frame_rate_code",
#else
  xWriteCodeVlc(pcSPS->getFrameRateCode(),4);//4, "frame_rate_code",
#endif

  xWriteCodeVlc(pcSPS->getBitRateLower(),18);//18,"bit_rate_lower",
  xWriteFlagVlc(1);//, "marker_bit"
  xWriteCodeVlc(pcSPS->getBitRateUpper(),12);//12,"bit_rate_upper", 
  xWriteFlagVlc((pcSPS->getLowDelay()? 1:0));//1, "low_delay", 
  xWriteFlagVlc(1);//, "marker_bit"
  xWriteFlagVlc((pcSPS->getTemporalIdEnableFlag()? 1:0));//1, "temporal_id_enable_flag",
#if YQH_AVS2_BITSTREAM
  xWriteCodeVlc(0 / (16 * 1024), 18);//18,"bbv_buffer_size", //yuquanhe@hisilicon.com
  xWriteCodeVlc(pcSPS->getLCUSize() == 64 ? 6 : pcSPS->getLCUSize() == 32 ? 5 : pcSPS->getLCUSize() == 16 ? 4 : pcSPS->getLCUSize() == 8 ? 3 : 3, 3);//3, "lcu_size",  //yuquanhe@hisilicon.com
#else
  xWriteCodeVlc(pcSPS->getBbvBufferSize()/(16*1024),18);//18,"bbv_buffer_size", 
  xWriteCodeVlc(pcSPS->getLCUSize()==64? 6:4,3);//3, "lcu_size", 
#endif
  xWriteFlagVlc((pcSPS->getWeightQuantEnableFlag()? 1:0));//1, "weight_quant_enable_flag",
  if (pcSPS->getWeightQuantEnableFlag()) 
  {
	  xWriteFlagVlc( (pcSPS->getbLoadSeqWeightQuantDataFlag()? 1:0));//1,"load_seq_weight_quant_data_flag",
	  if (pcSPS->getbLoadSeqWeightQuantDataFlag()) 
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
						  xWriteUvlc( pcSPS->getWeightQuantCoeff4x4(y,x));///<20170107 Cfg需要返工,4x4数据 m_uiWeightQuantCoeff"weight_quant_coeff",
					  }
					  else
					  {
						  xWriteUvlc(pcSPS->getWeightQuantCoeff8x8(y,x));///<20170107 Cfg需要返工,8x8数据 m_uiWeightQuantCoeff"weight_quant_coeff", 
					  }
				  }
			  }
		  }
	  }
  }
  xWriteFlagVlc((pcSPS->getScenePictureDisableFlag()? 1:0));//1, "scene_picture_disable_flag", 
  xWriteFlagVlc((pcSPS->getMultiHypothesisSkipEnableFlag()? 1:0));//1, "multi_hypothesis_skip_enable_flag", 
  xWriteFlagVlc((pcSPS->getDualHypothesisPredictionEnableFlag()? 1:0));//1, "dual_hypothesis_prediction_enable_flag",
  xWriteFlagVlc((pcSPS->getWeightedSkipEnableFlag()? 1:0));//1, "weighted_skip_enable_flag", 
  xWriteFlagVlc((pcSPS->getAsymmetricMotionPartitionsEnableFlag()? 1:0));//1, "asymmetric_motion_partitions_enable_flag",
  xWriteFlagVlc((pcSPS->getNonsquareQuadtreeTansformEnableFlag()? 1:0));//1, "nonsquare_quadtree_transform_enable_flag", 
  xWriteFlagVlc((pcSPS->getNonsquareIntraPredictionEnableFlag()? 1:0));//1, "nonsquare_intra_prediction_enable_flag",
  xWriteFlagVlc((pcSPS->getSecondaryTransformEnableFlag()? 1:0));//1, "secondary_transform_enable_flag",
  xWriteFlagVlc((pcSPS->getSampleAdaptiveOffsetEnableFlag()? 1:0));//1, "sample_adaptive_offset_enable_flag",
  xWriteFlagVlc((pcSPS->getAdaptiveLoopFilterEnableFlag()? 1:0));//1, "adaptive_loop_filter_enable_flag",
  xWriteFlagVlc((pcSPS->getPmvrEnableFlag()? 1:0));//1, "pmvr_enable_flag", 
  xWriteFlagVlc(1);//, "marker_bit"
  //   xWriteCodeVlc(pcSPS->getNumOfRcs(),6); //         6, "num_of_rcs",           ///<参考帧管理
  //   xWriteFlagVlc((pcSPS->getReferedByOtherFlag(0)? 1:0));//1, "refered_by_others_flag",
  //   xWriteCodeVlc(pcSPS->getNumOfReferencePicture(0),3);//3, "num_of_reference_picture",
  //   xWriteCodeVlc(pcSPS->getDeltaDoiOfReferencePicture(0,0),6);//6, "delta_doi_of_reference_picture",
  //   xWriteCodeVlc(pcSPS->getNumOfRemovedPicture(0),3);//3, "num_of_removed_picture", 
  //   xWriteCodeVlc(pcSPS->getDeltaDoiOfRemovedPicture(0,0),6);//6, "delta_doi_of_removed_picture", 
#if !YQH_AVS2_BITSTREAM
  xWriteFlagVlc(1);// "marker_bit"
#endif
#if RPS 
  const TComRPSList* rpsList = pcSPS->getRPSList();
  //xWriteCodeVlc(rpsList->getNumberOfReferencePictureSets(), 6);    //已经扩展ldp=14
#if RPS_BUG
  xWriteCodeVlc(pcSPS->getGOPSize(), 6);    //RD rpsList->getNumberOfReferencePictureSets()=4
#else
  xWriteCodeVlc(4, 6);    //RD rpsList->getNumberOfReferencePictureSets()=4
#endif
#if RPS_BUG
  for (Int i = 0; i < pcSPS->getGOPSize(); i++)
#else
  for (Int i = 0; i < 4; i++)
#endif
  {
	  const TComReferencePictureSet* rps = rpsList->getReferencePictureSet(i);
	  codeRefPicSetList(rps, i);
  }
#endif
#if YQH_AVS2_BITSTREAM
  if (pcSPS->getLowDelay() == 0) //yuquanhe@hisilicon.com
#else
  if (pcSPS->getLowDelay()) 
#endif
  {
	  xWriteCodeVlc(pcSPS->getOutputReorderDelay(),5 );//, "output_reorder_delay",
  }
  xWriteFlagVlc((pcSPS->getCrossSliceLoopfilterEnableFlag()? 1:0));// "cross_slice_loopfilter_enable_flag",
#if YQH_AVS2_BITSTREAM
  xWriteCodeVlc(0, 2);//, "reserved bits"
#else
  xWriteFlagVlc(1);
  xWriteFlagVlc(1);//, "reserved bits"
#endif
#endif
  return;
}
#if AVS3_EXTENSION_LWJ
Void  TEncSbac::codeUserData(TComUserData* pcUserData)
{
  xWriteCodeVlc(0x1B2, 32);	//32,"user_data_start_code",
  xWriteCodeVlc(pcUserData->getUserData(), 8);				//8,"user_data",

#if AVS3_EXTENSION_DEBUG_SYC
  xWriteCodeVlc(pcUserData->getUserData(), 8);				//8,"user_data",
  xWriteCodeVlc(pcUserData->getUserData(), 8);				//8,"user_data",
  xWriteCodeVlc(pcUserData->getUserData(), 8);				//8,"user_data",
  xWriteCodeVlc(pcUserData->getUserData(), 8);				//8,"user_data",
  xWriteCodeVlc(pcUserData->getUserData(), 8);				//8,"user_data",
#else
  xWriteCodeVlc(1, 24);				//24,暂时以此作为user_data的结束标志
#endif
#if AVS3_EXTENSION_DEBUG_SYC
  return;
#endif
}
Void  TEncSbac::codeExtSeqDisplay(TComSPS* pcSPS)
{
  xWriteCodeVlc(0x1B5, 32);//extension_start_code
  xWriteCodeVlc(pcSPS->getExtSeqDisplay()->getExtId(), 4);					//4, "extension_id", 
  xWriteCodeVlc(pcSPS->getExtSeqDisplay()->getVideoFormat(), 3);				//3, "video_format",
  xWriteFlagVlc((pcSPS->getExtSeqDisplay()->getSampleRange() ? 1:0));			//1, "sample_range",
  xWriteFlagVlc((pcSPS->getExtSeqDisplay()->getColourDescription() ?1:0));	//1, "colour_description",
  if (pcSPS->getExtSeqDisplay()->getColourDescription())
  {
    xWriteCodeVlc(pcSPS->getExtSeqDisplay()->getColourPrimaries(), 8);		//8, "colour_primaries",
    xWriteCodeVlc(pcSPS->getExtSeqDisplay()->getTransferCharacteristics(), 8);			//8, "transfer_characteristics",
    xWriteCodeVlc(pcSPS->getExtSeqDisplay()->getMatrixCoeff(), 8);			//8, "matrix_coeff",
  }
  xWriteCodeVlc(pcSPS->getExtSeqDisplay()->getDisplayHorSize(), 14);			//14, "display_horizontal_size",
  xWriteFlagVlc(1);															//1, "marker_bit", 
  xWriteCodeVlc(pcSPS->getExtSeqDisplay()->getDisplayVerSize(), 14);			//14, "display_vertical_size",
  xWriteFlagVlc((pcSPS->getExtSeqDisplay()->getTdModeFlag() ? 1 : 0));		//1, "td_mode_flag",
  if (pcSPS->getExtSeqDisplay()->getTdModeFlag())
  {
    xWriteCodeVlc(pcSPS->getExtSeqDisplay()->getTdPackingMode(), 8);		//8, "td_packing_mode",
    xWriteFlagVlc((pcSPS->getExtSeqDisplay()->getViewReverseFlag() ? 1 : 0));		//1, "view_reverse_flag",
  }
}
Void  TEncSbac::codeExtScalability(TComSPS* pcSPS)
{
  xWriteCodeVlc(0x1B5, 32);//extension_start_code
  xWriteCodeVlc(pcSPS->getExtScalability()->getExtId(), 4);					//4, "extension_id", 
  xWriteCodeVlc(pcSPS->getExtScalability()->getNumOfTemporalLevelMinus1(), 3);	//3, "num_of_temporal_level_minus1",
  for (int i = 0; i < pcSPS->getExtScalability()->getNumOfTemporalLevelMinus1(); i++)
  {
    xWriteCodeVlc(pcSPS->getExtScalability()->getTemporalFrameRateCode(i), 4);	//4, "temporal_frame_rate_code[i]", 
    xWriteCodeVlc(pcSPS->getExtScalability()->getTemporalBitRateLower(i), 18);	//18, "temporal_bit_rate_lower[i]", 
    xWriteFlagVlc(1);															//1, "marker_bit", 
    xWriteCodeVlc(pcSPS->getExtScalability()->getTemporalBitRateUpper(i), 12);	//12, "temporal_bit_rate_upper[i]", 
  }
#if AVS3_EXTENSION_DEBUG_SYC
  return;
#endif
}
Void  TEncSbac::codeReservedExt()
{
  xWriteCodeVlc(0x1B5, 32);//extension_start_code
#if !AVS3_EXTENSION_DEBUG_SYC
  xWriteCodeVlc(0, 4);					//4, "extension_id", //wrong syntax
#endif
  xWriteCodeVlc(1, 8);//8, reserved_extension_data_byte
  xWriteCodeVlc(1, 8);//8, reserved_extension_data_byte
#if !AVS3_EXTENSION_DEBUG_SYC
  xWriteCodeVlc(1, 24);				//24,暂时以此作为reserved_extension_data_byte的结束标志
#endif
#if AVS3_EXTENSION_DEBUG_SYC
  return;
#endif
}
#endif
#if AVS3_EXTENSION_CYH
Void  TEncSbac::codeExtCopyright(TComExtCopyright* pcExtCopyright)
{
#if !AVS3_EXTENSION_DEBUG_SYC
  int i;
#endif
  xWriteCodeVlc(0x1B5, 32);//extension_start_code
  xWriteCodeVlc(pcExtCopyright->getExtId(), 4);					//4,"extension_id",
  xWriteFlagVlc((pcExtCopyright->getCopyrightFlag() ? 1 : 0));		//1,"copyright_flag",
  xWriteCodeVlc(pcExtCopyright->getCopyrightId(), 8);			//8,"copyright_id",
  xWriteFlagVlc((pcExtCopyright->getOriginalOrCopy() ? 1 : 0));		//1,"original_or_copy",
  xWriteCodeVlc(0, 7);//7,"reserved_bits",
  xWriteFlagVlc(1);														//1, "marker_bit", 
  xWriteCodeVlc(pcExtCopyright->getCopyrightNum1(), 20);		//20,"copyright_number_1",
  xWriteFlagVlc(1);														//1, "marker_bit", 
  xWriteCodeVlc(pcExtCopyright->getCopyrightNum2(), 22);		//22,"copyright_number_2",
  xWriteFlagVlc(1);														//1, "marker_bit", 
  xWriteCodeVlc(pcExtCopyright->getCopyrightNum3(), 22);		//22,"copyright_number_3",
  //next_start_code
#if AVS3_EXTENSION_DEBUG_SYC
  return;
#endif
}
Void  TEncSbac::codeExtCameraPra(TComExtCameraPra* pcExtCameraPra)
{
#if !AVS3_EXTENSION_DEBUG_SYC
  int i;
#endif
  xWriteCodeVlc(0x1B5, 32);//extension_start_code
  xWriteCodeVlc(pcExtCameraPra->getExtId(), 4);						//4,"extension_id",
  xWriteFlagVlc(0);															//reserved_bits
  xWriteCodeVlc(pcExtCameraPra->getCameraId(), 7);					//7,"camera_id",
  xWriteFlagVlc(1);															//1, "marker_bit", 
  xWriteCodeVlc(pcExtCameraPra->getHeightOfImageDevice(), 22);		//22,"height_of_image_device",
  xWriteFlagVlc(1);															//1, "marker_bit", 
  xWriteCodeVlc(pcExtCameraPra->getFocalLenth(), 22);				//22,"focal_length",
  xWriteFlagVlc(1);															//1, "marker_bit", 
  xWriteCodeVlc(pcExtCameraPra->getFNum(), 22);						//22,"f_number",
  xWriteFlagVlc(1);															//1, "marker_bit",
  xWriteCodeVlc(pcExtCameraPra->getVerAngleOfView(), 22);			//22,"vertical_angle_of_view",
  xWriteFlagVlc(1);															//1, "marker_bit",
  xWriteCodeVlc(pcExtCameraPra->getCameraPosXUpper(), 16);			//16,"camera_position_x_upper",
  xWriteFlagVlc(1);															//1, "marker_bit",
  xWriteCodeVlc(pcExtCameraPra->getCameraPosXLower(), 16);			//16,"camera_position_x_lower",
  xWriteFlagVlc(1);															//1, "marker_bit",
  xWriteCodeVlc(pcExtCameraPra->getCameraPosYUpper(), 16);			//16,"camera_position_y_upper",
  xWriteFlagVlc(1);															//1, "marker_bit",
  xWriteCodeVlc(pcExtCameraPra->getCameraPosYLower(), 16);			//16,"camera_position_y_lower",
  xWriteFlagVlc(1);															//1, "marker_bit",
  xWriteCodeVlc(pcExtCameraPra->getCameraPosZUpper(), 16);			//16,"camera_position_z_upper",
  xWriteFlagVlc(1);															//1, "marker_bit",
  xWriteCodeVlc(pcExtCameraPra->getCameraPosZLower(), 16);			//16,"camera_position_z_lower",
  xWriteFlagVlc(1);															//1, "marker_bit",
  xWriteCodeVlc(pcExtCameraPra->getCameraDirX(), 22);				//22,"camera_direction_x",
  xWriteFlagVlc(1);															//1, "marker_bit",
  xWriteCodeVlc(pcExtCameraPra->getCameraDirY(), 22);				//22,"camera_direction_y",
  xWriteFlagVlc(1);															//1, "marker_bit",
  xWriteCodeVlc(pcExtCameraPra->getCameraDirZ(), 22);				//22,"camera_direction_z",
  xWriteFlagVlc(1);															//1, "marker_bit",
  xWriteCodeVlc(pcExtCameraPra->getImagePlaneVerX(), 22);			//22,"image_plane_vertical_x",
  xWriteFlagVlc(1);															//1, "marker_bit",
  xWriteCodeVlc(pcExtCameraPra->getImagePlaneVerY(), 22);			//22,"image_plane_vertical_y",
  xWriteFlagVlc(1);															//1, "marker_bit",
  xWriteCodeVlc(pcExtCameraPra->getImagePlaneVerZ(), 22);			//22,"image_plane_vertical_z",
  xWriteFlagVlc(1);															//1, "marker_bit",
  xWriteCodeVlc(0, 16);//16,"reserved_bits",
  //next_start_code()
#if AVS3_EXTENSION_DEBUG_SYC
  return;
#endif
}
#endif
#if AVS3_EXTENSION_LYT
Void TEncSbac::codeExtMetadata( TComExtMetadata* pcExtMetadata )
{
  Int c;
  xWriteCodeVlc(0x1B5, 32);//extension_start_code
  xWriteCodeVlc( pcExtMetadata->getExtId(),4 );//4,"extension_id",
  for ( c=0;c<3;c++ )
  {
    xWriteCodeVlc( pcExtMetadata->getDisplayPrimariesX(c),16 );//16,"display_primaries_x[3]",
    xWriteFlagVlc(1);//, "marker_bit"
    xWriteCodeVlc( pcExtMetadata->getDisplayPrimariesY(c),16 );//16,"display_primaries_y[3]",
    xWriteFlagVlc(1);//, "marker_bit"
  }
  xWriteCodeVlc(pcExtMetadata->getWhitePointX(), 16);//16,"white_point_x",
  xWriteFlagVlc(1);//, "marker_bit"
  xWriteCodeVlc(pcExtMetadata->getWhitePointY(), 16);//16,"white_point_y",
  xWriteFlagVlc(1);//, "marker_bit"
  xWriteCodeVlc(pcExtMetadata->getMaxDisplayMasteringLum(), 16);//16,"max_dispaly_mastering_luminance",
  xWriteFlagVlc(1);//, "marker_bit"
  xWriteCodeVlc(pcExtMetadata->getMinDisplayMasteringLum(), 16);//16,"min_dispaly_mastering_luminance",
  xWriteFlagVlc(1);//, "marker_bit"
  xWriteCodeVlc(pcExtMetadata->getMaxContentLightLevel(), 16);//16,"max_content_light_level",
  xWriteFlagVlc(1);//, "marker_bit"
  xWriteCodeVlc(pcExtMetadata->getMaxPicAverageLightLevel(), 16);//16,"max_picture_average_light_level",
  xWriteFlagVlc(1);//, "marker_bit"
  xWriteCodeVlc(0, 16);//16,"reserved_bits",
  return;
}

Void TEncSbac::codeExtRoiPra(TComPicHeader* pcPicHeader)
{
  xWriteCodeVlc(0x1B5, 32);//extension_start_code
  xWriteCodeVlc(pcPicHeader->getExtRoiPra()->getExtId(), 4);//4,"entension_id",
  xWriteCodeVlc(pcPicHeader->getExtRoiPra()->getCurrPicRoiNum(), 8);//8,"current_picture_roi_num",
  Int roiIndex = 0, i, j;
  if (pcPicHeader->getSeqHeader()->getFieldCodedSequence() != 0)//PictureType
  {
    xWriteCodeVlc(pcPicHeader->getExtRoiPra()->getPrevPicRoiNum(), 8);//8,"prev_picture_roi_num"
    for (i = 0; i < pcPicHeader->getExtRoiPra()->getPrevPicRoiNum(); i++)
    {
      xWriteUvlc(pcPicHeader->getExtRoiPra()->getRoiSkipRun());//"roi_skip_run"
      if (pcPicHeader->getExtRoiPra()->getRoiSkipRun() != '0')
      {
        for (j = 0; j < pcPicHeader->getExtRoiPra()->getRoiSkipRun(); j++)
        {
          xWriteCodeVlc(*(pcPicHeader->getExtRoiPra()->getSkipRoiMode() + i + j), 1);//1,"skip_roi_mode[i+j]"
          if (j % 22 == 0)
          {
            xWriteFlagVlc(1);//, "marker_bit"
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
          xWriteFlagVlc(1);
        }
      }
      else
      {
        xWriteSvlc(pcPicHeader->getExtRoiPra()->getRoiAxisxDelta());
        xWriteFlagVlc(1);
        xWriteSvlc(pcPicHeader->getExtRoiPra()->getRoiAxisyDelta());
        xWriteFlagVlc(1);
        xWriteSvlc(pcPicHeader->getExtRoiPra()->getRoiWidthDelta());
        xWriteFlagVlc(1);
        xWriteSvlc(pcPicHeader->getExtRoiPra()->getRoiHeightDelta());
        xWriteFlagVlc(1);
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
    xWriteCodeVlc(pcPicHeader->getExtRoiPra()->getRoiAxisx(), 6);
    xWriteFlagVlc(1);
    xWriteCodeVlc(pcPicHeader->getExtRoiPra()->getRoiAxisy(), 6);
    xWriteFlagVlc(1);
    xWriteCodeVlc(pcPicHeader->getExtRoiPra()->getRoiWidth(), 6);
    xWriteFlagVlc(1);
    xWriteCodeVlc(pcPicHeader->getExtRoiPra()->getRoiHeight(), 6);
    xWriteFlagVlc(1);
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
  return;
}

Void TEncSbac::codeExtPicDisplay(TComPicHeader* pcPicHeader)
{
  Int i, NumberOfFrameCentreOffsets, a;
  xWriteCodeVlc(0x1B5, 32);//extension_start_code
  xWriteCodeVlc(pcPicHeader->getExtPicDisplay()->getExtId(), 4);
  if (pcPicHeader->getSeqHeader()->getProgressiveSequence() == 1)
  {
    if (pcPicHeader->getRepeatFirstField() == 1)
    {
      if (pcPicHeader->getTopFieldFirst() == 1)
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
    if (pcPicHeader->getRepeatFirstField() == 1)
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
    a = 16 * (i + 1);
    xWriteCodeVlc(a, 16);//16,"picture_centre_horizontal_offset"
    xWriteFlagVlc(1);//, "marker_bit"
    xWriteCodeVlc(a, 16);//16,"picture_centre_vertical_offset"
    xWriteFlagVlc(1);//, "marker_bit"
  }
  return;
}
#endif

#if !AVS3_DELETE_PPS_SYC
Void TEncSbac::codePPS( TComPPS* pcPPS )
{
  // uiFirstByte
  xWriteCodeVlc( NAL_REF_IDC_PRIORITY_HIGHEST, 2);
  xWriteCodeVlc( 0, 1);
  xWriteCodeVlc( NAL_UNIT_PPS, 5);
  return;
}
#endif
#if AVS3_PIC_HEADER_ZL
Void TEncSbac::codeIPicHeader( TComPicHeader* pcPicHeader )
{

  //IPicHeader
  xWriteCodeVlc( pcPicHeader->getIntraPicStartCode(),32 );//32, "Intra_Pic_Start_Code"
  xWriteCodeVlc( pcPicHeader->getBbvDelay(),32 );//32, "bbv delay"

  xWriteFlagVlc( pcPicHeader->getTimeCodeFlag()? 1 : 0 );//1, "time_code_flag"

  if ( pcPicHeader->getTimeCodeFlag() )
  {
    //计算TimeCode，计算过程还未加入
    xWriteCodeVlc( pcPicHeader->getTimeCode(), 24 );//24, "time_code"
  }

  if ( !pcPicHeader->getSeqHeader()->getScenePictureDisableFlag() )
  {
    //背景帧默认关闭,如果打开背景帧需要重新setScenePicFlag()
    xWriteFlagVlc( pcPicHeader->getScenePicFlag()? 1 : 0 );//1, "scene_picture_flag"
    if ( pcPicHeader->getScenePicFlag() )
    {
      //加入背景帧之后需要重新setScenePicOutputFlag()
      xWriteFlagVlc( pcPicHeader->getScenePicOutputFlag()? 1 : 0 );//1, "scene_picture_output_flag"
    }
  }

  //decode_order_index还未加入计算过程，decode_order_index=coding_order % 256;
#if RPS_BUG  //暂时写入POC的值 LDP RA不支持   
#if BUG_816
  xWriteCodeVlc(pcPicHeader->getPOC(), 10);//8, "coding_order"
#else
  xWriteCodeVlc((pcPicHeader->getPOC() % 256), 8);//8, "coding_order"
#endif
#else
  xWriteCodeVlc(pcPicHeader->getDecOrderIndex(), 8);//8, "coding_order"
#endif
  if ( pcPicHeader->getSeqHeader()->getTemporalIdEnableFlag() )
  {
    //时间层标识允许标志和RPS有关，需要重新set
    xWriteCodeVlc( pcPicHeader->getTemporalId(),3 );// "temporal_id"
  }

  //PicOutputDelay需要重新set,还未加入其计算过程
  UInt uiPicOutputDelay = 0;
  if ( !pcPicHeader->getSeqHeader()->getLowDelay() )
  {
    //RD：uiPicOutputDelay = img->tr - hc->coding_order + pcPicHeader->getSeqHeader()->getOutputReorderDelay();
    pcPicHeader->setPicOutputDelay( uiPicOutputDelay );
  }

  if ( !pcPicHeader->getSeqHeader()->getLowDelay() && !(!pcPicHeader->getSeqHeader()->getScenePictureDisableFlag() && !pcPicHeader->getScenePicOutputFlag()) )
  {
    xWriteUvlc( pcPicHeader->getPicOutputDelay() );//"picture_output_delay"
  }

#if RPS
  Int RPSIdx = (pcPicHeader->getPOC() - 1) % pcPicHeader->getGOPSize();
  xWriteFlagVlc((pcPicHeader->getUseRcsFlag()||(pcPicHeader->getIntraPeriod()==1)) ? 1 : 0);//1, "use RCS in SPS"
  if ((pcPicHeader->getUseRcsFlag() || (pcPicHeader->getIntraPeriod() == 1)))
  {

	  xWriteCodeVlc(RPSIdx, 5);//5, "Rcs_Index"
  }
  else
  {
	  xWriteFlagVlc(pcPicHeader->getRPS()->getRefered() ? 1 : 0);
	  xWriteCodeVlc(pcPicHeader->getRPS()->getNumberOfRefPictures(), 3);
	  for (Int i = 0; i < pcPicHeader->getRPS()->getNumberOfRefPictures(); i++)
	  {
		  xWriteCodeVlc(abs(pcPicHeader->getRPS()->getRefDeltaPOC(i)), 6);
	  }
	  xWriteCodeVlc(pcPicHeader->getRPS()->getNumberOfRemPictures(), 3);
	  for (Int j = 0; j < pcPicHeader->getRPS()->getNumberOfRemPictures(); j++)
	  {
		  xWriteCodeVlc(abs(pcPicHeader->getRPS()->getRemDeltaPOC(j)), 6);
	  }
	  xWriteFlagVlc(1);
  }
#else

  //参考帧管理
  xWriteFlagVlc( pcPicHeader->getUseRcsFlag()? 1 : 0 );//1, "use RCS in SPS"
  if ( pcPicHeader->getUseRcsFlag() )
  {
    xWriteCodeVlc( pcPicHeader->getRcsIndex(), 5 );//5, "Rcs_Index"
  }
  else
  {
    //参考配置集，参考帧管理 
  }
#endif
  if ( pcPicHeader->getSeqHeader()->getLowDelay() )
  {
    xWriteUvlc( pcPicHeader->getBbvCheckTimes() );//"bbv check times"
  }
  xWriteFlagVlc( pcPicHeader->getProgressiveFrame()? 1 : 0 );//1, "progressive frame"
  if( !pcPicHeader->getProgressiveFrame() )
  {
    xWriteFlagVlc( (pcPicHeader->getPicStructure() == PS_BLEND )? 1 : 0 );//1, "picture_structure"
  }
  xWriteFlagVlc( pcPicHeader->getTopFieldFirst()? 1 : 0 );//1, "top field first"
  xWriteFlagVlc( pcPicHeader->getRepeatFirstField()? 1 : 0 );//1, "repeat first field"

  //RD中if条件是interlaceCodingOption==3
  if( pcPicHeader->getSeqHeader()->getFieldCodedSequence() )
  {
    xWriteFlagVlc( pcPicHeader->getTopFieldPicFlag()? 1 : 0 );//1, "is top field"
    xWriteFlagVlc( 1 );//1, "reserved bit for interlace coding"
  }
  xWriteFlagVlc( pcPicHeader->getFixedPicQP()? 1 : 0 );//1, "fixed picture qp"
  if( !pcPicHeader->getSeqHeader()->getScenePictureDisableFlag() )
  {
    //背景帧QP写入码流
  }
  else
  {
    xWriteCodeVlc( pcPicHeader->getPicQP(),7 );//7, "I picture QP"
  }

  xWriteFlagVlc( pcPicHeader->getLoopFilterDisableFlag()? 1 : 0 );//1, "loop filter disable"
  if ( !pcPicHeader->getLoopFilterDisableFlag() )
  {
    xWriteFlagVlc( pcPicHeader->getLoopFilterSyntax()->m_bLoppFilterParamFlag? 1 : 0 );//1, "loop filter parameter flag"
    if( pcPicHeader->getLoopFilterSyntax()->m_bLoppFilterParamFlag )
    {
      xWriteUvlc( pcPicHeader->getLoopFilterSyntax()->m_iAlphaCOffset );//"alpha offset"
      xWriteUvlc( pcPicHeader->getLoopFilterSyntax()->m_iBetaOffset );//"beta offset"
    }
    else
    {
      pcPicHeader->getLoopFilterSyntax()->m_iAlphaCOffset = 0;
      pcPicHeader->getLoopFilterSyntax()->m_iBetaOffset = 0;
    }
  }

  xWriteFlagVlc( pcPicHeader->getChromaQuantParamDisableFlag()? 1 : 0 );//1, "chroma_quant_param_disable"
  if ( !pcPicHeader->getChromaQuantParamDisableFlag() )
  {
    xWriteUvlc( pcPicHeader->getChromaQuantParamDeltaCb() );//"chroma_quant_param_delta_cb"
    xWriteUvlc( pcPicHeader->getChromaQuantParamDeltaCr() );//"chroma_quant_param_delta_cr"
  }
  else
  {
    pcPicHeader->setChromaQuantParamDeltaCb( 0 );
    pcPicHeader->setChromaQuantParamDeltaCr( 0 );
  }

  if ( pcPicHeader->getSeqHeader()->getWeightQuantEnableFlag() )
  {
    xWriteFlagVlc( pcPicHeader->getPicWeightQuantEnableFlag()? 1 : 0 );//1, "pic_weight_quant_enable"
    if( pcPicHeader->getPicWeightQuantEnableFlag() )
    {
      xWriteCodeVlc(  pcPicHeader->getWeightQuantSyntax()->m_uiPicWeightQuantDataIndex, 2 );//2, "pic_weight_quant_data_index"
      if (pcPicHeader->getWeightQuantSyntax()->m_uiPicWeightQuantDataIndex == 1 )
      {
        xWriteFlagVlc( 0 );//1, "reserved_bits"
        xWriteCodeVlc(pcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantParamIndex, 2 );//2, "weighting_quant_param_index"
        xWriteCodeVlc(pcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantMode, 2 );//2, "weighting_quant_model"
        if ( pcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantParamIndex == 1 )
        {
          for( int i = 0; i < 6; i++ )
          {
            xWriteUvlc( pcPicHeader->getWeightQuantSyntax()->m_iWeightQuantParamDelta1[i] );//"quant_param_delta1"
          }
        }
        if ( pcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantParamIndex == 2 )
        {
          for( int i = 0; i < 6; i++ )
          {
            xWriteUvlc( pcPicHeader->getWeightQuantSyntax()->m_iWeightQuantParamDelta2[i] );////"quant_param_delta2"
          }
        }
      }//if (pcPicHeader->getWeightQuantSyntax()->m_uiPicWeightQuantDataIndex == 1 )
      else if ( pcPicHeader->getWeightQuantSyntax()->m_uiPicWeightQuantDataIndex == 2 )
      {
        for ( int iSizeId = 0; iSizeId < 2; iSizeId++ )
        {
          UInt WQMSize = 1 << (iSizeId + 2);
          for ( int i = 0; i < WQMSize; i++ )
          {
            for ( int j = 0; j < WQMSize; j++ )
            {
              if ( iSizeId == 0 )
              {
                xWriteUvlc( pcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantCoeff4x4[i][j] );//"weight_quant_coeff"
              }
              else
              {
                xWriteUvlc( pcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantCoeff8x8[i][j] );//"weight_quant_coeff"
              }
            }
          }
        }
      }//if ( pcPicHeader->getWeightQuantSyntax()->m_uiPicWeightQuantDataIndex == 2 )
    }//if( pcPicHeader->getPicWeightQuantEnableFlag() )
  }//if ( pcPicHeader->getSeqHeader()->getWeightQuantEnableFlag() )

  if ( pcPicHeader->getSeqHeader()->getAdaptiveLoopFilterEnableFlag() )
  {
    for( int i = 0; i < 3; i++ )
    {
      xWriteFlagVlc( pcPicHeader->getPicAlfEnableFlag(i)? 1 : 0 );//1, "picture_alf_enable_flag"
    }
    if ( pcPicHeader->getPicAlfEnableFlag(0) || pcPicHeader->getPicAlfEnableFlag(1) || pcPicHeader->getPicAlfEnableFlag(2) )
    {
      if ( pcPicHeader->getPicAlfEnableFlag(0) )
      {
        xWriteUvlc( pcPicHeader->getAlfFilterSyntax()->m_uiAlfFilterNumMinus1 );//"alf_filter_num_minus1"     
        for ( int i = 0; i < (pcPicHeader->getAlfFilterSyntax()->m_uiAlfFilterNumMinus1 + 1); i++ )
        {
          if ( i > 0 && pcPicHeader->getAlfFilterSyntax()->m_uiAlfFilterNumMinus1 != 15 )
          {
            xWriteUvlc( pcPicHeader->getAlfFilterSyntax()->m_uiAlfRegionDistance[i] );//"alf_region_distance"
          }
          for ( int j = 0; j < 9; j++ )
          {
            xWriteUvlc( pcPicHeader->getAlfFilterSyntax()->m_iAlfCoeffLuma[i][j] );//"alf_coeff_luma"
          }
        }
      }//if(pcPicHeader->getPicAlfEnableFlag(0))
      if ( pcPicHeader->getPicAlfEnableFlag(1) )
      {
        for ( int j = 0; j < 9; j++ )
        {
          xWriteUvlc( pcPicHeader->getAlfFilterSyntax()->m_iAlfCoeffChroma[0][j] );//"alf_coeff_chroma"
        }
      }
      if ( pcPicHeader->getPicAlfEnableFlag(2))
      {
        for ( int j = 0; j < 9; j++ )
        {
          xWriteUvlc( pcPicHeader->getAlfFilterSyntax()->m_iAlfCoeffChroma[1][j] );//"alf_coeff_chroma"
        }
      }
    }//if( pcPicHeader->getPicAlfEnableFlag(0) || pcPicHeader->getPicAlfEnableFlag(1) || pcPicHeader->getPicAlfEnableFlag(2) )
  }
  return;
}

Void TEncSbac::codePBPicHeader( TComPicHeader* pcPicHeader )
{
  //PBPicHeader
  xWriteCodeVlc( pcPicHeader->getInterPicStartCode(),32 );//32, "inter picture start code"
  xWriteCodeVlc( pcPicHeader->getBbvDelay(),32 );//32, "bbv delay"

  //根据PictureType来决定Pic_Coding_type,需要重新set
#if RPS_BUG
  xWriteCodeVlc(pcPicHeader->getPictureType(), 2);//2, "picture coding type"
#else
  xWriteCodeVlc(pcPicHeader->getPicCodingType(), 2 );//2, "picture coding type"
#endif
  //根据PicType,PicTypeB,ScenePicEnable来决定ScenePredFlag是false还是true
  if ( !pcPicHeader->getSeqHeader()->getScenePictureDisableFlag() && pcPicHeader->getPicCodingType() == 1 )
  {
    //背景帧暂时关闭
    xWriteFlagVlc( pcPicHeader->getScenePredFlag()? 1 : 0 );//1, "scene_pred_flag"
  }
  if ( !pcPicHeader->getSeqHeader()->getScenePictureDisableFlag() && pcPicHeader->getPicCodingType() != 2 )
  {
    if ( !pcPicHeader->getScenePredFlag() )
    {
      xWriteFlagVlc( pcPicHeader->getSceneRefEnableFlag()? 1 : 0 );//1, "background_reference_enable"
    }
  }
  //与coding_order有关，需要重新set
  //需要计算pcPicHeader->setDecOrderIndex(coding order % 256);
#if RPS_BUG  //暂时写入POC的值 LDP RA不支持   
#if BUG_816
  xWriteCodeVlc(pcPicHeader->getPOC(), 10);//8, "coding_order"
#else
  xWriteCodeVlc((pcPicHeader->getPOC() % 256), 8);//8, "coding_order"
#endif
#else
  xWriteCodeVlc(pcPicHeader->getDecOrderIndex(), 8);//8, "coding_order"
#endif
  if ( pcPicHeader->getSeqHeader()->getTemporalIdEnableFlag() )
  {
    //时间层标识允许标志和RPS有关，需要重新set
    //RD：if (img->typeb == BACKGROUND_IMG || (img->type == INTER_IMG && img->typeb == BP_IMG)) {he->cur_layer = 0;｝
    xWriteCodeVlc( pcPicHeader->getTemporalId(), 3 );//3, "temporal_id"
  }

  //PicOutputDelay暂时设置为0 
  UInt uiPicOutputDelay = 0;                            
  if ( !pcPicHeader->getSeqHeader()->getLowDelay() )
  {
    //需要计算RD:uiPicOutputDelay = img->tr - hc->coding_order + pcPicHeader->getSeqHeader()->getOutputReorderDelay();
    pcPicHeader->setPicOutputDelay( uiPicOutputDelay );
    xWriteUvlc( pcPicHeader->getPicOutputDelay() );//"displaydelay"
  }

#if RPS
  Int RPSIdx = (pcPicHeader->getPOC() - 1) % pcPicHeader->getGOPSize();
  xWriteFlagVlc((pcPicHeader->getUseRcsFlag() || (pcPicHeader->getIntraPeriod() == 1)) ? 1 : 0);//1, "use RCS in SPS"
  if ((pcPicHeader->getUseRcsFlag() || (pcPicHeader->getIntraPeriod() == 1)))
  {

    xWriteCodeVlc(RPSIdx, 5);//5, "Rcs_Index"
  }
  else
  {
    xWriteFlagVlc(pcPicHeader->getRPS()->getRefered() ? 1 : 0);
    xWriteCodeVlc(pcPicHeader->getRPS()->getNumberOfRefPictures(), 3);
    for (Int i = 0; i < pcPicHeader->getRPS()->getNumberOfRefPictures(); i++)
    {
      xWriteCodeVlc(abs(pcPicHeader->getRPS()->getRefDeltaPOC(i)), 6);
    }
    xWriteCodeVlc(pcPicHeader->getRPS()->getNumberOfRemPictures(), 3);
    for (Int j = 0; j < pcPicHeader->getRPS()->getNumberOfRemPictures(); j++)
    {
      xWriteCodeVlc(abs(pcPicHeader->getRPS()->getRemDeltaPOC(j)), 6);
    }
    xWriteFlagVlc(1);
  }
#else

  //参考帧管理
  xWriteFlagVlc(pcPicHeader->getUseRcsFlag() ? 1 : 0);//1, "use RCS in SPS"
  if (pcPicHeader->getUseRcsFlag())
  {
    xWriteCodeVlc(pcPicHeader->getRcsIndex(), 5);//5, "Rcs_Index"
  }
  else
  {
    //参考配置集，参考帧管理 
  }
#endif
  if ( pcPicHeader->getSeqHeader()->getLowDelay() )
  {
    xWriteUvlc( pcPicHeader->getBbvCheckTimes() );//"bbv check times"
  }
  xWriteFlagVlc( pcPicHeader->getProgressiveFrame()? 1 : 0 );//1, "progressive frame"
  if( !pcPicHeader->getProgressiveFrame() )
  {
    xWriteFlagVlc( (pcPicHeader->getPicStructure() == PS_BLEND )? 1 : 0 );//1, "picture_structure"
  }
  xWriteFlagVlc( pcPicHeader->getTopFieldFirst()? 1 : 0 );//1, "top field first"
  xWriteFlagVlc( pcPicHeader->getRepeatFirstField()? 1 : 0 );//1, "repeat first field"

  //RD中的if条件为interlaceCodingOption==3
  if( pcPicHeader->getSeqHeader()->getFieldCodedSequence() )
  {
    xWriteFlagVlc( pcPicHeader->getTopFieldPicFlag()? 1 : 0 );//1, "is top field"
    xWriteFlagVlc( 1 );//1, "reserved bit for interlace coding"
  }
  xWriteFlagVlc( pcPicHeader->getFixedPicQP()? 1 : 0 );//1, "fixed qp"

  //根据PicType,传QP进码流。分：INTER_IMG、 F_IMG、 B_IMG
  xWriteCodeVlc( pcPicHeader->getPicQP(),7 );

  if( !(pcPicHeader->getPicCodingType() == 2 && pcPicHeader->getPicStructure() == 1) )
  {
    xWriteFlagVlc( 0 );//1, "reserved_bit"
  }
  //RA配置
  //if(temporal_reference >= next_IDRtemporal_reference)
  //  pcPicHeader->setRADecodableFlag(true);
  //else
  //  pcPicHeader->setRADecodableFlag(false);
  xWriteFlagVlc( pcPicHeader->getRADecodableFlag()? 1: 0 );//1, "random_access_decodable_flag"

  xWriteFlagVlc( pcPicHeader->getLoopFilterDisableFlag()? 1 : 0 );//1, "loop filter disable"
  if ( !pcPicHeader->getLoopFilterDisableFlag() )
  {
    xWriteFlagVlc( pcPicHeader->getLoopFilterSyntax()->m_bLoppFilterParamFlag? 1 : 0 );//1, "loop filter parameter flag"
    if( pcPicHeader->getLoopFilterSyntax()->m_bLoppFilterParamFlag )
    {
      xWriteUvlc( pcPicHeader->getLoopFilterSyntax()->m_iAlphaCOffset );//"alpha offset"
      xWriteUvlc( pcPicHeader->getLoopFilterSyntax()->m_iBetaOffset );//"beta offset"
    }
    else
    {
      pcPicHeader->getLoopFilterSyntax()->m_iAlphaCOffset = 0;
      pcPicHeader->getLoopFilterSyntax()->m_iBetaOffset = 0;
    }
  }

  xWriteFlagVlc( pcPicHeader->getChromaQuantParamDisableFlag()? 1 : 0 );//1, "chroma_quant_param_disable"
  if ( !pcPicHeader->getChromaQuantParamDisableFlag() )
  {
    xWriteUvlc( pcPicHeader->getChromaQuantParamDeltaCb() );//"chroma_quant_param_delta_cb"
    xWriteUvlc( pcPicHeader->getChromaQuantParamDeltaCr() );//"chroma_quant_param_delta_cr"
  }
  else
  {
    pcPicHeader->setChromaQuantParamDeltaCb( 0 );
    pcPicHeader->setChromaQuantParamDeltaCr( 0 );
  }

  if ( pcPicHeader->getSeqHeader()->getWeightQuantEnableFlag() )
  {
    xWriteFlagVlc( pcPicHeader->getPicWeightQuantEnableFlag()? 1 : 0 );//1, "pic_weight_quant_enable"
    if( pcPicHeader->getPicWeightQuantEnableFlag() )
    {
      xWriteCodeVlc( pcPicHeader->getWeightQuantSyntax()->m_uiPicWeightQuantDataIndex, 2 );//2, "pic_weight_quant_data_index"
      if (pcPicHeader->getWeightQuantSyntax()->m_uiPicWeightQuantDataIndex == 1 )
      {
        xWriteFlagVlc( 0 );//1, "reserved_bits"
        xWriteCodeVlc( pcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantParamIndex, 2 );//2, "weighting_quant_param_index"
        xWriteCodeVlc( pcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantMode, 2 );//2, "weighting_quant_model"
        if ( pcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantParamIndex == 1 )
        {
          for( int i = 0; i < 6; i++ )
          {
            xWriteUvlc( pcPicHeader->getWeightQuantSyntax()->m_iWeightQuantParamDelta1[i] );//"quant_param_delta1"
          }
        }
        if ( pcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantParamIndex == 2 )
        {
          for( int i = 0; i < 6; i++ )
          {
            xWriteUvlc( pcPicHeader->getWeightQuantSyntax()->m_iWeightQuantParamDelta2[i] );//"quant_param_delta2"
          }
        }
      }
      else if ( pcPicHeader->getWeightQuantSyntax()->m_uiPicWeightQuantDataIndex == 2 )
      {
        for ( int iSizeId = 0; iSizeId < 2; iSizeId++ )
        {
          UInt WQMSize = 1 << (iSizeId + 2);
          for ( int i = 0; i < WQMSize; i++ )
          {
            for ( int j = 0; j < WQMSize; j++ )
            {
              if ( iSizeId == 0 )
              {
                xWriteUvlc( Clip3(1, 255, pcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantCoeff4x4[i][j]) );//"weight_quant_coeff"
              }
              else
              {
                xWriteUvlc( Clip3(1, 255, pcPicHeader->getWeightQuantSyntax()->m_uiWeightQuantCoeff8x8[i][j]) );//"weight_quant_coeff"
              }
            }
          }
        }
      }//if ( pcPicHeader->getWeightQuantSyntax()->m_uiPicWeightQuantDataIndex == 2 )
    }//if( pcPicHeader->getPicWeightQuantEnableFlag() )
  }//if ( pcPicHeader->getSeqHeader()->getWeightQuantEnableFlag() )

  if ( pcPicHeader->getSeqHeader()->getAdaptiveLoopFilterEnableFlag() )
  {
    for( int i = 0; i < 3; i++ )
    {
      xWriteFlagVlc( pcPicHeader->getPicAlfEnableFlag(i)? 1 : 0 );//1, "picture_alf_enable_flag"
    }
    if ( pcPicHeader->getPicAlfEnableFlag(0) || pcPicHeader->getPicAlfEnableFlag(1) || pcPicHeader->getPicAlfEnableFlag(2) )
    {
      if ( pcPicHeader->getPicAlfEnableFlag(0) )
      {
        xWriteUvlc( pcPicHeader->getAlfFilterSyntax()->m_uiAlfFilterNumMinus1 ); //"alf_filter_num_minus1"    
        for ( int i = 0; i < (pcPicHeader->getAlfFilterSyntax()->m_uiAlfFilterNumMinus1 + 1); i++ )
        {
          if (i > 0 && pcPicHeader->getAlfFilterSyntax()->m_uiAlfFilterNumMinus1 != 15 )
          {
            xWriteUvlc( pcPicHeader->getAlfFilterSyntax()->m_uiAlfRegionDistance[i] );//"alf_region_distance"
          }
          for ( int j = 0; j < 9; j++ )
          {
            xWriteUvlc( pcPicHeader->getAlfFilterSyntax()->m_iAlfCoeffLuma[i][j] );//"alf_coeff_luma"
          }
        }
      }//if(pcPicHeader->getPicAlfEnableFlag(0))
      if ( pcPicHeader->getPicAlfEnableFlag(1) )
      {
        for ( int j = 0; j < 9; j++ )
        {
          xWriteUvlc( pcPicHeader->getAlfFilterSyntax()->m_iAlfCoeffChroma[0][j] );//"alf_coeff_chroma"
        }
      }
      if ( pcPicHeader->getPicAlfEnableFlag(2))
      {
        for ( int j = 0; j < 9; j++ )
        {
          xWriteUvlc( pcPicHeader->getAlfFilterSyntax()->m_iAlfCoeffChroma[1][j] );//"alf_coeff_chroma"
        }
      }
    }//if( pcPicHeader->getPicAlfEnableFlag(0) || pcPicHeader->getPicAlfEnableFlag(1) || pcPicHeader->getPicAlfEnableFlag(2) )
  }

  return;
}
#endif



Void TEncSbac::codePictureHeader( TComPicture* pcPicture )
{
#if AVS3_SLICE_HEADER_SYC
  xWriteCodeVlc (pcPicture->getSliceStartCode(),32);
  if (pcPicture->getSPS()->getVerticalSize() > (144 * pcPicture->getSPS()->getLCUSize()))
  {
    xWriteCodeVlc (pcPicture->getSliceVerPosExtension(),3);
  }
  xWriteCodeVlc (pcPicture->getSliceHorPos(),8);
  if (pcPicture->getSPS()->getHorizontalSize() > (255 * pcPicture->getSPS()->getLCUSize()))
  {
    xWriteCodeVlc (pcPicture->getSliceHorPosExtension(),2);
  }
  if(pcPicture->getPic()->getPicHeader()->getFixedPicQP() == false)
  {
    xWriteFlagVlc (pcPicture->getFixedSliceQP() ? 1:0);
    xWriteCodeVlc (pcPicture->getSliceQP(),7);
  }
  if (pcPicture->getSPS()->getSampleAdaptiveOffsetEnableFlag())
  {
    for (UInt uicompIdx = 0; uicompIdx < 3; uicompIdx ++)
    {
      xWriteFlagVlc (pcPicture->getSliceSaoEnableFlag(uicompIdx) ? 1:0);
    }
  }
#endif

#if YQH_AVS2_BITSTREAM
  // xWriteCodeVlc  (pcPicture->getPOC(), 10 );   //  9 == SPS->Log2MaxFrameNum

#else

  // here someone can add an appropriated NalRefIdc type 
  xWriteCodeVlc( NAL_REF_IDC_PRIORITY_HIGHEST, 2);
  xWriteCodeVlc( 0, 1);
  xWriteCodeVlc( NAL_UNIT_CODED_PICTURE, 5);
#if !RPS_BUG
  xWriteCodeVlc  (pcPicture->getPOC(), 10 );   //  9 == SPS->Log2MaxFrameNum
  xWriteUvlc  (pcPicture->getPictureType() );
#endif  
  xWriteSvlc  (pcPicture->getPictureQp() );
  xWriteSvlc  ( pcPicture->getPictureQp() - pcPicture->getPictureChromaQp() );
  
  if (!pcPicture->isIntra())
  {
    xWriteFlagVlc  (pcPicture->isReferenced() ? 1 : 0);
  }
  
  xWriteFlagVlc  (pcPicture->getLoopFilterDisable());
  xWriteFlagVlc  (pcPicture->getDRBFlag() ? 1 : 0 );
#endif
  return;
}

Void TEncSbac::codeTerminatingBit( UInt uiSymbol )
{
#if wlq_AEC
	xEncodeBinF(uiSymbol);
#else
  m_uiRange--;

  if( uiSymbol ) 
  {
    m_uiCode += m_uiRange;
    m_uiRange = 1;
  }
  else 
  {
    if ( m_uiRange >= 0x8000 ) 
    {
      return;
    }
  }
  do 
  {
    m_uiRange <<= 1;
    m_uiCode <<= 1;
    if (--m_uiCodeBits == 0) 
    {
      xCarryPropagate( m_uiCode );
      m_uiCodeBits = 8;
    }
  } while (m_uiRange < 0x8000);
  #endif
}

Void TEncSbac::codePictureFinish()
{
#if wlq_AEC
	m_pcBitIf->flushBuffer();
#else
  UInt uiTemp = (m_uiCode + m_uiRange - 1) & 0xFFFF0000;

  if (uiTemp < m_uiCode) {
    uiTemp += 0x8000;
  }

  m_uiCode = uiTemp << m_uiCodeBits;
  xCarryPropagate(m_uiCode);

  m_uiCode <<= 8;
  xCarryPropagate(m_uiCode);

  xPutByte(0x00);
  while (m_uiStackedZeros > 0) {
    m_pcBitIf->write(0x00, 8);
    m_uiStackedZeros--;
  } 
  #endif
}

Void TEncSbac::load ( TEncSbac* pScr)
{
  this->xCopyFrom(pScr);
}

Void TEncSbac::store( TEncSbac* pDest)
{
  pDest->xCopyFrom( this );
}

Void TEncSbac::resetBits()
{ 
#if !wlq_AEC
	m_uiCode &= 0x7FFFF; 
#endif
 // m_uiCode &= 0x7FFFF; 
  m_uiCodeBits = 11; 
  m_ucPendingByte = 0; 
  m_bIsPendingByte = false; 
  m_uiStackedFFs = 0; 
  m_uiStackedZeros = 0;
#if 0//YQH_AEC_RDO_BUG
  m_uiByteCounter = 9;
  m_uiCodeBitsbiggo = 9;
#endif
  m_pcBitIf->resetBits(); 
}

UInt TEncSbac::getNumberOfWrittenBits()                
{ 
#if YQH_AEC_RDO_BUG

	//m_uiByteCounter=m_pcBitIf->getNumberOfWrittenBytes();
	//m_uiCodeBitsbiggo=m_pcBitIf->getCodeBits();
	//	if(m_pcBitIf->getCodeBits()!=11&&m_pcBitIf->getCodeBits()!=9) // m_Ebits_to_follow 
	//	printf("error m_Ebits_to_follow =%d m_uiCodeBits=%d m_s1=%d \n",m_pcBitIf->getNumberOfWrittenBits(),m_Ebits_to_follow,m_pcBitIf->getCodeBits());
	// return  8*m_pcBitIf->getNumberOfWrittenBytes() + m_Ebits_to_follow  + 8 - m_uiCodeBitsbiggo + m_E_s1;
	return 8 * m_pcBitIf->getNumberOfWrittenBytes() + m_Ebits_to_follow + 8 - m_pcBitIf->getCodeBits() + m_E_s1;
	// return 8*m_pcBitIf->getNumberOfWrittenBytes() + m_Ebits_to_follow  + 8 - m_pcBitIf->getCodeBits() + m_E_s1;
#else
	return m_pcBitIf->getNumberOfWrittenBits() + 8 * (m_uiStackedZeros + m_uiStackedFFs) + 8 * (m_bIsPendingByte ? 1 : 0) + 8 - m_uiCodeBits + 3;
#endif
}

Void TEncSbac::codePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  // get context function is here
  Int iPredMode = pcCU->getPredictionMode( uiAbsPartIdx );
#if inter_intra_1
  return;
#endif
  xEncodeBin( iPredMode == MODE_SKIP  ? 1 : 0, m_cCUPredModeSCModel.get( 0, 0, 0 ) );

  if ( iPredMode == MODE_SKIP )
  {
    return;
  }

  xEncodeBin( iPredMode == MODE_INTER ? 0 : 1, m_cCUPredModeSCModel.get( 0, 0, 1 ) );
}

#if YQH_INTRA
Void TEncSbac::codeTransSplitFlag(TComDataCU* pcCU, UInt uiAbsPartIdx) //yuquanhe
{
	PartSize eSize = pcCU->getPartitionSize(uiAbsPartIdx);
	UInt act_ctx = (eSize == SIZE_2Nx2N || eSize == SIZE_NxN || eSize == SIZE_2NxhN || eSize == SIZE_hNx2N) ? 1 : 0;
	if (pcCU->isIntra(uiAbsPartIdx))  //zhangyiCheck: IntraCuFlag == 1?
	{
#if ZHANGYI_INTRA
		PictureType cuPicType = pcCU->getPicture()->getPictureType();
		if ((pcCU->isSkip(uiAbsPartIdx)) && ((cuPicType == P_PICTURE) || (cuPicType == B_PICTURE) || (cuPicType == F_PICTURE)))
		{
			return;
		}
		UInt SizeInBit = pcCU->getLog2CUSize(uiAbsPartIdx);
		if ((SizeInBit == 3) || (((pcCU->getPicture()->getSPS()->getNonsquareIntraPredictionEnableFlag() == 1) && ((SizeInBit > 3) && (SizeInBit < 6)))))  //zhangyiCheck:要保证在这个条件不满足的情况下，cu的划分都是2NX2N
#endif
		{
			if ((SizeInBit == 4) || (SizeInBit == 5))
				act_ctx++;
			assert(act_ctx<3);
			xEncodeBin(eSize == SIZE_2Nx2N ? 0 : 1, m_cTUSplitFlagSCModel.get(0, 0, act_ctx));
		}
		return;
	}
}


#endif

#if INTER_GROUP
//AVS3 write cu_type_index 
Void TEncSbac::codeShapeOfPartitionIndex(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
	PartSize eSize = pcCU->getPartitionSize(uiAbsPartIdx);
	if (eSize == SIZE_2NxN || eSize == SIZE_Nx2N)
	{
		//write(0);
		xEncodeBin(1, m_cCUShapeOfPartitionSCModel.get(0, 0, 0));
	}
	else if (eSize == SIZE_2NxnU || SIZE_nLx2N)
	{
		//write(1);
		xEncodeBin(0, m_cCUShapeOfPartitionSCModel.get(0, 0, 0));
		xEncodeBin(1, m_cCUShapeOfPartitionSCModel.get(0, 0, 1));
	}
	else if (eSize == SIZE_2NxnD || SIZE_nRx2N)
	{
		//write(2);
		xEncodeBin(0, m_cCUShapeOfPartitionSCModel.get(0, 0, 0));
		xEncodeBin(0, m_cCUShapeOfPartitionSCModel.get(0, 0, 1));
	}
	else
	{
		return;
	}
	return;
}

Void TEncSbac::codeInterCUTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx)//codeInterCUTypeIndex?
{
#if	inter_intra_1
	int  binidx = 0;

	int act_ctx = 0;
	int act_sym;

	//const int MapPCUType[14] = {1, 3, 4, 6, 4, 4, 6, 6, 5, 2, 2,2,2,2};
	const int MapPCUType[14] = { 1, 2, 3, 4, 3, 3, 4, 4, 5, 6, 6, 6, 6, 6 };
	//const int MapBCUType[14] = {5, 1, 2, 3, 2, 2, 3, 3, 4, 6, 6,6,6,6}; //  0, 1, 2, 3, 4, 5, 6, 7, PNXN, I16MB, I8MB,xxx? InNxN,INxnN
#if inter_intra_2
	const int MapBCUType[14] = { 1, 2, 3, 4, 3, 3, 4, 4, 5, 6, 6, 6, 6, 6 }; //  0, 1, 2, 3, 4, 5, 6, 7, PNXN, I16MB, I8MB,xxx? InNxN,INxnN
#else
	const int MapBCUType[14] = { 33 }; //  0, 1, 2, 3, 4, 5, 6, 7, PNXN, I16MB, I8MB,xxx? InNxN,INxnN
#endif

	const int MapPCUTypeMin[14] = { 1, 2, 3, 4, 3, 3, 4, 4, -1, 5, 5, 5, 5, 5 };
	const int MapBCUTypeMin[14] = { 1, 2, 3, 4, 3, 3, 4, 4, -1, 5, 5, 5, 5, 5 };
	int max_bit = 0;
	int real_bit = 0;

	if (pcCU->getPicture()->getPictureType() != I_PICTURE)
	{
		if (pcCU->getLog2CUSize(uiAbsPartIdx) == B8X8_IN_BIT) {
			max_bit = 5;
		}
		else {
			max_bit = 6;
		}
		PartSize eMode = pcCU->getPartitionSize(uiAbsPartIdx);
		UInt partindx = eMode == SIZE_2Nx2N ? 1 : eMode == SIZE_2NxN ? 2 : eMode == SIZE_Nx2N ? 3 : eMode == SIZE_2NxnU ? 4 : eMode == SIZE_2NxnD ? 5 : eMode == SIZE_nLx2N ? 6 : eMode == SIZE_nRx2N ? 7 : 0;
		if (pcCU->isSkip(uiAbsPartIdx) || pcCU->isDirect(uiAbsPartIdx))
			partindx = 0;
		else if (pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA)
		{
			if (eMode == SIZE_2Nx2N)
				partindx = 10;
			else if (eMode == SIZE_NxN)
				partindx = 9;
			else if (eMode == SIZE_2NxhN)
				partindx = 12;
			else if (eMode == SIZE_hNx2N)
				partindx = 13;
			else
				printf("error \n");
		}
		else
			partindx = partindx;


		if (pcCU->getLog2CUSize(uiAbsPartIdx) == B8X8_IN_BIT) {
			act_sym = pcCU->getPicture()->getPictureType() == B_PICTURE ? MapBCUTypeMin[partindx] : MapPCUTypeMin[partindx];
		}
		else {
			act_sym = pcCU->getPicture()->getPictureType() == B_PICTURE ? MapBCUType[partindx] : MapPCUType[partindx];
		}

		if (pcCU->isSkip(uiAbsPartIdx))
		{
			act_sym = 0;
		}
		real_bit = act_sym;
		act_ctx = 0;

		while (act_sym >= 1)
		{
			if ((binidx == 5) && (pcCU->getLog2CUSize(uiAbsPartIdx) != MIN_CU_SIZE_IN_BIT))
			{
				xEncodeBinF(0);
			}
			else
			{
				xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, act_ctx));
			}

			binidx++;

			act_sym--;
			act_ctx++;

			if (act_ctx >= 5) {
				act_ctx = 5;
			}
		}
		if (real_bit < max_bit) {
			if ((binidx == 5) && (pcCU->getLog2CUSize(uiAbsPartIdx) != MIN_CU_SIZE_IN_BIT)) {
				xEncodeBinF(1);
			}
			else {
				xEncodeBin(1, m_cCUPartSizeSCModel.get(0, 0, act_ctx));
			}
		}




		if (pcCU->getLog2CUSize(uiAbsPartIdx) >= B16X16_IN_BIT)
		{
			//if (pcCU->getPicture()->getSPS()->getAsymmetricMotionPartitionsEnableFlag())
			{

				if (pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2NxN || pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_Nx2N)
				{
					xEncodeBin(1, m_cCUPartSizeSCModel.get(0, 0, 8));
				}
				else if (pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2NxnU || pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2NxnD
					|| pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_nLx2N || pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_nRx2N)
				{
					//biari_encode_symbol(eep_dp, 0, pAMPCTX + 0);     //SMP - AMP signal bit
					//	biari_encode_symbol(eep_dp, !(currMB->cuType % 2), pAMPCTX + 1);    //AMP shape

					xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 8));
					xEncodeBin(!(partindx % 2), m_cCUPartSizeSCModel.get(0, 0, 9));

				}

			}
		}
	}

#else
	PartSize eSize = pcCU->getPartitionSize(uiAbsPartIdx);
	
#if inter_direct_skip_bug

	if (pcCU->isSkip(uiAbsPartIdx) || pcCU->isDirect(uiAbsPartIdx))
	{
	//	if (pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, pcCU->getTransformIdx(uiAbsPartIdx)) || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA, pcCU->getTransformIdx(uiAbsPartIdx)))
		if (pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0) || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0) || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0))
		{
			xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 0));
			xEncodeBin(1, m_cCUPartSizeSCModel.get(0, 0, 1));
			return;
		}
		else
		{
			xEncodeBin(1, m_cCUPartSizeSCModel.get(0, 0, 0));
			return;
		}
	}

#else

	if (pcCU->isSkip(uiAbsPartIdx))
	{
		//write(0);
		xEncodeBin(1, m_cCUPartSizeSCModel.get(0, 0, 0));
		return;
	}
	else if (pcCU->isDirect(uiAbsPartIdx))
	{
		//write(1);
		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 0));
		xEncodeBin(1, m_cCUPartSizeSCModel.get(0, 0, 1));
		return;
	}
#endif
	//if (pcCU->isIntra(uiAbsPartIdx))
	//{
	//	if (pcCU->getLog2CUSize(uiAbsPartIdx) > 3)
	//	{
	//		//write(5);
	//		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 0));
	//		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 1));
	//		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 2));
	//		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 3));
	//		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 4));
	//		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 5));
	//		pcCU->setPartitionSizeIndex(uiAbsPartIdx, 6);
	//	}
	//	else
	//	{
	//		//write(6);
	//		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 0));
	//		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 1));
	//		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 2));
	//		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 3));
	//		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 4));
	//		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 5));
	//		pcCU->setPartitionSizeIndex(uiAbsPartIdx, 5);
	//	}
	//	codePartSize(pcCU, uiAbsPartIdx);
	//	return;
	//}

	switch (eSize)
	{
	case SIZE_2Nx2N:
	{
		//write(2);
		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 0));
		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 1));
		xEncodeBin(1, m_cCUPartSizeSCModel.get(0, 0, 2));
		pcCU->setPartitionSizeIndex(uiAbsPartIdx, 2);
		break;
	}
	case SIZE_2NxN:
	case SIZE_2NxnU:
	case SIZE_2NxnD:
	{
		//write(3);
		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 0));
		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 1));
		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 2));
		xEncodeBin(1, m_cCUPartSizeSCModel.get(0, 0, 3));
		pcCU->setPartitionSizeIndex(uiAbsPartIdx, 3);
#if niu_cutype_revise
		//pcCU->getPicture()->getSPS()->getAsymmetricMotionPartitionsEnableFlag()   添加后debug和release结果不一致。
		if ((pcCU->getWidth(uiAbsPartIdx) >= 16))
		{
#endif
			if (eSize == SIZE_2NxN)
			{
				//write(0);
				xEncodeBin(1, m_cCUShapeOfPartitionSCModel.get(0, 0, 0));
			}
			else if (eSize == SIZE_2NxnU)
			{
				//write(1);
				xEncodeBin(0, m_cCUShapeOfPartitionSCModel.get(0, 0, 0));
				xEncodeBin(1, m_cCUShapeOfPartitionSCModel.get(0, 0, 1));
			}
			else
			{
				//write(2);
				xEncodeBin(0, m_cCUShapeOfPartitionSCModel.get(0, 0, 0));
				xEncodeBin(0, m_cCUShapeOfPartitionSCModel.get(0, 0, 1));
			}
#if niu_cutype_revise
		}
#endif
		break;
	}
	case SIZE_Nx2N:
	case SIZE_nLx2N:
	case SIZE_nRx2N:
	{
		//write(4);
		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 0));
		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 1));
		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 2));
		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 3));
		xEncodeBin(1, m_cCUPartSizeSCModel.get(0, 0, 4));
		pcCU->setPartitionSizeIndex(uiAbsPartIdx, 4);
#if niu_cutype_revise
		//pcCU->getPicture()->getSPS()->getAsymmetricMotionPartitionsEnableFlag()   添加后debug和release结果不一致。
		if ((pcCU->getHeight(uiAbsPartIdx) >= 16))
		{
#endif 
			if (eSize == SIZE_Nx2N)
			{
				//write(0);
				xEncodeBin(1, m_cCUShapeOfPartitionSCModel.get(0, 0, 0));
			}
			else if (eSize == SIZE_nLx2N)
			{
				//write(1);
				xEncodeBin(0, m_cCUShapeOfPartitionSCModel.get(0, 0, 0));
				xEncodeBin(1, m_cCUShapeOfPartitionSCModel.get(0, 0, 1));
			}
			else
			{
				//write(2);
				xEncodeBin(0, m_cCUShapeOfPartitionSCModel.get(0, 0, 0));
				xEncodeBin(0, m_cCUShapeOfPartitionSCModel.get(0, 0, 1));
			}
#if niu_cutype_revise
		}
#endif
		break;
	}
	case SIZE_NxN:
	{
		//write(5);
		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 0));
		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 1));
		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 2));
		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 3));
		xEncodeBin(0, m_cCUPartSizeSCModel.get(0, 0, 4));
		xEncodeBin(1, m_cCUPartSizeSCModel.get(0, 0, 5));
		pcCU->setPartitionSizeIndex(uiAbsPartIdx, 5);
		break;
	}
	default:
	{
		assert(0);
	}
	}
#endif
}

Void TEncSbac::codeB2Nx2NInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
	UInt uiInterDir = pcCU->getInterDir(uiAbsPartIdx);  ///< uiIntraDir 1 to 4

	switch (uiInterDir)
	{
	case 1:           ///< INTER_FORWARD
	{
		write(0);
		break;
	}
	case 2:           ///< INTER_BACKWARD
	{
		write(1);
		break;
	}
	case 3:           ///< INTER_SYM
	{
		write(2);
		break;
	}
	case 4:           ///< BiD
	{
		write(3);
		break;
	}
	default:
		break;
	}
	return;
}
//AVS3

//AVS3
Void TEncSbac::codeBNxNInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
	UInt uiInterDir = pcCU->getInterDir(uiAbsPartIdx);  ///< uiIntraDir 1 to 4

	PredMode eMode = pcCU->getPredictionMode(uiAbsPartIdx);

	switch (uiInterDir)
	{
	case INTER_FORWARD:           ///< INTER_FORWARD
	{
		write(1);
		break;
	}
	case INTER_BACKWARD:           ///< INTER_BACKWARD
	{
		write(2);
		break;
	}
	case INTER_SYM:           ///< INTER_SYM
	{
		if (eMode == MODE_DIRECT)
		{
			write(0);
		}
		else                       //?直接这样else是否会有问题
		{
			write(3);
		}
		break;
	}
	case INTER_BID:           ///< BiD
	{
		write(4);
		break;
	}
	default:
		break;
	}
	return;
}
//AVS3

//AVS3
Void TEncSbac::codeFNxNInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
	UInt uiInterDir = pcCU->getInterDir(uiAbsPartIdx);  ///< uiIntraDir 1 to 4

	switch (uiInterDir)
	{
	case INTER_FORWARD:           ///< INTER_FORWARD
	{
		write(0);
		pcCU->setFPuTypeIndex(uiAbsPartIdx, 0);
		break;
	}
	case INTER_DUAL:           ///< DUAL
	{
		write(1);
		pcCU->setFPuTypeIndex(uiAbsPartIdx, 1);
		break;
	}
	default:
		break;
	}
	return;
}
//AVS3

//AVS3
Void TEncSbac::codeF2Nx2NInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
	UInt uiInterDir = pcCU->getInterDir(uiAbsPartIdx);  ///< uiIntraDir 1 to 4 
	if (uiInterDir == INTER_FORWARD)
	{
		write(0);
		pcCU->setFPuTypeIndex(uiAbsPartIdx, 0);
	}
	else if (uiInterDir == INTER_DUAL)
	{
		write(1);
		pcCU->setFPuTypeIndex(uiAbsPartIdx, 1);
	}
	return;
}
//AVS3

//AVS3
Void TEncSbac::codeF2NxnNInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
	UInt uiInterDir0 = pcCU->getInterDir(uiAbsPartIdx);  ///< uiIntraDir 1 to 4
	UInt uiInterDir1 = 0;
	UInt uiPartOffset = (pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1)) >> 2;
	switch (pcCU->getPartitionSize(uiAbsPartIdx))
	{
	case SIZE_2NxN:
	{
		uiAbsPartIdx += uiPartOffset << 1;
		uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
		break;
	}
	case SIZE_Nx2N:
	{
		uiAbsPartIdx += uiPartOffset;
		uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
		break;
	}
	case SIZE_2NxnU:
	{
		uiAbsPartIdx += uiPartOffset >> 1;
		uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
		break;
	}
	case SIZE_2NxnD:
	{
		uiAbsPartIdx += (uiPartOffset << 1) + (uiPartOffset >> 1);
		uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
		break;
	}
	case SIZE_nLx2N:
	{
		uiAbsPartIdx += uiPartOffset >> 2;
		uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
		break;
	}
	case SIZE_nRx2N:
	{
		uiAbsPartIdx += uiPartOffset + (uiPartOffset >> 2);
		uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
		break;
	}
	default:
		break;
	}
	if (pcCU->getLog2CUSize(uiAbsPartIdx) == 3)
	{
		return;
	}
	else if ((uiInterDir0 == INTER_FORWARD) && (uiInterDir0 == INTER_FORWARD))
	{
		write(0);
		pcCU->setFPuTypeIndex(uiAbsPartIdx, 0);
	}
	else if ((uiInterDir0 == INTER_FORWARD) && (uiInterDir0 == INTER_DUAL))
	{
		write(1);
		pcCU->setFPuTypeIndex(uiAbsPartIdx, 1);
	}
	else if ((uiInterDir0 == INTER_DUAL) && (uiInterDir0 == INTER_FORWARD))
	{
		write(2);
		pcCU->setFPuTypeIndex(uiAbsPartIdx, 2);
	}
	else if ((uiInterDir0 == INTER_DUAL) && (uiInterDir0 == INTER_DUAL))
	{
		write(3);
		pcCU->setFPuTypeIndex(uiAbsPartIdx, 3);
	}
	return;
}
//AVS3

//AVS3
Void TEncSbac::codeB2NxnNInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
	UInt uiInterDir0 = pcCU->getInterDir(uiAbsPartIdx);  ///< uiIntraDir 1 to 4
	UInt uiInterDir1 = 0;
	//UInt uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
	UInt uiPartOffset = (pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1)) >> 2;
	switch (pcCU->getPartitionSize(uiAbsPartIdx))
	{
	case SIZE_2NxN:
	{
		uiAbsPartIdx += uiPartOffset << 1;
		uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
		break;
	}
	case SIZE_Nx2N:
	{
		uiAbsPartIdx += uiPartOffset;
		uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
		break;
	}
	case SIZE_2NxnU:
	{
		uiAbsPartIdx += uiPartOffset >> 1;
		uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
		break;
	}
	case SIZE_2NxnD:
	{
		uiAbsPartIdx += (uiPartOffset << 1) + (uiPartOffset >> 1);
		uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
		break;
	}
	case SIZE_nLx2N:
	{
		uiAbsPartIdx += uiPartOffset >> 2;
		uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
		break;
	}
	case SIZE_nRx2N:
	{
		uiAbsPartIdx += uiPartOffset + (uiPartOffset >> 2);
		uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
		break;
	}
	default:
		break;
	}
	if (pcCU->getLog2CUSize(uiAbsPartIdx) == 3)
	{
		if (uiInterDir0 == INTER_FORWARD || uiInterDir1 == INTER_FORWARD)
		{
			write(0);
		}
		else if (uiInterDir0 == INTER_FORWARD || uiInterDir1 == INTER_BACKWARD)
		{
			write(1);
		}
		else if (uiInterDir0 == INTER_BACKWARD || uiInterDir1 == INTER_BACKWARD)
		{
			write(2);
		}
		else if (uiInterDir0 == INTER_BACKWARD || uiInterDir1 == INTER_FORWARD)
		{
			write(3);
		}
	}
	else if (pcCU->getLog2CUSize(uiAbsPartIdx) > 3)
	{
		if (uiInterDir0 == INTER_BID || uiInterDir1 == INTER_BID)
		{
			write(0);
		}
		else if (uiInterDir0 == INTER_BID || uiInterDir1 == INTER_BACKWARD)
		{
			write(1);
		}
		else if (uiInterDir0 == INTER_BID || uiInterDir1 == INTER_FORWARD)
		{
			write(2);
		}
		else if (uiInterDir0 == INTER_BID || uiInterDir1 == INTER_SYM)
		{
			write(3);
		}
		else if (uiInterDir0 == INTER_BACKWARD || uiInterDir1 == INTER_BACKWARD)
		{
			write(4);
		}
		else if (uiInterDir0 == INTER_BACKWARD || uiInterDir1 == INTER_BID)
		{
			write(5);
		}
		else if (uiInterDir0 == INTER_BACKWARD || uiInterDir1 == INTER_FORWARD)
		{
			write(6);
		}
		else if (uiInterDir0 == INTER_BACKWARD || uiInterDir1 == INTER_SYM)
		{
			write(7);
		}
		else if (uiInterDir0 == INTER_FORWARD || uiInterDir1 == INTER_FORWARD)
		{
			write(8);
		}
		else if (uiInterDir0 == INTER_FORWARD || uiInterDir1 == INTER_BID)
		{
			write(9);
		}
		else if (uiInterDir0 == INTER_FORWARD || uiInterDir1 == INTER_BACKWARD)
		{
			write(10);
		}
		else if (uiInterDir0 == INTER_FORWARD || uiInterDir1 == INTER_SYM)
		{
			write(11);
		}
		else if (uiInterDir0 == INTER_SYM || uiInterDir1 == INTER_SYM)
		{
			write(12);
		}
		else if (uiInterDir0 == INTER_SYM || uiInterDir1 == INTER_BID)
		{
			write(13);
		}
		else if (uiInterDir0 == INTER_SYM || uiInterDir1 == INTER_BACKWARD)
		{
			write(14);
		}
		else if (uiInterDir0 == INTER_SYM || uiInterDir1 == INTER_FORWARD)
		{
			write(15);
		}
	}
	return;
}
//AVS3

//AVS3
#if DMH
Void TEncSbac::writeDMHMode(TComDataCU *pcCU, UInt uiAbsPartIdx)
{
	const Int iEncMapTab[9] = { 0, 5, 6, 1, 2, 7, 8, 3, 4 };
	const Int dmhMode = pcCU->getDMHMode(uiAbsPartIdx);
	const Int iMapVal = iEncMapTab[dmhMode];
	const UInt index = (pcCU->getLog2CUSize(uiAbsPartIdx) - 3) * 4;
	Int iSymbol = (iMapVal == 0) ? 0 : 1;
	//xEncodeBin(0, m_cCUDmhSCModel.get(0, 0, 0));
	//return;
	xEncodeBin(iSymbol, m_cCUDmhSCModel.get(0, 0, index));

	if (iSymbol)
	{
		if (iMapVal < 3)
		{
			iSymbol = (iMapVal == 1) ? 0 : 1;
			xEncodeBin(0, m_cCUDmhSCModel.get(0, 0, index + 1));
			xEncodeBinEP(iSymbol);
		}
		else if (iMapVal < 5)
		{
			iSymbol = (iMapVal == 3) ? 0 : 1;
			xEncodeBin(1, m_cCUDmhSCModel.get(0, 0, index + 1));
			xEncodeBin(0, m_cCUDmhSCModel.get(0, 0, index + 2));
			xEncodeBinEP(iSymbol);
		}
		else
		{
			xEncodeBin(1, m_cCUDmhSCModel.get(0, 0, index + 1));
			xEncodeBin(1, m_cCUDmhSCModel.get(0, 0, index + 2));
			iSymbol = (iMapVal < 7) ? 0 : 1;
			xEncodeBinEP(iSymbol);
			iSymbol = (iMapVal % 2) ? 0 : 1;
			xEncodeBinEP(iSymbol);
		}
	}
	return;
}
#endif

Void TEncSbac::codeDMHMode(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
#if DMH

	if (pcCU->getLog2CUSize(uiAbsPartIdx) == 3 && pcCU->getPartitionSize(uiAbsPartIdx) != 0
		&& pcCU->getPicture()->isInterF())
	{
		return;
	}
#endif
	UInt uiInterDir0 = pcCU->getInterDir(uiAbsPartIdx);  ///< uiIntraDir 1 to 4
	if (pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N)
	{
		if (uiInterDir0 == INTER_FORWARD)
		{
#if DMH
			writeDMHMode(pcCU, uiAbsPartIdx);
#else
			write(pcCU->getDMHMode(uiAbsPartIdx));
#endif
		}
		else
		{
			return;
		}
	}
	else if ((pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxN && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_Nx2N) ||
		(pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxnU && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N))
	{
#if DMH
		UInt tempPartIdx = 0;
#endif
		UInt uiInterDir1 = 0;
		UInt uiPartOffset = (pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1)) >> 2;
		switch (pcCU->getPartitionSize(uiAbsPartIdx))
		{
		case SIZE_2NxN:
		{
#if DMH
			tempPartIdx = uiAbsPartIdx + (uiPartOffset << 1);
			uiInterDir1 = pcCU->getInterDir(tempPartIdx);
#else
			uiAbsPartIdx += uiPartOffset << 1;
			uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
#endif
			break;
		}
		case SIZE_Nx2N:
		{
#if DMH
			tempPartIdx = uiAbsPartIdx + uiPartOffset;
			uiInterDir1 = pcCU->getInterDir(tempPartIdx);
#else
			uiAbsPartIdx += uiPartOffset;
			uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
#endif
			break;
		}
		case SIZE_2NxnU:
		{
#if DMH
			tempPartIdx = uiAbsPartIdx + (uiPartOffset >> 1);
			uiInterDir1 = pcCU->getInterDir(tempPartIdx);
#else
			uiAbsPartIdx += uiPartOffset >> 1;
			uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
#endif
			break;
		}
		case SIZE_2NxnD:
		{
#if DMH
			tempPartIdx = uiAbsPartIdx + (uiPartOffset << 1) + (uiPartOffset >> 1);
			uiInterDir1 = pcCU->getInterDir(tempPartIdx);
#else
			uiAbsPartIdx += (uiPartOffset << 1) + (uiPartOffset >> 1);
			uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
#endif
			break;
		}
		case SIZE_nLx2N:
		{
#if DMH
			tempPartIdx = uiAbsPartIdx + (uiPartOffset >> 2);
			uiInterDir1 = pcCU->getInterDir(tempPartIdx);
#else
			uiAbsPartIdx += uiPartOffset >> 2;
			uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
#endif
			break;
		}
		case SIZE_nRx2N:
		{
#if DMH
			tempPartIdx = uiAbsPartIdx + uiPartOffset + (uiPartOffset >> 2);
			uiInterDir1 = pcCU->getInterDir(tempPartIdx);
#else
			uiAbsPartIdx += uiPartOffset + (uiPartOffset >> 2);
			uiInterDir1 = pcCU->getInterDir(uiAbsPartIdx);
#endif
			break;
		}
		default:
			break;
		}
		if ((uiInterDir0 == INTER_FORWARD) && (uiInterDir1 == INTER_FORWARD))
		{
#if DMH
			writeDMHMode(pcCU, uiAbsPartIdx);
#else
			write(pcCU->getDMHMode(uiAbsPartIdx));
#endif
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
#if DMH
			writeDMHMode(pcCU, uiAbsPartIdx);
#else
			write(pcCU->getDMHMode(uiAbsPartIdx));
#endif
		}
		else
		{
			return;
		}
	}
	return;
}
//AVS3
#endif

Void TEncSbac::codePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  PartSize eSize = pcCU->getPartitionSize( uiAbsPartIdx );

  if ( pcCU->isIntra( uiAbsPartIdx ) )  //zhangyiCheck: IntraCuFlag == 1?
  {
#if YQH_INTRA
	  codeTransSplitFlag( pcCU, uiAbsPartIdx );//for intra currenct
	  codeIntraPuTypeIdx( pcCU, uiAbsPartIdx );//for intra currenct
#else
#if ZHANGYI_INTRA
	  PictureType cuPicType = pcCU->getPicture()->getPictureType();
	  if ( (pcCU->isSkip( uiAbsPartIdx)) && ( (cuPicType == P_PICTURE) || (cuPicType == B_PICTURE) || (cuPicType == F_PICTURE)))
	  {
		  return;
	  }
	  UInt SizeInBit = pcCU->getLog2CUSize(uiAbsPartIdx);
	  if ( ( SizeInBit == 3) || (((pcCU->getPicture()->getSPS()->getNonsquareIntraPredictionEnableFlag() == 1) && ((SizeInBit > 3) && (SizeInBit < 6)))))  //zhangyiCheck:要保证在这个条件不满足的情况下，cu的划分都是2NX2N
#endif
    xEncodeBin( eSize == SIZE_2Nx2N? 1 : 0, m_cCUPartSizeSCModel.get( 0, 0, 0 ) );
#endif  
	  return;
  }

  switch(eSize)
  {
    case SIZE_2Nx2N:
    {
      xEncodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 0) );
      break;
    }
	case SIZE_2NxN:
	case SIZE_2NxnU:
	case SIZE_2NxnD:
		{
			xEncodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
			xEncodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 1) );

      if( pcCU->getLog2CUSize( uiAbsPartIdx ) > 3 )
			{
			  if (eSize == SIZE_2NxN)
		  	{
          xEncodeBin(1, m_cCUYPosiSCModel.get( 0, 0, 0 ));
		  	}
			  else
		  	{
          xEncodeBin(0, m_cCUYPosiSCModel.get( 0, 0, 0 ));
          xEncodeBin((eSize == SIZE_2NxnU? 0: 1), m_cCUYPosiSCModel.get( 0, 0, 1 ));
		  	}
			}
			break;
		}
	case SIZE_Nx2N:
	case SIZE_nLx2N:
	case SIZE_nRx2N:
		{
			xEncodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
			xEncodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
			xEncodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 2) );

			if (  pcCU->getLog2CUSize( uiAbsPartIdx ) > 3  )
			{
			  if (eSize == SIZE_Nx2N)
		  	{
          xEncodeBin(1, m_cCUXPosiSCModel.get( 0, 0, 0 ));
		  	}
			  else
		  	{
          xEncodeBin(0, m_cCUXPosiSCModel.get( 0, 0, 0 ));
          xEncodeBin((eSize == SIZE_nLx2N? 0: 1), m_cCUXPosiSCModel.get( 0, 0, 1 ));
		  	}
			}
			break;
		}
	case SIZE_NxN:
		{
			xEncodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
			xEncodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
			xEncodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 2) );

      if (pcCU->getPicture()->isInterB())
      {
  			xEncodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 3) );
			}
			break;
		}
    default:
    {
      assert(0);
    }
  }
}

#if 0//YQH_SPLIGFLAG_DEC_BUG2

#if WLQ_CUSplitFlag
Void TEncSbac::codeSplitFlag(TComDataCU* pcCU, UInt value, Bool key, UInt value2)
{
	if (key)
		xEncodeBin(value, m_cCUSplitFlagSCModel.get(0, 0, value2));
	else
		xEncodeBin(value, m_cCUSplitFlagSCModel.get(0, 0, pcCU->getLog2CUSize(0)));//yuquanhe@hisilicon.com
}
#endif
Void TEncSbac::codeSplitFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
#if niu_SplitFlag

#else
	if( uiDepth == g_uiMaxCUDepth - 1 )
		return;
#endif
	UInt uiCurrSplitFlag = ( pcCU->getDepth( uiAbsPartIdx ) > uiDepth ) ? 1 : 0;
#if YQH_SPLIGFLAG_BUG
	UInt SizeInBit = pcCU->getLog2CUSize(uiAbsPartIdx);

#if !AVS3_close_CUSplitFlag
	xEncodeBin(uiCurrSplitFlag, m_cCUSplitFlagSCModel.get(0, 0, SizeInBit));//yuquanhe@hisilicon.com
#endif
#endif
	return;
}


#else

#if WLQ_CUSplitFlag
Void TEncSbac::codeSplitFlag(TComDataCU* pcCU, UInt value, Bool key, UInt value2)
{
	if (key)
		xEncodeBin(value, m_cCUSplitFlagSCModel.get(0, 0, value2));
	else
		xEncodeBin(value, m_cCUSplitFlagSCModel.get(0, 0, pcCU->getLog2CUSize(0)));//yuquanhe@hisilicon.com
}
#else
Void TEncSbac::codeSplitFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( uiDepth == g_uiMaxCUDepth - 1 )
    return;

  UInt uiCurrSplitFlag = ( pcCU->getDepth( uiAbsPartIdx ) > uiDepth ) ? 1 : 0;
  
#if YQH_SPLIGFLAG_BUG
  UInt SizeInBit = pcCU->getLog2CUSize(uiAbsPartIdx);
  xEncodeBin( uiCurrSplitFlag, m_cCUSplitFlagSCModel.get( 0, 0, SizeInBit ) );//yuquanhe@hisilicon.com
#else
  #if YQH_INTRA
#if  !AVS3_close_CUSplitFlag
  xEncodeBin( uiCurrSplitFlag, m_cCUSplitFlagSCModel.get( 0, 0, uiDepth ) );//yuquanhe@hisilicon.com
#endif
#else
  xEncodeBin( uiCurrSplitFlag, m_cCUSplitFlagSCModel.get( 0, 0, 0 ) );
#endif
#endif
  return;
}
#endif

#endif
Void TEncSbac::codeTransformIdx( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiSymbol = pcCU->getTransformIdx(uiAbsPartIdx);

  uiSymbol > 0 ? uiSymbol = 1 : uiSymbol = 0;
  xEncodeBin( uiSymbol , m_cCUTransIdxSCModel.get( 0, 0, 0 ) );

  return;
}
#if	F_DHP_SYC
Void TEncSbac::codeInterDHP(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
	if (pcCU->getLog2CUSize(uiAbsPartIdx) == B8X8_IN_BIT && pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxN && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N)
	{
		assert(pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD);
		assert(pcCU->getInterDir(uiAbsPartIdx + (pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1)) - 1) == INTER_FORWARD);
		return;
	}
	UInt pdir0 = pcCU->getInterDir(uiAbsPartIdx);//currMB->b8pdir[0];
	UInt pdir1 = pdir0;//currMB->b8pdir[3];
	UInt uiPartOffset = (pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1)) >> 2;
	switch (pcCU->getPartitionSize(uiAbsPartIdx))
	{
	case SIZE_2Nx2N:
	{
		pdir1 = pdir0;
		break;
	}
	case SIZE_2NxN:
	{
		pdir1 = pcCU->getInterDir(uiAbsPartIdx + (uiPartOffset << 1));
		break;
	}
	case SIZE_Nx2N:
	{
		pdir1 = pcCU->getInterDir(uiAbsPartIdx + uiPartOffset);
		break;
	}
	case SIZE_2NxnU:
	{
		pdir1 = pcCU->getInterDir(uiAbsPartIdx + (uiPartOffset >> 1));
		break;
	}
	case SIZE_2NxnD:
	{
		pdir1 = pcCU->getInterDir(uiAbsPartIdx + (uiPartOffset << 1) + (uiPartOffset >> 1));
		break;
	}
	case SIZE_nLx2N:
	{
		pdir1 = pcCU->getInterDir(uiAbsPartIdx + (uiPartOffset >> 2));
		break;
	}
	case SIZE_nRx2N:
	{
		pdir1 = pcCU->getInterDir(uiAbsPartIdx + uiPartOffset + (uiPartOffset >> 2));
		break;
	}
	case SIZE_NxN:
	{
		//no NxN
		assert(0);
		break;
	}
	default:
		assert(0);
		break;
	}

	UInt act_ctx = 0;
	UInt symbol;

	//BiContextTypePtr pCTX = (img->currentSlice)->syn_ctx->pdir_dhp_contexts;

	pdir0 = (pdir0 == 0) ? 0 : 1;
	pdir1 = (pdir1 == 0) ? 0 : 1;


	if (pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N) {
		//biari_encode_symbol(eep_dp, pdir0, pCTX + act_ctx);
		xEncodeBin(pdir0, m_cCUInterDHPSCModel.get(0, 0, act_ctx));
	}
	else if (pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxN || pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N) {   //1010
		//biari_encode_symbol(eep_dp, pdir0, pCTX + act_ctx + 1);
		xEncodeBin(pdir0, m_cCUInterDHPSCModel.get(0, 0, act_ctx + 1));
		symbol = (pdir0 == pdir1);
		//biari_encode_symbol(eep_dp, symbol, pCTX + act_ctx + 2);
		xEncodeBin(symbol, m_cCUInterDHPSCModel.get(0, 0, act_ctx + 2));
	}

	return;
}
Void TEncSbac::codeInterDHP_FNXN(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
	UInt act_ctx;
	UInt act_sym;

	//BiContextTypePtr pCTX = (img->currentSlice)->syn_ctx->b8_type_dhp_contexts;

	act_sym = pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL ? 0 : 1;
	act_ctx = 0;

	//biari_encode_symbol(eep_dp, act_sym, pCTX + act_ctx);
	xEncodeBin(0, m_cCUInterDHPNXNSCModel.get(0, 0, act_ctx));

	return;
}
#endif


#if	inter_direct_skip_bug1
Void TEncSbac::codeInterWSM(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
	UInt uiInterWSMmode = pcCU->getInterSkipmode(uiAbsPartIdx);
#if F_MHPSKIP_SYC
	if (uiInterWSMmode > 3)
	{
		uiInterWSMmode = 0;
	}
#endif
	UInt  binIdx = 0;
	if (uiInterWSMmode == pcCU->getPicture()->getPicHeader()->getInterSkipmodeNumber() - 1)
	{
		for (UInt i = 0; i < uiInterWSMmode; i++)
		{
			xEncodeBin(0, m_cCUInterWSMSCModel.get(0, 0, binIdx));
			binIdx = binIdx > 2 ? 2 : binIdx + 1;
		}
	}
	else
	{
		for (UInt i = 0; i < uiInterWSMmode; i++)
		{
			xEncodeBin(0, m_cCUInterWSMSCModel.get(0, 0, binIdx));
			binIdx = binIdx > 2 ? 2 : binIdx + 1;
		}
		xEncodeBin(1, m_cCUInterWSMSCModel.get(0, 0, binIdx));
	}

  return;
}
#endif
#if	F_MHPSKIP_SYC
Void TEncSbac::codeInterMHPSKIP(TComDataCU* pcCU, UInt uiAbsPartIdx)
{

	//BiContextTypePtr pCTX = (img->currentSlice)->syn_ctx->p_skip_mode_contexts;
	UInt symbol = 0;
	if (pcCU->getInterSkipmode(uiAbsPartIdx) == 0)
	{
		symbol = 0;
	}
	else
	{
		assert(pcCU->getInterSkipmode(uiAbsPartIdx) > 3);
		symbol = pcCU->getInterSkipmode(uiAbsPartIdx) - 3;
	}

	UInt offset = 0;




	for (offset = 0; offset < symbol; offset++) {
		//biari_encode_symbol(eep_dp, 0, pCTX + offset);
		xEncodeBin(0, m_cCUInterMHPSKIPSCModel.get(0, 0, offset));
	}
	if (symbol < MH_PSKIP_NUM) {
		//biari_encode_symbol(eep_dp, 1, pCTX + offset);
		xEncodeBin(1, m_cCUInterMHPSKIPSCModel.get(0, 0, offset));
	}



	return;
}
#endif

#if WRITE_INTERDIR
Void TEncSbac::codeInterDirRD(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
	UInt uiPartOffset = (pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1)) >> 2;
	Int pDir0 = pcCU->getInterDir(uiAbsPartIdx);
	Int pDir1 = 0;

	switch (pcCU->getPartitionSize(uiAbsPartIdx))
	{
	case SIZE_2Nx2N:
	{
		pDir1 = pDir0;
		break;
	}
	case SIZE_2NxN:
	{
		uiAbsPartIdx += (uiPartOffset << 1);
		pDir1 = pcCU->getInterDir(uiAbsPartIdx);
		break;
	}
	case SIZE_Nx2N:
	{
		uiAbsPartIdx += uiPartOffset;
		pDir1 = pcCU->getInterDir(uiAbsPartIdx);
		break;
	}
	case SIZE_2NxnU:
	{
		uiAbsPartIdx += (uiPartOffset >> 1);
		pDir1 = pcCU->getInterDir(uiAbsPartIdx);
		break;
	}
	case SIZE_2NxnD:
	{
		uiAbsPartIdx += (uiPartOffset << 1) + (uiPartOffset >> 1);
		pDir1 = pcCU->getInterDir(uiAbsPartIdx);
		break;
	}
	case SIZE_nLx2N:
	{
		uiAbsPartIdx += (uiPartOffset >> 2);
		pDir1 = pcCU->getInterDir(uiAbsPartIdx);
		break;
	}
	case SIZE_nRx2N:
	{
		uiAbsPartIdx += uiPartOffset + (uiPartOffset >> 2);
		pDir1 = pcCU->getInterDir(uiAbsPartIdx);
		break;
	}

	default:
		break;
	}

	Int act_ctx = 0;
	Int act_sym;
	Int symbol;
	Int newPdir[4] = { 2, 1, 3, 0 };

	if (pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N)
	{
		act_sym = pDir0;
		while (act_sym >= 1)
		{
			xEncodeBin(0, m_cCUInterDirSCModel.get(0, 0, act_ctx));
			act_sym--;
			act_ctx++;
		}
		if (pDir0 != 3)
		{
			xEncodeBin(1, m_cCUInterDirSCModel.get(0, 0, act_ctx));
		}
	}
	else if (pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxN && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N && pcCU->getLog2CUSize(uiAbsPartIdx) == B8X8_IN_BIT)
	{

		pDir0 = pDir0 - 1;
		pDir1 = pDir1 - 1;

		pDir0 = newPdir[pDir0];
		pDir1 = newPdir[pDir1];

		act_sym = pDir0;

		if (act_sym == 1)
		{ // BW
			xEncodeBin(0, m_cCUMinInterDirSCModel.get(0, 0, act_ctx));
		}
		else
		{ // FW
			xEncodeBin(1, m_cCUMinInterDirSCModel.get(0, 0, act_ctx));
		}

		act_ctx = 1;

		symbol = (pDir0 == pDir1);
		xEncodeBin(symbol, m_cCUMinInterDirSCModel.get(0, 0, act_ctx));
	}
	else if (pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxN || pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N)
	{
		pDir0 = newPdir[pDir0];
		pDir1 = newPdir[pDir1];
		act_sym = pDir0;
		while (act_sym >= 1)
		{
			xEncodeBin(0, m_cCUInterDirSCModel.get(0, 0, act_ctx + 4));
			act_sym--;
			act_ctx++;
		}
		if (pDir0 != 3)
		{
			xEncodeBin(1, m_cCUInterDirSCModel.get(0, 0, act_ctx + 4));
		}

		act_ctx = 8;
		symbol = (pDir0 == pDir1);
		xEncodeBin(symbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));

		if (!symbol)
		{
			switch (pDir0)
			{
			case 0:
				act_ctx = 9;
				symbol = (pDir1 == 1);
				xEncodeBin(symbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
				if (!symbol)
				{
					act_ctx = 10;
					symbol = (pDir1 == 2);
					xEncodeBin(symbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
				}
				break;


			case 1:
				act_ctx = 11;
				symbol = (pDir1 == 0);
				xEncodeBin(symbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
				if (!symbol)
				{
					act_ctx = 12;
					symbol = (pDir1 == 2);
					xEncodeBin(symbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
				}
				break;

			case 2:
				act_ctx = 13;
				symbol = (pDir1 == 0);
				xEncodeBin(symbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
				if (!symbol)
				{
					act_ctx = 14;
					symbol = (pDir1 == 1);
					xEncodeBin(symbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
				}
				break;

			case 3:
				act_ctx = 15;
				symbol = (pDir1 == 0);
				xEncodeBin(symbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
				if (!symbol)
				{
					act_ctx = 16;
					symbol = (pDir1 == 1);
					xEncodeBin(symbol, m_cCUInterDirSCModel.get(0, 0, act_ctx));
				}
				break;
			}
		}

	}
}
#endif

Void TEncSbac::codeInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
	UInt uiInterDir = pcCU->getInterDir(uiAbsPartIdx);

	uiInterDir--;
	xEncodeBin((uiInterDir == 2 ? 1 : 0), m_cCUInterDirSCModel.get(0, 0, 0));
#if RPS
#if B_RPS_BUG_818
	if (pcCU->getPicture()->getPictureType() == B_PICTURE && pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_0, 0) == pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_1, 0))
#else
	if (pcCU->getPicture()->getPictureType() == B_PICTURE && pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_0, pcCU->getCUMvField(REF_PIC_0)->getRefIdx(uiAbsPartIdx)) == pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_1, pcCU->getCUMvField(REF_PIC_1)->getRefIdx(uiAbsPartIdx)))
#endif
#else
	if (pcCU->getPicture()->getPictureType() == B_PICTURE && pcCU->getPicture()->getRefPOC(REF_PIC_0) == pcCU->getPicture()->getRefPOC(REF_PIC_1))
#endif
	{
		assert(uiInterDir != 1);
		return;
	}
	if (uiInterDir < 2)
	{
		xEncodeBin(uiInterDir, m_cCUInterDirSCModel.get(0, 0, 1));
	}
	return;
}


#if ZHANGYI_INTRA
Void TEncSbac::codeIntraDirCb( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
	
	//UInt uiIntraDir = pcCU->getIntraDirCb( uiAbsPartIdx );
	UInt LumaMode = pcCU->getIntraDir(uiAbsPartIdx);
	UInt uiIntraDirCb = pcCU->getIntraDirCb( uiAbsPartIdx );
	Bool is_redundant = false;
	int lmode = 0;
	int l;
	if ( LumaMode == INTRA_VER_IDX || LumaMode == INTRA_HOR_IDX || LumaMode == INTRA_DC_IDX || LumaMode == INTRA_BI_IDX )
	{
		lmode = LumaMode == INTRA_VER_IDX ? INTRA_VERT_PRED_CHROMA : (LumaMode == INTRA_HOR_IDX ? INTRA_HOR_PRED_CHROMA : (LumaMode == INTRA_DC_IDX ? INTRA_DC_PRED_CHROMA : INTRA_BI_PRED_CHROMA));
		is_redundant = true;
	}
#if YQH_INTRA
	//yuquanhe@hisilicon.com
#if niu_getLeftCU
#if niu_LeftPU_revise
	UInt PULeftPartIdx = MAX_UINT;
	TComDataCU* pcCUAboveLeft = pcCU->getPULeft(PULeftPartIdx, pcCU->getZorderIdxInCU() + uiAbsPartIdx);
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
#if niu_LeftPU_revise
		act_ctx = pcCUAboveLeft->getIntraDirCb(PULeftPartIdx) != 0 ? 1 : 0;
#else
		act_ctx= pcCUAboveLeft->getIntraDirCb( uiAbsPartIdx )!=0 ? 1:0;
#endif

#endif
#if	WLQ_intra_Chroma_ctx_BUG
	act_ctx = 0;
#endif
	if (uiIntraDirCb == 0)
	{
		xEncodeBin(1, m_cCUIntraDirSCModel.get(0,0,act_ctx+7));
	}
	else
	{
		xEncodeBin(0, m_cCUIntraDirSCModel.get(0,0,act_ctx+7));
		if (is_redundant) //luma predmode is 0 2 12 or 24, the chroma is 1 4 3 2
		{
			if (uiIntraDirCb > lmode) //the minimum of uiIntraDirCb is 2
			{
				uiIntraDirCb -= 2; //其实是按照原来的模式号减去1来编，因为上面已经先编了一个0
			} else {
				uiIntraDirCb -= 1; //其实是按照原来的模式号编，因为上面已经先编了一个0
			}
		} else {
			uiIntraDirCb -= 1; //其实是按照原来的模式号编，因为上面已经先编了一个0
		}
		if (uiIntraDirCb == 0)
		{
			xEncodeBin(1, m_cCUIntraDirSCModel.get(0,0,7+2));
		} else {
			xEncodeBin(0, m_cCUIntraDirSCModel.get(0,0,7+2));
			l = uiIntraDirCb;
			while ((--l) > 0)
			{
				xEncodeBin(0, m_cCUIntraDirSCModel.get(0,0,7+2));
			}
			if (uiIntraDirCb < 3) //maxValue - 1
			{
				xEncodeBin(1, m_cCUIntraDirSCModel.get(0,0,7+2));
			}
		}
	}
	return;
}
#if ZHANGYI_INTRA
Void TEncSbac::codeIntraDirCbForRDO(TComDataCU* pcCU, UInt uiAbsPartIdx, Int iIntraDirCbValue )
{
	UInt LumaMode = pcCU->getIntraDir(uiAbsPartIdx);
	Bool is_redundant = false;
	int lmode = 0;
	int l;
	if ( LumaMode == INTRA_VER_IDX || LumaMode == INTRA_HOR_IDX || LumaMode == INTRA_DC_IDX || LumaMode == INTRA_BI_IDX )
	{
		lmode = LumaMode == INTRA_VER_IDX ? INTRA_VERT_PRED_CHROMA : (LumaMode == INTRA_HOR_IDX ? INTRA_HOR_PRED_CHROMA : (LumaMode == INTRA_DC_IDX ? INTRA_DC_PRED_CHROMA : INTRA_BI_PRED_CHROMA));
		is_redundant = true;
	}

#if YQH_INTRA
	//yuquanhe@hisilicon.com
#if niu_getLeftCU
#if niu_LeftPU_revise
	UInt PULeftPartIdx = MAX_UINT;
	TComDataCU* pcCUAboveLeft = pcCU->getPULeft(PULeftPartIdx, pcCU->getZorderIdxInCU() + uiAbsPartIdx);
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
#if niu_LeftPU_revise
		act_ctx = pcCUAboveLeft->getIntraDirCb(PULeftPartIdx) != 0 ? 1 : 0;
#else
		act_ctx = pcCUAboveLeft->getIntraDirCb(uiAbsPartIdx) != 0 ? 1 : 0;
#endif
#endif
#if	WLQ_intra_Chroma_ctx_BUG
	act_ctx = 0;
#endif
	if (iIntraDirCbValue == 0)
	{
		xEncodeBin(1, m_cCUIntraDirSCModel.get(0,0,act_ctx+7));
	}
	else
	{
		xEncodeBin(0, m_cCUIntraDirSCModel.get(0,0,act_ctx+7));
		if (is_redundant) //luma predmode is 0 2 12 or 24, the chroma is 1 4 3 2
		{
			if (iIntraDirCbValue > lmode) //the minimum of uiIntraDirCb is 2
			{
				iIntraDirCbValue -= 2; //其实是按照原来的模式号减去1来编，因为上面已经先编了一个0
			} else {
				iIntraDirCbValue -= 1; //其实是按照原来的模式号编，因为上面已经先编了一个0
			}
		} else {
			iIntraDirCbValue -= 1; //其实是按照原来的模式号编，因为上面已经先编了一个0
		}
		if (iIntraDirCbValue == 0)
		{
			xEncodeBin(1, m_cCUIntraDirSCModel.get(0,0,7+2));
		} else {
			xEncodeBin(0, m_cCUIntraDirSCModel.get(0,0,7+2));
			l = iIntraDirCbValue;
			while ((--l) > 0)
			{
				xEncodeBin(0, m_cCUIntraDirSCModel.get(0,0,7+2));
			}
			if (iIntraDirCbValue < 3) //maxValue - 1
			{
				xEncodeBin(1, m_cCUIntraDirSCModel.get(0,0,7+2));
			}
		}
	}
	return;
}
Void TEncSbac::codeIntraDirCrForRDO(TComDataCU* pcCU, UInt uiAbsPartIdx, Int iIntraDirCbValue )
{
	UInt LumaMode = pcCU->getIntraDir(uiAbsPartIdx);
	Bool is_redundant = false;
	int lmode = 0;
	int l;
	if ( LumaMode == INTRA_VER_IDX || LumaMode == INTRA_HOR_IDX || LumaMode == INTRA_DC_IDX || LumaMode == INTRA_BI_IDX )
	{
		lmode = LumaMode == INTRA_VER_IDX ? INTRA_VERT_PRED_CHROMA : (LumaMode == INTRA_HOR_IDX ? INTRA_HOR_PRED_CHROMA : (LumaMode == INTRA_DC_IDX ? INTRA_DC_PRED_CHROMA : INTRA_BI_PRED_CHROMA));
		is_redundant = true;
	}

	if (iIntraDirCbValue == 0)
	{
		xEncodeBin(1, m_cCUIntraDirSCModel.get(0,0,0));
	}
	else
	{
		xEncodeBin(0, m_cCUIntraDirSCModel.get(0,0,0));
		if (is_redundant) //luma predmode is 0 2 12 or 24, the chroma is 1 4 3 2
		{
			if (iIntraDirCbValue > lmode) //the minimum of uiIntraDirCb is 2
			{
				iIntraDirCbValue -= 2; //其实是按照原来的模式号减去1来编，因为上面已经先编了一个0
			} else {
				iIntraDirCbValue -= 1; //其实是按照原来的模式号编，因为上面已经先编了一个0
			}
		} else {
			iIntraDirCbValue -= 1; //其实是按照原来的模式号编，因为上面已经先编了一个0
		}
		if (iIntraDirCbValue == 0)
		{
			xEncodeBin(1, m_cCUIntraDirSCModel.get(0,0,0));
		} else {
			xEncodeBin(0, m_cCUIntraDirSCModel.get(0,0,0));
			l = iIntraDirCbValue;
			while ((--l) > 0)
			{
				xEncodeBin(0, m_cCUIntraDirSCModel.get(0,0,0));
			}
			if (iIntraDirCbValue < 3) //maxValue - 1
			{
				xEncodeBin(1, m_cCUIntraDirSCModel.get(0,0,0));
			}
		}
	}
	return;
}
#endif
Void TEncSbac::codeIntraDirCr( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
	//UInt uiIntraDir = pcCU->getIntraDirCr( uiAbsPartIdx );
	UInt LumaMode = pcCU->getIntraDir(uiAbsPartIdx);
	UInt uiIntraDirCr = pcCU->getIntraDirCr( uiAbsPartIdx );
	Bool is_redundant = false;
	int lmode = 0;
	int l;
	if ( LumaMode == INTRA_VER_IDX || LumaMode == INTRA_HOR_IDX || LumaMode == INTRA_DC_IDX || LumaMode == INTRA_BI_IDX )
	{
		lmode = LumaMode == INTRA_VER_IDX ? INTRA_VERT_PRED_CHROMA : (LumaMode == INTRA_HOR_IDX ? INTRA_HOR_PRED_CHROMA : (LumaMode == INTRA_DC_IDX ? INTRA_DC_PRED_CHROMA : INTRA_BI_PRED_CHROMA));
		is_redundant = true;
	}

	if (uiIntraDirCr == 0)
	{
		xEncodeBin(1, m_cCUIntraDirSCModel.get(0,0,0));
	}
	else
	{
		xEncodeBin(0, m_cCUIntraDirSCModel.get(0,0,0));
		if (is_redundant) //luma predmode is 0 2 12 or 24, the chroma is 1 4 3 2
		{
			if (uiIntraDirCr > lmode) //the minimum of uiIntraDirCb is 2
			{
				uiIntraDirCr -= 2; //其实是按照原来的模式号减去1来编，因为上面已经先编了一个0
			} else {
				uiIntraDirCr -= 1; //其实是按照原来的模式号编，因为上面已经先编了一个0
			}
		} else {
			uiIntraDirCr -= 1; //其实是按照原来的模式号编，因为上面已经先编了一个0
		}
		if (uiIntraDirCr == 0)
		{
			xEncodeBin(1, m_cCUIntraDirSCModel.get(0,0,0));
		} else {
			xEncodeBin(0, m_cCUIntraDirSCModel.get(0,0,0));
			l = uiIntraDirCr;
			while ((--l) > 0)
			{
				xEncodeBin(0, m_cCUIntraDirSCModel.get(0,0,0));
			}
			if (uiIntraDirCr < 3) //maxValue - 1
			{
				xEncodeBin(1, m_cCUIntraDirSCModel.get(0,0,0));
			}
		}
	}

	return;
}

Void TEncSbac::codeIntraPuTypeIdx( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
#if YQH_INTRA
	PartSize eSize = pcCU->getPartitionSize( uiAbsPartIdx );
	if ( pcCU->isIntra( uiAbsPartIdx ) )  //zhangyiCheck: IntraCuFlag == 1?
	{
		UInt SizeInBit = pcCU->getLog2CUSize(uiAbsPartIdx);
		if(((pcCU->getPicture()->getSPS()->getNonsquareIntraPredictionEnableFlag() == 1) && ((SizeInBit > 3) && (SizeInBit < 6)))&&eSize != SIZE_2Nx2N)
		{
			if(eSize == SIZE_hNx2N||eSize == SIZE_2NxhN)
				xEncodeBin( eSize == SIZE_hNx2N? 0 : 1, m_cPUIntraTypeSCModel.get( 0, 0, 0 ) );
		}
	}
#else
	xEncodeBin(0, m_cCUIntraDirSCModel.get(0,0,0)); //zhangyiCheck: how to get the syntax IntraPuTypeIndex
#endif

	return;
}
#endif
Void TEncSbac::codeIntraDir( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
#if ZHANGYI_INTRA
	Int  uiIntraDirValue = pcCU->getIntraDirValue( uiAbsPartIdx );
	if (uiIntraDirValue >= 0) //not in mpm
	{
		xEncodeBin(0, m_cCUIntraDirSCModel.get(0, 0, 0)); //截断一元码
		xEncodeBin((uiIntraDirValue & 0x10) >> 4, m_cCUIntraDirSCModel.get(0, 0, 0+1));
		xEncodeBin((uiIntraDirValue & 0x08) >> 3, m_cCUIntraDirSCModel.get(0, 0, 0+2));
		xEncodeBin((uiIntraDirValue & 0x04) >> 2, m_cCUIntraDirSCModel.get(0, 0, 0+3));
		xEncodeBin((uiIntraDirValue & 0x02) >> 1, m_cCUIntraDirSCModel.get(0, 0, 0+4));
		xEncodeBin((uiIntraDirValue & 0x01)        , m_cCUIntraDirSCModel.get(0, 0, 0+5));
	}
	else //in mpm
	{
		xEncodeBin(1, m_cCUIntraDirSCModel.get(0, 0, 0));
		xEncodeBin(uiIntraDirValue + 2, m_cCUIntraDirSCModel.get(0, 0, 0+6));
	}

#else
  UInt uiIntraDir = pcCU->getIntraDir( uiAbsPartIdx );
  xEncodeBin( ( uiIntraDir == 0 ? 0 : 1 ), m_cCUIntraDirSCModel.get( 0, 0, 0 ) );

  if ( uiIntraDir )
  {
    xEncodeBin( uiIntraDir-1, m_cCUIntraDirSCModel.get( 0, 0, 0 ) );
  }
#endif
  return;
}

#if ZHANGYI_INTRA
Void TEncSbac::codeIntraDirForRDO            ( Int iIntraDirForRDO )
{
	if (iIntraDirForRDO >= 0) //not in mpm
	{
		xEncodeBin(0, m_cCUIntraDirSCModel.get(0, 0, 0)); //截断一元码
		xEncodeBin((iIntraDirForRDO & 0x10) >> 4, m_cCUIntraDirSCModel.get(0, 0, 0+1));
		xEncodeBin((iIntraDirForRDO & 0x08) >> 3, m_cCUIntraDirSCModel.get(0, 0, 0+2));
		xEncodeBin((iIntraDirForRDO & 0x04) >> 2, m_cCUIntraDirSCModel.get(0, 0, 0+3));
		xEncodeBin((iIntraDirForRDO & 0x02) >> 1, m_cCUIntraDirSCModel.get(0, 0, 0+4));
		xEncodeBin((iIntraDirForRDO & 0x01)        , m_cCUIntraDirSCModel.get(0, 0, 0+5));
	}
	else //in mpm
	{
		xEncodeBin(1, m_cCUIntraDirSCModel.get(0, 0, 0));
		xEncodeBin(iIntraDirForRDO + 2, m_cCUIntraDirSCModel.get(0, 0, 0+6));
	}
}
#endif
#if RPS
//refidx
Void TEncSbac::codeRefIdx(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPic eRefPic)
{
  Int iRefIdx = pcCU->getCUMvField(eRefPic)->getRefIdx(uiAbsPartIdx);
  assert(iRefIdx >= 0);
  ContextModel *pCtx = m_cCURefPicSCModel.get(0);

  Int iActualCtxIdx;
  Bool bBSlice = (pcCU->getPicture()->getPictureType() == B_PICTURE);
  

  iActualCtxIdx = 0;
  if (iRefIdx == 0)
  {
    xEncodeBin(1, *pCtx);
  }
  else
  {
    xEncodeBin(0, *pCtx);
    if (bBSlice == false)
    {
      iRefIdx--;
      iActualCtxIdx = 4;

      while (iRefIdx >= 1)
      {
        xEncodeBin(0, *(pCtx + iActualCtxIdx));
        iRefIdx--;
        iActualCtxIdx++;
        if (iActualCtxIdx >= 5)
        {
          iActualCtxIdx = 5;
        }
      }
      if (pcCU->getCUMvField(eRefPic)->getRefIdx(uiAbsPartIdx) < pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic) - 1)
      {
        xEncodeBin(1, *(pCtx + iActualCtxIdx));
      }
    }
  }
  return;
}
#endif

//test
extern Int write_flag;

#if MVD
Void TEncSbac::codeMvd(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPic eRefPic)
{
	TComCUMvField* pcCUMvField = pcCU->getCUMvField(eRefPic);

#if RD_MVP
	Int iRefIdx = pcCU->getCUMvField(eRefPic)->getRefIdx(uiAbsPartIdx);
	TComMv cLastMv = pcCU->getMvFieldPred(uiAbsPartIdx, eRefPic, iRefIdx).getMv();
#else
	TComMv cLastMv = pcCU->getMvPred(uiAbsPartIdx, eRefPic);
#endif
	TComMv cCurrMv = pcCUMvField->getMv(uiAbsPartIdx);

	//printf("Addr=%d offset=%d %d  %d   %d  %d", pcCU->getAddr(), uiAbsPartIdx, cLastMv.getHor(), cLastMv.getVer(), cCurrMv.getHor(), cCurrMv.getVer());
	//if ((cLastMv.getHor() != 0 && uiAbsPartIdx != 0) || (cLastMv.getVer() != 0 && uiAbsPartIdx != 0))
	//{
	//	printf("  !!!!!!!!!!!!!!!!!!!!!");
	//	if (cLastMv.getHor() != 0)
	//		printf(" ||Hor:%d", pcCU->getMvFieldPred(0, eRefPic, iRefIdx).getMv().getHor());
	//	if (cLastMv.getVer() != 0)
	//		printf(" ||Ver:%d", pcCU->getMvFieldPred(0, eRefPic, iRefIdx).getMv().getVer());
	//}
	//printf("\n");

#if RD_PMVR
	Int iHor, iVer;
	if (pcCU->getPicture()->getSPS()->getPmvrEnableFlag() == 1)
	{
		TComMv mvd(0, 0);
		pcCU->pmvrMvdDerivation(mvd, cCurrMv, cLastMv);
		iHor = mvd.getHor();
		iVer = mvd.getVer();
	}
	else
	{
		iHor = cCurrMv.getHor() - cLastMv.getHor();
		iVer = cCurrMv.getVer() - cLastMv.getVer();
	}
#else
	Int iHor = cCurrMv.getHor() - cLastMv.getHor();
	Int iVer = cCurrMv.getVer() - cLastMv.getVer();
	TComMv mvd;
	mvd.set(iHor, iVer);
#endif
  //printf("\n%d,%d,%d\t",pcCU->getHeight(uiAbsPartIdx),pcCU->getWidth(uiAbsPartIdx),uiAbsPartIdx);
  //printf("%d,%d\t",cCurrMv.getHor(),cCurrMv.getVer());
#if mvd_revise
	TComMv mvd;
	mvd.set(iHor, iVer);
#endif

#if wlq_set_mvd
	mvd = pcCUMvField->getMvd(uiAbsPartIdx);
#endif

	//test
	//if (write_flag)
		//printf("Addr=%d PredMV:%d  %d	MVD:%d  %d\n", pcCU->getAddr(), cLastMv.getHor(), cLastMv.getVer(), iHor, iVer);
		//printf("Addr=%d PredMV:%d  %d	MVD:%d  %d\n", pcCU->getAddr(), cLastMv.getHor(), cLastMv.getVer(), mvd.getHor(), mvd.getVer());

	Int k = 1;//B帧需要修改，计算k
	Int mv_sign, act_sym, act_ctx = 0, exp_golomb_order;
	ContextModel* pCtx = m_cCUMvdSCModel.get(0, 0);
	for (int i = 0; i < 2; i++)
	{
		exp_golomb_order = 0;
		act_sym = absm(mvd[i]);
		k = i;
		if (act_sym < 3) {   // 0, 1, 2
			if (act_sym == 0) 
			{
				xEncodeBin(0, m_cCUMvdSCModel.get(0, k, act_ctx));
			}
			else if (act_sym == 1) 
			{
				xEncodeBin(1, m_cCUMvdSCModel.get(0, k, act_ctx));
				xEncodeBin(0, m_cCUMvdSCModel.get(0, k, 3));
			}
			else if (act_sym == 2) 
			{
				xEncodeBin(1, m_cCUMvdSCModel.get(0, k, act_ctx));
				xEncodeBin(1, m_cCUMvdSCModel.get(0, k, 3));
				xEncodeBin(0, m_cCUMvdSCModel.get(0, k, 4));
			}

		}
		else {
			if (act_sym % 2 == 1) {   //odds >3
				xEncodeBin(1, m_cCUMvdSCModel.get(0, k, act_ctx));
				xEncodeBin(1, m_cCUMvdSCModel.get(0, k, 3));
				xEncodeBin(1, m_cCUMvdSCModel.get(0, k, 4));
				xEncodeBinEP(0);
				act_sym = (act_sym - 3) / 2;

			}
			else { //even >3
				xEncodeBin(1, m_cCUMvdSCModel.get(0, k, act_ctx));
				xEncodeBin(1, m_cCUMvdSCModel.get(0, k, 3));
				xEncodeBin(1, m_cCUMvdSCModel.get(0, k, 4));
				xEncodeBinEP(1);
				act_sym = (act_sym - 4) / 2;
			}

			// exp_golomb part
			while (1) {
				if (act_sym >= (unsigned int)(1 << exp_golomb_order)) {
					xEncodeBinEP( 0);
					act_sym = act_sym - (1 << exp_golomb_order);
					exp_golomb_order++;
				}
				else {
					xEncodeBinEP(1);

					while (exp_golomb_order--) {   //next binary part
						xEncodeBinEP((unsigned char)((act_sym >> exp_golomb_order) & 1));
					}

					break;
				}
			}
		}

		if (mvd[i] != 0) {
			mv_sign = (mvd[i] >= 0) ? 0 : 1;
			xEncodeBinEP((unsigned char)mv_sign);
		}
	}
}
#else
#if RD_MVP
Void TEncSbac::codeMvd(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPic eRefPic)
{
	TComCUMvField* pcCUMvField = pcCU->getCUMvField(eRefPic);
	Int iRefIdx = pcCU->getCUMvField(eRefPic)->getRefIdx(uiAbsPartIdx);
	TComMv cLastMv = pcCU->getMvFieldPred(uiAbsPartIdx, eRefPic, iRefIdx).getMv();
	TComMv cCurrMv = pcCUMvField->getMv(uiAbsPartIdx);

#if RD_PMVR
	Int iHor, iVer;
	if (pcCU->getPicture()->getSPS()->getPmvrEnableFlag() == 1)
	{
		TComMv mvd(0, 0);
		pcCU->pmvrMvdDerivation(mvd, cCurrMv, cLastMv);
		iHor = mvd.getHor();
		iVer = mvd.getVer();
	}
	else
	{
		iHor = cCurrMv.getHor() - cLastMv.getHor();
		iVer = cCurrMv.getVer() - cLastMv.getVer();
	}
#else
	Int iHor = cCurrMv.getHor() - cLastMv.getHor();
	Int iVer = cCurrMv.getVer() - cLastMv.getVer();
#endif

	xWriteMvd(iHor, 0);
	xWriteMvd(iVer, 1);

	return;
}
#else
Void TEncSbac::codeMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPic eRefPic )
{
  TComCUMvField* pcCUMvField = pcCU->getCUMvField( eRefPic );
#if RD_MVP
	Int iRefIdx = pcCU->getCUMvField(eRefPic)->getRefIdx(uiAbsPartIdx);
	TComMv cLastMv = pcCU->getMvFieldPred(uiAbsPartIdx, eRefPic, iRefIdx).getMv();
#else
#if RPS
  TComMv cLastMv = pcCU->getMvFieldPred(uiAbsPartIdx, eRefPic).getMv();
#else
  TComMv cLastMv = pcCU->getMvPred(uiAbsPartIdx, eRefPic); s
#endif
#endif
 
  TComMv cCurrMv = pcCUMvField->getMv( uiAbsPartIdx );

#if RD_PMVR

	Int iHor, iVer;
	if (pcCU->getPicture()->getSPS()->getPmvrEnableFlag() == 1)
	{
		TComMv mvd(0, 0);
		pcCU->pmvrMvdDerivation(mvd, cCurrMv, cLastMv);
		iHor = mvd.getHor();
		iVer = mvd.getVer();
	}
	else
	{
		iHor = cCurrMv.getHor() - cLastMv.getHor();
		iVer = cCurrMv.getVer() - cLastMv.getVer();
	}
#else

	Int iHor = cCurrMv.getHor() - cLastMv.getHor();
	Int iVer = cCurrMv.getVer() - cLastMv.getVer();
#endif

  xWriteMvd( iHor, 0 );
  xWriteMvd( iVer, 1 );

  return;
}
#endif
#endif

Void TEncSbac::codeAllCbfs( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiCbf = 1;
  if( ( pcCU->getCbf(uiAbsPartIdx,TEXT_LUMA,0) == 0 ) && ( pcCU->getCbf(uiAbsPartIdx,TEXT_CHROMA_U,0) == 0 ) && ( pcCU->getCbf(uiAbsPartIdx,TEXT_CHROMA_V,0) == 0 ) )
    uiCbf = 0;
#if WLQ_set_CBP
  if (pcCU->isIntra(uiAbsPartIdx))
	  xEncodeBin(uiCbf, m_cCUQtRootCbfSCModel.get(0, 0, 0));
#endif
#if	!AVS3_DEBUG_NOCBP
  xEncodeBin(uiCbf, m_cCUQtRootCbfSCModel.get(0, 0, 0));
#endif
}

Void TEncSbac::codeCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  UInt uiCbf = pcCU->getCbf     ( uiAbsPartIdx, eType, uiTrDepth );
  UInt uiCtx = pcCU->getCtxQtCbf( uiAbsPartIdx, eType, uiTrDepth );
#if WLQ_set_CBP
  if (pcCU->isIntra(uiAbsPartIdx))
	  xEncodeBin(uiCbf, m_cCUQtCbfSCModel.get(0, eType ? eType - 1 : eType, uiCtx));
#endif
#if	!AVS3_DEBUG_NOCBP
  xEncodeBin(uiCbf, m_cCUQtCbfSCModel.get(0, eType ? eType - 1 : eType, uiCtx));
#endif
}

#if niu_CbpBit

typedef struct pix_pos {
	int available;   //ABCD
	int mb_addr;    //MB position
	int x;
	int y;
	int pos_x;     //4x4 x-pos
	int pos_y;
} PixelPos;

//test
extern UInt EncodeCu_flag;
//UInt nx2N_Pos[16] = { 0, 1, 4, 5, 16, 17, 20, 21, 32, 33, 36, 37, 48, 49, 52, 53};
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
		}
		else
			AbsPartIdx_real = uiAbsPartIdx;
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

				//UInt uiCurrPartNumb= neighborCU->getPic()->getNumPartInCU() >> (neighborCU->getDepth(uiAbsPartIdx) << 1);

				//if (neighborCU->getPic()->getNumPartInCU() == 16)
				//	AbsPartIdx_virtual = (tt % 4) * 4;
				//else
				//	//AbsPartIdx_virtual = (tt / 32 * 32) + (tt % 8) * 4;
				//	AbsPartIdx_virtual = (tt / 2 % 4) * 4;
			}
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

Void TEncSbac::writeCbpbit(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt b8, UInt cbp_bit)
{
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

	int maxWH = 1 << uiBitSize;
	UInt uiAbsPartIdx_real = ToRealPos(pcCU, uiAbsPartIdx);
	UInt PULeftPartIdx, PULeftPartIdx_real = MAX_UINT;
	UInt PUAbovePartIdx, PUAbovePartIdx_real = MAX_UINT;

	mb_x = (mb_x == 0) ? 0 : 1;
	mb_y = (mb_y == 0) ? 0 : 1;

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


	/*if (x >= 0 && x < maxWH && y >= 0 && y <maxWH)
	{
	block_a.available = 1;
	leftMB = pcCU;
	PULeftPartIdx = uiAbsPartIdx;
	}
	else*/
	{
		leftMB = pcCU->getPULeft(PULeftPartIdx_real, pcCU->getZorderIdxInCU() + uiAbsPartIdx_real);
		//leftMB = pcCU->getPULeft(PULeftPartIdx, pcCU->getZorderIdxInCU() + uiAbsPartIdx);
		block_a.available = (leftMB != NULL);
		if (leftMB != NULL)
		{
			PULeftPartIdx = ToVirtualPos(leftMB, PULeftPartIdx_real);
			//PULeftPartIdx = (PULeftPartIdx >> 2) << 2;
			//test
			UInt uiCurrPartNumb_left = leftMB->getPic()->getNumPartInCU() >> (leftMB->getDepth(PULeftPartIdx) << 1);
			UInt uiCurrPartNumb_pcCU = pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1);

			UInt AbsPartIdx = g_auiZscanToRaster[(PULeftPartIdx_real / uiCurrPartNumb_left) * uiCurrPartNumb_left];
			UInt AbsPartIdx_pcCU = g_auiZscanToRaster[(uiAbsPartIdx_real / uiCurrPartNumb_pcCU) * uiCurrPartNumb_pcCU];
			/*UInt AbsPartIdx = g_auiZscanToRaster[(PULeftPartIdx / uiCurrPartNumb_left) * uiCurrPartNumb_left];
			UInt AbsPartIdx_pcCU = g_auiZscanToRaster[(uiAbsPartIdx / uiCurrPartNumb_pcCU) * uiCurrPartNumb_pcCU];*/

			UInt uiNumPartInCUWidth = leftMB->getPic()->getNumPartInWidth();
			block_a.x = x + (pcCU->getCUPelX() + (AbsPartIdx_pcCU % uiNumPartInCUWidth) * MIN_BLOCK_SIZE) - (leftMB->getCUPelX() + (AbsPartIdx % uiNumPartInCUWidth)* MIN_BLOCK_SIZE);
			block_a.y = y + (pcCU->getCUPelY() + (AbsPartIdx_pcCU / uiNumPartInCUWidth) * MIN_BLOCK_SIZE) - (leftMB->getCUPelY() + (AbsPartIdx / uiNumPartInCUWidth)* MIN_BLOCK_SIZE);
		}
		else
		{
			block_a.y = 0;
			block_a.x = 0;
		}
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


	//test

	/*if (x >= 0 && x < maxWH && y >= 0 && y <maxWH)
	{
	block_b.available = 1;
	upMB = pcCU;
	PUAbovePartIdx = uiAbsPartIdx;
	}
	else*/
	{
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
			/*UInt AbsPartIdx = g_auiZscanToRaster[(PUAbovePartIdx / uiCurrPartNumb_up) * uiCurrPartNumb_up];
			UInt AbsPartIdx_pcCU = g_auiZscanToRaster[(uiAbsPartIdx / uiCurrPartNumb_pcCU) * uiCurrPartNumb_pcCU];*/

			UInt uiNumPartInCUWidth = upMB->getPic()->getNumPartInWidth();
			block_b.x = x + (pcCU->getCUPelX() + (AbsPartIdx_pcCU % uiNumPartInCUWidth) * MIN_BLOCK_SIZE) - (upMB->getCUPelX() + (AbsPartIdx % uiNumPartInCUWidth)* MIN_BLOCK_SIZE);
			block_b.y = y + (pcCU->getCUPelY() + (AbsPartIdx_pcCU / uiNumPartInCUWidth) * MIN_BLOCK_SIZE) - (upMB->getCUPelY() + (AbsPartIdx / uiNumPartInCUWidth)* MIN_BLOCK_SIZE);
		}
		else
		{
			block_b.y = 0;
			block_b.x = 0;
		}
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
		xEncodeBin(cbp_bit, m_cCUQtCbfSCModel.get(0, 2, 0));
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
		//test
#if niu_ab_print
		if (EncodeCu_flag && !pcCU->isIntra(uiAbsPartIdx))
			printf("Addr=%d Size=%d a=%d b=%d\n", pcCU->getAddr(), uiBitSize, a, b);
#endif

		xEncodeBin(cbp_bit, m_cCUQtCbfSCModel.get(0, 0, a + 2 * b));
	}
}
#endif

#if niu_write_cbf
Void TEncSbac::codeCbfY(TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth)
{
#if niu_intercbf_revise
	UInt uiCbf = 0;
#else
	UInt uiCbf = pcCU->getCbf(uiAbsPartIdx, eType, uiTrDepth);
	UInt uiCtx = pcCU->getCtxQtCbf(uiAbsPartIdx, eType, uiTrDepth);
	PartSize eSize = pcCU->getPartitionSize(uiAbsPartIdx);
#endif

#if niu_intercbf_revise
	UChar Trmode = pcCU->getTransformIdx(uiAbsPartIdx);
#endif

	UChar CbfU = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0);
	UChar CbfV = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0);
#if niu_intercbf_revise
	if (!pcCU->isIntra(uiAbsPartIdx))
	{
		UChar allYcbf[4] = { 0 };
		if (!Trmode)
			allYcbf[0] = pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, uiTrDepth);
		else
		{
			for (Int m = 0; m <= 3; m++)
			{
				UInt uiPartDepth = pcCU->getDepth(uiAbsPartIdx) + 1;
				UInt uiCurrPartNumb = pcCU->getPic()->getNumPartInCU() >> (uiPartDepth << 1);
				allYcbf[m] = pcCU->getCbf(uiAbsPartIdx + m*uiCurrPartNumb, TEXT_LUMA, 1);
			}
		}

		UChar flag = 0;
		flag = flag | CbfU;
		flag = flag | CbfV;
		for (Int m = 0; m < 4; m++)
			flag = flag | allYcbf[m];
		if (!(pcCU->isDirect(uiAbsPartIdx) || pcCU->isSkip(uiAbsPartIdx)))
		{
#if niu_CbpBit  //niu_CbpBit_inter
			writeCbpbit(pcCU, uiAbsPartIdx, 4, !flag);
#else
			xEncodeBin(!flag, m_cCUQtCbfSCModel.get(0, 2, 0));
#endif
		}
		if (flag)
		{
			//////////////////////////////   tr_size   /////////////////////////////////
			xEncodeBin(Trmode, m_cCUTransIdxSCModel.get(0, 0, 0));
			//////////////////////////////   chroma==0   /////////////////////////////////
			xEncodeBin((CbfU || CbfV), m_cCUQtCbfSCModel.get(0, 1, 0));// 0
			if (!Trmode)
			{
				if (!(CbfU || CbfV))
					return;
				else
				{
					if (CbfU || CbfV)
					{
						xEncodeBin((CbfU&&CbfV), m_cCUQtCbfSCModel.get(0, 1, 0 + 2));
						if (!(CbfU&&CbfV))
							xEncodeBin((CbfV == 1 ? 1 : 0), m_cCUQtCbfSCModel.get(0, 1, 0 + 2));
					}
#if niu_CbpBit  //niu_CbpBit_inter
					writeCbpbit(pcCU, uiAbsPartIdx, 0, allYcbf[0]);
#else
					xEncodeBin(allYcbf[0], m_cCUQtCbfSCModel.get(0, 0, 0 + 0));
#endif
				}
			}
			else
			{
				if (CbfU || CbfV)
				{
					xEncodeBin((CbfU&&CbfV), m_cCUQtCbfSCModel.get(0, 1, 0 + 2));
					if (!(CbfU&&CbfV))
						xEncodeBin((CbfV == 1 ? 1 : 0), m_cCUQtCbfSCModel.get(0, 1, 0 + 2));
				}
#if niu_CbpBit  //niu_CbpBit_inter
				for (Int m = 0; m <= 3; m++)
				{

					UInt uiPartDepth = pcCU->getDepth(uiAbsPartIdx) + 1;
					UInt uiCurrPartNumb = pcCU->getPic()->getNumPartInCU() >> (uiPartDepth << 1);
					UInt CCbf = pcCU->getCbf(uiAbsPartIdx + m * uiCurrPartNumb, TEXT_LUMA, 1);
					writeCbpbit(pcCU, uiAbsPartIdx + m * uiCurrPartNumb, m, CCbf);
				}
#else
				xEncodeBin(allYcbf[0], m_cCUQtCbfSCModel.get(0, 0, 0 + 0));
				xEncodeBin(allYcbf[1], m_cCUQtCbfSCModel.get(0, 0, 0 + 0));
				xEncodeBin(allYcbf[2], m_cCUQtCbfSCModel.get(0, 0, 0 + 0));
				xEncodeBin(allYcbf[3], m_cCUQtCbfSCModel.get(0, 0, 0 + 0));
#endif
			}
		}
	}
	else
	{
#endif
		//test eType ? eType - 1 ：eType
#if niu_intercbf_revise
		if (!Trmode)
#else
		if (eSize == SIZE_2Nx2N)
#endif

		{
			uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, uiTrDepth);
#if niu_intercbf_revise
#if niu_CbpBit
			writeCbpbit(pcCU, uiAbsPartIdx, 0, uiCbf);
#else
			xEncodeBin(uiCbf, m_cCUQtCbfSCModel.get(0, 0, 0));
#endif
#else
			xEncodeBin(uiCbf, m_cCUQtCbfSCModel.get(0, eType ? eType - 1 : eType, 0));
#endif
		}
		else
		{
			//NXN
			UChar CCbf = 0;
			for (Int m = 0; m <= 3; m++)
			{
#if ZY_MODIFY_CBF
				UInt uiPartDepth = pcCU->getDepth(uiAbsPartIdx) + 1;
				UInt uiCurrPartNumb = pcCU->getPic()->getNumPartInCU() >> (uiPartDepth << 1);
				CCbf = pcCU->getCbf(uiAbsPartIdx + m*uiCurrPartNumb, TEXT_LUMA, 1);
#else
				CCbf = pcCU->getCbf(uiAbsPartIdx + m * 1, TEXT_LUMA, 1);
#endif
#if niu_intercbf_revise
#if niu_CbpBit
				writeCbpbit(pcCU, uiAbsPartIdx + m * uiCurrPartNumb, m, CCbf);
#else
				xEncodeBin(CCbf, m_cCUQtCbfSCModel.get(0, 0, 0));
#endif
#else
				xEncodeBin(CCbf, m_cCUQtCbfSCModel.get(0, eType ? eType - 1 : eType, 0));
#endif
			}

		}
#if niu_intercbf_revise
		xEncodeBin((CbfU || CbfV), m_cCUQtCbfSCModel.get(0, 1, 0 + 1));
#else
		xEncodeBin((CbfU || CbfV), m_cCUQtCbfSCModel.get(0, eType ? eType - 1 : eType, 0 + 1));
#endif

		if (CbfU || CbfV)
		{
#if niu_intercbf_revise
			xEncodeBin((CbfU&&CbfV), m_cCUQtCbfSCModel.get(0, 1, 0 + 3));
#else
			xEncodeBin((CbfU&&CbfV), m_cCUQtCbfSCModel.get(0, eType ? eType - 1 : eType, 0 + 3));
#endif

			if (!(CbfU&&CbfV))
#if niu_intercbf_revise
				xEncodeBin((CbfV == 1 ? 1 : 0), m_cCUQtCbfSCModel.get(0, 1, 0 + 3));
#else
				xEncodeBin((CbfV == 1 ? 1 : 0), m_cCUQtCbfSCModel.get(0, eType ? eType - 1 : eType, 0 + 3));
#endif
		}

#if niu_intercbf_revise
	}
#endif
}

Void TEncSbac::codeCbfUV(TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth)
{
	UInt uiCbf = pcCU->getCbf(uiAbsPartIdx, eType, uiTrDepth);
	UInt uiCtx = pcCU->getCtxQtCbf(uiAbsPartIdx, eType, uiTrDepth);
#if WLQ_set_CBP
	if (pcCU->isIntra(uiAbsPartIdx))
		xEncodeBin(uiCbf, m_cCUQtCbfSCModel.get(0, eType ? eType - 1 : eType, uiCtx));
#endif
#if	!AVS3_DEBUG_NOCBP
	xEncodeBin(uiCbf, m_cCUQtCbfSCModel.get(0, eType ? eType - 1 : eType, uiCtx));
#endif
}


#endif

Void TEncSbac::codeCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiLog2Size, UInt uiDepth, TextType eTType, Bool bRD )
{
  // check LOT
#if niu_NSQT_revise
	UInt uiLog2Size_org = uiLog2Size;
#endif
  if ( uiLog2Size > m_pcPicture->getSPS()->getLog2MaxPhTrSize() )
  {
    uiLog2Size = m_pcPicture->getSPS()->getLog2MaxPhTrSize();
  }

#if ZHANGYI_INTRA_SDIP
  UInt uiSdipFlag = pcCU->getSDIPFlag(uiAbsPartIdx); //zhangyiCheckSDIP
  UInt uiSdipDir = pcCU->getSDIPDirection(uiAbsPartIdx); //zhangyiCheckSDIP
  UInt uiWidth, uiHeight;
  #if ZHANGYI_INTRA_SDIP_BUG_YQH
  if (uiSdipFlag && (eTType == TEXT_LUMA))
#else
  if (uiSdipFlag)
#endif
  {
	  uiWidth = uiSdipDir ? pcCU->getWidth(uiAbsPartIdx) : ((pcCU->getWidth(uiAbsPartIdx)) >> 2);
	  uiHeight = uiSdipDir ? (pcCU->getHeight(uiAbsPartIdx) >> 2) : pcCU->getHeight(uiAbsPartIdx);
  }
  else
  {
	  uiWidth = (1 << uiLog2Size);
	  uiHeight = (1 << uiLog2Size);
  }
#else
  UInt uiSize = 1 << uiLog2Size;
#endif
  
  UInt uiNumSig = 0;
  UInt uiCTXIdx = 0;
  
  // compute number of significant coefficients
  UInt  uiPart = 0;

#if CODEFFCODER_LDW_WLQ_YQH
  UInt uiNumofCoeff = 0;
  Int CoeffScanIndex=0,CGSacnIndex=0;
  Int         iWidth;
  Int         iHeight;
  UInt        uiPartAddr;
  Bool   bRevers=false;
  PartSize m_pePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
#if RD_NSQT
  Bool bNSQT = pcCU->getPicture()->getSPS()->getNonsquareQuadtreeTansformEnableFlag();

#if niu_NSQT_BUG
#if niu_NSQT_revise
  if ((uiLog2Size_org < pcCU->getLog2CUSize(uiAbsPartIdx)) && (eTType == TEXT_LUMA))
#else
  if ((uiLog2Size < pcCU->getLog2CUSize(uiAbsPartIdx)) && (eTType == TEXT_LUMA))
#endif
#else
  if ((uiLog2Size < pcCU->getLog2CUSize(0)) && (eTType == TEXT_LUMA)) //加入CU split之后是否有问题？？？
#endif

  {
#if niu_NSQT_dec
	  pcCU->getTUSize(0, iWidth, iHeight, uiLog2Size, m_pePartSize, bNSQT);//这个函数编解码端都会用到，应该放在common里面，TComDataCU.cpp
#else
	  getTUSize(0, iWidth, iHeight, uiLog2Size, m_pePartSize, bNSQT);//这个函数编解码端都会用到，应该放在common里面，TComDataCU.cpp
#endif
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
 
  #if ZHANGYI_INTRA_SDIP_BUG_YQH
  #if ZHANGYI_INTRA_SDIP
  if (uiSdipFlag && (eTType == TEXT_LUMA))
  {
	  iWidth = uiSdipDir ? pcCU->getWidth(uiAbsPartIdx) : ((pcCU->getWidth(uiAbsPartIdx)) >> 2);
	  iHeight = uiSdipDir ? (pcCU->getHeight(uiAbsPartIdx) >> 2) : pcCU->getHeight(uiAbsPartIdx);
}
#endif
#endif
#else
  getTUSize(0, iWidth, iHeight, uiLog2Size, m_pePartSize);
#endif
  //if((iWidth) * (iHeight) >= 16)
  if((iWidth>=4) && (iHeight>=4) )
 // if(0)
  {
	  CoeffCodingParam   m_CoeffCodingParam = pcCU->getCoeffCodingParam();

	  if((iWidth==2||iHeight==2)&&(eTType == TEXT_LUMA))
		  printf("error TU size iWidth=%d and iHeight =%d \n", iWidth,iHeight);
	  getCoeffScanAndCGSacn( CoeffScanIndex,  CGSacnIndex , iWidth, iHeight );

	  UInt  uiIntraModeIdx = g_auiIntraModeClassified[ static_cast<UInt>(pcCU->getIntraDir(uiAbsPartIdx))];
	  if(iWidth==iHeight&&pcCU->getPredictionMode(uiAbsPartIdx)==MODE_INTRA&&(eTType == TEXT_LUMA)&&uiIntraModeIdx==INTRA_PRED_HOR)
		  bRevers=true; 
//#if ZHANGYI_INTRA_SDIP
//	  if (CoeffScanIndex>16 || CGSacnIndex>9)
//#else
//	  if (CoeffScanIndex>16 || CGSacnIndex>5)
//#endif
//		  printf("ERROR CoeffScanIndex=%d CGSacnIndex==%d", CoeffScanIndex, CGSacnIndex);
	  
	  xCheckCoeffcode( pcCU, pcCoef, CoeffScanIndex,  iWidth, iHeight,bRevers, uiNumofCoeff ,m_CoeffCodingParam);
#if	  CODEFFCODER_LDW_WLQ_YQH_BUG2
	  if (bRD)
	  {
#if !niu_intercbf_revise
		  UInt uiTempDepth = uiDepth - pcCU->getDepth(uiAbsPartIdx);
		  pcCU->setCbfSubParts((uiNumofCoeff ? 1 : 0) << uiTempDepth, eTType, uiAbsPartIdx, uiDepth);
		  codeCbf(pcCU, uiAbsPartIdx, eTType, uiTempDepth);
#endif
	  }

#endif
	  codeCoeff( pcCU, pcCoef, uiAbsPartIdx,CGSacnIndex , iWidth, iHeight,eTType, uiNumofCoeff ,m_CoeffCodingParam);
#if INTERTEST
    if (!pcCU->isIntra(uiAbsPartIdx))
    {
      if (bRD)
      {
        UInt uiTempDepth = uiDepth - pcCU->getDepth(uiAbsPartIdx);
        pcCU->setCbfSubParts((uiNumofCoeff ? 1 : 0) << uiTempDepth, eTType, uiAbsPartIdx, uiDepth);
#if !niu_intercbf_revise
		codeCbf(pcCU, uiAbsPartIdx, eTType, uiTempDepth);
#endif
      }

      if (uiNumofCoeff == 0)
      {
        return;
      }
    }
#endif
  }
  else
#endif

  {

#if ZHANGYI_INTRA_SDIP
	  xCheckCoeff(pcCoef, uiWidth, 0, uiNumSig, uiPart);
#else
	  xCheckCoeff(pcCoef, uiSize, 0, uiNumSig, uiPart);
#endif
  
  if ( bRD )
  {
    UInt uiTempDepth = uiDepth - pcCU->getDepth( uiAbsPartIdx );
    pcCU->setCbfSubParts( ( uiNumSig ? 1 : 0 ) << uiTempDepth, eTType, uiAbsPartIdx, uiDepth );
    codeCbf( pcCU, uiAbsPartIdx, eTType, uiTempDepth );
  }
  
  if ( uiNumSig == 0 )
  {
    return;
  }
  
  eTType = eTType == TEXT_LUMA ? TEXT_LUMA : ( eTType == TEXT_NONE ? TEXT_NONE : TEXT_CHROMA );

  const UInt   uiMaxNumCoeff     = 1 << ( uiLog2Size << 1 );

  // Initialization of the scanning pattern
  const UInt* pucScan  = g_auiFrameScanXY[uiLog2Size];

  UInt uiSign, uiAbs;
  UInt uiRun = 0;
  UInt uiPrevLevel = 6;

  for( UInt uiScanPos = 0; uiScanPos < uiMaxNumCoeff; uiScanPos++ )
  {
    UInt uiIndex = pucScan[ uiScanPos ];
    if( pcCoef[ uiIndex ] )
    {
      // level and sign calculation
      if( pcCoef[ uiIndex ] > 0) 
      { 
        uiAbs = static_cast<UInt>( pcCoef[ uiIndex ]);  
        uiSign = 0; 
      }
      else
      { 
        uiAbs = static_cast<UInt>(-pcCoef[ uiIndex ]);  
        uiSign = 1; 
      }

      // run coding
      xWriteUnarySymbolSbac( uiRun, &m_cCURunSCModel.get( uiCTXIdx, eTType, Min(uiPrevLevel-1, 5)*2 ), 2 );

      // level coding 
      xWriteUnarySymbolSbac( uiAbs-1, &m_cCUAbsSCModel.get( uiCTXIdx, eTType, Min(uiPrevLevel-1,5)*2 ), 2 );

      // sign coding (Bypass coding)
      xEncodeBinEP( uiSign );

      // no need to code last_flag if a non-zero coefficient is at the last position
      if ( uiScanPos == uiMaxNumCoeff-1 )
      {
        break;
      }
      
      uiRun = 0;
      uiPrevLevel = uiAbs;
      uiNumSig--;
      UInt uiLast = ( uiNumSig == 0 ) ? 1 : 0;

      xEncodeBin( uiLast, m_cCULastSCModel.get( uiCTXIdx, eTType, 0 ) );

      if( uiLast )
      {
        break;
      }
    }
    else
    {
      uiRun++;
    }
  }
    }
  return;
}

Void TEncSbac::codeDBKIdx( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiSymbol;

  if( (g_auiRasterToPelX[g_auiZscanToRaster[uiAbsPartIdx]] + pcCU->getCUPelX()) )
  {
    uiSymbol = pcCU->getDBKIdx(0, uiAbsPartIdx) ? 1 : 0;
    xEncodeBin(uiSymbol, m_cDBKIdxSCModel.get(0,0,0));
  }

  if( (g_auiRasterToPelY[g_auiZscanToRaster[uiAbsPartIdx]] + pcCU->getCUPelY()) )
  {
    uiSymbol = pcCU->getDBKIdx(1, uiAbsPartIdx) ? 1 : 0;
    xEncodeBin(uiSymbol, m_cDBKIdxSCModel.get(0,0,1));
  }
}

Void TEncSbac::estBit( estBitsSbacStruct* pcEstBitsSbac, TextType eTType )
{
  // QtCbf bit estimation
  estCBFBit( pcEstBitsSbac, eTType );
  estRunLevelLastBit( pcEstBitsSbac, eTType );
}

Void TEncSbac::estCBFBit( estBitsSbacStruct* pcEstBitsSbac, TextType eTType )
{
  for( UInt uiBin = 0; uiBin < 2; uiBin++ )
  {
    for ( UInt uiCtx = 0; uiCtx < 3; uiCtx++ )
    {
      pcEstBitsSbac->blockQtCbfBits[uiCtx][uiBin] = biari_no_bits( uiBin, m_cCUQtCbfSCModel.get( 0, eTType, uiCtx ) );
    }
    pcEstBitsSbac->blockRootQtCbfBits[uiBin] = biari_no_bits( uiBin, m_cCUQtRootCbfSCModel.get( 0, 0, 0 ) );
  }
}

Void TEncSbac::estRunLevelLastBit( estBitsSbacStruct* pcEstBitsSbac, TextType eTType )
{
  eTType = eTType == TEXT_LUMA ? TEXT_LUMA : TEXT_CHROMA;

  for ( UInt uiPos = 0; uiPos < 6; uiPos++ )
  {
    for( UInt uiBin = 0; uiBin < 2; uiBin++ )    
    {
      for ( UInt uiCtx = 0; uiCtx < 2; uiCtx++ )
      {
        pcEstBitsSbac->runBits[uiPos][uiCtx][uiBin] = biari_no_bits ( uiBin, m_cCURunSCModel.get( 0, eTType, uiPos*2 + uiCtx ) );
        pcEstBitsSbac->levelBits[uiPos][uiCtx][uiBin] = biari_no_bits ( uiBin, m_cCUAbsSCModel.get( 0, eTType, uiPos*2 + uiCtx ) );
      }
    }
  }

  pcEstBitsSbac->lastBits[ 0 ][ 0 ] = biari_no_bits ( 0, m_cCULastSCModel.get( 0, eTType, 0 ) );
  pcEstBitsSbac->lastBits[ 0 ][ 1 ] = biari_no_bits ( 1, m_cCULastSCModel.get( 0, eTType, 0 ) );
}

Int TEncSbac::biari_no_bits( Short symbol, ContextModel& rcSCModel )
{
#if wlq_bit_est
	return wlq_FIX_BIT;
#else
#if wlq_AEC 
	Int  estBits;
	UInt lg_pmps = rcSCModel.getLG_PMPS();
	symbol = (Short)(symbol != 0);
	estBits = (symbol == rcSCModel.getMPS()) ? (lg_pmps >> 2) : LPSbits[lg_pmps >> 2];
	return (estBits);
#else
  UInt  uiEstBits;
  Short ui16State;
  
  symbol    = (Short)( symbol != 0 );
  ui16State = ( symbol == rcSCModel.getMps() ) ? 64 + stateMappingTable[rcSCModel.getState()] : 63 - stateMappingTable[rcSCModel.getState()];
  uiEstBits = entropyBits[ 127 - ui16State ];
  return uiEstBits;
#endif
#endif
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TEncSbac::xWriteUnarySymbolSbac( UInt uiSymbol, ContextModel* pcSCModel, UInt uiNumCtx )
{
  UInt uiCtxIdx = 0;

  xEncodeBin( uiSymbol ? 1 : 0, pcSCModel[ uiCtxIdx ] );
  
  if( 0 == uiSymbol)
  {
    return;
  }
  
  while( uiSymbol-- )
  {
    if ( uiCtxIdx < uiNumCtx - 1 )
    {
      uiCtxIdx++;
    }
    xEncodeBin( uiSymbol ? 1 : 0, pcSCModel[ uiCtxIdx ] );
  }
  
  return;
}

Void TEncSbac::xWriteUnarySymbol( UInt uiSymbol, ContextModel* pcSCModel, Int iOffset )
{
  xEncodeBin( uiSymbol ? 1 : 0, pcSCModel[0] );
  
  if( 0 == uiSymbol)
  {
    return;
  }
  
  while( uiSymbol-- )
  {
    xEncodeBin( uiSymbol ? 1 : 0, pcSCModel[ iOffset ] );
  }
  
  return;
}

Void TEncSbac::xWriteUnaryMaxSymbol( UInt uiSymbol, ContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol )
{
  xEncodeBin( uiSymbol ? 1 : 0, pcSCModel[ 0 ] );
  
  if ( uiSymbol == 0 )
  {
    return;
  }
  
  Bool bCodeLast = ( uiMaxSymbol > uiSymbol );
  
  while( --uiSymbol )
  {
    xEncodeBin( 1, pcSCModel[ iOffset ] );
  }
  if( bCodeLast )
  {
    xEncodeBin( 0, pcSCModel[ iOffset ] );
  }
  
  return;
}

Void TEncSbac::xWriteExGolomb( UInt uiSymbol, ContextModel* pcSCModel )
{
  UInt uiCount = 0;
  UInt uiMarkerCtxIdx = ( uiSymbol == 0 ) ? 0 : 1;
  Bool bIsFirstBin = true;
  while( uiSymbol >= (UInt)(1<<uiCount) )
  {
    UInt uiFirstLeadingOneCtxIdx = bIsFirstBin ? 0 : 1;
    xEncodeBin( 1, pcSCModel[ uiFirstLeadingOneCtxIdx ] );
    bIsFirstBin = false;
    uiSymbol -= 1<<uiCount;
    uiCount  ++;
  }
  xEncodeBin( 0, pcSCModel[ uiMarkerCtxIdx ] );

  while( uiCount-- )
  {
    xEncodeBinEP( (uiSymbol>>uiCount) & 1 );
  }
  return;
}

Void TEncSbac::xWriteEpExGolomb( UInt uiSymbol, UInt uiCount )
{
  while( uiSymbol >= (UInt)(1<<uiCount) )
  {
    xEncodeBinEP( 1 );
    uiSymbol -= 1<<uiCount;
    uiCount  ++;
  }
  xEncodeBinEP( 0 );
  while( uiCount-- )
  {
    xEncodeBinEP( (uiSymbol>>uiCount) & 1 );
  }
  
  return;
}

Void TEncSbac::xCopyFrom( TEncSbac* pSrc )
{
  // Copy SBAC variables
#if wlq_AEC
	this->m_Elow				= pSrc->m_Elow;
	this->m_E_s1				= pSrc->m_E_s1;
	this->m_E_t1				= pSrc->m_E_t1;
	this->m_s2				= pSrc->m_s2;
	this->m_t2				= pSrc->m_t2;
	this->m_s1				= pSrc->m_s1;
	this->m_t1				= pSrc->m_t1;
	this->m_Ebits_to_follow	= pSrc->m_Ebits_to_follow;
#if 0//YQH_AEC_RDO_BUG
	this->m_uiByteCounter				= pSrc->m_uiByteCounter;
	this->m_uiCodeBitsbiggo				= pSrc->m_uiCodeBitsbiggo;
#endif
#else
  this->m_uiRange         = pSrc->m_uiRange;
  this->m_uiCode          = pSrc->m_uiCode;
 #endif
  this->m_uiCodeBits      = pSrc->m_uiCodeBits;
  this->m_ucPendingByte   = pSrc->m_ucPendingByte;
  this->m_bIsPendingByte  = pSrc->m_bIsPendingByte;
  this->m_uiStackedFFs    = pSrc->m_uiStackedFFs;
  this->m_uiStackedZeros  = pSrc->m_uiStackedZeros;
  
  this->m_uiCoeffCost     = pSrc->m_uiCoeffCost;
  
  // Copy context models
  this->m_cCUSplitFlagSCModel      .copyFrom( &pSrc->m_cCUSplitFlagSCModel       );
#if YQH_INTRA
  this->m_cTUSplitFlagSCModel      .copyFrom( &pSrc->m_cTUSplitFlagSCModel       );//yuquanhe@hisilicon.com
  this->m_cPUIntraTypeSCModel      .copyFrom( &pSrc->m_cPUIntraTypeSCModel       );//yuquanhe@hisilicon.com
#endif
  
  this->m_cCUPredModeSCModel       .copyFrom( &pSrc->m_cCUPredModeSCModel        );
  this->m_cCUPartSizeSCModel       .copyFrom( &pSrc->m_cCUPartSizeSCModel        );
#if INTER_GROUP
	this->m_cCUShapeOfPartitionSCModel.copyFrom(&pSrc->m_cCUShapeOfPartitionSCModel);
#endif
	this->m_cCUInterDirSCModel       .copyFrom( &pSrc->m_cCUInterDirSCModel        );
#if WRITE_INTERDIR
	this->m_cCUMinInterDirSCModel.copyFrom(&pSrc->m_cCUMinInterDirSCModel);
#endif
#if F_DHP_SYC
	this->m_cCUInterDHPSCModel.copyFrom(&pSrc->m_cCUInterDHPSCModel);
	this->m_cCUInterDHPNXNSCModel.copyFrom(&pSrc->m_cCUInterDHPNXNSCModel);
#endif
#if inter_direct_skip_bug1
	this->m_cCUInterWSMSCModel.copyFrom(&pSrc->m_cCUInterWSMSCModel);
#endif
#if F_MHPSKIP_SYC
	this->m_cCUInterMHPSKIPSCModel.copyFrom(&pSrc->m_cCUInterMHPSKIPSCModel);
#endif
  this->m_cCUIntraDirSCModel       .copyFrom( &pSrc->m_cCUIntraDirSCModel        );
#if RPS
  this->m_cCURefPicSCModel.copyFrom(&pSrc->m_cCURefPicSCModel);
#endif
#if DMH
	this->m_cCUDmhSCModel.copyFrom(&pSrc->m_cCUDmhSCModel);
#endif
  this->m_cCUMvdSCModel            .copyFrom( &pSrc->m_cCUMvdSCModel             );
  this->m_cCUQtCbfSCModel          .copyFrom( &pSrc->m_cCUQtCbfSCModel           );
  this->m_cCUQtRootCbfSCModel      .copyFrom( &pSrc->m_cCUQtRootCbfSCModel       );
  this->m_cCUTransIdxSCModel       .copyFrom( &pSrc->m_cCUTransIdxSCModel        );
  this->m_cCURunSCModel            .copyFrom( &pSrc->m_cCURunSCModel             );
  this->m_cCULastSCModel           .copyFrom( &pSrc->m_cCULastSCModel            );
  this->m_cCUAbsSCModel            .copyFrom( &pSrc->m_cCUAbsSCModel             );
  this->m_cDBKIdxSCModel           .copyFrom( &pSrc->m_cDBKIdxSCModel            );
  this->m_cCUXPosiSCModel			     .copyFrom( &pSrc->m_cCUXPosiSCModel				);
  this->m_cCUYPosiSCModel			     .copyFrom( &pSrc->m_cCUXPosiSCModel				);
#if CODEFFCODER_LDW_WLQ_YQH
  this->m_cTULastCGSCModel            .copyFrom( &pSrc->m_cTULastCGSCModel          );//yuquanhe@hisilicon.com
  this->m_cTULevelRunSCModel          .copyFrom( &pSrc->m_cTULevelRunSCModel        );//yuquanhe@hisilicon.com
  this->m_cTUSigCGSCModel			  .copyFrom( &pSrc->m_cTUSigCGSCModel	        );//yuquanhe@hisilicon.com
  this->m_cTULastPosInCGSCModel		 .copyFrom( &pSrc->m_cTULastPosInCGSCModel	    );//yuquanhe@hisilicon.com
#endif

}

#if CODEFFCODER_LDW_WLQ_YQH

Void TEncSbac:: getCGposXAndposY(Int riWidth, Int riHeight,Int iCG,Int CGSacnIndex , Int& CGX, Int& CGY  )
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


Void TEncSbac::xCheckCoeffcode( TComDataCU* pcCU,TCoeff* pcCoef,Int CoeffScanIndex, Int iWidth, Int iHeight, Bool bReverse, UInt& uiNumofCoeff, CoeffCodingParam&  m_CoeffCodingParam)
{

	UInt   uiMaxNumCoeff     = iWidth*iHeight;
	if(CoeffScanIndex>18)
	{
		printf("error\n");
	}
	const UInt* pucScan  = g_auiFrameScanXY[CoeffScanIndex];
	const UInt* pucScanX ;
	const UInt* pucScanY;
	UInt uiRun = 0;
	UInt uiPrevLevel = 6;
	UInt ipos=0;
	Int run=-1;
	if(bReverse)
	{
		 pucScanX  = g_auiFrameScanY[CoeffScanIndex];
		 pucScanY  = g_auiFrameScanX[CoeffScanIndex];
	}
	else
	{
		pucScanX  = g_auiFrameScanX[CoeffScanIndex];
		 pucScanY  = g_auiFrameScanY[CoeffScanIndex];
	}

#if	YQH_DEBUG_ZY_BUG
	//pcCoef[0]=0; pcCoef[1]=pcCoef[2]=pcCoef[3]=0;
	//pcCoef[4]=1; pcCoef[5]=pcCoef[6]=pcCoef[7]=0;
	//pcCoef[8]=-1;pcCoef[9]=pcCoef[10]=pcCoef[11]=0;
	//pcCoef[12]=1;pcCoef[13]=pcCoef[14]=pcCoef[15]=0;
#endif
#if wlq_avs2x_debug
	int sign=0;
	//if (pcCU->getAddr() == 0 && iWidth == 16 && iHeight == 4)
	//	sign = 1;
#endif
	//////////////////
	for( UInt uiScanPos = 0; uiScanPos < uiMaxNumCoeff; uiScanPos++ )
	{
		UInt  iCG = uiScanPos >> 4;
		UInt uiIndex,uiIndexX,uiIndexY;
		if (uiScanPos % 16 == 0) {
			m_CoeffCodingParam.DCT_CGFlag[ iCG ] = 0;
			m_CoeffCodingParam.DCT_PairsInCG[ iCG ] = 0;
			run = -1;
		}
		run++;
		uiIndexX  = pucScanX[uiScanPos];
		uiIndexY  = pucScanY[uiScanPos];
		uiIndex = uiIndexX+uiIndexY*iWidth;
#if wlq_avs2x_debug
		if (sign)
			printf("%d\n", pcCoef[uiIndex]);
#endif
		if (uiScanPos % 16 == 15) {
			if (pcCoef[ uiIndex ] == 0) {
				m_CoeffCodingParam.DCT_CGLastRun[ iCG ] = run + 1;
			} else {
				m_CoeffCodingParam.DCT_CGLastRun[ iCG ] = 0;
			}
		}

		if( pcCoef[ uiIndex ] )
		{
			m_CoeffCodingParam.level[ipos] = pcCoef[ uiIndex ];
			m_CoeffCodingParam.run[ipos]   = run;
			run = -1;
			ipos++;
			uiNumofCoeff++;
			m_CoeffCodingParam.DCT_CGFlag[ iCG ] = 1;
			m_CoeffCodingParam.DCT_PairsInCG[ iCG ] ++;
		}
	}
#if wlq_avs2x_debug
	//if (sign)
	//	printf("end\n");
	sign = 0;
#endif
}



Void TEncSbac:: codeCoeffinCGLastXAndCGLastY(CoeffCodingParam&  m_CoeffCodingParam,TextType eTType,Int rank, Int numOfCG, Int ctxmode,Int iCG, Int CGx, Int  CGy  )
{
	int xx, yy,symbol;
	const UInt* pucScan  = g_auiFrameScanXY[2];
	const UInt* pucScanX  = g_auiFrameScanX[2];
	const UInt* pucScanY  = g_auiFrameScanY[2];
	xx = pucScanX[15 - m_CoeffCodingParam.DCT_CGLastRun[iCG]];
	yy = pucScanY[15 - m_CoeffCodingParam.DCT_CGLastRun[iCG]];
	if (rank != 0) {
		{
			xx = 3 - xx;
		}
		if (ctxmode != 0) {
			yy = 3 - yy;
		}
	}

	if ((CGx == 0 && CGy > 0 && ctxmode == 2) /*|| (ctxmode == 1)*/) {
		yy = yy ^ xx;
		xx = yy ^ xx;
		yy = xx ^ yy;
	}
	UInt offset,ctx;
#if	CODEFFCODER_LDW_WLQ_YQH_BUG
	if (eTType == TEXT_LUMA) {
		offset = (numOfCG == 1) ? (ctxmode / 2) * 4 : (((CGx > 0 &&
			CGy > 0) ? 0 : ((ctxmode / 2) * 4 + ((iCG == 0) ? 4 : 12))) + 8);
	}
	else {
		offset = (numOfCG == 1) ? 0 : 4;
	}
#else
	if (eTType == TEXT_LUMA) {
		offset = (numOfCG == 4) ? (ctxmode / 2) * 4 : (((CGx > 0 &&
			CGy > 0) ? 0 : ((ctxmode / 2) * 4 + ((iCG == 0) ? 4 : 12))) + 8);
	} else {
		offset = (numOfCG == 4) ? 0 : 4;
	}
#endif
	offset += (rank == 0 ? 0 : ((eTType != TEXT_LUMA) ? NUM_LAST_POS_CTX_CHROMA / 2 : NUM_LAST_POS_CTX_LUMA / 2));
	symbol = xx ;
	ctx = 0;
	while (symbol >= 1) {
		symbol -= 1;

		xEncodeBin(0,m_cTULastPosInCGSCModel.get( 0, (eTType != TEXT_LUMA), offset + ctx ));//
		ctx ++;
		if (ctx >= 2) {
			ctx = 2;
		}
		if (ctx >= 1) {
			ctx = 1;
		}
	}
	if (xx != 3) {
		xEncodeBin(1,m_cTULastPosInCGSCModel.get( 0, eTType != TEXT_LUMA, offset + ctx ));//

	}

	symbol = yy ;

	ctx = 0;
	while (symbol >= 1) {
		symbol -= 1;
		xEncodeBin(0,m_cTULastPosInCGSCModel.get( 0, eTType != TEXT_LUMA, offset + ctx + 2 ));//
		ctx ++;
		if (ctx >= 2) {
			ctx = 2;
		}
		if (ctx >= 1) {
			ctx = 1;
		}
	}
	if (yy != 3) {
		xEncodeBin(1,m_cTULastPosInCGSCModel.get( 0, eTType != TEXT_LUMA, offset + ctx + 2 ));//
	
	}


}

Void TEncSbac:: codeCoeffAbsLevel( Int absLevel,TextType eTType,int pairsInCG, UInt rank,  UInt iCG, int pos   )
{
	int symbol = absLevel - 1;

	int indiv = Min(2, (pairsInCG + 1) / 2);
	// pCTX = Primary[ min(rank, indiv + 2) ];
	int leve_base=Min(rank, indiv + 2)*17;
	int offset = ((iCG == 0 && pos > 12) ? 0 : 3) + indiv + 8;
	if (eTType==TEXT_LUMA) {
		offset += 3;
	}
	if (symbol > 31) {
		int exp_golomb_order = 0;  //需要后续修改
#if wlq_AEC
		xEncodeBinF(1);
#else
		xEncodeBin(1,m_cTULevelRunSCModel.get( 0, eTType!=TEXT_LUMA, 0 ));//
#endif
		symbol = symbol - 32;
		while (1) {
			if ((unsigned int)symbol >= (unsigned int)(1 << exp_golomb_order)) {

				xEncodeBinEP(0);//
				symbol = symbol - (1 << exp_golomb_order);
				exp_golomb_order++;
			} else {
				xEncodeBinEP(1);//
				while (exp_golomb_order--) {   //next binary part
					xEncodeBinEP((unsigned char)((symbol >> exp_golomb_order) & 1));//
				}

				break;
			}
		}
	} else 
	{
#if wlq_AEC
		xEncodeBinF(0);
#else
		//biari_encode_symbol_final(eep_dp, 0);
		xEncodeBin(0,m_cTULevelRunSCModel.get( 0, eTType!=TEXT_LUMA, 0 ));//
#endif
		int bins = 0;
		while (symbol >= 1) {
			symbol -= 1;
			// biari_encode_symbol(eep_dp, 0, pCTX + offset);
			xEncodeBin(0,m_cTULevelRunSCModel.get( 0, eTType!=TEXT_LUMA, leve_base+ offset ));//
			bins++;
		}
#if WLQ_coeff_debug
		if(bins < 31){
			xEncodeBin(1,m_cTULevelRunSCModel.get( 0, eTType!=TEXT_LUMA, leve_base+ offset ));
		}

#else
		xEncodeBin(1,m_cTULevelRunSCModel.get( 0, eTType!=TEXT_LUMA, leve_base+ offset ));
		//if (bins < 31) {
		//	xEncodeBin(1,m_cCUXPosiSCModel.get( 0, 0, 0  ));//
		//}
#endif
	}




}


Void TEncSbac:: codeCoeffCGLastXAndCGLastY(TextType eTType,Int riWidth, Int riHeight, UInt uiAbsPartIdx,Int iCG,Int CGSacnIndex , Int uiIntraModeIdx,Int& CGLastX, Int& CGLastY  )
{
	int numOfCoeff=riWidth*riHeight;
	UInt count = 0;

	if(numOfCoeff==64)
	{
		count = 0;
		while (count < iCG) {
			xEncodeBin(0,m_cTULastCGSCModel.get( 0, eTType != TEXT_LUMA, count ));//
			count++;
		}
		if (iCG < 3) {
			xEncodeBin(1,m_cTULastCGSCModel.get( 0, eTType != TEXT_LUMA, iCG ));//
		}
	}
	if(numOfCoeff==256||numOfCoeff==1024)
	{
		UInt numCGminus1X=(riWidth>>2)-1;
		UInt numCGminus1Y=(riHeight>>2)-1;
		UInt offset;
		if (eTType == TEXT_LUMA && uiIntraModeIdx == INTRA_PRED_DC_DIAG) {
			std:: swap(CGLastX,CGLastY);
			std:: swap(numCGminus1X,numCGminus1Y);
		}
		offset = (eTType != TEXT_LUMA) ? 3 : 9;

		if (CGLastX == 0 && CGLastY == 0) {
			xEncodeBin(0,m_cTULastCGSCModel.get( 0, eTType != TEXT_LUMA, offset ));//
		} else {
			xEncodeBin(1,m_cTULastCGSCModel.get( 0, eTType != TEXT_LUMA, offset ));//
			count = 0;

			offset = (eTType != TEXT_LUMA) ? 4 : 10;
			while (count < CGLastX) {
				xEncodeBin(0,m_cTULastCGSCModel.get( 0, eTType != TEXT_LUMA, offset ));//
				count++;
			}
			if (CGLastX < numCGminus1X) {
				xEncodeBin(1,m_cTULastCGSCModel.get( 0, eTType != TEXT_LUMA, offset ));//
			}

			offset =  (eTType != TEXT_LUMA) ? 5 : 11;
			if (CGLastX == 0) {
				count = 0;
				while (count < CGLastY - 1) {
					xEncodeBin(0,m_cTULastCGSCModel.get( 0, eTType != TEXT_LUMA, offset ));//
					//  biari_encode_symbol(eep_dp, 0, pCTXLastCG + offset);
					count++;
				}
				if (CGLastY < numCGminus1Y) {
					xEncodeBin(1,m_cTULastCGSCModel.get( 0, eTType != TEXT_LUMA, offset ));//
					//  biari_encode_symbol(eep_dp, 1, pCTXLastCG + offset);
				}
			} else {
				count = 0;
				while (count < CGLastY) {
					xEncodeBin(0,m_cTULastCGSCModel.get( 0,eTType != TEXT_LUMA, offset ));//
					//biari_encode_symbol(eep_dp, 0, pCTXLastCG + offset);
					count++;
				}
				if (CGLastY < numCGminus1Y) {
					xEncodeBin(1,m_cTULastCGSCModel.get( 0, eTType != TEXT_LUMA, offset ));//
				}
			}
		}
		if (eTType == TEXT_LUMA && uiIntraModeIdx == INTRA_PRED_DC_DIAG) {
			std:: swap(CGLastX,CGLastY);
		}

	}

}

Void TEncSbac::	codeCoeffRun(Int baseRun,Int absLevel,TextType eTType, int pairs,int numOfCoeff,int pairsInCG, Int ctxmode, UInt rank, UInt iCG, int pos ,CoeffCodingParam  m_CoeffCodingParam)
{
	int absSum5 = 0;
	int n = 0,k;

	for (k = pairs + 1; k <= pairs + pairsInCG; k ++) {
		n += m_CoeffCodingParam.run[k - 1];
		if (n >= 6) {
			break;
		}
		absSum5 += abs(m_CoeffCodingParam.level[k - 1]);
		n ++;
	}

	//	pCTX = Primary[ min((absSum5 + absLevel) / 2, 2) ];
	Int leve_base=Min((absSum5 + absLevel) / 2, 2)*17;
	int ctxpos = 0,moddiv,offset,px,py;
	const UInt* pucScanX  = g_auiFrameScanX[2];
	const UInt* pucScanY  = g_auiFrameScanY[2];

	if (15 - pos > 0) {
		px = pucScanX[15 - pos - 1 - ctxpos];
		py = pucScanY[15 - pos - 1 - ctxpos];
		//#if BBRY_CU8/////how to modify
		moddiv = (ctxmode == INTRA_PRED_VER) ? (py >> 1) : (/*(ctxmode == INTRA_PRED_HOR)?(px >> 1):*/(pos + ctxpos <= 9));
#if		CODEFFCODER_LDW_WLQ_YQH_BUG
		offset = ((iCG == 0) ? (pos + ctxpos == 14 ? 0 : (1 + moddiv)) : ((eTType == TEXT_LUMA) ? 2 : 3 + moddiv)) +
			(numOfCoeff == 16 ? 0 : 3);
		if (eTType == TEXT_LUMA) {
			moddiv = (ctxmode == INTRA_PRED_VER) ? ((py + 1) / 2) : (/*(ctxmode == INTRA_PRED_HOR)?(((px+1)/2)+3):*/((
				pos + ctxpos) > 11 ? 6 : ((pos + ctxpos) > 4 ? 7 : 8)));
			offset = ((iCG == 0) ? (pos + ctxpos == 14 ? 0 : (1 + moddiv % 3)) : (4 + moddiv % 3)) + (numOfCoeff == 16 ? 0 : 4);
		}
#else
		offset = ((iCG == 0) ? (pos + ctxpos == 14 ? 0 : (1 + moddiv)) : ((eTType== TEXT_LUMA) ? 2 : 3 + moddiv)) +
			(numOfCoeff == 64 ? 0 : 3);
		if (eTType== TEXT_LUMA) {
			moddiv = (ctxmode == INTRA_PRED_VER) ? ((py + 1) / 2) : (/*(ctxmode == INTRA_PRED_HOR)?(((px+1)/2)+3):*/((
				pos + ctxpos) > 11 ? 6 : ((pos + ctxpos) > 4 ? 7 : 8)));
			offset = ((iCG == 0) ? (pos + ctxpos == 14 ? 0 : (1 + moddiv % 3)) : (4 + moddiv % 3)) + (numOfCoeff == 64 ? 0 : 4);
		}
#endif
	}

	int symbol = baseRun;
	while (symbol >= 1) {
		symbol -= 1;
		//biari_encode_symbol(eep_dp, 0, pCTX + offset);
		xEncodeBin(0,m_cTULevelRunSCModel.get( 0, eTType!=TEXT_LUMA, leve_base+offset ));//
		ctxpos ++;
		if ((15 - pos - 1 - ctxpos) >= 0) {
			px = pucScanX[15 - pos - 1 - ctxpos];
			py = pucScanY[15 - pos - 1 - ctxpos];
			//#if BBRY_CU8/////how to modify
			moddiv = (ctxmode == INTRA_PRED_VER) ? (py >> 1) : (/*(ctxmode == INTRA_PRED_HOR)?(px >> 1):*/(pos + ctxpos <= 9));
#if		CODEFFCODER_LDW_WLQ_YQH_BUG
			offset = ((iCG == 0) ? (pos + ctxpos == 14 ? 0 : (1 + moddiv)) : (eTType == TEXT_LUMA ? 2 : 3 + moddiv)) +
				(numOfCoeff == 16 ? 0 : 3);
			if (eTType == TEXT_LUMA) {
				moddiv = (ctxmode == INTRA_PRED_VER) ? ((py + 1) / 2) : (/*(ctxmode == INTRA_PRED_HOR)?(((px+1)/2)+3):*/((
					pos + ctxpos) > 11 ? 6 : ((pos + ctxpos) > 4 ? 7 : 8)));
				offset = ((iCG == 0) ? (pos + ctxpos == 14 ? 0 : (1 + moddiv % 3)) : (4 + moddiv % 3)) + (numOfCoeff == 16 ? 0 : 4);
			}
#else
			offset = ((iCG == 0) ? (pos + ctxpos == 14 ? 0 : (1 + moddiv)) : (eTType== TEXT_LUMA ? 2 : 3 + moddiv)) +
				(numOfCoeff == 64 ? 0 : 3);
			if (eTType== TEXT_LUMA) {
				moddiv = (ctxmode == INTRA_PRED_VER) ? ((py + 1) / 2) : (/*(ctxmode == INTRA_PRED_HOR)?(((px+1)/2)+3):*/((
					pos + ctxpos) > 11 ? 6 : ((pos + ctxpos) > 4 ? 7 : 8)));
				offset = ((iCG == 0) ? (pos + ctxpos == 14 ? 0 : (1 + moddiv % 3)) : (4 + moddiv % 3)) + (numOfCoeff == 64? 0 : 4);
			}
#endif
		}
	}

	if (baseRun < 16 - 1 - pos) {
		//biari_encode_symbol(eep_dp, 1, pCTX + offset);
		xEncodeBin(1,m_cTULevelRunSCModel.get( 0, eTType!=TEXT_LUMA, leve_base+offset ));//
	}



}


Void TEncSbac:: codeCoeff( TComDataCU* pcCU,TCoeff* pcCoef,UInt uiAbsPartIdx,Int CGSacnIndex ,Int iWidth, Int iHeight,TextType eTType,  UInt uiNumofCoeff, CoeffCodingParam&  m_CoeffCodingParam)
{
	if(uiNumofCoeff)
	{
		UInt numOfCG=(iWidth*iHeight)>>4;
		UInt numOfCoeffInCG = 16;
		UInt rank = 0;
		int CGx=0,CGy=0;
		int CGLastX=0,CGLastY=0;
		int sigCGctx,pos,pairsInCG;
		int pairs=uiNumofCoeff;
		int numCGminus1 = numOfCG - 1;
		int count = 0;
		int numCGminus1X=(iWidth>>2)-1;
		int numCGminus1Y=(iHeight>>2)-1;
		int Level_sign[17] = { -1};
		int T_Chr[5] = { 0, 1, 2, 4, 3000};
		PredMode m_pePreMode = pcCU->getPredictionMode(uiAbsPartIdx);
		Int ctxmode=INTRA_PRED_DC_DIAG;
		if(eTType==TEXT_LUMA&&(m_pePreMode == MODE_INTRA))
		{
			ctxmode = g_auiIntraModeClassified[ pcCU->getIntraDir(uiAbsPartIdx)];
			if (ctxmode == INTRA_PRED_HOR) 
				ctxmode = INTRA_PRED_VER;
		}

		for (Int iCG = numOfCG - 1; iCG >= 0; iCG --) 
		{
			if( numOfCG>1) 
				getCGposXAndposY(iWidth, iHeight,iCG,CGSacnIndex ,  CGx, CGy  );
			if (rank == 0 && m_CoeffCodingParam.DCT_CGFlag[ iCG ] && numOfCG>1) 
			{
				// getCGposXAndposY(iWidth, iHeight,iCG,CGSacnIndex ,  CGx, CGy  );
				codeCoeffCGLastXAndCGLastY(eTType, iWidth, iHeight, uiAbsPartIdx,iCG,CGSacnIndex ,ctxmode, CGx,  CGy  );
			}
			//! Sig CG Flag
			if (rank > 0) {
				//getCGposXAndposY(iWidth, iHeight,iCG,CGSacnIndex ,  CGx, CGy  );
				sigCGctx = eTType != TEXT_LUMA ? 0 : ((iCG == 0) ?  0 : 1);
				xEncodeBin(m_CoeffCodingParam.DCT_CGFlag[ iCG ],m_cTUSigCGSCModel.get( 0, eTType!=TEXT_LUMA, sigCGctx ));//
			}
			//! (Run, Level)
			if (m_CoeffCodingParam.DCT_CGFlag[ iCG ]) {
				pos  = 0;
				pairsInCG = 0;
				Int baseLevel , baseRun,absLevel;
				for (UInt i = m_CoeffCodingParam.DCT_PairsInCG[ iCG ]; i >= 0; i--, pairs--, pairsInCG++) {

					//pCTX = Primary[rank];

					if (i == m_CoeffCodingParam.DCT_PairsInCG[ iCG ]) 
					{
						codeCoeffinCGLastXAndCGLastY( m_CoeffCodingParam, eTType, rank, numOfCG, ctxmode, iCG, CGx,  CGy  );
						pos += m_CoeffCodingParam.DCT_CGLastRun[iCG];
					}

					if (pos == 16) {
						break;
					}

					if (i == 0) {
						baseLevel = 0;
						baseRun = 0;
					} else {
						baseLevel = m_CoeffCodingParam.level[pairs - 1];
						baseRun = m_CoeffCodingParam.run[pairs - 1];
					}
					absLevel = abs(baseLevel);

					codeCoeffAbsLevel( absLevel,eTType,pairsInCG, rank,  iCG, pos   );

					codeCoeffRun( baseRun,absLevel,eTType,pairs,(numOfCG<<4),pairsInCG, ctxmode,rank,  iCG, pos ,m_CoeffCodingParam);
					if (baseLevel > 0) {
						Level_sign[i] = 0;
					} else {
						Level_sign[i] = 1;
					}

					//! Update Rank
					if (absLevel > T_Chr[rank]) {
						if (absLevel <= 2) {
							rank = absLevel;
						} else if (absLevel <= 4) {
							rank = 3;
						} else {
							rank = 4;
						}
					}

					if (baseRun == 16 - 1 - pos) {
						pairs --;
						pairsInCG ++;
						break;
					}

					//! Update position
					pos += (baseRun + 1);
				}
				for (int i = m_CoeffCodingParam.DCT_PairsInCG[ iCG ]; i > 0; i--) {
					// biari_encode_symbol_eq_prob(eep_dp, Level_sign[i]);
					xEncodeBinEP(Level_sign[i]);//
				}
			}
		}
	}
	//  DCT_Pairs = 0;
}
#if !niu_NSQT_dec
#if RD_NSQT
Void TEncSbac::getTUSize(UInt uiPartIdx, Int& riWidth, Int& riHeight, Int uiLog2Size, PartSize m_pePartSize, Bool bNSQT)
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
#if ZY_INTRA_MODIFY_TUSIZE
		riWidth = (1 << (uiLog2Size + 1));      riHeight = (1 << (uiLog2Size - 1));
#else
		riWidth = (1 << uiLog2Size);      riHeight = (1 << uiLog2Size) >> 2;
#endif
		break;
	case SIZE_hNx2N: //yuquanhe@hisilicon.com
		if (uiLog2Size == 5)
			uiLog2Size--;
#if ZY_INTRA_MODIFY_TUSIZE
		riWidth = (1 << (uiLog2Size - 1));      riHeight = (1 << (uiLog2Size + 1));
#else
		riWidth = (1 << uiLog2Size) >> 2;     riHeight = (1 << uiLog2Size);
#endif
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
#endif
Void TEncSbac::getTUSize( UInt uiPartIdx,  Int& riWidth, Int& riHeight,Int uiLog2Size,  PartSize m_pePartSize  )
{
	switch ( m_pePartSize)
	{
	case SIZE_2Nx2N:
		riWidth = (1<<uiLog2Size);      riHeight = (1<<uiLog2Size);      
		break;
		//case SIZE_2NxN:
		//	riWidth = (1<<uiLog2Size);      riHeight = (1<<uiLog2Size);   
		//	break;
#if	!ZHANGYI_INTRA_SDIP_BUG_YQH
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
#if ZY_INTRA_MODIFY_TUSIZE
	case SIZE_NxN:
		riWidth = (1 << uiLog2Size); riHeight = (1 << uiLog2Size);
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
	default:
		riWidth = (1<<uiLog2Size);      riHeight = (1<<uiLog2Size);    
		//	assert (0);
		break;
	}
}

Void TEncSbac:: getCoeffScanAndCGSacn(Int& CoeffScanIndex, Int& CGSacnIndex , Int riWidth, Int riHeight )
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


	printf("error TU riWidth==%d riHeight=%d",riWidth,riHeight);
}

#endif


Void TEncSbac::xCheckCoeff( TCoeff* pcCoef, UInt uiSize, UInt uiDepth, UInt& uiNumofCoeff, UInt& uiPart )
{
  UInt ui = uiSize>>uiDepth;
  if( uiPart == 0 )
  {
    if( ui <= 2 )
    {
      UInt x, y;
      TCoeff* pCeoff = pcCoef;
      for( y=0 ; y<ui ; y++ )
      {
        for( x=0 ; x<ui ; x++ )
        {
          if( pCeoff[x] != 0 )
          {
            uiNumofCoeff++;
          }
        }
        pCeoff += uiSize;
      }
    }
    else
    {
      xCheckCoeff( pcCoef,                            uiSize, uiDepth+1, uiNumofCoeff, uiPart ); uiPart++; //1st Part
      xCheckCoeff( pcCoef             + (ui>>1),      uiSize, uiDepth+1, uiNumofCoeff, uiPart ); uiPart++; //2nd Part
      xCheckCoeff( pcCoef + (ui>>1)*uiSize,           uiSize, uiDepth+1, uiNumofCoeff, uiPart ); uiPart++; //3rd Part
      xCheckCoeff( pcCoef + (ui>>1)*uiSize + (ui>>1), uiSize, uiDepth+1, uiNumofCoeff, uiPart );           //4th Part
    }
  }
  else
  {
    UInt x, y;
    TCoeff* pCeoff = pcCoef;
    for( y=0 ; y<ui ; y++ )
    {
      for( x=0 ; x<ui ; x++ )
      {
        if( pCeoff[x] != 0 )
        {
          uiNumofCoeff++;
        }
      }
      pCeoff += uiSize;
    }
  }
}

Void TEncSbac::xWriteMvd( Int iMvd, UInt uiCtx )
{
  UInt uiSign = 0;
  if ( 0 > iMvd )
  {
    uiSign = 1;
    iMvd   = -iMvd;
  }

  xWriteUnarySymbolSbac(iMvd, &m_cCUMvdSCModel.get( 0, uiCtx, 0 ), 5);

  // Sign coding only when MVD is non-zero.
  if ( iMvd )
  {
    xEncodeBinEP( uiSign );
  }
  return;
}

Void TEncSbac::xWriteCodeVlc ( UInt uiCode, UInt uiLength )
{
  assert ( uiLength > 0 );
  m_pcBitIf->write( uiCode, uiLength );
}

Void TEncSbac::xWriteUvlc     ( UInt uiCode )
{
#if wlq_vlc
	Int  len,info;
	UInt bitpattern;
	xUeLinfo(uiCode, &len, &info);
	xSymbol2Uvlc(len,info,&bitpattern);
	m_pcBitIf->write( bitpattern, len );
#else
  UInt uiLength = 1;
  UInt uiTemp = ++uiCode;
  
  assert ( uiTemp );
  
  while( 1 != uiTemp )
  {
    uiTemp >>= 1;
    uiLength += 2;
  }
  
  m_pcBitIf->write( uiCode, uiLength );
  #endif
}

Void TEncSbac::xWriteSvlc     ( Int iCode )
{
#if wlq_vlc
	Int  len,info;
	UInt bitpattern;
	xSeLinfo(iCode, &len, &info);
	xSymbol2Uvlc(len,info,&bitpattern);
	m_pcBitIf->write( bitpattern, len );
#else
  UInt uiCode;
  
  uiCode = xConvertToUInt( iCode );
  xWriteUvlc( uiCode );
  #endif
}

#if wlq_vlc
Void  TEncSbac::xUeLinfo  ( UInt uiCode ,Int* len, Int* info)
{
	Int i, nn;

	nn = (uiCode + 1) / 2;

	for (i = 0; i < 16 && nn != 0; i++) {
		nn /= 2;
	}

	*len = 2 * i + 1;
	*info = uiCode + 1 - (Int) pow(2.0, i);
}

Void  TEncSbac::xSeLinfo  ( Int iCode ,Int* len, Int* info )
{

	Int i, n, sign, nn;

	sign = 0;

	if (iCode <= 0) {
		sign = 1;
	}

	n = abs(iCode) << 1;

	//n+1 is the number in the code table.  Based on this we find length and info

	nn = n / 2;

	for (i = 0; i < 16 && nn != 0; i++) {
		nn /= 2;
	}

	*len = i * 2 + 1;
	*info = n - (Int) pow(2.0, i) + sign;

}
Void TEncSbac::xSymbol2Uvlc          ( Int len, Int info, UInt* bitpattern)
{
	int suffix_len = len / 2;
	*bitpattern = (1 << suffix_len) | (info & ((1 << suffix_len) - 1));
}
#endif
Void TEncSbac::xWriteFlagVlc( UInt uiCode )
{
  m_pcBitIf->write( uiCode, 1 );
}

Void TEncSbac::xEncodeBin( UInt uiSymbol, ContextModel &rcCtxModel )
{
#if wlq_AEC
	UChar cycno = rcCtxModel.getCycno();
	UInt low = m_Elow;
	UChar cwr = 0;
	UInt lg_pmps = rcCtxModel.getLG_PMPS();
	UInt t_rLPS = 0;
	UChar s_flag = 0, is_LPS = 0;
	UInt  curr_byte = 0;
	short int bitstogo = 0;
	UChar bit_o = 0, bit_oa = 0, byte_no = 0;
	UInt low_byte[3] = {0};

	m_s1 = m_E_s1;
	m_t1 = m_E_t1;
#if !wlq_AEC_r
	assert(eep != NULL);
#endif

	cwr = (cycno <= 1) ? 3 : (cycno == 2) ? 4 : 5;

	if (uiSymbol != 0) {
		uiSymbol = 1;
	}

	if (m_t1 >= (lg_pmps >> LG_PMPS_SHIFTNO_AVS2)) {
		m_s2 = m_s1;
		m_t2 = m_t1 - (lg_pmps >> LG_PMPS_SHIFTNO_AVS2);
		s_flag = 0;
	} else {
		m_s2 = m_s1 + 1;
		m_t2 = 256 + m_t1 - (lg_pmps >> LG_PMPS_SHIFTNO_AVS2);
		s_flag = 1;
	}

	if (uiSymbol == rcCtxModel.getMPS()) {   //MPS happens

		if (cycno == 0) {
			cycno = 1;
		}

		m_s1 = m_s2;
		m_t1 = m_t2;

		//no updating of interval range and low here or
		//renorm to guarantee s1<8, after renorm, s1 --;

		if (m_s1) { //renorm
			//left shift 1 bit
			bit_o = (low >> 9) & 1;
			{
				bit_oa = (low >> 8) & 1;
				if (bit_o) {
					m_pcBitIf->put_one_bit_plus_outstanding(1,&m_Ebits_to_follow);
				} else {
					if (!bit_oa) {   //00
						m_pcBitIf->put_one_bit_plus_outstanding(0,&m_Ebits_to_follow);
					} else { //01
						m_Ebits_to_follow++;
						bit_oa = 0;
					}
				}
				m_s1--;
			}
			//restore low
			low = ((bit_oa << 8) | (low & 0xff)) << 1;
		}
	} else { //--LPS
		is_LPS = 1;
		cycno = (cycno <= 2) ? (cycno + 1) : 3;

		if (s_flag == 0) {
			t_rLPS = lg_pmps >> LG_PMPS_SHIFTNO_AVS2;  //t_rLPS -- 9bits
		} else { //s2=s1 + 1
			t_rLPS = m_t1 + (lg_pmps >> LG_PMPS_SHIFTNO_AVS2);    //t_rLPS<HALF
		}

		low_byte[0] = low + ((m_t2 + 256) >> m_s2);     //first low_byte: 10bits
		low_byte[1] = (m_t2 << (8 - m_s2)) & 0xff;

		//restore range
		while (t_rLPS < QUARTER_AVS2) {
			t_rLPS = t_rLPS << 1;
			m_s2 ++;
		}
		//left shift s2 bits
		{
			curr_byte = low_byte[0];
			bitstogo = 9;
			bit_oa = (curr_byte >> bitstogo) & 1;
			byte_no = 0;

			while (m_s2 > 0) {
				bit_o = bit_oa;
				bitstogo--;

				if (bitstogo < 0) {
					curr_byte = low_byte[++byte_no];
					bitstogo = 7;
				}

				bit_oa = (curr_byte >> bitstogo) & 1;

				if (bit_o) {
					m_pcBitIf->put_one_bit_plus_outstanding(1,&m_Ebits_to_follow);
				} else {
					if (!bit_oa) {   //00
						m_pcBitIf->put_one_bit_plus_outstanding(0,&m_Ebits_to_follow);
					} else { //01
						m_Ebits_to_follow++;
						bit_oa = 0;
					}
				}

				m_s2--;
			}

			if (bitstogo >= 1) {
				low = ((low_byte[0] << (9 - bitstogo)) | (low_byte[1] >> (bitstogo - 1)));
				low = low & ((bit_oa << 9) | 0x1ff);
			} else {
				low = ((low_byte[0] << (9)) | (low_byte[1] << (1)));
				low = low & ((bit_oa << 9) | 0x1ff);
			}
		}
		m_s1 = 0;
		m_t1 = t_rLPS & 0xff;
	}

	//updating other parameters
	rcCtxModel.setCycno(cycno);
	m_Elow = low;
	m_E_s1 = m_s1;
	m_E_t1 = m_t1;

	//update probability estimation
	if (is_LPS) {
		switch (cwr) {
		case 3:
			lg_pmps = lg_pmps + 197;
			break;
		case 4:
			lg_pmps = lg_pmps + 95;
			break;
		default:
			lg_pmps = lg_pmps + 46;
		}

		if (lg_pmps >= (256 << LG_PMPS_SHIFTNO_AVS2)) {
			lg_pmps = (512 << LG_PMPS_SHIFTNO_AVS2) - 1 - lg_pmps;
			rcCtxModel.setMPS(!(rcCtxModel.getMPS()));
		}
	} else {
		lg_pmps = lg_pmps - (unsigned int)(lg_pmps >> cwr) - (unsigned int)(lg_pmps >> (cwr + 2));
	}

	rcCtxModel.setLG_PMPS(lg_pmps);
#else
	UInt uiCode    = m_uiCode;
	UInt uiRange  = m_uiRange;
  UInt uiLPS;
  UChar ucNextStateLPS, ucNextStateMPS;

  uiLPS = TComCABACTables::sm_auiLPSTable[rcCtxModel.getState()];
  ucNextStateLPS = uiLPS & 0xFF;
  uiLPS >>= 8;
  ucNextStateMPS = uiLPS & 0xFF;
  uiLPS >>= 8;

  uiRange -= uiLPS;
  if( uiSymbol != rcCtxModel.getMps() ) 
  {
    if ( uiRange >= uiLPS) 
    {
      uiCode += uiRange;
      uiRange = uiLPS;
    }
    rcCtxModel.setStateMps(ucNextStateLPS);
  }
  else 
  {
    if ( uiRange >= 0x8000 ) 
    {
      m_uiRange = uiRange;
      m_uiCode = uiCode;
      return;
    }
    if ( uiRange < uiLPS ) 
    {
      uiCode += uiRange;
      uiRange = uiLPS;
    }
    rcCtxModel.setStateMps(ucNextStateMPS);
  }
  do 
  {
    uiRange <<= 1;
    uiCode <<= 1;
    if (--m_uiCodeBits == 0) 
    {
      xCarryPropagate( uiCode );
      m_uiCodeBits = 8;
    }
  } while (uiRange < 0x8000);

  m_uiRange = uiRange;
  m_uiCode = uiCode;

  return;
  #endif
}

Void TEncSbac::xEncodeBinEP( UInt uiSymbol )
{
#if wlq_AEC
	UChar bit_o = 0, bit_oa = 0;
	UInt low_byte[3] = {0};
	UInt low = m_Elow;
	m_t1 = m_E_t1;
	if (uiSymbol != 0) { //LPS
		//update low
		low_byte[0] = low + ((m_t1 + 0x100) >> 1);     //first low_byte: 10bits
		low_byte[1] = (m_t1 << 7) & 0xff;
	}

	else { //MPS
		//LOW is same
		low_byte[0] = low; //first low_byte: 10bits
		low_byte[1] = 0;
	}

	bit_o = (low_byte[0] >> 9) & 1;
	bit_oa = (low_byte[0] >> 8) & 1;
	//out bit
	if (bit_o) {
		m_pcBitIf->put_one_bit_plus_outstanding(1,&m_Ebits_to_follow);
	} else {
		if (!bit_oa) {   //00
			m_pcBitIf->put_one_bit_plus_outstanding(0,&m_Ebits_to_follow);
		} else { //01
			m_Ebits_to_follow++;
			bit_oa = 0;
		}
	}
	//update low
	low = ((((bit_oa << 8) | (low_byte[0] & 0xff)) << 1) | ((low_byte[1] >> 7) & 1));
	m_Elow = low;
#else
	UInt uiCode    = m_uiCode;
	UInt uiRange   = m_uiRange >> 1;

  if( uiSymbol != 0 ) 
  {
    uiCode += uiRange;
  }

  uiRange <<= 1;
  uiCode <<= 1;
  if (--m_uiCodeBits == 0) 
  {
    xCarryPropagate( uiCode );
    m_uiCodeBits = 8;
  }

  m_uiRange = uiRange;
  m_uiCode = uiCode;

  return;
  #endif
}
#if wlq_AEC
Void TEncSbac::xEncodeBinF        ( UInt  uiSymbol )
{
	UChar bit_o = 0, bit_oa = 0, byte_no = 0;
	UInt low_byte[3] = {0};
	UInt  curr_byte = 0;
	short int bitstogo = 0;
	UInt low = m_Elow;
	UInt t_rLPS = 0;
	UInt s_flag = 0;
	m_t1 = m_E_t1;
	if (m_t1) {  // get s2,t2
		m_s2 = m_s1;
		m_t2 = m_t1 - 1;
		s_flag = 0;
	} else {
		m_s2 = m_s1 + 1;
		m_t2 = 255;
		s_flag = 1;
	}
	if (uiSymbol) { //LPS //rLPS=1

		low_byte[0] = low + ((m_t2 + 256) >> m_s2);     //first low_byte: 10bits
		low_byte[1] = (m_t2 << (8 - m_s2)) & 0xff;
		m_s2 += 8;
		curr_byte = low_byte[0];
		bitstogo = 9;
		bit_oa = (curr_byte >> bitstogo) & 1;
		byte_no = 0;
		while (m_s2 > 0) {
			bit_o = bit_oa;
			bitstogo--;
			if (bitstogo < 0) {
				curr_byte = low_byte[++byte_no];
				bitstogo = 7;
			}
			bit_oa = (curr_byte >> bitstogo) & 1;

			if (bit_o) {
				m_pcBitIf->put_one_bit_plus_outstanding(1,&m_Ebits_to_follow);
			} else {
				if (!bit_oa) {   //00
					m_pcBitIf->put_one_bit_plus_outstanding(0,&m_Ebits_to_follow);
				} else { //01
					m_Ebits_to_follow++;
					bit_oa = 0;
				}
			}

			m_s2--;
		}
		//restore low

		if (bitstogo >= 1) {
			low = ((low_byte[0] << (9 - bitstogo)) | (low_byte[1] >> (bitstogo - 1)));
			low = low & ((bit_oa << 9) | 0x1ff);
		} else {
			low = ((low_byte[0] << (9)) | (low_byte[1] << (1)));
			low = low & ((bit_oa << 9) | 0x1ff);
		}
		m_t1 = 0;
		m_s1 = 0;
	} else { //MPS
		m_s1 = m_s2;
		m_t1 = m_t2;
		if (m_s1) {
			m_s1--;
			m_t1 = 255; //(255<<1)|0
			bit_o = (low >> 9) & 1;
			bit_oa = (low >> 8) & 1;
			if (bit_o) {
				m_pcBitIf->put_one_bit_plus_outstanding(1,&m_Ebits_to_follow);

			} else {
				if (!bit_oa) {   //00
					m_pcBitIf->put_one_bit_plus_outstanding(0,&m_Ebits_to_follow);

				} else { //01
					m_Ebits_to_follow++;

					bit_oa = 0;
				}

			}
			//restore low
			low = ((bit_oa << 8) | (low & 0xff)) << 1;
		}
	}
	m_Elow = low;
	m_E_t1 = m_t1;
	m_E_s1 =  m_s1;
}
#endif
Void TEncSbac::xPutByte( UChar ucByte)
{
  if (m_bIsPendingByte) {
    if (m_ucPendingByte == 0) {
      m_uiStackedZeros++;
    }
    else {
      while (m_uiStackedZeros > 0) {
        m_pcBitIf->write(0x00, 8);
        m_uiStackedZeros--;
      }
      m_pcBitIf->write(m_ucPendingByte, 8);
    }
  }
  m_ucPendingByte = ucByte;
  m_bIsPendingByte = true;
}

Void TEncSbac::xCarryPropagate( UInt& ruiCode )
{
  UInt uiOutBits = ruiCode >> 19;
  ruiCode &= 0x7FFFF;

  if (uiOutBits < 0xFF) {
    while (m_uiStackedFFs !=0 ) {
      xPutByte(0xFF);
      m_uiStackedFFs--;
    }
    xPutByte(uiOutBits);
  }
  else if (uiOutBits > 0xFF) {
    m_ucPendingByte++;
    while (m_uiStackedFFs !=0 ) {
      xPutByte(0x00);
      m_uiStackedFFs--;
    }
    xPutByte(uiOutBits & 0xFF);
  }
  else {
    m_uiStackedFFs++;
  }
}
