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

/** \file     TypeDef.h
\brief    Define basic types, new types and enumerations
*/

#ifndef _TYPEDEF__
#define _TYPEDEF__


//////////////////////////////////

//////////////////////////////////
//

#define F_L1_FOR_DHP_SYC            1
#define F_L1_FOR_MHPSKIP_SYC        1
/*#define F_L1_FOR_WSM_SYC            1*/
#define TEMP_FAST_DHP               1//DHP的亚像素搜索暂时调小，便于测试

#define CLOSE_DESTROY_WHEN_DEC      0//已解决，强制为0，整理时可删
#define F_DEBUG_828                 1//调试结束需要打开

#define ZP_DEBUG_828                1//修改代码书写问题
#define ZP_DEBUG_829                1//解决解码crash问题
#define REFLISTBUG                  1 //增加F帧后向链
#define POC_256_BUG                 0 //暂时关掉，还没验证



#define B_MHBSKIP_SYC               0
#define B_SKIP_ZP                   1

#define F_MHPSKIP_SYC               1
#define F_MHPSKIP_SYC_FIXED_MV      0//固定MHPSKIP的MV值
#define F_MHPSKIP_SYC_DEBUG         1
#define F_MHPSKIP_SYC_DEBUG_2       1

#define F_DUAL_DEBUG                1


#define F_DHP_SYC                   1
#define F_DHP_SYC_OP                1//20170821
#define F_DHP_SYC_OP_2              1//20170824

#define F_DHP_SYC_MVD_DEBUG         1
#define F_DHP_SYC_REF_DEBUG         1

#define F_DHP_SYC_NSQT              1


#define debug_dir                  0
#define rd_bipred                  1
#define B_ME_REVISE                1
#define rd_bipred_YQH              1
#define rd_mvd                     1
#define rd_sym                     1
#define rd_mvd_yqh                 1
#define WRITE_INTERDIR             1
#define WLQ_Interdir_bug           1
#define YQH_B_INTER                1


#define DIFSym_bug                       1

#define compensation_bug                 1
#define inter_direct_skip_bug            1
#define inter_direct_skip_bug1            1
#define inter_direct_skip_bug2            1
#define inter_direct_skip_bug3            1
#define inter_direct_skip_bug4            1

#define inter_intra_1                    1
#define inter_intra_2                     1
#define inter_intra_3                     1

#define SPS_BUG_YU                 1




#define mvd_revise                  0

#define FME_SUB_ON                  1

#if FME_SUB_ON
#define chazhi_bug                  1     // SubPel 时打开 本来是1，有bug
#define subsearch_bug               1     // SubPel 时打开
#define FME_SUB						1
#define FME_SUB_BUG					1
#define niu_CloseSubPel						0   // SubPel 时关闭
#define niu_CloseFastSubPel					0

#else
#define chazhi_bug                  0     // SubPel 时打开
#define subsearch_bug               0     // SubPel 时打开
#define FME_SUB						0
#define FME_SUB_BUG					0
#define niu_CloseSubPel						1   // SubPel 时关闭
#define niu_CloseFastSubPel					1

#endif
#define wlq_mem_bug						1

#define WLQ_set_bug					1
#define WLQ_debug                   0  //test
#define WLQ_2NxnU                   1

#define RD_PMVR                     0  // 之前为0
//
#define niu_multimode				 1
#define niu_NSQT_BUG                1

#define niu_cutype_revise			1

#define niu_cutype_dec				1
#define niu_cbf_dec					1
#define niu_predmode_dec			1
#define niu_mvd_dec					1

#define niu_mvpred_debug			1
#define niu_LeftPU_revise			1
#define niu_PredInterChroma_Bug		1
#define niu_NSQT_revise				1         // NSQT

#define niu_CbpBit					0         // intra: a b
#define niu_IntraCbp_revise			0		  // a b   同时打开
#define niu_NSQT_dec				1

#define ZY_MODIFY_CBF				        1
#define ZY_MODIFY_DATA_STRUCTURE	        1
#define ZHANGYI_INTRA_SDIP                  1
#define ZY_MODIFY_COEFF_CODING		        1
#define ZHANGYI_INTRA_MODIFY_TRANS	        1
#define ZY_INTRA_MODIFY_TUSIZE              1
#define ZHANGYI_INTRA_SDIP_BUG_YQH          1
#define ZHANGYI_MODIFY_CTX_OF_SPLITFLAG     1


