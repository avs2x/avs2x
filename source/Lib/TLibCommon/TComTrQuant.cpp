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

/** \file     TComTrQuant.cpp
    \brief    transform and quantization class
*/

#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include "TComTrQuant.h"
#include "TComPic.h"
#include "ContextTables.h"
#if ZHOULULU_RDOQ
#include "ContextModel.h"
#endif
// ====================================================================================================================
// Constants
// ====================================================================================================================

#define RDOQ_CHROMA                 1           ///< use of RDOQ in chroma

#if ZHOULULU_QT
#define FACTO_BIT                   5           ///< use for array shift in transform 
#define LIMIT_BIT                   16          ///< use for array shift in transform 
#define MAX_PHSIZE                  32
#define MAX_LOG2PHSIZE              5
#define SEC_TR_MIN_BITSIZE          3           ///< apply secT to greater than or equal to 8x8 block
#define SEC_TR_SIZE                 4

_inline int sign(int a, int b)
{
    int x;
    x = abs(a);

    if (b >= 0) 
    {
        return x;
    }
    else 
    {
        return -x;
    }
}

#endif
// ====================================================================================================================
// Tables
// ====================================================================================================================

// State mapping table for RDOQ handling
UChar stateMappingTable[113]=
{0, 16, 31, 45, 59, 62, 62, 62, 62, 62, 62, 62, 62, 62, 0, 6, 12, 19, 25, 30, 36, 42, 47, 53, 59, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 0, 3, 7, 11, 15, 19, 23, 27, 30, 34, 38, 41, 45, 48, 52, 55, 58, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 0, 2, 5, 8, 11, 14, 17, 20, 22, 25, 28, 30, 33, 36, 38, 41, 0, 2, 4, 7, 9, 12, 14, 16, 0, 2, 4, 6, 8, 10, 12, 0, 1, 3, 5, 7, 8, 1, 2, 4, 5, 0, 1, 2, 0, 1, 0, 0, 0};

// RDOQ parameter
Int entropyBits[128]=
{
  895,    943,    994,   1048,   1105,   1165,   1228,   1294,
  1364,   1439,   1517,   1599,   1686,   1778,   1875,   1978,
  2086,   2200,   2321,   2448,   2583,   2725,   2876,   3034,
  3202,   3380,   3568,   3767,   3977,   4199,   4435,   4684,
  4948,   5228,   5525,   5840,   6173,   6527,   6903,   7303,
  7727,   8178,   8658,   9169,   9714,  10294,  10914,  11575,
  12282,  13038,  13849,  14717,  15650,  16653,  17734,  18899,
  20159,  21523,  23005,  24617,  26378,  28306,  30426,  32768,
  32768,  35232,  37696,  40159,  42623,  45087,  47551,  50015,
  52479,  54942,  57406,  59870,  62334,  64798,  67262,  69725,
  72189,  74653,  77117,  79581,  82044,  84508,  86972,  89436,
  91900,  94363,  96827,  99291, 101755, 104219, 106683, 109146,
  111610, 114074, 116538, 119002, 121465, 123929, 126393, 128857,
  131321, 133785, 136248, 138712, 141176, 143640, 146104, 148568,
  151031, 153495, 155959, 158423, 160887, 163351, 165814, 168278,
  170742, 173207, 175669, 178134, 180598, 183061, 185525, 187989
};

// ====================================================================================================================
// Qp class member functions
// ====================================================================================================================

QpParam::QpParam()
{
}

// ====================================================================================================================
// TComTrQuant class member functions
// ====================================================================================================================

TComTrQuant::TComTrQuant()
{
  // allocate temporary buffers
  m_piTempCoeff = new TCoeff[ MAX_CU_SIZE*MAX_CU_SIZE ];
  
  // allocate bit estimation class  (for RDOQ)
  m_pcEstBitsSbac = new estBitsSbacStruct;
#if wlq_WQ
  initScalingList();
#endif
}

TComTrQuant::~TComTrQuant()
{
  // delete temporary buffers
  if ( m_piTempCoeff )
  {
    delete [] m_piTempCoeff;
    m_piTempCoeff = NULL;
  }
  
  // delete bit estimation class
  if ( m_pcEstBitsSbac ) delete m_pcEstBitsSbac;
#if wlq_WQ
  destroyScalingList();
#endif
}

/// Including Chroma QP Parameter setting
Void TComTrQuant::setQp( Int iQp, Int iCQp )
{
  iQp = Max( Min( iQp, MAX_QP ), 0 );
  m_cLQP.setQpParam( iQp );

  iCQp = Max( Min( iCQp, MAX_QP ), 0 );
  m_cCQP.setQpParam( iCQp );
}
Void TComTrQuant::setQPforQuant( TextType eTxtType )
{
  if( eTxtType == TEXT_LUMA )
  {
    m_pQP = &m_cLQP;
    m_dCurrLambda = m_dLambda;
  }
  else
  {
    m_pQP = &m_cCQP;
    m_dCurrLambda = m_dChromaLambda;
  }
}
#if MATRIX_MULT
/** NxN forward transform (2D) using brute force matrix multiplication (3 nested loops)
 *  \param block pointer to input data (residual)
 *  \param coeff pointer to output data (transform coefficients)
 *  \param uiStride stride of input data
 *  \param uiTrSize transform size (uiTrSize x uiTrSize)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
void xTr(Pel *block, TCoeff *coeff, UInt uiStride, UInt uiTrSize)
{
  Int i,j,k,iSum;
  Int tmp[32*32];
  const TCoeff *iT;
  UInt uiLog2TrSize = g_aucConvertToBit[ uiTrSize ];

  if (uiTrSize==2)
  {
    iT  = g_aiT2[0];
  }
  else
  if (uiTrSize==4)
  {
    iT  = g_aiT4[0];
  }
  else if (uiTrSize==8)
  {
    iT = g_aiT8[0];
  }
  else if (uiTrSize==16)
  {
    iT = g_aiT16[0];
  }
  else if (uiTrSize==32)
  {
    iT = g_aiT32[0];
  }
  else{
    assert(0);
  }

  int shift_1st = uiLog2TrSize - 1; // log2(N) - 1
  int add_1st = 1<<(shift_1st-1);
  int shift_2nd = uiLog2TrSize + 6;
  int add_2nd = 1<<(shift_2nd-1);

  /* Horizontal transform */
  for (i=0; i<uiTrSize; i++)
  {
    for (j=0; j<uiTrSize; j++)
    {
      iSum = 0;
      for (k=0; k<uiTrSize; k++)
      {
        iSum += iT[i*uiTrSize+k]*block[j*uiStride+k];
      }
      tmp[i*uiTrSize+j] = (iSum + add_1st)>>shift_1st;
    }
  }
  /* Vertical transform */
  for (i=0; i<uiTrSize; i++)
  {                 
    for (j=0; j<uiTrSize; j++)
    {
      iSum = 0;
      for (k=0; k<uiTrSize; k++)
      {
        iSum += iT[i*uiTrSize+k]*tmp[j*uiTrSize+k];        
      }
      coeff[i*uiTrSize+j] = (iSum + add_2nd)>>shift_2nd; 
    }
  }  
}

/** NxN inverse transform (2D) using brute force matrix multiplication (3 nested loops)
 *  \param coeff pointer to input data (transform coefficients)
 *  \param block pointer to output data (residual)
 *  \param uiStride stride of output data
 *  \param uiTrSize transform size (uiTrSize x uiTrSize)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
void xITr(TCoeff *coeff, Pel *block, UInt uiStride, UInt uiTrSize)
{
  int i,j,k,iSum;
  Int tmp[32*32];
  const TCoeff *iT;
  UInt uiLog2TrSize = g_aucConvertToBit[ uiTrSize ];

  if (uiTrSize==2)
  {
    iT  = g_aiT2[0];
  }
  else
  if (uiTrSize==4)
  {
    iT  = g_aiT4[0];
  }
  else if (uiTrSize==8)
  {
    iT = g_aiT8[0];
  }
  else if (uiTrSize==16)
  {
    iT = g_aiT16[0];
  }
  else if (uiTrSize==32)
  {
    iT = g_aiT32[0];
  }
  else{
    assert(0);
  }
  int shift_1st = SHIFT_INV_1ST;
  int add_1st = 1<<(shift_1st-1);  
  int shift_2nd = SHIFT_INV_2ND;
  int add_2nd = 1<<(shift_2nd-1);

  /* Horizontal transform */
  for (i=0; i<uiTrSize; i++)
  {    
    for (j=0; j<uiTrSize; j++)
    {
      iSum = 0;
      for (k=0; k<uiTrSize; k++)
      {        
        iSum += iT[k*uiTrSize+i]*coeff[k*uiTrSize+j]; 
      }
      tmp[i*uiTrSize+j] = (iSum + add_1st)>>shift_1st;
    }
  }   
  /* Vertical transform */
  for (i=0; i<uiTrSize; i++)
  {   
    for (j=0; j<uiTrSize; j++)
    {
      iSum = 0;
      for (k=0; k<uiTrSize; k++)
      {        
        iSum += iT[k*uiTrSize+j]*tmp[i*uiTrSize+k];
      }
      block[i*uiStride+j] = (iSum + add_2nd)>>shift_2nd;
    }
  }
}

#else //MATRIX_MULT

/** 2x2 forward transform implemented using partial butterfly structure (1D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterfly2(TCoeff* block,TCoeff* coeff,int shift)
{
  int j;  
  int E,O;
#if TANSFOR_BUG_YQH
  int add = shift == 0 ? 0 : 1<<(shift-1);
  if(shift<0)
	  printf("error transform shift <0");
#else
  int add = 1<<(shift-1);
#endif

  for (j=0; j<2; j++)
  {    
    /* E and O */
    E = block[j*2+0] + block[j*2+1];
    O = block[j*2+0] - block[j*2+1];

    coeff[0*2+j] = (g_aiT2[0][0]*E + add)>>shift;
    coeff[1*2+j] = (g_aiT2[1][0]*O + add)>>shift;
  }
}
/** 2x2 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
void xTr2(TCoeff *block,TCoeff* coeff)
{
  int shift_1st = 0;        // log2(2) - 1
  int shift_2nd = 7;        // log2(2) + 6
  TCoeff tmp[2][2]; 

  partialButterfly2(block,&tmp[0][0],shift_1st);
  partialButterfly2(&tmp[0][0],coeff,shift_2nd);
}

/** 2x2 inverse transform implemented using partial butterfly structure (1D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterflyInverse2(TCoeff* tmp,TCoeff* block,int shift)
{
  partialButterfly2( tmp, block, shift ); // for 2x2, fwd = inv since A-1 = AT = A
}

/** 2x2 inverse transform (2D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
void xITr2(TCoeff* coeff,TCoeff* block)
{
  int shift_1st = SHIFT_INV_1ST;
  int shift_2nd = SHIFT_INV_2ND;
  TCoeff tmp[2][2];

  partialButterflyInverse2(coeff,&tmp[0][0],shift_1st);
  partialButterflyInverse2(&tmp[0][0],block,shift_2nd);
}
/** 4x4 forward transform implemented using partial butterfly structure (1D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 *  \param shift specifies right shift after 1D transform
 */
#if ZHOULULU_QT
void partialButterfly4(TCoeff* block,TCoeff* coeff, int iNumRows, int shift)
{
  int j;  
  int E[2],O[2];
#if TANSFOR_BUG_YQH
  int add = shift == 0 ? 0 : 1<<(shift-1);
  if(shift<0)
	  printf("error transform shift <0");
#else
  int add = 1<<(shift-1);
#endif

  for (j=0; j<iNumRows; j++)
  {    
    /* E and O */
    E[0] = block[j*4+0] + block[j*4+3];
    O[0] = block[j*4+0] - block[j*4+3];
    E[1] = block[j*4+1] + block[j*4+2];
    O[1] = block[j*4+1] - block[j*4+2];

    coeff[0*iNumRows+j] = (g_aiT4[0][0]*E[0] + g_aiT4[0][1]*E[1] + add)>>shift;
    coeff[2*iNumRows+j] = (g_aiT4[2][0]*E[0] + g_aiT4[2][1]*E[1] + add)>>shift;
    coeff[1*iNumRows+j] = (g_aiT4[1][0]*O[0] + g_aiT4[1][1]*O[1] + add)>>shift;
    coeff[3*iNumRows+j] = (g_aiT4[3][0]*O[0] + g_aiT4[3][1]*O[1] + add)>>shift;
  }

}

/** 4x4 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
void xTr4(TCoeff* block,TCoeff* coeff, int shift_1st, int shift_2nd)
{  
  TCoeff tmp[4][4]; 
  
  partialButterfly4(block,&tmp[0][0], 4, shift_1st);
  partialButterfly4(&tmp[0][0],coeff, 4, shift_2nd);
}

/** 4x4 inverse transform implemented using partial butterfly structure (1D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterflyInverse4(TCoeff* tmp,TCoeff* block, int iNumRows, int shift, int bit_depth)
{
  int j;    
  int E[2],O[2];
#if TANSFOR_BUG_YQH
  int add = shift == 0 ? 0 : 1<<(shift-1);
  if(shift<0)
	  printf("error transform shift <0");
#else
  int add = 1<<(shift-1);
#endif
  int min_val, max_val;

  max_val = (1 << (bit_depth - 1)) - 1;
  min_val = - max_val - 1;

  for (j=0; j<iNumRows; j++)
  {    
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */    
    O[0] = g_aiT4[1][0]*tmp[1*iNumRows+j] + g_aiT4[3][0]*tmp[3*iNumRows+j];
    O[1] = g_aiT4[1][1]*tmp[1*iNumRows+j] + g_aiT4[3][1]*tmp[3*iNumRows+j];
    E[0] = g_aiT4[0][0]*tmp[0*iNumRows+j] + g_aiT4[2][0]*tmp[2*iNumRows+j];
    E[1] = g_aiT4[0][1]*tmp[0*iNumRows+j] + g_aiT4[2][1]*tmp[2*iNumRows+j];
    
    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */ 
    block[j*4+0] =  Clip3(min_val, max_val, (E[0] + O[0] + add)>>shift);
    block[j*4+1] =  Clip3(min_val, max_val, (E[1] + O[1] + add)>>shift);
    block[j*4+2] =  Clip3(min_val, max_val, (E[1] - O[1] + add)>>shift);
    block[j*4+3] =  Clip3(min_val, max_val, (E[0] - O[0] + add)>>shift);
  }
}

/** 4x4 inverse transform (2D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
void xITr4(TCoeff* coeff,TCoeff* block,int shift_1st, int shift_2nd, int clipValue_1st, int clipValue_2nd)
{
  TCoeff tmp[4][4];
  
  partialButterflyInverse4(coeff,&tmp[0][0],4,shift_1st,clipValue_1st);
  partialButterflyInverse4(&tmp[0][0],block,4,shift_2nd,clipValue_2nd);
}
#else
void partialButterfly4(TCoeff* block,TCoeff* coeff,int shift)
{
  int j;  
  int E[2],O[2];
  int add = 1<<(shift-1);

  for (j=0; j<4; j++)
  {    
    /* E and O */
    E[0] = block[j*4+0] + block[j*4+3];
    O[0] = block[j*4+0] - block[j*4+3];
    E[1] = block[j*4+1] + block[j*4+2];
    O[1] = block[j*4+1] - block[j*4+2];

    coeff[0*4+j] = (g_aiT4[0][0]*E[0] + g_aiT4[0][1]*E[1] + add)>>shift;
    coeff[2*4+j] = (g_aiT4[2][0]*E[0] + g_aiT4[2][1]*E[1] + add)>>shift;
    coeff[1*4+j] = (g_aiT4[1][0]*O[0] + g_aiT4[1][1]*O[1] + add)>>shift;
    coeff[3*4+j] = (g_aiT4[3][0]*O[0] + g_aiT4[3][1]*O[1] + add)>>shift;
  }
}

/** 4x4 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
void xTr4(TCoeff* block,TCoeff* coeff)
{
  int shift_1st = 1;        // log2(4) - 1
  int shift_2nd = 8;        // log2(4) + 6
  TCoeff tmp[4][4]; 

  partialButterfly4(block,&tmp[0][0],shift_1st);
  partialButterfly4(&tmp[0][0],coeff,shift_2nd);
}

/** 4x4 inverse transform implemented using partial butterfly structure (1D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterflyInverse4(TCoeff* tmp,TCoeff* block,int shift)
{
  int j;    
  int E[2],O[2];
  int add = 1<<(shift-1);

  for (j=0; j<4; j++)
  {    
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */    
    O[0] = g_aiT4[1][0]*tmp[1*4+j] + g_aiT4[3][0]*tmp[3*4+j];
    O[1] = g_aiT4[1][1]*tmp[1*4+j] + g_aiT4[3][1]*tmp[3*4+j];
    E[0] = g_aiT4[0][0]*tmp[0*4+j] + g_aiT4[2][0]*tmp[2*4+j];
    E[1] = g_aiT4[0][1]*tmp[0*4+j] + g_aiT4[2][1]*tmp[2*4+j];
    
    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */ 
    block[j*4+0] = (E[0] + O[0] + add)>>shift;
    block[j*4+1] = (E[1] + O[1] + add)>>shift;
    block[j*4+2] = (E[1] - O[1] + add)>>shift;
    block[j*4+3] = (E[0] - O[0] + add)>>shift;
  }
}

