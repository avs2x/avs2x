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

/** \file     TComPicture.h
    \brief    picture header and SPS class (header)
*/

#ifndef __TCOMPICTURE__
#define __TCOMPICTURE__


#include "CommonDef.h"
#include "TComList.h"
#if RPS
#include<vector>
#endif
class TComPic;
#if RPS
/// Reference Picture Set class
class TComReferencePictureSet
{
private:
	Int  m_iNumberOfRefPictures;
	Int  m_iNumberOfNegativeRefPictures;
	Int  m_iNumberOfPositiveRefPictures;
	Int  m_iNumberOfRemPictures;
	Int  m_iRefDeltaPOC[MAX_NUM_REF_PICS];//当前frame POC 与参考SET集合里的REC POC的差
	Int  m_iRemDeltaPOC[MAX_NUM_REM_PICS];//移出frame delta poc
	Int  m_iRefPOC[MAX_NUM_REF_PICS]; //REF POC
	Int  m_iRemPOC[MAX_NUM_REM_PICS];
	Bool m_bRefered;
public:
	TComReferencePictureSet();
	virtual ~TComReferencePictureSet();

	Void    setRefDeltaPOC(Int bufferNum, Int RefDeltaPOC);//参考
	Void    setRemDeltaPOC(Int bufferNum, Int RemDeltaPOC);//移出
	Void    setRefPOC(Int bufferNum, Int RefDeltaPOC);
	Void    setRemPOC(Int bufferNum, Int RemDeltaPOC);
	Void    setNumberOfRefPictures(Int numberOfRefPictures);
	Void    setNumberOfRemPictures(Int numberOfRemPictures);
	Void    setNumberOfNegativeRefPictures(Int numberOfNegativeRefPictures);
	Void    setNumberOfPositiveRefPictures(Int numberOfPositiveRefPictures);
	Int     getRefDeltaPOC(Int bufferNum) const;
	Int     getRemDeltaPOC(Int bufferNum) const;
	Int     getRefPOC(Int bufferNum) const;
	Int     getRemPOC(Int bufferNum) const;
	Int     getNumberOfRefPictures() const;
	Int     getNumberOfRemPictures() const;
	Int     getNumberOfNegativeRefPictures()const;;
	Int     getNumberOfPositiveRefPictures()const;;


	Void     setRefered(Bool b)                            { m_bRefered = b; }
	Bool     getRefered() const                { return m_bRefered; }
};

///Reference Picture Set 的集合 MAX = 7  RD 18.0 max=16
class TComRPSList
{
private:
	std::vector<TComReferencePictureSet> m_referencePictureSets;

public:
	TComRPSList()                                           { }
	virtual                        ~TComRPSList()                                           { }

	Void                           create(Int numberOfEntries)                            { m_referencePictureSets.resize(numberOfEntries); }
	Void                           destroy()                                               { }


	TComReferencePictureSet*       getReferencePictureSet(Int referencePictureSetNum)       { return &m_referencePictureSets[referencePictureSetNum]; }
	const TComReferencePictureSet* getReferencePictureSet(Int referencePictureSetNum) const { return &m_referencePictureSets[referencePictureSetNum]; }

	Int                            getNumberOfReferencePictureSets() const                  { return Int(m_referencePictureSets.size()); }
};

#endif
class TComPic;

// ====================================================================================================================
// Class definition
// ====================================================================================================================
#if AVS3_EXTENSION_LWJ 
class TComUserData
{
private:
  UInt	m_uiUserDataStartCode;
  UInt	m_uiUserData;

public:
  TComUserData();
  virtual ~TComUserData();

  UInt getUserDataStartCode()						{ return m_uiUserDataStartCode; }
  Void setUserDataStartCode(UInt ui)				{ m_uiUserDataStartCode = ui; }
  UInt getUserData()								{ return m_uiUserData; }
  Void setUserData(UInt ui)						{ m_uiUserData = ui; }

};

class TComExtSeqDisplay
{
private:
  UInt	m_uiExtId;
  UInt	m_uiVideoFormat;
  UInt	m_uiSampleRange;
  UInt	m_uiColourDescription;
  UInt	m_uiColourPrimaries;
  UInt	m_uiTransferCharacteristics;
  UInt	m_uiMatrixCoeff;
  UInt	m_uiDisplayHorSize;
  UInt	m_uiDisplayVerSize;
  Bool    m_bTdModeFlag;
  UInt	m_uiTdPackingMode;
  Bool    m_bViewReverseFlag;

public:
  TComExtSeqDisplay();
  virtual ~TComExtSeqDisplay();

  UInt getExtId()											{ return m_uiExtId; }
  Void setExtId(UInt ui)									{ m_uiExtId = ui; }
  UInt getVideoFormat()									{ return m_uiVideoFormat; }
  Void setVideoFormat(UInt ui)							{ m_uiVideoFormat = ui; }
  UInt getSampleRange()									{ return m_uiSampleRange; }
  Void setSampleRange(UInt ui)							{ m_uiSampleRange = ui; }
  UInt getColourDescription()								{ return m_uiColourDescription; }
  Void setColourDescription(UInt ui)						{ m_uiColourDescription = ui; }
  UInt getColourPrimaries()								{ return m_uiColourPrimaries; }
  Void setColourPrimaries(UInt ui)						{ m_uiColourPrimaries = ui; }
  UInt getTransferCharacteristics()						{ return m_uiTransferCharacteristics; }
  Void setTransferCharacteristics(UInt ui)				{ m_uiTransferCharacteristics = ui; }
  UInt getMatrixCoeff()									{ return m_uiMatrixCoeff; }
  Void setMatrixCoeff(UInt ui)							{ m_uiMatrixCoeff = ui; }
  UInt getDisplayHorSize()								{ return m_uiDisplayHorSize; }
  Void setDisplayHorSize(UInt ui)							{ m_uiDisplayHorSize = ui; }
  UInt getDisplayVerSize()								{ return m_uiDisplayVerSize; }
  Void setDisplayVerSize(UInt ui)							{ m_uiDisplayVerSize = ui; }
  Bool getTdModeFlag()									{ return m_bTdModeFlag; }
  Void setTdModeFlag(Bool b)								{ m_bTdModeFlag = b; }
  UInt getTdPackingMode()									{ return m_uiTdPackingMode; }
  Void setTdPackingMode(UInt ui)							{ m_uiTdPackingMode = ui; }
  Bool getViewReverseFlag()								{ return m_bViewReverseFlag; }
  Void setViewReverseFlag(Bool b)							{ m_bViewReverseFlag = b; }

};

class TComExtScalability
{
private:
  UInt	m_uiExtId;
  UInt	m_uiNumOfTemporalLevelMinus1;
  UInt	m_uiTemporalFrameRateCode[MAX_TEMPORAL_ID];
  UInt	m_uiTemporalBitRateLower[MAX_TEMPORAL_ID];
  UInt	m_uiTemporalBitRateUpper[MAX_TEMPORAL_ID];

public:
  TComExtScalability();
  virtual ~TComExtScalability();

  UInt getExtId()											{ return m_uiExtId; }
  Void setExtId(UInt ui)									{ m_uiExtId = ui; }
  UInt getNumOfTemporalLevelMinus1()						{ return m_uiNumOfTemporalLevelMinus1; }
  Void setNumOfTemporalLevelMinus1(UInt ui)				{ m_uiNumOfTemporalLevelMinus1 = ui; }
  UInt getTemporalFrameRateCode(Int Idx)                  { return m_uiTemporalFrameRateCode[Idx]; }
  Void setTemporalFrameRateCode(Int Idx, UInt	ui)         { m_uiTemporalFrameRateCode[Idx] = ui; }
  UInt getTemporalBitRateLower(Int Idx)                   { return m_uiTemporalBitRateLower[Idx]; }
  Void setTemporalBitRateLower(Int Idx, UInt	ui)         { m_uiTemporalBitRateLower[Idx] = ui; }
  UInt getTemporalBitRateUpper(Int Idx)                   { return m_uiTemporalBitRateUpper[Idx]; }
  Void setTemporalBitRateUpper(Int Idx, UInt	ui)         { m_uiTemporalBitRateUpper[Idx] = ui; }

};
#endif
#if AVS3_EXTENSION_CYH
class TComExtCopyright
{
private:
  UInt	m_uiExtId;
  Bool	m_bCopyrightFlag;
  UInt	m_uiCopyrightId;
  Bool	m_bOriginalOrCopy;
  UInt	m_uiCopyrightNum1;
  UInt	m_uiCopyrightNum2;
  UInt	m_uiCopyrightNum3;
public:
  TComExtCopyright();
  virtual ~TComExtCopyright();

