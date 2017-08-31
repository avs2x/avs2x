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

/** \file     TComRom.cpp
    \brief    global variables & functions
*/

#include "TComRom.h"
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

#if wlq_avs2x_debug
int g_sign=0;
#endif

#if niu_ChromaRDO_revise
int MProbableMode[4] = { -2 };
#endif

#if ZHANGYI_INTRA
const int g_Left_Down_Avail_Matrix64[16][16] = {
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
}
;
const int g_Left_Down_Avail_Matrix32[8][8] = {
	{1, 0, 1, 0, 1, 0, 1, 0},
	{1, 0, 0, 0, 1, 0, 0, 0},
	{1, 0, 1, 0, 1, 0, 1, 0},
	{1, 0, 0, 0, 0, 0, 0, 0},
	{1, 0, 1, 0, 1, 0, 1, 0},
	{1, 0, 0, 0, 1, 0, 0, 0},
	{1, 0, 1, 0, 1, 0, 1, 0},
	{0, 0, 0, 0, 0, 0, 0, 0}
}
;
const int g_Left_Down_Avail_Matrix16[4][4] = {
	{1, 0, 1, 0},
	{1, 0, 0, 0},
	{1, 0, 1, 0},
	{0, 0, 0, 0}
}
;
const int g_Left_Down_Avail_Matrix8[2][2] = {
	{1, 0},
	{0, 0},
}
;

const int g_Up_Right_Avail_Matrix64[16][16] = {
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0}
}
;
const int g_Up_Right_Avail_Matrix32[8][8] =
	//qyu 0823 0: 8 1:16 2: 32  pu size
{
	{1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 1, 0, 1, 0, 1, 0},
	{1, 1, 1, 0, 1, 1, 1, 0},
	{1, 0, 1, 0, 1, 0, 1, 0},
	{1, 1, 1, 1, 1, 1, 1, 0},
	{1, 0, 1, 0, 1, 0, 1, 0},
	{1, 1, 1, 0, 1, 1, 1, 0},
	{1, 0, 1, 0, 1, 0, 1, 0}
}
;
const int g_Up_Right_Avail_Matrix16[4][4] = {
	{1, 1, 1, 1},
	{1, 0, 1, 0},
	{1, 1, 1, 0},
	{1, 0, 1, 0}
}
;
const int g_Up_Right_Avail_Matrix8[2][2] = {
	{1, 1},
	{1, 0},
}
;
#endif
#if CHROMA_QP_YQH
const Char QP_SCALE_CR[64] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 42, 43, 43, 44, 44, 45, 45,
	46, 46, 47, 47, 48, 48, 48, 49, 49, 49,
	50, 50, 50, 51,
};

#endif

// ====================================================================================================================
// Initialize / destroy functions
// ====================================================================================================================

// initialize ROM variables
Void initROM()
{
  Int i, c;

  ::memset( g_aucConvertToBit,   -1, sizeof( g_aucConvertToBit ) );
  c=0;
  for ( i=1; i<MAX_CU_SIZE; i*=2 )
  {
    g_aucConvertToBit[ i ] = c;
    c++;
  }
  g_aucConvertToBit[ i ] = c;

#if ZHOULULU_RDOQ
  c = 1;
  g_auiFrameScanXY[0] = new UInt[c*c];
  g_auiFrameScanX[0] = new UInt[c*c];
  g_auiFrameScanY[0] = new UInt[c*c];

  g_auiFrameScanXY[0][0] = 0;
  g_auiFrameScanX[0][0] = 0;
  g_auiFrameScanY[0][0] = 0;
#endif


  // g_auiFrameScanXY[ g_aucConvertToBit[ transformSize ] ]: zigzag scan array for transformSize
  c=2;
  for ( i=1; i<MAX_CU_DEPTH; i++ )
  {
    g_auiFrameScanXY[ i ] = new UInt[ c*c ];
    g_auiFrameScanX [ i ] = new UInt[ c*c ];
    g_auiFrameScanY [ i ] = new UInt[ c*c ];
    initFrameScanXY( g_auiFrameScanXY[i], g_auiFrameScanX[i], g_auiFrameScanY[i], c, c );
    c <<= 1;
  }
#if CODEFFCODER_LDW_WLQ_YQH
  c=4;
  for ( i=8; i<2+8; i=i+2 )
  {

	  g_auiFrameScanXY[ i ] = new UInt[ c*c ];
	  g_auiFrameScanX [ i ] = new UInt[ c*c ];
	  g_auiFrameScanY [ i ] = new UInt[ c*c ];
	  initZigzagScan_NSQT( g_auiFrameScanXY[i], g_auiFrameScanX[i], g_auiFrameScanY[i], c>>1, c<<1 );
	  g_auiFrameinvScanXY[ i-8 ] = new UInt[ c*c ];
	  initZigzagInvScan( g_auiFrameinvScanXY[i-8], g_auiFrameScanXY[i], c>>1, c<<1 );
	  
	  g_auiFrameScanXY[ i+1 ] = new UInt[ c*c ];
	  g_auiFrameScanX [ i+1 ] = new UInt[ c*c ];
	  g_auiFrameScanY [ i+1 ] = new UInt[ c*c ];
	  initZigzagScan_NSQT( g_auiFrameScanXY[i+1], g_auiFrameScanX[i+1], g_auiFrameScanY[i+1], c<<1, c>>1 );
	  g_auiFrameinvScanXY[ i+1-8 ] = new UInt[ c*c ];
	  initZigzagInvScan( g_auiFrameinvScanXY[i+1-8], g_auiFrameScanXY[i+1],  c<<1, c>>1);
  }

  c=4;
  for ( i=2; i<(4); i=i+1 )
  {
	  g_auiFrameinvScanXY[ i ] = new UInt[ c*c ];
	  initZigzagInvScan( g_auiFrameinvScanXY[i], g_auiFrameScanXY[i],  c, c );
	  c <<= 1;
  }

  c=8;
  for ( i=2+8; i<(6+8); i=i+2 )
  {

	  g_auiFrameScanXY[ i ] = new UInt[ c*c ];
	  g_auiFrameScanX [ i ] = new UInt[ c*c ];
	  g_auiFrameScanY [ i ] = new UInt[ c*c ];
	  initCGZigzagScanXY( g_auiFrameScanXY[i], g_auiFrameScanX[i], g_auiFrameScanY[i], c>>1, c<<1 );
	  g_auiFrameScanXY[ i+1 ] = new UInt[ c*c ];
	  g_auiFrameScanX [ i+1 ] = new UInt[ c*c ];
	  g_auiFrameScanY [ i+1 ] = new UInt[ c*c ];
	  initCGZigzagScanXY( g_auiFrameScanXY[i+1], g_auiFrameScanX[i+1], g_auiFrameScanY[i+1], c<<1, c>>1  );
	  c <<= 1;
  }

  c=8;
  for ( i=6+8; i<(9+8); i++ )
  {
	  g_auiFrameScanXY[ i ] = new UInt[ c*c ];
	  g_auiFrameScanX [ i ] = new UInt[ c*c ];
	  g_auiFrameScanY [ i ] = new UInt[ c*c ];
	  initCGZigzagScanXY( g_auiFrameScanXY[i], g_auiFrameScanX[i], g_auiFrameScanY[i], c, c );
	  c <<= 1;
  }
#endif

}