/** 4x4 inverse transform (2D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
void xITr4(TCoeff* coeff,TCoeff* block)
{
  int shift_1st = SHIFT_INV_1ST;
  int shift_2nd = SHIFT_INV_2ND;
  TCoeff tmp[4][4];
  
  partialButterflyInverse4(coeff,&tmp[0][0],shift_1st);
  partialButterflyInverse4(&tmp[0][0],block,shift_2nd);
}
#endif



/** 8x8 forward transform implemented using partial butterfly structure (1D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 *  \param shift specifies right shift after 1D transform
 */
#if ZHOULULU_QT
void partialButterfly8(TCoeff* block,TCoeff* coeff, int iNumRows, int shift)
{
  int j,k;  
  int E[4],O[4];
  int EE[2],EO[2];
#if TANSFOR_BUG_YQH
  int add = shift == 0 ? 0 : 1<<(shift-1);
#else
  int add = 1<<(shift-1);
#endif

  for (j=0; j<iNumRows; j++)
  {    
    /* E and O*/
    for (k=0;k<4;k++)
    {
      E[k] = block[j*8+k] + block[j*8+7-k];
      O[k] = block[j*8+k] - block[j*8+7-k];
    }    
    /* EE and EO */
    EE[0] = E[0] + E[3];    
    EO[0] = E[0] - E[3];
    EE[1] = E[1] + E[2];
    EO[1] = E[1] - E[2];

    coeff[0*iNumRows+j] = (g_aiT8[0][0]*EE[0] + g_aiT8[0][1]*EE[1] + add)>>shift;
    coeff[4*iNumRows+j] = (g_aiT8[4][0]*EE[0] + g_aiT8[4][1]*EE[1] + add)>>shift; 
    coeff[2*iNumRows+j] = (g_aiT8[2][0]*EO[0] + g_aiT8[2][1]*EO[1] + add)>>shift;
    coeff[6*iNumRows+j] = (g_aiT8[6][0]*EO[0] + g_aiT8[6][1]*EO[1] + add)>>shift; 

    coeff[1*iNumRows+j] = (g_aiT8[1][0]*O[0] + g_aiT8[1][1]*O[1] + g_aiT8[1][2]*O[2] + g_aiT8[1][3]*O[3] + add)>>shift;
    coeff[3*iNumRows+j] = (g_aiT8[3][0]*O[0] + g_aiT8[3][1]*O[1] + g_aiT8[3][2]*O[2] + g_aiT8[3][3]*O[3] + add)>>shift;
    coeff[5*iNumRows+j] = (g_aiT8[5][0]*O[0] + g_aiT8[5][1]*O[1] + g_aiT8[5][2]*O[2] + g_aiT8[5][3]*O[3] + add)>>shift;
    coeff[7*iNumRows+j] = (g_aiT8[7][0]*O[0] + g_aiT8[7][1]*O[1] + g_aiT8[7][2]*O[2] + g_aiT8[7][3]*O[3] + add)>>shift;
  }
}

/** 8x8 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff  output data (transform coefficients)
 */
void xTr8(TCoeff* block,TCoeff* coeff, int shift_1st, int shift_2nd)
{
  TCoeff tmp[8][8]; 

  partialButterfly8(block,&tmp[0][0], 8, shift_1st);
  partialButterfly8(&tmp[0][0],coeff, 8, shift_2nd);
}

/** 8x8 inverse transform implemented using partial butterfly structure (1D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterflyInverse8(TCoeff* tmp,TCoeff* block, int iNumRows, int shift, int bit_depth)
{
  int j,k;    
  int E[4],O[4];
  int EE[2],EO[2];
#if TANSFOR_BUG_YQH
  int add = shift == 0 ? 0 : 1<<(shift-1);
  if(shift<0)
	  printf("error transform shift <0");
#else
  int add = 1<<(shift-1);
#endif
  int min_val, max_val;

  max_val = (1 << (bit_depth - 1)) - 1;
  min_val = - max_val - 1;

  for (j=0; j<iNumRows; j++)
  {    
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
    for (k=0;k<4;k++)
    {
      O[k] = g_aiT8[ 1][k]*tmp[ 1*iNumRows+j] + g_aiT8[ 3][k]*tmp[ 3*iNumRows+j] + g_aiT8[ 5][k]*tmp[ 5*iNumRows+j] + g_aiT8[ 7][k]*tmp[ 7*iNumRows+j];
    }
   
    EO[0] = g_aiT8[2][0]*tmp[2*iNumRows+j] + g_aiT8[6][0]*tmp[6*iNumRows+j];
    EO[1] = g_aiT8[2][1]*tmp[2*iNumRows+j] + g_aiT8[6][1]*tmp[6*iNumRows+j];
    EE[0] = g_aiT8[0][0]*tmp[0*iNumRows+j] + g_aiT8[4][0]*tmp[4*iNumRows+j];
    EE[1] = g_aiT8[0][1]*tmp[0*iNumRows+j] + g_aiT8[4][1]*tmp[4*iNumRows+j];

    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */ 
    E[0] = EE[0] + EO[0];
    E[3] = EE[0] - EO[0];
    E[1] = EE[1] + EO[1];
    E[2] = EE[1] - EO[1];
    for (k=0;k<4;k++)
    {
      block[j*8+k] = Clip3(min_val, max_val,(E[k] + O[k] + add)>>shift);
      block[j*8+k+4] = Clip3(min_val, max_val,(E[3-k] - O[3-k] + add)>>shift);
    }        
  }
}

/** 8x8 inverse transform (2D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 */
void xITr8(TCoeff* coeff,TCoeff* block, int shift_1st, int shift_2nd, int clipValue_1st, int clipValue_2nd)
{
  TCoeff tmp[8][8];
  
  partialButterflyInverse8(coeff,&tmp[0][0],8,shift_1st,clipValue_1st);
  partialButterflyInverse8(&tmp[0][0],block,8,shift_2nd,clipValue_2nd);
}


#else
void partialButterfly8(TCoeff* block,TCoeff* coeff,int shift)
{
  int j,k;  
  int E[4],O[4];
  int EE[2],EO[2];
  int add = 1<<(shift-1);

  for (j=0; j<8; j++)
  {    
    /* E and O*/
    for (k=0;k<4;k++)
    {
      E[k] = block[j*8+k] + block[j*8+7-k];
      O[k] = block[j*8+k] - block[j*8+7-k];
    }    
    /* EE and EO */
    EE[0] = E[0] + E[3];    
    EO[0] = E[0] - E[3];
    EE[1] = E[1] + E[2];
    EO[1] = E[1] - E[2];

    coeff[0*8+j] = (g_aiT8[0][0]*EE[0] + g_aiT8[0][1]*EE[1] + add)>>shift;
    coeff[4*8+j] = (g_aiT8[4][0]*EE[0] + g_aiT8[4][1]*EE[1] + add)>>shift; 
    coeff[2*8+j] = (g_aiT8[2][0]*EO[0] + g_aiT8[2][1]*EO[1] + add)>>shift;
    coeff[6*8+j] = (g_aiT8[6][0]*EO[0] + g_aiT8[6][1]*EO[1] + add)>>shift; 

    coeff[1*8+j] = (g_aiT8[1][0]*O[0] + g_aiT8[1][1]*O[1] + g_aiT8[1][2]*O[2] + g_aiT8[1][3]*O[3] + add)>>shift;
    coeff[3*8+j] = (g_aiT8[3][0]*O[0] + g_aiT8[3][1]*O[1] + g_aiT8[3][2]*O[2] + g_aiT8[3][3]*O[3] + add)>>shift;
    coeff[5*8+j] = (g_aiT8[5][0]*O[0] + g_aiT8[5][1]*O[1] + g_aiT8[5][2]*O[2] + g_aiT8[5][3]*O[3] + add)>>shift;
    coeff[7*8+j] = (g_aiT8[7][0]*O[0] + g_aiT8[7][1]*O[1] + g_aiT8[7][2]*O[2] + g_aiT8[7][3]*O[3] + add)>>shift;
  }
}

/** 8x8 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff  output data (transform coefficients)
 */
void xTr8(TCoeff* block,TCoeff* coeff)
{
  int shift_1st = 2;      // log2(8) - 1
  int shift_2nd = 9;      // log2(8) + 6
  TCoeff tmp[8][8]; 

  partialButterfly8(block,&tmp[0][0],shift_1st);
  partialButterfly8(&tmp[0][0],coeff,shift_2nd);
}

/** 8x8 inverse transform implemented using partial butterfly structure (1D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterflyInverse8(TCoeff* tmp,TCoeff* block,int shift)
{
  int j,k;    
  int E[4],O[4];
  int EE[2],EO[2];
  int add = 1<<(shift-1);

  for (j=0; j<8; j++)
  {    
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
    for (k=0;k<4;k++)
    {
      O[k] = g_aiT8[ 1][k]*tmp[ 1*8+j] + g_aiT8[ 3][k]*tmp[ 3*8+j] + g_aiT8[ 5][k]*tmp[ 5*8+j] + g_aiT8[ 7][k]*tmp[ 7*8+j];
    }
   
    EO[0] = g_aiT8[2][0]*tmp[2*8+j] + g_aiT8[6][0]*tmp[6*8+j];
    EO[1] = g_aiT8[2][1]*tmp[2*8+j] + g_aiT8[6][1]*tmp[6*8+j];
    EE[0] = g_aiT8[0][0]*tmp[0*8+j] + g_aiT8[4][0]*tmp[4*8+j];
    EE[1] = g_aiT8[0][1]*tmp[0*8+j] + g_aiT8[4][1]*tmp[4*8+j];

    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */ 
    E[0] = EE[0] + EO[0];
    E[3] = EE[0] - EO[0];
    E[1] = EE[1] + EO[1];
    E[2] = EE[1] - EO[1];
    for (k=0;k<4;k++)
    {
      block[j*8+k] = (E[k] + O[k] + add)>>shift;
      block[j*8+k+4] = (E[3-k] - O[3-k] + add)>>shift;
    }        
  }
}

/** 8x8 inverse transform (2D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 */
void xITr8(TCoeff* coeff,TCoeff* block)
{
  int shift_1st = SHIFT_INV_1ST;
  int shift_2nd = SHIFT_INV_2ND;
  TCoeff tmp[8][8];
  
  partialButterflyInverse8(coeff,&tmp[0][0],shift_1st);
  partialButterflyInverse8(&tmp[0][0],block,shift_2nd);
}

#endif


/** 16x16 forward transform implemented using partial butterfly structure (1D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 *  \param shift specifies right shift after 1D transform
 */
#if ZHOULULU_QT
void partialButterfly16(TCoeff* block,TCoeff* coeff, int iNumRows, int shift)
{
  int j,k;
  int E[8],O[8];
  int EE[4],EO[4];
  int EEE[2],EEO[2];
#if TANSFOR_BUG_YQH
  int add = shift == 0 ? 0 : 1<<(shift-1);
  if(shift<0)
	  printf("error transform shift <0");
#else
  int add = 1<<(shift-1);
#endif

  for (j=0; j<iNumRows; j++)
  {    
    /* E and O*/
    for (k=0;k<8;k++)
    {
      E[k] = block[j*16+k] + block[j*16+15-k];
      O[k] = block[j*16+k] - block[j*16+15-k];
    } 
    /* EE and EO */
    for (k=0;k<4;k++)
    {
      EE[k] = E[k] + E[7-k];
      EO[k] = E[k] - E[7-k];
    }
    /* EEE and EEO */
    EEE[0] = EE[0] + EE[3];    
    EEO[0] = EE[0] - EE[3];
    EEE[1] = EE[1] + EE[2];
    EEO[1] = EE[1] - EE[2];

    coeff[ 0*iNumRows+j] = (g_aiT16[ 0][0]*EEE[0] + g_aiT16[ 0][1]*EEE[1] + add)>>shift;        
    coeff[ 8*iNumRows+j] = (g_aiT16[ 8][0]*EEE[0] + g_aiT16[ 8][1]*EEE[1] + add)>>shift;    
    coeff[ 4*iNumRows+j] = (g_aiT16[ 4][0]*EEO[0] + g_aiT16[ 4][1]*EEO[1] + add)>>shift;        
    coeff[12*iNumRows+j] = (g_aiT16[12][0]*EEO[0] + g_aiT16[12][1]*EEO[1] + add)>>shift;

    for (k=2;k<16;k+=4)
    {
      coeff[k*iNumRows+j] = (g_aiT16[k][0]*EO[0] + g_aiT16[k][1]*EO[1] + g_aiT16[k][2]*EO[2] + g_aiT16[k][3]*EO[3] + add)>>shift;      
    }
    
    for (k=1;k<16;k+=2)
    {
      coeff[k*iNumRows+j] = (g_aiT16[k][0]*O[0] + g_aiT16[k][1]*O[1] + g_aiT16[k][2]*O[2] + g_aiT16[k][3]*O[3] + 
                            g_aiT16[k][4]*O[4] + g_aiT16[k][5]*O[5] + g_aiT16[k][6]*O[6] + g_aiT16[k][7]*O[7] + add)>>shift;
    }

  }
}

/** 16x16 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 */
void xTr16(TCoeff* block,TCoeff* coeff, int shift_1st, int shift_2nd)
{
  TCoeff tmp[16][16]; 

  partialButterfly16(block,&tmp[0][0], 16, shift_1st);
  partialButterfly16(&tmp[0][0],coeff, 16, shift_2nd);
}

/** 16x16 inverse transform implemented using partial butterfly structure (1D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterflyInverse16(TCoeff* tmp,TCoeff* block, int iNumRows, int shift, int bit_depth)
{
  int j,k;  
  int E[8],O[8];
  int EE[4],EO[4];
  int EEE[2],EEO[2];
#if TANSFOR_BUG_YQH
  int add = shift == 0 ? 0 : 1<<(shift-1);
  if(shift<0)
	  printf("error transform shift <0");
#else
  int add = 1<<(shift-1);
#endif
  int min_val, max_val;

  max_val = (1 << (bit_depth - 1)) - 1;
  min_val = - max_val - 1;

  for (j=0; j<iNumRows; j++)
  {    
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
    for (k=0;k<8;k++)
    {
      O[k] = g_aiT16[ 1][k]*tmp[ 1*iNumRows+j] + g_aiT16[ 3][k]*tmp[ 3*iNumRows+j] + g_aiT16[ 5][k]*tmp[ 5*iNumRows+j] + g_aiT16[ 7][k]*tmp[ 7*iNumRows+j] + 
             g_aiT16[ 9][k]*tmp[ 9*iNumRows+j] + g_aiT16[11][k]*tmp[11*iNumRows+j] + g_aiT16[13][k]*tmp[13*iNumRows+j] + g_aiT16[15][k]*tmp[15*iNumRows+j];
    }
    for (k=0;k<4;k++)
    {
      EO[k] = g_aiT16[ 2][k]*tmp[ 2*iNumRows+j] + g_aiT16[ 6][k]*tmp[ 6*iNumRows+j] + g_aiT16[10][k]*tmp[10*iNumRows+j] + g_aiT16[14][k]*tmp[14*iNumRows+j];
    }
    EEO[0] = g_aiT16[4][0]*tmp[4*iNumRows+j] + g_aiT16[12][0]*tmp[12*iNumRows+j];
    EEE[0] = g_aiT16[0][0]*tmp[0*iNumRows+j] + g_aiT16[ 8][0]*tmp[ 8*iNumRows+j];
    EEO[1] = g_aiT16[4][1]*tmp[4*iNumRows+j] + g_aiT16[12][1]*tmp[12*iNumRows+j];
    EEE[1] = g_aiT16[0][1]*tmp[0*iNumRows+j] + g_aiT16[ 8][1]*tmp[ 8*iNumRows+j];

    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */ 
    for (k=0;k<2;k++)
    {
      EE[k] = EEE[k] + EEO[k];
      EE[k+2] = EEE[1-k] - EEO[1-k];
    }    
    for (k=0;k<4;k++)
    {
      E[k] = EE[k] + EO[k];
      E[k+4] = EE[3-k] - EO[3-k];
    }    
    for (k=0;k<8;k++)
    {
      block[j*16+k] = Clip3(min_val,max_val,(E[k] + O[k] + add)>>shift);
      block[j*16+k+8] = Clip3(min_val,max_val,(E[7-k] - O[7-k] + add)>>shift);
    }        
  }
}

/** 16x16 inverse transform (2D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 */
void xITr16(TCoeff* coeff,TCoeff* block, int shift_1st, int shift_2nd, int clipValue_1st, int clipValue_2nd)
{
  TCoeff tmp[16][16];
  
  partialButterflyInverse16(coeff,&tmp[0][0],16,shift_1st, clipValue_1st);
  partialButterflyInverse16(&tmp[0][0],block,16,shift_2nd, clipValue_2nd);
}


