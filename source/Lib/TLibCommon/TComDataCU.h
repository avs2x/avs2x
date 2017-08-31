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

/** \file     TComDataCU.h
    \brief    CU data structure (header)
    \todo     not all entities are documented
*/

#ifndef _TCOMDATACU_
#define _TCOMDATACU_

#include <assert.h>

// Include files
#include "CommonDef.h"
#include "TComMotionInfo.h"
#include "TComPicture.h"
#include "TComRdCost.h"
#include "TComPattern.h"

#if ZHANGYI_INTRA_SDIP
#include "TComYuv.h"
#endif

#include <algorithm>
#include <vector>

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// CU data structure class
class TComDataCU
{
private:
  
  // -------------------------------------------------------------------------------------------------------------------
  // class pointers
  // -------------------------------------------------------------------------------------------------------------------
  
  TComPic*      m_pcPic;              ///< picture class pointer
  TComPicture*  m_pcPicture;            ///< picture header pointer
  TComPattern*  m_pcPattern;          ///< neighbour access class pointer

  // -------------------------------------------------------------------------------------------------------------------
  // CU description
  // -------------------------------------------------------------------------------------------------------------------
  
  UInt          m_uiCUAddr;           ///< CU address in a picture
  UInt          m_uiAbsIdxInLCU;      ///< absolute address in a CU. It's Z scan order
  UInt          m_uiCUPelX;           ///< CU position in a pixel (X)
  UInt          m_uiCUPelY;           ///< CU position in a pixel (Y)
  UInt          m_uiNumPartition;     ///< total number of minimum partitions in a CU
  UChar*        m_puhLog2CUSize;      ///< array of CU sizes

  UChar*        m_puhWidth;           ///< array of widths
  UChar*        m_puhHeight;          ///< array of heights

  UChar*        m_puhDepth;           ///< array of depths
  
  // -------------------------------------------------------------------------------------------------------------------
  // CU data
  // -------------------------------------------------------------------------------------------------------------------
  
#if INTER_GROUP
	Int*          m_iDMHMode;
	Int*          m_iPartitionSizeIndex;       ///< cu type index
	Int           m_iFPuTypeIndex[4];
#endif

  PartSize*     m_pePartSize;         ///< array of partition sizes

  UChar*				m_puhTrIdx;						///< array of transform indices

  PredMode*     m_pePredMode;         ///< array of prediction modes
  UChar*        m_puhCbf[3];          ///< array of coded block flags (CBF)
#if RPS
  TComCUMvField m_acCUMvField[NUM_REF_PIC_LIST_01];     ///< array of motion vectors  //enum已经替换，不单独宏注释
#else
  TComCUMvField m_acCUMvField[2];     ///< array of motion vectors
#endif
  TCoeff*       m_pcTrCoeffY;         ///< transformed coefficient buffer (Y)
  TCoeff*       m_pcTrCoeffCb;        ///< transformed coefficient buffer (Cb)
  TCoeff*       m_pcTrCoeffCr;        ///< transformed coefficient buffer (Cr)
  
  // -------------------------------------------------------------------------------------------------------------------
  // neighbour access variables
  // -------------------------------------------------------------------------------------------------------------------
  
  TComDataCU*   m_pcCUAboveLeft;      ///< pointer of above-left CU
  TComDataCU*   m_pcCUAboveRight;     ///< pointer of above-right CU
  TComDataCU*   m_pcCUAbove;          ///< pointer of above CU
  TComDataCU*   m_pcCULeft;           ///< pointer of left CU
#if  CODEFFCODER_LDW_WLQ_YQH
   CoeffCodingParam  m_CoeffCodingParam; //yuquanhe@hisilicon.com
#endif
  // -------------------------------------------------------------------------------------------------------------------
  // coding tool information
  // -------------------------------------------------------------------------------------------------------------------
#if inter_direct_skip_bug1
   UChar*       m_pcInterSkipmode;
#endif