Void destroyROM()
{
  Int i;
  
  for ( i=0; i<MAX_CU_DEPTH; i++ )
  {
    delete[] g_auiFrameScanXY[i];
    delete[] g_auiFrameScanX [i];
    delete[] g_auiFrameScanY [i];
  }
#if CODEFFCODER_LDW_WLQ_YQH
  for ( i=7; i<MAX_CU_DEPTH+12; i++ )//yuquanhe@hisilicon.com
  {
	  delete[] g_auiFrameScanXY[i];
	  delete[] g_auiFrameScanX [i];
	  delete[] g_auiFrameScanY [i];
  }
  for ( i=0; i<4; i++ )//yuquanhe@hisilicon.com
  {
	  delete[] g_auiFrameinvScanXY[i];
  }


#endif
}

// ====================================================================================================================
// Data structure related table & variable
// ====================================================================================================================
#if wlq_mem_bug
UInt g_uiLog2MaxCUSize = MAX_CU_DEPTH_Pel;
UInt g_uiMaxCUDepth    = MAX_CU_DEPTH;
#else
UInt g_uiLog2MaxCUSize = MAX_CU_DEPTH;
UInt g_uiMaxCUDepth = MAX_CU_DEPTH;
#endif


UInt g_auiZscanToRaster [ MAX_NUM_SPU_W*MAX_NUM_SPU_W ] = { 0, };
UInt g_auiRasterToZscan [ MAX_NUM_SPU_W*MAX_NUM_SPU_W ] = { 0, };
UInt g_auiRasterToPelX  [ MAX_NUM_SPU_W*MAX_NUM_SPU_W ] = { 0, };
UInt g_auiRasterToPelY  [ MAX_NUM_SPU_W*MAX_NUM_SPU_W ] = { 0, };

Void initZscanToRaster ( Int iMaxDepth, Int iDepth, UInt uiStartVal, UInt*& rpuiCurrIdx )
{
  Int iStride = 1 << ( iMaxDepth - 1 );
  
  if ( iDepth == iMaxDepth )
  {
    rpuiCurrIdx[0] = uiStartVal;
    rpuiCurrIdx++;
  }
  else
  {
    Int iStep = iStride >> iDepth;
    initZscanToRaster( iMaxDepth, iDepth+1, uiStartVal,                     rpuiCurrIdx );
    initZscanToRaster( iMaxDepth, iDepth+1, uiStartVal+iStep,               rpuiCurrIdx );
    initZscanToRaster( iMaxDepth, iDepth+1, uiStartVal+iStep*iStride,       rpuiCurrIdx );
    initZscanToRaster( iMaxDepth, iDepth+1, uiStartVal+iStep*iStride+iStep, rpuiCurrIdx );
  }
}

Void initRasterToZscan ( UInt uiLog2MaxCUSize, UInt uiMaxDepth )
{
  UInt  uiLog2NumPartInWidth = uiMaxDepth - 1;
  
  for ( UInt i = 0; i < 1<<(uiLog2NumPartInWidth<<1); i++ )
  {
    g_auiRasterToZscan[ g_auiZscanToRaster[i] ] = i;
  }
}

Void initRasterToPelXY ( UInt uiLog2MaxCUSize, UInt uiMaxDepth )
{
  UInt    i;
  
  UInt* uiTempX = &g_auiRasterToPelX[0];
  UInt* uiTempY = &g_auiRasterToPelY[0];
  
  UInt  uiLog2MinCUSize      = uiLog2MaxCUSize - uiMaxDepth + 1;
  UInt  uiLog2NumPartInWidth = uiMaxDepth - 1;
  UInt  uiNumPartInWidth = 1<<uiLog2NumPartInWidth;
  
  uiTempX[0] = 0; uiTempX++;
  for ( i = 1; i < uiNumPartInWidth; i++ )
  {
    uiTempX[0] = uiTempX[-1] + (1<<uiLog2MinCUSize); uiTempX++;
  }
  for ( i = 1; i < uiNumPartInWidth; i++ )
  {
    memcpy(uiTempX, uiTempX-uiNumPartInWidth, sizeof(UInt)*uiNumPartInWidth);
    uiTempX += uiNumPartInWidth;
  }
  
  for ( i = 1; i < (1<<(uiLog2NumPartInWidth<<1)); i++ )
  {
    uiTempY[i] = ( i >>uiLog2NumPartInWidth )<<uiLog2MinCUSize;
  }
};

#if ZHOULULU_QT

const unsigned short g_uiQuantTable[80] = 
{
	32768, 29775, 27554, 25268, 23170, 21247, 19369, 17770,
	16302, 15024, 13777, 12634, 11626, 10624, 9742, 8958,
	8192, 7512, 6889, 6305, 5793, 5303, 4878, 4467,
	4091, 3756, 3444, 3161, 2894, 2654, 2435, 2235,
	2048, 1878, 1722, 1579, 1449, 1329, 1218, 1117,
	1024, 939, 861, 790, 724, 664, 609, 558,
	512, 470, 430, 395, 362, 332, 304, 279,
	256, 235, 215, 197, 181, 166, 152, 140,
	128, 116, 108, 99, 91, 83, 76, 69,
	64, 59, 54, 49, 45, 41, 38, 35

};

const unsigned short g_uiDeQuantShift[80] = 
{
	15, 15, 15, 15, 15, 15, 15, 15,
	14, 14, 14, 14, 14, 14, 14, 14,
	14, 13, 13, 13, 13, 13, 13, 13,
	12, 12, 12, 12, 12, 12, 12, 12,
	12, 11, 11, 11, 11, 11, 11, 11,
	11, 10, 10, 10, 10, 10, 10, 10,
	10, 9, 9, 9, 9, 9, 9, 9,
	8, 8, 8, 8, 8, 8, 8, 8,
	7, 7, 7, 7, 7, 7, 7, 7,
	6, 6, 6, 6, 6, 6, 6, 6

};