#define AVS3_FANGWEIQISHIMA                 0

#define wlq_avs2x_debug				1
#define rdot_bug					1
#define wlq_inter_tu_rdo_debug      1

#define DEBUG_ME        0   //ME debug打印,独立宏   wlq
#define print_rd        0	 //ME debug打印,独立宏   wlq
#define print_key       0

#define wlq_WQ			0    //加权量化,独立宏
#define wlq_WQ_q		0    //非独立宏

#define wlq_FME         1    //独立宏		//始终打开，全搜索时不影响
#define wlq_FME_q       1    //非独立宏		//始终打开，全搜索时不影响
#define wlq_FME_p       0

#define wlq_FME_nomvp	0   // FME MVP

#define YUVMargin_debug		1    //inter   增大内存，设置为一个较大定值   // + 1024
#define wlq_Mv_check		1
#define wlq_Mv_check_temp   1
#define wlq_set_mvd			0
#define wlq_pred_clip		1

#define WLQ_noCUtype          0  //绝对不写CUtype   //test
#define WLQ_set_CBP           0  //帧内写CBP，帧间不写 与宏AVS3_DEBUG_NOCBP关联  //test
#define WLQ_nocoeff           1  //不写系数
#define WLQ_coeff_debug       1  //系数bug

#define WLQ_HALFCTU_BUG		1

#define YQH_CUTYPE_BUG                      1
#define WLQ_CUTYPE_BUG						1

#define AVS3_close_CUSplitFlag				0

#define AVS3_Intra_print_Luma				0
#define AVS3_Intra_print_Chroma				0
#define WLQ_CUSplitFlag                     1
#define WLQ_CUSplitFlag_inter               1
#define WLQ_inter_bug						1 
#define WLQ_inter_Chroma_TU                 1
#define WLQ_CUcost_print                    0
#define terimating_1                        1
#define WLQ_rdcost                          1
#define WLQ_RPS_debug                       1

#define WLQ_intra_Chroma_ctx_BUG            0      ///bug!!!!

#define del_rec_0                           1


#define DEBUG_MV        0    //打开，全搜索和FMEmvp=0
//***************************************************************************************

#define  WLQ_BITS_COUNTER_INTER             1
#define WLQ_CLOSE_REF_COST_FOR_DEBUG		1
#define REF_BUG								1
// ====================================================================================================================
// Inter prediction of AVS2
// ====================================================================================================================
#define RPS                         1
#define RPS_BUG                     1
#define FULLSEARCHFAST                        0     //可以加快fullsearch 速度40%
#define INTERBUG                    1   //zp
#define INTERTEST                   1

#define PREDINFO                    1

#define DPB                         1   //解码缓存
#if DPB
#define   MV_DECIMATION_FACTOR      4
#endif

#define PSKIP                       1
#define BSKIP                       1
#define PBSKIP_BUG_YQH              0



#define YQH_INTRA_LAMDA_DUG             0

#define B_RPS                       1
#define B_RPS_BUG                   1
#define B_RPS_BUG_814               1

#define B_RPS_BUG_815                  1
#define F_RPS                          1
#define LAMBDA_BUG                     1
#define RPS_BUG_815                    1

#define RPS_BUG_816                    0

#define BUG_815                        1
#define BUG_816                        1
#define B_RPS_BUG_818                  1
#define B_RPS_BUG_819                  1
#define BUG_819                        1
#define RPS820                         1
#define B_RPS_BUG_820                  1

#define B_RPS_BUG_821                  1
#define IDR                            1

#define RPS_BUG_YQH                    1
#define RPS_BUG_YQH1                    1
#define DIRECTSKIP_BUG_YQH             1

#if BSKIP
#define DIRECTION                4
#endif
#if B_MHBSKIP_SYC
#define DS_FORWARD               4
#define DS_BACKWARD              2
#define DS_SYM                   3
#define DS_BID                   1
#endif

#if PSKIP
#define MH_PSKIP_NUM             4
#define NUM_OFFSET               0
#define BID_P_FST                1
#define BID_P_SND                2
#define FW_P_FST                 3
#define FW_P_SND                 4
#endif

#define INTER611                    1
#define INTER612                    1
#define INTER613                    1
#define INTER614                    1