  UInt getExtId()                                { return m_uiExtId; }
  Void setExtId(UInt ui)                         { m_uiExtId = ui; }
  Bool getCopyrightFlag()						   { return m_bCopyrightFlag; }
  Void setCopyrightFlag(Bool b)				   { m_bCopyrightFlag = b; }
  UInt getCopyrightId()                          { return m_uiCopyrightId; }
  Void setCopyrightId(UInt ui)                   { m_uiCopyrightId = ui; }
  Bool getOriginalOrCopy()					   { return m_bOriginalOrCopy; }
  Void setOriginalOrCopy(Bool b)				   { m_bOriginalOrCopy = b; }
  UInt getCopyrightNum1()                        { return m_uiCopyrightNum1; }
  Void setCopyrightNum1(UInt ui)                 { m_uiCopyrightNum1 = ui; }
  UInt getCopyrightNum2()                        { return m_uiCopyrightNum2; }
  Void setCopyrightNum2(UInt ui)                 { m_uiCopyrightNum2 = ui; }
  UInt getCopyrightNum3()                        { return m_uiCopyrightNum3; }
  Void setCopyrightNum3(UInt ui)                 { m_uiCopyrightNum3 = ui; }
};
class TComExtCameraPra
{
private:
  UInt	m_uiExtId;
  UInt	m_uiCameraId;
  UInt	m_uiHeightOfImageDevice;
  UInt	m_uiFocalLenth;
  UInt	m_uiFNum;
  UInt	m_uiVerAngleOfView;
  UInt	m_uiCameraPosXUpper;
  UInt	m_uiCameraPosXLower;
  UInt	m_uiCameraPosYUpper;
  UInt	m_uiCameraPosYLower;
  UInt	m_uiCameraPosZUpper;
  UInt	m_uiCameraPosZLower;
  UInt	m_uiCameraDirX;
  UInt	m_uiCameraDirY;
  UInt	m_uiCameraDirZ;
  UInt	m_uiImagePlaneVerX;
  UInt	m_uiImagePlaneVerY;
  UInt	m_uiImagePlaneVerZ;
public:
  TComExtCameraPra();
  virtual ~TComExtCameraPra();

  UInt getExtId()                                { return m_uiExtId; }
  Void setExtId(UInt ui)                         { m_uiExtId = ui; }
  UInt getCameraId()                             { return m_uiCameraId; }
  Void setCameraId(UInt ui)                      { m_uiCameraId = ui; }
  UInt getHeightOfImageDevice()                  { return m_uiHeightOfImageDevice; }
  Void setHeightOfImageDevice(UInt ui)           { m_uiHeightOfImageDevice = ui; }
  UInt getFocalLenth()                           { return m_uiFocalLenth; }
  Void setFocalLenth(UInt ui)                    { m_uiFocalLenth = ui; }
  UInt getFNum()                                 { return m_uiFNum; }
  Void setFNum(UInt ui)                          { m_uiFNum = ui; }
  UInt getVerAngleOfView()                       { return m_uiVerAngleOfView; }
  Void setVerAngleOfView(UInt ui)                { m_uiVerAngleOfView = ui; }
  UInt getCameraPosXUpper()                      { return m_uiCameraPosXUpper; }
  Void setCameraPosXUpper(UInt ui)               { m_uiCameraPosXUpper = ui; }
  UInt getCameraPosXLower()                      { return m_uiCameraPosXLower; }
  Void setCameraPosXLower(UInt ui)               { m_uiCameraPosXLower = ui; }
  UInt getCameraPosYUpper()                      { return m_uiCameraPosYUpper; }
  Void setCameraPosYUpper(UInt ui)               { m_uiCameraPosYUpper = ui; }
  UInt getCameraPosYLower()                      { return m_uiCameraPosYLower; }
  Void setCameraPosYLower(UInt ui)               { m_uiCameraPosYLower = ui; }
  UInt getCameraPosZUpper()                      { return m_uiCameraPosZUpper; }
  Void setCameraPosZUpper(UInt ui)               { m_uiCameraPosZUpper = ui; }
  UInt getCameraPosZLower()                      { return m_uiCameraPosZLower; }
  Void setCameraPosZLower(UInt ui)               { m_uiCameraPosZLower = ui; }
  UInt getCameraDirX()                           { return m_uiCameraDirX; }
  Void setCameraDirX(UInt ui)                    { m_uiCameraDirX = ui; }
  UInt getCameraDirY()                           { return m_uiCameraDirY; }
  Void setCameraDirY(UInt ui)					   { m_uiCameraDirY = ui; }
  UInt getCameraDirZ()						   { return m_uiCameraDirZ; }
  Void setCameraDirZ(UInt ui)				       { m_uiCameraDirZ = ui; }
  UInt getImagePlaneVerX()                       { return m_uiImagePlaneVerX; }
  Void setImagePlaneVerX(UInt ui)                { m_uiImagePlaneVerX = ui; }
  UInt getImagePlaneVerY()                       { return m_uiImagePlaneVerY; }
  Void setImagePlaneVerY(UInt ui)                { m_uiImagePlaneVerY = ui; }
  UInt getImagePlaneVerZ()                       { return m_uiImagePlaneVerZ; }
  Void setImagePlaneVerZ(UInt ui)                { m_uiImagePlaneVerZ = ui; }

};
#endif
#if AVS3_EXTENSION_LYT
struct ROI
{
  Int	asisx;
  Int	asisy;
  Int width;
  Int	height;
};

class TComExtRoiPra
{
private:
  UInt	m_uiExtId;
  UInt	m_uiCurrPicRoiNum;
  UInt	m_uiPrevPicRoiNum;
  UInt	m_uiRoiSkipRun;
  UInt*	m_puiSkipRoiMode;
  UInt	m_uiRoiAxisxDelta;
  UInt	m_uiRoiAxisyDelta;
  UInt	m_uiRoiWidthDelta;
  UInt	m_uiRoiHeightDelta;
  UInt	m_uiRoiAxisx;
  UInt	m_uiRoiAxisy;
  UInt	m_uiRoiWidth;
  UInt	m_uiRoiHeight;
  ROI*	ROIInfo;
  ROI*	PrevROIInfo;
public:
  TComExtRoiPra();
  ~TComExtRoiPra();

