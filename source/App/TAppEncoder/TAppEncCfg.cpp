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

/** \file     TAppEncCfg.cpp
    \brief    Handle encoder configuration parameters
*/

#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <string>
#include "TAppEncCfg.h"
#include "../../App/TAppCommon/program_options_lite.h"

#ifdef WIN32
#define strdup _strdup
#endif

using namespace std;
namespace po = df::program_options_lite;

/* configuration helper funcs */
void doOldStyleCmdlineOn(po::Options& opts, const std::string& arg);
void doOldStyleCmdlineOff(po::Options& opts, const std::string& arg);

// ====================================================================================================================
// Local constants
// ====================================================================================================================

/// max value of source padding size
/** \todo replace it by command line option
 */
#define MAX_PAD_SIZE                16

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TAppEncCfg::TAppEncCfg()
{
  m_aidQP = NULL;
}

TAppEncCfg::~TAppEncCfg()
{
  if ( m_aidQP )
  {
    delete[] m_aidQP;
  }
}

Void TAppEncCfg::create()
{
}

Void TAppEncCfg::destroy()
{
}
#if RPS
std::istringstream &operator>>(std::istringstream &in, GOPEntry &entry)     //input
{
	in >> entry.m_iSliceType;
	in >> entry.m_iPOC;
	in >> entry.m_iDOC;
	in >> entry.m_iQPOffset;
	in >> entry.m_iNumRefPics;
	in >> entry.m_bRefered;
	for (Int i = 0; i < entry.m_iNumRefPics; i++)
	{
		in >> entry.m_iReferencePics[i];
	}
	in >> entry.m_iNumRemPics;

#if RPS_BUG
	for (Int i = 0; i < entry.m_iNumRemPics; i++)
#else
	for (Int i = 0; i < entry.m_iNumRefPics; i++)
#endif
	{
		in >> entry.m_iRemovePics[i];
	}
	in >> entry.m_iTemporalId;
	return in;
}
#endif
// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** \param  argc        number of arguments
    \param  argv        array of arguments
    \retval             true when success
 */
