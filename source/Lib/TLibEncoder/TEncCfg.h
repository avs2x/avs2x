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

/** \file     TEncCfg.h
    \brief    encoder configuration class (header)
*/

#ifndef __TENCCFG__
#define __TENCCFG__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../TLibCommon/CommonDef.h"
#include <assert.h>

// ====================================================================================================================
// Class definition
// ====================================================================================================================
#if RPS
struct GOPEntry
{
	Char m_iSliceType;
	Int m_iPOC;
	Int m_iDOC;
	Int m_iQPOffset;
	Int m_iNumRefPics;
	Int m_iNumberOfNegativeRefPics;
	Int m_iNumberOfPositiveRefPics;
	Bool m_bRefered;
	Int m_iReferencePics[MAX_NUM_REF_PICS];
	Int m_iNumRemPics;
	Int m_iRemovePics[MAX_NUM_REF_PICS];
	Int m_iTemporalId;
	GOPEntry()
		: m_iSliceType('P')
		, m_iPOC(-1)
		, m_iDOC(-1)
		, m_iQPOffset(0)
		, m_iNumRefPics(0)
		, m_iNumberOfNegativeRefPics(0)
		, m_iNumberOfPositiveRefPics(0)
		, m_bRefered(true)
		, m_iNumRemPics(0)
		, m_iTemporalId(0)
	{
		::memset(m_iReferencePics, 0, sizeof(m_iReferencePics));
		::memset(m_iRemovePics, 0, sizeof(m_iReferencePics));
	}
};

std::istringstream &operator>>(std::istringstream &in, GOPEntry &entry);     //input

#endif
/// encoder configuration class
class TEncCfg
{
protected:

  //====== Coding Structure ========
  UInt      m_uiIntraPeriod;
  Int       m_iGOPSize;
  Int       m_iRateGOPSize;
  
  Int       m_iQP;                              //  if (AdaptiveQP == OFF) 
  Int       m_iCQP;                             //  if (AdaptiveQP == OFF) 
  Int       m_aiPad[2];
  
  //======= Transform =============
  UInt      m_uiLog2MaxPhTrSize;

  //====== Motion search ========
  Int       m_iFastSearch;                      //  0:Full search  1:Diamond  2:PMVFAST
  Int       m_iMaxDeltaQP;                      //  Max. absolute delta QP (1:default)
  
  //====== Tool list ========
  Bool      m_bUseALF;
  Bool      m_bUseHADME;
  Bool      m_bUseGPB;
  Bool      m_bUseLDC;
  Bool      m_bUsePAD;
  Bool      m_bUseNRF;
  Bool      m_bUseBQP;
  Bool      m_bUseFastEnc;
  
  Int*      m_aidQP;

#if ENUM_AND_MACRO_YUQUANHE
  //AVS2
  // Files
  //char*     m_pchInputFile;             //no20                         ///< source file name
  Int       m_iFrameSkip;               //no24      FrameSkipNums      ///< number of skipped frames from the beginning
  Int       m_iFrameToBeEncoded;        //no7                          ///< number of encoded frames
  Int       m_iSourceWidth;              //no15                        ///< source width in pixel
  Int       m_iSourceHeight;             //no16                        ///< source height in pixel
  UInt      m_uiInputSampleBitDepth;      //no102                      /// Source bit depth (8 bit or 10 bit for all components)
  UInt      m_uiBitDepth;                 //no101                    ///Internal coding bit depth
  //Maximum Size
  UInt      m_uiMaxCUSize;          //no1                           ///< max. CU size in pixel