  UInt	getExtId()			{ return m_uiExtId; }
  Void	setExtId(UInt ui)	{ m_uiExtId = ui; }
  UInt	getCurrPicRoiNum()			{ return m_uiCurrPicRoiNum; }
  Void	setCurrPicRoiNum(UInt ui)	{ m_uiCurrPicRoiNum = ui; }
  UInt	getPrevPicRoiNum()			{ return m_uiPrevPicRoiNum; }
  Void	setPrevPicRoiNum(UInt ui)	{ m_uiPrevPicRoiNum = ui; }
  UInt	getRoiSkipRun()			{ return m_uiRoiSkipRun; }
  Void	setRoiSkipRun(UInt ui)	{ m_uiRoiSkipRun = ui; }
  UInt*	getSkipRoiMode()			{ return m_puiSkipRoiMode; }
  Void	setSkipRoiMode(UInt ui, Int i)	{ *(m_puiSkipRoiMode + i) = ui; }
  UInt	getRoiAxisxDelta()			{ return m_uiRoiAxisxDelta; }
  Void	setRoiAxisxDelta(UInt ui)	{ m_uiRoiAxisxDelta = ui; }
  UInt	getRoiAxisyDelta()			{ return m_uiRoiAxisyDelta; }
  Void	setRoiAxisyDelta(UInt ui)	{ m_uiRoiAxisyDelta = ui; }
  UInt	getRoiWidthDelta()			{ return m_uiRoiWidthDelta; }
  Void	setRoiWidthDelta(UInt ui)	{ m_uiRoiWidthDelta = ui; }
  UInt	getRoiHeightDelta()			{ return m_uiRoiHeightDelta; }
  Void	setRoiHeightDelta(UInt ui)	{ m_uiRoiHeightDelta = ui; }
  UInt	getRoiAxisx()			{ return m_uiRoiAxisx; }
  Void	setRoiAxisx(UInt ui)	{ m_uiRoiAxisx = ui; }
  UInt	getRoiAxisy()			{ return m_uiRoiAxisy; }
  Void	setRoiAxisy(UInt ui)	{ m_uiRoiAxisy = ui; }
  UInt	getRoiWidth()			{ return m_uiRoiWidth; }
  Void	setRoiWidth(UInt ui)	{ m_uiRoiWidth = ui; }
  UInt	getRoiHeight()			{ return m_uiRoiHeight; }
  Void	setRoiHeight(UInt ui)	{ m_uiRoiHeight = ui; }
  ROI*	getROIInfo()			{ return ROIInfo; }
  ROI*	getPrevROIInfo()			{ return PrevROIInfo; }
};

class TComExtMetadata
{
private:
  UInt	m_uiExtId;
  UInt	m_uiDisplayPrimariesX[3];
  UInt	m_uiDisplayPrimariesY[3];
  UInt	m_uiWhitePointX;
  UInt	m_uiWhitePointY;
  UInt	m_uiMaxDisplayMasteringLum;
  UInt	m_uiMinDisplayMasteringLum;
  UInt	m_uiMaxContentLightLevel;
  UInt	m_uiMaxPicAverageLightLevel;
public:
  TComExtMetadata();
  virtual ~TComExtMetadata();

  UInt	getExtId()					{ return m_uiExtId; }
  Void	setExtId(UInt ui)			{ m_uiExtId = ui; }
  UInt	getDisplayPrimariesX(UInt ui)			{ return m_uiDisplayPrimariesX[ui]; }
  Void	setDisplayPrimariesX(UInt ui, UInt u)	{ m_uiDisplayPrimariesX[ui] = u; }
  UInt	getDisplayPrimariesY(UInt ui)			{ return m_uiDisplayPrimariesY[ui]; }
  Void	setDisplayPrimariesY(UInt ui, UInt u)	{ m_uiDisplayPrimariesY[ui] = u; }
  UInt	getWhitePointX()					{ return m_uiWhitePointX; }
  Void	setWhitePointX(UInt ui)			{ m_uiWhitePointX = ui; }
  UInt	getWhitePointY()					{ return m_uiWhitePointY; }
  Void	setWhitePointY(UInt ui)			{ m_uiWhitePointY = ui; }
  UInt	getMaxDisplayMasteringLum()					{ return m_uiMaxDisplayMasteringLum; }
  Void	setMaxDisplayMasteringLum(UInt ui)			{ m_uiMaxDisplayMasteringLum = ui; }
  UInt	getMinDisplayMasteringLum()					{ return m_uiMinDisplayMasteringLum; }
  Void	setMinDisplayMasteringLum(UInt ui)			{ m_uiMinDisplayMasteringLum = ui; }
  UInt	getMaxContentLightLevel()					{ return m_uiMaxContentLightLevel; }
  Void	setMaxContentLightLevel(UInt ui)			{ m_uiMaxContentLightLevel = ui; }
  UInt	getMaxPicAverageLightLevel()					{ return m_uiMaxPicAverageLightLevel; }
  Void	setMaxPicAverageLightLevel(UInt ui)			{ m_uiMaxPicAverageLightLevel = ui; }
};

class TComExtPicDisplay
{
private:
  UInt	m_uiExtId;
  UInt	m_uiPicCentreHorOffset;
  UInt	m_uiPicCentreVerOffset;
public:
  TComExtPicDisplay();
  ~TComExtPicDisplay();

  UInt	getExtId()			{ return m_uiExtId; }
  Void	setExtId(UInt ui)	{ m_uiExtId = ui; }
  UInt	getPicCentreHorOffset()			{ return m_uiPicCentreHorOffset; }
  Void	setPicCentreHorOffset(UInt ui)	{ m_uiPicCentreHorOffset = ui; }
  Int		getPicCentreVerOffset()			{ return m_uiPicCentreVerOffset; }
  Void	setPicCentreVerOffset(UInt ui)	{ m_uiPicCentreVerOffset = ui; }
};
#endif
/// SPS class
class TComSPS
{
private:
#if AVS3_SEQ_HEADER_SYC
	UInt	        m_uiVideoSequenceStartCode;
	Profile::Name     	m_eProfileId;
	Level::Name	        m_eLevelId;
	Bool	        m_bProgressiveSequence;
	PictureTypeInSequence	   m_eFieldCodedSequence;
	UInt	        m_uiHorizontalSize;
	UInt	        m_uiVerticalSize;
	ChromaFormat	m_eChromaFormat;
	UInt	        m_uiSamplePrecision;
	UInt	        m_uiEncodingPrecision;
	UInt	        m_uiAspectRatio;
	UInt	        m_uiFrameRateCode;
	UInt	        m_uiBitRateLower;
	UInt	        m_uiBitRateUpper;
	Bool	        m_bLowDelay;
	Bool	        m_bTemporalIdEnableFlag;
	UInt	        m_uiBbvBufferSize;
	UInt	        m_uiLCUSize;
	Bool	        m_bWeightQuantEnableFlag;
	Bool	        m_bLoadSeqWeightQuantDataFlag;
	UInt	        m_uiWeightQuantCoeff4x4[4][4];
	UInt	        m_uiWeightQuantCoeff8x8[8][8];
	Bool	        m_bScenePictureDisableFlag;
	Bool	        m_bMultiHypothesisSkipEnableFlag;
	Bool	        m_bDualHypothesisPredictionEnableFlag;
	Bool	        m_bWeightedSkipEnableFlag;
	Bool	        m_bAsymmetricMotionPartitionsEnableFlag;
	Bool	        m_bNonsquareQuadtreeTansformEnableFlag;
	Bool	        m_bNonsquareIntraPredictionEnableFlag;
	Bool	        m_bSecondaryTransformEnableFlag;
	Bool	        m_bSampleAdaptiveOffsetEnableFlag;
	Bool	        m_bAdaptiveLoopFilterEnableFlag;
	Bool	        m_bPmvrEnableFlag;
	UInt	        m_uiNumOfRcs;
	Bool	        m_bReferedByOtherFlag[MAX_NUM_OF_RCS];
	UInt	        m_uiNumOfReferencePicture[MAX_NUM_OF_RCS];
	UInt	        m_uiDeltaDoiOfReferencePicture[MAX_NUM_OF_RCS][MAX_NUM_REF_PICS];
	UInt	        m_uiNumOfRemovedPicture[MAX_NUM_OF_RCS];
	UInt	        m_uiDeltaDoiOfRemovedPicture[MAX_NUM_OF_RCS][MAX_NUM_REF_PICS];
	UInt	        m_uiOutputReorderDelay;
	Bool	        m_bCrossSliceLoopfilterEnableFlag;
#if rd_sym
	Bool          m_bBackgroundEnable;
#endif
#endif