Bool TAppEncCfg::parseCfg( Int argc, Char* argv[] )
{
  bool do_help = false;
  
  string cfg_InputFile;
  string cfg_BitstreamFile;
  string cfg_ReconFile;
  string cfg_dQPFile;

#if ENUM_AND_MACRO_YUQUANHE
  // string to char*
  string cfg_InputROIDataFile;
  string cfg_TraceFile;                        //no26
  string cfg_SeqWQFile;                       //no65
  string cfg_WeightParamDetailed;             //no69
  string cfg_WeightParamUnDetailed;           //no70
  string cfg_PicWQFile;                       //no75
  string cfg_BGFileName;                      //no87
  string cfg_BackgroundRefFile;               //no88
#endif

  po::Options opts;
  opts.addOptions()
  ("help", do_help, false, "this help text")
  ("c", po::parseConfigFile, "configuration file name")

#if ENUM_AND_MACRO_YUQUANHE
   // Files
  ("InputFile,i"                 , cfg_InputFile,                 string(""),      "original YUV input file name")//*******  copy  *******//
  ("FrameSkipNums,-fs"           , m_iFrameSkip,                           0,      "Number of frames to skip at start of input YUV")//*******  copy  *******//
  ("FramesToBeEncoded,f"         , m_iFrameToBeEncoded,                    0,      "number of frames to be encoded (default=all)")//*******  copy  *******//
  ("FrameToBeEncoded"            , m_iFrameToBeEncoded,                    0,      "depricated alias of FramesToBeEncoded")//*******  copy  *******//
  ("SourceWidth,-wdt"            , m_iSourceWidth,                         0,      "Source picture width")//*******  copy  *******//
  ("SourceHeight,-hgt"           , m_iSourceHeight,                        0,      "Source picture height")//*******  copy  *******//
  ("InputSampleBitDepth"         , m_uiInputSampleBitDepth,                8u,     "Source bit depth (8 bit or 10 bit for all components)")
  ("SampleBitDepth"              , m_uiBitDepth,                           8u,     "Source bit depth (8 bit or 10 bit for all components)")
  ("BitDepth"                    , m_uiBitDepth,                           8u,     "")
  ("TraceFile"                   , cfg_TraceFile,                  string(""),     "")
  ("ReconFile,o"                 , cfg_ReconFile,                  string(""),     "reconstructed YUV output file name")//*******  copy  *******//
  ("BitstreamFile,b"             , cfg_BitstreamFile,              string(""),     "bitstream output file name")//*******  copy  *******//
  ("InterSearch16x16"            , m_bInterSearch16x16,                  true,     "Inter block search 16x16 (0=disable, 1=enable)")
  ("InterSearch16x8"             , m_bInterSearch16x8,                   true,     "Inter block search 16x8 (0=disable, 1=enable)")
  ("InterSearch8x16"             , m_bInterSearch8x16,                   true,     "Inter block search 8x16 (0=disable, 1=enable)")
  ("InterSearch8x8"              , m_bInterSearch8x8,                    true,     "Inter block search 8x8 (0=disable, 1=enable)")
  ("InterSearchAMP"              , m_bInterSearchAMP,                    true,     " ")
  //Maximum Size
  ("MaxCUSize"                   , m_uiMaxCUSize,                         64u,      "Maximum CU size")
  ("MaxCUSize,s"                 , m_uiMaxCUSize,                         64u,      "max CU size")
  //Encoder Control
  ("ProfileID"                   , m_iProfileId,                           32,      "Profile ID")
  ("LevelID"                     , m_iLevelId,                             66,      "Level ID")
  ("IntraPeriod,-ip"             , m_iIntraPeriod,                         -1,      "intra period in frames, (-1: only first frame)")//*******  copy  *******//
  ("VECPeriod"                   , m_iVECPeriod,                            0,      "Period of VEC, distance between two sequence headers   (0=doesn't exist at the bitstream)")
  ("SeqHeaderPeriod"             , m_iSeqHeaderPeriod,                      1,      "Period of Sequence Header, Number of IntraPeriod  (0=only first)")
  ("QPIFrame"                    , m_iQPIFrame,                            45,      "Quant. param for first frame (intra) (0-63)")
  ("QPPFrame"                    , m_iQPPFrame,                            46,      "Quant. param for remaining frames (0-63)")
  ("QPBFrame"                    , m_iQPBFrame,                            47,      "Quant. param for B frames (0-63)")
  ("UseHadamard"                 , m_bUseHadamard,                       true,      "Hadamard transform (0=not used, 1=used)")
  ("FME"                         , m_bFME,                               true,      "Fast Motion Estimation (0=disable, 1=UMHexagonS)")
  ("SearchRange,-sr"             , m_iSearchRange,                         96,      "motion search range") //*******  copy  *******//
  ("NumberReferenceFrames"       , m_iNumberReferenceFrames,                4,      "Number of previous frames used for inter motion search (1-5)")
  ("ASR"                         , m_bUseASR,                           false,      "adaptive motion search range")//*******  copy  *******// Q??
  ("PicExtensionData"            , m_bPicExtensionData,                 false,      "Encode picture level extension data (copyright, picture_display, camera_parameters)")
  ("OutPutEncPic"                , m_bOutPutEncPic,                      true,      "")
                                                                                    
  //F Frames                                                                                                                                          
  ("FFrameEnable"                , m_bFFrameEnable,                      true,      "(0: Don't use F frames  1:Use F frames instead of P frames)")
  ("MHPSkipEnable"               , m_bMHPSkipEnable,                     true,      "(0: Don't use MH_PSKIP, 1:Use MH_PSKIP)")
  ("DHPEnable"                   , m_bDHPEnable,                         true,      "(0: Don't use DHP,      1:Use DHP)")
  ("WSMEnable"                   , m_bWSMEnable,                         true,      "(0: Don't use WSM,      1:Use WSM)")
                                 
  //B Frames                     
  ("NumberBFrames"               , m_iNumberBFrames,                        7,      "Number of B frames inserted (0=not used)")
  ("PFrameSkip"                  , m_iPFrameSkip,                           7,      "RA: IMG_F SKIP NUM")  // Q??
  ("HierarchicalCoding"          , m_bHierarchicalCoding,                true,      "")//*******  copy  *******//
#if IDR
  ("IDR"                         , m_bIDREnable,                         true,      "")//*******  copy  *******//
#endif                                                                               
  //RD Optimization                                                                 
  ("RDOptimization"              , m_bRDOptimization,                    true,      "rd-optimized mode decision (0:off, 1:on)")
                                                                                    
  //High Level Syntax                                                               
  ("ProgressiveSequence"        , m_bProgressiveSequence,                true,      "(0: Progressive and/or Interlace, 1: Progressive Only)")
  ("ProgressiveFrame"           , m_bProgressiveFrame,                   true,      "Progressive_frame")
  ("InterlaceCodingOption"       , m_iInterlaceCodingOption,                0,      "(0: frame coding, 1: Field coding, 2: PAFF, 3: Field picture coding)")
  ("RepeatFirstField"            , m_iRepeatFirstField,                     0,      "Source") // Q ???  
  ("TopFieldFirst"               , m_iTopFieldFirst,                        0,      "Source") // Q ???
  ("HDRMetaDataExtension"        , m_bHDRMetaDataExtension,              true,      "(0: Don't transmit metadata; 1: transmit metadata, the parameters need to be modified inside the code)")
                                                                                    
  //Coding Tools                                                                    
  ("PMVREnable"                  , m_bPMVREnable,                        true,      " ") // coding tool
  ("NSQT"                        , m_bNSQT,                              true,      " ")
  ("SDIP"                        , m_bSDIP,                              true,      " ")
  ("SECTEnable"                  , m_bSECTEnable,                        true,      "(0: Not use Secondary Transform,      1:Use Secondary Transform)")
  ("DMHEnableEncoder"            , m_bDMHEnableEncoder,                  true,      "(0: turn off  1: turn on in encoder RDO dcision)")
                                                                                    
  //Loop filter parameters                                                          
  ("CrossSliceLoopFilter"        , m_bCrossSliceLoopFilter,              true,      "Enable Cross Slice Boundary Filter (0=Disable, 1=Enable)")
  ("LoopFilterDisable"           , m_bLoopFilterDisable,                false,      "")//*******  copy  *******//
  ("LoopFilterParameter"         , m_bLoopFilterParameter,               true,      "Send loop filter parameter (0= No parameter, 1= Send Parameter) ")
  ("LoopFilterAlphaOffset"       , m_iLoopFilterAlphaOffset,                0,      "Aplha offset in loop filter")
  ("LoopFilterBetaOffset"        , m_iLoopFilterBetaOffset,                 0,      "LoopFilterBetaOffset")
  ("SAOEnable"                   , m_bSAOEnable,                         true,      "Enable SAO (1=SAO on, 0=SAO OFF)")
  ("ALFEnable"                   , m_bALFEnable,                         true,      "ALFEnable")
  ("ALFLowLatencyEncodingEnable" , m_bALFLowLatencyEncodingEnable,       true,      "ALFLowLatencyEncodingEnable")

  //Slice parameters
  ("NumberOfLCUsInSlice"         , m_iNumberOfLCUsInSlice,                  0,      "number of LCUs in one Slice  (0: one slice per frame)")

  //Frame Rate
  ("FrameRate,-fr"               , m_iFrameRate,                            0,      "Frame rate")
                                                                                    
  //chroma format parameters                                                        
  ("ChromaFormat"                , m_iChromaFormat,                         1,      "(0=4:0:0,1=4:2:0,2=4:2:2)")
  ("YUVStructure"                , m_iYUVStructure,                         0,      "(0=YUV,1=U0Y0 V0Y1)")
                                                                                    
  //Frequency Weighting Quantization                                                
  ("WQEnable"                    , m_bWQEnable,                         false,      "Frequency Weighting Quantization (0=disable,1=enable)")
  ("SeqWQM"                      , m_bSeqWQM,                           false,      "(0:default, 1:user define)")
  ("SeqWQFile"                   , cfg_SeqWQFile,                  string(""),      " ")
  ("PicWQEnable"                 , m_bPicWQEnable,                      false,      "Frame level Frequency Weighting Quantization (0=disable,1=enable)")
  ("PicWQDataIndex"              , m_iPicWQDataIndex,                       0,      "Picture level WQ data index (0:refer to seq_header,1:derived by WQ parameter,2:load from pic_header)")
  ("PicWQFile"                   , cfg_PicWQFile,                  string(""),      " ")
  ("WQParam"                     , m_iWQParam,                              0,      "Weighting Quantization Parameter(0=Default, 1=UnDetailed, 2=Detailed) ")
  ("WQModel"                     , m_iWQModel,                              1,      "Frequency Weighting Quantization Model (0-2)")
  ("WeightParamDetailed"         , cfg_WeightParamDetailed,        string(""),      "WeightParamDetailed")
  ("WeightParamUnDetailed"       , cfg_WeightParamUnDetailed,      string(""),      " ")
  ("ChromaDeltaQPDisable"        , m_bChromaDeltaQPDisable,              true,      "1: Chroma Delta QP disable; 0: Chroma Delta QP enable")
  ("ChromaDeltaU"                , m_iChromaDeltaU,                         0,      "Chroma component U delta QP")
  ("ChromaDeltaV"                , m_iChromaDeltaV,                         0,      "Chroma component V delta QP")
  ("ChromaHDRDeltaQPDisable"     , m_bChromaHDRDeltaQPDisable,          false,      " ")
  ("ChromaQPScale"               , m_iChromaQPScale,                        0,      " ")
  ("ChromaQPOffset"              , m_iChromaQPOffset,                       0,      " ")
  ("CbQPScale"                   , m_iCbQPScale,                            0,      " ")
  ("CrQPScale"                   , m_iCrQPScale,                            0,      " ")

  //Delta QP and Adaptive Quantization
  ("DeltaQP"                     , m_bDeltaQP,                          false,      "Enable dqp")
                                                                                    
  //ROI                                                                             
  ("InputROIDataFile"            , cfg_InputROIDataFile,           string(""),      "Input position data")// wait to trans
  ("ROICoding"                   , m_iROICoding,                            0,      "ROI coding scheme (0=disable, 1=differential-coding scheme)")

  //Background Prediction
  ("BackgroundEnable"            , m_bBackgroundEnable,                 false,      " ")
  ("BGFileName"                  , cfg_BGFileName,                 string(""),      " ")
  ("BackgroundRefFile"           , cfg_BackgroundRefFile,          string(""),      " ") // Q ??
  ("BGInputNumber"               , m_iBGInputNumber,                        1,      " ")
  ("BackgroundPeriod"            , m_iBackgroundPeriod,                   112,      " ")
  ("ModelNumber"                 , m_iModelNumber,                        120,      " ")
  ("BackgroundQP"                , m_iBackgroundQP,                        23,      " ")
  ("ModelMethod"                 , m_iModelMethod,                          1,      " ")
                                                                                   
  //3D Packing Mode                                                                
  ("TDMode"                      , m_iTDMode,                               0,      "(0: Ordinary 2D Video, 1: 3D Video)")
  ("ViewPackingMode"             , m_iViewPackingMode,                      0,      "(0: SbS, 1: TaB, 2: 4 view, 3-255: user define)")
  ("ViewReverse"                 , m_iViewReverse,                          0,      "(0: default view order, 1: reversed view order)")
                                                                                    
  //unclear  parameter                                                              
  ("OutputMergedPicture"         , m_iOutputMergedPicture,                  0,      " ") // Q ???
  ("TemporalScalableFlag"        , m_iTemporalScalableFlag,                 0,      " ")
                                                                                    
  //delete                                                                          
  ("BGFastMode"                  , m_iBGFastMode,                           1,      " ")
  ("SliceParameter"              , m_iSliceParameter,                       0,      " ")// Q ??  
                                                                                    
  // Encoder Optimization Tools                                                     
  //RDOQ                                                                            
  ("RDOQEnable", m_bUseRDOQ, true)//*******  copy  *******//                                                    
  ("LambdaFactor"                , m_iLambdaFactor,                        75,      " ")
  ("LambdaFactorP"               , m_iLambdaFactorP,                      120,      " ")
  ("LambdaFactorB"               , m_iLambdaFactorB,                      100,      " ")
                                                                                   
  //Refine QP  (Only used at RA configuration)                                     
  ("RefineQP"                    , m_bRefineQP,                          true,      "Enable refined quantization")
                                                                                    
  //TDRDO (only used at LD configuration)                                           
  ("TDRDOEnable"                 , m_bTDRDOEnable,                      false,      " ")
                                                                                    
  //AQPO                                                                            
  ("AQPOEnable"                  , m_bAQPOEnable,                       false,      "AQPOEnable")
                                                                                    
  //RATECONTROL                                                                     
  ("RateControl"                 , m_bRateControl,                      false,      "(0: Disable, 1: Enable. if DeltaQP=1 will perform LCU level rate control)")
  ("TargetBitRate"               , m_iTargetBitRate,                        0,      "(target bit-rate, default 0：in bps, 1000000 means 1 mbps)")
  ("RCInitialQP"                 , m_iRCInitialQP,                          0,      "RCInitialQP")
                                                                                    
  //MD5                                                                             
  ("MD5Enable"                   , m_bMD5Enable,                        false,      "m_iMD5Enable")
#else
  ("InputFile,i"                 , cfg_InputFile,                  string(""), "original YUV input file name")
  ("BitstreamFile,b"             , cfg_BitstreamFile,              string(""), "bitstream output file name")
  ("ReconFile,o"                 , cfg_ReconFile,                  string(""), "reconstructed YUV output file name")
  ("FramesToBeEncoded,f"         , m_iFrameToBeEncoded,                     0, "number of frames to be encoded (default=all)")
  ("FrameToBeEncoded"            , m_iFrameToBeEncoded,                     0, "depricated alias of FramesToBeEncoded")
  ("FrameSkip,-fs"               , m_iFrameSkip,                            0, "Number of frames to skip at start of input YUV")
  ("SourceWidth,-wdt"            , m_iSourceWidth,                          0, "Source picture width")
  ("SourceHeight,-hgt"           , m_iSourceHeight,                         0, "Source picture height")
  ("BitDepth"                    , m_uiBitDepth,                           8u )
  ("FrameRate,-fr"               , m_iFrameRate,                            0, "Frame rate")
  /* Unit definition parameters */
  ("MaxCUSize"                   , m_uiMaxCUSize,                         64u)
  ("MaxCUSize,s"                 , m_uiMaxCUSize,                         64u, "max CU size")
  ("IntraPeriod,-ip"             , m_iIntraPeriod,                         -1, "intra period in frames, (-1: only first frame)")
  ("HierarchicalCoding"          , m_bHierarchicalCoding,                true)
  ("SearchRange,-sr"             , m_iSearchRange,                         96, "motion search range")
  ("ASR"                         , m_bUseASR,                           false, "adaptive motion search range")
  /* Deblocking filter parameters */
  ("LoopFilterDisable"           , m_bLoopFilterDisable,                false)
#endif
  /* File, I/O and source parameters */
  ("HorizontalPadding,-pdx"      , m_aiPad[0],                              0, "horizontal source padding size")
  ("VerticalPadding,-pdy"        , m_aiPad[1],                              0, "vertical source padding size")
  ("PAD"                         , m_bUsePAD,                           false, "automatic source padding of multiple of 16" )
 
  /* todo: remove defaults from MaxCUSize */
  ("MaxPartitionDepth,h"         , m_uiMaxCUDepth,                         4u, "CU depth")  
  ("Log2MaxPhysTrSize,t"         , m_uiLog2MaxPhTrSize,                    5u, "maximum physical transform size")

  /* Coding structure paramters */
  ("GOPSize,g"                   , m_iGOPSize,                              1, "GOP size of temporal structure")
  ("RateGOPSize,-rg"             , m_iRateGOPSize,                         -1, "GOP size of hierarchical QP assignment (-1: implies inherit GOPSize value)")
  ("LowDelayCoding"              , m_bUseLDC,                           false, "low-delay mode")
  ("GPB"                         , m_bUseGPB,                           false, "generalized B instead of P in low-delay mode")
  ("NRF"                         , m_bUseNRF,                            true, "non-reference frame marking in last layer")
  ("BQP"                         , m_bUseBQP,                           false, "hier-P style QP assignment in low-delay mode")
 
  /* motion options */
  ("FastSearch"                  , m_iFastSearch,                           1, "0:Full search  1:Diamond  2:PMVFAST")
  ("HadamardME"                  , m_bUseHADME,                          true, "hadamard ME for fractional-pel")
 
  /* Quantization parameters */
  ("QP,q"                        , m_fQP,                                30.0, "Qp value, if value is float, QP is switched once during encoding")
  ("ChromaQP,-cq"                , m_fCQP,                               30.0, "Chroma Qp value, if value is float, QP is switched once during encoding")
  ("dQPFile,m"                   , cfg_dQPFile,                    string(""), "dQP file name")
  ("RDOQ"                        , m_bUseRDOQ,                           true)

  /* Coding tools */

  /* Misc. */
  ("FEN"                         , m_bUseFastEnc,                       false, "fast encoder setting")
  
  /* Compatability with old style -1 FOO or -0 FOO options. */
  ("1"                           , doOldStyleCmdlineOn,                        "turn option <name> on")
  ("0"                           , doOldStyleCmdlineOff,                       "turn option <name> off")
  ;
  
#if RPS
  for (Int i = 1; i<MAX_GOP + 1; i++)
  {
	  std::ostringstream cOSS;
	  cOSS << "Frame" << i;
	  opts.addOptions()(cOSS.str(), m_GOPList[i - 1], GOPEntry());
  }
#endif

  po::setDefaults(opts);
  po::scanArgv(opts, argc, (const char**) argv);
  
  if (argc == 1 || do_help)
  {
    /* argc == 1: no options have been specified */
    po::doHelp(cout, opts);
    xPrintUsage();
    return false;
  }
  
  /*
   * Set any derived parameters
   */
  /* convert std::string to c string for compatibility */
  m_pchInputFile = cfg_InputFile.empty() ? NULL : strdup(cfg_InputFile.c_str());
  m_pchBitstreamFile = cfg_BitstreamFile.empty() ? NULL : strdup(cfg_BitstreamFile.c_str());
  m_pchReconFile = cfg_ReconFile.empty() ? NULL : strdup(cfg_ReconFile.c_str());
  m_pchdQPFile = cfg_dQPFile.empty() ? NULL : strdup(cfg_dQPFile.c_str());
  
#if ENUM_AND_MACRO_YUQUANHE
  m_pchInputROIDataFile = cfg_InputROIDataFile.empty() ? NULL : strdup(cfg_InputROIDataFile.c_str());
  m_pchTraceFile = cfg_TraceFile.empty() ? NULL : strdup(cfg_TraceFile.c_str());
  m_pchSeqWQFile = cfg_SeqWQFile.empty() ? NULL : strdup(cfg_SeqWQFile.c_str());
  m_pchWeightParamDetailed = cfg_WeightParamDetailed.empty() ? NULL : strdup(cfg_WeightParamDetailed.c_str());
  m_pchWeightParamUnDetailed = cfg_WeightParamUnDetailed.empty() ? NULL : strdup(cfg_WeightParamUnDetailed.c_str());
  m_pchPicWQFile = cfg_PicWQFile.empty() ? NULL : strdup(cfg_PicWQFile.c_str());
  m_pchBGFileName = cfg_BGFileName.empty() ? NULL : strdup(cfg_BGFileName.c_str());
  m_pchBackgroundRefFile = cfg_BackgroundRefFile.empty() ? NULL : strdup(cfg_BackgroundRefFile.c_str());
#endif
  if (m_iRateGOPSize == -1)
  {
    /* if rateGOPSize has not been specified, the default value is GOPSize */
    m_iRateGOPSize = m_iGOPSize;
  }
  
  // compute source padding size
  if ( m_bUsePAD )
  {
    if ( m_iSourceWidth%MAX_PAD_SIZE )
    {
      m_aiPad[0] = (m_iSourceWidth/MAX_PAD_SIZE+1)*MAX_PAD_SIZE - m_iSourceWidth;
    }
    
    if ( m_iSourceHeight%MAX_PAD_SIZE )
    {
      m_aiPad[1] = (m_iSourceHeight/MAX_PAD_SIZE+1)*MAX_PAD_SIZE - m_iSourceHeight;
    }
  }
  m_iSourceWidth  += m_aiPad[0];
  m_iSourceHeight += m_aiPad[1];
  
  // allocate picture-based dQP values
  m_aidQP = new Int[ m_iFrameToBeEncoded + m_iRateGOPSize + 1 ];
  ::memset( m_aidQP, 0, sizeof(Int)*( m_iFrameToBeEncoded + m_iRateGOPSize + 1 ) );
  
  // handling of floating-point QP values
  // if QP is not integer, sequence is split into two sections having QP and QP+1
  m_iQP = (Int)( m_fQP );
#if CHROMA_QP_YQH
  m_iCQP=  (Int) QP_SCALE_CR[Clip3(0, 63,(int) (m_fQP - MIN_QP))];
   //m_iCQP = (Int)( m_fQP);
#else
  m_iCQP = (Int)( m_fCQP );
#endif
  if ( m_iQP < m_fQP )
  {
    Int iSwitchPOC = (Int)( m_iFrameToBeEncoded - (m_fQP - m_iQP)*m_iFrameToBeEncoded + 0.5 );
    
    iSwitchPOC = (Int)( (Double)iSwitchPOC / m_iRateGOPSize + 0.5 )*m_iRateGOPSize;
    for ( Int i=iSwitchPOC; i<m_iFrameToBeEncoded + m_iRateGOPSize + 1; i++ )
    {
      m_aidQP[i] = 1;
    }
  }
  
  // reading external dQP description from file
  if ( m_pchdQPFile )
  {
    FILE* fpt=fopen( m_pchdQPFile, "r" );
    if ( fpt )
    {
      Int iValue;
      Int iPOC = 0;
      while ( iPOC < m_iFrameToBeEncoded )
      {
        if ( fscanf(fpt, "%d", &iValue ) == EOF ) break;
        m_aidQP[ iPOC ] = iValue;
        iPOC++;
      }
      fclose(fpt);
    }
  }

  // check validity of input parameters
  xCheckParameter();
  
  // set global varibles
  xSetGlobal();
  
  // print-out parameters
  xPrintParameter();
  
  return true;
}