  UChar*        m_puhInterDir;        ///< array of inter directions
  UChar*        m_puhIntraDir;        ///< array of intra directions
#if ZHANGYI_INTRA
  UChar*        m_puhIntraDirCb;
  UChar*        m_puhIntraDirCr;
  Char*        m_puhIntraDirValue; //the true value of intra pred mode that is written into the bitstream
  Char*        m_puhIntraDirCbValue;
  Char*        m_puhIntraDirCrValue;
#endif

#if ZHANGYI_INTRA_SDIP
  UChar*        m_pSDIPFlag;           ///< array of SDIP flags
  UChar*        m_pSDIPDirection;      ///< array of SDIP directions 
#endif
  // -------------------------------------------------------------------------------------------------------------------
  // motion vector predictor information
  // -------------------------------------------------------------------------------------------------------------------

#ifdef RPS
  TComMvField        m_cMvPred[NUM_REF_PIC_LIST_01];
#else
  TComMv        m_cMvPred[2];
#endif

#if PSKIP
  TComMvField m_cTmpFirstMvPred[MH_PSKIP_NUM + NUM_OFFSET + 1]; //tmp_pref_fst[MH_PSKIP_NUM + NUM_OFFSET + 1];  tmp_fstPSkipMv[MH_PSKIP_NUM + NUM_OFFSET + 1][3];
  TComMvField m_cTmpSecondMvPred[MH_PSKIP_NUM + NUM_OFFSET + 1];//tmp_pref_snd[MH_PSKIP_NUM + NUM_OFFSET + 1];  tmp_sndPSkipMv[MH_PSKIP_NUM + NUM_OFFSET + 1][3];
#endif                                                          
#if BSKIP                                                       
  TComMv m_cTempForwardBSkipMv[DIRECTION + 1];  //img->tmp_fwBSkipMv
  TComMv m_cTempBackwardBSkipMv[DIRECTION + 1]; //img->tmp_bwBSkipMv
#endif

  // -------------------------------------------------------------------------------------------------------------------
  // misc. variables
  // -------------------------------------------------------------------------------------------------------------------
  
  Bool          m_bDecSubCu;          ///< indicates decoder-mode
  Double        m_dTotalCost;         ///< sum of partition RD costs
  UInt          m_uiTotalDistortion;  ///< sum of partition distortion
  UInt          m_uiTotalBits;        ///< sum of partition bits

  UChar*        m_pauhDBKIdx[2];      //Ver, Hor
protected:
  
  /// compute required bits to encode MVD
  UInt          xGetMvdBits           ( TComMv cMvd );
  UInt          xGetComponentBits     ( Int iVal );
  
  /// compute scaling factor from POC difference
  Int           xGetDistScaleFactor   ( Int iCurrPOC, Int iCurrRefPOC, Int iColPOC, Int iColRefPOC );
  
  /// calculate all CBF's from coefficients
  Void          xCalcCuCbf            ( UChar* puhCbf, UInt uiTrDepth, UInt uiCbfDepth, UInt uiCuDepth );
  
public:
  TComDataCU();
  virtual ~TComDataCU();
  
  Void   setDBKIdx  (UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth, UChar uiIdx);
  UChar  getDBKIdx  (UInt uiDir, UInt uiAbsPartIdx)              { return m_pauhDBKIdx[uiDir][uiAbsPartIdx];  }
  UChar* getDBKIdx  (UInt uiDir)                                 { return m_pauhDBKIdx[uiDir];  }
  Void   resetDBKIdx(UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth);

  // -------------------------------------------------------------------------------------------------------------------
  // create / destroy / initialize / copy
  // -------------------------------------------------------------------------------------------------------------------
  
  Void          create                ( UInt uiNumPartition, UInt uiLog2CUSize, Bool bDecSubCu );
  Void          destroy               ();
  
  Void          initCU                ( TComPic* pcPic, UInt uiCUAddr );
  Void          initEstData           ( UInt uiDepth );
  Void          initSubCU             ( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth );
  
  Void          copySubCU             ( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth );
  Void          copyInterPredInfoFrom ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPic eRefPic );
  Void          copyPartFrom          ( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth );
  
  Void          copyToPic             ( UChar uiDepth );
  Void          copyToPic             ( UChar uiDepth, UInt uiPartIdx, UInt uiPartDepth );
  
