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

/** \file     TComRom.h
    \brief    global variables & functions (header)
*/

#ifndef __TCOMROM__
#define __TCOMROM__

#include "CommonDef.h"

#include<stdio.h>
#include<iostream>

// ====================================================================================================================
// Macros
// ====================================================================================================================

#if ENUM_AND_MACRO_YUQUANHE
#if wlq_mem_bug
#define     MAX_CU_DEPTH            6                           // log2(LCUSize)
#else
#define     MAX_CU_DEPTH            7                           // log2(LCUSize)
#endif
#else 
#define     MAX_CU_DEPTH            7                           // log2(LCUSize)
#endif
#define     MAX_CU_SIZE             (1<<(MAX_CU_DEPTH))         // maximum allowable size of CU

#if wlq_mem_bug
#define     MAX_CU_DEPTH_Pel            7                           // log2(LCUSize)
#endif

#define     MIN_PU_SIZE             2
#define     MAX_NUM_SPU_W           (MAX_CU_SIZE/MIN_PU_SIZE)   // maximum number of SPU in horizontal line

#if wlq_AEC
//数据定义
#define B_BITS_AVS2  10
#define QUARTER_AVS2    (1 << (B_BITS_AVS2-2))

#define LG_PMPS_SHIFTNO_AVS2 2
//数据定义
#define NUN_VALUE_BOUND_AVS2          254  // M3090: Make sure rs1 will not overflow for 8-bit unsign char
#endif

#if wlq_WQ
#define WQ_FLATBASE_INBIT_AVS2		6

#define DETAILED_AVS2				1

#define WQ_MODE_F_AVS2  0  //M2331 2008-04
#define WQ_MODE_U_AVS2  1  //M2331 2008-04
#define WQ_MODE_D_AVS2  2  //M2331 2008-04
#endif

#if wlq_avs2x_debug
extern int g_sign;
#endif
#if ZHANGYI_INTRA
extern const int g_Left_Down_Avail_Matrix64[16][16];
extern const int g_Up_Right_Avail_Matrix64[16][16];
extern const int g_Left_Down_Avail_Matrix32[8][8];
extern const int g_Up_Right_Avail_Matrix32[8][8];
extern const int g_Left_Down_Avail_Matrix16[4][4];
extern const int g_Up_Right_Avail_Matrix16[4][4];
extern const int g_Left_Down_Avail_Matrix8[2][2];
extern const int g_Up_Right_Avail_Matrix8[2][2];
#endif

#if niu_ChromaRDO_revise
extern int MProbableMode[4];
#endif

#if CHROMA_QP_YQH
extern const Char QP_SCALE_CR[64];
#endif
// ====================================================================================================================
// Initialize / destroy functions
// ====================================================================================================================

Void         initROM();
Void         destroyROM();
Void         initFrameScanXY( UInt* pBuff, UInt* pBuffX, UInt* pBuffY, Int iWidth, Int iHeight );

Void         initSigLastScanPattern( UInt* puiScanPattern, const UInt uiLog2BlockSize, const bool bDownLeft );

// ====================================================================================================================
// Data structure related table & variable
// ====================================================================================================================

// flexible conversion from relative to absolute index
extern       UInt   g_auiZscanToRaster[ MAX_NUM_SPU_W*MAX_NUM_SPU_W ];
extern       UInt   g_auiRasterToZscan[ MAX_NUM_SPU_W*MAX_NUM_SPU_W ];

Void         initZscanToRaster ( Int iMaxDepth, Int iDepth, UInt uiStartVal, UInt*& rpuiCurrIdx );
Void         initRasterToZscan ( UInt uiLog2MaxCUSize, UInt uiMaxDepth         );

#if CODEFFCODER_LDW_WLQ_YQH
Void initCGZigzagScanXY( UInt* pBuff, UInt* pBuffX, UInt* pBuffY, Int iWidth, Int iHeight );//yuquanhe@hisilicon.com
Void initZigzagScan_NSQT( UInt* pBuff, UInt* pBuffX, UInt* pBuffY, Int iWidth, Int iHeight );//yuquanhe@hisilicon.com
Void initZigzagInvScan( UInt* pBuff, UInt* pBuffZigzag, Int iWidth, Int iHeight );//yuquanhe@hisilicon.com
#endif

// conversion of partition index to picture pel position
extern       UInt   g_auiRasterToPelX[ MAX_NUM_SPU_W*MAX_NUM_SPU_W ];
extern       UInt   g_auiRasterToPelY[ MAX_NUM_SPU_W*MAX_NUM_SPU_W ];

Void         initRasterToPelXY ( UInt uiLog2MaxCUSize, UInt uiMaxDepth );

// global variable (LCU width/height, max. CU depth)
extern       UInt g_uiLog2MaxCUSize;
extern       UInt g_uiMaxCUDepth;

#define      QUANT_IQUANT_SHIFT    20    // Q(QP%6) * IQ(QP%6) = 2^20
#define      QUANT_SHIFT           14    // Q(4) = 2^14
#define      SCALE_BITS            15    // Inherited from TMuC, pressumably for fractional bit estimates in RDOQ
#define      MAX_TR_DYNAMIC_RANGE  15    // Maximum transform dynamic range (excluding sign bit)

#define      SHIFT_INV_1ST          7    // Shift after first inverse transform stage
#define      SHIFT_INV_2ND         12    // Shift after second inverse transform stage