#else
void partialButterfly16(TCoeff* block,TCoeff* coeff,int shift)
{
  int j,k;
  int E[8],O[8];
  int EE[4],EO[4];
  int EEE[2],EEO[2];
  int add = 1<<(shift-1);

  for (j=0; j<16; j++)
  {    
    /* E and O*/
    for (k=0;k<8;k++)
    {
      E[k] = block[j*16+k] + block[j*16+15-k];
      O[k] = block[j*16+k] - block[j*16+15-k];
    } 
    /* EE and EO */
    for (k=0;k<4;k++)
    {
      EE[k] = E[k] + E[7-k];
      EO[k] = E[k] - E[7-k];
    }
    /* EEE and EEO */
    EEE[0] = EE[0] + EE[3];    
    EEO[0] = EE[0] - EE[3];
    EEE[1] = EE[1] + EE[2];
    EEO[1] = EE[1] - EE[2];

    coeff[ 0*16+j] = (g_aiT16[ 0][0]*EEE[0] + g_aiT16[ 0][1]*EEE[1] + add)>>shift;        
    coeff[ 8*16+j] = (g_aiT16[ 8][0]*EEE[0] + g_aiT16[ 8][1]*EEE[1] + add)>>shift;    
    coeff[ 4*16+j] = (g_aiT16[ 4][0]*EEO[0] + g_aiT16[ 4][1]*EEO[1] + add)>>shift;        
    coeff[12*16+j] = (g_aiT16[12][0]*EEO[0] + g_aiT16[12][1]*EEO[1] + add)>>shift;

    for (k=2;k<16;k+=4)
    {
      coeff[k*16+j] = (g_aiT16[k][0]*EO[0] + g_aiT16[k][1]*EO[1] + g_aiT16[k][2]*EO[2] + g_aiT16[k][3]*EO[3] + add)>>shift;      
    }
    
    for (k=1;k<16;k+=2)
    {
      coeff[k*16+j] = (g_aiT16[k][0]*O[0] + g_aiT16[k][1]*O[1] + g_aiT16[k][2]*O[2] + g_aiT16[k][3]*O[3] + 
                     g_aiT16[k][4]*O[4] + g_aiT16[k][5]*O[5] + g_aiT16[k][6]*O[6] + g_aiT16[k][7]*O[7] + add)>>shift;
    }

  }
}

/** 16x16 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 */
void xTr16(TCoeff* block,TCoeff* coeff)
{
  int shift_1st = 3;    // log2(16) - 1
  int shift_2nd = 10;   // log2(16) + 6
  TCoeff tmp[16][16]; 

  partialButterfly16(block,&tmp[0][0],shift_1st);
  partialButterfly16(&tmp[0][0],coeff,shift_2nd);
}

/** 16x16 inverse transform implemented using partial butterfly structure (1D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterflyInverse16(TCoeff* tmp,TCoeff* block,int shift)
{
  int j,k;  
  int E[8],O[8];
  int EE[4],EO[4];
  int EEE[2],EEO[2];
  int add = 1<<(shift-1);

  for (j=0; j<16; j++)
  {    
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
    for (k=0;k<8;k++)
    {
      O[k] = g_aiT16[ 1][k]*tmp[ 1*16+j] + g_aiT16[ 3][k]*tmp[ 3*16+j] + g_aiT16[ 5][k]*tmp[ 5*16+j] + g_aiT16[ 7][k]*tmp[ 7*16+j] + 
             g_aiT16[ 9][k]*tmp[ 9*16+j] + g_aiT16[11][k]*tmp[11*16+j] + g_aiT16[13][k]*tmp[13*16+j] + g_aiT16[15][k]*tmp[15*16+j];
    }
    for (k=0;k<4;k++)
    {
      EO[k] = g_aiT16[ 2][k]*tmp[ 2*16+j] + g_aiT16[ 6][k]*tmp[ 6*16+j] + g_aiT16[10][k]*tmp[10*16+j] + g_aiT16[14][k]*tmp[14*16+j];
    }
    EEO[0] = g_aiT16[4][0]*tmp[4*16+j] + g_aiT16[12][0]*tmp[12*16+j];
    EEE[0] = g_aiT16[0][0]*tmp[0*16+j] + g_aiT16[ 8][0]*tmp[ 8*16+j];
    EEO[1] = g_aiT16[4][1]*tmp[4*16+j] + g_aiT16[12][1]*tmp[12*16+j];
    EEE[1] = g_aiT16[0][1]*tmp[0*16+j] + g_aiT16[ 8][1]*tmp[ 8*16+j];

    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */ 
    for (k=0;k<2;k++)
    {
      EE[k] = EEE[k] + EEO[k];
      EE[k+2] = EEE[1-k] - EEO[1-k];
    }    
    for (k=0;k<4;k++)
    {
      E[k] = EE[k] + EO[k];
      E[k+4] = EE[3-k] - EO[3-k];
    }    
    for (k=0;k<8;k++)
    {
      block[j*16+k] = (E[k] + O[k] + add)>>shift;
      block[j*16+k+8] = (E[7-k] - O[7-k] + add)>>shift;
    }        
  }
}

/** 16x16 inverse transform (2D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 */
void xITr16(TCoeff* coeff,TCoeff* block)
{
  int shift_1st = SHIFT_INV_1ST;
  int shift_2nd = SHIFT_INV_2ND;
  TCoeff tmp[16][16];
  
  partialButterflyInverse16(coeff,&tmp[0][0],shift_1st);
  partialButterflyInverse16(&tmp[0][0],block,shift_2nd);
}

#endif

/** 32x32 forward transform implemented using partial butterfly structure (1D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 *  \param shift specifies right shift after 1D transform
 */
#if ZHOULULU_QT
void partialButterfly32(TCoeff* block,TCoeff* coeff, int iNumRows, int shift)
{
  int j,k;
  int E[16],O[16];
  int EE[8],EO[8];
  int EEE[4],EEO[4];
  int EEEE[2],EEEO[2];
#if TANSFOR_BUG_YQH
  int add = shift == 0 ? 0 : 1<<(shift-1);
  if(shift<0)
	  printf("error transform shift <0");
#else
  int add = 1<<(shift-1);
#endif

  for (j=0; j<iNumRows; j++)
  {    
    /* E and O*/
    for (k=0;k<16;k++)
    {
      E[k] = block[j*32+k] + block[j*32+31-k];
      O[k] = block[j*32+k] - block[j*32+31-k];
    } 
    /* EE and EO */
    for (k=0;k<8;k++)
    {
      EE[k] = E[k] + E[15-k];
      EO[k] = E[k] - E[15-k];
    }
    /* EEE and EEO */
    for (k=0;k<4;k++)
    {
      EEE[k] = EE[k] + EE[7-k];
      EEO[k] = EE[k] - EE[7-k];
    }
    /* EEEE and EEEO */
    EEEE[0] = EEE[0] + EEE[3];    
    EEEO[0] = EEE[0] - EEE[3];
    EEEE[1] = EEE[1] + EEE[2];
    EEEO[1] = EEE[1] - EEE[2];

    coeff[ 0*iNumRows+j] = (g_aiT32[ 0][0]*EEEE[0] + g_aiT32[ 0][1]*EEEE[1] + add)>>shift;
    coeff[16*iNumRows+j] = (g_aiT32[16][0]*EEEE[0] + g_aiT32[16][1]*EEEE[1] + add)>>shift;
    coeff[ 8*iNumRows+j] = (g_aiT32[ 8][0]*EEEO[0] + g_aiT32[ 8][1]*EEEO[1] + add)>>shift; 
    coeff[24*iNumRows+j] = (g_aiT32[24][0]*EEEO[0] + g_aiT32[24][1]*EEEO[1] + add)>>shift;
    for (k=4;k<32;k+=8)
    {
      coeff[k*iNumRows+j] = (g_aiT32[k][0]*EEO[0] + g_aiT32[k][1]*EEO[1] + g_aiT32[k][2]*EEO[2] + g_aiT32[k][3]*EEO[3] + add)>>shift;
    }       
    for (k=2;k<32;k+=4)
    {
      coeff[k*iNumRows+j] = (g_aiT32[k][0]*EO[0] + g_aiT32[k][1]*EO[1] + g_aiT32[k][2]*EO[2] + g_aiT32[k][3]*EO[3] + 
                     g_aiT32[k][4]*EO[4] + g_aiT32[k][5]*EO[5] + g_aiT32[k][6]*EO[6] + g_aiT32[k][7]*EO[7] + add)>>shift;
    }       
    for (k=1;k<32;k+=2)
    {
      coeff[k*iNumRows+j] = (g_aiT32[k][ 0]*O[ 0] + g_aiT32[k][ 1]*O[ 1] + g_aiT32[k][ 2]*O[ 2] + g_aiT32[k][ 3]*O[ 3] + 
                     g_aiT32[k][ 4]*O[ 4] + g_aiT32[k][ 5]*O[ 5] + g_aiT32[k][ 6]*O[ 6] + g_aiT32[k][ 7]*O[ 7] +
                     g_aiT32[k][ 8]*O[ 8] + g_aiT32[k][ 9]*O[ 9] + g_aiT32[k][10]*O[10] + g_aiT32[k][11]*O[11] + 
                     g_aiT32[k][12]*O[12] + g_aiT32[k][13]*O[13] + g_aiT32[k][14]*O[14] + g_aiT32[k][15]*O[15] + add)>>shift;
    }
  }
}

/** 32x32 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 */
void xTr32(TCoeff* block,TCoeff* coeff, int shift_1st, int shift_2nd)
{
  TCoeff tmp[32][32]; 

  partialButterfly32(block,&tmp[0][0], 32, shift_1st);
  partialButterfly32(&tmp[0][0],coeff, 32, shift_2nd);
}

/** 32x32 inverse transform implemented using partial butterfly structure (1D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterflyInverse32(TCoeff* tmp,TCoeff* block, int iNumRows, int shift, int bit_depth)
{
  int j,k;  
  int E[16],O[16];
  int EE[8],EO[8];
  int EEE[4],EEO[4];
  int EEEE[2],EEEO[2];
#if TANSFOR_BUG_YQH
  int add = shift == 0 ? 0 : 1<<(shift-1);
  if(shift<0)
	  printf("error transform shift <0");
#else
  int add = 1<<(shift-1);
#endif
  int min_val, max_val;

  max_val = (1 << (bit_depth - 1)) - 1;
  min_val = - max_val - 1;

  for (j=0; j<iNumRows; j++)
  {    
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
    for (k=0;k<16;k++)
    {
      O[k] = g_aiT32[ 1][k]*tmp[ 1*iNumRows+j] + g_aiT32[ 3][k]*tmp[ 3*iNumRows+j] + g_aiT32[ 5][k]*tmp[ 5*iNumRows+j] + g_aiT32[ 7][k]*tmp[ 7*iNumRows+j] + 
             g_aiT32[ 9][k]*tmp[ 9*iNumRows+j] + g_aiT32[11][k]*tmp[11*iNumRows+j] + g_aiT32[13][k]*tmp[13*iNumRows+j] + g_aiT32[15][k]*tmp[15*iNumRows+j] + 
             g_aiT32[17][k]*tmp[17*iNumRows+j] + g_aiT32[19][k]*tmp[19*iNumRows+j] + g_aiT32[21][k]*tmp[21*iNumRows+j] + g_aiT32[23][k]*tmp[23*iNumRows+j] + 
             g_aiT32[25][k]*tmp[25*iNumRows+j] + g_aiT32[27][k]*tmp[27*iNumRows+j] + g_aiT32[29][k]*tmp[29*iNumRows+j] + g_aiT32[31][k]*tmp[31*iNumRows+j];
    }
    for (k=0;k<8;k++)
    {
      EO[k] = g_aiT32[ 2][k]*tmp[ 2*iNumRows+j] + g_aiT32[ 6][k]*tmp[ 6*iNumRows+j] + g_aiT32[10][k]*tmp[10*iNumRows+j] + g_aiT32[14][k]*tmp[14*iNumRows+j] + 
              g_aiT32[18][k]*tmp[18*iNumRows+j] + g_aiT32[22][k]*tmp[22*iNumRows+j] + g_aiT32[26][k]*tmp[26*iNumRows+j] + g_aiT32[30][k]*tmp[30*iNumRows+j];
    }
    for (k=0;k<4;k++)
    {
      EEO[k] = g_aiT32[4][k]*tmp[4*iNumRows+j] + g_aiT32[12][k]*tmp[12*iNumRows+j] + g_aiT32[20][k]*tmp[20*iNumRows+j] + g_aiT32[28][k]*tmp[28*iNumRows+j];
    }
    EEEO[0] = g_aiT32[8][0]*tmp[8*iNumRows+j] + g_aiT32[24][0]*tmp[24*iNumRows+j];
    EEEO[1] = g_aiT32[8][1]*tmp[8*iNumRows+j] + g_aiT32[24][1]*tmp[24*iNumRows+j];
    EEEE[0] = g_aiT32[0][0]*tmp[0*iNumRows+j] + g_aiT32[16][0]*tmp[16*iNumRows+j];    
    EEEE[1] = g_aiT32[0][1]*tmp[0*iNumRows+j] + g_aiT32[16][1]*tmp[16*iNumRows+j];

    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
    EEE[0] = EEEE[0] + EEEO[0];
    EEE[3] = EEEE[0] - EEEO[0];
    EEE[1] = EEEE[1] + EEEO[1];
    EEE[2] = EEEE[1] - EEEO[1];    
    for (k=0;k<4;k++)
    {
      EE[k] = EEE[k] + EEO[k];
      EE[k+4] = EEE[3-k] - EEO[3-k];
    }    
    for (k=0;k<8;k++)
    {
      E[k] = EE[k] + EO[k];
      E[k+8] = EE[7-k] - EO[7-k];
    }    
    for (k=0;k<16;k++)
    {
      block[j*32+k] = Clip3(min_val, max_val, (E[k] + O[k] + add)>>shift);
      block[j*32+k+16] = Clip3(min_val, max_val, (E[15-k] - O[15-k] + add)>>shift);
    }        
  }
}

/** 32x32 inverse transform (2D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 */
void xITr32(TCoeff* coeff,TCoeff* block, int shift_1st, int shift_2nd, int clipValue_1st, int clipValue_2nd)
{
  TCoeff tmp[32][32];
  
  partialButterflyInverse32(coeff,&tmp[0][0],32,shift_1st, clipValue_1st);
  partialButterflyInverse32(&tmp[0][0],block,32,shift_2nd, clipValue_2nd);
}
#else
void partialButterfly32(TCoeff* block,TCoeff* coeff,int shift)
{
  int j,k;
  int E[16],O[16];
  int EE[8],EO[8];
  int EEE[4],EEO[4];
  int EEEE[2],EEEO[2];
  int add = 1<<(shift-1);

  for (j=0; j<32; j++)
  {    
    /* E and O*/
    for (k=0;k<16;k++)
    {
      E[k] = block[j*32+k] + block[j*32+31-k];
      O[k] = block[j*32+k] - block[j*32+31-k];
    } 
    /* EE and EO */
    for (k=0;k<8;k++)
    {
      EE[k] = E[k] + E[15-k];
      EO[k] = E[k] - E[15-k];
    }
    /* EEE and EEO */
    for (k=0;k<4;k++)
    {
      EEE[k] = EE[k] + EE[7-k];
      EEO[k] = EE[k] - EE[7-k];
    }
    /* EEEE and EEEO */
    EEEE[0] = EEE[0] + EEE[3];    
    EEEO[0] = EEE[0] - EEE[3];
    EEEE[1] = EEE[1] + EEE[2];
    EEEO[1] = EEE[1] - EEE[2];

    coeff[ 0*32+j] = (g_aiT32[ 0][0]*EEEE[0] + g_aiT32[ 0][1]*EEEE[1] + add)>>shift;
    coeff[16*32+j] = (g_aiT32[16][0]*EEEE[0] + g_aiT32[16][1]*EEEE[1] + add)>>shift;
    coeff[ 8*32+j] = (g_aiT32[ 8][0]*EEEO[0] + g_aiT32[ 8][1]*EEEO[1] + add)>>shift; 
    coeff[24*32+j] = (g_aiT32[24][0]*EEEO[0] + g_aiT32[24][1]*EEEO[1] + add)>>shift;
    for (k=4;k<32;k+=8)
    {
      coeff[k*32+j] = (g_aiT32[k][0]*EEO[0] + g_aiT32[k][1]*EEO[1] + g_aiT32[k][2]*EEO[2] + g_aiT32[k][3]*EEO[3] + add)>>shift;
    }       
    for (k=2;k<32;k+=4)
    {
      coeff[k*32+j] = (g_aiT32[k][0]*EO[0] + g_aiT32[k][1]*EO[1] + g_aiT32[k][2]*EO[2] + g_aiT32[k][3]*EO[3] + 
                     g_aiT32[k][4]*EO[4] + g_aiT32[k][5]*EO[5] + g_aiT32[k][6]*EO[6] + g_aiT32[k][7]*EO[7] + add)>>shift;
    }       
    for (k=1;k<32;k+=2)
    {
      coeff[k*32+j] = (g_aiT32[k][ 0]*O[ 0] + g_aiT32[k][ 1]*O[ 1] + g_aiT32[k][ 2]*O[ 2] + g_aiT32[k][ 3]*O[ 3] + 
                     g_aiT32[k][ 4]*O[ 4] + g_aiT32[k][ 5]*O[ 5] + g_aiT32[k][ 6]*O[ 6] + g_aiT32[k][ 7]*O[ 7] +
                     g_aiT32[k][ 8]*O[ 8] + g_aiT32[k][ 9]*O[ 9] + g_aiT32[k][10]*O[10] + g_aiT32[k][11]*O[11] + 
                     g_aiT32[k][12]*O[12] + g_aiT32[k][13]*O[13] + g_aiT32[k][14]*O[14] + g_aiT32[k][15]*O[15] + add)>>shift;
    }
  }
}