// ====================================================================================================================
// Private member functions
// ====================================================================================================================

Bool confirmPara(Bool bflag, const char* message);

Void TAppEncCfg::xCheckParameter()
{
  bool check_failed = false; /* abort if there is a fatal configuration problem */
#define xConfirmPara(a,b) check_failed |= confirmPara(a,b)
  // check range of parameters
  xConfirmPara( m_iFrameRate <= 0,                                                          "Frame rate must be more than 1" );
  xConfirmPara( m_iFrameSkip < 0,                                                           "Frame Skipping must be more than 0" );
  xConfirmPara( m_iFrameToBeEncoded <= 0,                                                   "Total Number Of Frames encoded must be more than 1" );
  xConfirmPara( m_iGOPSize < 1 ,                                                            "GOP Size must be more than 1" );
  xConfirmPara( m_iGOPSize > 1 &&  m_iGOPSize % 2,                                          "GOP Size must be a multiple of 2, if GOP Size is greater than 1" );
#if !RPS_BUG
  xConfirmPara((m_iIntraPeriod > 1 && m_iIntraPeriod < m_iGOPSize) || m_iIntraPeriod < 0, "Intra period must be more than GOP size, or 1 , 0");
#else
  xConfirmPara((m_iIntraPeriod > 0 && m_iIntraPeriod < m_iGOPSize) || m_iIntraPeriod == 0, "Intra period must be more than GOP size, or -1 , not 0");
#endif  xConfirmPara( m_iQP < 0 || m_iQP > 127,   
  xConfirmPara( m_iQP < 0 || m_iQP > 127,                                                   "quantization parameter is out of supported range (0 to 127)" );
  xConfirmPara( m_iCQP < 0 || m_iCQP > 127,                                                 "chroma quantization parameter is out of supported range (0 to 127)" );
  xConfirmPara( m_iFastSearch < 0 || m_iFastSearch > 2,                                     "Fast Search Mode is not supported value (0:Full search  1:Diamond  2:PMVFAST)" );
  xConfirmPara( m_iSearchRange < 0 ,                                                        "Search Range must be more than 0" );
#if !RPS
  xConfirmPara(m_iFrameToBeEncoded != 1 && m_iFrameToBeEncoded <= m_iGOPSize, "Total Number of Frames to be encoded must be larger than GOP size");
#endif
  xConfirmPara( (m_uiMaxCUSize  >> m_uiMaxCUDepth) < 2,                                     "Minimum partition size should be larger than or equal to 2");
  xConfirmPara( (m_iSourceWidth  % (m_uiMaxCUSize >> (m_uiMaxCUDepth-1)))!=0,               "Frame width should be multiple of minimum CU size");
  xConfirmPara( (m_iSourceHeight % (m_uiMaxCUSize >> (m_uiMaxCUDepth-1)))!=0,               "Frame height should be multiple of minimum CU size");
  xConfirmPara( m_uiLog2MaxPhTrSize < 2,                                                    "Physical transform size must be greater than 2");
  
  // max CU width and height should be power of 2
  UInt ui = m_uiMaxCUSize;
  m_uiLog2MaxCUSize = 0;
  while(ui)
  {
    ui >>= 1;
    if( (ui & 1) == 1)
      xConfirmPara( ui != 1 , "Max CU size should be 2^n");

    m_uiLog2MaxCUSize++;
  }
  m_uiLog2MaxCUSize--;
  
#if  RPS
  ///<LDP
  ///<intra-only
  if (m_iIntraPeriod == 1 && m_GOPList[0].m_iPOC == -1)
  {
	  m_GOPList[0] = GOPEntry();
	  m_GOPList[0].m_iPOC = 1;
#if RPS_BUG
	  m_GOPList[0].m_iNumRemPics = 1; //zengpeng 20170522
	  m_GOPList[0].m_iRemovePics[0] = -1;//zengpeng 20170522
	  m_iExtraRPSs = 0;
#endif
  }
#if RPS_BUG
  else
  {
#endif
	  ////扩大GOPLIST的参考链，在帧间参考时，ref_num=4 然而REC_Buffer中数量不够，且关键帧的数量也不够
#if !RPS_BUG
	  //m_iGOPSize = 4;
#endif
  ////扩大GOPLIST的参考链，在帧间参考时，ref_num=4 然而REC_Buffer中数量不够，且关键帧的数量也不够
  // m_iGOPSize = 4;
  Int CheckGOP = 1;
  m_iExtraRPSs = 0;
  Bool errorGOP = false;
  Int DelayRemPicPOC = 0;
  Bool verifiedGOP = false;

  //初始化list01的参考帧数   先暂时没有初始化扩展链
  //帧的移除还有一定问题，在GOPlist扩展后，移除第一个关键帧存在问题。因此在后面时，写一个判定，在POC<1时，
  while (!errorGOP&&!verifiedGOP)
  {
	  Int curGOP = (CheckGOP - 1) % m_iGOPSize; //循环GOPLIST 直到参考结构稳定， LDP下ref_num=4，GOPLIST长度为13
	  Int curPOC = ((CheckGOP - 1) / m_iGOPSize)*m_iGOPSize + m_GOPList[curGOP].m_iPOC;//GOPSIZE = 4 current GOPSIZE = 1
	  if (m_GOPList[curGOP].m_iPOC<0)
	  {
		  printf("\nError: found fewer Reference Picture Sets than GOPSize\n");
		  errorGOP = true;
	  }
	  else
	  {
		  Bool beforeRefI = false; //当一个I帧到来时，需要将RPS扩展至m_iExtraRPSs+gopsize大小,才能正确管理参考SET
		  Bool beforeRemI = false; //当一个I帧到来时，需要将RPS扩展至m_iExtraRPSs + gopsize大小, 才能正确管理移出SET
		  //当参考结构稳定后，跳出GOPlist扩展
		  for (int i = 0; i < m_GOPList[curGOP].m_iNumRefPics; i++)
		  {
			  Int absPOC = curPOC + m_GOPList[curGOP].m_iReferencePics[i];
			  if (absPOC < 0)
			  {
				  beforeRefI = true;
			  }
		  }
		  for (int i = 0; i < m_GOPList[curGOP].m_iNumRemPics; i++)
		  {
			  Int absPOC = curPOC + m_GOPList[curGOP].m_iRemovePics[i];
			  if (absPOC < 0)
			  {
				  beforeRefI = true;
			  }
		  }
		  if (!beforeRefI&&!errorGOP&&!beforeRemI)
		  {
			  verifiedGOP = true;
		  }
		  else
		  {
			  m_GOPList[m_iGOPSize + m_iExtraRPSs] = m_GOPList[curGOP];

			  //参考管理
			  Int NewNumRefPics = 0;
			  Int NewNumRemPics = 0;
			  Int NumRefPicsActive = 0;
			  for (int i = 0; i < m_GOPList[curGOP].m_iNumRefPics; i++)
			  {
				  m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iReferencePics[i] = 0;
				  Int RefPicPOC = curPOC + m_GOPList[curGOP].m_iReferencePics[i];
				  if (RefPicPOC >= 0)
				  {
					  m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iReferencePics[i] = m_GOPList[curGOP].m_iReferencePics[i];
					  NewNumRefPics++;
				  }
			  }
			  NumRefPicsActive = NewNumRefPics;
			  //Insert REF_frame
#if B_RPS
			  if (m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iSliceType != 'B')
			  {
				  for (Int InsertRefPic = -2; InsertRefPic >= -CheckGOP; InsertRefPic--)
				  {
					  for (int i = 0; i < NewNumRefPics; i++)
					  {
						  if (m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iReferencePics[i] < InsertRefPic)
						  {
							  ///重排序，且参考帧数加1
							  Int TempNum = NewNumRefPics;
							  while ((TempNum - i) > 0)
							  {
								  m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iReferencePics[TempNum] = m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iReferencePics[TempNum - 1];
								  TempNum--;
							  }
							  m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iReferencePics[i] = InsertRefPic;
							  NumRefPicsActive++;
							  break;
						  }
						  else if (m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iReferencePics[i] == InsertRefPic)
						  {
							  break;
						  }
					  }
					  NewNumRefPics = NumRefPicsActive;
					  if (NumRefPicsActive >= m_GOPList[curGOP].m_iNumRefPics)
					  {
						  break;
					  }
				  }
			  }
#else
			  for (Int InsertRefPic = -2; InsertRefPic >= -CheckGOP; InsertRefPic--)
			  {
				  for (int i = 0; i < NewNumRefPics; i++)
				  {
					  if (m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iReferencePics[i] < InsertRefPic)
					  {
						  ///重排序，且参考帧数加1
						  Int TempNum = NewNumRefPics;
						  while ((TempNum - i) > 0)
						  {
							  m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iReferencePics[TempNum] = m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iReferencePics[TempNum - 1];
							  TempNum--;
						  }
						  m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iReferencePics[i] = InsertRefPic;
						  NumRefPicsActive++;
						  break;
					  }
					  else if (m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iReferencePics[i] == InsertRefPic)
					  {
						  break;
					  }
				  }
				  NewNumRefPics = NumRefPicsActive;
				  if (NumRefPicsActive >= m_GOPList[curGOP].m_iNumRefPics)
				  {
					  break;
				  }
			  }
#endif
			  NewNumRefPics = NumRefPicsActive;
			  m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iNumRefPics = NewNumRefPics;
			  ///check
			  for (int i = 0; i < NewNumRefPics; i++)
			  {
				  if (m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iReferencePics[i] == 0)
				  {
					  printf("\nError: found fewer Reference Picture Sets than GOPSize\n");
					  errorGOP = true;
				  }
			  }
			  //移除管理REMPIC
			  if (NewNumRefPics< m_GOPList[curGOP].m_iNumRefPics)//当参考帧的数量不足NumRefPics=4时，不做移除操作，但是会存在一些帧在随后的帧buffer中，无法被移除。
			  {
				  for (int j = 0; j < m_GOPList[curGOP].m_iNumRemPics; j++)
				  {
					  m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iRemovePics[j] = 0;
					  DelayRemPicPOC = (curPOC + m_GOPList[curGOP].m_iRemovePics[j]>0) ? (curPOC + m_GOPList[curGOP].m_iRemovePics[j]) : 0;
				  }

				  NewNumRemPics = 0;
			  }
			  else
			  {
				  for (int j = 0; j < m_GOPList[curGOP].m_iNumRemPics; j++)
				  {
					  m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iRemovePics[j] = 0;
					  Int RemPicPOC = curPOC + m_GOPList[curGOP].m_iRemovePics[j];
					  if (RemPicPOC >= 0)
					  {
						  m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iRemovePics[j] = m_GOPList[curGOP].m_iRemovePics[j];
						  NewNumRemPics++;
					  }
					  if (DelayRemPicPOC > 0)
					  {
						  m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iRemovePics[j + NewNumRemPics] = DelayRemPicPOC - curPOC;
						  DelayRemPicPOC = 0;
						  NewNumRemPics++;
						  j += NewNumRemPics;
					  }
				  }
			  }
			  m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iNumRemPics = NewNumRemPics;
#if B_RPS
			  m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iPOC = curPOC;
#else
			  m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iPOC = CheckGOP;
#endif
			  m_GOPList[m_iGOPSize + m_iExtraRPSs].m_iDOC = CheckGOP; //暂时没采用DOC顺序
			  CheckGOP++;
			  m_iExtraRPSs++;
		  }
	  }
  }
  for (int i = 0; i < m_iGOPSize + m_iExtraRPSs; i++)
  {
	  m_GOPList[i].m_iNumberOfNegativeRefPics = 0;
	  m_GOPList[i].m_iNumberOfPositiveRefPics = 0;
	  for (int j = 0; j < m_GOPList[i].m_iNumRefPics; j++)
	  {
		  if (m_GOPList[i].m_iReferencePics[j] < 0)
		  {
			  m_GOPList[i].m_iNumberOfNegativeRefPics++;
		  }
		  else
		  {
			  m_GOPList[i].m_iNumberOfPositiveRefPics++;
		  }
	  }
  }
#if RPS_BUG
  }
#endif

#endif
#undef xConfirmPara
  if (check_failed)
  {
    exit(EXIT_FAILURE);
  }
}