#if CODEFFCODER_LDW_WLQ_YQH
  CoeffCodingParam&  getCoeffCodingParam            ()           { return m_CoeffCodingParam;       }
#endif
  // -------------------------------------------------------------------------------------------------------------------
  // member functions for CU description
  // -------------------------------------------------------------------------------------------------------------------
#if inter_direct_skip_bug1
  Void          setInterSkipmode(UInt uiIdx, UChar  uh) { m_pcInterSkipmode[uiIdx] = uh; }
  Void          setInterSkipmodeSubParts(UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth);

  UChar*        getInterSkipmode()                                 { return m_pcInterSkipmode; }
  UChar         getInterSkipmode(UInt uiIdx)                        { return m_pcInterSkipmode[uiIdx]; }

#endif


#if INTER_GROUP
	Bool          isDirect(UInt uiPartIdx)  { return m_pePredMode[uiPartIdx] == MODE_DIRECT; }
	Int           getWeightedSkipMode(UInt uiPartIdx = 0){ return 0; }// 还没有这个模式
	Void          setPartitionSizeIndex(UInt uiIdx, Int uh){ m_iPartitionSizeIndex[uiIdx] = uh; }
	Int           getPartitionSizeIndex(UInt uiIdx){ return m_iPartitionSizeIndex[uiIdx]; }
	Int*          getPartitionSizeIndex(){ return m_iPartitionSizeIndex; }
	//pcCU->getFPuTypeIndex() == 0 || (pcCU->getPartitionSizeIndex() == 5 && pcCU->getFFourPuTypeIndex() == 0)
	Int*          getDMHMode(){ return m_iDMHMode; }
	Int           getDMHMode(Int idx){ return m_iDMHMode[idx]; }
	Void          setDMHMode(Int idx, Int uh){ m_iDMHMode[idx] = uh; }
	Int           getFPuTypeIndex(Int idx){ return m_iFPuTypeIndex[idx]; }
	Void          setFPuTypeIndex(Int idx, Int TypeIndex){ m_iFPuTypeIndex[idx] = TypeIndex; }
	Int           getFFourTypeIndex();
#endif
#if DMH
	Void          setDMHModeSubPart(Int dmh_mode, UInt uiAbsPartIdx, UInt uiDepth);
