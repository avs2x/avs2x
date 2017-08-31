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

/** \file     ContextTables.h
    \brief    Defines constants and tables for SBAC
    \todo     number of context models is not matched to actual use, should be fixed
*/

#ifndef __CONTEXTTABLES__
#define __CONTEXTTABLES__

// ====================================================================================================================
// Constants
// ====================================================================================================================
#if YQH_INTRA
#if YQH_SPLIGFLAG_BUG
#define NUM_SPLIT_FLAG_CTX              8       ///< number of context models for split flag  /* in OV */ //yuquanhe@hisilicon.com
#else
#if WLQ_CUSplitFlag
#define NUM_SPLIT_FLAG_CTX              7
#else
#define NUM_SPLIT_FLAG_CTX              6       ///< number of context models for split flag  /* in OV */ //yuquanhe@hisilicon.com
#endif
#endif
#else
#define NUM_SPLIT_FLAG_CTX            1       ///< number of context models for split flag  /* in OV */
#endif
#define NUM_PRED_MODE_CTX             2       ///< number of context models for prediction mode
#if INTER_GROUP
#if inter_intra_1
#define NUM_PART_SIZE_CTX             20
#else
#define NUM_PART_SIZE_CTX             6
#endif
#define NUM_SHAPE_OF_PARTITION_CTX    2
#else
#define NUM_PART_SIZE_CTX             4
#endif

#if WRITE_INTERDIR
#define NUM_INTER_DIR_CTX             18
#define NUM_MIN_INTER_DIR_CTX         2
#else
#if rd_sym
#define NUM_INTER_DIR_CTX             3
#else
#define NUM_INTER_DIR_CTX             2       ///< number of context models for inter prediction direction  /* in OV */
#endif
#endif


#define NUM_INTER_WSM_CTX             4       ///< number of context models for inter prediction direction  /* in OV */

#if F_MHPSKIP_SYC || B_MHBSKIP_SYC
#define NUM_INTER_MHPSKIP_CTX         4       ///< number of context models for F MHPSkip  /* in OV */
#endif


#if F_DHP_SYC
#define NUM_INTER_DHP_CTX             3       ///< number of context models for dhp prediction direction  /* in OV */
#define NUM_INTER_DHP_NXN_CTX         1       ///< number of context models for B8x8 dhp prediction direction  /* in OV */
#endif

#if YQH_INTRA
#define NUM_INTRA_DIR_CTX             10       ///< number of context models for inter prediction direction  /* in OV */ //yuquanhe
#else
#define NUM_INTRA_DIR_CTX             2       ///< number of context models for inter prediction direction  /* in OV */
#endif
#if RPS //3-22 ZP
#define NUM_REF_NO_CTX                6       ///< number of context models for reference index
#endif
#if DMH
#define NUM_DMH_MODE_CTX              15
#endif
#define NUM_MV_RES_CTX                5       ///< number of context models for motion vector difference    /* in OV */
#if niu_write_cbf
#define NUM_QT_CBF_CTX                5       ///< number of context models for QT CBF /* in OV */
#else
#define NUM_QT_CBF_CTX                3       ///< number of context models for QT CBF /* in OV */
#endif
#define NUM_QT_ROOT_CBF_CTX           1       ///< number of context models for QT ROOT CBF /* in OV */
#define NUM_CBF_CTX                   4       ///< number of context models for CBF
#define NUM_RUN_CTX                   12      ///< number of context models for zero-run
#define NUM_LAST_FLAG_CTX             1       ///< number of context models for last flag
#define NUM_ABS_CTX                   12      ///< number of context models for magnitude
#define NUM_DBK_IDX_CTX               2

#define NUM_CU_X_POS_CTX              2       ///< number of context models for partition size (AMP)
#define NUM_CU_Y_POS_CTX              2       ///< number of context models for partition size (AMP)

#if YQH_INTRA
#define NUM_TRANSSPLIT_FLAG_CTX         3       ///< number of context models for split flag  /* in OV */ //yuquanhe@hisilicon.com
#define NUM_TNTRA_PU_TYPE_CTX           1       ///< number of context models for split flag  /* in OV */ //yuquanhe@hisilicon.com
//#else
//#define NUM_INTRA_DIR_CTX             2       ///< number of context models for inter prediction direction  /* in OV */
//#define NUM_SPLIT_FLAG_CTX            1       ///< number of context models for split flag  /* in OV */
#endif

#if ZHOULULU_RDOQ
#define NUM_MAP_CTX                  17
#define NUM_LAST_CG_CTX_LUMA         12
#define NUM_LAST_CG_CTX_CHROMA       6
#define NUM_SIGCG_CTX_LUMA           2
#define NUM_SIGCG_CTX_CHROMA         1
#define NUM_LAST_POS_CTX_LUMA        56
#define NUM_LAST_POS_CTX_CHROMA      16
#endif


#if CODEFFCODER_LDW_WLQ_YQH

#define NUM_LAST_CG_CTX               12   //yuquanhe@hisilicon.com
#define NUM_SIGCG_CTX                 2    //yuquanhe@hisilicon.com
#define NUM_LAST_POS_INCG_CTX         56   //yuquanhe@hisilicon.com
#define NUM_LEVEL_RUN_INCG_CTX        136  //yuquanhe@hisilicon.com

#endif



#endif