  //extention
#if AVS3_EXTENSION_LWJ
  TComUserData*				m_pcUserData;
  TComExtSeqDisplay*			m_pcExtSeqDisplay;
  TComExtScalability*			m_pcExtScalability;
#endif
#if AVS3_EXTENSION_CYH
  TComExtCopyright*			m_pcExtCopyright;
  TComExtCameraPra*			m_pcExtCameraPra;
#endif
#if AVS3_EXTENSION_LYT
  TComExtMetadata*	m_pcExtMetadata;
#endif

  // Structure
  UInt        m_uiWidth;
  UInt        m_uiHeight;
  Int         m_aiPad[2];
  UInt        m_uiLog2MaxCUSize;
  UInt        m_uiMaxCUDepth;
#if RPS
  TComRPSList      m_RPSList;
#if RPS_BUG
  UInt        m_uiGOPSize;
  Int         m_iIntraPeriod;
#endif
#endif
#if LAMBDA_BUG
  Int m_iPictureCodingOrder;
#endif
#if INTER614
  Int         m_iNumberReferenceFrames;
#endif
  // Tool list
  UInt        m_uiLog2MaxPhTrSize;
  Bool        m_bUseDQP;
  Bool        m_bUseLDC;
  Bool        m_bUsePAD;

  // Parameter
  UInt        m_uiBitDepth;
  
public:
  TComSPS();
  virtual ~TComSPS();
  // avs3
#if AVS3_SEQ_HEADER_SYC
  Void setVideoSequenceStartCode                   ( UInt	ui )                       { m_uiVideoSequenceStartCode = ui; }
  UInt getVideoSequenceStartCode                   ()                                { return m_uiVideoSequenceStartCode; }

  Void setProfileId                                ( Profile::Name profile )         { m_eProfileId = profile; }
  Profile::Name getProfileId                       ()                                { return m_eProfileId; }
  Void setLevelId                                  ( Level::Name level )             { m_eLevelId = level; }
  Level::Name getLevelId                           ()                                { return m_eLevelId; }

  Void setProgressiveSequence                      ( Bool	b )                        { m_bProgressiveSequence = b; }
  Bool getProgressiveSequence                      ()                                { return m_bProgressiveSequence; }
  Void setFieldCodedSequence                       ( PictureTypeInSequence	b )      { m_eFieldCodedSequence = b; }
  PictureTypeInSequence getFieldCodedSequence      ()                                { return m_eFieldCodedSequence; }
  Void setHorizontalSize                           ( UInt	ui )                       { m_uiHorizontalSize = ui; }
  UInt getHorizontalSize                           ()                                { return m_uiHorizontalSize; }
  Void setVerticalSize                             ( UInt	ui )                       { m_uiVerticalSize = ui; }
  UInt getVerticalSize                             ()                                { return m_uiVerticalSize; }
  Void setChromaFormat                             ( ChromaFormat chromaformat )     { m_eChromaFormat = chromaformat; }
  ChromaFormat getChromaFormat                     ()                                { return m_eChromaFormat; }
  Void setSamplePrecision                          ( UInt	ui )                       { m_uiSamplePrecision = ui; }
  UInt getSamplePrecision                          ()                                { return m_uiSamplePrecision; }
  Void setEncodingPrecision                        ( UInt	ui )                       { m_uiEncodingPrecision = ui; }
  UInt getEncodingPrecision                        ()                                { return m_uiEncodingPrecision; }
  Void setAspectRatio                              ( UInt	ui )                       { m_uiAspectRatio = ui; }
  UInt getAspectRatio                              ()                                { return m_uiAspectRatio; }
  Void setFrameRateCode                            ( UInt	ui )                       { m_uiFrameRateCode = ui; }
  UInt getFrameRateCode                            ()                                { return m_uiFrameRateCode; }
  Void setBitRateLower                             ( UInt	ui )                       { m_uiBitRateLower = ui; }
  UInt getBitRateLower                             ()                                { return m_uiBitRateLower; }
  Void setBitRateUpper                             ( UInt	ui )                       { m_uiBitRateUpper = ui; }
  UInt getBitRateUpper                             ()                                { return m_uiBitRateUpper; }
  Void setLowDelay                                 ( Bool	b )                        { m_bLowDelay = b; }
  Bool getLowDelay                                 ()                                { return m_bLowDelay; }
  Void setTemporalIdEnableFlag                     ( Bool	b )                        { m_bTemporalIdEnableFlag = b; }
  Bool getTemporalIdEnableFlag                     ()                                { return m_bTemporalIdEnableFlag; }
  Void setBbvBufferSize                            ( UInt	ui )                       { m_uiBbvBufferSize = ui; }
  UInt getBbvBufferSize                            ()                                { return m_uiBbvBufferSize; }
  Void setLCUSize                                  ( UInt	ui )                       { m_uiLCUSize = ui; }
  UInt getLCUSize                                  ()                                { return m_uiLCUSize; }
  Void setWeightQuantEnableFlag                    ( Bool	b )                        { m_bWeightQuantEnableFlag = b; }
  Bool getWeightQuantEnableFlag                    ()                                { return m_bWeightQuantEnableFlag; }
  Void setbLoadSeqWeightQuantDataFlag              ( Bool	b )                        { m_bLoadSeqWeightQuantDataFlag = b; }
  Bool getbLoadSeqWeightQuantDataFlag              ()                                { return m_bLoadSeqWeightQuantDataFlag; }
  Void setWeightQuantCoeff4x4                      ( Int Idx1, Int Idx2, UInt	ui )   { m_uiWeightQuantCoeff4x4[Idx1][Idx2] = ui; }
  UInt getWeightQuantCoeff4x4                      ( Int Idx1, Int Idx2)             { return m_uiWeightQuantCoeff4x4[Idx1][Idx2]; }
  Void setWeightQuantCoeff8x8                      ( Int Idx1, Int Idx2, UInt	ui )   { m_uiWeightQuantCoeff8x8[Idx1][Idx2] = ui; }
  UInt getWeightQuantCoeff8x8                      ( Int Idx1, Int Idx2)             { return m_uiWeightQuantCoeff8x8[Idx1][Idx2]; }
  Void setScenePictureDisableFlag                  ( Bool	b )                        { m_bScenePictureDisableFlag = b; }
  Bool getScenePictureDisableFlag                  ()                                { return m_bScenePictureDisableFlag; }
  Void setMultiHypothesisSkipEnableFlag            ( Bool	b )                        { m_bMultiHypothesisSkipEnableFlag = b; }
  Bool getMultiHypothesisSkipEnableFlag            ()                                { return m_bMultiHypothesisSkipEnableFlag; }
  Void setDualHypothesisPredictionEnableFlag       ( Bool	b )                        { m_bDualHypothesisPredictionEnableFlag = b; }
  Bool getDualHypothesisPredictionEnableFlag       ()                                { return m_bDualHypothesisPredictionEnableFlag; }
  Void setWeightedSkipEnableFlag                   ( Bool	b )                        { m_bWeightedSkipEnableFlag = b; }
  Bool getWeightedSkipEnableFlag                   ()                                { return m_bWeightedSkipEnableFlag; }
  Void setAsymmetricMotionPartitionsEnableFlag     ( Bool	b )                        { m_bAsymmetricMotionPartitionsEnableFlag = b; }
  Bool getAsymmetricMotionPartitionsEnableFlag     ()                                { return m_bAsymmetricMotionPartitionsEnableFlag; }
  Void setNonsquareQuadtreeTansformEnableFlag      ( Bool	b )                        { m_bNonsquareQuadtreeTansformEnableFlag = b; }
  Bool getNonsquareQuadtreeTansformEnableFlag      ()                                { return m_bNonsquareQuadtreeTansformEnableFlag; }
  Void setNonsquareIntraPredictionEnableFlag       ( Bool	b )                        { m_bNonsquareIntraPredictionEnableFlag = b; }
  Bool getNonsquareIntraPredictionEnableFlag       ()                                { return m_bNonsquareIntraPredictionEnableFlag; }
  Void setSecondaryTransformEnableFlag             ( Bool	b )                        { m_bSecondaryTransformEnableFlag = b; }
  Bool getSecondaryTransformEnableFlag             ()                                { return m_bSecondaryTransformEnableFlag; }
  Void setSampleAdaptiveOffsetEnableFlag           ( Bool	b )                        { m_bSampleAdaptiveOffsetEnableFlag = b; }
  Bool getSampleAdaptiveOffsetEnableFlag           ()                                { return m_bSampleAdaptiveOffsetEnableFlag; }
  Void setAdaptiveLoopFilterEnableFlag             ( Bool	b )                        { m_bAdaptiveLoopFilterEnableFlag = b; }
  Bool getAdaptiveLoopFilterEnableFlag             ()                                { return m_bAdaptiveLoopFilterEnableFlag; }
  Void setPmvrEnableFlag                           ( Bool	b )                        { m_bPmvrEnableFlag = b; }
  Bool getPmvrEnableFlag                           ()                                { return m_bPmvrEnableFlag; }
  Void setNumOfRcs                                 ( UInt ui )                       { m_uiNumOfRcs = ui; }
  UInt getNumOfRcs                                 ()                                { return m_uiNumOfRcs; }
  Void setReferedByOtherFlag                       ( Int Idx, Bool	b )              { m_bReferedByOtherFlag[Idx] = b; }
  Bool getReferedByOtherFlag                       ( Int Idx )                       { return m_bReferedByOtherFlag[Idx]; }
  Void setNumOfReferencePicture                    ( Int Idx, UInt	ui )             { m_uiNumOfReferencePicture[Idx] = ui; }
  UInt getNumOfReferencePicture                    ( Int Idx )                       { return m_uiNumOfReferencePicture[Idx]; }
  Void setDeltaDoiOfReferencePicture               ( Int Idx1, Int Idx2, UInt	ui )   { m_uiDeltaDoiOfReferencePicture[Idx1][Idx2] = ui; }
  UInt getDeltaDoiOfReferencePicture               ( Int Idx1, Int Idx2 )            { return m_uiDeltaDoiOfReferencePicture[Idx1][Idx2]; }
  Void setNumOfRemovedPicture                      ( Int Idx, UInt	ui )             { m_uiNumOfRemovedPicture[Idx] = ui; }
  UInt getNumOfRemovedPicture                      ( Int Idx )                       { return m_uiNumOfRemovedPicture[Idx]; }
  Void setDeltaDoiOfRemovedPicture                 ( Int Idx1, Int Idx2, UInt	ui )   { m_uiDeltaDoiOfRemovedPicture[Idx1][Idx2] = ui; }
  UInt getDeltaDoiOfRemovedPicture                 ( Int Idx1, Int Idx2 )            { return m_uiDeltaDoiOfRemovedPicture[Idx1][Idx2]; }
  Void setOutputReorderDelay                       ( UInt	ui )                       { m_uiOutputReorderDelay = ui; }
  UInt getOutputReorderDelay                       ()                                { return m_uiOutputReorderDelay; }
  Void setCrossSliceLoopfilterEnableFlag           ( Bool	b )                        { m_bCrossSliceLoopfilterEnableFlag = b; }
  Bool getCrossSliceLoopfilterEnableFlag           ()                                { return m_bCrossSliceLoopfilterEnableFlag; }
#if rd_sym
  Void setBackgroundEnableFlag(Bool b)                          { m_bBackgroundEnable = b; }
  Bool getBackgroundEnableFlag()                                { return m_bBackgroundEnable; }
#endif
#endif
#if AVS3_EXTENSION_LWJ
  Void					setUserData		 (TComUserData* pcUserData)				{ m_pcUserData = pcUserData; }
  TComUserData*			getUserData		 ()										{ return m_pcUserData; }
  Void					setExtSeqDisplay (TComExtSeqDisplay* pcExtSeqDisplay)	{ m_pcExtSeqDisplay = pcExtSeqDisplay; }
  TComExtSeqDisplay*	getExtSeqDisplay ()										{ return m_pcExtSeqDisplay; }
  Void					setExtScalability(TComExtScalability* pcExtScalability)	{ m_pcExtScalability = pcExtScalability; }
  TComExtScalability*	getExtScalability()										{ return m_pcExtScalability; }
#endif
#if AVS3_EXTENSION_CYH
  Void					setExtCopyright(TComExtCopyright* pcExtCopyright)		{ m_pcExtCopyright = pcExtCopyright; }
  TComExtCopyright*		getExtCopyright()										{ return m_pcExtCopyright; }
  Void					setExtCameraPra(TComExtCameraPra* pcExtCameraPra)		{ m_pcExtCameraPra = pcExtCameraPra; }
  TComExtCameraPra*		getExtCameraPra()										{ return m_pcExtCameraPra; }
#endif
#if AVS3_EXTENSION_LYT
  TComExtMetadata*		getExtMetadata()						{ return m_pcExtMetadata; }
  Void					setExtMetadata(TComExtMetadata* u)		{ m_pcExtMetadata = u; }
#endif
  // structure
  Void setWidth        ( UInt u ) { m_uiWidth = u;           }
  UInt getWidth        ()         { return  m_uiWidth;       }
  Void setHeight       ( UInt u ) { m_uiHeight = u;          }
  UInt getHeight       ()         { return  m_uiHeight;      }
  Void setLog2MaxCUSize( UInt u ) { m_uiLog2MaxCUSize = u;      }
  UInt getLog2MaxCUSize()         { return  m_uiLog2MaxCUSize;  }
  Void setMaxCUDepth   ( UInt u ) { m_uiMaxCUDepth = u;      }
  UInt getMaxCUDepth   ()         { return  m_uiMaxCUDepth;  }
  