#endif

  TComPic*      getPic                ()                        { return m_pcPic;           }
  TComPicture*  getPicture              ()                        { return m_pcPicture;         }
  UInt&         getAddr               ()                        { return m_uiCUAddr;        }
  UInt&         getZorderIdxInCU      ()                        { return m_uiAbsIdxInLCU; }
  UInt          getCUPelX             ()                        { return m_uiCUPelX;        }
  UInt          getCUPelY             ()                        { return m_uiCUPelY;        }
  TComPattern*  getPattern            ()                        { return m_pcPattern;       }
  
  UChar*        getDepth              ()                        { return m_puhDepth;        }
  UChar         getDepth              ( UInt uiIdx )            { return m_puhDepth[uiIdx]; }
  Void          setDepth              ( UInt uiIdx, UChar  uh ) { m_puhDepth[uiIdx] = uh;   }
  
  Void          setDepthSubParts      ( UInt uiDepth, UInt uiAbsPartIdx );
  
  // -------------------------------------------------------------------------------------------------------------------
  // member functions for CU data
  // -------------------------------------------------------------------------------------------------------------------
  
  PartSize*     getPartitionSize      ()                        { return m_pePartSize;        }
  PartSize      getPartitionSize      ( UInt uiIdx )            { return m_pePartSize[uiIdx]; }
  Void          setPartitionSize      ( UInt uiIdx, PartSize uh){ m_pePartSize[uiIdx] = uh;   }
  Void          setPartSizeSubParts   ( PartSize eMode, UInt uiAbsPartIdx, UInt uiDepth );

  UChar*				getTransformIdx				()												{ return m_puhTrIdx;          }
	UChar					getTransformIdx				( UInt uiIdx )            { return m_puhTrIdx[uiIdx];		}
  Void					setTransformIdx				( UInt uiIdx, UChar  uh ) { m_puhTrIdx[uiIdx] = uh;			}
  Void					setTrIdxSubParts      ( UInt uiTrIdx, UInt uiAbsPartIdx, UInt uiDepth );

  PredMode*     getPredictionMode     ()                        { return m_pePredMode;        }
  PredMode      getPredictionMode     ( UInt uiIdx )            { return m_pePredMode[uiIdx]; }
  Void          setPredictionMode     ( UInt uiIdx, PredMode uh){ m_pePredMode[uiIdx] = uh;   }
  Void          setPredModeSubParts   ( PredMode eMode, UInt uiAbsPartIdx, UInt uiDepth );
  
  UChar*        getWidth              ()                        { return m_puhWidth;          }
  UChar         getWidth              ( UInt uiIdx )            { return m_puhWidth[uiIdx];   }
  Void          setWidth              ( UInt uiIdx, UChar  uh ) { m_puhWidth[uiIdx] = uh;     }
  
  UChar*        getHeight             ()                        { return m_puhHeight;         }
  UChar         getHeight             ( UInt uiIdx )            { return m_puhHeight[uiIdx];  }
  Void          setHeight             ( UInt uiIdx, UChar  uh ) { m_puhHeight[uiIdx] = uh;    }

  UChar*        getLog2CUSize         ()                        { return m_puhLog2CUSize;          }
  UChar         getLog2CUSize         ( UInt uiIdx )            { return m_puhLog2CUSize[uiIdx];   }
  Void          setLog2CUSize         ( UInt uiIdx, UChar  uh ) { m_puhLog2CUSize[uiIdx] = uh;     }
  
  Void          setSizeSubParts       ( UInt uiWidth, UInt uiHeight, UInt uiAbsPartIdx, UInt uiDepth );
  
  TComCUMvField* getCUMvField         ( RefPic e )          { return  &m_acCUMvField[e];  }
  
  TCoeff*&      getCoeffY             ()                        { return m_pcTrCoeffY;        }
  TCoeff*&      getCoeffCb            ()                        { return m_pcTrCoeffCb;       }
  TCoeff*&      getCoeffCr            ()                        { return m_pcTrCoeffCr;       }
  
  UChar         getCbf    ( UInt uiIdx, TextType eType )                  { return m_puhCbf[g_aucConvertTxtTypeToIdx[eType]][uiIdx];  }
  UChar*        getCbf    ( TextType eType )                              { return m_puhCbf[g_aucConvertTxtTypeToIdx[eType]];         }
  UChar         getCbf    ( UInt uiIdx, TextType eType, UInt uiTrDepth )  { return ( ( getCbf( uiIdx, eType ) >> uiTrDepth ) & 0x1 ); }
  Void          setCbf    ( UInt uiIdx, TextType eType, UChar uh )        { m_puhCbf[g_aucConvertTxtTypeToIdx[eType]][uiIdx] = uh;    }
  Void          clearCbf  ( UInt uiIdx, TextType eType, UInt uiNumParts );
  UChar         getQtRootCbf          ( UInt uiIdx )                      { return getCbf( uiIdx, TEXT_LUMA, 0 ) || getCbf( uiIdx, TEXT_CHROMA_U, 0 ) || getCbf( uiIdx, TEXT_CHROMA_V, 0 ); }

  Void					setCuCbfLuma          ( UInt uiAbsPartIdx, UInt uiLumaTrMode, UInt uiPartDepth = 0											);
  Void					setCuCbfChroma        ( UInt uiAbsPartIdx, UInt uiChromaTrMode, UInt uiPartDepth = 0										);
  Void					setCuCbfChromaUV      ( UInt uiAbsPartIdx, UInt uiChromaTrMode, TextType eTxt,  UInt uiPartDepth = 0		);

  Void          setCbfSubParts        ( UInt uiCbfY, UInt uiCbfU, UInt uiCbfV, UInt uiAbsPartIdx, UInt uiDepth          );
  Void          setCbfSubParts        ( UInt uiCbf, TextType eTType, UInt uiAbsPartIdx, UInt uiDepth                    );
  
  // -------------------------------------------------------------------------------------------------------------------
  // member functions for coding tool information
  // -------------------------------------------------------------------------------------------------------------------
  
  UChar*        getInterDir           ()                        { return m_puhInterDir;               }
  UChar         getInterDir           ( UInt uiIdx )            { return m_puhInterDir[uiIdx];        }
  Void          setInterDir           ( UInt uiIdx, UChar  uh ) { m_puhInterDir[uiIdx] = uh;          }
  Void          setInterDirSubParts   ( UInt uiDir,  UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx );
  UChar*        getIntraDir           ()                        { return m_puhIntraDir;               }
  UChar         getIntraDir           ( UInt uiIdx )            { return m_puhIntraDir[uiIdx];        }
  Void          setIntraDir           ( UInt uiIdx, UChar  uh ) { m_puhIntraDir[uiIdx] = uh;          }
  Void          setIntraDirSubParts   ( UInt uiDir,  UInt uiAbsPartIdx, UInt uiDepth );