/** 32x32 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 */
void xTr32(TCoeff* block,TCoeff* coeff)
{
  int shift_1st = 4;    // log2(32) - 1
  int shift_2nd = 11;   // log2(32) + 6
  TCoeff tmp[32][32]; 

  partialButterfly32(block,&tmp[0][0],shift_1st);
  partialButterfly32(&tmp[0][0],coeff,shift_2nd);
}

/** 32x32 inverse transform implemented using partial butterfly structure (1D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterflyInverse32(TCoeff* tmp,TCoeff* block,int shift)
{
  int j,k;  
  int E[16],O[16];
  int EE[8],EO[8];
  int EEE[4],EEO[4];
  int EEEE[2],EEEO[2];
  int add = 1<<(shift-1);

  for (j=0; j<32; j++)
  {    
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
    for (k=0;k<16;k++)
    {
      O[k] = g_aiT32[ 1][k]*tmp[ 1*32+j] + g_aiT32[ 3][k]*tmp[ 3*32+j] + g_aiT32[ 5][k]*tmp[ 5*32+j] + g_aiT32[ 7][k]*tmp[ 7*32+j] + 
             g_aiT32[ 9][k]*tmp[ 9*32+j] + g_aiT32[11][k]*tmp[11*32+j] + g_aiT32[13][k]*tmp[13*32+j] + g_aiT32[15][k]*tmp[15*32+j] + 
             g_aiT32[17][k]*tmp[17*32+j] + g_aiT32[19][k]*tmp[19*32+j] + g_aiT32[21][k]*tmp[21*32+j] + g_aiT32[23][k]*tmp[23*32+j] + 
             g_aiT32[25][k]*tmp[25*32+j] + g_aiT32[27][k]*tmp[27*32+j] + g_aiT32[29][k]*tmp[29*32+j] + g_aiT32[31][k]*tmp[31*32+j];
    }
    for (k=0;k<8;k++)
    {
      EO[k] = g_aiT32[ 2][k]*tmp[ 2*32+j] + g_aiT32[ 6][k]*tmp[ 6*32+j] + g_aiT32[10][k]*tmp[10*32+j] + g_aiT32[14][k]*tmp[14*32+j] + 
              g_aiT32[18][k]*tmp[18*32+j] + g_aiT32[22][k]*tmp[22*32+j] + g_aiT32[26][k]*tmp[26*32+j] + g_aiT32[30][k]*tmp[30*32+j];
    }
    for (k=0;k<4;k++)
    {
      EEO[k] = g_aiT32[4][k]*tmp[4*32+j] + g_aiT32[12][k]*tmp[12*32+j] + g_aiT32[20][k]*tmp[20*32+j] + g_aiT32[28][k]*tmp[28*32+j];
    }
    EEEO[0] = g_aiT32[8][0]*tmp[8*32+j] + g_aiT32[24][0]*tmp[24*32+j];
    EEEO[1] = g_aiT32[8][1]*tmp[8*32+j] + g_aiT32[24][1]*tmp[24*32+j];
    EEEE[0] = g_aiT32[0][0]*tmp[0*32+j] + g_aiT32[16][0]*tmp[16*32+j];    
    EEEE[1] = g_aiT32[0][1]*tmp[0*32+j] + g_aiT32[16][1]*tmp[16*32+j];

    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
    EEE[0] = EEEE[0] + EEEO[0];
    EEE[3] = EEEE[0] - EEEO[0];
    EEE[1] = EEEE[1] + EEEO[1];
    EEE[2] = EEEE[1] - EEEO[1];    
    for (k=0;k<4;k++)
    {
      EE[k] = EEE[k] + EEO[k];
      EE[k+4] = EEE[3-k] - EEO[3-k];
    }    
    for (k=0;k<8;k++)
    {
      E[k] = EE[k] + EO[k];
      E[k+8] = EE[7-k] - EO[7-k];
    }    
    for (k=0;k<16;k++)
    {
      block[j*32+k] = (E[k] + O[k] + add)>>shift;
      block[j*32+k+16] = (E[15-k] - O[15-k] + add)>>shift;
    }        
  }
}

/** 32x32 inverse transform (2D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 */
void xITr32(TCoeff* coeff,TCoeff* block)
{
  int shift_1st = SHIFT_INV_1ST;
  int shift_2nd = SHIFT_INV_2ND;
  TCoeff tmp[32][32];
  
  partialButterflyInverse32(coeff,&tmp[0][0],shift_1st);
  partialButterflyInverse32(&tmp[0][0],block,shift_2nd);
}
#endif

#endif //MATRIX_MULT

#if ZHOULULU_QT
// Functions for Secondary Transforms
void xTr2nd_8_1d_Hor(TCoeff *psSrc, int iStride, int shift)
{
    TCoeff *p =NULL;

    int i, j , k, sum;
    int rnd_factor;
    int tmpSrc[SEC_TR_SIZE][SEC_TR_SIZE];

    if (shift >= 1) 
    {
        rnd_factor = 1 << (shift - 1);
    } else 
    {
        rnd_factor = 0;
    }

    p = psSrc;
    for (i = 0; i < SEC_TR_SIZE; i++) 
    {
        for (j = 0; j < SEC_TR_SIZE; j++) 
        {
            tmpSrc[i][j] = p[j];
        }
        p += iStride;
    }

    for (i = 0; i < SEC_TR_SIZE; i++) 
    {
        for (j = 0; j < SEC_TR_SIZE; j++) 
        {
            sum = rnd_factor;
            for (k = 0; k < SEC_TR_SIZE; k++) 
            {
                sum += g_as_SEC_TRANS[i][k] * tmpSrc[j][k];
            }
            if (shift >= 1) 
            {
                // todo need some optimization
                psSrc[j * iStride + i] = Clip3(-32768, 32767, sum >> shift);
            } 
            else 
            {
                psSrc[j * iStride + i] = Clip3(-32768, 32767, sum);
            }
        }
    }
}

void xTr2nd_8_1d_Vert(TCoeff *psSrc, int iStride, int shift)
{
    TCoeff *p =NULL;

    int i, j , k, sum;
    int rnd_factor;
    int tmpSrc[SEC_TR_SIZE][SEC_TR_SIZE];

    if (shift >= 1) 
    {
        rnd_factor = 1 << (shift - 1);
    } 
    else 
    {
        rnd_factor = 0;
    }

    p = psSrc;
    for (i = 0; i < SEC_TR_SIZE; i++) 
    {
        for (j = 0; j < SEC_TR_SIZE; j++) 
        {
            tmpSrc[i][j] = p[j];
        }
        p += iStride;
    }

    p = psSrc;
    for (i = 0; i < SEC_TR_SIZE; i++) 
    {
        for (j = 0; j < SEC_TR_SIZE; j++) 
        {
            sum = rnd_factor;
            for (k = 0; k < SEC_TR_SIZE; k++) 
            {
                sum += g_as_SEC_TRANS[i][k] * tmpSrc[k][j];
            }
            if (shift >= 1) 
            {
                p[j] = Clip3(-32768, 32767, sum >> shift);
            }
            else 
            {
                p[j] = Clip3(-32768, 32767, sum);
            }
        }
        p += iStride;
    }
}

void xTr2nd_8_1d_Inv_Vert(TCoeff *psSrc, int iStride, int shift)
{
    TCoeff *p =NULL;

    int i, j , k, sum;
    int rnd_factor;
    int tmpSrc[SEC_TR_SIZE][SEC_TR_SIZE];

    if (shift >= 1) 
    {
        rnd_factor = 1 << (shift - 1);
    } 
    else 
    {
        rnd_factor = 0;
    }

    p = psSrc;
    for (i = 0; i < SEC_TR_SIZE; i++) 
    {
        for (j = 0; j < SEC_TR_SIZE; j++) 
        {
            tmpSrc[i][j] = p[j];
        }
        p += iStride;
    }

    p = psSrc;
    for (i = 0; i < SEC_TR_SIZE; i++) 
    {
        for (j = 0; j < SEC_TR_SIZE; j++) 
        {
            sum = rnd_factor;
            for (k = 0; k < SEC_TR_SIZE; k++) 
            {
                sum += g_as_SEC_TRANS[k][i] * tmpSrc[k][j];
            }
            if (shift >= 1) 
            {
                psSrc[j] = Clip3(-32768, 32767, sum >> shift);
            } 
            else 
            {
                psSrc[j] = Clip3(-32768, 32767, sum);
            }
        }
        p += iStride;
    }
}

void xTr2nd_8_1d_Inv_Hor(TCoeff *psSrc, int iSize, int shift)
{
    TCoeff *p =NULL;
    int i, j , k, sum;
    int rnd_factor;
    int tmpSrc[SEC_TR_SIZE][SEC_TR_SIZE];

    if (shift >= 1) 
    {
        rnd_factor = 1 << (shift - 1);
    } 
    else 
    {
        rnd_factor = 0;
    }

    p = psSrc;
    for (i = 0; i < SEC_TR_SIZE; i++) 
    {
        for (j = 0; j < SEC_TR_SIZE; j++) 
        {
            tmpSrc[i][j] = p[j];
        }
        p += iSize;
    }

    for (i = 0; i < SEC_TR_SIZE; i++) 
    {
        for (j = 0; j < SEC_TR_SIZE; j++) 
        {
            sum = rnd_factor;
            for (k = 0; k < SEC_TR_SIZE; k++) 
            {
                sum += g_as_SEC_TRANS[k][i] * tmpSrc[j][k];
            }
            if (shift >= 1) 
            {
                psSrc[j * iSize + i] = Clip3(-32768, 32767, sum >> shift);
            } 
            else 
            {
                psSrc[j * iSize + i] = Clip3(-32768, 32767, sum);
            }
        }
    }
}

void xCTr_4_1d_Hor(TCoeff* pSrc,TCoeff* pDst, int shift)
{
    int i, j , k, sum;
    int rnd_factor = shift == 0 ? 0 : 1 << (shift - 1);

    for (i = 0; i < 4; i++) 
    {
        for (j = 0; j < 4; j++) 
        {
            sum = rnd_factor;
            for (k = 0; k < 4; k++) 
            {
                sum += g_as_C_TRANS[i][k] * pSrc[j * 4 + k];
            }
            pDst[j * 4 + i] = Clip3(-32768, 32767, sum >> shift);
        }
    }
}

void xCTr_4_1d_Vert(TCoeff* pSrc,TCoeff* pDst, int shift)
{
    TCoeff *p =NULL;
    int i, j , k, sum;
    int rnd_factor = shift == 0 ? 0 : 1 << (shift - 1);

    p = pDst;
    for (i = 0; i < 4; i++) 
    {
        for (j = 0; j < 4; j++) 
        {
            sum = rnd_factor;
            for (k = 0; k < 4; k++) 
            {
                sum += g_as_C_TRANS[i][k] * pSrc[k * 4 +j];
            }
            p[j] = Clip3(-32768, 32767, sum >> shift);
        }
        p += 4;
    }
}

void xCTr4(TCoeff* block,TCoeff* coeff, int shift_1st, int shift_2nd)
{
    TCoeff tmp[4][4]; 

    xCTr_4_1d_Hor(block, &tmp[0][0], shift_1st);
    xCTr_4_1d_Vert(&tmp[0][0], coeff, shift_2nd);

}

void xCTr_4_1d_Inv_Vert(TCoeff *psSrc, TCoeff* pDst, int shift)
{
    TCoeff *p =NULL;
    int i, j , k, sum;
    int rnd_factor = shift == 0 ? 0 : 1 << (shift - 1);

    p = pDst;
    for (i = 0; i < 4; i++) 
    {
        for (j = 0; j < 4; j++) 
        {
            sum = rnd_factor;
            for (k = 0; k < 4; k++) 
            {
                sum += g_as_C_TRANS[k][i] * psSrc[k * 4 + j];
            }
            p[j] = Clip3(-32768, 32767, sum >> shift);
        }
        p += 4;
    }
}

void xCTr_4_1d_Inv_Hor(TCoeff *psSrc, TCoeff* pDst, int shift)
{
    TCoeff *p =NULL;
    int i, j , k, sum;
    int rnd_factor = shift == 0 ? 0 : 1 << (shift - 1);

    for (i = 0; i < 4; i++) 
    {
        for (j = 0; j < 4; j++) 
        {
            sum = rnd_factor;
            for (k = 0; k < 4; k++) 
            {
                sum += g_as_C_TRANS[k][i] * psSrc[j * 4 + k];
            }
            //psSrc[j][i] = Clip3(-32768,32767,sum >> shift);
            pDst[j * 4 + i] = sum >> shift;
        }
    }
}
void xCITr4(TCoeff* coeff, TCoeff* block, int shift_1st, int shift_2nd)
{
    TCoeff tmp[4][4]; 

    xCTr_4_1d_Inv_Hor(coeff, &tmp[0][0], shift_1st);
    xCTr_4_1d_Inv_Vert(&tmp[0][0], block, shift_2nd);

}

#endif

Void TComTrQuant::init( UInt uiMaxPhTrSize, Bool bUseRDOQ,  Bool bEnc )
{
  m_uiMaxPhTrSize   = uiMaxPhTrSize;

  m_bEnc         = bEnc;
  m_bUseRDOQ     = bUseRDOQ;
}
#if ZHOULULU_QT
#if wlq_WQ
Void TComTrQuant::quant_B8(TComDataCU* pcCU, UShort usQ, Int mode, TCoeff* piCoef, TCoeff* piQCoef, Int iWidth, Int iHeight, UInt& uiAcSum, UInt uiLog2Size, Int sample_bit_depth)
#else
void quant_B8(UShort usQ, Int mode, TCoeff* piCoef, TCoeff* piQCoef, Int iWidth, Int iHeight, UInt& uiAcSum, UInt uiLog2Size, Int sample_bit_depth)
#endif
{
    int i, j;
    long iLevel, iSign;
    int qp_const;
    int iQBits;
    TCoeff* pSrc = NULL;
    TCoeff* pDst = NULL;

    int intra = (mode > 3) ? 1 : 0;    
#if wlq_WQ
	Int WQMSize;
	Int *levelscale;
	Int levelscale_coef;
	Bool WeightQuantEnableFlag = getWeightQuantEnableFlag();

	if (WeightQuantEnableFlag)
	{
		WQMSize = 1 << uiLog2Size;
		levelscale = getQuantCoeff(uiLog2Size - 2);
	}
#endif
    if( iWidth > MAX_PHSIZE || iHeight > MAX_PHSIZE )
    {
        iWidth  = iWidth >> 1;
        iHeight = iHeight >> 1;
    }

    Int iStride = iWidth;

    iQBits = 15 + LIMIT_BIT - (sample_bit_depth + 1) - uiLog2Size;
    qp_const = intra ? (1 << iQBits) * 10 / 31 : (1 << iQBits) * 10 / 62;
    
    pSrc = piCoef;
    pDst = piQCoef;
    for (i = 0; i < iHeight; i++) 
    {
        for (j = 0; j < iWidth; j++) 
        {
            iLevel  =  (long)pSrc[j];
            iSign   = (iLevel < 0 ? -1: 1);
#if wlq_WQ
#if wlq_WQ_q
			if (WeightQuantEnableFlag)
			{
				levelscale_coef = levelscale[ (i & (WQMSize - 1)) *iWidth + (j & (WQMSize - 1)) ];
				iLevel = (((abs(iLevel) * levelscale_coef + (1 << 18)) >> 19) * usQ + qp_const) >> iQBits;
			}
			else
				iLevel = (abs(iLevel) * usQ + qp_const) >> iQBits;
#endif
#else
            iLevel = (abs(iLevel) * usQ + qp_const) >> iQBits;
#endif

#define QuantClip 1
#if QuantClip
            pDst[j] = (TCoeff)(Clip3(-32768, 32767, (iLevel * iSign)));
#else
            pDst[j] = iLevel * iSign;
#endif
            uiAcSum += iLevel;

        }
        pSrc += iStride;
        pDst += iStride;
    }
}
Void TComTrQuant::xQuant( TComDataCU* pcCU, TCoeff* piCoef, TCoeff*& piQCoef, UInt uiLog2Size, Int iWidth, Int iHeight, UInt& uiAcSum, TextType eTType, UInt uiAbsPartIdx )
{
    PartSize partSize = pcCU->getPartitionSize(uiAbsPartIdx);

    TCoeff* pSrc = piCoef;
    TCoeff* pDes = piQCoef;
    Int   iAdd = 0;
    
    if ( 0/*m_bUseRDOQ*/ )
    {
        xRateDistOptQuant( pcCU, piCoef, pDes, uiLog2Size, uiAcSum, eTType, uiAbsPartIdx );
    }
    else
    {       
        UInt uiQ = m_pQP->getQuantValue();
        UInt uiBitDepth = g_uiBitDepth;

        // todo check mode
#if INTERTEST
        UInt mode;    //跟RD保持一致  ZP
        if (pcCU->isIntra(uiAbsPartIdx))
        {
          mode = 4;
        }
        else
        {
          mode = 0;
        }
#else
        UInt mode = 4;
#endif
#if wlq_WQ
		quant_B8(pcCU, uiQ, mode, pSrc, pDes, iWidth, iHeight, uiAcSum, uiLog2Size, uiBitDepth);
#else
        quant_B8(uiQ, mode, pSrc, pDes, iWidth, iHeight, uiAcSum, uiLog2Size, uiBitDepth);
#endif

    } //if RDOQ
}
#if wlq_WQ   
Void TComTrQuant::xDeQuant( TComDataCU* pcCU, TCoeff* pSrc, TCoeff*& pDes, Int iLog2Size)
#else
Void TComTrQuant::xDeQuant( TCoeff* pSrc, TCoeff*& pDes, Int iLog2Size)
#endif
{    
    Int iWidth;
    Int iHeight;
    Int iShift,iAdd,iCoeffQ;
    UInt uiQ;
    UInt uiBitDepth = g_uiBitDepth;
    UInt coef_num = 0;
#if wlq_WQ
	UChar WQMSizeId;
	Int WQMSize;
	Int iStride;
	Int *cur_wq_matrix;
	Int yy, xx;
	Int wqm_coef;
#endif

    TCoeff* piQCoef = pSrc;
    TCoeff* piCoef  = pDes;

    if( m_iWidth > MAX_PHSIZE || m_iHeight > MAX_PHSIZE )
    {
        iWidth  = m_iWidth >> 1;
        iHeight = m_iHeight >> 1;
    }
    else
    {
        iWidth  = m_iWidth;
        iHeight = m_iHeight;
    }
   
    uiQ = m_pQP->getDequantValue();
    iShift = m_pQP->getDeQuantShift() + (uiBitDepth + 1) + iLog2Size - LIMIT_BIT;
    iAdd = iShift == 0 ? 0 : 1 << (iShift-1);
    coef_num  = iWidth * iHeight;
#if wlq_WQ
	Bool WeightQuantEnableFlag = getWeightQuantEnableFlag();
	Int wqm_shift = 2;
	if (WeightQuantEnableFlag)
	{
		WQMSizeId = iLog2Size - 2;
		WQMSize = 1 << (WQMSizeId + 2);
		iStride = 1 << (WQMSizeId + 2);
		cur_wq_matrix = getDequantCoeff(WQMSizeId);
	}
#endif
    for( Int n = 0; n < coef_num; n++ )
	{      
#if wlq_WQ
		if (WeightQuantEnableFlag)
		{
			yy = n / iWidth;
			xx = n - yy*iWidth;
			if ((WQMSizeId == 0) || (WQMSizeId == 1))
			{
				iStride = WQMSize;
				wqm_coef = cur_wq_matrix[(yy & (iStride - 1)) * iStride + (xx & (iStride - 1))];
			}
			else if (WQMSizeId == 2)
			{
				iStride = WQMSize >> 1;
				wqm_coef = cur_wq_matrix[((yy >> 1) & (iStride - 1)) * iStride + ((xx >> 1) & (iStride - 1))];
			}
			else if (WQMSizeId == 3)
			{
				iStride = WQMSize >> 2;
				wqm_coef = cur_wq_matrix[((yy >> 2) & (iStride - 1)) * iStride + ((xx >> 2) & (iStride - 1))];
			}

			iCoeffQ = ((((((long long int)piQCoef[n] * wqm_coef) >> wqm_shift) * (Int)uiQ) >> 4) + (long long int)iAdd) >>
			iShift; // dequantization, M2239, N1466
		}
		else
		{
			iCoeffQ = (piQCoef[n] * (Int)uiQ + iAdd) >> iShift;
		}
#else
        iCoeffQ = ( piQCoef[n]*(Int)uiQ + iAdd) >> iShift;
#endif
        piCoef[n] = Clip3(-32768,32767,iCoeffQ); 
    }
}

