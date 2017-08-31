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

/** \file     TAppEncTop.cpp
    \brief    Encoder application class
*/

#include <list>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

#include "TAppEncTop.h"

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TAppEncTop::TAppEncTop()
{
  m_iFrameRcvd = 0;
}

TAppEncTop::~TAppEncTop()
{
}

Void TAppEncTop::xInitLibCfg()
{
  
  //====== Coding Structure ========
  m_cTEncTop.setGOPSize                      ( m_iGOPSize );
  m_cTEncTop.setRateGOPSize                  ( m_iRateGOPSize );
  m_cTEncTop.setQP                           ( m_iQP ); 
  m_cTEncTop.setCQP                          ( m_iCQP ); 
  m_cTEncTop.setPad                          ( m_aiPad );
  
  //====== Motion search ========
  m_cTEncTop.setFastSearch                   ( m_iFastSearch  );

  //====== Tool list ========
  m_cTEncTop.setUseHADME                     ( m_bUseHADME    );
  m_cTEncTop.setUseGPB                       ( m_bUseGPB      );
  m_cTEncTop.setdQPs                         ( m_aidQP        );
  m_cTEncTop.setUseLDC                       ( m_bUseLDC      );
  m_cTEncTop.setUsePAD                       ( m_bUsePAD      );
  
  m_cTEncTop.setLog2MaxPhTrSize              ( m_uiLog2MaxPhTrSize );

  m_cTEncTop.setUseNRF                       ( m_bUseNRF      );
  m_cTEncTop.setUseBQP                       ( m_bUseBQP      );
  m_cTEncTop.setUseFastEnc                   ( m_bUseFastEnc  );

#if ENUM_AND_MACRO_YUQUANHE
   //==== Files ========
  m_cTEncTop.setFrameSkip                    ( m_iFrameSkip            );
  m_cTEncTop.setFrameToBeEncoded             ( m_iFrameToBeEncoded     );
  m_cTEncTop.setSourceWidth                  ( m_iSourceWidth          );
  m_cTEncTop.setSourceHeight                 ( m_iSourceHeight         );
  m_cTEncTop.setInputSampleBitDepth          ( m_uiInputSampleBitDepth );
  m_cTEncTop.setBitDepth                     ( m_uiBitDepth            );

  //==== Maximum Size =======
  m_cTEncTop.setMaxCUSize                    ( m_uiMaxCUSize );

  //==== Encoder Control =======
  m_cTEncTop.setProfileId                    ( m_iProfileId             );
  m_cTEncTop.setLevelId                      ( m_iLevelId               );
  m_cTEncTop.setIntraPeriod                  ( m_iIntraPeriod           );
  m_cTEncTop.setVECPeriod                    ( m_iVECPeriod             );
  m_cTEncTop.setSeqHeaderPeriod              ( m_iSeqHeaderPeriod       );
  m_cTEncTop.setQPIFrame                     ( m_iQPIFrame              );
  m_cTEncTop.setQPPFrame                     ( m_iQPPFrame              );
  m_cTEncTop.setQPBFrame                     ( m_iQPBFrame              );
  m_cTEncTop.setUseHadamard                  ( m_bUseHadamard           );
  m_cTEncTop.setFME                          ( m_bFME                   );
  m_cTEncTop.setSearchRange                  ( m_iSearchRange           );
  m_cTEncTop.setNumberReferenceFrames        ( m_iNumberReferenceFrames );
  m_cTEncTop.setInterSearch16x16             ( m_bInterSearch16x16      );
  m_cTEncTop.setInterSearch16x8              ( m_bInterSearch16x8       );
  m_cTEncTop.setInterSearch8x16              ( m_bInterSearch8x16       );
  m_cTEncTop.setInterSearch8x8               ( m_bInterSearch8x8        );
  m_cTEncTop.setUseASR                       ( m_bUseASR                );
  m_cTEncTop.setPicExtensionData             ( m_bPicExtensionData      );
  m_cTEncTop.setOutPutEncPic                 ( m_bOutPutEncPic          );

  //==== F Frames ======
  m_cTEncTop.setFFrameEnable                 ( m_bFFrameEnable  );
  m_cTEncTop.setDHPEnable                    ( m_bDHPEnable     );
  m_cTEncTop.setMHPSkipEnable                ( m_bMHPSkipEnable );
  m_cTEncTop.setWSMEnable                    ( m_bWSMEnable     );

  //==== B Frames ======
  m_cTEncTop.setNumberBFrames                ( m_iNumberBFrames      );
  m_cTEncTop.setPFrameSkip                   ( m_iPFrameSkip         );
  m_cTEncTop.setHierarchicalCoding           ( m_bHierarchicalCoding );
#if IDR
  m_cTEncTop.setIDREnable                    ( m_bIDREnable          );
#endif
  //==== RD Optimization ======
  m_cTEncTop.setRDOptimization               ( m_bRDOptimization );

  //==== High Level Syntax ======
  m_cTEncTop.setProgressiveSequence          ( m_bProgressiveSequence   );
  m_cTEncTop.setProgressiveFrame             ( m_bProgressiveFrame      );
  m_cTEncTop.setInterlaceCodingOption        ( m_iInterlaceCodingOption );
  m_cTEncTop.setRepeatFirstField             ( m_iRepeatFirstField      );
  m_cTEncTop.setTopFieldFirst                ( m_iTopFieldFirst         );
  m_cTEncTop.setHDRMetaDataExtension         ( m_bHDRMetaDataExtension  );

  //==== Coding Tools ======
  m_cTEncTop.setPMVREnable                   ( m_bPMVREnable       );
  m_cTEncTop.setNSQT                         ( m_bNSQT             );
  m_cTEncTop.setSDIP                         ( m_bSDIP             );
  m_cTEncTop.setSECTEnable                   ( m_bSECTEnable       );
  m_cTEncTop.setDMHEnableEncoder             ( m_bDMHEnableEncoder );
#if RPS
  m_cTEncTop.setGopList(m_GOPList);
  m_cTEncTop.setExtraRPSs(m_iExtraRPSs);
  m_cTEncTop.setGOPSize(m_iGOPSize);
#endif
  //=== Loop filter parameters ======
  m_cTEncTop.setCrossSliceLoopFilter         ( m_bCrossSliceLoopFilter        );
  m_cTEncTop.setLoopFilterDisable            ( m_bLoopFilterDisable           );
  m_cTEncTop.setLoopFilterParameter          ( m_bLoopFilterParameter         );
  m_cTEncTop.setLoopFilterAlphaOffset        ( m_iLoopFilterAlphaOffset       );
  m_cTEncTop.setLoopFilterBetaOffset         ( m_iLoopFilterBetaOffset        );
  m_cTEncTop.setSAOEnable                    ( m_bSAOEnable                   );
  m_cTEncTop.setALFEnable                    ( m_bALFEnable                   );
  m_cTEncTop.setALFLowLatencyEncodingEnable  ( m_bALFLowLatencyEncodingEnable );

  //==== Slice parameters ======
  m_cTEncTop.setNumberOfLCUsInSlice          ( m_iNumberOfLCUsInSlice );

  //==== Frame Rate ======
  m_cTEncTop.setFrameRate                    ( m_iFrameRate );

  //==== chroma format parameters ======
  m_cTEncTop.setChromaFormat                 ( m_iChromaFormat );
  m_cTEncTop.setYUVStructure                 ( m_iYUVStructure );

  //==== Frequency Weighting Quantization ======
  m_cTEncTop.setWQEnable                     ( m_bWQEnable                );
  m_cTEncTop.setSeqWQM                       ( m_bSeqWQM                  );
  m_cTEncTop.setPicWQEnable                  ( m_bPicWQEnable             );
  m_cTEncTop.setPicWQDataIndex               ( m_iPicWQDataIndex          );
  m_cTEncTop.setWQParam                      ( m_iWQParam                 );
  m_cTEncTop.setWQModel                      ( m_iWQModel                 );
  m_cTEncTop.setChromaDeltaQPDisable         ( m_bChromaDeltaQPDisable    );
  m_cTEncTop.setChromaDeltaU                 ( m_iChromaDeltaU            );
  m_cTEncTop.setChromaDeltaV                 ( m_iChromaDeltaV            );
  m_cTEncTop.setChromaHDRDeltaQPDisable      ( m_bChromaHDRDeltaQPDisable );
  m_cTEncTop.setChromaQPScale                ( m_iChromaQPScale           );
  m_cTEncTop.setChromaQPOffset               ( m_iChromaQPOffset          );
  m_cTEncTop.setCbQPScale                    ( m_iCbQPScale               );
  m_cTEncTop.setCrQPScale                    ( m_iCrQPScale               );

  //==== Delta QP and Adaptive Quantization =====
  m_cTEncTop.setDeltaQP                      ( m_bDeltaQP );

  //=== ROI ======
  m_cTEncTop.setROICoding                    ( m_iROICoding );

  //=== Background Prediction ======
  m_cTEncTop.setBackgroundEnable             ( m_bBackgroundEnable );
  m_cTEncTop.setBGInputNumber                ( m_iBGInputNumber    );
  m_cTEncTop.setBackgroundPeriod             ( m_iBackgroundPeriod );
  m_cTEncTop.setModelNumber                  ( m_iModelNumber      );
  m_cTEncTop.setBackgroundQP                 ( m_iBackgroundQP     );
  m_cTEncTop.setModelMethod                  ( m_iModelMethod      );

  //=== 3D Packing Mode ======
  m_cTEncTop.setTDMode                       ( m_iTDMode          );
  m_cTEncTop.setViewPackingMode              ( m_iViewPackingMode );
  m_cTEncTop.setViewReverse                  ( m_iViewReverse     );

  //=== unclear  parameter ======
  m_cTEncTop.setOutputMergedPicture          ( m_iOutputMergedPicture  );
  m_cTEncTop.setTemporalScalableFlag         ( m_iTemporalScalableFlag );

  //==== delete ======
  m_cTEncTop.setBGFastMode                   ( m_iBGFastMode     );
  m_cTEncTop.setSliceParameter               ( m_iSliceParameter );

  //==== RDOQ ======
  m_cTEncTop.setUseRDOQ                      ( m_bUseRDOQ       );
  m_cTEncTop.setLambdaFactor                 ( m_iLambdaFactor  );
  m_cTEncTop.setLambdaFactorP                ( m_iLambdaFactorP );
  m_cTEncTop.setLambdaFactorB                ( m_iLambdaFactorB );

  //==== Refine QP ======
  m_cTEncTop.setRefineQP                     ( m_bRefineQP );

  //==== TDRDO ======
  m_cTEncTop.setTDRDOEnable                  ( m_bTDRDOEnable );

  //==== AQPO ======
  m_cTEncTop.setAQPOEnable                   ( m_bAQPOEnable );

  //==== RATECONTROL ======
  m_cTEncTop.setRateControl                  ( m_bRateControl   );
  m_cTEncTop.setTargetBitRate                ( m_iTargetBitRate );
  m_cTEncTop.setRCInitialQP                  ( m_iRCInitialQP   );

  //==== MD5 ======
  m_cTEncTop.setMD5Enable                    ( m_bMD5Enable );
#else
  m_cTEncTop.setFrameRate                    ( m_iFrameRate        );
  m_cTEncTop.setFrameSkip                    ( m_iFrameSkip        );
  m_cTEncTop.setSourceWidth                  ( m_iSourceWidth      );
  m_cTEncTop.setSourceHeight                 ( m_iSourceHeight     );
  m_cTEncTop.setFrameToBeEncoded             ( m_iFrameToBeEncoded );

  //====== Coding Structure ========
  m_cTEncTop.setIntraPeriod                  ( m_iIntraPeriod );

  //===== Picture ========
  m_cTEncTop.setHierarchicalCoding(m_bHierarchicalCoding);

  //====== Loop/Deblock Filter ========
  m_cTEncTop.setLoopFilterDisable(m_bLoopFilterDisable);

    //====== Motion search ========
  m_cTEncTop.setSearchRange                  ( m_iSearchRange );

    //====== Tool list ========
  m_cTEncTop.setUseASR                       ( m_bUseASR  );
  m_cTEncTop.setUseRDOQ                      ( m_bUseRDOQ );
#endif

}