#if niu_NSQT_dec   // from TEncSbac
  Void getTUSize(UInt uiPartIdx, Int& riWidth, Int& riHeight, Int uiLog2Size, PartSize m_pePartSize, Bool bNSQT);
#endif
#if wlq_FME
  Void          getInterPUxy(Int iPartIdx, Int& x, Int& y);
#endif
#if ZHANGYI_INTRA
  Char*        getIntraDirValue           ()                        { return m_puhIntraDirValue ;               }
  Char*        getIntraDirCbValue            ()                        { return m_puhIntraDirCbValue ;               }
  Char*        getIntraDirCrValue            ()                        { return m_puhIntraDirCrValue ;               }
  Char         getIntraDirValue           ( UInt uiIdx )            { return m_puhIntraDirValue[uiIdx];        }
  Char        getIntraDirCbValue           ( UInt uiIdx )                        { return m_puhIntraDirCbValue[uiIdx];               }
  Char        getIntraDirCrValue           ( UInt uiIdx )                        { return m_puhIntraDirCrValue[uiIdx];               }
  Void          setIntraDirSubPartsValue   ( Int uiDir,  UInt uiAbsPartIdx, UInt uiDepth );
  Void          setIntraDirSubPartsCbValue( Int uiDir, UInt uiAbsPartIdx, UInt uiDepth);
  Void          setIntraDirSubPartsCrValue( Int uiDir, UInt uiAbsPartIdx, UInt uiDepth);
  UChar*        getIntraDirCb           ()                        { return m_puhIntraDirCb;               }
  UChar*        getIntraDirCr           ()                        { return m_puhIntraDirCr;               }
  UChar        getIntraDirCb           ( UInt uiIdx )                        { return m_puhIntraDirCb[uiIdx];               }
  UChar        getIntraDirCr           ( UInt uiIdx )                        { return m_puhIntraDirCr[uiIdx];               }
  Void          setIntraDirSubPartsCb( UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth);
  Void          setIntraDirSubPartsCr( UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth);
#endif

  // -------------------------------------------------------------------------------------------------------------------
  // member functions for accessing partition information
  // -------------------------------------------------------------------------------------------------------------------

  Void          getPartIndexAndSize   ( UInt uiPartIdx, UInt& ruiPartAddr, Int& riWidth, Int& riHeight );
  UChar         getNumPartInter       ();

  Bool          isFirstAbsZorderIdxInDepth (UInt uiAbsPartIdx, UInt uiDepth);
#if INTER612

#if niu_mvpred_debug
  Void          getPartIndexFromAddr(UInt& uiPartIdx, UInt uiAbsIdxInLCU, UInt uiAbsPartIdx);
#else
  Void          getPartIndexFromAddr(UInt& uiPartIdx, UInt ruiPartAddr);
#endif

#endif
  // -------------------------------------------------------------------------------------------------------------------
  // member functions for motion vector
  // -------------------------------------------------------------------------------------------------------------------
  
  Void          getMv                 ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPic eRefPic, TComMv& rcMv );
  Void          clipMv                ( TComMv&     rcMv     );
#if PSKIP
  TComMvField getTmpFirstMvPred(Int i) { return m_cTmpFirstMvPred[i]; }
  TComMvField getTmpSecondMvPred(Int i) { return m_cTmpSecondMvPred[i]; }


#endif