  Void setLog2MaxPhTrSize ( UInt u ) { m_uiLog2MaxPhTrSize = u;     }
  UInt getLog2MaxPhTrSize ()         { return m_uiLog2MaxPhTrSize;  }

  Void setPad         (Int iPad[2]) { m_aiPad[0] = iPad[0]; m_aiPad[1] = iPad[1]; }
  Void setPadX        ( Int  u ) { m_aiPad[0] = u; }
  Void setPadY        ( Int  u ) { m_aiPad[1] = u; }
  Int  getPad         ( Int  u ) { assert(u < 2); return m_aiPad[u];}
  Int* getPad         ( )        { return m_aiPad; }
#if RPS
  Void                   createRPSList(Int numRPS);
  const TComRPSList*     getRPSList() const     { return &m_RPSList; }
  TComRPSList*           getRPSList()           { return &m_RPSList; }
#if RPS_BUG
  Void                   setGOPSize(UInt u)     { m_uiGOPSize = u;    }
  UInt                   getGOPSize()           { return m_uiGOPSize; }
  Void setIntraPeriod(Int u)    { m_iIntraPeriod = u; }
  Int  getIntraPeriod()         { return m_iIntraPeriod; }
#endif
#endif

#if LAMBDA_BUG
  Void setPictureCodingOrder(Int i){ m_iPictureCodingOrder = i; }
  Int  getPictureCodingOrder() { return  m_iPictureCodingOrder; }
#endif


#if INTER614
  Void setNumberReferenceFrames(Int u)    { m_iNumberReferenceFrames = u; }
  Int  getNumberReferenceFrames()         { return m_iNumberReferenceFrames; }
#endif
  // Tool list
  Bool getUseDQP      ()         { return m_bUseDQP;        }
  Bool getUseLDC      ()         { return m_bUseLDC;        }
  Bool getUsePAD      ()         { return m_bUsePAD;        }
  
  Void setUseDQP      ( Bool b ) { m_bUseDQP   = b;         }  
  Void setUseLDC      ( Bool b ) { m_bUseLDC   = b;         }
  Void setUsePAD      ( Bool b ) { m_bUsePAD   = b;         }
  