  //Encoder Control
  Int       m_iProfileId;			//No2					    // bitstream 
  Int	      m_iLevelId;             //No3                       // bitstream
  Int       m_iIntraPeriod;         //No4                       //Period of I-Frames (0=only first)
  Int       m_iVECPeriod;           //no5                       // Period of VEC, distance between two sequence headers   (0=doesn't exist at the bitstream)
  Int       m_iSeqHeaderPeriod;      //no6		                // Period of Sequence Header, Number of IntraPeriod  (0=only first)
  Int       m_iQPIFrame;             //no8		                // Quant. param for first frame (intra) (0-63)
  Int		m_iQPPFrame;             //no9		                // Quant. param for remaining frames (0-63)
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
  Bool      m_bInterSearchAMP;
  Bool      m_bPicExtensionData;            //no41            //Encode picture level extension data (copyright, picture_display, camera_parameters)  
  Bool      m_bOutPutEncPic;                 //no85

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
  Bool       m_bProgressiveFrame;           //no57	    //(0: Interlace, 1: Progressive) 
  Int       m_iInterlaceCodingOption;       //no43      //(0: frame coding, 1: Field coding, 2: PAFF, 3: Field picture coding)
  Int       m_iRepeatFirstField;             //NO44
  Int       m_iTopFieldFirst;                //no45       
  Bool      m_bHDRMetaDataExtension;          //no47   //(0: Don't transmit metadata; 1: transmit metadata, the parameters need to be modified inside the code)
#if RPS
  GOPEntry  m_GOPList[MAX_GOP];
  Int       m_iExtraRPSs;
#endif
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
  Bool       m_bSeqWQM;                            //no64
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
//==== File I/O ========
Int       m_iFrameRate;
Int       m_iFrameSkip;
Int       m_iSourceWidth;
Int       m_iSourceHeight;
Int       m_iFrameToBeEncoded;
//====== B Picture ========
Bool      m_bHierarchicalCoding;              //  hierarchical-B coding
//====== Loop/Deblock Filter ========
Bool      m_bLoopFilterDisable;
//====== Motion search ========
Int       m_iSearchRange;                     //  0:Full frame
//====== Tool list ========
Bool      m_bUseASR;
Bool      m_bUseRDOQ;
#endif



public:
  TEncCfg()          {}
  virtual ~TEncCfg() {}
  
  //====== Coding Structure ========
  Void      setGOPSize                      ( Int   i )      { m_iGOPSize = i;                                     }
  Void      setRateGOPSize                  ( Int   i )      { m_iRateGOPSize = i;                                 }
  
  Void      setQP                           ( Int   i )      { m_iQP = i;                                          }
  Void      setCQP                          ( Int   i )      { m_iCQP = i;                                         }
  Void      setPad                          ( Int*  iPad )   { for ( Int i = 0; i < 2; i++ ) m_aiPad[i] = iPad[i]; }

  //======== Transform =============
  Void      setLog2MaxPhTrSize              ( UInt  u )      { m_uiLog2MaxPhTrSize = u; }
  
  //====== Motion search ========
  Void      setFastSearch                   ( Int   i )      { m_iFastSearch = i;       }
  Void      setMaxDeltaQP                   ( Int   i )      { m_iMaxDeltaQP = i;       }
  
  //==== Coding Structure ========
  Int       getGOPSize                      ()      { return  m_iGOPSize;                    }
  Int       getRateGOPSize                  ()      { return  m_iRateGOPSize;                }
  Int       getQP                           ()      { return  m_iQP;                         }
  Int       getCQP                          ()      { return  m_iCQP;                        }
  Int       getPad                          ( Int i ) { assert (i < 2 ); return  m_aiPad[i]; }
#if RPS
  GOPEntry  getGOPEntry(Int   i)            { return m_GOPList[i]; }
  Void      setGopList(GOPEntry*  GOPList) { for (Int i = 0; i < MAX_GOP; i++) m_GOPList[i] = GOPList[i]; }
#endif
  //======== Transform =============
  UInt      getLog2MaxPhTrSize              ()      { return  m_uiLog2MaxPhTrSize; }
  
  //==== Motion search ========
  Int       getFastSearch                   ()      { return  m_iFastSearch;  }
  Int       getMaxDeltaQP                   ()      { return  m_iMaxDeltaQP;  }
  