#if F_DHP_SYC
  Void MV_Scale(TComDataCU *pcCU, TComMv &cMvPred, TComMv tempmv, Int iSrcRef, Int iDstRef);
#endif

#if RD_MVP
	//认为getMvPred反回时需要增加iRefIdx
	//TComMvField        getMvFieldPred(UInt uiAbsPartIdx, RefPic eRefPic);
	TComMvField   getMvFieldPred(UInt uiAbsPartIdx, RefPic eRefPic, Int iRef);
	Int           scaleMotionVector(Int iMotionVector, RefPic eRefPic, Int eCurrBlkRef, Int eNbrBlkRef, Int iRef, Int iDelta2);
	Int           calculateDistance(RefPic eRefPic, Int  iBlkRef, Int iFwBw);
	Int           scaleMotionVector_y1(Int iMvY, RefPic eRefPic, Int iCurrBlkRef, Int iNbrBlkRef, Int iRef);
	Int           getDeltas(Int *iDelta,          //delt for original MV
		Int *iDelta2,         //delt for scaled MV
		Int iOrigPOC,
		Int iOrigRefPOC,
		Int iScaledPOC,
		Int iScaledRefPOC);
#else
#if RPS
  //认为getMvPred反回时需要增加iRefIdx
  TComMvField        getMvFieldPred(UInt uiAbsPartIdx, RefPic eRefPic);
#else
  TComMv        getMvPred(UInt uiAbsPartIdx, RefPic eRefPic);
#endif
#endif

#if BSKIP
  Void setSkipMotionVectorPredictor(UInt uiAbsPartIdx);
#endif

#if RD_PMVR
	Void          pmvrMvdDerivation(TComMv& mvd, TComMv mv, TComMv mvp);
	Void          pmvrMvDerivation(TComMv& mv, TComMv mvd, TComMv mvp);
	static _inline Int           pmvrSign(Int val);
#endif

#if DPB
	Void          compressMvAndRef();
#endif

#if PSKIP
	Void   setPSkipMvField(UInt uiAbsPartIdx);
#endif


  // -------------------------------------------------------------------------------------------------------------------
  // utility functions for neighbouring information
  // -------------------------------------------------------------------------------------------------------------------
  
  TComDataCU*   getCULeft                   () { return m_pcCULeft;       }
  TComDataCU*   getCUAbove                  () { return m_pcCUAbove;      }
  TComDataCU*   getCUAboveLeft              () { return m_pcCUAboveLeft;  }
  TComDataCU*   getCUAboveRight             () { return m_pcCUAboveRight; }
  

  TComDataCU*   getPULeft                   ( UInt&  uiLPartUnitIdx , UInt uiCurrPartUnitIdx );
  TComDataCU*   getPUAbove                  ( UInt&  uiAPartUnitIdx , UInt uiCurrPartUnitIdx );
  TComDataCU*   getPUAboveLeft              ( UInt&  uiALPartUnitIdx, UInt uiCurrPartUnitIdx );
#if INTER611
  TComDataCU*   getPUAboveRight(UInt&  uiARPartUnitIdx, UInt uiCurrPartUnitIdx, UInt shift);
#else
  TComDataCU*   getPUAboveRight(UInt&  uiARPartUnitIdx, UInt uiCurrPartUnitIdx);
#endif
  


#if BSKIP || PSKIP
  TComDataCU*   getPULeft1(UInt&  uiLPartUnitIdx, UInt uiCurrPartUnitIdx, UInt shift);
  TComDataCU*   getPUAbove1(UInt&  uiAPartUnitIdx, UInt uiCurrPartUnitIdx, UInt shift);
#endif


  TComDataCU*   getPUBelowLeft              ( UInt& uiBLPartUnitIdx, UInt uiCurrPartUnitIdx );
  TComDataCU*   getPUAboveRightAdi          ( UInt&  uiARPartUnitIdx, UInt uiPuWidth, UInt uiCurrPartUnitIdx );
  TComDataCU*   getPUBelowLeftAdi           ( UInt& uiBLPartUnitIdx, UInt uiPuHeight, UInt uiCurrPartUnitIdx );
