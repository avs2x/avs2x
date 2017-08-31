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

/** \file     CommonDef.h
    \brief    Defines constants, macros and tool parameters
*/

#ifndef __COMMONDEF__
#define __COMMONDEF__

// this pragma can be used for turning off "signed and unsigned mismatch"
#if _MSC_VER > 1000
#pragma warning( disable : 4018 )
#endif // _MSC_VER > 1000
#include "TypeDef.h"
#include "TComRom.h"

// ====================================================================================================================
// Version information
// ====================================================================================================================

#define VERSION        "1.0"                 ///< Current software version

// ====================================================================================================================
// Platform information
// ====================================================================================================================

#ifdef __GNUC__
#define COMPILEDBY  "[GCC %d.%d.%d]", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__
#ifdef __IA64__
#define ONARCH    "[on 64-bit] "
#else
#define ONARCH    "[on 32-bit] "
#endif
#endif

#ifdef __INTEL_COMPILER
#define COMPILEDBY  "[ICC %d]", __INTEL_COMPILER
#elif  _MSC_VER
#define COMPILEDBY  "[VS %d]", _MSC_VER
#endif

#ifndef COMPILEDBY
#define COMPILEDBY "[Unk-CXX]"
#endif

#ifdef _WIN32
#define ONOS        "[Windows]"
#elif  __linux
#define ONOS        "[Linux]"
#elif  __CYGWIN__
#define ONOS        "[Cygwin]"
#elif __APPLE__
#define ONOS        "[Mac OS X]"
#else
#define ONOS "[Unk-OS]"
#endif

#define OS_BITS          "[%d bit] ", (sizeof(void*) == 8 ? 64 : 32) ///< used for checking 64-bit O/S

#ifndef NULL
#define NULL              0
#endif

// ====================================================================================================================
// Common constants
// ====================================================================================================================

#define _SUMMARY_OUT_               0           ///< print-out PSNR results of all pictures to summary.txt
#define _SUMMARY_PIC_               0           ///< print-out PSNR results for each picture type to summary.txt

#define MAX_GOP                     64          ///< max. value of hierarchical GOP size

#define MAX_UINT                    0xFFFFFFFFU ///< max. value of unsigned 32-bit integer
#define MAX_INT                     2147483647  ///< max. value of signed 32-bit integer
#define MAX_DOUBLE                  1.7e+308    ///< max. value of double-type value

#define MIN_QP                      0
#define MAX_QP                      127 //QUANT_PATENT_FREE

#define NOT_VALID                   -1
#if wlq_Mv_check
#define BASELINE_PROFILE_AVS2         32  //0x20
#define BASELINE10_PROFILE_AVS2       34  //0x22
#endif
// ====================================================================================================================
// Macro functions
// ====================================================================================================================
#if ZHOULULU_RDOQ
#define SWAP(x,y)                {(y)=(y)^(x);(x)=(y)^(x);(y)=(x)^(y);}
#endif
#define Max(x, y)                   ((x)>(y)?(x):(y))                                                 ///< max of (x, y)
#define Min(x, y)                   ((x)<(y)?(x):(y))                                                 ///< min of (x, y)
#define Median(a,b,c)               ((a)>(b)?(a)>(c)?(b)>(c)?(b):(c):(a):(b)>(c)?(a)>(c)?(a):(c):(b)) ///< 3-point median
#define Clip(x)                     ( Min(g_uiBASE_MAX, Max( 0, (x)) ) )                              ///< clip with bit-depth range
#define Clip3( MinVal, MaxVal, a)   ( ((a)<(MinVal)) ? (MinVal) : (((a)>(MaxVal)) ? (MaxVal) :(a)) )  ///< general min/max clip

#define DATA_ALIGN                  1                                                                 ///< use 32-bit aligned malloc/free
#if     DATA_ALIGN && _WIN32 && ( _MSC_VER > 1300 )
#define xMalloc( type, len )        _aligned_malloc( sizeof(type)*(len), 32 )
#define xFree( ptr )                _aligned_free  ( ptr )
#else
#define xMalloc( type, len )        malloc   ( sizeof(type)*(len) )
#define xFree( ptr )                free     ( ptr )
#endif