  // Bit-depth
  UInt      getBitDepth     ()         { return m_uiBitDepth;     }
  Void      setBitDepth     ( UInt u ) { m_uiBitDepth = u;        }
};

#if !AVS3_DELETE_PPS_SYC
/// PPS class
class TComPPS
{
private:
  
public:
  TComPPS();
  virtual ~TComPPS();
  
};
#endif
#if AVS3_PIC_HEADER_ZL
/// AVS PicHeader class
class TComPicHeader
{
private:

  TComSPS*    m_pcSeqHeader;
  //extention
#if AVS3_EXTENSION_LWJ
  TComUserData*				m_pcUserData;
#endif
#if AVS3_EXTENSION_CYH
  TComExtCopyright*			m_pcExtCopyright;
  TComExtCameraPra*			m_pcExtCameraPra;
#endif
#if AVS3_EXTENSION_LYT
  TComExtPicDisplay*	m_pcExtPicDisplay;
  TComExtRoiPra*		m_pcExtRoiPra;
#endif
//#if INTERTEST
//  Bool        MVtestEnable;
//#endif
  UInt        m_uiIntraPicStartCode;
  UInt        m_uiInterPicStartCode;

  Int         m_iBbvDelay;
  UInt        m_uiPicCodingType;

  Bool        m_bScenePredFlag;
  Bool        m_bSceneRefEnableFlag;

  Bool        m_bTimeCodeFlag;
  UInt        m_uiTimeCode;

  Bool        m_bScenePicFlag;
  Bool        m_bScenePicOutputFlag;

  UInt        m_uiDecOrderIndex;
  UInt        m_uiTemporalId;
  UInt        m_uiPicOutputDelay;

  Bool        m_bUseRcsFlag;
  UInt        m_uiRcsIndex;
  //2017-3-25 ZP_rps 
#if RPS
  PictureType   m_ePictureType;
  Int m_iPOC;
  Int m_iGOPSize;
  Int         m_iIntraPeriod;
  const TComReferencePictureSet* m_pRPS;   // pointer to RPS
  TComReferencePictureSet m_LocalRPS;
#if B_RPS_BUG_821
  TComReferencePictureSet m_AdjustRPS;
#endif
  Int         m_iNumRefIdx[NUM_REF_PIC_LIST_01];
  TComPic*                   m_apcRefPicList[NUM_REF_PIC_LIST_01][MAX_NUM_REF_PICS + 1];
  Int                        m_aiRefPOCList[NUM_REF_PIC_LIST_01][MAX_NUM_REF_PICS + 1];
  Int   m_iRPSIdx;
#endif

#if BSKIP
  Int         m_iTemporalReferenceNextP;
  Int         m_iPictureDistance;
  Int         m_iTemporalReferenceForwardDistance;
  Int         m_iTemporalReference;
#endif

#if inter_direct_skip_bug1
  UChar m_ucInterSkipmodeNumber;
#endif


  UInt        m_uiBbvCheckTimes;

  Bool        m_bProgressiveFrame;
  PictureStructure  m_ePicStructure;

  Bool        m_bTopFieldFirst;
  Bool        m_bRepeatFirstField;
  Bool        m_bTopFieldPicFlag;

  Bool        m_bFixedPicQP;
  UInt        m_uiPicQP;

  Bool        m_bRADecodableFlag;

  Bool        m_bLoopFilterDisableFlag;
  LFCUSyntax  m_stLoopFilterSyntax;

  Bool        m_bChromaQuantParamDisableFlag;
  UInt        m_uiChromaQuantParamDeltaCb;
  UInt        m_uiChromaQuantParamDeltaCr;

  Bool        m_bPicWeightQuantEnableFlag;
  WeightQuantSyntax  m_stWeightQuantSyntax;

  Bool        m_bPicAlfEnableFlag[3];
  AlfFilterSyntax  m_stAlfFilterSyntax;

public:
  TComPicHeader();
  virtual ~TComPicHeader();

  Void      initPicHeader();
  Void      setSeqHeader                       ( TComSPS* pcSeqHeader ) { m_pcSeqHeader = pcSeqHeader; }
  TComSPS*  getSeqHeader                       ()                 { return m_pcSeqHeader;  }
#if AVS3_EXTENSION_LWJ
  Void					setUserData(TComUserData* pcUserData)				{ m_pcUserData = pcUserData; }
  TComUserData*			getUserData()										{ return m_pcUserData; }
#endif
#if AVS3_EXTENSION_CYH
  Void					setExtCopyright(TComExtCopyright* pcExtCopyright)		{ m_pcExtCopyright = pcExtCopyright; }
  TComExtCopyright*		getExtCopyright()										{ return m_pcExtCopyright; }
  Void					setExtCameraPra(TComExtCameraPra* pcExtCameraPra)		{ m_pcExtCameraPra = pcExtCameraPra; }
  TComExtCameraPra*		getExtCameraPra()										{ return m_pcExtCameraPra; }
#endif
#if AVS3_EXTENSION_LYT
  TComExtPicDisplay*	getExtPicDisplay()						{ return m_pcExtPicDisplay; }
  Void					setExtPicDisplay(TComExtPicDisplay* u)	{ m_pcExtPicDisplay = u; }
  TComExtRoiPra*		getExtRoiPra()							{ return m_pcExtRoiPra; }
  Void					setExtRoiPra(TComExtRoiPra* u)			{ m_pcExtRoiPra = u; }
#endif
  Void setIntraPicStartCode              ( UInt uiIdx ) { m_uiIntraPicStartCode = uiIdx;  }
  UInt getIntraPicStartCode              ()             { return  m_uiIntraPicStartCode;  }

  Void setInterPicStartCode              ( UInt uiIdx ) { m_uiInterPicStartCode = uiIdx;  }
  UInt getInterPicStartCode              ()             { return  m_uiInterPicStartCode;  }
//#if INTERTEST
//  Void setMVtestEnable(Bool b)     { MVtestEnable = b; }
//  Bool getMVtestEnable()             { return  MVtestEnable; }
//#endif
  Void setBbvDelay                       ( Int i )      { m_iBbvDelay = i;     }
  Int  getBbvDelay                        ()             { return  m_iBbvDelay; }

#if inter_direct_skip_bug1
  Void   setInterSkipmodeNumber(UChar i)      { m_ucInterSkipmodeNumber = i; }
  UChar  getInterSkipmodeNumber()             { return  m_ucInterSkipmodeNumber; }
#endif

  Void setPicCodingType                  ( UInt uiIdx ) { m_uiPicCodingType = uiIdx;      }
  UInt getPicCodingType                  ()             { return  m_uiPicCodingType;      }

  Void setScenePredFlag                  ( Bool b )     { m_bScenePredFlag = b;           }
  Bool getScenePredFlag                  ()             { return  m_bScenePredFlag;       }

  Void setSceneRefEnableFlag             ( Bool b )     { m_bSceneRefEnableFlag = b;       }
  Bool getSceneRefEnableFlag             ()             { return  m_bSceneRefEnableFlag;   }

  Void setTimeCodeFlag                   ( Bool b )     { m_bTimeCodeFlag = b;         }
  Bool getTimeCodeFlag                   ()             { return  m_bTimeCodeFlag;     }

  Void setTimeCode                       ( UInt uiIdx ) { m_uiTimeCode = uiIdx;     }
  UInt getTimeCode                       ()             { return  m_uiTimeCode;     }

  Void setScenePicFlag                   ( Bool b )     { m_bScenePicFlag = b;      }
  Bool getScenePicFlag                   ()             { return  m_bScenePicFlag;  }

  Void setScenePicOutputFlag             ( Bool b )     { m_bScenePicOutputFlag = b;         }
  Bool getScenePicOutputFlag             ()             { return  m_bScenePicOutputFlag;     }