/** \todo use of global variables should be removed later
 */
Void TAppEncCfg::xSetGlobal()
{
  // set max CU width & height
  g_uiLog2MaxCUSize  = m_uiLog2MaxCUSize;
  
  // compute actual CU depth with respect to config depth and max transform size
  g_uiMaxCUDepth = m_uiMaxCUDepth;
  
  // set internal bit-depth and constants
  g_uiBitDepth     = m_uiBitDepth;                      // base bit-depth
  g_uiBASE_MAX     = ((1<<(g_uiBitDepth))-1);
}

Void TAppEncCfg::xPrintParameter()
{
  printf("\n");
  printf("Input          File          : %s\n", m_pchInputFile          );
  printf("Bitstream      File          : %s\n", m_pchBitstreamFile      );
  printf("Reconstruction File          : %s\n", m_pchReconFile          );
  printf("Real     Format              : %dx%d %dHz\n", m_iSourceWidth - m_aiPad[0], m_iSourceHeight-m_aiPad[1], m_iFrameRate );
  printf("Internal Format              : %dx%d %dHz\n", m_iSourceWidth, m_iSourceHeight, m_iFrameRate );
  printf("Frame index                  : %d - %d (%d frames)\n", m_iFrameSkip, m_iFrameSkip+m_iFrameToBeEncoded-1, m_iFrameToBeEncoded );
  printf("CU size / depth              : %d / %d\n", m_uiMaxCUSize, m_uiMaxCUDepth );
  printf("Max Physical Transform size  : %dx%d\n", 1 << m_uiLog2MaxPhTrSize, 1 << m_uiLog2MaxPhTrSize );

  printf("Motion search range          : %d\n", m_iSearchRange );
  printf("Intra period                 : %d\n", m_iIntraPeriod );
  printf("QP                           : %5.2f, %5.2f\n", m_fQP, m_fCQP );
  printf("GOP size                     : %d\n", m_iGOPSize );
  printf("Rate GOP size                : %d\n", m_iRateGOPSize );

  printf("\n");
  
  printf("TOOL CFG: ");
  printf("HAD:%d ", m_bUseHADME           );
  printf("RDQ:%d ", m_bUseRDOQ            );
  printf("ASR:%d ", m_bUseASR             );
  printf("PAD:%d ", m_bUsePAD             );
  printf("LDC:%d ", m_bUseLDC             );
  printf("NRF:%d ", m_bUseNRF             );
  printf("BQP:%d ", m_bUseBQP             );
  printf("GPB:%d ", m_bUseGPB             );
  printf("FEN:%d ", m_bUseFastEnc         );

  printf("\n");
  
  fflush(stdout);
}

