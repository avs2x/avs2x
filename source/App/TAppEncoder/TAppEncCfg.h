/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  
 *
 * Copyright (c) 2010-2012, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TAppEncCfg.h
    \brief    Handle encoder configuration parameters (header)
*/

#ifndef __TAPPENCCFG__
#define __TAPPENCCFG__
#include "../../Lib/TLibCommon/CommonDef.h"
#if RPS
#include "../../Lib/TLibEncoder/TEncCfg.h"
#endif
// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// encoder configuration class
class TAppEncCfg
{
protected:
 
  // source specification
  Bool      m_bUsePAD;                                        ///< flag for using source padding
  Int       m_aiPad[2];                                       ///< number of padded pixels for width and height
  
  // coding structure
  Int       m_iGOPSize;                                       ///< GOP size of hierarchical structure
  Int       m_iRateGOPSize;                                   ///< GOP size for QP variance
  Bool      m_bUseLDC;                                        ///< flag for using low-delay coding mode
  Bool      m_bUseNRF;                                        ///< flag for using non-referenced frame in hierarchical structure
  Bool      m_bUseGPB;                                        ///< flag for using generalized P & B structure

  // coding quality
  Double    m_fQP;                                            ///< QP value of key-picture (floating point)
  Int       m_iQP;                                            ///< QP value of key-picture (integer)
  Double    m_fCQP;                                           ///< Chroma QP value of key-picture (floating point)
  Int       m_iCQP;                                           ///< Crhoma QP value of key-picture (integer)
  char*     m_pchdQPFile;                                     ///< QP offset for each picture (initialized from external file)
  Int*      m_aidQP;                                          ///< array of picture QP values

  // coding unit (CU) definition
  UInt      m_uiMaxCUSize;                                    ///< max. CU size in pixel
  UInt      m_uiLog2MaxCUSize;                                ///< log2 of max. CU size in pixel
  UInt      m_uiMaxCUDepth;                                   ///< max. CU depth
  Bool      m_bUseFastEnc;                                    //?
  // transform unit (TU) definition
  UInt      m_uiLog2MaxPhTrSize;
  
  // coding tools (encoder-only parameters)
  Bool      m_bUseHADME;                                      ///< flag for using HAD in sub-pel ME
  Bool      m_bUseBQP;                                        ///< flag for using B-picture based QP assignment in low-delay hier. structure
  Int       m_iFastSearch;                                    ///< ME mode, 0 = full, 1 = diamond, 2 = PMVFAST

#if ENUM_AND_MACRO_YUQUANHE 
  ///<AVS2 RD cfg
  // Files
  char*     m_pchInputFile;             //no20                         ///< source file name
  Int       m_iFrameSkip;               //no24      FrameSkipNums      ///< number of skipped frames from the beginning
  Int       m_iFrameToBeEncoded;        //no7                          ///< number of encoded frames
  Int       m_iSourceWidth;              //no15                        ///< source width in pixel
  Int       m_iSourceHeight;             //no16                        ///< source height in pixel
  UInt      m_uiInputSampleBitDepth;      //no102                      /// Source bit depth (8 bit or 10 bit for all components)
  UInt      m_uiBitDepth;                 //no101                    ///Internal coding bit depth
  char*     m_pchTraceFile;               //no26
  char*     m_pchReconFile;               //no21                     ///< output reconstruction file
  char*     m_pchBitstreamFile;           //no25                     ///< output bitstream file .avs

  //Encoder Control
  Int       m_iProfileId;			//No2					    // bitstream 
  Int	      m_iLevelId;             //No3                       // bitstream
  Int       m_iIntraPeriod;         //No4                       //Period of I-Frames (0=only first)
  Int       m_iVECPeriod;           //no5                       // Period of VEC, distance between two sequence headers   (0=doesn't exist at the bitstream)
  Int       m_iSeqHeaderPeriod;      //no6		                // Period of Sequence Header, Number of IntraPeriod  (0=only first)
  Int       m_iQPIFrame;             //no8		                // Quant. param for first frame (intra) (0-63)
  Int		    m_iQPPFrame;             //no9		                // Quant. param for remaining frames (0-63)
  Int       m_iQPBFrame;             //no35                    // Quant. param for B frames (0-63)
  Bool      m_bUseHadamard;          //no11			           // Hadamard transform
  Bool      m_bFME;                   //NO12                  //Fast Motion Estimation (0=disable, 1=UMHexagonS)
  Int       m_iSearchRange;           //no13                  // Max search range
  Int       m_iNumberReferenceFrames;  //no14                 // Number of previous frames used for inter motion search (1-5)
  Bool      m_bInterSearch16x16;         //no36              //Inter block search 16x16 (0=disable, 1=enable)
  Bool      m_bInterSearch16x8;           //no37			 //Inter block search 16x8  (0=disable, 1=enable)
  Bool      m_bInterSearch8x16;           //no38			 //Inter block search  8x16 (0=disable, 1=enable)
  Bool      m_bInterSearch8x8;             //no39			 //Inter block search  8x8  (0=disable, 1=enable) 
  Bool      m_bUseASR;                     //no40?   AMP      ///< flag for using adaptive motion search range
  Bool      m_bPicExtensionData;            //no41            //Encode picture level extension data (copyright, picture_display, camera_parameters)  
  Bool      m_bOutPutEncPic;                 //no85
  Bool      m_bInterSearchAMP;