#define H_QP                        1
#define LAMBDA                      1
#if LAMBDA
#define LAM_2Level_TU                 0.8    //lambda
#define SHIFT_QP                      11
#define  LAMBDA_ACCURACY_BITS         16
#define  LAMBDA_FACTOR(lambda)        ((int)((double)(1<<LAMBDA_ACCURACY_BITS)*lambda+0.5))
#endif
#define SEARCHPATCH                   1

#if INTER614
#define  WEIGHTED_COST(factor,bits)   (((factor)*(bits))>>LAMBDA_ACCURACY_BITS)
#define  REF_COST_FWD(f,ref)          (WEIGHTED_COST(f,((img->num_ref_pic_active_fwd_minus1==0)? 0:refbits[(ref)])))
#endif


#define INTER_GROUP                   1
#define PU_ZY                         1
#define RD_DIRECT                     1
#define RD_MVP                        1
#define MVD                           1
#define DCTIF                         0
#define IF_RD                         1
#define DEBUG_BIT                     0
#define DEBUG_ONLY2Nx2N               1
#define DEBUG_ONLY2Nx2N_YQH               1

#define RDCOST                        1
#if RD_PMVR || FME_SUB
#define TH 2
#endif

#define absm(A) ((A)<(0) ? (-(A)):(A)) //!< abs macro, faster than procedure
#if RD_MVP
#define HALF_PIXEL_COMPENSATION_PMV 1
#define MV_SCALE 1
#define MV_CLIP 1
#define MV_RANG                            1 
#define REMOVE_UNUSED                      1 
#define HALF_PIXEL_COMPENSATION            1
#define FIX_MAX_REF	                 1

#define XY_MIN_PMV                   1
#if XY_MIN_PMV

#define B64X64_IN_BIT                6
#define B16X16_IN_BIT                4
#define B32X32_IN_BIT                5

#define MIN_CU_SIZE                  8
#define MIN_BLOCK_SIZE               4
#define MIN_CU_SIZE_IN_BIT           3
#define MIN_BLOCK_SIZE_IN_BIT        2

/* MV scaling 14 bit */
#define MULTI                        16384
#define HALF_MULTI                   8192
#define OFFSET                       14
#endif

#endif


// ====================================================================================================================
// Intra prediction of AVS2
// ====================================================================================================================

#define ZHANGYI_INTRA                    1 
#define ZHANGYI_INTRA_MODIFY             1
#define ZY_INTRA_LAMDA                   1  //modify the intra lambda of luma and chroma
#define ZY_INTRA_2X2MODIFY               1

#define ZY_DEBUG_PRINT                    0 //test, 系数编码前输出系数矩阵

#define  YQH_INTRA                       1
#define  YQH_INTRA_LAMDA                 1
#define  YQH_INTRA_NO2X2                 0
#define  CHROMA_QP_YQH                   1
#define  TANSFOR_BUG_YQH                 1
#define YQH_BUFF_RELEASE                 1
#define  YQH_INTRA_BUG                       1
#define  YQH_INTRA_SDIP                      1  //test  值为1。。 ??
#define  YQH_RDOINTRA_BUG                    1
#define  YQH_AVS3_DIR_RDO_BUG                1
#define  YQH_RDO_BITOURNT_BUG                1
#define  YQH_RDO_CHROMA_BUG                  1
#define  YQH_RDO_InitEstBits_BUG             1
#define  YQH_AEC_RDO_BUG                     1
#define  UV_NO_SPLIT_TRANSFROM               1

#define  YQH_AVS2_BITSTREAM                 0
#define RD_NSQT                             1  //1
#define RD_NSQT_BUG                         1  //1

#define RD_NSQT_BUG_YQH                     1  //1

#define YQH_SYS                             1

#define intra_premode_debug_YQH			   1

#define niu_set_intra_premode_debug			0
#define niu_write_cbf						1
#define niu_intercbf_revise			        1
#define AVS3_DEBUG_NOCBP                    1
#define niu_SplitFlag                       1

#define niu_terimating				1   
#define niu_state_test				1 
#define niu_coeff					1
#define niu_ChromaRDO_revise		1 
#define niu_CHRDO_DEBUG				1
#define niu_getLeftCU				1
#define niu_rate_divide_2           1