  //==== Tool list ========
  Void      setUseHADME                     ( Bool  b )     { m_bUseHADME   = b; }
  Void      setUseGPB                       ( Bool  b )     { m_bUseGPB     = b; }
  Void      setUseLDC                       ( Bool  b )     { m_bUseLDC     = b; }
  Void      setUsePAD                       ( Bool  b )     { m_bUsePAD     = b; }
  Void      setUseNRF                       ( Bool  b )     { m_bUseNRF     = b; }
  Void      setUseBQP                       ( Bool  b )     { m_bUseBQP     = b; }
  Void      setUseFastEnc                   ( Bool  b )     { m_bUseFastEnc = b; }
 
  Void      setdQPs                         ( Int*  p )     { m_aidQP       = p; }
  Bool      getUseHADME                     ()      { return m_bUseHADME;   }
  Bool      getUseGPB                       ()      { return m_bUseGPB;     }
  Bool      getUseLDC                       ()      { return m_bUseLDC;     }
  Bool      getUsePAD                       ()      { return m_bUsePAD;     }
  Bool      getUseNRF                       ()      { return m_bUseNRF;     }
  Bool      getUseBQP                       ()      { return m_bUseBQP;     }
  Bool      getUseFastEnc                   ()      { return m_bUseFastEnc; }

  Int*      getdQPs                         ()      { return m_aidQP;       }

#if ENUM_AND_MACRO_YUQUANHE
//==== Files ========
  Void      setFrameSkip                    ( Int   i )      { m_iFrameSkip = i;                  }
  Void      setFrameToBeEncoded             ( Int   i )      { m_iFrameToBeEncoded = i;           }
  Void      setSourceWidth                  ( Int   i )      { m_iSourceWidth = i;                }
  Void      setSourceHeight                 ( Int   i )      { m_iSourceHeight = i;               }
  Void      setInputSampleBitDepth          ( Int   i )      { m_uiInputSampleBitDepth = (UInt)i; }
  Void      setBitDepth                     ( Int   i )      { m_uiBitDepth = (UInt)i; }

  //==== Maximum Size =======
  Void      setMaxCUSize                    ( Int   i )      { m_uiMaxCUSize = (UInt)i; }

  //==== Encoder Control =======
  Void      setProfileId                    ( Int   i )      { m_iProfileId = i;             }
  Void      setLevelId                      ( Int   i )      { m_iLevelId = i;               }
  Void      setIntraPeriod                  ( Int   i )      { m_iIntraPeriod = i;           }
  Void      setVECPeriod                    ( Int   i )      { m_iVECPeriod = i;             }
  Void      setSeqHeaderPeriod              ( Int   i )      { m_iSeqHeaderPeriod = i;       }
  Void      setQPIFrame                     ( Int   i )      { m_iQPIFrame = i;              }
  Void      setQPPFrame                     ( Int   i )      { m_iQPPFrame = i;              }
  Void      setQPBFrame                     ( Int   i )      { m_iQPBFrame = i;              }
  Void      setUseHadamard                  ( Bool  b )      { m_bUseHadamard = b;           }
  Void      setFME                          ( Bool  b )      { m_bFME = b;                   }
  Void      setSearchRange                  ( Int   i )      { m_iSearchRange = i;           }
  Void      setNumberReferenceFrames        ( Int   i )      { m_iNumberReferenceFrames = i; }
  Void      setInterSearch16x16             ( Bool  b )      { m_bInterSearch16x16 = b;      }
  Void      setInterSearch16x8              ( Bool  b )      { m_bInterSearch16x8 = b;       }
  Void      setInterSearch8x16              ( Bool  b )      { m_bInterSearch8x16 = b;       }
  Void      setInterSearch8x8               ( Bool  b )      { m_bInterSearch8x8 = b;        }
  Void      setUseASR                       ( Bool  b )      { m_bUseASR = b;                }
  Void      setPicExtensionData             ( Bool  b )      { m_bPicExtensionData = b;      }
  Void      setOutPutEncPic                 ( Bool  b )      { m_bOutPutEncPic = b;          }
  Void      setInterSearchAMP               ( Bool  b )      { m_bInterSearchAMP = b;        }