  Void setDecOrderIndex                  ( UInt uiIdx ) { m_uiDecOrderIndex = uiIdx;     }
  UInt getDecOrderIndex                  ()             { return  m_uiDecOrderIndex;     }

  Void setTemporalId                     ( UInt uiIdx ) { m_uiTemporalId = uiIdx;     }
  UInt getTemporalId                     ()             { return  m_uiTemporalId;     }

  Void setPicOutputDelay                 ( UInt uiIdx ) { m_uiPicOutputDelay = uiIdx;     }
  UInt getPicOutputDelay                 ()             { return  m_uiPicOutputDelay;     }

  Void setUseRcsFlag                     ( Bool b )     { m_bUseRcsFlag = b;         }
  Bool getUseRcsFlag                     ()             { return  m_bUseRcsFlag;     }

  Void setRcsIndex                       ( UInt uiIdx ) { m_uiRcsIndex = uiIdx;     }
  UInt getRcsIndex                       ()             { return  m_uiRcsIndex;     }

  Void setBbvCheckTimes                  ( UInt uiIdx ) { m_uiBbvCheckTimes = uiIdx;     }
  UInt getBbvCheckTimes                  ()             { return  m_uiBbvCheckTimes;     }

  Void setProgressiveFrame               ( Bool b )     { m_bProgressiveFrame = b;         }
  Bool getProgressiveFrame               ()             { return  m_bProgressiveFrame;     }
#if RPS
  const TComReferencePictureSet* getRPS()                   { return m_pRPS; }
  Void setIntraPeriod(Int iIdx){ m_iIntraPeriod = iIdx; }
  Int  getIntraPeriod()           { return m_iIntraPeriod; }
  Int       getPOC()                          { return  m_iPOC; }
  Void  setPOC(Int i){ m_iPOC = i; }
  Int getGOPSize()    { return m_iGOPSize; }
  Void  setGOPSize(Int i)  { m_iGOPSize = i; }
  Void      setRPS(const TComReferencePictureSet *pcRPS)         { m_pRPS = pcRPS; }
  TComReferencePictureSet *getLocalRPS(){ return &m_LocalRPS; } //initRPS
#if B_RPS_BUG_821
  TComReferencePictureSet *getAdjustRPS(){ return &m_AdjustRPS; } //initRPS
#endif
  Void                        setRPSidx(Int rpsIdx)            { m_iRPSIdx = rpsIdx; }
  Int                         getRPSidx() const                  { return m_iRPSIdx; }
  PictureType getPictureType()    { return m_ePictureType; }
  Void  setPictureType(PictureType e)  { m_ePictureType = e; }
  Int       getNumRefIdx(RefPic e)const                      { return   m_iNumRefIdx[e]; }
  Void      setNumRefIdx(RefPic e, Int i)                    { m_iNumRefIdx[e] = i; }
  TComPic*                    getRefPic(RefPic e, Int iRefIdx)                  { return m_apcRefPicList[e][iRefIdx]; }
  const TComPic*              getRefPic(RefPic e, Int iRefIdx) const            { return m_apcRefPicList[e][iRefIdx]; }
  Int                         getRefPOC(RefPic e, Int iRefIdx) const            { return m_aiRefPOCList[e][iRefIdx]; }
  Void      setRefPic(TComPic* p, RefPic e, Int iRefIdx)        { m_apcRefPicList[e][iRefIdx] = p; }
  Void      setRefPOC(Int i, RefPic e, Int iRefIdx)             { m_aiRefPOCList[e][iRefIdx] = i; }
  Void      setRefPOCList();
  Void      setRefPic(TComList<TComPic*>& rcListPic);
  Void      ApplyReferencePictureSet(TComList<TComPic*>& rcListPic, const TComReferencePictureSet *RPSList);
  Bool      isIntra()                          { return  m_ePictureType == I_PICTURE; }
  Bool      isInterB()                          { return  m_ePictureType == B_PICTURE; }
  Bool      isInterP()                          { return  m_ePictureType == P_PICTURE; }

#if IDR
  Void      ApplyIDR(TComList<TComPic*>& rcListPic, Bool bUseIDR, PictureType e);

  Bool      checkThatAllRefPicsAreAvailable(TComList<TComPic*>& rcListPic, const TComReferencePictureSet *pReferencePictureSet);
  Void      createExplicitReferencePictureSetFromReference(TComList<TComPic*>& rcListPic, const TComReferencePictureSet *pReferencePictureSet);
#endif
#if F_RPS
  Bool      isInterF()                          { return  m_ePictureType == F_PICTURE; }
#endif

#endif
#if BSKIP
  Void setTemporalReferenceNextP(Int i){ m_iTemporalReferenceNextP = i; }
  Int  getTemporalReferenceNextP(){ return m_iTemporalReferenceNextP; }
  Void setPictureDistance(Int i){ m_iPictureDistance = i; }
  Int  getPictureDistance(){ return m_iPictureDistance; }
  Void setTemporalReferenceForwardDistance(Int i){ m_iTemporalReferenceForwardDistance = i; }
  Int  getTemporalReferenceForwardDistance(){ return m_iTemporalReferenceForwardDistance; }
  Void      setRefDistance();
  Void setTemporalReference(Int i){ m_iTemporalReference = i; }
  Int  getTemporalReference(){ return m_iTemporalReference; }

#endif
  Void setPicStructure                   ( PictureStructure picturestruct )     { m_ePicStructure = picturestruct; }
  PictureStructure getPicStructure          ()                                  { return  m_ePicStructure;      }

  Void setTopFieldFirst                  ( Bool b )     { m_bTopFieldFirst = b;         }
  Bool getTopFieldFirst                  ()             { return  m_bTopFieldFirst;     }

  Void setRepeatFirstField               ( Bool b )     { m_bRepeatFirstField = b;         }
  Bool getRepeatFirstField               ()             { return  m_bRepeatFirstField;     }

  Void setTopFieldPicFlag                ( Bool b )     { m_bTopFieldPicFlag = b;         }
  Bool getTopFieldPicFlag                ()             { return  m_bTopFieldPicFlag;     }

  Void setFixedPicQP                     ( Bool b )     { m_bFixedPicQP = b;         }
  Bool getFixedPicQP                     ()             { return  m_bFixedPicQP;     }

  Void setPicQP                          ( UInt uiIdx ) { m_uiPicQP = uiIdx;     }
  UInt getPicQP                          ()             { return  m_uiPicQP;     }

  Void setRADecodableFlag                ( Bool b )     { m_bRADecodableFlag = b;         }
  Bool getRADecodableFlag                ()             { return  m_bRADecodableFlag;     }

  Void setLoopFilterDisableFlag          ( Bool b )     { m_bLoopFilterDisableFlag = b;         }
  Bool getLoopFilterDisableFlag          ()             { return  m_bLoopFilterDisableFlag;     }

  Void setChromaQuantParamDisableFlag    ( Bool b )     { m_bChromaQuantParamDisableFlag = b;         }
  Bool getChromaQuantParamDisableFlag    ()             { return  m_bChromaQuantParamDisableFlag;     }

  Void setChromaQuantParamDeltaCb        ( UInt uiIdx ) { m_uiChromaQuantParamDeltaCb = uiIdx;     }
  UInt getChromaQuantParamDeltaCb        ()             { return  m_uiChromaQuantParamDeltaCb;     }

  Void setChromaQuantParamDeltaCr        ( UInt uiIdx ) { m_uiChromaQuantParamDeltaCr = uiIdx;     }
  UInt getChromaQuantParamDeltaCr        ()             { return  m_uiChromaQuantParamDeltaCr;     }

  Void setPicWeightQuantEnableFlag       ( Bool b )     { m_bPicWeightQuantEnableFlag = b;         }
  Bool getPicWeightQuantEnableFlag       ()             { return  m_bPicWeightQuantEnableFlag;     }