#define ZY_INTRA_MODIFY_PRED              0
#define niu_intra_pavil                   1 
#define niu_intra_data_struct             1	
#define niu_IntraBiAdi_FIX                1 

#define CODEFFCODER_LDW_WLQ_YQH_BUG			1
#define CODEFFCODER_LDW_WLQ_YQH_BUG2		1
#define YQH_SPLIGFLAG_BUG                1

#define YQH_DEBUG_ZY_BUG                  0  //20170604

#define  WLQ_AEC_CODER                   1
#define YQH_SPLIGFLAG_DEC            1
#define YQH_SPLIGFLAG_DEC_BUG2            1

#if WLQ_AEC_CODER
//熵编码
#define wlq_AEC					         1					//独立宏！！！
#define wlq_vlc					         0					//ue、se		//独立宏！！！
#define wlq_AEC_r			             1				//不能设0
#define wlq_AEC_q			             1				//不能设0

#define wlq_bit_est				         1    //比特估计返回值为常数,可以关闭				独立宏！！！
#define wlq_FIX_BIT			             10   //比特估计返回值为常数的数值
//熵编码
#endif

#define CODEFFCODER_LDW_WLQ_YQH          1
// ====================================================================================================================
// Basic type redefinition
// ====================================================================================================================
#define ENUM_AND_MACRO_YUQUANHE     1
#define ZHOULULU_QT                 1 
#define ZHOULULU_SEC_TRANS          1



#define FIX_BUG                     1

#define AVS3_SEQ_HEADER_SYC                   1   ///<  avs2 sequence header
#define AVS3_DELETE_PPS_SYC                   1   ///<  delete PPS class
#define AVS3_PIC_HEADER_ZL                    1   ///<  avs2 pictuer header
#define AVS3_SLICE_HEADER_SYC                 1   ///<  avs2 slice header
#define AVS3_EXTENSION_LWJ		    	      1	  ///<  avs2 UserData, ExtSeqDisplay, ExtScalability
#define AVS3_EXTENSION_CYH                    1   ///<  avs2 copyright_extension,camera_parameters_extension
#define AVS3_EXTENSION_LYT			          1	  ///<  avs2 ExtMetadata, ExtRoiPra, ExtPicDispaly
#define AVS3_EXTENSION_DEBUG_SYC              1   ///<  avs2 Extention debug
#define AVS3_EXTENTION_OFF_SYC                1   ///<  avs2 Extention_off flag

#define DMH                                   1
#if     DMH
#define DMH_MODE_NUM 5
#endif


typedef       void                Void;
typedef       bool                Bool;

typedef       char                Char;
typedef       unsigned char       UChar;
#if DPB
typedef       signed char        SChar;
#endif
typedef       short               Short;
typedef       unsigned short      UShort;
typedef       int                 Int;
typedef       unsigned int        UInt;
typedef       long                Long;
typedef       unsigned long       ULong;
typedef       double              Double;
#if wlq_WQ
typedef       float               Float;
#endif
// ====================================================================================================================
// 64-bit integer type
// ====================================================================================================================

#ifdef _MSC_VER
typedef       __int64             Int64;

#if _MSC_VER <= 1200 // MS VC6
typedef       __int64             UInt64;   // MS VC6 does not support unsigned __int64 to double conversion
#else
typedef       unsigned __int64    UInt64;
#endif

#else

typedef       long long           Int64;
typedef       unsigned long long  UInt64;

#endif

// ====================================================================================================================
// Type definition
// ====================================================================================================================

typedef       UChar           Pxl;        ///< 8-bit pixel type
typedef       Short           Pel;        ///< 16-bit pixel type
typedef       Short           TCoeff;     ///< transform coefficient

/// parameters for adaptive loop filter
class TComPicSym;

/// parameters for deblocking filter
typedef struct _LFCUParam
{
	Bool bInternalEdge;                     ///< indicates internal edge
	Bool bLeftEdge;                         ///< indicates left edge
	Bool bTopEdge;                          ///< indicates top edge
} LFCUParam;

/// parameters for TENTM coefficient VLC
typedef struct _LastCoeffStruct
{
	int level;
	int last_pos;
} LastCoeffStruct;