  //==== F Frames ======
  Void      setFFrameEnable                 ( Bool  b )      { m_bFFrameEnable = b;  }
  Void      setDHPEnable                    ( Bool  b )      { m_bDHPEnable = b;     }
  Void      setMHPSkipEnable                ( Bool  b )      { m_bMHPSkipEnable = b; }
  Void      setWSMEnable                    ( Bool  b )      { m_bWSMEnable = b;     }

  //==== B Frames ======
  Void      setNumberBFrames                ( Int   i )      { m_iNumberBFrames = i;      }
  Void      setPFrameSkip                   ( Int   i )      { m_iPFrameSkip = i;         }
  Void      setHierarchicalCoding           ( Bool  b )      { m_bHierarchicalCoding = b; }
#if IDR
  Void      setIDREnable                    ( Bool  b )      { m_bIDREnable = b;          }
#endif
  //==== RD Optimization ======
  Void      setRDOptimization               ( Bool  b )      { m_bRDOptimization = b; }

  //==== High Level Syntax ======
  Void      setProgressiveSequence          ( Bool  b)      { m_bProgressiveSequence = b;    }
  Void      setProgressiveFrame             ( Bool  b )      { m_bProgressiveFrame = b;      }
  Void      setInterlaceCodingOption        ( Int   i )      { m_iInterlaceCodingOption = i; }
  Void      setRepeatFirstField             ( Int   i )      { m_iRepeatFirstField = i;      }
  Void      setTopFieldFirst                ( Int   i )      { m_iTopFieldFirst = i;         }
  Void      setHDRMetaDataExtension         ( Bool  b )      { m_bHDRMetaDataExtension = b;  }

  //==== Coding Tools ======
  Void      setPMVREnable                   ( Bool  b )      { m_bPMVREnable = b;       }
  Void      setNSQT                         ( Bool  b )      { m_bNSQT = b;             }
  Void      setSDIP                         ( Bool  b )      { m_bSDIP = b;             }
  Void      setSECTEnable                   ( Bool  b )      { m_bSECTEnable = b;       }
  Void      setDMHEnableEncoder             ( Bool  b )      { m_bDMHEnableEncoder = b; }

  //==== Loop filter parameters ======
  Void      setCrossSliceLoopFilter         ( Bool  b )      { m_bCrossSliceLoopFilter = b;        }
  Void      setLoopFilterDisable            ( Bool  b )      { m_bLoopFilterDisable = b;           }
#if RPS
  Void      setExtraRPSs(Int   i)      { m_iExtraRPSs = i; }
#endif 
  Void      setLoopFilterParameter          ( Bool  b )      { m_bLoopFilterParameter = b;         }
  Void      setLoopFilterAlphaOffset        ( Int   i )      { m_iLoopFilterAlphaOffset = i;       }
  Void      setLoopFilterBetaOffset         ( Int   i )      { m_iLoopFilterBetaOffset = i;        }
  Void      setSAOEnable                    ( Bool  b )      { m_bSAOEnable = b;                   }
  Void      setALFEnable                    ( Bool  b )      { m_bALFEnable = b;                   }
  Void      setALFLowLatencyEncodingEnable  ( Bool  b )      { m_bALFLowLatencyEncodingEnable = b; }

  //==== Slice parameters ======
  Void      setNumberOfLCUsInSlice          ( Int   i )      { m_iNumberOfLCUsInSlice = i; }

  //==== Frame Rate ======
  Void      setFrameRate                    ( Int   i )      { m_iFrameRate = i; }

  //==== chroma format parameters ======
  Void      setChromaFormat                 ( Int   i )      { m_iChromaFormat = i; }
  Void      setYUVStructure                 ( Int   i )      { m_iYUVStructure = i; }

