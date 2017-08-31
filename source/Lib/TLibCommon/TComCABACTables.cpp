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

/** \file     TEncPIPETables.cpp
    \brief    PIPE encoding tables
*/

#include "TComCABACTables.h"


const UChar TComCABACTables::sm_State2Idx[64] =
{
  0,   0,   1,   2,       2,   3,   3,   4,
  4,   4,   5,   5,       5,   6,   6,   6,
  6,   7,   7,   7,       7,   8,   8,   8,
  8,   8,   8,   8,       8,   9,   9,   9,
  9,   9,   9,   9,       9,   9,   9,   9,
  10,  10,  10,  10,      10,  10,  10,  10,
  10,  10,  10,  10,      10,  10,  10,  10,
  11,  11,  11,  11,      11,  11,  11,  11
};

const UChar TComCABACTables::sm_Idx2State[12] =
{
  0,  2,  3,  4,  
  7,  9, 13, 17, 
  21, 34, 48, 61
};

#define V(a,b,c,d) (((UInt)a << 16) | ((UInt)c << 8) | ((UInt)d << 7) | b)

const UInt TComCABACTables::sm_auiLPSTable[113] =
{
/*
 * Index, Qe_Value, Next_Index_LPS, Next_Index_MPS, Switch_MPS
 */
/*   0 */  V( 0x5A1D,   1,   1, 1 ),
/*   1 */  V( 0x2586,  14,   2, 0 ),
/*   2 */  V( 0x1114,  16,   3, 0 ),
/*   3 */  V( 0x080B,  18,   4, 0 ),
/*   4 */  V( 0x03D8,  20,   5, 0 ),
/*   5 */  V( 0x01DA,  23,   6, 0 ),
/*   6 */  V( 0x00E5,  25,   7, 0 ),
/*   7 */  V( 0x006F,  28,   8, 0 ),
/*   8 */  V( 0x0036,  30,   9, 0 ),
/*   9 */  V( 0x001A,  33,  10, 0 ),
/*  10 */  V( 0x000D,  35,  11, 0 ),
/*  11 */  V( 0x0006,   9,  12, 0 ),
/*  12 */  V( 0x0003,  10,  13, 0 ),
/*  13 */  V( 0x0001,  12,  13, 0 ),
/*  14 */  V( 0x5A7F,  15,  15, 1 ),
/*  15 */  V( 0x3F25,  36,  16, 0 ),
/*  16 */  V( 0x2CF2,  38,  17, 0 ),
/*  17 */  V( 0x207C,  39,  18, 0 ),
/*  18 */  V( 0x17B9,  40,  19, 0 ),
/*  19 */  V( 0x1182,  42,  20, 0 ),
/*  20 */  V( 0x0CEF,  43,  21, 0 ),
/*  21 */  V( 0x09A1,  45,  22, 0 ),
/*  22 */  V( 0x072F,  46,  23, 0 ),
/*  23 */  V( 0x055C,  48,  24, 0 ),
/*  24 */  V( 0x0406,  49,  25, 0 ),
/*  25 */  V( 0x0303,  51,  26, 0 ),
/*  26 */  V( 0x0240,  52,  27, 0 ),
/*  27 */  V( 0x01B1,  54,  28, 0 ),
/*  28 */  V( 0x0144,  56,  29, 0 ),
/*  29 */  V( 0x00F5,  57,  30, 0 ),
/*  30 */  V( 0x00B7,  59,  31, 0 ),
/*  31 */  V( 0x008A,  60,  32, 0 ),
/*  32 */  V( 0x0068,  62,  33, 0 ),
/*  33 */  V( 0x004E,  63,  34, 0 ),
/*  34 */  V( 0x003B,  32,  35, 0 ),
/*  35 */  V( 0x002C,  33,   9, 0 ),
/*  36 */  V( 0x5AE1,  37,  37, 1 ),
/*  37 */  V( 0x484C,  64,  38, 0 ),
/*  38 */  V( 0x3A0D,  65,  39, 0 ),
/*  39 */  V( 0x2EF1,  67,  40, 0 ),
/*  40 */  V( 0x261F,  68,  41, 0 ),
/*  41 */  V( 0x1F33,  69,  42, 0 ),
/*  42 */  V( 0x19A8,  70,  43, 0 ),
/*  43 */  V( 0x1518,  72,  44, 0 ),
/*  44 */  V( 0x1177,  73,  45, 0 ),
/*  45 */  V( 0x0E74,  74,  46, 0 ),
/*  46 */  V( 0x0BFB,  75,  47, 0 ),
/*  47 */  V( 0x09F8,  77,  48, 0 ),
/*  48 */  V( 0x0861,  78,  49, 0 ),
/*  49 */  V( 0x0706,  79,  50, 0 ),
/*  50 */  V( 0x05CD,  48,  51, 0 ),
/*  51 */  V( 0x04DE,  50,  52, 0 ),
/*  52 */  V( 0x040F,  50,  53, 0 ),
/*  53 */  V( 0x0363,  51,  54, 0 ),
/*  54 */  V( 0x02D4,  52,  55, 0 ),
/*  55 */  V( 0x025C,  53,  56, 0 ),
/*  56 */  V( 0x01F8,  54,  57, 0 ),
/*  57 */  V( 0x01A4,  55,  58, 0 ),
/*  58 */  V( 0x0160,  56,  59, 0 ),
/*  59 */  V( 0x0125,  57,  60, 0 ),
/*  60 */  V( 0x00F6,  58,  61, 0 ),
/*  61 */  V( 0x00CB,  59,  62, 0 ),
/*  62 */  V( 0x00AB,  61,  63, 0 ),
/*  63 */  V( 0x008F,  61,  32, 0 ),
/*  64 */  V( 0x5B12,  65,  65, 1 ),
/*  65 */  V( 0x4D04,  80,  66, 0 ),
/*  66 */  V( 0x412C,  81,  67, 0 ),
/*  67 */  V( 0x37D8,  82,  68, 0 ),
/*  68 */  V( 0x2EF8,  83,  69, 0 ),
/*  69 */  V( 0x293C,  84,  70, 0 ),
/*  70 */  V( 0x2379,  86,  71, 0 ),
/*  71 */  V( 0x1EDF,  87,  72, 0 ),
/*  72 */  V( 0x1AA9,  87,  73, 0 ),
/*  73 */  V( 0x174E,  72,  74, 0 ),
/*  74 */  V( 0x1424,  72,  75, 0 ),
/*  75 */  V( 0x119C,  74,  76, 0 ),
/*  76 */  V( 0x0F6B,  74,  77, 0 ),
/*  77 */  V( 0x0D51,  75,  78, 0 ),
/*  78 */  V( 0x0BB6,  77,  79, 0 ),
/*  79 */  V( 0x0A40,  77,  48, 0 ),
/*  80 */  V( 0x5832,  80,  81, 1 ),
/*  81 */  V( 0x4D1C,  88,  82, 0 ),
/*  82 */  V( 0x438E,  89,  83, 0 ),
/*  83 */  V( 0x3BDD,  90,  84, 0 ),
/*  84 */  V( 0x34EE,  91,  85, 0 ),
/*  85 */  V( 0x2EAE,  92,  86, 0 ),
/*  86 */  V( 0x299A,  93,  87, 0 ),
/*  87 */  V( 0x2516,  86,  71, 0 ),
/*  88 */  V( 0x5570,  88,  89, 1 ),
/*  89 */  V( 0x4CA9,  95,  90, 0 ),
/*  90 */  V( 0x44D9,  96,  91, 0 ),
/*  91 */  V( 0x3E22,  97,  92, 0 ),
/*  92 */  V( 0x3824,  99,  93, 0 ),
/*  93 */  V( 0x32B4,  99,  94, 0 ),
/*  94 */  V( 0x2E17,  93,  86, 0 ),
/*  95 */  V( 0x56A8,  95,  96, 1 ),
/*  96 */  V( 0x4F46, 101,  97, 0 ),
/*  97 */  V( 0x47E5, 102,  98, 0 ),
/*  98 */  V( 0x41CF, 103,  99, 0 ),
/*  99 */  V( 0x3C3D, 104, 100, 0 ),
/* 100 */  V( 0x375E,  99,  93, 0 ),
/* 101 */  V( 0x5231, 105, 102, 0 ),
/* 102 */  V( 0x4C0F, 106, 103, 0 ),
/* 103 */  V( 0x4639, 107, 104, 0 ),
/* 104 */  V( 0x415E, 103,  99, 0 ),
/* 105 */  V( 0x5627, 105, 106, 1 ),
/* 106 */  V( 0x50E7, 108, 107, 0 ),
/* 107 */  V( 0x4B85, 109, 103, 0 ),
/* 108 */  V( 0x5597, 110, 109, 0 ),
/* 109 */  V( 0x504F, 111, 107, 0 ),
/* 110 */  V( 0x5A10, 110, 111, 1 ),
/* 111 */  V( 0x5522, 112, 109, 0 ),
/* 112 */  V( 0x59EB, 112, 111, 1 )
};