#if CODEFFCODER_LDW_WLQ_YQH// 
struct CoeffCodingParam
{
	int* level;
	int* run;
	UChar* DCT_CGLastRun;//一个CG中最后一个RUN的个数
	UChar* DCT_CGFlag;//一个CG中是否有非0系数
	UChar* DCT_PairsInCG;//一个CG中有多少对RUN,LEVCEL对
};
#define NUM_LAST_POS_CTX_LUMA   56  //yuquanhe@hisilicon.com
#define NUM_LAST_POS_CTX_CHROMA 16  //yuquanhe@hisilicon.com

typedef enum {
	INTRA_PRED_VER = 0,
	INTRA_PRED_HOR,
	INTRA_PRED_DC_DIAG
};

#endif
// ====================================================================================================================
// Enumeration
// ====================================================================================================================

/// Nal
enum NalRefIdc
{
	NAL_REF_IDC_PRIORITY_LOWEST = 0,
	NAL_REF_IDC_PRIORITY_LOW,
	NAL_REF_IDC_PRIORITY_HIGH,
	NAL_REF_IDC_PRIORITY_HIGHEST
};

enum NalUnitType
{
	NAL_UNIT_UNSPECIFIED_0 = 0,
	NAL_UNIT_CODED_PICTURE,
	NAL_UNIT_CODED_PICTURE_DATAPART_A,
	NAL_UNIT_CODED_PICTURE_DATAPART_B,
	NAL_UNIT_CODED_PICTURE_DATAPART_C,
	NAL_UNIT_CODED_PICTURE_IDR,
	NAL_UNIT_SEI,
	NAL_UNIT_SPS,
	NAL_UNIT_PPS,
	NAL_UNIT_ACCESS_UNIT_DELIMITER,
	NAL_UNIT_END_OF_SEQUENCE,
	NAL_UNIT_END_OF_STREAM,
	NAL_UNIT_FILLER_DATA,
	NAL_UNIT_RESERVED_13,
	NAL_UNIT_RESERVED_14,
	NAL_UNIT_RESERVED_15,
	NAL_UNIT_RESERVED_16,
	NAL_UNIT_RESERVED_17,
	NAL_UNIT_RESERVED_18,
	NAL_UNIT_RESERVED_19,
	NAL_UNIT_RESERVED_20,
	NAL_UNIT_RESERVED_21,
	NAL_UNIT_RESERVED_22,
	NAL_UNIT_RESERVED_23,
	NAL_UNIT_UNSPECIFIED_24,
	NAL_UNIT_UNSPECIFIED_25,
	NAL_UNIT_UNSPECIFIED_26,
	NAL_UNIT_UNSPECIFIED_27,
	NAL_UNIT_UNSPECIFIED_28,
	NAL_UNIT_UNSPECIFIED_29,
	NAL_UNIT_UNSPECIFIED_30,
	NAL_UNIT_UNSPECIFIED_31,
	NAL_UNIT_INVALID,
};

//////////////////AVS 3 group///////////////////

#if ZHANGYI_INTRA

enum ComponentID
{
	COMPONENT_Y = 0,
	COMPONENT_Cb = 1,
	COMPONENT_Cr = 2,
	MAX_NUM_COMPONENT = 3
};

typedef enum PRED_BUF_E
{
	PRED_BUF_UNFILTERED = 0,
	PRED_BUF_FILTERED = 1,
	NUM_PRED_BUF = 2
} PRED_BUF;
#endif
/// supported picture type
enum PictureType
{
	I_PICTURE,
	P_PICTURE,
	B_PICTURE,
#if ENUM_AND_MACRO_YUQUANHE
	F_PICTURE,
	GB_PICTURE,
	BP_IMG_PICTURE
#endif
};

/// supported prediction type
enum PredMode
{
	MODE_SKIP,            ///< SKIP mode
#if ENUM_AND_MACRO_YUQUANHE
	MODE_DIRECT,            ///< SKIP mode
#endif
	MODE_INTER,           ///< inter-prediction mode
	MODE_INTRA,           ///< intra-prediction mode
	MODE_NONE = 15
};

#if wlq_WQ
enum ScalingListSize
{
	SCALING_LIST_4x4 = 0,
	SCALING_LIST_8x8,
	SCALING_LIST_16x16,
	SCALING_LIST_32x32,
	SCALING_LIST_SIZE_NUM
};

enum PredModeWQ
{
	MODE_INTER_WQ = 0,     ///< inter-prediction mode
	MODE_INTRA_WQ = 1,     ///< intra-prediction mode
	SCALING_LIST_NUM = 2,
};
#endif

