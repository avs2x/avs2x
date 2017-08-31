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

/** \file     TComPicSym.cpp
    \brief    picture symbol class
*/

#include "TComPicSym.h"

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

Void TComPicSym::create  ( Int iPicWidth, Int iPicHeight, UInt uiLog2MaxCUSize, UInt uiMaxDepth )
{
  UInt i;
  
  m_apcTComPicture      = new TComPicture;
#if AVS3_PIC_HEADER_ZL
  m_apcTComPicHeader      = new TComPicHeader;
#endif
  
  m_uhTotalDepth      = uiMaxDepth;
  m_uiNumPartitions   = 1<<(m_uhTotalDepth<<1);
  
  m_uiMinCUWidth      = (1<<uiLog2MaxCUSize) >> m_uhTotalDepth;
  m_uiMinCUHeight     = (1<<uiLog2MaxCUSize) >> m_uhTotalDepth;

  m_uiLog2MaxCUSize   = uiLog2MaxCUSize;
  m_uiLog2MinCUSize   = uiLog2MaxCUSize - m_uhTotalDepth;
  
  m_uiNumPartInWidth  = 1<<(m_uiLog2MaxCUSize - m_uiLog2MinCUSize);
  
  m_uiWidthInCU       = ( iPicWidth & ((1<<m_uiLog2MaxCUSize)-1) ) ? (iPicWidth  >> m_uiLog2MaxCUSize) + 1 : iPicWidth  >> m_uiLog2MaxCUSize;
  m_uiHeightInCU      = ( iPicHeight& ((1<<m_uiLog2MaxCUSize)-1) ) ? (iPicHeight >> m_uiLog2MaxCUSize) + 1 : iPicHeight >> m_uiLog2MaxCUSize;
  
  m_uiNumCUsInFrame   = m_uiWidthInCU * m_uiHeightInCU;
  m_apcTComDataCU     = new TComDataCU*[m_uiNumCUsInFrame];
  
  for ( i=0; i<m_uiNumCUsInFrame ; i++ )
  {
    m_apcTComDataCU[i] = new TComDataCU;
    m_apcTComDataCU[i]->create( m_uiNumPartitions, m_uiLog2MaxCUSize, false );
  }
#if DPB
  #if  DIRECTSKIP_BUG_YQH
#else
  if (m_dpbPerCtuData == NULL)
#endif
  {
	  m_dpbPerCtuData = new DPBPerCtuData[m_uiNumCUsInFrame];
	  for (UInt i = 0; i<m_uiNumCUsInFrame; i++)
	  {
		  for (Int j = 0; j<NUM_REF_PIC_LIST_01; j++)
		  {
			  m_dpbPerCtuData[i].m_CUMvField[j].create(m_uiNumPartitions);
#if RPS_BUG_YQH1
			  m_dpbPerCtuData[i].m_CUMvField[j].clearMvField();
#endif
		  }
		  m_dpbPerCtuData[i].m_pcPicture = NULL;
	  }
  }
#endif

}

Void TComPicSym::destroy()
{
  Int i;
  
  delete m_apcTComPicture;
  m_apcTComPicture = NULL;
#if AVS3_PIC_HEADER_ZL
  
  delete m_apcTComPicHeader;
  m_apcTComPicHeader = NULL;
#endif

  
  for (i = 0; i < m_uiNumCUsInFrame; i++)
  {
    m_apcTComDataCU[i]->destroy();
    delete m_apcTComDataCU[i];
    m_apcTComDataCU[i] = NULL;
  }
  delete [] m_apcTComDataCU;
  m_apcTComDataCU = NULL;

#if DPB
  if (m_dpbPerCtuData != NULL)
  {
	  for (UInt i = 0; i<m_uiNumCUsInFrame; i++)
	  {
		  for (Int j = 0; j<NUM_REF_PIC_LIST_01; j++)
		  {
			  m_dpbPerCtuData[i].m_CUMvField[j].destroy();
		  }
	  }
	  delete[] m_dpbPerCtuData;
	  m_dpbPerCtuData = NULL;
  }
#endif




}