  Void setPicAlfEnableFlag               ( Bool b,UInt uiIdx ) { m_bPicAlfEnableFlag[uiIdx] = b;         }
  Bool getPicAlfEnableFlag               ( UInt uiIdx )        { return  m_bPicAlfEnableFlag[uiIdx];     }

  LFCUSyntax* getLoopFilterSyntax        ()             { return  &m_stLoopFilterSyntax;    }
  WeightQuantSyntax* getWeightQuantSyntax()             { return  &m_stWeightQuantSyntax;   }
  AlfFilterSyntax* getAlfFilterSyntax    ()             { return  &m_stAlfFilterSyntax;     }
#if RPS
  protected:
    TComPic*  xGetRefPicList(TComList<TComPic*>& rcListPic, Int uiPOCCurr);
#endif
};
#endif

/// picture header class
class TComPicture
{
private:
#if AVS3_SLICE_HEADER_SYC
  //  Bitstream writing
  UInt	m_uiSliceStartCode;
  UInt	m_uiSliceVerPosExtension;
  UInt	m_uiSliceHorPos;
  UInt	m_uiSliceHorPosExtension;
  Bool	m_bFixedSliceQP;
  UInt	m_uiSliceQP;
  Bool	m_bSliceSaoEnableFlag[3];
#endif
  //  Bitstream writing
  Int         m_iPOC;
  PictureType   m_ePictureType;
  Int         m_iPictureQp;
  Int         m_iPictureCQp;
  Bool        m_bLoopFilterDisable;
  
  Bool        m_bDRBFlag;             //  flag for future usage as reference buffer
  
  //  Data
  TComPic*    m_apcRefPic[2];
  Int         m_aiRefPOC [2];
  Int         m_iDepth;
  
  // referenced picture?
  Bool        m_bRefenced;
  
  // access channel
  TComSPS*    m_pcSPS;
#if RPS
  TComPicHeader* m_pcPicHeader;
#endif
#if !AVS3_DELETE_PPS_SYC
  TComPPS*    m_pcPPS;
#endif
  TComPic*    m_pcPic;
  
  Double      m_dLambda;

#if RD_MVP
	Bool				m_bFieldSequence = false;
	Int					m_iPicDist = 1;
	Int					m_iFwRefDist = 0;
	Int					m_iPicTrNextP = 0;
#endif
  
public:
  TComPicture();
  virtual ~TComPicture();
  
  Void      initPicture       ();
#if AVS3_SLICE_HEADER_SYC
  //  Bitstream writing
  UInt getSliceStartCode        ()	                {return m_uiSliceStartCode;}
  Void setSliceStartCode        (UInt ui)	          {m_uiSliceStartCode = ui;}
  UInt getSliceVerPosExtension  ()	                {return m_uiSliceVerPosExtension;}
  Void setSliceVerPosExtension  (UInt ui)	          {m_uiSliceVerPosExtension = ui;}
  UInt getSliceHorPos           ()	                {return m_uiSliceHorPos;}
  Void setSliceHorPos           (UInt ui)	          {m_uiSliceHorPos = ui;}
  UInt getSliceHorPosExtension  ()	                {return m_uiSliceHorPosExtension;}
  Void setSliceHorPosExtension  (UInt ui)	          {m_uiSliceHorPosExtension = ui;}
  Bool getFixedSliceQP          ()	                {return m_bFixedSliceQP;}
  Void setFixedSliceQP          (Bool b)	          {m_bFixedSliceQP = b;}
  UInt getSliceQP               ()	                {return m_uiSliceQP;}
  Void setSliceQP               (UInt ui)	          {m_uiSliceQP = ui;}
  Bool getSliceSaoEnableFlag    (UInt ui)      	    {return m_bSliceSaoEnableFlag[ui];}
  Void setSliceSaoEnableFlag    (UInt ui, Bool b)   {m_bSliceSaoEnableFlag[ui] = b;}
#endif
  Void      setSPS          ( TComSPS* pcSPS ) { m_pcSPS = pcSPS; }
  TComSPS*  getSPS          () { return m_pcSPS; }
#if RPS
  Void setPicHeader(TComPicHeader* pcPicHeader){ m_pcPicHeader = pcPicHeader; }
  TComPicHeader *getPicHeader(){ return m_pcPicHeader; }
#endif
#if !AVS3_DELETE_PPS_SYC
  Void      setPPS          ( TComPPS* pcPPS ) { m_pcPPS = pcPPS; }
  TComPPS*  getPPS          () { return m_pcPPS; }
#endif
  PictureType getPictureType    ()                          { return  m_ePictureType;         }
  Int       getPOC          ()                          { return  m_iPOC;           }
  Int       getPictureQp      ()                          { return  m_iPictureQp;           }
  Int       getPictureChromaQp      ()                          { return  m_iPictureCQp;           }
  Bool      getDRBFlag      ()                          { return  m_bDRBFlag;           }
  Bool      getLoopFilterDisable()                      { return  m_bLoopFilterDisable; }
  TComPic*  getPic              ()                              { return  m_pcPic;                      }
#if !RPS
  TComPic*  getRefPic(RefPic e)    { return  m_apcRefPic[e]; }//?
  Int       getRefPOC(RefPic e)    { return  m_aiRefPOC[e]; }//?
#endif
  Int       getDepth            ()                              { return  m_iDepth;                     }

  Void      setReferenced(Bool b)                               { m_bRefenced = b; }
  Bool      isReferenced()                                      { return m_bRefenced; }
  
  Void      setPOC              ( Int i )                       { m_iPOC              = i;      }
  Void      setPictureType        ( PictureType e )                 { m_ePictureType        = e;      }
  Void      setPictureQp          ( Int i, Int iC )                       { m_iPictureQp = i; m_iPictureCQp = iC; }
  Void      setDRBFlag          ( Bool b )                      { m_bDRBFlag = b;               }
  Void      setLoopFilterDisable( Bool b )                      { m_bLoopFilterDisable= b;      }
  
#if !RPS
  Void      setRefPic(TComPic* p, RefPic e)        { m_apcRefPic[e] = p; }
  Void      setRefPOC(Int i, RefPic e)             { m_aiRefPOC[e] = i; }
#endif
  Void      setPic              ( TComPic* p )                  { m_pcPic             = p;      }
  Void      setDepth            ( Int iDepth )                  { m_iDepth            = iDepth; }
  
#if !RPS
  Void      setRefPic(TComList<TComPic*>& rcListPic);
  Void      setRefPOCList();
#endif

  
  Bool      isIntra         ()                          { return  m_ePictureType == I_PICTURE;  }
  Bool      isInterB        ()                          { return  m_ePictureType == B_PICTURE;  }
  Bool      isInterP        ()                          { return  m_ePictureType == P_PICTURE;  }

#if F_RPS
  Bool      isInterF()                          { return  m_ePictureType == F_PICTURE; }
#endif

  Void      setLambda( Double d ) { m_dLambda = d; }
  Double    getLambda() { return m_dLambda;        }
  
  static Void      sortPicList         ( TComList<TComPic*>& rcListPic );
#if RD_MVP
	Void      setFieldSequence(Bool b) { m_bFieldSequence = b; }
	Bool      getFieldSequence() { return m_bFieldSequence; }

	//Void      setPicDistance(Int i) { m_iPicDist = i; }
	Int		    getPicDistance() { return m_iPicDist; }

	//Void      setFwRefDistance(Int i) { m_iFwRefDist = i; }
	Int		    getFwRefDistance() { return m_iFwRefDist; }

	//Void      setPicTrNextP(Int i) { m_iPicTrNextP = i; }
	Int		    getPicTrNextP() { return m_iPicTrNextP; }

#endif
protected:
  TComPic*  xGetRefPic  (TComList<TComPic*>& rcListPic,
                         Bool                bDRBFlag,
                         UInt                uiPOCCurr,
                         RefPic              eRefPic,
                         UInt                uiNthRefPic );
  
};// END CLASS DEFINITION TComPicture


#endif // __TCOMPICTURE__