const unsigned short g_uiDeQuantTable[80] = {

	32768, 36061, 38968, 42495, 46341, 50535, 55437, 60424,
	32932, 35734, 38968, 42495, 46177, 50535, 55109, 59933,
	65535, 35734, 38968, 42577, 46341, 50617, 55027, 60097,
	32809, 35734, 38968, 42454, 46382, 50576, 55109, 60056,
	65535, 35734, 38968, 42495, 46320, 50515, 55109, 60076,
	65535, 35744, 38968, 42495, 46341, 50535, 55099, 60087,
	65535, 35734, 38973, 42500, 46341, 50535, 55109, 60097,
	32771, 35734, 38965, 42497, 46341, 50535, 55109, 60099,
	32768, 36061, 38968, 42495, 46341, 50535, 55437, 60424,
	32932, 35734, 38968, 42495, 46177, 50535, 55109, 59933

};

const UInt g_uiQuantStep[128] = {
	64,   66,   67,   69,   70,   72,   74,   76,   78,   80,
	82,   84,   87,   89,   92,   94,   97,  100,  103,  106,
	109,  112,  116,  119,  123,  127,  131,  135,  139,  144,
	149,  154,  159,  164,  170,  176,  182,  188,  195,  202,
	209,  217,  225,  234,  242,  252,  261,  271,  282,  293,
	304,  317,  329,  343,  357,  371,  387,  403,  420,  438,
	457,  476,  497,  519,  542,  566,  591,  618,  646,  675,
	706,  739,  774,  810,  848,  889,  931,  976, 1024, 1074,
	1127, 1183, 1242, 1305, 1371, 1441, 1514, 1593, 1675, 1763,
	1856, 1954, 2058, 2168, 2285, 2409, 2540, 2680, 2828, 2985,
	3151, 3329, 3517, 3717, 3929, 4155, 4395, 4651, 4923, 5213,
	5521, 5850, 6200, 6572, 6970, 7394, 7846, 8328, 8843, 9392,
	9979, 10605, 11274, 11990, 12754, 13572, 14447, 15383
};

#else

const UInt g_uiQuantStep[128] = {
    64,   66,   67,   69,   70,   72,   74,   76,   78,   80,
    82,   84,   87,   89,   92,   94,   97,  100,  103,  106,
   109,  112,  116,  119,  123,  127,  131,  135,  139,  144,
   149,  154,  159,  164,  170,  176,  182,  188,  195,  202,
   209,  217,  225,  234,  242,  252,  261,  271,  282,  293,
   304,  317,  329,  343,  357,  371,  387,  403,  420,  438,
   457,  476,  497,  519,  542,  566,  591,  618,  646,  675,
   706,  739,  774,  810,  848,  889,  931,  976, 1024, 1074,
  1127, 1183, 1242, 1305, 1371, 1441, 1514, 1593, 1675, 1763,
  1856, 1954, 2058, 2168, 2285, 2409, 2540, 2680, 2828, 2985,
  3151, 3329, 3517, 3717, 3929, 4155, 4395, 4651, 4923, 5213,
  5521, 5850, 6200, 6572, 6970, 7394, 7846, 8328, 8843, 9392,
  9979, 10605, 11274, 11990, 12754, 13572, 14447, 15383
};
#endif

#if ZHOULULU_QT
const TCoeff g_aiT2[2][2] = {
	{ 64, 64},
	{ 64,-64},
};

const TCoeff g_aiT4[4][4] = {
	{  32,    32,     32,     32 },
	{  42,    17,    -17,    -42 },
	{  32,   -32,    -32,     32 },
	{  17,   -42,     42,    -17 }
};

const TCoeff g_aiT8[8][8] = {
	{  32,    32,     32,     32,     32,     32,     32,     32    },
	{  44,    38,     25,      9,     -9,    -25,    -38,    -44    },
	{  42,    17,    -17,    -42,    -42,    -17,     17,     42    },
	{  38,    -9,    -44,    -25,     25,     44,      9,    -38    },
	{  32,   -32,    -32,     32,     32,    -32,    -32,     32    },
	{  25,   -44,      9,     38,    -38,     -9,     44,    -25    },
	{  17,   -42,     42,    -17,    -17,     42,    -42,     17    },
	{   9,   -25,     38,    -44,     44,    -38,     25,     -9    }
};

const TCoeff g_aiT16[16][16] = {
	{  32,    32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32    },
	{  45,    43,     40,     35,     29,     21,     13,      4,     -4,    -13,    -21,    -29,    -35,    -40,    -43,    -45    },
	{  44,    38,     25,      9,     -9,    -25,    -38,    -44,    -44,    -38,    -25,     -9,      9,     25,     38,     44    },
	{  43,    29,      4,    -21,    -40,    -45,    -35,    -13,     13,     35,     45,     40,     21,     -4,    -29,    -43    },
	{  42,    17,    -17,    -42,    -42,    -17,     17,     42,     42,     17,    -17,    -42,    -42,    -17,     17,     42    },
	{  40,     4,    -35,    -43,    -13,     29,     45,     21,    -21,    -45,    -29,     13,     43,     35,     -4,    -40    },
	{  38,    -9,    -44,    -25,     25,     44,      9,    -38,    -38,      9,     44,     25,    -25,    -44,     -9,     38    },
	{  35,   -21,    -43,      4,     45,     13,    -40,    -29,     29,     40,    -13,    -45,     -4,     43,     21,    -35    },
	{  32,   -32,    -32,     32,     32,    -32,    -32,     32,     32,    -32,    -32,     32,     32,    -32,    -32,     32    },
	{  29,   -40,    -13,     45,     -4,    -43,     21,     35,    -35,    -21,     43,      4,    -45,     13,     40,    -29    },
	{  25,   -44,      9,     38,    -38,     -9,     44,    -25,    -25,     44,     -9,    -38,     38,      9,    -44,     25    },
	{  21,   -45,     29,     13,    -43,     35,      4,    -40,     40,     -4,    -35,     43,    -13,    -29,     45,    -21    },
	{  17,   -42,     42,    -17,    -17,     42,    -42,     17,     17,    -42,     42,    -17,    -17,     42,    -42,     17    },
	{  13,   -35,     45,    -40,     21,      4,    -29,     43,    -43,     29,     -4,    -21,     40,    -45,     35,    -13    },
	{   9,   -25,     38,    -44,     44,    -38,     25,     -9,     -9,     25,    -38,     44,    -44,     38,    -25,      9    },
	{   4,   -13,     21,    -29,     35,    -40,     43,    -45,     45,    -43,     40,    -35,     29,    -21,     13,     -4    }
};