#if ZHOULULU_QT
extern const unsigned short g_uiQuantTable[80];
extern const unsigned short g_uiDeQuantTable[80];
extern const unsigned short g_uiDeQuantShift[80];
// to be modified
extern const UInt g_uiQuantStep[128];
#else
extern const UInt g_uiQuantStep[128];
#endif


#if ZHOULULU_QT
extern const TCoeff g_aiT2[2][2];
extern const TCoeff g_aiT4[4][4];
extern const TCoeff g_aiT8[8][8];
extern const TCoeff g_aiT16[16][16];
extern const TCoeff g_aiT32[32][32];

extern const TCoeff g_as_C_TRANS[4][4];
extern const TCoeff g_as_SEC_TRANS[4][4];
#else

extern const TCoeff g_aiT2[2][2];
extern const TCoeff g_aiT4[4][4];
extern const TCoeff g_aiT8[8][8];
extern const TCoeff g_aiT16[16][16];
extern const TCoeff g_aiT32[32][32];
#endif
// ====================================================================================================================
// Quantization & DeQuantization
// ====================================================================================================================

extern       UInt   g_aiQuantCoef4      [6];
extern       Int    g_aiDequantCoef4    [6];
extern       UInt   g_aiQuantCoef       [6][16];
extern       Int    g_aiDequantCoef     [6][16];
extern       Int    g_aiDequantCoef64   [6][64];
extern       UInt   g_aiQuantCoef64     [6][64];
extern       UInt   g_aiQuantCoef256    [6][256];
extern       UInt   g_aiDeQuantCoef256  [6][256];
extern       UInt   g_aiQuantCoef1024   [6][1024];
extern       UInt   g_aiDeQuantCoef1024 [6][1024];
extern       UInt   g_aiQuantCoef4096   [6];
extern       UInt   g_aiDeQuantCoef4096 [6];

// ====================================================================================================================
// Luma QP to Chroma QP mapping
// ====================================================================================================================

//extern const UChar  g_aucChromaScale      [52];

// ====================================================================================================================
// Scanning order & context mapping table
// ====================================================================================================================
 #if CODEFFCODER_LDW_WLQ_YQH

#if ZHOULULU_RDOQ

#define    AVS_SCAN1X1       0
#define    AVS_SCAN2X2       1
#define    AVS_SCAN4X4       2
#define    AVS_SCAN8X8       3
#define    AVS_SCAN16X16     4
#define    AVS_SCAN32X32     5



#define    AVS_SCAN2X8       8
#define    AVS_SCAN8X2       9

#define    AVS_CG_SCAN4X16   10
#define    AVS_CG_SCAN16X4   11
#define    AVS_CG_SCAN8X32   12
#define    AVS_CG_SCAN32X8   13
#define    AVS_CG_SCAN8X8    14
#define    AVS_CG_SCAN16X16  15
#define    AVS_CG_SCAN32X32  16

#endif

extern       UInt*  g_auiFrameScanXY[ MAX_CU_DEPTH +12 ];    // raster index     from scanning index
extern       UInt*  g_auiFrameScanX [ MAX_CU_DEPTH +12 ];    // raster index (x) from scanning index
extern       UInt*  g_auiFrameScanY [ MAX_CU_DEPTH +12 ];    // raster index (y) from scanning index
extern       UInt*  g_auiFrameinvScanXY [  4 ];    // raster index (y) from scanning index
extern       UInt   g_auiAntiScan8[64];    
#define	NUM_INTRA_MODE              33 //yuquanhe@hisilicon.com
extern const UInt g_auiIntraModeClassified[NUM_INTRA_MODE] ; //yuquanhe@hisilcion.com// 2D context mapping for coefficients
#else
extern       UInt*  g_auiFrameScanXY[ MAX_CU_DEPTH  ];    // raster index     from scanning index
extern       UInt*  g_auiFrameScanX [ MAX_CU_DEPTH  ];    // raster index (x) from scanning index
extern       UInt*  g_auiFrameScanY [ MAX_CU_DEPTH  ];    // raster index (y) from scanning index
#endif

extern       UInt   g_auiAntiScan8[64];                   // 2D context mapping for coefficients

// ====================================================================================================================
// Bit-depth
// ====================================================================================================================

extern       UInt g_uiBitDepth;
extern       UInt g_uiBASE_MAX;

// ====================================================================================================================
// Texture type to integer mapping
// ====================================================================================================================

extern const UChar g_aucConvertTxtTypeToIdx[4];

// ====================================================================================================================
// Misc.
// ====================================================================================================================

extern       Char   g_aucConvertToBit  [ MAX_CU_SIZE+1 ];   // from width to log2(width)
#if wlq_WQ
extern const UInt g_scalingListSize[SCALING_LIST_SIZE_NUM];
extern const Int g_scaleM[4][4];
extern const Short g_wq_param_default[2][6];

extern const int g_LambdaQPTab[3][3];
extern const  double g_LambdaFTab[3][3];

extern const UChar g_WeightQuantModel8x8[4][64];
extern const UChar g_WeightQuantModel4x4[4][16];
#endif
#if wlq_FME
extern Pel line[64];
extern const Int g_blk_size_AVS2[20][2];
extern const Int quant_coef_AVS2[6][4][4];
#endif

#if DMH
extern const int dmh_pos[DMH_MODE_NUM + DMH_MODE_NUM - 1][2][2];
#endif

#endif  //__TCOMROM__