  //==== Frequency Weighting Quantization ======
  Void      setWQEnable                     ( Bool  b )      { m_bWQEnable = b;                }
  Void      setSeqWQM                       ( Bool  b )      { m_bSeqWQM = b;                  }
  Void      setPicWQEnable                  ( Bool  b )      { m_bPicWQEnable = b;             }
  Void      setPicWQDataIndex               ( Int   i )      { m_iPicWQDataIndex = i;          }
  Void      setWQParam                      ( Int   i )      { m_iWQParam = i;                 }
  Void      setWQModel                      ( Int   i )      { m_iWQModel = i;                 }
  Void      setChromaDeltaQPDisable         ( Bool  b )      { m_bChromaDeltaQPDisable = b;    }
  Void      setChromaDeltaU                 ( Int   i )      { m_iChromaDeltaU = i;            }
  Void      setChromaDeltaV                 ( Int   i )      { m_iChromaDeltaV = i;            }
  Void      setChromaHDRDeltaQPDisable      ( Bool  b )      { m_bChromaHDRDeltaQPDisable = b; }
  Void      setChromaQPScale                ( Int   i )      { m_iChromaQPScale = i;           }
  Void      setChromaQPOffset               ( Int   i )      { m_iChromaQPOffset = i;          }
  Void      setCbQPScale                    ( Int   i )      { m_iCbQPScale = i;               }
  Void      setCrQPScale                    ( Int   i )      { m_iCrQPScale = i;               }

  //==== Delta QP and Adaptive Quantization ======
  Void      setDeltaQP                      ( Bool  b )      { m_bDeltaQP = b; }

  //==== ROI ======
  Void      setROICoding                    ( Int   i )      { m_iROICoding = i; }
  
  //==== Background Prediction ======
  Void      setBackgroundEnable             ( Bool  b )      { m_bBackgroundEnable = b; }
  Void      setBGInputNumber                ( Int   i )      { m_iBGInputNumber = i;    }
  Void      setBackgroundPeriod             ( Int   i )      { m_iBackgroundPeriod = i; }
  Void      setModelNumber                  ( Int   i )      { m_iModelNumber = i;      }
  Void      setBackgroundQP                 ( Int   i )      { m_iBackgroundQP = i;     }
  Void      setModelMethod                  ( Int   i )      { m_iModelMethod = i;      }

  //==== 3D Packing Mode ======
  Void      setTDMode                       ( Int   i )      { m_iTDMode = i;          }
  Void      setViewPackingMode              ( Int   i )      { m_iViewPackingMode = i; }
  Void      setViewReverse                  ( Int   i )      { m_iViewReverse = i;     }

  //==== unclear  parameter ======
  Void      setOutputMergedPicture          ( Int   i )      { m_iOutputMergedPicture = i;  }
  Void      setTemporalScalableFlag         ( Int   i )      { m_iTemporalScalableFlag = i; }

  //==== delete ======
  Void      setBGFastMode                   ( Int   i )      { m_iBGFastMode = i;     }
  Void      setSliceParameter               ( Int   i )      { m_iSliceParameter = i; }

  //==== RDOQ ======
  Void      setUseRDOQ                      ( Bool  b )      { m_bUseRDOQ = b;       }
  Void      setLambdaFactor                 ( Int   i )      { m_iLambdaFactor = i;  }
  Void      setLambdaFactorP                ( Int   i )      { m_iLambdaFactorP = i; }
  Void      setLambdaFactorB                ( Int   i )      { m_iLambdaFactorB = i; }

  //==== Refine QP ======
  Void      setRefineQP                     ( Bool  b )      { m_bRefineQP = b; }

  //==== TDRDO ======
  Void      setTDRDOEnable                  ( Bool  b )      { m_bTDRDOEnable = b; }

  //==== AQPO ======
  Void      setAQPOEnable                   ( Bool  b )      { m_bAQPOEnable = b; }