#if ZHANGYI_INTRA_SDIP
  Void copyToPicLuma(TComDataCU* pcCU, TComPicYuv* pcPicYuvDst, TComYuv* pcRecoYuvSrc, UInt iCuAddr, UInt uiAbsZorderIdx, UInt uiDepth, UInt uiPartIdx);
  TComDataCU* getPULeftSDIP(UInt& uiLPartUnitIdx, UInt uiCurrPartUnitIdx);
  TComDataCU* getPUAboveSDIP(UInt& uiLPartUnitIdx, UInt uiCurrPartUnitIdx);
  TComMv getMvPredDec(UInt uiAbsPartIdx, RefPic eRefPic);
  UChar*        getSDIPFlag()                        { return m_pSDIPFlag; }
  UChar         getSDIPFlag(UInt uiIdx)            { return m_pSDIPFlag[uiIdx]; }
  Void          setSDIPFlag(UInt uiIdx, UChar  uh) { m_pSDIPFlag[uiIdx] = uh; }
  Void          setSDIPFlagSubParts(UChar SDIPFlag, UInt uiAbsPartIdx, UInt uiDepth);
  UChar*        getSDIPDirection()                        { return m_pSDIPDirection; }
  UChar         getSDIPDirection(UInt uiIdx)            { return m_pSDIPDirection[uiIdx]; }
  Void          setSDIPDirection(UInt uiIdx, UChar  uh) { m_pSDIPDirection[uiIdx] = uh; }
  Void          setSDIPDirectionSubParts(UChar Direction, UInt uiAbsPartIdx, UInt uiDepth);
  UInt          convertNonSquareUnitToLine(UInt uiAbsPartIdx);
  UInt          convertLineToNonSquareUnit(UInt uiAbsPartIdx, UInt uiLine);
#endif


  Void          deriveLeftRightTopIdx       ( UInt uiPartIdx, UInt& ruiPartIdxLT, UInt& ruiPartIdxRT );
  Void          deriveLeftBottomIdx         ( UInt uiPartIdx, UInt& ruiPartIdxLB );
  
  Void          deriveLeftRightTopIdxAdi    ( UInt& ruiPartIdxLT, UInt& ruiPartIdxRT, UInt uiPartOffset, UInt uiPartDepth );
  Void          deriveLeftBottomIdxAdi      ( UInt& ruiPartIdxLB, UInt  uiPartOffset, UInt uiPartDepth );
  
  // -------------------------------------------------------------------------------------------------------------------
  // member functions for modes
  // -------------------------------------------------------------------------------------------------------------------
  
  Bool          isSkip    ( UInt uiPartIdx )  { return m_pePredMode[ uiPartIdx ] == MODE_SKIP;  }
  Bool          isIntra   ( UInt uiPartIdx )  { return m_pePredMode[ uiPartIdx ] == MODE_INTRA; }
#if ZHOULULU_RDOQ
  Bool          isInter(UInt uiPartIdx)  { return m_pePredMode[uiPartIdx] == MODE_INTER; }
#endif
  // -------------------------------------------------------------------------------------------------------------------
  // member functions for SBAC context
  // -------------------------------------------------------------------------------------------------------------------
  
  UInt          getCtxSplitFlag                 ( UInt   uiAbsPartIdx, UInt uiDepth                   );
  UInt          getCtxQtCbf                     ( UInt   uiAbsPartIdx, TextType eType, UInt uiTrDepth );
  UInt          getCtxQtRootCbf                 ( UInt   uiAbsPartIdx                                 );
  UInt          getCtxPredMode                  ( UInt   uiAbsPartIdx                                 );
  UInt          getCtxInterDir                  ( UInt   uiAbsPartIdx                                 );
  
  // -------------------------------------------------------------------------------------------------------------------
  // member functions for RD cost storage
  // -------------------------------------------------------------------------------------------------------------------
  
  Double&       getTotalCost()                  { return m_dTotalCost;        }
  UInt&         getTotalDistortion()            { return m_uiTotalDistortion; }
  UInt&         getTotalBits()                  { return m_uiTotalBits;       }
  UInt&         getTotalNumPart()               { return m_uiNumPartition;    }
};

#endif

