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

/** \file     TComPicSym.h
    \brief    picture symbol class (header)
*/

#ifndef __TCOMPICSYM__
#define __TCOMPICSYM__


// Include files
#include "CommonDef.h"
#include "TComPicture.h"
#include "TComDataCU.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// picture symbol class
class TComPicSym
{
private:
  UInt          m_uiWidthInCU;
  UInt          m_uiHeightInCU;
  
  UInt          m_uiLog2MaxCUSize;
  UInt          m_uiLog2MinCUSize;

  UInt          m_uiMinCUWidth;
  UInt          m_uiMinCUHeight;

  UChar         m_uhTotalDepth;       ///< max. depth
  UInt          m_uiNumPartitions;
  UInt          m_uiNumPartInWidth;
  UInt          m_uiNumCUsInFrame;
  
  TComPicture*    m_apcTComPicture;
#if AVS3_PIC_HEADER_ZL
  TComPicHeader* m_apcTComPicHeader;
#endif
  TComDataCU**  m_apcTComDataCU;        ///< array of CU data
  
public:
  Void        create  ( Int iPicWidth, Int iPicHeight, UInt uiLog2MaxCUSize, UInt uiMaxDepth );
  Void        destroy ();
  
  TComPicture*  getPicture()                { return  m_apcTComPicture;               }
#if AVS3_PIC_HEADER_ZL
  TComPicHeader * getPicHeader()           {return m_apcTComPicHeader;}
#endif
  
  UInt         getFrameWidthInCU()       { return m_uiWidthInCU;                 }
  UInt         getFrameHeightInCU()      { return m_uiHeightInCU;                }

  UInt         getMinCUWidth()           { return m_uiMinCUWidth;                }
  UInt         getMinCUHeight()          { return m_uiMinCUHeight;               }

  UInt         getLog2MinCUSize()        { return m_uiLog2MinCUSize;             }
  UInt         getNumberOfCUsInFrame()   { return m_uiNumCUsInFrame;  }
  TComDataCU*& getCU( UInt uiCUAddr )    { return m_apcTComDataCU[uiCUAddr];     }
  
  
  Void        setPicture(TComPicture* p)    { m_apcTComPicture = p;                   }
#if AVS3_PIC_HEADER_ZL
  Void        setPicHeader(TComPicHeader* p)    { m_apcTComPicHeader = p;                   }
#endif


  UInt       getNumPartition()          { return m_uiNumPartitions;             }
  UInt       getNumPartInWidth()        { return m_uiNumPartInWidth;            }

#if DPB 
public:
	struct DPBPerCtuData
	{
		TComCUMvField  m_CUMvField[NUM_REF_PIC_LIST_01];
		TComPicture*   m_pcPicture;
		TComCUMvField* getCUMvField(RefPic e)   { return &m_CUMvField[e]; }
		const TComPicture* getPicture()                         const { return m_pcPicture; }
	};

	Bool                 hasDPBPerCtuData() const                            { return (m_dpbPerCtuData != 0); };
	DPBPerCtuData&       getDPBPerCtuData(UInt uiCUAddr)                    { return m_dpbPerCtuData[uiCUAddr]; }
	const DPBPerCtuData& getDPBPerCtuData(UInt uiCUAddr) const              { return m_dpbPerCtuData[uiCUAddr]; }

private:
	DPBPerCtuData* m_dpbPerCtuData;
#endif

#if DPB
public:
	TComPicSym() :m_dpbPerCtuData(NULL) {}
	~TComPicSym() {}
#endif

};// END CLASS DEFINITION TComPicSym


#endif // __TCOMPICSYM__