const TCoeff g_aiT32[32][32] = {
	{  32,    32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32,     32    },
	{  45,    45,     44,     43,     41,     39,     36,     34,     30,     27,     23,     19,     15,     11,      7,      2,     -2,     -7,    -11,    -15,    -19,    -23,    -27,    -30,    -34,    -36,    -39,    -41,    -43,    -44,    -45,    -45    },
	{  45,    43,     40,     35,     29,     21,     13,      4,     -4,    -13,    -21,    -29,    -35,    -40,    -43,    -45,    -45,    -43,    -40,    -35,    -29,    -21,    -13,     -4,      4,     13,     21,     29,     35,     40,     43,     45    },
	{  45,    41,     34,     23,     11,     -2,    -15,    -27,    -36,    -43,    -45,    -44,    -39,    -30,    -19,     -7,      7,     19,     30,     39,     44,     45,     43,     36,     27,     15,      2,    -11,    -23,    -34,    -41,    -45    },
	{  44,    38,     25,      9,     -9,    -25,    -38,    -44,    -44,    -38,    -25,     -9,      9,     25,     38,     44,     44,     38,     25,      9,     -9,    -25,    -38,    -44,    -44,    -38,    -25,     -9,      9,     25,     38,     44    },
	{  44,    34,     15,     -7,    -27,    -41,    -45,    -39,    -23,     -2,     19,     36,     45,     43,     30,     11,    -11,    -30,    -43,    -45,    -36,    -19,      2,     23,     39,     45,     41,     27,      7,    -15,    -34,    -44    },
	{  43,    29,      4,    -21,    -40,    -45,    -35,    -13,     13,     35,     45,     40,     21,     -4,    -29,    -43,    -43,    -29,     -4,     21,     40,     45,     35,     13,    -13,    -35,    -45,    -40,    -21,      4,     29,     43    },
	{  43,    23,     -7,    -34,    -45,    -36,    -11,     19,     41,     44,     27,     -2,    -30,    -45,    -39,    -15,     15,     39,     45,     30,      2,    -27,    -44,    -41,    -19,     11,     36,     45,     34,      7,    -23,    -43    },
	{  42,    17,    -17,    -42,    -42,    -17,     17,     42,     42,     17,    -17,    -42,    -42,    -17,     17,     42,     42,     17,    -17,    -42,    -42,    -17,     17,     42,     42,     17,    -17,    -42,    -42,    -17,     17,     42    },
	{  41,    11,    -27,    -45,    -30,      7,     39,     43,     15,    -23,    -45,    -34,      2,     36,     44,     19,    -19,    -44,    -36,     -2,     34,     45,     23,    -15,    -43,    -39,     -7,     30,     45,     27,    -11,    -41    },
	{  40,     4,    -35,    -43,    -13,     29,     45,     21,    -21,    -45,    -29,     13,     43,     35,     -4,    -40,    -40,     -4,     35,     43,     13,    -29,    -45,    -21,     21,     45,     29,    -13,    -43,    -35,      4,     40    },
	{  39,    -2,    -41,    -36,      7,     43,     34,    -11,    -44,    -30,     15,     45,     27,    -19,    -45,    -23,     23,     45,     19,    -27,    -45,    -15,     30,     44,     11,    -34,    -43,     -7,     36,     41,      2,    -39    },
	{  38,    -9,    -44,    -25,     25,     44,      9,    -38,    -38,      9,     44,     25,    -25,    -44,     -9,     38,     38,     -9,    -44,    -25,     25,     44,      9,    -38,    -38,      9,     44,     25,    -25,    -44,     -9,     38    },
	{  36,   -15,    -45,    -11,     39,     34,    -19,    -45,     -7,     41,     30,    -23,    -44,     -2,     43,     27,    -27,    -43,      2,     44,     23,    -30,    -41,      7,     45,     19,    -34,    -39,     11,     45,     15,    -36    },
	{  35,   -21,    -43,      4,     45,     13,    -40,    -29,     29,     40,    -13,    -45,     -4,     43,     21,    -35,    -35,     21,     43,     -4,    -45,    -13,     40,     29,    -29,    -40,     13,     45,      4,    -43,    -21,     35    },
	{  34,   -27,    -39,     19,     43,    -11,    -45,      2,     45,      7,    -44,    -15,     41,     23,    -36,    -30,     30,     36,    -23,    -41,     15,     44,     -7,    -45,     -2,     45,     11,    -43,    -19,     39,     27,    -34    },
	{  32,   -32,    -32,     32,     32,    -32,    -32,     32,     32,    -32,    -32,     32,     32,    -32,    -32,     32,     32,    -32,    -32,     32,     32,    -32,    -32,     32,     32,    -32,    -32,     32,     32,    -32,    -32,     32    },
	{  30,   -36,    -23,     41,     15,    -44,     -7,     45,     -2,    -45,     11,     43,    -19,    -39,     27,     34,    -34,    -27,     39,     19,    -43,    -11,     45,      2,    -45,      7,     44,    -15,    -41,     23,     36,    -30    },
	{  29,   -40,    -13,     45,     -4,    -43,     21,     35,    -35,    -21,     43,      4,    -45,     13,     40,    -29,    -29,     40,     13,    -45,      4,     43,    -21,    -35,     35,     21,    -43,     -4,     45,    -13,    -40,     29    },
	{  27,   -43,     -2,     44,    -23,    -30,     41,      7,    -45,     19,     34,    -39,    -11,     45,    -15,    -36,     36,     15,    -45,     11,     39,    -34,    -19,     45,     -7,    -41,     30,     23,    -44,      2,     43,    -27    },
	{  25,   -44,      9,     38,    -38,     -9,     44,    -25,    -25,     44,     -9,    -38,     38,      9,    -44,     25,     25,    -44,      9,     38,    -38,     -9,     44,    -25,    -25,     44,     -9,    -38,     38,      9,    -44,     25    },
	{  23,   -45,     19,     27,    -45,     15,     30,    -44,     11,     34,    -43,      7,     36,    -41,      2,     39,    -39,     -2,     41,    -36,     -7,     43,    -34,    -11,     44,    -30,    -15,     45,    -27,    -19,     45,    -23    },
	{  21,   -45,     29,     13,    -43,     35,      4,    -40,     40,     -4,    -35,     43,    -13,    -29,     45,    -21,    -21,     45,    -29,    -13,     43,    -35,     -4,     40,    -40,      4,     35,    -43,     13,     29,    -45,     21    },
	{  19,   -44,     36,     -2,    -34,     45,    -23,    -15,     43,    -39,      7,     30,    -45,     27,     11,    -41,     41,    -11,    -27,     45,    -30,     -7,     39,    -43,     15,     23,    -45,     34,      2,    -36,     44,    -19    },
	{  17,   -42,     42,    -17,    -17,     42,    -42,     17,     17,    -42,     42,    -17,    -17,     42,    -42,     17,     17,    -42,     42,    -17,    -17,     42,    -42,     17,     17,    -42,     42,    -17,    -17,     42,    -42,     17    },
	{  15,   -39,     45,    -30,      2,     27,    -44,     41,    -19,    -11,     36,    -45,     34,     -7,    -23,     43,    -43,     23,      7,    -34,     45,    -36,     11,     19,    -41,     44,    -27,     -2,     30,    -45,     39,    -15    },
	{  13,   -35,     45,    -40,     21,      4,    -29,     43,    -43,     29,     -4,    -21,     40,    -45,     35,    -13,    -13,     35,    -45,     40,    -21,     -4,     29,    -43,     43,    -29,      4,     21,    -40,     45,    -35,     13    },
	{  11,   -30,     43,    -45,     36,    -19,     -2,     23,    -39,     45,    -41,     27,     -7,    -15,     34,    -44,     44,    -34,     15,      7,    -27,     41,    -45,     39,    -23,      2,     19,    -36,     45,    -43,     30,    -11    },
	{   9,   -25,     38,    -44,     44,    -38,     25,     -9,     -9,     25,    -38,     44,    -44,     38,    -25,      9,      9,    -25,     38,    -44,     44,    -38,     25,     -9,     -9,     25,    -38,     44,    -44,     38,    -25,      9    },
	{   7,   -19,     30,    -39,     44,    -45,     43,    -36,     27,    -15,      2,     11,    -23,     34,    -41,     45,    -45,     41,    -34,     23,    -11,     -2,     15,    -27,     36,    -43,     45,    -44,     39,    -30,     19,     -7    },
	{   4,   -13,     21,    -29,     35,    -40,     43,    -45,     45,    -43,     40,    -35,     29,    -21,     13,     -4,     -4,     13,    -21,     29,    -35,     40,    -43,     45,    -45,     43,    -40,     35,    -29,     21,    -13,      4    },
	{   2,    -7,     11,    -15,     19,    -23,     27,    -30,     34,    -36,     39,    -41,     43,    -44,     45,    -45,     45,    -45,     44,    -43,     41,    -39,     36,    -34,     30,    -27,     23,    -19,     15,    -11,      7,     -2    }
};