#if ENUM_AND_MACRO_YUQUANHE
enum InterPredDir
{
	INTER_DUAL,            ///< 
	INTER_FORWARD,            ///< 
	INTER_BACKWARD,           ///< i
#if rd_mvd
	INTER_BID,
	INTER_SYM          ///< in
#else
	INTER_SYM,           ///< in
	INTER_BID
#endif
};
enum ChromaFormat
{
	CHROMA_400 = 0,
	CHROMA_420 = 1,
	CHROMA_422 = 2,
	CHROMA_444 = 3,
	NUM_CHROMA_FORMAT = 4
};

enum MVP_DIR
{
	MVPRED_xy_MIN = 0,
	MVPRED_L = 1,
	MVPRED_U = 2,
	MVPRED_UR = 3,
};
/// reference list index
enum RefPicList
{
	REF_PIC_LIST_0 = 0,   ///< reference list 0
	REF_PIC_LIST_1 = 1,   ///< reference list 1
	NUM_REF_PIC_LIST_01 = 2,
	REF_PIC_LIST_X = 100  ///< special mark
};
enum Intra_Neighbor_Available
{
	NEIGHBOR_INTRA_LEFT = 0,
	NEIGHBOR_INTRA_UP = 1,
	NEIGHBOR_INTRA_UP_RIGHT = 2,
	NEIGHBOR_INTRA_UP_LEFT = 3,
	NEIGHBOR_INTRA_LEFT_DOWN = 4
};


//sunyucheng zengli
namespace Profile
{
	enum Name
	{
		FORBIDEN = 0,
		BASELINE_PICTURE_PROFILE = 18,
		BASELINE_PROFILE = 32,
		BASELINE10_PROFILE = 34,
	};
};

namespace Level
{
	enum Name
	{
		//NOTE: RExt - code = (level * 30)
		FORBIDDEN = 0,
		LEVEL2_0_15 = 16,
		LEVEL2_0_30 = 18,
		LEVEL2_0_60 = 20,
		LEVEL4_0_30 = 32,
		LEVEL4_0_60 = 34,
		LEVEL6_0_30 = 64,
		LEVEL6_2_30 = 66,
		LEVEL6_0_60 = 68,
		LEVEL6_2_60 = 70,
		LEVEL6_0_120 = 72,
		LEVEL6_2_120 = 74,
		LEVEL8_0_30 = 80,
		LEVEL8_2_30 = 82,
		LEVEL8_0_60 = 84,
		LEVEL8_2_60 = 86,
		LEVEL8_0_120 = 88,
		LEVEL8_2_120 = 90,
		LEVEL10_0_30 = 96,
		LEVEL10_2_30 = 98,
		LEVEL10_0_60 = 100,
		LEVEL10_2_60 = 102,
		LEVEL10_0_120 = 104,
		LEVEL10_2_120 = 106,
	};
}


//sequence header
enum PictureTypeInSequence
{
	PTS_FRAME_SEQUENCE,
	FIELD_SEQUENCE,
};

//picture header
enum PictureStructure
{
	PS_SUCCESSIVELY,
	PS_BLEND,
};

//copyright extension
enum CopyrightType
{
	CT_COPY,
	CT_ORIGINAL,
};

//sequence_display_extension
enum SampleRange
{
	SR_FROM_ZERO,
	SR_FROM_16,
};
#endif

#if AVS3_PIC_HEADER_ZL////

struct LFCUSyntax
{
	Bool        m_bLoppFilterParamFlag;
	Int         m_iAlphaCOffset;
	Int         m_iBetaOffset;
};

struct WeightQuantSyntax
{
	UInt        m_uiPicWeightQuantDataIndex;
	UInt        m_uiWeightQuantParamIndex;
	UInt        m_uiWeightQuantMode;
	Int         m_iWeightQuantParamDelta1[6];
	Int         m_iWeightQuantParamDelta2[6];
	UInt	    m_uiWeightQuantCoeff4x4[4][4];
	UInt	    m_uiWeightQuantCoeff8x8[8][8];
};

struct AlfFilterSyntax
{
	UInt        m_uiAlfFilterNumMinus1;
	UInt        m_uiAlfRegionDistance[16];
	Int         m_iAlfCoeffLuma[16][9];
	Int         m_iAlfCoeffChroma[2][9];
};
#endif