  //==== RATECONTROL ======
  Void      setRateControl                  ( Bool  b )      { m_bRateControl = b;   }
  Void      setTargetBitRate                ( Int   i )      { m_iTargetBitRate = i; }
  Void      setRCInitialQP                  ( Int   i )      { m_iRCInitialQP = i;   }

  //==== MD5 ======
  Void      setMD5Enable                    ( Bool  b )      { m_bMD5Enable = b; }

  //AVS3 get function
   //==== Files ========
  Int       getFrameSkip                   ()      { return m_iFrameSkip;            }
  Int       getFrameToBeEncoded            ()      { return m_iFrameToBeEncoded;     }
  Int       getSourceWidth                 ()      { return m_iSourceWidth;          }
  Int       getSourceHeight                ()      { return m_iSourceHeight;         }
  UInt      getInputSampleBitDepth         ()      { return m_uiInputSampleBitDepth; }
  UInt      getBitDepth                    ()      { return m_uiBitDepth;            }

  //Maximum Size
  UInt      getMaxCUSize                   ()      { return m_uiMaxCUSize; }
  //Encoder Control
  Int       getProfileId                    ()      { return  m_iProfileId;            }
  Int       getLevelId                      ()      { return m_iLevelId;               }
  Int       getIntraPeriod                  ()      { return m_iIntraPeriod;           }
  Int       getVECPeriod                    ()      { return  m_iVECPeriod;            }
  Int       getSeqHeaderPeriod              ()      { return m_iSeqHeaderPeriod;       }
  Int       getQPIFrame                     ()      { return m_iQPIFrame;              }
  Int       getQPPFrame                     ()      { return  m_iQPPFrame;             }
  Int       getQPBFrame                     ()      { return m_iQPBFrame;              }
  Bool      getUseHadamard                  ()      { return m_bUseHadamard;           }
  Bool      getFME                          ()      { return m_bFME;                   }
  Int       getSearchRange                  ()      { return  m_iSearchRange;          }
  Int       getNumberReferenceFrames        ()      { return m_iNumberReferenceFrames; }
  Bool      getInterSearch16x16             ()      { return m_bInterSearch16x16;      }
  Bool      getInterSearch16x8              ()      { return m_bInterSearch16x8;       }
  Bool      getInterSearch8x16              ()      { return m_bInterSearch8x16;       }
  Bool      getInterSearch8x8               ()      { return m_bInterSearch8x8;        }
  Bool      getUseASR                       ()      { return m_bUseASR;                }
  Bool      getInterSearchAMP               ()      { return m_bInterSearchAMP;        }
  Bool      getPicExtensionData             ()      { return m_bPicExtensionData;      }
  Bool      getOutPutEncPic                 ()      { return m_bOutPutEncPic;          }

  //F Frames
  Bool      getFFrameEnable                 ()      { return m_bFFrameEnable;          }
  Bool      getDHPEnable                    ()      { return m_bDHPEnable;             }
  Bool      getMHPSkipEnable                ()      { return m_bMHPSkipEnable;         }
  Bool      getWSMEnable                    ()      { return m_bWSMEnable;             }

  //B Frames
  Int       getNumberBFrames                ()      { return  m_iNumberBFrames;        }
  Int       getPFrameSkip                   ()      { return m_iPFrameSkip;            }
  Bool      getHierarchicalCoding           ()      { return m_bHierarchicalCoding;    }
#if IDR
  Bool      getIDREnable                    ()      { return m_bIDREnable;             }
#endif
  //RD Optimization
  Bool      getRDOptimization               ()      { return m_bRDOptimization; }
  //High Level Syntax
  Bool      getProgressiveSequence          ()      { return m_bProgressiveSequence;    }
  Bool      getProgressiveFrame             ()      { return m_bProgressiveFrame;       }
  Int       getInterlaceCodingOption        ()      { return  m_iInterlaceCodingOption; }
  Int       getRepeatFirstField             ()      { return m_iRepeatFirstField;       }
  Int       getTopFieldFirst                ()      { return m_iTopFieldFirst;          }
  Bool      getHDRMetaDataExtension         ()      { return m_bHDRMetaDataExtension;   }