#else
Void TComTrQuant::xQuant( TComDataCU* pcCU, TCoeff* pSrc, TCoeff*& pDes, Int iLog2Size, UInt& uiAcSum, TextType eTType, UInt uiAbsPartIdx )
{
  UInt uiSize = 1<<iLog2Size;
  if( uiSize > m_uiMaxPhTrSize )
  {
    uiSize    = m_uiMaxPhTrSize;
    iLog2Size = g_aucConvertToBit[uiSize];
  }

  TCoeff* piCoef    = pSrc;
  TCoeff* piQCoef   = pDes;
  Int   iAdd = 0;
  
  if ( m_bUseRDOQ && (eTType == TEXT_LUMA || RDOQ_CHROMA) )
  {
    xRateDistOptQuant( pcCU, piCoef, pDes, iLog2Size, uiAcSum, eTType, uiAbsPartIdx );
  }
  else
  {
    const UInt*  pucScan;
    pucScan        = g_auiFrameScanXY [ iLog2Size ];

    UInt uiQ = m_pQP->getQuantValue();
    UInt uiBitDepth = g_uiBitDepth;

    UInt iTransformShift = MAX_TR_DYNAMIC_RANGE - uiBitDepth - iLog2Size;  // Represents scaling through forward transform
    Int iQBits = QUANT_SHIFT + iTransformShift;                // Right shift of non-RDOQ quantizer;  level = (coeff*uiQ + offset)>>q_bits
    iAdd = (pcCU->getPicture()->getPictureType()==I_PICTURE ? 171 : 85) << (iQBits-9);

    for( Int n = 0; n < 1<<(iLog2Size<<1); n++ )
    {
      Long iLevel;
      Int  iSign;
      UInt uiBlockPos = pucScan[n]; 
      iLevel  = (Long) piCoef[uiBlockPos];
      iSign   = (iLevel < 0 ? -1: 1);      

      iLevel = (abs(iLevel) * uiQ + iAdd ) >> iQBits;

      uiAcSum += iLevel;
      iLevel *= iSign;        
      piQCoef[uiBlockPos] = (TCoeff)iLevel;
    } // for n
  } //if RDOQ
}

Void TComTrQuant::xDeQuant( TCoeff* pSrc, TCoeff*& pDes, Int iLog2Size )
{
    UInt uiSize = 1<<iLog2Size;
    if( uiSize > m_uiMaxPhTrSize )
    {
        uiSize    = m_uiMaxPhTrSize;
        iLog2Size = g_aucConvertToBit[uiSize];
    }

    TCoeff* piQCoef = pSrc;
    TCoeff* piCoef  = pDes;

    Int iShift,iAdd,iCoeffQ;
    UInt uiQ;
    UInt uiBitDepth = g_uiBitDepth;

    UInt iTransformShift = MAX_TR_DYNAMIC_RANGE - uiBitDepth - iLog2Size; 
    iShift = QUANT_IQUANT_SHIFT - QUANT_SHIFT - iTransformShift;
    iAdd = iShift == 0 ? 0 : 1 << (iShift-1);

    uiQ = m_pQP->getDequantValue();

    for( Int n = 0; n < 1<<(iLog2Size<<1); n++ )
    {
        iCoeffQ = ( piQCoef[n]*(Int)uiQ +iAdd ) >> iShift;
        piCoef[n] = Clip3(-32768,32767,iCoeffQ);
    }
}
#endif

#if RD_NSQT
Void TComTrQuant::setBlockSize(TComDataCU* pcCU, UInt uiLog2Size, TextType eTType, UInt uiAbsPartIdx, UInt uiTrMode)
{
	// todo get parameter, check what is trans_size
	Bool  bUseNSQT = pcCU->getPicture()->getSPS()->getNonsquareQuadtreeTansformEnableFlag();
#if ZHANGYI_INTRA_SDIP
	Bool   bUseSDIP = pcCU->getPicture()->getSPS()->getNonsquareIntraPredictionEnableFlag();//zhangyiModify: 0618
	//bUseSDIP = pcCU->getSDIPFlag(0);  //zhangyiModify: 0618
#else
	Bool  bUseSDIP = false;
#endif

	PartSize partSize = pcCU->getPartitionSize(uiAbsPartIdx);

	assert((pcCU->getPicture()->getSPS()->getLog2MaxCUSize() >= uiLog2Size));

	// derive iWidth, iHeight
	if (uiTrMode == 1)
	{
		if (bUseNSQT && pcCU->getLog2CUSize(uiAbsPartIdx) > 3 && eTType == TEXT_LUMA && ((partSize == SIZE_2NxN ||
			partSize == SIZE_2NxnU || partSize == SIZE_2NxnD) /*&&  b8 < 4 &&*/ /*currMB->trans_size == 1*/))
		{
			m_iWidth = (1 << (uiLog2Size + 1));
			m_iHeight = (1 << (uiLog2Size - 1));
		}
		else if (bUseNSQT && pcCU->getLog2CUSize(uiAbsPartIdx) > 3 && eTType == TEXT_LUMA && ((partSize == SIZE_Nx2N ||
			partSize == SIZE_nLx2N || partSize == SIZE_nRx2N)  /*&& b8 < 4 &&*/ /*currMB->trans_size == 1*/))
		{
			m_iWidth = (1 << (uiLog2Size - 1));
			m_iHeight = (1 << (uiLog2Size + 1));
		}
		else if (bUseSDIP && eTType == TEXT_LUMA  && partSize == SIZE_2NxhN /*&& currMB->trans_size == 1*/ /*&&  b8 < 4*/)
		{
			m_iWidth = (1 << (uiLog2Size + 1));
			m_iHeight = (1 << (uiLog2Size - 1));
		}
		else if (bUseSDIP && eTType == TEXT_LUMA  && partSize == SIZE_hNx2N /*&& currMB->trans_size == 1*/ /*&&  b8 < 4*/)
		{
			m_iWidth = (1 << (uiLog2Size - 1));
			m_iHeight = (1 << (uiLog2Size + 1));
		}
		else
		{
			m_iWidth = (1 << uiLog2Size);
			m_iHeight = (1 << uiLog2Size);
		}
	}
	else
	{
		m_iWidth = (1 << uiLog2Size);
		m_iHeight = (1 << uiLog2Size);
	}
}
#endif


#if ZHOULULU_QT

Void TComTrQuant::setBlockSize (TComDataCU* pcCU, UInt uiLog2Size, TextType eTType, UInt uiAbsPartIdx)
{
    // todo get parameter, check what is trans_size
#if ZHANGYI_INTRA_SDIP
	Bool  bUseNSQT = pcCU->getPicture()->getSPS()->getNonsquareQuadtreeTansformEnableFlag();
#else
	Bool  bUseNSQT = false;
#endif
#if ZHANGYI_INTRA_SDIP
	Bool   bUseSDIP = pcCU->getPicture()->getSPS()->getNonsquareIntraPredictionEnableFlag();//zhangyiModify: 0618
	//bUseSDIP = pcCU->getSDIPFlag(0);  //zhangyiModify: 0618
#else
	Bool  bUseSDIP = false;
#endif

    PartSize partSize = pcCU->getPartitionSize(uiAbsPartIdx);

    assert( (pcCU->getPicture()->getSPS()->getLog2MaxCUSize() >= uiLog2Size) );

    // derive iWidth, iHeight
    if (bUseNSQT && pcCU->getLog2CUSize(uiAbsPartIdx) > 3  && eTType == TEXT_LUMA && ((partSize == SIZE_2NxN ||
        partSize == SIZE_2NxnU || partSize == SIZE_2NxnD) /*&&  b8 < 4 &&*/ /*currMB->trans_size == 1*/)) 
    {
        m_iWidth       = (1 << (uiLog2Size + 1));
        m_iHeight      = (1 << (uiLog2Size - 1));
    } 
    else if (bUseNSQT && pcCU->getLog2CUSize(uiAbsPartIdx) > 3 && eTType == TEXT_LUMA && ((partSize == SIZE_Nx2N ||
        partSize == SIZE_nLx2N || partSize == SIZE_nRx2N)  /*&& b8 < 4 &&*/ /*currMB->trans_size == 1*/)) 
    {
        m_iWidth       = (1 << (uiLog2Size - 1));
        m_iHeight      = (1 << (uiLog2Size + 1));
    } 
    else if (bUseSDIP && eTType == TEXT_LUMA  && partSize == SIZE_2NxhN /*&& currMB->trans_size == 1*/ /*&&  b8 < 4*/) 
    {
        m_iWidth       = (1 << (uiLog2Size + 1));
        m_iHeight      = (1 << (uiLog2Size - 1));
    } 
    else if (bUseSDIP && eTType == TEXT_LUMA  && partSize == SIZE_hNx2N /*&& currMB->trans_size == 1*/ /*&&  b8 < 4*/) 
    {
        m_iWidth       = (1 << (uiLog2Size - 1));
        m_iHeight      = (1 << (uiLog2Size + 1));
    } 
    else 
    {
        m_iWidth       = (1 << uiLog2Size);
        m_iHeight      = (1 << uiLog2Size);
    }
}

Void TComTrQuant::transformMxN( TComDataCU* pcCU, Pel* pcResidual, UInt uiStride, TCoeff*& rpcCoeff, 
                                UInt uiLog2Size, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx )
{
  uiAbsSum = 0;
  UInt uiLog2SizeX = 0;
  UInt uiLog2SizeY = 0;
  
  assert( (pcCU->getPicture()->getSPS()->getLog2MaxCUSize() >= uiLog2Size) ); 

  // 如果iWidth或iHeight超过64，则在Tansform函数里会进行降采样，输出的m_piTempCoeff的为降采样后的系数矩阵，stride为iWidth >> 1
  if (m_iWidth == m_iHeight)
  {
      xTansformNxN( pcCU, pcResidual, uiStride, m_piTempCoeff, uiLog2Size, eTType, uiAbsPartIdx );
  }
  else
  {
      xTansformNSQT ( pcCU, pcResidual, uiStride, m_piTempCoeff, m_iWidth, m_iHeight, uiLog2Size, eTType, uiAbsPartIdx );
  }
#if wlq_avs2x_debug
  if (g_sign && eTType == TEXT_LUMA)
  {
	  printf("\n");
	  printf("T_coeff\n");
	  for (int iii = 0; iii < 1 << uiLog2Size; iii++)
	  {
		  for (int jjj = 0; jjj < 1 << uiLog2Size; jjj++)
		  {
			  printf("%d\t", m_piTempCoeff[iii*(1 << uiLog2Size) + jjj]);
		  }
		  printf("\n");
	  }
  }

#endif
  xQuant( pcCU, m_piTempCoeff, rpcCoeff, uiLog2Size, m_iWidth, m_iHeight, uiAbsSum, eTType, uiAbsPartIdx );

}
#else
Void TComTrQuant::transformNxN( TComDataCU* pcCU, Pel* pcResidual, UInt uiStride, TCoeff*& rpcCoeff, UInt uiLog2Size, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx )
{
    uiAbsSum = 0;

    assert( (pcCU->getPicture()->getSPS()->getLog2MaxCUSize() >= uiLog2Size) );

    xT( pcResidual, uiStride, m_piTempCoeff, 1<<uiLog2Size );
    xQuant( pcCU, m_piTempCoeff, rpcCoeff, uiLog2Size, uiAbsSum, eTType, uiAbsPartIdx );
}
#endif

#if ZHOULULU_QT
Void TComTrQuant::invtransformMxN( TComDataCU* pcCU, Pel*& rpcResidual, UInt uiStride, TCoeff* pcCoeff, UInt uiLog2Size, TextType eTType, UInt uiAbsPartIdx )
{
#if wlq_avs2x_debug
	if (g_sign && eTType == TEXT_LUMA)
	{
		printf("\n");
		printf("TQ_QT\n");
		UInt W = 1 << uiLog2Size;	UInt H = 1 << uiLog2Size;
		if (g_sign == 2)
		{
			W = W << 1;  H = H >> 1;
		}
		for (int iii = 0; iii < H; iii++)
		{
			for (int jjj = 0; jjj < W; jjj++)
			{
				printf("%d ", pcCoeff[iii * W + jjj]);
			}
			printf("\n");
		}
	}
	//test
	g_sign = 0;
#endif
#if wlq_WQ
	xDeQuant( pcCU, pcCoeff, m_piTempCoeff, uiLog2Size);
#else
    xDeQuant( pcCoeff, m_piTempCoeff, uiLog2Size);
#endif
#if wlq_avs2x_debug
	if (g_sign && eTType == TEXT_LUMA)
	{
		printf("\n");
		printf("TQQ_T\n");
		for (int iii = 0; iii < (1 << uiLog2Size); iii++)
		{
			for (int jjj = 0; jjj < (1 << uiLog2Size); jjj++)
			{
				printf("%d\t", m_piTempCoeff[iii*(1 << uiLog2Size) + jjj]);
			}
			printf("\n");
		}
	}
#endif
    if (m_iWidth == m_iHeight)
    {
        xIT( pcCU, m_piTempCoeff, rpcResidual, uiStride, uiLog2Size, eTType, uiAbsPartIdx);
    }
    else
    {
        xINSQT ( pcCU, m_piTempCoeff, rpcResidual, uiStride, uiLog2Size, eTType, uiAbsPartIdx );
    }
#if wlq_avs2x_debug
	if (g_sign && eTType == TEXT_LUMA)
	{
		printf("\n");
		printf("TQQT_\n");
		for (int iii = 0; iii < (1 << uiLog2Size); iii++)
		{
			for (int jjj = 0; jjj < (1 << uiLog2Size); jjj++)
			{
				printf("%d\t", rpcResidual[iii * uiStride + jjj]);
			}
			printf("\n");
}
	}
#endif    
}
#else
Void TComTrQuant::invtransformNxN( Pel*& rpcResidual, UInt uiStride, TCoeff* pcCoeff, UInt uiLog2Size, TextType eTType )
{
  xDeQuant( pcCoeff, m_piTempCoeff, uiLog2Size);
  xIT( m_piTempCoeff, rpcResidual, uiStride, 1<<uiLog2Size );
}
#endif