  //F Frames
  Bool      m_bFFrameEnable;              //no28        // (0: Don't use F frames  1:Use F frames instead of P frames)
  Bool      m_bDHPEnable;                 //no29		// (0: Don't use DHP,      1:Use DHP) 
  Bool      m_bMHPSkipEnable;             //no30		// (0: Don't use MH_PSKIP, 1:Use MH_PSKIP) 
  Bool      m_bWSMEnable;                 //no32        // (0: Don't use WSM,      1:Use WSM) 

  //B Frames
  Int       m_iNumberBFrames;             //no33       // Number of B frames inserted
  Int       m_iPFrameSkip;                //no10         //Number of frames to be skipped in input (e.g 2 will code every third frame)     
  Bool      m_bHierarchicalCoding;        //no34          ///< flag for specifying hierarchical B structure
#if IDR
  Bool      m_bIDREnable;
#endif
  //RD Optimization
  Bool      m_bRDOptimization;              //no42         //rd-optimized mode decision (0:off, 1:on)

  //High Level Syntax
  Bool      m_bProgressiveSequence;        //no56       //(0: Progressive and/or Interlace, 1: Progressive Only)
  Bool      m_bProgressiveFrame;           //no57	    //(0: Interlace, 1: Progressive) 
  Int       m_iInterlaceCodingOption;       //no43      //(0: frame coding, 1: Field coding, 2: PAFF, 3: Field picture coding)
  Int       m_iRepeatFirstField;             //NO44
  Int       m_iTopFieldFirst;                //no45       
  Bool      m_bHDRMetaDataExtension;          //no47   //(0: Don't transmit metadata; 1: transmit metadata, the parameters need to be modified inside the code)

  //Coding Tools
  Bool      m_bPMVREnable;                        //no80       // coding tool
  Bool      m_bNSQT;                              //no81       // coding tool
  Bool      m_bSDIP;                              //no83
  Bool      m_bSECTEnable;                        //no100       //tool
  Bool      m_bDMHEnableEncoder;                 //no31         //turn on in encoder RDO dcision

  //Loop filter parameters
  Bool      m_bCrossSliceLoopFilter;              //no55     //Enable Cross Slice Boundary Filter (0=Disable, 1=Enable)
  Bool      m_bLoopFilterDisable;             //no48           //Disable loop filter in picture header (0=Filter, 1=No Filter)
  Bool      m_bLoopFilterParameter;            //no49           Send loop filter parameter (0= No parameter, 1= Send Parameter) 
  Int       m_iLoopFilterAlphaOffset;           //no50			 Aplha offset in loop filter  
  Int       m_iLoopFilterBetaOffset;             //no51			 Beta offset in loop filter
  Bool      m_bSAOEnable;                        //no52			 Enable SAO (1=SAO on, 0=SAO OFF)
  Bool      m_bALFEnable;                        //no53			 Enable ALF (1=ALF on, 0=ALF OFF)
  Bool      m_bALFLowLatencyEncodingEnable;       //no54 		 Enable Low Latency ALF (1=Low Latency ALF, 0=High Efficiency ALF)

  //Slice parameters
  Int       m_iNumberOfLCUsInSlice;               //no58		 number of LCUs in one Slice  (0: one slice per frame)

  //Frame Rate
  Int       m_iFrameRate;                         //no60                     ///< source frame-rates (Hz)

  //chroma format parameters
  Int       m_iChromaFormat;                      //no61   (0=4:0:0,1=4:2:0,2=4:2:2) 
  Int       m_iYUVStructure;                      //no62   (0=YUV,1=U0Y0 V0Y1)

  //Frequency Weighting Quantization
  Bool      m_bWQEnable;                          //no63    //Frequency Weighting Quantization (0=disable,1=enable)
  Bool      m_bSeqWQM;                            //no64
  char*     m_pchSeqWQFile;                       //no65
  Bool      m_bPicWQEnable;                       //no66
  Int       m_iPicWQDataIndex;                    //no74
  char*     m_pchPicWQFile;                       //no75
  Int       m_iWQParam;                           //no67
  Int       m_iWQModel;                           //no68
  char*     m_pchWeightParamDetailed;             //no69
  char*     m_pchWeightParamUnDetailed;           //no70
  Bool      m_bChromaDeltaQPDisable;              //no71
  Int       m_iChromaDeltaU;                      //no72
  Int       m_iChromaDeltaV;                      //no73
  Bool      m_bChromaHDRDeltaQPDisable;           //no95         // 1:HDR Chroma Delta QP disable; 0: Chroma HDR Delta QP enable
  Int       m_iChromaQPScale;                     //no96	     // Linear chroma QP offset mapping (scale) based on QP /100
  Int       m_iChromaQPOffset;                    //no97	      // Linear chroma QP offset mapping (offset) based on QP
  Int       m_iCbQPScale;                         //no98	      // Scale factor depending on capture and representation color space
  Int       m_iCrQPScale;                         //no99	      // Scale factor depending on capture and representation color space 

#if RPS
  GOPEntry  m_GOPList[MAX_GOP];                               ///< the coding structure entries from the config file
  Int       m_iExtraRPSs;                                     ///< extra RPSs added to handle CRA
#endif