Void TAppEncTop::xCreateLib()
{
  // Video I/O
  m_cTVideoIOYuvInputFile.open( m_pchInputFile,     false );  // read  mode
  m_cTVideoIOYuvReconFile.open( m_pchReconFile,     true  );  // write mode
  m_cTVideoIOBitsFile.openBits( m_pchBitstreamFile, true  );  // write mode
  
  // Neo Decoder
  m_cTEncTop.create();
}

Void TAppEncTop::xDestroyLib()
{
  // Video I/O
  m_cTVideoIOYuvInputFile.close();
  m_cTVideoIOYuvReconFile.close();
  m_cTVideoIOBitsFile.closeBits();
  
  // Neo Decoder
  m_cTEncTop.destroy();
}

Void TAppEncTop::xInitLib()
{
  m_cTEncTop.init();
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/**
 - create internal class
 - initialize internal variable
 - until the end of input YUV file, call encoding function in TEncTop class
 - delete allocated buffers
 - destroy internal class
 .
 */
Void TAppEncTop::encode()
{
  TComPicYuv*       pcPicYuvOrg = new TComPicYuv;
  TComPicYuv*       pcPicYuvRec = NULL;
  TComBitstream*    pcBitstream = NULL;
  
  // initialize internal class & member variables
  xInitLibCfg();
  xCreateLib();
  xInitLib();
  
  // main encoder loop
  Int   iNumEncoded = 0;
  Bool  bEos = false;
  
  // allocate original YUV buffer
  pcPicYuvOrg->create( m_iSourceWidth, m_iSourceHeight, m_uiLog2MaxCUSize, m_uiMaxCUDepth );
  
  while ( !bEos )
  {
    // get buffers
    xGetBuffer( pcPicYuvRec, pcBitstream );
    
    // read input YUV file
    m_cTVideoIOYuvInputFile.read( pcPicYuvOrg, m_aiPad );
    
    // increase number of received frames
    m_iFrameRcvd++;
    
    // check end of file
    bEos = ( m_cTVideoIOYuvInputFile.isEof() == 1 ?   true : false  );
    bEos = ( m_iFrameRcvd == m_iFrameToBeEncoded ?    true : bEos   );
    
    // call encoding function for one frame
    m_cTEncTop.encode( bEos, pcPicYuvOrg, m_cListPicYuvRec, m_cListBitstream, iNumEncoded );
    
    // write bistream to file if necessary
    if ( iNumEncoded > 0 )
    {
      xWriteOutput( iNumEncoded );
    }
  }
  // delete original YUV buffer
  pcPicYuvOrg->destroy();
  delete pcPicYuvOrg;
  pcPicYuvOrg = NULL;
  
  // delete used buffers in encoder class
  m_cTEncTop.deletePicBuffer();
  
  // delete buffers & classes
  xDeleteBuffer();
  xDestroyLib();
  
  return;
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

/**
 - application has picture buffer list with size of GOP
 - picture buffer list acts as ring buffer
 - end of the list has the latest picture
 .
 */
Void TAppEncTop::xGetBuffer( TComPicYuv*& rpcPicYuvRec, TComBitstream*& rpcBitStream )
{
  if ( m_iGOPSize == 0 )
  {
    if (m_cListPicYuvRec.size() == 0)
    {
      rpcPicYuvRec = new TComPicYuv;
      rpcBitStream = new TComBitstream;
      
      rpcPicYuvRec->create( m_iSourceWidth, m_iSourceHeight, m_uiLog2MaxCUSize, m_uiMaxCUDepth );
      rpcBitStream->create( (m_iSourceWidth * m_iSourceHeight * 3) >> 1 );
      
      m_cListPicYuvRec.pushBack( rpcPicYuvRec );
      m_cListBitstream.pushBack( rpcBitStream );
    }
    
    rpcPicYuvRec = m_cListPicYuvRec.popFront();
    rpcBitStream = m_cListBitstream.popFront();
    
    m_cListPicYuvRec.pushBack( rpcPicYuvRec );
    m_cListBitstream.pushBack( rpcBitStream );
    
    return;
  }
  
  // org. buffer
  if ( m_cListPicYuvRec.size() == (UInt)m_iGOPSize )
  {
    rpcPicYuvRec = m_cListPicYuvRec.popFront();
    rpcBitStream = m_cListBitstream.popFront();
    
    rpcBitStream->rewindStreamPacket();
  }
  else
  {
    rpcPicYuvRec = new TComPicYuv;
    rpcBitStream = new TComBitstream;
    
    rpcPicYuvRec->create( m_iSourceWidth, m_iSourceHeight, m_uiLog2MaxCUSize, m_uiMaxCUDepth );
    rpcBitStream->create( (m_iSourceWidth * m_iSourceHeight * 3) >> 1 );
  }
  m_cListPicYuvRec.pushBack( rpcPicYuvRec );
  m_cListBitstream.pushBack( rpcBitStream );
}

Void TAppEncTop::xDeleteBuffer( )
{
  TComList<TComPicYuv*>::iterator iterPicYuvRec  = m_cListPicYuvRec.begin();
  TComList<TComBitstream*>::iterator iterBitstream = m_cListBitstream.begin();
  
  Int iSize = Int( m_cListPicYuvRec.size() );
  
  for ( Int i = 0; i < iSize; i++ )
  {
    TComPicYuv*  pcPicYuvRec  = *(iterPicYuvRec++);
    TComBitstream* pcBitstream = *(iterBitstream++);
    
    pcPicYuvRec->destroy();
    pcBitstream->destroy();
    
    delete pcPicYuvRec; pcPicYuvRec = NULL;
    delete pcBitstream; pcBitstream = NULL;
  }
  
}

/** \param iNumEncoded  number of encoded frames
 */
Void TAppEncTop::xWriteOutput( Int iNumEncoded )
{
  Int i;
  
  TComList<TComPicYuv*>::iterator iterPicYuvRec = m_cListPicYuvRec.end();
  TComList<TComBitstream*>::iterator iterBitstream = m_cListBitstream.begin();
  
  for ( i = 0; i < iNumEncoded; i++ )
  {
    --iterPicYuvRec;
  }
  
  for ( i = 0; i < iNumEncoded; i++ )
  {
    TComPicYuv*  pcPicYuvRec  = *(iterPicYuvRec++);
    TComBitstream* pcBitstream = *(iterBitstream++);
    
    m_cTVideoIOYuvReconFile.write( pcPicYuvRec, m_aiPad );
    m_cTVideoIOBitsFile.writeBits( pcBitstream );
  }
}