Void TComTrQuant::invRecurTransformNxN( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eTxt, Pel*& rpcResidual, UInt uiAddr, UInt uiStride, UInt uiLog2Size, UInt uiMaxTrMode, UInt uiTrMode, TCoeff* rpcCoeff )
{   
  UInt uiWidth  = 1<<uiLog2Size;
  UInt uiHeight = 1<<uiLog2Size;
  if( !pcCU->getCbf(uiAbsPartIdx, eTxt, uiTrMode) )
      return;

#if RD_NSQT_BUG
#if UV_NO_SPLIT_TRANSFROM
  if ((uiTrMode == uiMaxTrMode) || (eTxt == TEXT_CHROMA_U) || (eTxt == TEXT_CHROMA_V))
#else
  if ( uiTrMode == uiMaxTrMode )
#endif
  {
	  if (!pcCU->getCbf(uiAbsPartIdx, eTxt, uiTrMode))
		  return;
#else
  if( !pcCU->getCbf(uiAbsPartIdx, eTxt, uiTrMode) )
	  return;

#if UV_NO_SPLIT_TRANSFROM
  if ((uiTrMode == uiMaxTrMode) || (eTxt == TEXT_CHROMA_U) || (eTxt == TEXT_CHROMA_V))
#else
  if ( uiTrMode == uiMaxTrMode )
#endif
  {
#endif
    Pel* pResi = rpcResidual + uiAddr;
#if ZHOULULU_QT
#if RD_NSQT
	setBlockSize(pcCU, uiLog2Size, eTxt, uiAbsPartIdx, uiTrMode);
#else
	setBlockSize( pcCU, uiLog2Size, eTxt, uiAbsPartIdx );
#endif
    invtransformMxN( pcCU, pResi, uiStride, rpcCoeff, uiLog2Size, eTxt, uiAbsPartIdx);
#else
    invtransformNxN( pResi, uiStride, rpcCoeff, uiLog2Size, eTxt );
#endif
  }
  else
  {
#if RD_NSQT
	  uiTrMode++;
	  uiWidth >>= 1;
	  uiHeight >>= 1;
	  UInt uiAddrOffset = uiHeight * uiStride;
	  UInt uiCoefOffset = uiWidth * uiHeight;
#if RD_NSQT_BUG_YQH
	  //test   pcCU->getTotalNumPart()
	  //UInt uiPartOffset = pcCU->getTotalNumPart() >> (uiTrMode << 1);
	  UInt uiPartOffset = pcCU->getPic()->getNumPartInCU() >> ((pcCU->getDepth(0) + uiTrMode) << 1);
#else
	  UInt uiPartOffset = pcCU->getPic()->getNumPartInCU() >> (uiTrMode << 1);
#endif
	  if (pcCU->getPicture()->getSPS()->getNonsquareQuadtreeTansformEnableFlag())
	  {
		  PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
		  switch (ePartSize)
		  {
		  case SIZE_2Nx2N:
		  case SIZE_NxN:
			  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
			  rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
			  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiWidth, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
			  rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
			  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
			  rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
			  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset + uiWidth, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
			 
			  break;
		  case SIZE_2NxN:
		  case SIZE_2NxnU:
		  case SIZE_2NxnD:
		  case SIZE_2NxhN:
#if RD_NSQT_BUG_YQH
			  if (uiLog2Size>3)
			  {
				  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
				  rpcCoeff += uiCoefOffset;
				  //  uiAbsPartIdx += uiPartOffset >> 1;
				  uiAbsPartIdx += uiPartOffset;
				  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + (uiAddrOffset >> 1), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
				  rpcCoeff += uiCoefOffset;
				// uiAbsPartIdx += uiPartOffset + (uiPartOffset >> 1);
				 uiAbsPartIdx += uiPartOffset;
				  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
				  rpcCoeff += uiCoefOffset;
				  // uiAbsPartIdx += uiPartOffset >> 1;
				  uiAbsPartIdx += uiPartOffset;
				  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset + (uiAddrOffset >> 1), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);

			  }
			  else
			  {
				  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
				  rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
				  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiWidth, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
				  rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
				  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
				  rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
				  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset + uiWidth, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);

			  }


#else

			  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
			  rpcCoeff += uiCoefOffset;
			  uiAbsPartIdx += uiPartOffset >> 1;
			  //uiAbsPartIdx += uiPartOffset;
			  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + (uiAddrOffset >> 1), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
			  rpcCoeff += uiCoefOffset;
			  uiAbsPartIdx += uiPartOffset + (uiPartOffset >> 1);
			  //uiAbsPartIdx += uiPartOffset;
			  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
			  rpcCoeff += uiCoefOffset;
			  uiAbsPartIdx += uiPartOffset >> 1;
			  //uiAbsPartIdx += uiPartOffset;
			  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset + (uiAddrOffset >> 1), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);

#endif	
			  break;
		  case SIZE_Nx2N:
		  case SIZE_nLx2N:
		  case SIZE_nRx2N:
		  case SIZE_hNx2N:
#if RD_NSQT_BUG_YQH
			  if (uiLog2Size>3)
			  {
				  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
				  //rpcCoeff += uiCoefOffset >> 2;
				  rpcCoeff += uiCoefOffset;
				 // uiAbsPartIdx += uiPartOffset >> 2;
				  uiAbsPartIdx += uiPartOffset;
				  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + (uiWidth >> 1), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
				  //rpcCoeff += (uiCoefOffset >> 2) + (uiCoefOffset >> 1);
				  rpcCoeff += uiCoefOffset;
				 // uiAbsPartIdx += (uiPartOffset >> 2) + (uiPartOffset >> 1);
				  uiAbsPartIdx += uiPartOffset;
				  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiWidth, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
				  //rpcCoeff += uiCoefOffset >> 2; 
				  rpcCoeff += uiCoefOffset;
				 // uiAbsPartIdx += uiPartOffset >> 2;
				  uiAbsPartIdx += uiPartOffset;
				  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiWidth + (uiWidth >> 1), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);

			  }
			  else
			  {
				  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
				  rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
				  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiWidth, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
				  rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
				  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
				  rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
				  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset + uiWidth, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);

			  }
#else
			  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
			  //rpcCoeff += uiCoefOffset >> 2;
			  rpcCoeff += uiCoefOffset;
			  uiAbsPartIdx += uiPartOffset >> 2;
			  //uiAbsPartIdx += uiPartOffset;
			  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + (uiWidth >> 1), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
			  //rpcCoeff += (uiCoefOffset >> 2) + (uiCoefOffset >> 1);
			  rpcCoeff += uiCoefOffset;
			  uiAbsPartIdx += (uiPartOffset >> 2) + (uiPartOffset >> 1);
			  //uiAbsPartIdx += uiPartOffset;
			  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiWidth, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
			  //rpcCoeff += uiCoefOffset >> 2; 
			  rpcCoeff += uiCoefOffset;
			  uiAbsPartIdx += uiPartOffset >> 2;
			  //uiAbsPartIdx += uiPartOffset;
			  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiWidth + (uiWidth >> 1), uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);

#endif	
			  break;

		  default:
			  break;
		  }
	  }
	  else
	  {
		  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
		  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiWidth, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
		  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
		  invRecurTransformNxN(pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset + uiWidth, uiStride, uiLog2Size - 1, uiMaxTrMode, uiTrMode, rpcCoeff);
	  }
#else
    uiTrMode++;
    uiWidth  >>= 1;
    uiHeight >>= 1;
    UInt uiAddrOffset = uiHeight * uiStride;
    UInt uiCoefOffset = uiWidth * uiHeight;
    UInt uiPartOffset = pcCU->getTotalNumPart() >> (uiTrMode<<1);
    invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr                         , uiStride, uiLog2Size-1, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
    invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiWidth               , uiStride, uiLog2Size-1, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
    invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset          , uiStride, uiLog2Size-1, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
    invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset + uiWidth, uiStride, uiLog2Size-1, uiMaxTrMode, uiTrMode, rpcCoeff );
#endif
  }
}

// ------------------------------------------------------------------------------------------------
// Logical transform
// ------------------------------------------------------------------------------------------------

// ====================================================================================================================
// Tables
// ====================================================================================================================
#define LOT_MAX_WLT_TAP							2						///< number of wavelet transform tap, (5-3)
static Pel iEBuff[ MAX_CU_SIZE + LOT_MAX_WLT_TAP*2 ];

// ====================================================================================================================
// Local functions
// ====================================================================================================================
void xWavelet53( Pel* pBuff, Int iSize, Int iLevel )
{
  if ( iLevel == 0 ) return;

  Pel* pExt = &iEBuff[ LOT_MAX_WLT_TAP ];
  Int  i, n, x, y;
  Int  iN, iN2;

  Int ySize, n2, y2, x2;
  //Pel temp[64 * 64];

  iN = iSize; 
  iN2 = iN>>1;
  for ( i=0; i<iLevel; i++ )
  {
    // step #1: horizontal transform
    for ( y=0, ySize=0; y<iN; y++, ySize+=iSize )
    {
      // copy
      ::memcpy( pExt, &pBuff[ ySize ], sizeof(Pel)*iN );

      // reflection
      pExt[ -1	 ] = pExt[ +1   ];
      pExt[ -2	 ] = pExt[ +2	];
      pExt[ iN   ] = pExt[ iN-2 ];
      pExt[ iN+1 ] = pExt[ iN-3 ];

      // filtering (H)
      for ( n=-1; n<iN; n+=2 ) 
      {
          pExt[ n ] -= (( pExt[ n-1 ] + pExt[ n+1 ] ) >> 1);
      }

      // filtering (L)
      for ( n =0; n<iN; n+=2 ) 
      {
          pExt[ n ] += (( pExt[ n-1 ] + pExt[ n+1 ] + 2 ) >> 2);
      }

      // copy
      for ( x=0; x<iN2; x++ )
      {
        x2 = x<<1;
        pBuff[ x+ySize ] = pExt[ x2 ];
      }
    }

    // step #2: vertical transform
    for ( x=0; x<iN; x++ )
    {
      // copy
      for ( y=0, ySize=0; y<iN; y++, ySize += iSize ) pExt[ y ] = pBuff[ x+ySize ];

      // reflection
      pExt[ -1		 ] = pExt[ +1   ];
      pExt[ -2		 ] = pExt[ +2	  ];
      pExt[ iN   ] = pExt[ iN-2 ];
      pExt[ iN+1 ] = pExt[ iN-3 ];

      // filtering (H)
      for ( n=-1; n<iN; n+=2 ) pExt[ n ] -= ( pExt[ n-1 ] + pExt[ n+1 ] ) >> 1;

      // filtering (L)
#if HIGH_ACC_LOT_SCALE
      for ( n =0; n<iN; n+=2 ) 
      {
          pExt[ n ]  = ( pExt[ n ] << 1 ) + ( ( pExt[ n-1 ] + pExt[ n+1 ] + 1 ) >> 1 );
      }
#else
      for ( n =0; n<iN; n+=2 ) 
      {
          pExt[ n ] += ( pExt[ n-1 ] + pExt[ n+1 ] + 2 ) >> 2;
      }
#endif

      // copy
      n2 = iN2*iSize;
      for ( y=0, ySize=0; y<iN2; y++, ySize+=iSize )
      {
        y2 = y<<1;
#if HIGH_ACC_LOT_SCALE
        pBuff[ x+ySize ] = pExt[ y2 ];
#else
        pBuff[ x+ySize ] = pExt[ y2 ] << 1; // scale by 2
#endif
      }
    }

    // shift size
    iN >>= 1;
    iN2 = iN >> 1;
  }
 /* int j = 0;
  for ( j = 0; j < MAX_PHSIZE; j++ )
  {
      ::memcpy( &pBuff[ j*iSize ], &temp[ j*iSize ], MAX_PHSIZE*sizeof(TCoeff) );
  }
*/
}