const TCoeff g_as_C_TRANS[4][4] = 
{
	{    34,    58,    72,     81,},
	{    77,    69,    -7,    -75,},
	{    79,   -33,   -75,     58,},
	{    55,   -84,    73,    -28,}

};

const TCoeff g_as_SEC_TRANS[4][4] = 
{
	{   123,   -35,    -8,    -3,},
	{   -32,  -120,    30,    10,},
	{    14,    25,   123,   -22,},
	{     8,    13,    19,   126,},
};
#else
const TCoeff g_aiT2[2][2] = {
  { 64, 64},
  { 64,-64},
};

const TCoeff g_aiT4[4][4] = {
  { 64, 64, 64, 64},
  { 84, 35,-35,-84},
  { 64,-64,-64, 64},
  { 35,-84, 84,-35}
};

const TCoeff g_aiT8[8][8] = {
  { 64, 64, 64, 64, 64, 64, 64, 64},
  { 89, 75, 50, 18,-18,-50,-75,-89},
  { 84, 35,-35,-84,-84,-35, 35, 84},
  { 75,-18,-89,-50, 50, 89, 18,-75},
  { 64,-64,-64, 64, 64,-64,-64, 64},
  { 50,-89, 18, 75,-75,-18, 89,-50},
  { 35,-84, 84,-35,-35, 84,-84, 35},
  { 18,-50, 75,-89, 89,-75, 50,-18}
};

const TCoeff g_aiT16[16][16] = {
  { 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64},
  { 90, 87, 80, 70, 57, 43, 26,  9, -9,-26,-43,-57,-70,-80,-87,-90},
  { 89, 75, 50, 18,-18,-50,-75,-89,-89,-75,-50,-18, 18, 50, 75, 89},
  { 87, 57,  9,-43,-80,-90,-70,-26, 26, 70, 90, 80, 43, -9,-57,-87},
  { 84, 35,-35,-84,-84,-35, 35, 84, 84, 35,-35,-84,-84,-35, 35, 84},
  { 80,  9,-70,-87,-26, 57, 90, 43,-43,-90,-57, 26, 87, 70, -9,-80},
  { 75,-18,-89,-50, 50, 89, 18,-75,-75, 18, 89, 50,-50,-89,-18, 75},
  { 70,-43,-87,  9, 90, 26,-80,-57, 57, 80,-26,-90, -9, 87, 43,-70},
  { 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64},
  { 57,-80,-26, 90, -9,-87, 43, 70,-70,-43, 87,  9,-90, 26, 80,-57},
  { 50,-89, 18, 75,-75,-18, 89,-50,-50, 89,-18,-75, 75, 18,-89, 50},
  { 43,-90, 57, 26,-87, 70,  9,-80, 80, -9,-70, 87,-26,-57, 90,-43},
  { 35,-84, 84,-35,-35, 84,-84, 35, 35,-84, 84,-35,-35, 84,-84, 35},
  { 26,-70, 90,-80, 43,  9,-57, 87,-87, 57, -9,-43, 80,-90, 70,-26},
  { 18,-50, 75,-89, 89,-75, 50,-18,-18, 50,-75, 89,-89, 75,-50, 18},
  {  9,-26, 43,-57, 70,-80, 87,-90, 90,-87, 80,-70, 57,-43, 26, -9}
};

