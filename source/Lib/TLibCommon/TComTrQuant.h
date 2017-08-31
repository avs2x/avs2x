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

/** \file     TComTrQuant.h
    \brief    transform and quantization class (header)
*/

#ifndef __TCOMTRQUANT__
#define __TCOMTRQUANT__

#include "CommonDef.h"
#include "TComYuv.h"
#include "TComDataCU.h"

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define QP_BITS                 15

// AQO Parameter
#define QOFFSET_BITS            15
#define QOFFSET_BITS_LTR        9

// LTR Butterfly Paramter
#define ECore16Shift            10
#define DCore16Shift            10
#define ECore32Shift            10
#define DCore32Shift            10

#define DenShift16              6
#define DenShift32              8

// ====================================================================================================================
// Type definition
// ====================================================================================================================

typedef struct
{
  Int runBits[6][2][2];
  Int levelBits[6][2][2];
  Int lastBits[1][2];
  Int blockRootQtCbfBits[2];
  Int blockQtCbfBits[3][2];
} estBitsSbacStruct;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// QP class
class QpParam
{
public:
  QpParam();
  
  Int m_iQP;

  Int m_iForwardMultiplier, m_iInverseMultiplier, m_iInverseShift;
public:
  Void setQpParam( Int iQP )
  {
      assert ( iQP >= MIN_QP && iQP <= MAX_QP );
      m_iQP   = iQP;

#if ZHOULULU_QT 
      m_iInverseMultiplier = g_uiDeQuantTable[iQP];
      m_iForwardMultiplier = g_uiQuantTable[iQP];
      m_iInverseShift      = g_uiDeQuantShift[iQP];
#else
    m_iInverseMultiplier = g_uiQuantStep[iQP];
    m_iForwardMultiplier = (int)(1048576/m_iInverseMultiplier);
#endif
  }
  
  Void clear()
  {
    m_iQP   = 0;
  }
#if ZHOULULU_QT
  __inline int getDeQuantShift() {return m_iInverseShift; }
#endif
  __inline int getQuantValue() {return m_iForwardMultiplier; }

  __inline int getDequantValue() {return m_iInverseMultiplier; }
}; // END CLASS DEFINITION QpParam

/// transform and quantization class
class TComTrQuant
{
private:
  TCoeff m_aiBlock[MAX_CU_SIZE*MAX_CU_SIZE];
  TCoeff m_aiCoeff[MAX_CU_SIZE*MAX_CU_SIZE];
public:
  TComTrQuant();
  ~TComTrQuant();

  // initialize class
  Void init                 ( UInt uiMaxTrSize, Bool bUseRDOQ = false,  Bool bEnc = false );
#if RD_NSQT
  Void setBlockSize(TComDataCU* pcCU, UInt uiLog2Size, TextType eTType, UInt uiAbsPartIdx, UInt uiTrMode);
#endif
  // transform & inverse transform functions
#if ZHOULULU_QT
  Void setBlockSize         (TComDataCU* pcCU, UInt uiLog2Size, TextType eTType, UInt uiAbsPartIdx);
  Void transformMxN         ( TComDataCU* pcCU, Pel*   pcResidual, UInt uiStride, TCoeff*& rpcCoeff, UInt uiLog2Size, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx );
  Void invtransformMxN      ( TComDataCU* pcCU, Pel*& rpcResidual, UInt uiStride, TCoeff*   pcCoeff, UInt uiLog2Size, TextType eTType, UInt uiAbsPartIdx );
#else
  Void transformNxN         ( TComDataCU* pcCU, Pel*   pcResidual, UInt uiStride, TCoeff*& rpcCoeff, UInt uiLog2Size, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx );
  Void invtransformNxN      ( Pel*& rpcResidual, UInt uiStride, TCoeff*   pcCoeff, UInt uiLog2Size, TextType eTType );
#endif
#if wlq_WQ
  Void quant_B8(TComDataCU* pcCU, UShort usQ, Int mode, TCoeff* piCoef, TCoeff* piQCoef, Int iWidth, Int iHeight, UInt& uiAcSum, UInt uiLog2Size, Int sample_bit_depth);
#endif
  Void invRecurTransformNxN( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eTxt, Pel*& rpcResidual, UInt uiAddr, UInt uiStride, UInt uiLog2Size, UInt uiMaxTrMode, UInt uiTrMode, TCoeff* rpcCoeff );

  // Misc functions
  Void setQp( Int iQp, Int iCQp );
  Void setQPforQuant( TextType eTxtType);
  Void setLambda(Double dLambda) { m_dLambda = dLambda;}
  Void setChromaLambda(Double dLambda) { m_dChromaLambda = dLambda;}
  estBitsSbacStruct* m_pcEstBitsSbac;
#if wlq_WQ
  Void setWeightQuantEnableFlag(Bool WeightQuantEnableFlag)  { m_weightQuantEnableFlag = WeightQuantEnableFlag; }
  Bool getWeightQuantEnableFlag()  { return m_weightQuantEnableFlag; }
  Int* getQuantCoeff(UInt size)		{ return m_quantCoef[size]; };  //!< get Quant Coefficent
  Int* getDequantCoeff(UInt size)	{ return m_dequantCoef[size]; };  //!< get DeQuant Coefficent
  Short* getWqMatrix(UInt i, UInt j)		{ return m_wq_matrix[i][j]; }
#endif

protected:
#if ZHOULULU_QT
    Int m_iWidth;
    Int m_iHeight;
#endif
  TCoeff*  m_piTempCoeff;
  UInt*    m_puiQuantMtx;
  