void xInvWavelet53( Pel* pBuff, Int iSize, Int iLevel )
{
  if ( iLevel == 0 ) return;

  Pel* pExt = &iEBuff[ LOT_MAX_WLT_TAP ];
  Int  i, n, x, y;
  Int  iN, iN2;
  Int  ySize, y2, iN3, x2;

  iN = iSize >> ( iLevel-1 ); iN2 = iN>>1; iN3 = iN2*iSize;
  for ( i=iLevel-1; i>=0; i-- )
  {
    // step #1: vertical transform
    for ( x=0; x<iN; x++ )
    {
      // copy
      for ( y=0, ySize=0; y<iN2; y++, ySize+=iSize )
      {
        y2 = y<<1;
        pExt[ y2 ] = pBuff[ x+ySize ];
      }

      // reflection
      pExt[ iN ] = pExt[ iN-2 ];

      // filtering (even pixel)
      for ( n=0; n<=iN;   n+=2 ) { pExt[ n ] >>= 1; }

      // filtering (odd pixel)
      for ( n=1; n<=iN-1; n+=2 ) { pExt[ n ] = ( pExt[ n-1 ] + pExt[ n+1 ] ) >> 1; }

      // copy
      for ( y=0, ySize=0; y<iN; y++, ySize+=iSize ) pBuff[ x+ySize ] = pExt[ y ];
    }

    // step #2: horizontal transform
    for ( y=0, ySize=0; y<iN; y++, ySize+=iSize )
    {
      // copy
      for ( x=0; x<iN2; x++ )
      {
        x2 = x<<1;
        pExt[ x2 ] = pBuff[ ySize + x ];
      }

      // reflection
      pExt[ iN ] = pExt[ iN-2 ];

      // filtering (odd pixel)
      for ( n=1; n<=iN-1; n+=2 ) { pExt[ n ] = ( pExt[ n-1 ] + pExt[ n+1 ] ) >> 1; }

      // copy
      ::memcpy( &pBuff[ ySize ], pExt, sizeof(Pel)*iN );
    }

    // shift size
    iN <<= 1;
    iN2 = iN >> 1;
    iN3 = iN2*iSize;
  }
}
/** Wrapper function between HM interface and core NxN forward transform (2D) 
 *  \param piBlkResi input data (residual)
 *  \param psCoeff output data (transform coefficients)
 *  \param uiStride stride of input residual data
 *  \param iSize transform size (iSize x iSize)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
#if ZHOULULU_QT
Void TComTrQuant::xTansformNxN( TComDataCU* pcCU, Pel* piBlkResi, UInt uiStride, TCoeff* psCoeff, Int iLog2Size, TextType eTType, UInt uiAbsPartIdx)
{
  Int    j;
  Int    iSize            = 1 << iLog2Size; 
  // todo get parameters below
  Int    bit_depth        = 8;
  Bool   bSecTransEnabled = 0;
  Int    shift_1st = (bit_depth + 1) + FACTO_BIT + iLog2Size - LIMIT_BIT;        // log2(size)
  Int    shift_2nd = FACTO_BIT + iLog2Size;                                      // log2(size) + 5

  // prepare source buffer
  for ( j=0; j<iSize; j++ )
  {
    ::memcpy( &m_aiBlock[ j*iSize ], piBlkResi+j*uiStride, iSize*sizeof(TCoeff) );
  }

  // check LOT
  if ( iLog2Size > MAX_LOG2PHSIZE ) // todo check m_uiMaxPhTrsize LOT最大变换尺寸能不能由用户配置
  {
    xWavelet53( m_aiBlock, iSize, iLog2Size - MAX_LOG2PHSIZE );
    
    for ( j = 1; j < MAX_PHSIZE; j++ )
    {
        ::memcpy( &m_aiBlock[ j*MAX_PHSIZE ], &m_aiBlock[ j*iSize ], MAX_PHSIZE * sizeof(TCoeff) );
    }
    
    shift_2nd -= 1;    // because after wavelet, the residue is 10-bit so shift_1st should add 1

    iLog2Size = iLog2Size - 1;
    iSize = iSize >> 1;

  }    

  if (iLog2Size == 2)
  {   
    if ( pcCU->isIntra(uiAbsPartIdx) && (eTType == TEXT_LUMA) &&  bSecTransEnabled)
    { 
        xCTr4(m_aiBlock, psCoeff, shift_1st + 1, shift_2nd + 1);
    }
    else
    {
        xTr4(m_aiBlock, psCoeff, shift_1st, shift_2nd);   
    }      
  }
  else if (iLog2Size == 3)
  {
    xTr8(m_aiBlock, psCoeff, shift_1st, shift_2nd);       
  }
  else if (iLog2Size == 4)
  {   
    xTr16(m_aiBlock, psCoeff, shift_1st, shift_2nd);       
  }
  else if (iLog2Size == 5)
  {   
    xTr32(m_aiBlock, psCoeff, shift_1st, shift_2nd);       
  }
  else 
  {
    assert(0);
  }
 
  // secondary transform for Intra luma 8x8+ 
  if ( pcCU->isIntra(uiAbsPartIdx) && (eTType == TEXT_LUMA) && bSecTransEnabled 
        && (iLog2Size >= SEC_TR_MIN_BITSIZE))   
  {
      //todo get Intra Pred Mode and block_available_up
      UInt uiIntraPredMode      = 23;
      UInt block_available_up   = 0;
      UInt block_available_left = 0;
      
      if (uiIntraPredMode >= 0 && uiIntraPredMode <= 23 && block_available_up) 
      {          
         xTr2nd_8_1d_Vert(psCoeff, iSize, 7);
      }

      if (((uiIntraPredMode >= 13 && uiIntraPredMode <= 32) || (uiIntraPredMode >= 0 && uiIntraPredMode <= 2)) 
          && block_available_left) 
      {
         xTr2nd_8_1d_Hor(psCoeff, iSize, 7);
      }

  }

//#endif  
}
#else
Void TComTrQuant::xT( Pel* piBlkResi, UInt uiStride, TCoeff* psCoeff, Int iSize )
{
    Int    j;

    // prepare source buffer
    for ( j=0; j<iSize; j++ )
    {
        ::memcpy( &m_aiBlock[ j*iSize ], piBlkResi+j*uiStride, iSize*sizeof(TCoeff) );
    }

    // check LOT
    if ( iSize > m_uiMaxPhTrSize )
    {
        Int iLog2Size   = g_aucConvertToBit[ iSize ];
        Int iLog2PhSize = g_aucConvertToBit[ m_uiMaxPhTrSize ];

        xWavelet53( m_aiBlock, iSize, iLog2Size-iLog2PhSize );

        for ( j=1; j<m_uiMaxPhTrSize; j++ )
        {
            ::memcpy( &m_aiBlock[ j*m_uiMaxPhTrSize ], &m_aiBlock[ j*iSize ], m_uiMaxPhTrSize*sizeof(TCoeff) );
        }

        iSize = m_uiMaxPhTrSize;
    }

#if MATRIX_MULT  
    xTr( m_aiBlock, psCoeff, iSize, (UInt)iSize );
#else
    if (iSize==2)
    {
        xTr2(m_aiBlock,psCoeff);     
    }
    else
        if (iSize==4)
        {   
            xTr4(m_aiBlock,psCoeff);     
        }
        else if (iSize==8)
        {
            xTr8(m_aiBlock,psCoeff);       
        }
        else if (iSize==16)
        {   
            xTr16(m_aiBlock,psCoeff);       
        }
        else if (iSize==32)
        {   
            xTr32(m_aiBlock,psCoeff);       
        }
        else 
        {
            assert(0);
        }
#endif  
}
#endif

#if ZHOULULU_QT

void  xWaveletNSQT(Pel* pBuff, int iSize, int iN)
{
    Pel* pExt = &iEBuff[ LOT_MAX_WLT_TAP ];
    int  i, n, x, y;
    int  iN2;
    int iLevel = 1;
    int ySize, y2, x2;

    /*
    for (i = 0; i < iN; i++) 
    {
    memcpy(&pBuff[i * iSize], &curr_blk[i * iSize], iSize * sizeof(int));
    }
    */
    for (i = 0; i < iLevel; i++) 
    {
        // step #1: horizontal transform
        iN2 = iSize >> 1;
        for (y = 0, ySize = 0; y < iN; y++, ySize += iSize) 
        {
            // copy
#if niu_NSQT_dec
			memcpy(pExt, &pBuff[ySize], sizeof(Pel)*iSize);
#else
			memcpy(pExt, &pBuff[ySize], sizeof(int)*iSize);
#endif

            // reflection
            pExt[ -1     ] = pExt[ +1   ];
            pExt[ -2     ] = pExt[ +2   ];
            pExt[ iSize   ] = pExt[ iSize - 2 ];
            pExt[ iSize + 1 ] = pExt[ iSize - 3 ];

            // filtering (H)
            for (n = -1; n < iSize; n += 2) 
            {
                pExt[ n ] -= (pExt[ n - 1 ] + pExt[ n + 1 ]) >> 1;
            }

            // filtering (L)
            for (n = 0; n < iSize; n += 2) 
            {
                pExt[ n ] += (pExt[ n - 1 ] + pExt[ n + 1 ] + 2) >> 2;
            }

            // copy
            for (x = 0; x < iN2; x++) 
            {
                x2 = x << 1;
                pBuff[ x + ySize ] = pExt[ x2 ];
            }
        }
        // step #2: vertical transform
        iN2 = iN >> 1;
        for (x = 0; x < iSize; x++) 
        {
            // copy
            for (y = 0, ySize = 0; y < iN; y++, ySize += iSize) 
            {
                pExt[ y ] = pBuff[ x + ySize ];
            }

            // reflection
            pExt[ -1     ] = pExt[ +1   ];
            pExt[ -2     ] = pExt[ +2   ];
            pExt[ iN   ] = pExt[ iN - 2 ];
            pExt[ iN + 1 ] = pExt[ iN - 3 ];

            // filtering (H)
            for (n = -1; n < iN; n += 2) 
            {
                pExt[ n ] -= (pExt[ n - 1 ] + pExt[ n + 1 ]) >> 1;
            }

            // filtering (L)

            for (n = 0; n < iN; n += 2) 
            {
                pExt[ n ]  = (pExt[ n ] << 1) + ((pExt[ n - 1 ] + pExt[ n + 1 ] + 1) >> 1);
            }

            // copy
            for (y = 0, ySize = 0; y < iN2; y++, ySize += iSize) 
            {
                y2 = y << 1;
                pBuff[ x + ySize ] = pExt[ y2 ];
            }
        }
    }
}


void  xInvWaveletNSQT(Pel* pBuff, int iSize, int iN)
{
    Pel *pExt = &iEBuff[ LOT_MAX_WLT_TAP ];
    int  i, n, x, y;
    int  iN2;
    int  ySize, y2, x2;
    int  iLevel = 1;
    //int *pBuff = malloc( 64 * 64 * sizeof( int ) );

    /*for (i = 0; i < iN; i++) {
        memcpy(&pBuff[i * iSize], &curr_blk[i][0], (iSize >> 1) * sizeof(int));
    }*/

    for (i = iLevel - 1; i >= 0; i--) 
    {
        // step #1: vertical transform
        iN2 = iN >> 1;
        for (x = 0; x < (iSize >> 1); x++) 
        {
            // copy
            for (y = 0, ySize = 0; y < iN2; y++, ySize += iSize) 
            {
                y2 = y << 1;
                pExt[ y2 ] = pBuff[ x + ySize ];
            }

            // reflection
            pExt[ iN ] = pExt[ iN - 2 ];

            // filtering (even pixel)
            for (n = 0; n <= iN;   n += 2) 
            {
                pExt[ n ] >>= 1;
            }

            // filtering (odd pixel)
            for (n = 1; n <= iN - 1; n += 2) 
            {
                pExt[ n ] = (pExt[ n - 1 ] + pExt[ n + 1 ]) >> 1;
            }

            // copy
            for (y = 0, ySize = 0; y < iN; y++, ySize += iSize) 
            {
                pBuff[ x + ySize ] = pExt[ y ];
            }
        }

        // step #2: horizontal transform
        iN2 = iSize >> 1;
        for (y = 0, ySize = 0; y < iN; y++, ySize += iSize) 
        {
            // copy
            for (x = 0; x < iN2; x++) 
            {
                x2 = x << 1;
                pExt[ x2 ] = pBuff[ ySize + x ];
            }

            // reflection
            pExt[ iSize ] = pExt[ iSize - 2 ];

            // filtering (odd pixel)
            for (n = 1; n <= iSize - 1; n += 2) 
            {
                pExt[ n ] = (pExt[ n - 1 ] + pExt[ n + 1 ]) >> 1;
            }

            // copy
            memcpy(&pBuff[ ySize ], pExt, sizeof(Pel)*iSize);
        }
    }
    /*for (i = 0; i < iN; i++) 
    {
        memcpy(&curr_blk[i][0], &pBuff[i * iSize], iSize * sizeof(int));
    }*/
    //free( pBuff );
}

Void TComTrQuant::xTansformNSQT ( TComDataCU* pcCU, Pel* piBlkResi, UInt uiStride, TCoeff* psCoeff, Int iWidth, Int iHeight, UInt iLog2Size, TextType eTType, UInt uiAbsPartIdx )
{
    int j = 0;
    int iVer   = 0;
    int iHor   = 0;
    // todo get parameters below
    Int  bit_depth        = 8;    
    Bool bSecTransEnabled = 0;    
    int shift_1st = (bit_depth + 1) + FACTO_BIT + iLog2Size - LIMIT_BIT;
    int shift_2nd = FACTO_BIT + iLog2Size;
    int iSize     = 1 << iLog2Size;;
    PartSize partSize = pcCU->getPartitionSize(uiAbsPartIdx);

    TCoeff tmp[MAX_CU_SIZE * MAX_CU_SIZE];  

    // prepare source buffer
    for ( j=0; j<iHeight; j++ )
    {
        ::memcpy( &m_aiBlock[ j*iWidth ], piBlkResi+j*uiStride, iWidth*sizeof(TCoeff) );
    }

    if (iLog2Size == 5) 
    {
        shift_1st += 1; // because after wavelet, the residue is 10-bit
    }

    if (iLog2Size == 3) 
    {
        if (iWidth == 16)     // hor
        {   
            partialButterfly16(m_aiBlock, tmp, 4, shift_1st + 1);
            partialButterfly4(tmp, psCoeff, 16, shift_2nd - 1);
        } 
        else if (iWidth == 4) // ver
        {
            partialButterfly4(m_aiBlock, tmp, 16, shift_1st - 1);
            partialButterfly16(tmp, psCoeff, 4, shift_2nd + 1);
        }
    }
    else if (iLog2Size == 4) 
    {
        if (iWidth == 32) 
        {
            partialButterfly32(m_aiBlock, tmp, 8, shift_1st + 1);
            partialButterfly8(tmp, psCoeff, 32, shift_2nd - 1);
        } 
        else if (iWidth == 8) 
        {
            partialButterfly8(m_aiBlock, tmp, 32, shift_1st - 1);
            partialButterfly32(tmp, psCoeff, 8, shift_2nd + 1);
        }
    } 
    else if (iLog2Size == 5) 
    {   
        xWaveletNSQT(m_aiBlock, iWidth, iHeight);

        for (j = 1; j < (iHeight >> 1); j++) 
        {
#if niu_NSQT_dec
			memcpy(&m_aiBlock[j * (iWidth >> 1)], &m_aiBlock[j * iWidth], (iWidth >> 1) * sizeof(Pel));
#else
			memcpy(&m_aiBlock[j * (iWidth >> 1)], &m_aiBlock[j * iWidth], (iWidth >> 1) * sizeof(int));
#endif
        }

        if (iWidth == 64) 
        {   
            partialButterfly32(m_aiBlock, tmp, 8, shift_1st);
            partialButterfly8(tmp, psCoeff, 32, shift_2nd - 2);            
        } 
        else if (iWidth == 16) 
        {
            partialButterfly8(m_aiBlock, tmp, 32, shift_1st - 2);
            partialButterfly32(tmp, psCoeff, 8, shift_2nd);
        }

        iWidth = iWidth >> 1;
        iHeight = iHeight >> 1;
    }    

    if (pcCU->isIntra(uiAbsPartIdx) && bSecTransEnabled && (eTType == TEXT_LUMA)) 
    {
        //todo get Intra Pred Mode and block_available_up
        UInt uiIntraPredMode      = pcCU->getIntraDir(uiAbsPartIdx);
        UInt block_available_up   = 0;
        UInt block_available_left = 0;

        if (uiIntraPredMode >= 0 && uiIntraPredMode <= 23 && block_available_up) 
        {          
            xTr2nd_8_1d_Vert(psCoeff, iWidth, 7);
        }

        if (((uiIntraPredMode >= 13 && uiIntraPredMode <= 32) || (uiIntraPredMode >= 0 && uiIntraPredMode <= 2)) 
            && block_available_left) 
        {
            xTr2nd_8_1d_Hor(psCoeff, iWidth, 7);
        }
    }

}
#endif