Void TAppEncCfg::xPrintUsage()
{
  printf( "          <name> =  GPB - generalized B instead of P in low-delay mode\n");
  printf( "                   HAD - hadamard ME for fractional-pel\n");
  printf( "                   RDQ - RDOQ\n");
  printf( "                   LDC - low-delay mode\n");
  printf( "                   NRF - non-reference frame marking in last layer\n");
  printf( "                   BQP - hier-P style QP assignment in low-delay mode\n");
  printf( "                   PAD - automatic source padding of multiple of 16\n");
  printf( "                   ASR - adaptive motion search range\n");
  printf( "                   FEN - fast encoder setting\n");  

  printf( "\n" );
  printf( "  Example 1) TAppEncoder.exe -c test.cfg -q 32 -g 8 -f 9 -s 64 -h 4\n");
  printf("              -> QP 32, hierarchical-B GOP 8, 9 frames, 64x64-4x4 CU (~2x2 CU)\n\n");
  printf( "  Example 2) TAppEncoder.exe -c test.cfg -q 32 -g 4 -f 9 -s 64 -h 4 -1 LDC\n");
  printf("              -> QP 32, hierarchical-P GOP 4, 9 frames, 64x64-4x4 CU (~2x2 CU)\n\n");
}

Bool confirmPara(Bool bflag, const char* message)
{
  if (!bflag)
    return false;
  
  printf("Error: %s\n",message);
  return true;
}

/* helper function */
/* for handling "-1/-0 FOO" */
void translateOldStyleCmdline(const char* value, po::Options& opts, const std::string& arg)
{
  const char* argv[] = {arg.c_str(), value};
  /* replace some short names with their long name varients */
  if (arg == "LDC")
  {
    argv[0] = "LowDelayCoding";
  }
  else if (arg == "RDQ")
  {
    argv[0] = "RDOQ";
  }
  else if (arg == "HAD")
  {
    argv[0] = "HadamardME";
  }

  /* issue a warning for change in FEN behaviour */
  if (arg == "FEN")
  {
    /* xxx todo */
  }
  po::storePair(opts, argv[0], argv[1]);
}

void doOldStyleCmdlineOn(po::Options& opts, const std::string& arg)
{
  translateOldStyleCmdline("1", opts, arg);
}

void doOldStyleCmdlineOff(po::Options& opts, const std::string& arg)
{
  translateOldStyleCmdline("0", opts, arg);
}