const TCoeff g_aiT32[32][32] = {
  { 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64},
  { 90, 90, 88, 85, 82, 78, 73, 67, 61, 54, 47, 39, 30, 22, 13,  4, -4,-13,-22,-30,-39,-47,-54,-61,-67,-73,-78,-82,-85,-88,-90,-90},
  { 90, 87, 80, 70, 57, 43, 26,  9, -9,-26,-43,-57,-70,-80,-87,-90,-90,-87,-80,-70,-57,-43,-26, -9,  9, 26, 43, 57, 70, 80, 87, 90},
  { 90, 82, 67, 47, 22, -4,-30,-54,-73,-85,-90,-88,-78,-61,-39,-13, 13, 39, 61, 78, 88, 90, 85, 73, 54, 30,  4,-22,-47,-67,-82,-90},
  { 89, 75, 50, 18,-18,-50,-75,-89,-89,-75,-50,-18, 18, 50, 75, 89, 89, 75, 50, 18,-18,-50,-75,-89,-89,-75,-50,-18, 18, 50, 75, 89},
  { 88, 67, 30,-13,-54,-82,-90,-78,-47, -4, 39, 73, 90, 85, 61, 22,-22,-61,-85,-90,-73,-39,  4, 47, 78, 90, 82, 54, 13,-30,-67,-88},
  { 87, 57,  9,-43,-80,-90,-70,-26, 26, 70, 90, 80, 43, -9,-57,-87,-87,-57, -9, 43, 80, 90, 70, 26,-26,-70,-90,-80,-43,  9, 57, 87},
  { 85, 47,-13,-67,-90,-73,-22, 39, 82, 88, 54, -4,-61,-90,-78,-30, 30, 78, 90, 61,  4,-54,-88,-82,-39, 22, 73, 90, 67, 13,-47,-85},
  { 84, 35,-35,-84,-84,-35, 35, 84, 84, 35,-35,-84,-84,-35, 35, 84, 84, 35,-35,-84,-84,-35, 35, 84, 84, 35,-35,-84,-84,-35, 35, 84},
  { 82, 22,-54,-90,-61, 13, 78, 85, 30,-47,-90,-67,  4, 73, 88, 39,-39,-88,-73, -4, 67, 90, 47,-30,-85,-78,-13, 61, 90, 54,-22,-82},
  { 80,  9,-70,-87,-26, 57, 90, 43,-43,-90,-57, 26, 87, 70, -9,-80,-80, -9, 70, 87, 26,-57,-90,-43, 43, 90, 57,-26,-87,-70,  9, 80},
  { 78, -4,-82,-73, 13, 85, 67,-22,-88,-61, 30, 90, 54,-39,-90,-47, 47, 90, 39,-54,-90,-30, 61, 88, 22,-67,-85,-13, 73, 82,  4,-78},
  { 75,-18,-89,-50, 50, 89, 18,-75,-75, 18, 89, 50,-50,-89,-18, 75, 75,-18,-89,-50, 50, 89, 18,-75,-75, 18, 89, 50,-50,-89,-18, 75},
  { 73,-30,-90,-22, 78, 67,-39,-90,-13, 82, 61,-47,-88, -4, 85, 54,-54,-85,  4, 88, 47,-61,-82, 13, 90, 39,-67,-78, 22, 90, 30,-73},
  { 70,-43,-87,  9, 90, 26,-80,-57, 57, 80,-26,-90, -9, 87, 43,-70,-70, 43, 87, -9,-90,-26, 80, 57,-57,-80, 26, 90,  9,-87,-43, 70},
  { 67,-54,-78, 39, 85,-22,-90,  4, 90, 13,-88,-30, 82, 47,-73,-61, 61, 73,-47,-82, 30, 88,-13,-90, -4, 90, 22,-85,-39, 78, 54,-67},
  { 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64},
  { 61,-73,-47, 82, 30,-88,-13, 90, -4,-90, 22, 85,-39,-78, 54, 67,-67,-54, 78, 39,-85,-22, 90,  4,-90, 13, 88,-30,-82, 47, 73,-61},
  { 57,-80,-26, 90, -9,-87, 43, 70,-70,-43, 87,  9,-90, 26, 80,-57,-57, 80, 26,-90,  9, 87,-43,-70, 70, 43,-87, -9, 90,-26,-80, 57},
  { 54,-85, -4, 88,-47,-61, 82, 13,-90, 39, 67,-78,-22, 90,-30,-73, 73, 30,-90, 22, 78,-67,-39, 90,-13,-82, 61, 47,-88,  4, 85,-54},
  { 50,-89, 18, 75,-75,-18, 89,-50,-50, 89,-18,-75, 75, 18,-89, 50, 50,-89, 18, 75,-75,-18, 89,-50,-50, 89,-18,-75, 75, 18,-89, 50},
  { 47,-90, 39, 54,-90, 30, 61,-88, 22, 67,-85, 13, 73,-82,  4, 78,-78, -4, 82,-73,-13, 85,-67,-22, 88,-61,-30, 90,-54,-39, 90,-47},
  { 43,-90, 57, 26,-87, 70,  9,-80, 80, -9,-70, 87,-26,-57, 90,-43,-43, 90,-57,-26, 87,-70, -9, 80,-80,  9, 70,-87, 26, 57,-90, 43},
  { 39,-88, 73, -4,-67, 90,-47,-30, 85,-78, 13, 61,-90, 54, 22,-82, 82,-22,-54, 90,-61,-13, 78,-85, 30, 47,-90, 67,  4,-73, 88,-39},
  { 35,-84, 84,-35,-35, 84,-84, 35, 35,-84, 84,-35,-35, 84,-84, 35, 35,-84, 84,-35,-35, 84,-84, 35, 35,-84, 84,-35,-35, 84,-84, 35},
  { 30,-78, 90,-61,  4, 54,-88, 82,-39,-22, 73,-90, 67,-13,-47, 85,-85, 47, 13,-67, 90,-73, 22, 39,-82, 88,-54, -4, 61,-90, 78,-30},
  { 26,-70, 90,-80, 43,  9,-57, 87,-87, 57, -9,-43, 80,-90, 70,-26,-26, 70,-90, 80,-43, -9, 57,-87, 87,-57,  9, 43,-80, 90,-70, 26},
  { 22,-61, 85,-90, 73,-39, -4, 47,-78, 90,-82, 54,-13,-30, 67,-88, 88,-67, 30, 13,-54, 82,-90, 78,-47,  4, 39,-73, 90,-85, 61,-22},
  { 18,-50, 75,-89, 89,-75, 50,-18,-18, 50,-75, 89,-89, 75,-50, 18, 18,-50, 75,-89, 89,-75, 50,-18,-18, 50,-75, 89,-89, 75,-50, 18},
  { 13,-39, 61,-78, 88,-90, 85,-73, 54,-30,  4, 22,-47, 67,-82, 90,-90, 82,-67, 47,-22, -4, 30,-54, 73,-85, 90,-88, 78,-61, 39,-13},
  {  9,-26, 43,-57, 70,-80, 87,-90, 90,-87, 80,-70, 57,-43, 26, -9, -9, 26,-43, 57,-70, 80,-87, 90,-90, 87,-80, 70,-57, 43,-26,  9},
  {  4,-13, 22,-30, 39,-47, 54,-61, 67,-73, 78,-82, 85,-88, 90,-90, 90,-90, 88,-85, 82,-78, 73,-67, 61,-54, 47,-39, 30,-22, 13, -4}
};
#endif
/*
const UChar g_aucChromaScale[52]=
{
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
  10,11,12,13,14,15,16,17,18,19,
  20,21,22,23,24,25,26,27,28,29,
  29,30,31,32,32,33,34,34,35,35,
  36,36,37,37,37,38,38,38,39,39,
  39,39
};
//*/

// ====================================================================================================================
// chroma
// ====================================================================================================================

const UChar g_aucConvertTxtTypeToIdx[4] = { 0, 1, 1, 2 };

// ====================================================================================================================
// Bit-depth
// ====================================================================================================================

UInt g_uiBitDepth     = 8;    // base bit-depth

UInt g_uiBASE_MAX     = 255;  // max.