  QpParam* m_pQP;  // Qp to be used
  QpParam  m_cLQP; // Luma Qp
  QpParam  m_cCQP; // Chroma Qp
  Double   m_dLambda;
  Double   m_dCurrLambda;
  Double   m_dChromaLambda;
  
  UInt     m_uiMaxPhTrSize;   // max physical transform size
  Bool     m_bEnc;
  Bool     m_bUseRDOQ;
#if wlq_WQ
  Bool	   m_weightQuantEnableFlag;
  Int      *m_quantCoef[SCALING_LIST_SIZE_NUM]; ///< array of quantization matrix coefficient 4x4
  Int      *m_dequantCoef[SCALING_LIST_SIZE_NUM]; ///< array of dequantization matrix coefficient 4x4
  Short    *m_wq_matrix[2][2];					//wq_matrix[matrix_id][detail/undetail][coef]
#endif

private:
#if ZHOULULU_QT
  // forward Transform
  Void xTansformNxN  ( TComDataCU* pcCU, Pel* pResidual, UInt uiStride, TCoeff* plCoeff, Int iLog2Size, TextType eTType, UInt uiAbsPartIdx );
  Void xTansformNSQT ( TComDataCU* pcCU, Pel* pResidual, UInt uiStride, TCoeff* plCoeff, Int iWidth, Int iHeight, UInt iLog2Size, TextType eTType, UInt uiAbsPartIdx );
  Void xQuant       ( TComDataCU* pcCU, TCoeff* pSrc, TCoeff*& pDes, UInt iLog2Size, Int iWidth, Int iHeight, UInt& uiAcSum, TextType eTType, UInt uiAbsPartIdx );
#else
  Void xT   ( Pel* pResidual, UInt uiStride, TCoeff* plCoeff, Int iSize );
  Void xQuant       ( TComDataCU* pcCU, TCoeff* pSrc, TCoeff*& pDes, Int iLog2Size, UInt& uiAcSum, TextType eTType, UInt uiAbsPartIdx );
#endif
  // quantization  
#if wlq_WQ
  Void initScalingList();
  Void destroyScalingList();
#endif
  // RDOQ functions
  Void           xRateDistOptQuant ( TComDataCU*                     pcCU,
                                     TCoeff*                         plSrcCoeff,
                                     TCoeff*&                        piDstCoeff,
                                     UInt                            uiLog2Size,
                                     UInt&                           uiAbsSum,
                                     TextType                        eTType,
                                     UInt                            uiAbsPartIdx );

  __inline UInt  xGetCodedLevel    ( Double&                         rd64UncodedCost,
                                     Double&                         rd64CodedCost,
                                     Long                            lLevelDouble,
                                     UInt                            uiMaxAbsLevel,
                                     UInt                            uiRun,
                                     UShort                          uiCtxRun,
                                     UShort                          uiCtxLevel,
                                     Int                             iQBits,
                                     Double                          dTemp   ) const;

  __inline Double xGetICRateCost   ( UInt                            uiAbsLevel,
                                     UInt                            uiRun,
                                     UInt                            uiCtxRun,
                                     UInt                            uiCtxLevel   ) const;

  __inline Double xGetICost        ( Double                          dRate         ) const; 
  __inline Double xGetIEPRate      (                                               ) const;

#if ZHOULULU_QT
#if wlq_WQ
  Void xDeQuant			( TComDataCU* pcCU, TCoeff* pSrc, TCoeff*& pDes, Int iLog2Size);
#else
  Void xDeQuant         ( TCoeff* pSrc,     TCoeff*& pDes, Int iLog2Size);
#endif
  Void xIT              ( TComDataCU* pcCU, TCoeff* plCoef, Pel* pResidual, UInt uiStride, Int iLog2Size, TextType eTType, UInt uiAbsPartIdx );
  Void xINSQT           ( TComDataCU* pcCU, TCoeff* plCoef, Pel* pResidual, UInt uiStride, Int iLog2Size, TextType eTType, UInt uiAbsPartIdx );
#else
  // dequantization
  Void xDeQuant         ( TCoeff* pSrc,     TCoeff*& pDes, Int iLog2Size );

  // inverse transform
  Void xIT    ( TCoeff* plCoef, Pel* pResidual, UInt uiStride, Int iSize ); 
#endif

};// END CLASS DEFINITION TComTrQuant


#endif // __TCOMTRQUANT__