  //Delta QP and Adaptive Quantization
  Bool      m_bDeltaQP;                           //no82    Enable dqp

  //ROI
  char*     m_pchInputROIDataFile;       //no22      //Input position data
  Int       m_iROICoding;                 //no23     //ROI coding scheme (0=disable, 1=differential-coding scheme)

  //Background Prediction
  Bool      m_bBackgroundEnable;                  //no86
  char*     m_pchBGFileName;                      //no87
  char*     m_pchBackgroundRefFile;               //no88
  Int       m_iBGInputNumber;                     //no89
  Int       m_iBackgroundPeriod;                  //no90
  Int       m_iModelNumber;                       //no91
  Int       m_iBackgroundQP;                      //no92
  Int       m_iModelMethod;                       //no93

  //3D Packing Mode
  Int       m_iTDMode;                  //no17         //(0: Ordinary 2D Video, 1: 3D Video)   
  Int       m_iViewPackingMode;         //no18         //(0: SbS, 1: TaB, 2: 4 view, 3-255: user define)  
  Int       m_iViewReverse;              //no19        //(0: default view order, 1: reversed view order)  

  //unclear  parameter
  Int       m_iOutputMergedPicture;           //no46
  Int       m_iTemporalScalableFlag;         //no27
  //delete
  Int       m_iBGFastMode;                        //no94
  Int       m_iSliceParameter;                    //no59

  // Encoder Optimization Tools//
  //RDOQ
  Bool      m_bUseRDOQ;                           //no76          RDOQEnable    ///< flag for using RD optimized quantization
  Int       m_iLambdaFactor;                      //no77
  Int       m_iLambdaFactorP;                     //no78
  Int       m_iLambdaFactorB;                     //no79

  //Refine QP  (Only used at RA configuration)
  Bool      m_bRefineQP;                          //no84 

  //TDRDO (only used at LD configuration)
  Bool      m_bTDRDOEnable;                       //no103

  //AQPO
  Bool      m_bAQPOEnable;                        //no104

  //RATECONTROL
  Bool      m_bRateControl;                       //no105        //(0: Disable, 1: Enable. if DeltaQP=1 will perform LCU level rate control)
  Int       m_iTargetBitRate;                     //no106		 //(target bit-rate, default 0£ºin bps, 1000000 means 1 mbps)
  Int       m_iRCInitialQP;                       //no107		 //(initial QP, default 0: QPIFrame)

  //MD5
  Bool      m_bMD5Enable;                         //no108

#else
// file I/O
  char*     m_pchInputFile;                                   ///< source file name
  char*     m_pchBitstreamFile;                               ///< output bitstream file
  char*     m_pchReconFile;                                   ///< output reconstruction file
  
  // source specification
  Int       m_iFrameRate;                                     ///< source frame-rates (Hz)
  Int       m_iFrameSkip;                                     ///< number of skipped frames from the beginning
  Int       m_iSourceWidth;                                   ///< source width in pixel
  Int       m_iSourceHeight;                                  ///< source height in pixel
  Int       m_iFrameToBeEncoded;                              ///< number of encoded frames
  
  // coding structure
  Int       m_iIntraPeriod;                                   ///< period of I-picture (random access period)
  Bool      m_bHierarchicalCoding;                            ///< flag for specifying hierarchical B structure
  
  // coding tools (bit-depth)
  UInt      m_uiBitDepth;                                     ///< base bit-depth
  
  // coding tools (loop filter)
  Bool      m_bLoopFilterDisable;                             ///< flag for using deblocking filter
  
  // coding tools (encoder-only parameters)
  Bool      m_bUseASR;                                        ///< flag for using adaptive motion search range
  Bool      m_bUseRDOQ;                                       ///< flag for using RD optimized quantization
  Int       m_iSearchRange;                                   ///< ME search range
  Bool      m_bUseFastEnc;                                    ///< flag for using fast encoder setting
#endif

  // internal member functions
  Void  xSetGlobal      ();                                   ///< set global variables
  Void  xCheckParameter ();                                   ///< check validity of configuration values
  Void  xPrintParameter ();                                   ///< print configuration values
  Void  xPrintUsage     ();                                   ///< print usage
  
public:
  TAppEncCfg();
  virtual ~TAppEncCfg();
  
public:
  Void  create    ();                                         ///< create option handling class
  Void  destroy   ();                                         ///< destroy option handling class
  Bool  parseCfg  ( Int argc, Char* argv[] );                 ///< parse configuration file to fill member variables
  
};// END CLASS DEFINITION TAppEncCfg

#endif // __TAPPENCCFG__