// ====================================================================================================================
// Misc.
// ====================================================================================================================

Char  g_aucConvertToBit  [ MAX_CU_SIZE+1 ];

// ====================================================================================================================
// Scanning order & context model mapping
// ====================================================================================================================
#if CODEFFCODER_LDW_WLQ_YQH

UInt* g_auiFrameScanXY[ MAX_CU_DEPTH +12 ];//yuquanhe@hisilicon.com
UInt* g_auiFrameScanX [ MAX_CU_DEPTH +12 ];//yuquanhe@hisilicon.com
UInt* g_auiFrameScanY [ MAX_CU_DEPTH +12 ];//yuquanhe@hisilicon.com
UInt* g_auiFrameinvScanXY [ 4 ];//yuquanhe@hisilicon.com
const UInt g_auiIntraModeClassified[NUM_INTRA_MODE] = {2, 2, 2, 1, 1, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 0}; //lidaowen yuquanhe@hisilicon.com
#else
// scanning order table
UInt* g_auiFrameScanXY[ MAX_CU_DEPTH  ];
UInt* g_auiFrameScanX [ MAX_CU_DEPTH  ];
UInt* g_auiFrameScanY [ MAX_CU_DEPTH  ];
#endif
// scanning order to 8x8 context model mapping table
UInt  g_auiAntiScan8  [64];

// initialize g_auiFrameScanXY
Void initFrameScanXY( UInt* pBuff, UInt* pBuffX, UInt* pBuffY, Int iWidth, Int iHeight )
{
  Int x, y, c = 0;
  
  // starting point
  pBuffX[ c ] = 0;
  pBuffY[ c ] = 0;
  pBuff[ c++ ] = 0;
  
  // loop
  x=1; y=0;
  while (1)
  {
    // decrease loop
    while ( x>=0 )
    {
      if ( x >= 0 && x < iWidth && y >= 0 && y < iHeight )
      {
        pBuffX[ c ] = x;
        pBuffY[ c ] = y;
        pBuff[ c++ ] = x+y*iWidth;
      }
      x--; y++;
    }
    x=0;
    
    // increase loop
    while ( y>=0 )
    {
      if ( x >= 0 && x < iWidth && y >= 0 && y < iHeight )
      {
        pBuffX[ c ] = x;
        pBuffY[ c ] = y;
        pBuff[ c++ ] = x+y*iWidth;
      }
      x++; y--;
    }
    y=0;
    
    // termination condition
    if ( c >= iWidth*iHeight ) break;
  }
  
  // LTR_2D_CONTEXT_MAPPING
  if (iWidth == 8 && iHeight == 8)
  {
    for( c = 0; c < iWidth*iHeight; c++)
    {
      g_auiAntiScan8[pBuff[c]] = c;
    }
  }
}

Void initSigLastScanPattern( UInt* puiScanPattern, const UInt uiLog2BlockSize, const bool bDownLeft )
{
  const int   iBlockSize    = 1 << uiLog2BlockSize;
  const UInt  uiNumScanPos  = UInt( iBlockSize * iBlockSize );
  UInt        uiNextScanPos = 0;
  
  for( UInt uiScanLine = 0; uiNextScanPos < uiNumScanPos; uiScanLine++ )
  {
    int    iPrimDim  = int( uiScanLine );
    int    iScndDim  = 0;
    while( iPrimDim >= iBlockSize )
    {
      iScndDim++;
      iPrimDim--;
    }
    while( iPrimDim >= 0 && iScndDim < iBlockSize )
    {
      puiScanPattern[ uiNextScanPos++ ] = ( bDownLeft ? iScndDim * iBlockSize + iPrimDim : iPrimDim * iBlockSize + iScndDim );
      iScndDim++;
      iPrimDim--;
    }
  }
  return;
}

#if CODEFFCODER_LDW_WLQ_YQH
Void initZigzagScan_NSQT( UInt* pBuff, UInt* pBuffX, UInt* pBuffY, Int iWidth, Int iHeight )
{
	Int x, y, c = 0;

	// starting point
	pBuffX[ c ] = 0;
	pBuffY[ c ] = 0;
	pBuff[ c++ ] = 0;

	// loop
	x = 1;
	y = 0;
	while (1) {
		// decrease loop
		while (x >= 0) {
			if (x >= 0 && x < iWidth && y >= 0 && y < iHeight) {
				pBuffX[ c ] = x;
				pBuffY[ c ] = y;
				pBuff[ c++ ] = x+y*iWidth;
			}
			x--;
			y++;
		}
		x = 0;

		// increase loop
		while (y >= 0) {
			if (x >= 0 && x < iWidth && y >= 0 && y < iHeight) {
				pBuffX[ c ] = x;
				pBuffY[ c ] = y;
				pBuff[ c++ ] = x+y*iWidth;
			}
			x++;
			y--;
		}
		y = 0;

		// termination condition
		if (c >= iWidth * iHeight) {
			break;
		}
	}


}


Void initZigzagInvScan( UInt* pBuff, UInt* pBuffZigzag, Int iWidth, Int iHeight )
{
	for(Int y=0;y<iHeight;y++)
     for(Int x=0;x<iWidth;x++)
	 {
		 pBuff[pBuffZigzag[x+y*iWidth]]=x+y*iWidth;
	 }
	 return;
}
Void initCGZigzagScanXY( UInt* pBuff, UInt* pBuffX, UInt* pBuffY, Int iWidth, Int iHeight )
{
	Int i, j, k = 0;
	UInt* pBufftemp ; 
	pBufftemp= new UInt[ (iWidth / 4)*(iHeight / 4) ];
	UInt* pBuffXtemp; 
	pBuffXtemp= new UInt[ (iWidth / 4)*(iHeight / 4) ];; 
	UInt* pBuffYtemp ;
	pBuffYtemp= new UInt[ (iWidth / 4)*(iHeight / 4) ];;

	// starting point
	pBuffX[ 0 ] = 0;
	pBuffY[ 0 ] = 0;
	pBuff[ 0 ] = 0;

	initZigzagScan_NSQT(pBufftemp, pBuffXtemp,pBuffYtemp,iWidth / 4, iHeight / 4); //for AVS2
	for (i = 1; i < iWidth * iHeight ; i++) {
		j = i / 16;
		k = i % 16;
		pBuffX[i] = pBuffXtemp[j] * 4 + g_auiFrameScanX[2][k];
		pBuffY[i] = pBuffYtemp[j] * 4 + g_auiFrameScanY[2][k];
		pBuff[i] = pBuffX[i] +pBuffY[i] *iWidth;
	}
	//yuquanhe@hisilicon.com
	delete[] pBufftemp;
	delete[] pBuffXtemp;
	delete[] pBuffYtemp;

}