  //Coding Tools
  Bool      getPMVREnable                   ()      { return m_bPMVREnable;             }
  Bool      getNSQT                         ()      { return m_bNSQT;                   }
  Bool      getSDIP                         ()      { return m_bSDIP;                   }
  Bool      getSECTEnable                   ()      { return m_bSECTEnable;             }
  Bool      getDMHEnableEncoder             ()      { return m_bDMHEnableEncoder;       }

  //Loop filter parameters
  Bool      getCrossSliceLoopFilter         ()      { return m_bCrossSliceLoopFilter;        }
  Bool      getLoopFilterDisable            ()      { return m_bLoopFilterDisable;           }
  Bool      getLoopFilterParameter          ()      { return m_bLoopFilterParameter;         }
  Int       getLoopFilterAlphaOffset        ()      { return m_iLoopFilterAlphaOffset;       }
  Int       getLoopFilterBetaOffset         ()      { return m_iLoopFilterBetaOffset;        }
  Bool      getSAOEnable                    ()      { return m_bSAOEnable;                   }
  Bool      getALFEnable                    ()      { return m_bALFEnable;                   }
  Bool      getALFLowLatencyEncodingEnable  ()      { return m_bALFLowLatencyEncodingEnable; }

  //Slice parameters
  Int       getNumberOfLCUsInSlice          ()      { return m_iNumberOfLCUsInSlice; }

  //Frame Rate
  Int       getFrameRate                    ()      { return m_iFrameRate; }

  //chroma format parameters
  Int       getChromaFormat                 ()      { return m_iChromaFormat; }
  Int       getYUVStructure                 ()      { return m_iYUVStructure; }
  //Frequency Weighting Quantization
  Bool      getWQEnable                     ()      { return m_bWQEnable;                }
  Bool      getSeqWQM                       ()      { return m_bSeqWQM;                  }
  Bool      getPicWQEnable                  ()      { return m_bPicWQEnable;             }
  Int       getPicWQDataIndex               ()      { return m_iPicWQDataIndex;          }
  Int       getWQParam                      ()      { return m_iWQParam;                 }
  Int       getWQModel                      ()      { return m_iWQModel;                 }
  Bool      getChromaDeltaQPDisable         ()      { return m_bChromaDeltaQPDisable;    }
  Int       getChromaDeltaU                 ()      { return m_iChromaDeltaU;            }
  Int       getChromaDeltaV                 ()      { return m_iChromaDeltaV;            }
  Bool      getChromaHDRDeltaQPDisable      ()      { return m_bChromaHDRDeltaQPDisable; }
  Int       getChromaQPScale                ()      { return m_iChromaQPScale;           }
  Int       getChromaQPOffset               ()      { return m_iChromaQPOffset;          }
  Int       getCbQPScale                    ()      { return m_iCbQPScale;               }
  Int       getCrQPScale                    ()      { return m_iCrQPScale;               }
  //Delta QP and Adaptive Quantization
  Bool      getDeltaQP                      ()      { return m_bDeltaQP; }
  //ROI
  Int       getROICoding                    ()      { return m_iROICoding; }
  //Background Prediction
  Bool      getBackgroundEnable             ()      { return m_bBackgroundEnable; }
  Int       getBGInputNumber                ()      { return m_iBGInputNumber;    }
  Int       getBackgroundPeriod             ()      { return m_iBackgroundPeriod; }
  Int       getModelNumber                  ()      { return  m_iModelNumber;     }
  Int       getBackgroundQP                 ()      { return m_iBackgroundQP;     }
  Int       getModelMethod                  ()      { return m_iModelMethod;      }
  
  //3D Packing Mode
  Int       getTDMode                       ()      { return  m_iTDMode;         }
  Int       getViewPackingMode              ()      { return m_iViewPackingMode; }
  Int       getViewReverse                  ()      { return m_iViewReverse;     }