/// supported partition shape
enum PartSize
{
	SIZE_2Nx2N,           ///< symmetric motion partition,  2Nx2N
	SIZE_2NxN,            ///< symmetric motion partition,  2Nx N
	SIZE_Nx2N,            ///< symmetric motion partition,   Nx2N
#if !INTER612
	SIZE_NxN,             ///< symmetric motion partition,   Nx N
#endif
	SIZE_2NxnU,           ///< asymmetric motion partition, 2Nx( N/2) + 2Nx(3N/2)
	SIZE_2NxnD,           ///< asymmetric motion partition, 2Nx(3N/2) + 2Nx( N/2)
	SIZE_nLx2N,           ///< asymmetric motion partition, ( N/2)x2N + (3N/2)x2N
	SIZE_nRx2N,           ///< asymmetric motion partition, (3N/2)x2N + ( N/2)x2N
#if INTER612
	SIZE_NxN,             ///< symmetric motion partition,   Nx N
#endif
#if ENUM_AND_MACRO_YUQUANHE
	SIZE_2NxhN,            // intra SDIP
	SIZE_hNx2N,            //intra SDIP
#endif
	SIZE_NONE = 15
};

/// texture component type
enum TextType
{
	TEXT_LUMA,            ///< luma
	TEXT_CHROMA,          ///< chroma (U+V)
	TEXT_CHROMA_U,        ///< chroma U
	TEXT_CHROMA_V,        ///< chroma V
	TEXT_ALL,             ///< Y+U+V
	TEXT_NONE = 15
};

/// reference list index
enum RefPic
{
	REF_PIC_0 = 0,   ///< reference list 0
	REF_PIC_1 = 1,   ///< reference list 1
	REF_PIC_X = 100  ///< special mark
};

/// distortion function index
enum DFunc
{
	DF_DEFAULT = 0,
	DF_SSE = 1,      ///< general size SSE
	DF_SSE4 = 2,      ///<   4xM SSE
	DF_SSE8 = 3,      ///<   8xM SSE
	DF_SSE16 = 4,      ///<  16xM SSE
	DF_SSE32 = 5,      ///<  32xM SSE
	DF_SSE64 = 6,      ///<  64xM SSE
	DF_SSE16N = 7,      ///< 16NxM SSE

	DF_SAD = 8,      ///< general size SAD
	DF_SAD4 = 9,      ///<   4xM SAD
	DF_SAD8 = 10,     ///<   8xM SAD
	DF_SAD16 = 11,     ///<  16xM SAD
	DF_SAD32 = 12,     ///<  32xM SAD
	DF_SAD64 = 13,     ///<  64xM SAD
	DF_SAD16N = 14,     ///< 16NxM SAD

	DF_SADS = 15,     ///< general size SAD with step
	DF_SADS4 = 16,     ///<   4xM SAD with step
	DF_SADS8 = 17,     ///<   8xM SAD with step
	DF_SADS16 = 18,     ///<  16xM SAD with step
	DF_SADS32 = 19,     ///<  32xM SAD with step
	DF_SADS64 = 20,     ///<  64xM SAD with step
	DF_SADS16N = 21,     ///< 16NxM SAD with step

	DF_HADS = 22,     ///< general size Hadamard with step
	DF_HADS4 = 23,     ///<   4xM HAD with step
	DF_HADS8 = 24,     ///<   8xM HAD with step
	DF_HADS16 = 25,     ///<  16xM HAD with step
	DF_HADS32 = 26,     ///<  32xM HAD with step
	DF_HADS64 = 27,     ///<  64xM HAD with step
	DF_HADS16N = 28,     ///< 16NxM HAD with step

	DF_SSE_FRAME = 33     ///< Frame-based SSE
};

/// index for SBAC based RD optimization
enum CI_IDX
{
	CI_CURR_BEST = 0,     ///< best mode index
	CI_NEXT_BEST,         ///< next best index
	CI_TEMP_BEST,         ///< temporal index
	CI_CHROMA_INTRA,      ///< chroma intra index
	CI_QT_TRAFO_TEST,
	CI_QT_TRAFO_ROOT,
#if WLQ_CUSplitFlag
	CI_COPY,
#endif
	CI_NUM,               ///< total number
};

/// interpolation filter type
#endif