#endif
#if wlq_WQ
const UInt g_scalingListSize[SCALING_LIST_SIZE_NUM] = { 16, 64, 256, 1024 };

const Int g_scaleM[4][4] = {
	{ 32768, 32768, 32768, 32768 },
	{ 32768, 32768, 32768, 32768 },
	{ 32768, 32768, 32768, 32768 },
	{ 32768, 32768, 32768, 32768 }
};

const Short g_wq_param_default[2][6] = {
	{ 67, 71, 71, 80, 80, 106 },
	{ 64, 49, 53, 58, 58, 64 }
};

const int g_LambdaQPTab[3][3] = {       //M2331 2008-04
	//      F   U   D
	0, 0, -2,
	-2, -1, 0,
	-1, 0, 0
};


const  double g_LambdaFTab[3][3] = {    //M2331 2008-04
	//  F     U     D
	0.68, 0.70, 0.60,
	0.62, 0.68, 0.60,
	0.62, 0.70, 0.85
};

const UChar g_WeightQuantModel8x8[4][64] = {
	//   l a b c d h
	//   0 1 2 3 4 5
	{
		// Mode 0
		0, 0, 0, 4, 4, 4, 5, 5,
		0, 0, 3, 3, 3, 3, 5, 5,
		0, 3, 2, 2, 1, 1, 5, 5,
		4, 3, 2, 2, 1, 5, 5, 5,
		4, 3, 1, 1, 5, 5, 5, 5,
		4, 3, 1, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5
	},
	{
		// Mode 1
		0, 0, 0, 4, 4, 4, 5, 5,
		0, 0, 4, 4, 4, 4, 5, 5,
		0, 3, 2, 2, 2, 1, 5, 5,
		3, 3, 2, 2, 1, 5, 5, 5,
		3, 3, 2, 1, 5, 5, 5, 5,
		3, 3, 1, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5
	},
	{
		// Mode 2
		0, 0, 0, 4, 4, 3, 5, 5,
		0, 0, 4, 4, 3, 2, 5, 5,
		0, 4, 4, 3, 2, 1, 5, 5,
		4, 4, 3, 2, 1, 5, 5, 5,
		4, 3, 2, 1, 5, 5, 5, 5,
		3, 2, 1, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5
	},
	{
		// Mode 3
		0, 0, 0, 3, 2, 1, 5, 5,
		0, 0, 4, 3, 2, 1, 5, 5,
		0, 4, 4, 3, 2, 1, 5, 5,
		3, 3, 3, 3, 2, 5, 5, 5,
		2, 2, 2, 2, 5, 5, 5, 5,
		1, 1, 1, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5
	}
};

const UChar g_WeightQuantModel4x4[4][16] = {
	//   l a b c d h
	//   0 1 2 3 4 5
	{
		// Mode 0
		0, 4, 3, 5,
		4, 2, 1, 5,
		3, 1, 1, 5,
		5, 5, 5, 5
	},
	{
		// Mode 1
		0, 4, 4, 5,
		3, 2, 2, 5,
		3, 2, 1, 5,
		5, 5, 5, 5
	},
	{
		// Mode 2
		0, 4, 3, 5,
		4, 3, 2, 5,
		3, 2, 1, 5,
		5, 5, 5, 5
	},
	{
		// Mode 3
		0, 3, 1, 5,
		3, 4, 2, 5,
		1, 2, 2, 5,
		5, 5, 5, 5
	}
};


#endif
#if wlq_FME
Pel line[64];
const Int g_blk_size_AVS2[20][2] = {
	{ MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE * 2 },
	{ MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE * 2 },
	{ MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE * 2 },
	{ MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE * 2 },
	{ MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE },
	{ MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE },
	{ MIN_BLOCK_SIZE, MIN_BLOCK_SIZE * 2 },
	{ MIN_BLOCK_SIZE, MIN_BLOCK_SIZE * 2 },
	{ MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE / 2 },
	{ MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE * 3 / 2 },
	{ MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE * 3 / 2 },
	{ MIN_BLOCK_SIZE * 2, MIN_BLOCK_SIZE / 2 },
	{ MIN_BLOCK_SIZE / 2, MIN_BLOCK_SIZE * 2 },
	{ MIN_BLOCK_SIZE * 3 / 2, MIN_BLOCK_SIZE * 2 },
	{ MIN_BLOCK_SIZE * 3 / 2, MIN_BLOCK_SIZE * 2 },
	{ MIN_BLOCK_SIZE / 2, MIN_BLOCK_SIZE * 2 },
	{ MIN_BLOCK_SIZE, MIN_BLOCK_SIZE },
	{ MIN_BLOCK_SIZE, MIN_BLOCK_SIZE },
	{ MIN_BLOCK_SIZE, MIN_BLOCK_SIZE },
	{ MIN_BLOCK_SIZE, MIN_BLOCK_SIZE }
};

const Int quant_coef_AVS2[6][4][4] = {
	{ { 13107, 8066, 13107, 8066 }, { 8066, 5243, 8066, 5243 }, { 13107, 8066, 13107, 8066 }, { 8066, 5243, 8066, 5243 } },
	{ { 11916, 7490, 11916, 7490 }, { 7490, 4660, 7490, 4660 }, { 11916, 7490, 11916, 7490 }, { 7490, 4660, 7490, 4660 } },
	{ { 10082, 6554, 10082, 6554 }, { 6554, 4194, 6554, 4194 }, { 10082, 6554, 10082, 6554 }, { 6554, 4194, 6554, 4194 } },
	{ { 9362, 5825, 9362, 5825 }, { 5825, 3647, 5825, 3647 }, { 9362, 5825, 9362, 5825 }, { 5825, 3647, 5825, 3647 } },
	{ { 8192, 5243, 8192, 5243 }, { 5243, 3355, 5243, 3355 }, { 8192, 5243, 8192, 5243 }, { 5243, 3355, 5243, 3355 } },
	{ { 7282, 4559, 7282, 4559 }, { 4559, 2893, 4559, 2893 }, { 7282, 4559, 7282, 4559 }, { 4559, 2893, 4559, 2893 } }
};

#if DMH
const int dmh_pos[DMH_MODE_NUM + DMH_MODE_NUM - 1][2][2] = {
	{ { 0, 0 }, { 0, 0 } },
	{ { -1, 0 }, { 1, 0 } },
	{ { 0, -1 }, { 0, 1 } },
	{ { -1, 1 }, { 1, -1 } },
	{ { -1, -1 }, { 1, 1 } },
	{ { -2, 0 }, { 2, 0 } },
	{ { 0, -2 }, { 0, 2 } },
	{ { -2, 2 }, { 2, -2 } },
	{ { -2, -2 }, { 2, 2 } }
};
#endif

#endif