  //unclear  parameter
  Int       getOutputMergedPicture          ()      { return m_iOutputMergedPicture;  }
  Int       getTemporalScalableFlag         ()      { return m_iTemporalScalableFlag; }
  //delete
  Int       getBGFastMode                   ()      { return m_iBGFastMode;     }
  Int       getSliceParameter               ()      { return m_iSliceParameter; }

  //RDOQ
  Bool      getUseRDOQ                      ()      { return m_bUseRDOQ;        }
  Int       getLambdaFactor                 ()      { return m_iLambdaFactor;   }
  Int       getLambdaFactorP                ()      { return m_iLambdaFactorP;  }
  Int       getLambdaFactorB                ()      { return  m_iLambdaFactorB; }
  //Refine QP  (Only used at RA configuration)
  Bool      getRefineQP                     ()      { return m_bRefineQP; }
  //TDRDO (only used at LD configuration)
  Bool      getTDRDOEnable                  ()      { return m_bTDRDOEnable; }
  //AQPO
  Bool      getAQPOEnable                   ()      { return m_bAQPOEnable; }
  //RATECONTROL
  Bool      getRateControl                  ()      { return m_bRateControl;   }
  Int       getTargetBitRate                ()      { return m_iTargetBitRate; }
  Int       getRCInitialQP                  ()      { return m_iRCInitialQP;   }

  //MD5
  Bool      getMD5Enable                    ()      { return m_bMD5Enable; }
#else
  Void      setFrameRate                    ( Int   i )      { m_iFrameRate = i;        }
  Void      setFrameSkip                    ( Int   i )      { m_iFrameSkip = i;        }
  Void      setSourceWidth                  ( Int   i )      { m_iSourceWidth = i;      }
  Void      setSourceHeight                 ( Int   i )      { m_iSourceHeight = i;     }
  Void      setFrameToBeEncoded             ( Int   i )      { m_iFrameToBeEncoded = i; }

  //====== Coding Structure ========
  Void      setIntraPeriod                  ( Int   i )      { m_uiIntraPeriod = (UInt)i; }

  //====== b; Picture ========
  Void      setHierarchicalCoding           ( Bool  b )      { m_bHierarchicalCoding = b; }

  //====== Loop/Deblock Filter ========
  Void      setLoopFilterDisable            ( Bool  b )      { m_bLoopFilterDisable = b; }

  //====== Motion search ========
  Void      setSearchRange                  ( Int   i )      { m_iSearchRange = i; }

  //====== Sequence ========
  Int       getFrameRate                    ()               { return  m_iFrameRate;        }
  Int       getFrameSkip                    ()               { return  m_iFrameSkip;        }
  Int       getSourceWidth                  ()               { return  m_iSourceWidth;      }
  Int       getSourceHeight                 ()               { return  m_iSourceHeight;     }
  Int       getFrameToBeEncoded             ()               { return  m_iFrameToBeEncoded; }
                                                          
  //==== Coding Structure ========
  UInt      getIntraPeriod                  ()               { return  m_uiIntraPeriod; }

  //==== b; Picture ========
  Bool      getHierarchicalCoding           ()               { return  m_bHierarchicalCoding; }
                                                             
  //==== Loop/Deblock Filter ========                        
  Bool      getLoopFilterDisable            ()               { return  m_bLoopFilterDisable;  }

  //====== Motion search ========
  Int       getSearchRange                  ()      { return  m_iSearchRange; }

  //==== Tool list ========
  Void      setUseASR                       ( Bool  b )      { m_bUseASR = b;         }
  Void      setUseRDOQ                      ( Bool  b )      { m_bUseRDOQ = b;        }
  Bool      getUseASR                       ()               { return m_bUseASR;      }
  Bool      getUseRDOQ                      ()               { return m_bUseRDOQ;     }
#endif
};

#endif // !defined(AFX_TENCCFG_H__6B99B797_F4DA_4E46_8E78_7656339A6C41__INCLUDED_)