#define FATAL_ERROR_0(MESSAGE, EXITCODE)                      \
{                                                             \
  printf(MESSAGE);                                            \
  exit(EXITCODE);                                             \
}

// ====================================================================================================================
// Coding tool configuration
// ====================================================================================================================

// Lambda control
#define HB_LAMBDA_FOR_LDC           1           ///< use of B-style lambda for non-key pictures in low-delay mode

// Fast estimation of generalized B in low-delay mode
#define GPB_SIMPLE                  1           ///< Simple GPB mode
#if     GPB_SIMPLE
#define GPB_SIMPLE_UNI              1           ///< Simple mode for uni-direction
#endif

// Fast ME using smoother MV assumption
#define FASTME_SMOOTHER_MV          1           ///< reduce ME time using faster option

// Adaptive search range depending on POC difference
#define ADAPT_SR_SCALE              1           ///< division factor for adaptive search range

// Interpolation filter
#define CHROMA_INTERP_CLIP          1           ///< clip for chroma interpolation filter

// Transform
#define MATRIX_MULT                 0           ///< matrix multiplication form of core transform
#define HIGH_ACC_LOT_SCALE          1           ///< high-accuracy lot scaling

//////////////////AVS 3 group///////////////////


 #if ENUM_AND_MACRO_YUQUANHE

//CUSIZE
#define MIN_CU_SIZE                  8
//#define MAX_CU_SIZE                64 //Reference TComRom.h
#define	MIN_BLOCK_SIZE               4
#define	MIN_CU_SIZE_IN_BIT           3
#define	MIN_BLOCK_SIZE_IN_BIT        2
#define	BLOCK_MULTIPLE              (MIN_CU_SIZE/(MIN_BLOCK_SIZE))
#define	MAX_CU_SIZE_IN_BIT           6
#define	B4X4_IN_BIT                  2
#define	B8X8_IN_BIT                  3
#define	B16X16_IN_BIT                4
#define	B32X32_IN_BIT                5
#define	B64X64_IN_BIT                6
	//SAO
#define	NUM_BO_OFFSET               32
#define	MAX_NUM_SAO_CLASSES         32
#define	NUM_SAO_BO_CLASSES_LOG2      5
#define	NUM_SAO_BO_CLASSES_IN_BIT    5
#define	MAX_DOUBLE                1.7e+308
#define	NUM_SAO_EO_TYPES_LOG2        2
#define	NUM_SAO_BO_CLASSES        (1<<NUM_SAO_BO_CLASSES_LOG2)
#define	SAO_RATE_THR                0.75
#define	SAO_RATE_CHROMA_THR          1
#define	SAO_SHIFT_PIX_NUM            4
#define	SAO_PARA_CROSS_SLICE         1
#define	SAO_MULSLICE_FTR_FIX         1
	//INTRA PREDICTION
#define	NUM_INTRA_MODE              33
#define INTRA_NUM_FULL_RD           9
#define	INTRA_NUM_CHROMA             5

#define	INTRA_DC_IDX                 0  ///< index for
#define INTRA_PLANAR_IDX             1
#define INTRA_BI_IDX                2 ///< index for
#define INTRA_VER_IDX                12///< index for 
#define INTRA_HOR_IDX               24 ///< index for 

#define INTRA_NUM_CHROMA             5 ///< total 
#define INTRA_DM_PRED_CHROMA         0
#define	INTRA_DC_PRED_CHROMA         1
#define	INTRA_HOR_PRED_CHROMA        2
#define	INTRA_VERT_PRED_CHROMA       3
#define	INTRA_BI_PRED_CHROMA         4

#define MAX_NUM_REF_PICS            7     
#if RPS
#define MAX_NUM_REM_PICS            4
#endif
///< max. number of pictures used for reference
#define MAX_NUM_OF_RCS              32     
///< max. number of reference configuration sets
#if AVS3_EXTENSION_DEBUG_SYC
#define MAX_TEMPORAL_ID             7     
///< max. number of temporal id
#endif
#endif
#endif // end of #ifndef  __COMMONDEF__