/** Wrapper function between HM interface and core NxN inverse transform (2D) 
 *  \param plCoef input data (transform coefficients)
 *  \param pResidual output data (residual)
 *  \param uiStride stride of input residual data
 *  \param iSize transform size (iSize x iSize)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
#if ZHOULULU_QT
Void TComTrQuant::xIT( TComDataCU* pcCU, TCoeff* plCoef, Pel* pResidual, UInt uiStride, Int iLog2Size, TextType eTType, UInt uiAbsPartIdx )
{
  Int    j;
  Int    iSize            = 1 << iLog2Size; 
  // todo get parameters below
  Int    bit_depth        = 8;
  Bool   bSecTransEnabled = 0;
  Int    shift_1st        = 5;      
  Int    shift_2nd        = 19 - g_uiBitDepth + 1;                     
  Int    clipValue_1st    = LIMIT_BIT;
  Int    clipValue_2nd    = g_uiBitDepth + 1;

  if (iLog2Size > 5)
  {
      shift_2nd -= 1;
      clipValue_2nd += 1;
  }
  
  // secondary transform for Intra luma 8x8+ 
  if ( pcCU->isIntra(uiAbsPartIdx) && (eTType == TEXT_LUMA) && bSecTransEnabled 
      && (iLog2Size >= SEC_TR_MIN_BITSIZE))   
  {
      //todo get Intra Pred Mode and block_available_up
      UInt uiIntraPredMode      = 23;
      UInt block_available_up   = 0;
      UInt block_available_left = 0;

      if (uiIntraPredMode >= 0 && uiIntraPredMode <= 23 && block_available_up) 
      {          
          xTr2nd_8_1d_Inv_Vert(plCoef, iSize, 7);
      }

      if (((uiIntraPredMode >= 13 && uiIntraPredMode <= 32) || (uiIntraPredMode >= 0 && uiIntraPredMode <= 2)) 
          && block_available_left) 
      {
          xTr2nd_8_1d_Inv_Hor(plCoef, iSize, 7);
      }

  }
  
  if (iLog2Size == 2)
  {   
    if ( pcCU->isIntra(uiAbsPartIdx) && (eTType == TEXT_LUMA) &&  bSecTransEnabled)
    { 
        xCITr4(m_aiBlock, plCoef, shift_1st + 1, shift_2nd + 1);
    }
    else
    {
        xITr4( plCoef, m_aiBlock, shift_1st, shift_2nd, clipValue_1st, clipValue_2nd);   
    }      
  }
  else if (iLog2Size == 3)
  {
    xITr8( plCoef, m_aiBlock, shift_1st, shift_2nd, clipValue_1st, clipValue_2nd);       
  }
  else if (iLog2Size == 4)
  {   
    xITr16( plCoef, m_aiBlock, shift_1st, shift_2nd, clipValue_1st, clipValue_2nd);       
  }
  else if (iLog2Size >= 5)
  {   
    xITr32( plCoef, m_aiBlock, shift_1st, shift_2nd, clipValue_1st, clipValue_2nd);       
  }
  else 
  {
    assert(0);
  }
 
  // check LOT
  if ( iLog2Size > MAX_LOG2PHSIZE ) // todo check m_uiMaxPhTrsize LOT最大变换尺寸能不能由用户配置
  {
      //Int iLog2Size   = g_aucConvertToBit[ iSize ];
      //Int iLog2PhSize = g_aucConvertToBit[ m_uiMaxPhTrSize ];
      
      ::memset( m_aiCoeff, 0, sizeof(TCoeff)*iSize*iSize );
      for ( j=0; j<MAX_PHSIZE; j++ )
      {
          ::memcpy( m_aiCoeff+j*iSize, m_aiBlock+j*MAX_PHSIZE, MAX_PHSIZE*sizeof(TCoeff) );
      }

      xInvWavelet53( m_aiCoeff, iSize, iLog2Size - MAX_LOG2PHSIZE);

      for ( j=0; j<iSize; j++ )
      {
          ::memcpy( pResidual+j*uiStride, m_aiCoeff+j*iSize, iSize*sizeof(TCoeff) );
      }

  }   
  else
  {
      for ( j=0; j<iSize; j++ )
      {
          ::memcpy( pResidual+j*uiStride, &m_aiBlock[ j*iSize ], iSize*sizeof(TCoeff) );
      }

  }

}

Void TComTrQuant::xINSQT ( TComDataCU* pcCU, TCoeff* plCoef, Pel* pResidual, UInt uiStride, Int iLog2Size, TextType eTType, UInt uiAbsPartIdx)
{
    int j = 0;
    int iVer   = 0;
    int iHor   = 0;
    // todo get parameters below
    Bool bSecTransEnabled = 0;    
    int shift_1st     = 5;
    int shift_2nd     = 19 - g_uiBitDepth + 1;
    Int clipValue_1st = LIMIT_BIT;
    Int clipValue_2nd = g_uiBitDepth + 1;;
    int iSize         = 1 << iLog2Size;;
    PartSize partSize = pcCU->getPartitionSize(uiAbsPartIdx);
	
    TCoeff tmp[MAX_CU_SIZE * MAX_CU_SIZE];      
    
    if (iLog2Size == 5) 
    {
        shift_2nd     = shift_2nd - 1;
        clipValue_2nd = clipValue_2nd + 1;
    }

    if (pcCU->isIntra(uiAbsPartIdx) && bSecTransEnabled && (eTType == TEXT_LUMA)) 
    {
        //todo get Intra Pred Mode and block_available_up
        UInt uiIntraPredMode      = pcCU->getIntraDir(uiAbsPartIdx);
        UInt block_available_up   = 0;
        UInt block_available_left = 0;

        if (uiIntraPredMode >= 0 && uiIntraPredMode <= 23 && block_available_up) 
        {          
            xTr2nd_8_1d_Inv_Vert(plCoef, m_iWidth, 7);
        }

        if (((uiIntraPredMode >= 13 && uiIntraPredMode <= 32) || (uiIntraPredMode >= 0 && uiIntraPredMode <= 2)) 
            && block_available_left) 
        {
            xTr2nd_8_1d_Inv_Hor(plCoef, m_iWidth, 7);
        }
    }    

    if (iLog2Size == 3) 
    {
        if (m_iWidth == 16)     // hor
        {   
            partialButterflyInverse4(plCoef, tmp, 16, shift_1st, clipValue_1st);
            partialButterflyInverse16(tmp, m_aiBlock, 4, shift_2nd, clipValue_2nd);            
        } 
        else if (m_iWidth == 4) // ver
        {
            partialButterflyInverse16(plCoef, tmp, 4, shift_1st, clipValue_1st);
            partialButterflyInverse4(tmp, m_aiBlock, 16, shift_2nd, clipValue_2nd);            
        }
    }
    else if (iLog2Size == 4) 
    {
        if (m_iWidth == 32) 
        {
            partialButterflyInverse8(plCoef, tmp, 32, shift_1st, clipValue_1st);
            partialButterflyInverse32(tmp, m_aiBlock, 8, shift_2nd, clipValue_2nd);
        } 
        else if (m_iWidth == 8) 
        {
            partialButterflyInverse32(plCoef, tmp, 8, shift_1st, clipValue_1st);
            partialButterflyInverse8(tmp, m_aiBlock, 32, shift_2nd, clipValue_2nd);
        }
    } 
    else if (iLog2Size == 5) 
    {   
        if (m_iWidth == 64) 
        {  
            partialButterflyInverse8(plCoef, tmp, 32, shift_1st, clipValue_1st);
            partialButterflyInverse32(tmp, m_aiBlock, 8, shift_2nd, clipValue_2nd);            
        } 
        else if (m_iWidth == 16) 
        {
            partialButterflyInverse32(plCoef, tmp, 8, shift_1st, clipValue_1st);
            partialButterflyInverse8(tmp, m_aiBlock, 32, shift_2nd, clipValue_2nd);
        }
    }   

    if (m_iWidth > MAX_PHSIZE || m_iHeight > MAX_PHSIZE)
    {
        ::memset( m_aiCoeff, 0, sizeof(TCoeff)*m_iWidth*m_iHeight );
        for ( j=0; j < (m_iHeight >> 1); j++ )
        {
            ::memcpy( m_aiCoeff+j*m_iWidth, m_aiBlock+j*(m_iWidth >> 1), (m_iWidth >> 1) * sizeof(TCoeff) );
        }

        xInvWaveletNSQT(m_aiCoeff, m_iWidth, m_iHeight);
#if !niu_NSQT_revise
		xWaveletNSQT(m_aiCoeff, 64, 16);
#endif
        
        for ( j=0; j<m_iHeight; j++ )
        {
            ::memcpy( pResidual+j*uiStride, m_aiCoeff+j*m_iWidth, m_iWidth*sizeof(TCoeff) );
        }
    }
    else
    {
        for ( j=0; j<m_iHeight; j++ )
        {
            ::memcpy( pResidual+j*uiStride, m_aiBlock+j*m_iWidth, m_iWidth*sizeof(TCoeff) );
        }
    }

}
#else
Void TComTrQuant::xIT( TCoeff* plCoef, Pel* pResidual, UInt uiStride, Int iSize )
{
  Int    j;
  Int    iOrgSize = iSize;

  // check LOT
  if ( iSize > m_uiMaxPhTrSize )
  {
    iSize = m_uiMaxPhTrSize;
  }

#if MATRIX_MULT
  xITr( plCoef, m_aiBlock, iSize, (UInt)iSize );
#else
  if (iSize==2)
  {
    xITr2(plCoef,m_aiBlock);
  }
  else
  if (iSize==4)
  {    
    xITr4(plCoef,m_aiBlock);       
  }
  else if (iSize==8)
  {
    xITr8(plCoef,m_aiBlock);       
  }
  else if (iSize==16)
  {
    xITr16(plCoef,m_aiBlock);       
  }
  else if (iSize==32)
  {
    xITr32(plCoef,m_aiBlock);       
  }
  else
  {
    assert(0);
  }
#endif

  // restore size
  iSize = iOrgSize;

  // check LOT
  if ( iSize > m_uiMaxPhTrSize )
  {
    Int iLog2Size   = g_aucConvertToBit[ iSize ];
    Int iLog2PhSize = g_aucConvertToBit[ m_uiMaxPhTrSize ];

    ::memset( m_aiCoeff, 0, sizeof(TCoeff)*iSize*iSize );
    for ( j=0; j<m_uiMaxPhTrSize; j++ )
    {
      ::memcpy( m_aiCoeff+j*iSize, m_aiBlock+j*m_uiMaxPhTrSize, m_uiMaxPhTrSize*sizeof(TCoeff) );
    }

    xInvWavelet53( m_aiCoeff, iSize, iLog2Size-iLog2PhSize );

    for ( j=0; j<iSize; j++ )
    {
      ::memcpy( pResidual+j*uiStride, m_aiCoeff+j*iSize, iSize*sizeof(TCoeff) );
    }
  }
  else
  {
    for (j=0; j<iSize; j++)
    {
      ::memcpy( pResidual+j*uiStride, m_aiBlock+j*iSize, iSize*sizeof(TCoeff) );
    }
  }
}
#endif

#if wlq_WQ
Void TComTrQuant::initScalingList()
{
	for (UInt sizeId = 0; sizeId < SCALING_LIST_SIZE_NUM; sizeId++)
	{
		m_quantCoef[sizeId]   = new Int[g_scalingListSize[sizeId]];
		m_dequantCoef[sizeId] = new Int[64];
	}
	for (UInt j = 0; j < 2; j++)
	{
		m_wq_matrix[0][j] = new Short[16];
		m_wq_matrix[1][j] = new Short[64];
	}
}

/** destroy quantization matrix array
*/
Void TComTrQuant::destroyScalingList()
{
	for (UInt sizeId = 0; sizeId < SCALING_LIST_SIZE_NUM; sizeId++)
	{
		if (m_quantCoef[sizeId])
		{
			delete[] m_quantCoef[sizeId];
		}
		if (m_dequantCoef[sizeId])
		{
			delete[] m_dequantCoef[sizeId];
		}
	}
	for (UInt j = 0; j < 2; j++)
	{
		if (m_wq_matrix[0][j])
			delete[] m_wq_matrix[0][j];
		if (m_wq_matrix[1][j])
			delete[] m_wq_matrix[1][j];
	}
}
#endif

// RDOQ
Void TComTrQuant::xRateDistOptQuant                 ( TComDataCU*                     pcCU,
                                                      TCoeff*                         plSrcCoeff,
                                                      TCoeff*&                        piDstCoeff,
                                                      UInt                            uiLog2Size,
                                                      UInt&                           uiAbsSum,
                                                      TextType                        eTType,
                                                      UInt                            uiAbsPartIdx )
{
  Double dTemp       = 0;

  UInt uiQ = m_pQP->getQuantValue();
  UInt uiBitDepth = g_uiBitDepth;

  Int iTransformShift = MAX_TR_DYNAMIC_RANGE - uiBitDepth - uiLog2Size;  // Represents scaling through forward transform
  double dErrScale = (double)(1<<SCALE_BITS);                              // Compensate for scaling of bitcount in Lagrange cost function
  dErrScale = dErrScale*pow(2.0,-2.0*iTransformShift);                     // Compensate for scaling through forward transform
  dErrScale = dErrScale/(double)(uiQ*uiQ);                                 // Compensate for qp-dependent multiplier applied before calculating the Lagrange cost function
  Int iQBits = QUANT_SHIFT + iTransformShift;
  Double     d64BlockUncodedCost = 0;
  const UInt uiLog2BlkSize       = uiLog2Size;
  const UInt uiMaxNumCoeff       = 1 << ( uiLog2BlkSize << 1 );
  
  Int  piCoeff      [ MAX_CU_SIZE * MAX_CU_SIZE ];
  Long plLevelDouble[ MAX_CU_SIZE * MAX_CU_SIZE ];
  
  ::memset( piDstCoeff,    0, sizeof(TCoeff) *   uiMaxNumCoeff        );
  ::memset( piCoeff,       0, sizeof(Int)    *   uiMaxNumCoeff        );
  ::memset( plLevelDouble, 0, sizeof(Long)   *   uiMaxNumCoeff        );

  // Initialization of the scanning pattern
  const UInt* pucScan  = g_auiFrameScanXY[uiLog2BlkSize];

  eTType = eTType == TEXT_LUMA ? TEXT_LUMA : ( eTType == TEXT_CHROMA_U ? TEXT_CHROMA : TEXT_CHROMA_U );

  UInt uiLastScanPosP1 = 1;

  //===== quantization =====
  for( UInt uiScanPos = 0; uiScanPos < uiMaxNumCoeff; uiScanPos++ )
  {
    UInt    uiBlkPos = pucScan[ uiScanPos ];
    Long    lLevelDouble = plSrcCoeff[ uiBlkPos ];

    dTemp = dErrScale;  
    lLevelDouble = abs(lLevelDouble * (Long)uiQ);   

    plLevelDouble[ uiBlkPos ] = lLevelDouble;
    UInt uiMaxAbsLevel = lLevelDouble >> iQBits;
    Bool bLowerInt     = ( ( lLevelDouble - Long( uiMaxAbsLevel << iQBits ) ) < Long( 1 << ( iQBits - 1 ) ) ) ? true : false;
    
    if( !bLowerInt )
    {
      uiMaxAbsLevel++;
    }
    
    Double dErr          = Double( lLevelDouble );
    d64BlockUncodedCost += dErr * dErr * dTemp;
    
    piCoeff[ uiBlkPos ] = plSrcCoeff[ uiBlkPos ] > 0 ? uiMaxAbsLevel : -Int( uiMaxAbsLevel );
    
    if ( uiMaxAbsLevel > 0 )
    {
      uiLastScanPosP1 = uiScanPos + 1;
    }
  }

  Int     ui16CtxQtCbf;
  UInt    uiBestLastIdxP1   = 0;
  Double  d64BestCost       = 0;
  Double  d64BaseCost       = 0;
  Double  d64CodedCost      = 0;
  Double  d64UncodedCost    = 0;

  if( !pcCU->isIntra( uiAbsPartIdx ) && eTType == TEXT_LUMA )
  {
    d64BestCost = d64BlockUncodedCost + xGetICost( m_pcEstBitsSbac->blockRootQtCbfBits[ 0 ] );
    d64BaseCost = d64BlockUncodedCost + xGetICost( m_pcEstBitsSbac->blockRootQtCbfBits[ 1 ] );
  }
  else
  {
    ui16CtxQtCbf = pcCU->getCtxQtCbf( uiAbsPartIdx, eTType, 0 );
    d64BestCost = d64BlockUncodedCost + xGetICost( m_pcEstBitsSbac->blockQtCbfBits[ ui16CtxQtCbf ][ 0 ] );
    d64BaseCost = d64BlockUncodedCost + xGetICost( m_pcEstBitsSbac->blockQtCbfBits[ ui16CtxQtCbf ][ 1 ] );
  }

  UInt uiRun = 0;
  UInt uiPrevLevel = 6;

  for( UInt uiScanPos = 0; uiScanPos < uiMaxNumCoeff; uiScanPos++ )
  {
    UInt   uiBlkPos     = pucScan[ uiScanPos ];

    dTemp = dErrScale;  
    UInt    uiLevel                 = xGetCodedLevel( d64UncodedCost, d64CodedCost, plLevelDouble[ uiBlkPos ], abs( piCoeff[ uiBlkPos ] ), uiRun, Min( uiPrevLevel-1, 5 ), Min( uiPrevLevel-1, 5 ), iQBits, dTemp );

    piDstCoeff[ uiBlkPos ]          = plSrcCoeff[ uiBlkPos ] < 0 ? -Int( uiLevel ) : uiLevel;
    d64BaseCost                    -= d64UncodedCost;
    d64BaseCost                    += d64CodedCost;
    
    if( uiLevel )
    {
      //----- check for last flag -----
      Double  d64CostLastZero       = xGetICost( m_pcEstBitsSbac->lastBits[ 0 ][ 0 ] );
      Double  d64CostLastOne        = xGetICost( m_pcEstBitsSbac->lastBits[ 0 ][ 1 ] );
      Double  d64CurrIsLastCost     = d64BaseCost + d64CostLastOne;
      d64BaseCost                  += d64CostLastZero;
      
      if( d64CurrIsLastCost < d64BestCost )
      {
        d64BestCost       = d64CurrIsLastCost;
        uiBestLastIdxP1   = uiScanPos + 1;
      }
      uiRun = 0;
      uiPrevLevel = uiLevel;
    }
    else
    {
      uiRun++;
    }    
  }
  
  //===== clean uncoded coefficients =====
  {
    for( UInt uiScanPos = 0; uiScanPos < uiMaxNumCoeff; uiScanPos++ )
    {
      UInt    uiBlkPos  = pucScan[ uiScanPos ];
      
      if( uiScanPos < uiBestLastIdxP1 )
      {
        if( piDstCoeff[ uiBlkPos ] )
        {
          uiAbsSum += abs( piDstCoeff[ uiBlkPos ] );
        }
      }
      else
      {
        piDstCoeff[ uiBlkPos ] = 0;
      }
    }
  }
}

__inline UInt TComTrQuant::xGetCodedLevel ( Double&                         rd64UncodedCost,
                                            Double&                         rd64CodedCost,
                                            Long                            lLevelDouble,
                                            UInt                            uiMaxAbsLevel,
                                            UInt                            uiRun,
                                            UShort                          uiCtxRun,
                                            UShort                          uiCtxLevel,
                                            Int                             iQBits,
                                            Double                          dTemp   ) const
{
  UInt   uiBestAbsLevel = 0;
  Double dErr1          = Double( lLevelDouble );
  
  rd64UncodedCost = dErr1 * dErr1 * dTemp;
  rd64CodedCost   = rd64UncodedCost + xGetICRateCost( 0, uiRun, uiCtxRun, uiCtxLevel );
  
  UInt uiMinAbsLevel = ( uiMaxAbsLevel > 1 ? uiMaxAbsLevel - 1 : 1 );
  for( UInt uiAbsLevel = uiMaxAbsLevel; uiAbsLevel >= uiMinAbsLevel ; uiAbsLevel-- )
  {
    Double i64Delta  = Double( lLevelDouble  - Long( uiAbsLevel << iQBits ) );
    Double dErr      = Double( i64Delta );
    Double dCurrCost = dErr * dErr * dTemp + xGetICRateCost( uiAbsLevel, uiRun, uiCtxRun, uiCtxLevel );
    
    if( dCurrCost < rd64CodedCost )
    {
      uiBestAbsLevel  = uiAbsLevel;
      rd64CodedCost   = dCurrCost;
    }
  }
  return uiBestAbsLevel;
}

__inline Double TComTrQuant::xGetICRateCost ( UInt                            uiAbsLevel,
                                              UInt                            uiRun,
                                              UInt                            uiCtxRun,
                                              UInt                            uiCtxLevel   ) const
{
  if( uiAbsLevel == 0 )
  {
    Double iRate = 0;
    if ( uiRun == 0 )
    {
      iRate += m_pcEstBitsSbac->runBits[ uiCtxRun ][ 0 ][ 1 ];
    }
    else
    {
      iRate += m_pcEstBitsSbac->runBits[ uiCtxRun ][ 1 ][ 1 ];
    }
    return xGetICost( iRate );
  }

  Double iRate = xGetIEPRate();
  if( uiRun == 0 )
  {
    iRate += m_pcEstBitsSbac->runBits[ uiCtxRun ][ 0 ][ 0 ];
  }
  else
  {
    iRate += m_pcEstBitsSbac->runBits[ uiCtxRun ][ 1 ][ 0 ];
  }

  if( uiAbsLevel == 1 )
  {
    iRate += m_pcEstBitsSbac->levelBits[ uiCtxLevel ][ 0 ][ 0 ];
  }
  else
  {
    iRate += m_pcEstBitsSbac->levelBits[ uiCtxLevel ][ 0 ][ 1 ];
    iRate += m_pcEstBitsSbac->levelBits[ uiCtxLevel ][ 1 ][ 1 ] * (Int)(uiAbsLevel-2);
    iRate += m_pcEstBitsSbac->levelBits[ uiCtxLevel ][ 1 ][ 0 ];
  }
  return xGetICost( iRate );
}

__inline Double TComTrQuant::xGetICost        ( Double                          dRate         ) const
{
  assert(m_dCurrLambda == m_dLambda || m_dCurrLambda == m_dChromaLambda);
  return m_dCurrLambda * dRate;
}

__inline Double TComTrQuant::xGetIEPRate      (                                               ) const
{
  return 32768;
}
