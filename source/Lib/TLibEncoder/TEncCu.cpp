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

/** \file     TEncCU.cpp
    \brief    CU encoder class
*/

#include <stdio.h>
#include "TEncTop.h"
#include "TEncCu.h"
#include "TEncAnalyze.h"

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

/**
 \param    uiTotalDepth  total number of allowable depth
 \param    uiMaxWidth    largest CU width
 \param    uiMaxHeight   largest CU height
 */
Void TEncCu::create(UChar uhTotalDepth, UInt uiLog2MaxCUSize)
{
  Int i;
  m_uhTotalDepth   = uhTotalDepth + 1;

  m_ppcBestCU      = new TComDataCU*[uhTotalDepth];
  m_ppcTempCU      = new TComDataCU*[uhTotalDepth];
  
  m_pcResiYuvBest = new TComYuv; m_pcResiYuvBest->create(uiLog2MaxCUSize);
  m_pcPredYuvTemp = new TComYuv; m_pcPredYuvTemp->create(uiLog2MaxCUSize);
  m_pcResiYuvTemp = new TComYuv; m_pcResiYuvTemp->create(uiLog2MaxCUSize);
  m_pcRecoYuvTemp = new TComYuv; m_pcRecoYuvTemp->create(uiLog2MaxCUSize, 1);
  m_pcOrigYuv     = new TComYuv; m_pcOrigYuv    ->create(uiLog2MaxCUSize);
  
  UInt uiNumPartitions;
  for( i=0 ; i<m_uhTotalDepth-1 ; i++)
  {
    uiNumPartitions = 1<<( ( uhTotalDepth - i )<<1 );
    
    m_ppcBestCU[i] = new TComDataCU; m_ppcBestCU[i]->create( uiNumPartitions, uiLog2MaxCUSize - i, false );
    m_ppcTempCU[i] = new TComDataCU; m_ppcTempCU[i]->create( uiNumPartitions, uiLog2MaxCUSize - i, false );
  }
  
  // initialize partition order.
  UInt* piTmp = &g_auiZscanToRaster[0];
  initZscanToRaster( m_uhTotalDepth, 1, 0, piTmp);
  initRasterToZscan( uiLog2MaxCUSize, m_uhTotalDepth );
  
  // initialize conversion matrix from partition index to pel
  initRasterToPelXY( uiLog2MaxCUSize, m_uhTotalDepth );
}

Void TEncCu::destroy()
{
  Int i;
  for( i=0 ; i<m_uhTotalDepth-1 ; i++)
  {
    if(m_ppcBestCU[i])
    {
      m_ppcBestCU[i]->destroy();      delete m_ppcBestCU[i];      m_ppcBestCU[i] = NULL;
    }
    if(m_ppcTempCU[i])
    {
      m_ppcTempCU[i]->destroy();      delete m_ppcTempCU[i];      m_ppcTempCU[i] = NULL;
    }
  }
  if(m_ppcBestCU)
  {
    delete [] m_ppcBestCU;
    m_ppcBestCU = NULL;
  }
  if(m_ppcTempCU)
  {
    delete [] m_ppcTempCU;
    m_ppcTempCU = NULL;
  }
  if(m_pcResiYuvBest)
  {
    m_pcResiYuvBest->destroy();
    delete m_pcResiYuvBest;
    m_pcResiYuvBest = NULL;
  }
  if(m_pcPredYuvTemp)
  {
    m_pcPredYuvTemp->destroy();
    delete m_pcPredYuvTemp;
    m_pcPredYuvTemp = NULL;
  }
  if(m_pcResiYuvTemp)
  {
    m_pcResiYuvTemp->destroy();
    delete m_pcResiYuvTemp;
    m_pcResiYuvTemp = NULL;
  }
  if(m_pcRecoYuvTemp)
  {
    m_pcRecoYuvTemp->destroy();
    delete m_pcRecoYuvTemp;
    m_pcRecoYuvTemp = NULL;
  }
  if(m_pcOrigYuv)
  {
    m_pcOrigYuv->destroy();
    delete m_pcOrigYuv;
    m_pcOrigYuv = NULL;
  }
}

/** \param    pcEncTop      pointer of encoder class
 */
Void TEncCu::init( TEncTop* pcEncTop )
{
  m_pcEncCfg          = pcEncTop;
  m_pcPredSearch      = pcEncTop->getPredSearch();
  m_pcTrQuant         = pcEncTop->getTrQuant();
  m_pcBitCounter      = pcEncTop->getBitCounter();
  m_pcRdCost          = pcEncTop->getRdCost();
  
  m_pcEntropyCoder    = pcEncTop->getEntropyCoder();
  m_pcSbacCoder       = pcEncTop->getSbacCoder();
  
  m_pppcRDSbacCoder   = pcEncTop->getRDSbacCoder();
  m_pcRDGoOnSbacCoder = pcEncTop->getRDGoOnSbacCoder();
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** \param  rpcCU pointer of CU data class
 */
Void TEncCu::compressCU( TComDataCU*& rpcCU )
{
  // initialize CU data
  m_ppcBestCU[0]->initCU( rpcCU->getPic(), rpcCU->getAddr() );
  m_ppcTempCU[0]->initCU( rpcCU->getPic(), rpcCU->getAddr() );

  m_pcOrigYuv->copyFromPicYuv( rpcCU->getPic()->getPicYuvOrg(), rpcCU->getAddr(), rpcCU->getZorderIdxInCU(), 1<<rpcCU->getLog2CUSize(0) );

  // analysis of CU
#if WLQ_CUSplitFlag 
  xCompressCU(m_ppcBestCU[0], m_ppcTempCU[0], 0, 0);
#else
  xCompressCU(m_ppcBestCU[0], m_ppcTempCU[0], 0); //LCU
#endif
 
}

/** \param  pcCU  pointer of CU data class
 */
Void TEncCu::encodeCU(TComDataCU* pcCU)
{
#if niu_terimating
	Bool bTerminatePicture = (pcCU->getAddr() == pcCU->getPic()->getNumCUsInFrame() - 1) ? true : false;
	if (pcCU->getAddr() != 0)
		m_pcEntropyCoder->encodeTerminatingBit(0);
	xEncodeCU(pcCU, 0, 0);

#else
	// encode CU data
	xEncodeCU(pcCU, 0, 0);
	//--- write terminating bit ---
	Bool bTerminatePicture = (pcCU->getAddr() == pcCU->getPic()->getNumCUsInFrame() - 1) ? true : false;
	m_pcEntropyCoder->encodeTerminatingBit(bTerminatePicture ? 1 : 0);
#endif

	// Encode picture finish
	if (bTerminatePicture)
	{
#if terimating_1

#if YQH_SPLIGFLAG_DEC
		m_pcEntropyCoder->encodeTerminatingBit(bTerminatePicture ? 1 : 0);
#else
		m_pcEntropyCoder->encodeTerminatingBit(1);
#endif
#endif
		m_pcEntropyCoder->encodePictureFinish();
	}
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================
#if WLQ_CUSplitFlag 
#if WLQ_rdcost
Double TEncCu::xCompressCU(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiDepth, Double split_rd_cost)
#else
Void TEncCu::xCompressCU(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiDepth, Double split_rd_cost)
#endif
#else
Void TEncCu::xCompressCU( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiDepth )
#endif
{
  TComPic* pcPic = rpcBestCU->getPic();
  
  Bool bBoundary = false;
  UInt uiLPelX   = rpcBestCU->getCUPelX();
  UInt uiRPelX   = uiLPelX + (1<<rpcBestCU->getLog2CUSize(0))  - 1;
  UInt uiTPelY   = rpcBestCU->getCUPelY();
  UInt uiBPelY   = uiTPelY + (1<<rpcBestCU->getLog2CUSize(0)) - 1;

  UInt uiPrevBits = rpcBestCU->getTotalBits();
  UInt uiCurrBits = 0;
  //test

#if WLQ_CUSplitFlag
  Double split_cost = 0;
#endif
#if WLQ_rdcost
  Double min_rd_cost = MAX_DOUBLE;
#endif
  if( ( uiRPelX < rpcBestCU->getPicture()->getSPS()->getWidth() ) && ( uiBPelY < rpcBestCU->getPicture()->getSPS()->getHeight() ) )
  {
  }
  else
  {
    bBoundary = true;
  }
#if WLQ_CUSplitFlag
  m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
#endif
  // further split
#if WLQ_debug
  if( uiDepth < g_uiMaxCUDepth - 2 )
#else
  if( uiDepth < g_uiMaxCUDepth - 1 )
#endif
  {
    UChar       uhNextDepth         = uiDepth+1;
    TComDataCU* pcSubBestPartCU     = m_ppcBestCU[uhNextDepth];
    TComDataCU* pcSubTempPartCU     = m_ppcTempCU[uhNextDepth];
#if WLQ_rdcost
	min_rd_cost = 0;
#endif
#if WLQ_CUSplitFlag
	if (!bBoundary)
	{
		UInt rate = m_pcEntropyCoder->getNumberOfWrittenBits();
		m_pcEntropyCoder->encodeSplitFlag(rpcBestCU, 1, false, 0);
#if WLQ_rdcost 
		split_cost = rpcBestCU->getPicture()->getLambda()*(m_pcEntropyCoder->getNumberOfWrittenBits() - rate);
#else
		split_cost = (m_pcEntropyCoder->getNumberOfWrittenBits() - rate);
#endif
	}
#endif
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++ )
    {
      pcSubBestPartCU->initSubCU( rpcBestCU, uiPartUnitIdx, uhNextDepth );           // clear sub partition datas or init.
      pcSubTempPartCU->initSubCU( rpcBestCU, uiPartUnitIdx, uhNextDepth );           // clear sub partition datas or init.

      if( ( pcSubBestPartCU->getCUPelX() < pcSubBestPartCU->getPicture()->getSPS()->getWidth() ) && ( pcSubBestPartCU->getCUPelY() < pcSubBestPartCU->getPicture()->getSPS()->getHeight() ) )
      {
        if ( 0 == uiPartUnitIdx) //initialize RD with previous depth buffer
        {
          m_pppcRDSbacCoder[uhNextDepth][CI_CURR_BEST]->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
        }
        else
        {			
          m_pppcRDSbacCoder[uhNextDepth][CI_CURR_BEST]->load(m_pppcRDSbacCoder[uhNextDepth][CI_NEXT_BEST]);
        }
#if WLQ_CUSplitFlag 
#if WLQ_rdcost
		min_rd_cost += xCompressCU(pcSubBestPartCU, pcSubTempPartCU, uhNextDepth, split_rd_cost + split_cost); //sub-cu
#else
		xCompressCU(pcSubBestPartCU, pcSubTempPartCU, uhNextDepth, split_rd_cost + split_cost); //sub-cu
#endif
#else
        xCompressCU( pcSubBestPartCU, pcSubTempPartCU, uhNextDepth ); //sub-cu
#endif
        
        rpcTempCU->copyPartFrom( pcSubBestPartCU, uiPartUnitIdx, uhNextDepth );         // Keep best part data to current temporary data.
      }
    }
#if !WLQ_CUSplitFlag   
    if( !bBoundary )
    {
      m_pcEntropyCoder->resetBits();
#if YQH_RDO_InitEstBits_BUG
	  UInt bits_init = m_pcEntropyCoder->getNumberOfWrittenBits();
#endif
#if WLQ_CUSplitFlag
	  m_pcEntropyCoder->encodeSplitFlag( rpcTempCU, 1 );
#else
      m_pcEntropyCoder->encodeSplitFlag( rpcTempCU, 0, uiDepth, true );
#endif
#if YQH_RDO_InitEstBits_BUG
	  rpcTempCU->getTotalBits() += (m_pcEntropyCoder->getNumberOfWrittenBits() - bits_init);
#else       
      rpcTempCU->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // split bits
#endif
    }
#endif
    m_pppcRDSbacCoder[uhNextDepth][CI_NEXT_BEST]->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);
#if WLQ_rdcost
	rpcTempCU->getTotalCost() = min_rd_cost;
#else
    rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );
#endif
#if WLQ_rdcost
	xCheckBestMode( rpcBestCU, rpcTempCU, 0, uiDepth, true ); 
#else
    xCheckBestMode( rpcBestCU, rpcTempCU, uiDepth, true );                                 // RD compare current larger prediction with sub partitioned prediction.
#endif
  }                                                                                  // with sub partitioned prediction.

#if WLQ_HALFCTU_BUG
  m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
#endif

  if( bBoundary )
  {
    rpcBestCU->copyToPic(uiDepth);                                                     // Copy Best data to Picture for next partition prediction.
#if WLQ_rdcost
	#if WLQ_HALFCTU_BUG
	return min_rd_cost;
	#else
		return MAX_DOUBLE;
	#endif
#else
	return;>	TAppEncoder.exe!TEncCu::xCompressCU(TComDataCU * & rpcBestCU, TComDataCU * & rpcTempCU, unsigned int uiDepth, double split_rd_cost) 行 321	C++

#endif
  }
#if !WLQ_HALFCTU_BUG
  m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[ uiDepth ][ CI_CURR_BEST ] );
#endif

#if !WLQ_CUSplitFlag
  if( uiDepth < g_uiMaxCUDepth - 1 )
  {
    m_pcEntropyCoder->resetBits();
#if YQH_RDO_InitEstBits_BUG
	uiCurrBits = m_pcEntropyCoder->getNumberOfWrittenBits();
#endif
	//test
#if WLQ_CUSplitFlag
	m_pcEntropyCoder->encodeSplitFlag( rpcTempCU, 1 );
#else
    m_pcEntropyCoder->encodeSplitFlag( rpcTempCU, 0, uiDepth, true );
#endif
#if YQH_RDO_InitEstBits_BUG
	uiCurrBits = m_pcEntropyCoder->getNumberOfWrittenBits() - uiCurrBits; // split bits
#else
    uiCurrBits = m_pcEntropyCoder->getNumberOfWrittenBits(); // split bits
#endif
  }
  else
    uiCurrBits = 0;
#endif
  m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[ uiDepth ][ CI_CURR_BEST ] );

#if WLQ_CUSplitFlag
  Double split_cost_up = 0;
  if (rpcTempCU->getLog2CUSize(0) > MIN_CU_SIZE_IN_BIT)
  {
	  UInt rate = m_pcEntropyCoder->getNumberOfWrittenBits();
	  m_pcEntropyCoder->encodeSplitFlag(rpcTempCU, 0, false, 0);
#if WLQ_rdcost
	  split_cost_up = rpcTempCU->getPicture()->getLambda()*(m_pcEntropyCoder->getNumberOfWrittenBits() - rate);
#else
	  split_cost_up = (m_pcEntropyCoder->getNumberOfWrittenBits() - rate);
#endif
	  m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
  }
#endif
  // do inter modes
  if (pcPic->getPicture()->getPictureType() != I_PICTURE)
  {
    // try skip mode
#if DEBUG_ONLY2Nx2N
#if WLQ_CUSplitFlag_inter
#if WLQ_rdcost
	  //  WLQ_Inter_DEBUG
	  //  SIZE_2Nx2N SIZE_2NxN SIZE_Nx2N   SIZE_2NxnU SIZE_2NxnD  SIZE_nLx2N SIZE_nLx2N
	  //
#if niu_multimode
	  //注意 true 和 false
	  //test
	  #if   YQH_INTRA_LAMDA_DUG
	  if (pcPic->getPicture()->getPictureType() == P_PICTURE)
	  {
		    min_rd_cost = xCheckRDCostSkip(rpcBestCU, rpcTempCU, SIZE_2Nx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, true);	rpcTempCU->initEstData(uiDepth);
		   min_rd_cost = xCheckRDCostSkip_NO_QT(rpcBestCU, rpcTempCU, SIZE_2Nx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, false);	rpcTempCU->initEstData(uiDepth);

		//  min_rd_cost = xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_2Nx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, true);	rpcTempCU->initEstData(uiDepth);
	  }
	  else
	  {
		   // min_rd_cost = xCheckRDCostSkip(rpcBestCU, rpcTempCU, SIZE_2Nx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, true);	rpcTempCU->initEstData(uiDepth);
		  //  min_rd_cost = xCheckRDCostSkip_NO_QT(rpcBestCU, rpcTempCU, SIZE_2Nx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, false);	rpcTempCU->initEstData(uiDepth);
		  min_rd_cost = xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_2Nx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, true);	rpcTempCU->initEstData(uiDepth);
	  }

#else
	  min_rd_cost = xCheckRDCostSkip(rpcBestCU, rpcTempCU, SIZE_2Nx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, true);	rpcTempCU->initEstData(uiDepth);
	  min_rd_cost = xCheckRDCostSkip_NO_QT(rpcBestCU, rpcTempCU, SIZE_2Nx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, false);	rpcTempCU->initEstData(uiDepth);
	  ////
#if F_MHPSKIP_SYC
	  if (
#if B_MHBSKIP_SYC
		  (
#endif	  
		  pcPic->getPicture()->getSPS()->getMultiHypothesisSkipEnableFlag() == true
#if B_MHBSKIP_SYC
		  && (pcPic->getPicture()->getPictureType() == F_PICTURE)) || pcPic->getPicture()->getPictureType() == B_PICTURE
#endif
		  
		  )//MHPSKIP+MHBSKIP
	  {
#if B_MHBSKIP_SYC
		  min_rd_cost = xCheckRDCostMHSkip(rpcBestCU, rpcTempCU, SIZE_2Nx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, false);	rpcTempCU->initEstData(uiDepth);
		  min_rd_cost = xCheckRDCostMHSkip_NO_QT(rpcBestCU, rpcTempCU, SIZE_2Nx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, false);	rpcTempCU->initEstData(uiDepth);

#else
		  min_rd_cost = xCheckRDCostMHPSkip(rpcBestCU, rpcTempCU, SIZE_2Nx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, false);	rpcTempCU->initEstData(uiDepth);
		  min_rd_cost = xCheckRDCostMHPSkip_NO_QT(rpcBestCU, rpcTempCU, SIZE_2Nx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, false);	rpcTempCU->initEstData(uiDepth);
#endif
	  }
#endif
	  min_rd_cost = xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_2Nx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, false);	rpcTempCU->initEstData(uiDepth);

    min_rd_cost = xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_2NxN, min_rd_cost, split_cost, split_cost_up, split_rd_cost, false);	rpcTempCU->initEstData(uiDepth);
    min_rd_cost = xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_Nx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, false);	rpcTempCU->initEstData(uiDepth);
  if (rpcTempCU->getLog2CUSize(0) > 3)
  {
	 ////min_rd_cost = xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_NxN, min_rd_cost, split_cost, split_cost_up, split_rd_cost, false);	rpcTempCU->initEstData(uiDepth);

	  min_rd_cost = xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_2NxnU, min_rd_cost, split_cost, split_cost_up, split_rd_cost, false);	rpcTempCU->initEstData(uiDepth);
	  min_rd_cost = xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_2NxnD, min_rd_cost, split_cost, split_cost_up, split_rd_cost, false);	rpcTempCU->initEstData(uiDepth);
	  min_rd_cost = xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_nLx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, false);	rpcTempCU->initEstData(uiDepth);
	  min_rd_cost = xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_nRx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, false);	rpcTempCU->initEstData(uiDepth);
  }
#endif
#else
	  min_rd_cost = xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_nRx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, true); //true or false多模式需改造 
	  rpcTempCU->initEstData(uiDepth);
#endif
#else
	  xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_2Nx2N, split_cost, split_cost_up, split_rd_cost, true); //true or false多模式需改造
	  rpcTempCU->initEstData(uiDepth);
#endif
	  
#else
		xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_2Nx2N, uiCurrBits);  rpcTempCU->initEstData(uiDepth);
#endif
#else
    xCheckRDCostSkip ( rpcBestCU, rpcTempCU, uiCurrBits );  rpcTempCU->initEstData( uiDepth );
 

#if RD_DIRECT
      xCheckRDCostDirect(rpcBestCU, rpcTempCU, uiCurrBits);  rpcTempCU->initEstData(uiDepth);
#endif
    // try inter mode
    xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2Nx2N, uiCurrBits );  rpcTempCU->initEstData( uiDepth );
   
   xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_2NxN, uiCurrBits);  rpcTempCU->initEstData(uiDepth);
   xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_Nx2N, uiCurrBits);  rpcTempCU->initEstData(uiDepth);
#if PU_ZY
		if (rpcTempCU->getLog2CUSize(0) > 3)
		{
			xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_NxN, uiCurrBits);  rpcTempCU->initEstData(uiDepth);
		}
#else
		xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_NxN, uiCurrBits);  rpcTempCU->initEstData(uiDepth);
#endif

		// SIZE_2NxnU, SIZE_2NxnD, SIZE_nLx2N, SIZE_nRx2N
    if( rpcTempCU->getLog2CUSize( 0 ) > 3 )
    {
		  xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2NxnU, uiCurrBits ); rpcTempCU->initEstData( uiDepth );
		  xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2NxnD, uiCurrBits ); rpcTempCU->initEstData( uiDepth );
		  xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_nLx2N, uiCurrBits ); rpcTempCU->initEstData( uiDepth );
		 xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_nRx2N, uiCurrBits ); rpcTempCU->initEstData( uiDepth );
    }
#endif
  }
    // speedup for inter frames

#if DEBUG_ONLY2Nx2N
#if DEBUG_ONLY2Nx2N_YQH
  if (pcPic->getPicture()->getPictureType() == I_PICTURE ||
	  rpcBestCU->getDepth(0) != uiDepth ||
	  rpcBestCU->getCbf(0, TEXT_LUMA) != 0 ||
	  rpcBestCU->getCbf(0, TEXT_CHROMA_U) != 0 ||
	  rpcBestCU->getCbf(0, TEXT_CHROMA_V) != 0) // avoid very complex intra if it is unlikely
#else
	if (pcPic->getPicture()->getPictureType() == I_PICTURE)

#endif
#else
    if (pcPic->getPicture()->getPictureType() == I_PICTURE ||
      rpcBestCU->getDepth(0) != uiDepth ||
      rpcBestCU->getCbf(0, TEXT_LUMA) != 0 ||
      rpcBestCU->getCbf(0, TEXT_CHROMA_U) != 0 ||
      rpcBestCU->getCbf(0, TEXT_CHROMA_V) != 0) // avoid very complex intra if it is unlikely
#endif
		{
      // try intra mode
#if WLQ_CUSplitFlag
#if WLQ_rdcost
			#if	ZHANGYI_INTRA_SDIP_BUG_YQH
			#if ZHANGYI_INTRA_SDIP
			rpcTempCU->setSDIPFlagSubParts(0, 0, rpcTempCU->getDepth(0));
			rpcTempCU->setSDIPDirectionSubParts(0, 0, rpcTempCU->getDepth(0));
#endif
#endif
			min_rd_cost = xCheckRDCostIntra(rpcBestCU, rpcTempCU, SIZE_2Nx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, rpcTempCU->getLog2CUSize(0) == 3 ? false : true);
#else
			xCheckRDCostIntra(rpcBestCU, rpcTempCU, SIZE_2Nx2N, split_cost, split_cost_up, split_rd_cost, rpcTempCU->getLog2CUSize(0) == 3 ? false : true);
#endif
#else
      xCheckRDCostIntra(rpcBestCU, rpcTempCU, SIZE_2Nx2N, uiCurrBits);
#endif
      rpcTempCU->initEstData(uiDepth);

#if ZHANGYI_INTRA
      UInt SizeInBit = rpcTempCU->getLog2CUSize(0); //zhangyiCheck: whether is 0
#if ZHANGYI_INTRA_SDIP
	  if (SizeInBit == 3)
#else
	  if ((SizeInBit == 3) || (((rpcTempCU->getPicture()->getSPS()->getNonsquareIntraPredictionEnableFlag() == 1) && ((SizeInBit > 3) && (SizeInBit < 6)))))
#endif
#endif
      {
#if	YQH_INTRA_SDIP

		  if (SizeInBit == 3)
		  {
#if WLQ_CUSplitFlag
#if WLQ_rdcost
			 min_rd_cost = xCheckRDCostIntra(rpcBestCU, rpcTempCU, SIZE_NxN, min_rd_cost, split_cost, split_cost_up, split_rd_cost, false);
#else
			  xCheckRDCostIntra(rpcBestCU, rpcTempCU, SIZE_NxN, split_cost, split_cost_up, split_rd_cost, true);
#endif
#else
			  xCheckRDCostIntra(rpcBestCU, rpcTempCU, SIZE_NxN, uiCurrBits);
#endif
			  rpcTempCU->initEstData(uiDepth);
		  }
		  else
		  {
			  //SDIP
		  }
	  }
#if ZHANGYI_INTRA_SDIP
		  if (((SizeInBit > 3) && (SizeInBit < 6)) && rpcTempCU->getPicture()->getSPS()->getNonsquareIntraPredictionEnableFlag())
		  {
			  Bool bNeedSDIPflag = false;
			  if (rpcTempCU->getWidth(0) == 16 || rpcTempCU->getWidth(0) == 32)
			  {
				  bNeedSDIPflag = true;
			  }
			  if (bNeedSDIPflag)
			  {

				  rpcTempCU->setSDIPFlagSubParts(1, 0, rpcTempCU->getDepth(0));
				  rpcTempCU->setSDIPDirectionSubParts(1, 0, rpcTempCU->getDepth(0));

				  min_rd_cost = xCheckRDCostIntra(rpcBestCU, rpcTempCU, SIZE_2NxhN, min_rd_cost, split_cost, split_cost_up, split_rd_cost, true);
				  rpcTempCU->initEstData(uiDepth);

				  rpcTempCU->setSDIPFlagSubParts(1, 0, rpcTempCU->getDepth(0));
				  rpcTempCU->setSDIPDirectionSubParts(0, 0, rpcTempCU->getDepth(0));
				  min_rd_cost = xCheckRDCostIntra(rpcBestCU, rpcTempCU, SIZE_hNx2N, min_rd_cost, split_cost, split_cost_up, split_rd_cost, true);
				  rpcTempCU->initEstData(uiDepth);

			  }
	  }
#endif

#else
#if	!YQH_INTRA_NO2X2
        xCheckRDCostIntra(rpcBestCU, rpcTempCU, SIZE_NxN, uiCurrBits);
        rpcTempCU->initEstData(uiDepth);
#endif
#endif

   
    }

  rpcBestCU->copyToPic(uiDepth);                                                                       // Copy Best data to Picture for next partition prediction.

  // Assert if Best prediction mode is NONE
  // Selected mode's RD-cost must be not MAX_DOUBLE.
  assert( rpcBestCU->getPredictionMode( 0 ) != MODE_NONE  );
  assert( rpcBestCU->getTotalCost     (   ) != MAX_DOUBLE );
#if WLQ_rdcost
  return min_rd_cost;
#endif
}

Void TEncCu::xEncodeCU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  TComPic* pcPic = pcCU->getPic();
  
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (1<<(g_uiLog2MaxCUSize - uiDepth))  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (1<<(g_uiLog2MaxCUSize - uiDepth)) - 1;
  
  if( ( uiRPelX < pcCU->getPicture()->getSPS()->getWidth() ) && ( uiBPelY < pcCU->getPicture()->getSPS()->getHeight() ) )
  {
#if !WLQ_CUSplitFlag
#if WLQ_CUSplitFlag
	  m_pcEntropyCoder->encodeSplitFlag(pcCU, 1);
#else
    m_pcEntropyCoder->encodeSplitFlag( pcCU, uiAbsPartIdx, uiDepth );
#endif
#endif
  }
  else
  {
    bBoundary = true;
  }
  
  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth - 1) ) ) || bBoundary )
  {
    UInt uiQNumParts = ( pcPic->getNumPartInCU() >> (uiDepth<<1) )>>2;
#if WLQ_CUSplitFlag
	if (!bBoundary)
		m_pcEntropyCoder->encodeSplitFlag(pcCU, 1, true, g_uiLog2MaxCUSize - uiDepth);
#endif
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
      
      if( ( uiLPelX < pcCU->getPicture()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getPicture()->getSPS()->getHeight() ) )
        xEncodeCU( pcCU, uiAbsPartIdx, uiDepth+1 );
    }
    return;
  }
#if WLQ_CUSplitFlag
  if (g_uiLog2MaxCUSize - uiDepth> MIN_CU_SIZE_IN_BIT && !bBoundary)
	  m_pcEntropyCoder->encodeSplitFlag(pcCU, 0, true, g_uiLog2MaxCUSize - uiDepth);
#endif
#if INTER_GROUP
	//AVS3
	//m_pcEntropyCoder->encodePredMode(pcCU, uiAbsPartIdx);
#if WLQ_CUTYPE_BUG
#if inter_intra_1
  if (!pcCU->getPicture()->isIntra())
#else
  if (!pcCU->isIntra(uiAbsPartIdx))
#endif
	m_pcEntropyCoder->encodeInterCUTypeIndex(pcCU, uiAbsPartIdx);
#endif
#if  !YQH_CUTYPE_BUG
	m_pcEntropyCoder->encodeInterCUTypeIndex(pcCU, uiAbsPartIdx);
#endif

	//AVS3
#else
	m_pcEntropyCoder->encodePredMode(pcCU, uiAbsPartIdx);
	m_pcEntropyCoder->encodePartSize(pcCU, uiAbsPartIdx);
#endif

#if  YQH_CUTYPE_BUG
#if WLQ_CUTYPE_BUG
	if(pcCU->isIntra(uiAbsPartIdx))
	{
#if  !inter_intra_1
		m_pcEntropyCoder->encodePredMode(pcCU, uiAbsPartIdx);
#endif
		m_pcEntropyCoder->encodePartSize(pcCU, uiAbsPartIdx);
	}
#else
	m_pcEntropyCoder->encodePredMode(pcCU, uiAbsPartIdx);
	m_pcEntropyCoder->encodePartSize(pcCU, uiAbsPartIdx);
#endif
#endif

#if ZHANGYI_INTRA
#if !YQH_INTRA
  m_pcEntropyCoder->encodeIntraPuTypeIndex(pcCU, uiAbsPartIdx);
#endif
#endif
  m_pcEntropyCoder->encodePredInfo( pcCU, uiAbsPartIdx );
  // Encode Coefficients
#if niu_rate_divide_2
#if niu_write_cbf
  m_pcEntropyCoder->encodeCbfY(pcCU, uiAbsPartIdx, TEXT_LUMA,     0);
#else
  m_pcEntropyCoder->encodeCbf(pcCU, uiAbsPartIdx, TEXT_LUMA, 0);
#endif
#endif
  m_pcEntropyCoder->encodeCoeff( pcCU, uiAbsPartIdx, uiDepth, pcCU->getLog2CUSize(uiAbsPartIdx) );
#if ZY_DEBUG_PRINT
  FILE* coeff_encoder = fopen("E:\\AVS3 PLATFORM\\AVS3YUQUAHE_V1+EXTENTION\\coeff_encoder.txt", "at+");
  UInt width = pcCU->getWidth(uiAbsPartIdx);
  TCoeff* pCoeff = pcCU->getCoeffY();
  TCoeff* pCoeffCb = pcCU->getCoeffCb();
  TCoeff* pCoeffCr = pcCU->getCoeffCr();
  if (pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N)
  {
	  fprintf(coeff_encoder, "pos: (%d, %d), size: %d, uiAbsPartIdx: %d, partSize: %d, LumaDir: %d, ChromaDir: %d\n", pcCU->getCUPelX(), pcCU->getCUPelY(), pcCU->getWidth(uiAbsPartIdx), uiAbsPartIdx, pcCU->getPartitionSize(uiAbsPartIdx), pcCU->getIntraDir(uiAbsPartIdx), pcCU->getIntraDirCb(uiAbsPartIdx));
  }
  else
  {
	  fprintf(coeff_encoder, "pos: (%d, %d), size: %d, uiAbsPartIdx: %d, partSize: %d, LumaDir: %d %d %d %d, ChromaDir: %d\n", pcCU->getCUPelX(), pcCU->getCUPelY(), pcCU->getWidth(uiAbsPartIdx), uiAbsPartIdx, pcCU->getPartitionSize(uiAbsPartIdx), pcCU->getIntraDir(uiAbsPartIdx),pcCU->getIntraDir(uiAbsPartIdx+1),pcCU->getIntraDir(uiAbsPartIdx+2),pcCU->getIntraDir(uiAbsPartIdx+3), pcCU->getIntraDirCb(uiAbsPartIdx));
  }
  //fprintf(coeff_encoder, "pos: (%d, %d), size: %d, uiAbsPartIdx: %d, partSize: %d, LumaDir: %d, ChromaDir: %d\n", pcCU->getCUPelX(), pcCU->getCUPelY(), pcCU->getWidth(uiAbsPartIdx), uiAbsPartIdx, pcCU->getPartitionSize(uiAbsPartIdx), pcCU->getIntraDir(uiAbsPartIdx), pcCU->getIntraDirCb(uiAbsPartIdx));
  for (UInt y = 0; y < width; y++)
  {
	  for (UInt x = 0; x < width; x++)
	  {
		  fprintf(coeff_encoder, "%d ", pCoeff[y * width + x]);
	  }
	  fputc('\n', coeff_encoder);
  }
  UInt widthC = width / 2;
  for (UInt y = 0; y < widthC; y++)
  {
	  for (UInt x = 0; x < widthC; x++)
	  {
		  fprintf(coeff_encoder, "%d ", pCoeffCb[y * widthC + x]);
	  }
	  fputc('\n', coeff_encoder);
  }
  for (UInt y = 0; y < widthC; y++)
  {
	  for (UInt x = 0; x < widthC; x++)
	  {
		  fprintf(coeff_encoder, "%d ", pCoeffCr[y * widthC + x]);
	  }
	  fputc('\n', coeff_encoder);
  }
  fputc('\n', coeff_encoder);
  fclose(coeff_encoder);
#endif
  // Encode deblock flags
  m_pcEntropyCoder->encodeDBKIdx(pcCU, uiAbsPartIdx);
}

//test
UInt curr_cbfY = 0;
UInt curr_cbfU = 0;
UInt curr_cbfV = 0;

//Void TEncCu::xCheckRDCostSkip( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiSplitbits )
Double TEncCu::xCheckRDCostSkip(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double minRdCostD, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag)
{	
	#if	inter_direct_skip_bug1
  for (UInt i = 0; i < rpcTempCU->getPicture()->getPicHeader()->getInterSkipmodeNumber(); i++)
  {
	  if (i)
		  Flag = false;
#endif
	  
	  UInt uiDepth = rpcTempCU->getDepth(0);
	  
	  rpcTempCU->setInterSkipmodeSubParts(i, 0, uiDepth);

	  rpcTempCU->setDepthSubParts(uiDepth, 0);
	  rpcTempCU->setPredModeSubParts(MODE_DIRECT, 0, uiDepth);
	  rpcTempCU->setPartSizeSubParts(SIZE_2Nx2N, 0, uiDepth);
	 
	  m_pcPredSearch->predSkipSearch(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp);
	  m_pcPredSearch->encodeResAndCalcRdInterCU(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp, m_pcResiYuvTemp, m_pcResiYuvBest, m_pcRecoYuvTemp, true);
	  if (Flag)
		  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion()) + split_cost_up;
	  else
		  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion()) + split_cost_up;

	  if (!(minRdCostD > 1e100) && Flag)
	  {
		  rpcBestCU->getTotalCost() = minRdCostD + split_cost_down;
	  }
	  else
		  rpcBestCU->getTotalCost() = minRdCostD;

	  //test
	  curr_cbfY = rpcTempCU->getCbf(0,TEXT_LUMA,0);
	  curr_cbfU = rpcTempCU->getCbf(0, TEXT_CHROMA_U, 0);
	  curr_cbfV = rpcTempCU->getCbf(0, TEXT_CHROMA_V, 0);

#if DIRECTSKIP_BUG_YQH
	  if (!(rpcTempCU->getCbf(0, TEXT_LUMA, 0) || rpcTempCU->getCbf(0, TEXT_CHROMA_U, 0) || rpcTempCU->getCbf(0, TEXT_CHROMA_V, 0)))
	  {
		  rpcTempCU->setPredModeSubParts(MODE_SKIP, 0, uiDepth);
		  rpcTempCU->setPartSizeSubParts(SIZE_2Nx2N, 0, uiDepth);

	  }
#endif

	  minRdCostD = xCheckBestMode(rpcBestCU, rpcTempCU, splitRdCost, uiDepth);
	  rpcTempCU->initEstData(uiDepth);
#if	inter_direct_skip_bug1
  }
#endif
  return minRdCostD;

}

//test
extern UInt curr_cbfY;
extern UInt curr_cbfU;
extern UInt curr_cbfV;

#if inter_direct_skip_bug
Double TEncCu::xCheckRDCostSkip_NO_QT(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double minRdCostD, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag)
//Void TEncCu::xCheckRDCostSkip_NO_QT(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiSplitbits)
{
	if (curr_cbfY || curr_cbfU || curr_cbfV)
	//if (rpcBestCU->getCbf(0, TEXT_LUMA, 0) || rpcBestCU->getCbf(0, TEXT_CHROMA_U, 0) || rpcBestCU->getCbf(0, TEXT_CHROMA_V, 0))
	{
#if	inter_direct_skip_bug1
		for (UInt i = 0; i < rpcTempCU->getPicture()->getPicHeader()->getInterSkipmodeNumber(); i++)
		{
			
#endif
			UInt uiDepth = rpcTempCU->getDepth(0);

			rpcTempCU->setInterSkipmodeSubParts(i, 0, uiDepth);

			rpcTempCU->setDepthSubParts(uiDepth, 0);
			rpcTempCU->setPredModeSubParts(MODE_SKIP, 0, uiDepth);
			rpcTempCU->setPartSizeSubParts(SIZE_2Nx2N, 0, uiDepth);

		
			m_pcPredSearch->predSkipSearch(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp);
			m_pcPredSearch->encodeResAndCalcRdInterCU_NO_QT(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp, m_pcResiYuvTemp, m_pcResiYuvBest, m_pcRecoYuvTemp, true);
			if (Flag)
				rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion()) + split_cost_up;
			else
				rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion()) + split_cost_up;


			if (!(minRdCostD > 1e100) && Flag)
			{
				rpcBestCU->getTotalCost() = minRdCostD + split_cost_down;
			}
			else
				rpcBestCU->getTotalCost() = minRdCostD;

			minRdCostD = xCheckBestMode(rpcBestCU, rpcTempCU, splitRdCost, uiDepth);
			rpcTempCU->initEstData(uiDepth);
		}
		return minRdCostD;
#if	inter_direct_skip_bug1
	}
#endif
	return minRdCostD;

}

#endif

#if B_MHBSKIP_SYC
Double TEncCu::xCheckRDCostMHSkip(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double minRdCostD, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag)
{
	UInt uiRecallNUm = (rpcTempCU->getPicture()->getPictureType() == F_PICTURE) ? MH_PSKIP_NUM : DIRECTION;

	for (UInt i = 0; i < uiRecallNUm; i++)
	{
#if B_MHBSKIP_SYC
		if (rpcTempCU->getPicture()->getPictureType() == B_PICTURE && i == 2)
		{
			i++;//no SYM
		}
#endif

		UInt uiDepth = rpcTempCU->getDepth(0);

		rpcTempCU->setInterSkipmodeSubParts(4 + i, 0, uiDepth);

		rpcTempCU->setDepthSubParts(uiDepth, 0);
		rpcTempCU->setPredModeSubParts(MODE_DIRECT, 0, uiDepth);
		rpcTempCU->setPartSizeSubParts(SIZE_2Nx2N, 0, uiDepth);

		m_pcPredSearch->predSkipSearch(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp);
		m_pcPredSearch->encodeResAndCalcRdInterCU(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp, m_pcResiYuvTemp, m_pcResiYuvBest, m_pcRecoYuvTemp, true);
		if (Flag)
			rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion()) + split_cost_up;
		else
			rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion()) + split_cost_up;

		if (!(minRdCostD > 1e100) && Flag)
		{
			rpcBestCU->getTotalCost() = minRdCostD + split_cost_down;
		}
		else
			rpcBestCU->getTotalCost() = minRdCostD;

		//test
		curr_cbfY = rpcTempCU->getCbf(0, TEXT_LUMA, 0);
		curr_cbfU = rpcTempCU->getCbf(0, TEXT_CHROMA_U, 0);
		curr_cbfV = rpcTempCU->getCbf(0, TEXT_CHROMA_V, 0);

#if DIRECTSKIP_BUG_YQH
		if (!(rpcTempCU->getCbf(0, TEXT_LUMA, 0) || rpcTempCU->getCbf(0, TEXT_CHROMA_U, 0) || rpcTempCU->getCbf(0, TEXT_CHROMA_V, 0)))
		{
			rpcTempCU->setPredModeSubParts(MODE_SKIP, 0, uiDepth);
			rpcTempCU->setPartSizeSubParts(SIZE_2Nx2N, 0, uiDepth);

		}
#endif

		minRdCostD = xCheckBestMode(rpcBestCU, rpcTempCU, splitRdCost, uiDepth);
		rpcTempCU->initEstData(uiDepth);
	}

	return minRdCostD;

}
Double TEncCu::xCheckRDCostMHSkip_NO_QT(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double minRdCostD, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag)
//Void TEncCu::xCheckRDCostSkip_NO_QT(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiSplitbits)
{
	UInt uiRecallNUm = (rpcTempCU->getPicture()->getPictureType() == F_PICTURE) ? MH_PSKIP_NUM : DIRECTION;
	if (curr_cbfY || curr_cbfU || curr_cbfV)
		//if (rpcBestCU->getCbf(0, TEXT_LUMA, 0) || rpcBestCU->getCbf(0, TEXT_CHROMA_U, 0) || rpcBestCU->getCbf(0, TEXT_CHROMA_V, 0))
	{
		for (UInt i = 0; i < uiRecallNUm; i++)
		{
#if B_MHBSKIP_SYC
			if (rpcTempCU->getPicture()->getPictureType() == B_PICTURE && i == 2)
			{
				i++;//no SYM
			}
#endif
			UInt uiDepth = rpcTempCU->getDepth(0);

			rpcTempCU->setInterSkipmodeSubParts(4 + i, 0, uiDepth);

			rpcTempCU->setDepthSubParts(uiDepth, 0);
			rpcTempCU->setPredModeSubParts(MODE_SKIP, 0, uiDepth);
			rpcTempCU->setPartSizeSubParts(SIZE_2Nx2N, 0, uiDepth);


			m_pcPredSearch->predSkipSearch(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp);
			m_pcPredSearch->encodeResAndCalcRdInterCU_NO_QT(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp, m_pcResiYuvTemp, m_pcResiYuvBest, m_pcRecoYuvTemp, true);
			if (Flag)
				rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion()) + split_cost_up;
			else
				rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion()) + split_cost_up;


			if (!(minRdCostD > 1e100) && Flag)
			{
				rpcBestCU->getTotalCost() = minRdCostD + split_cost_down;
			}
			else
				rpcBestCU->getTotalCost() = minRdCostD;

			minRdCostD = xCheckBestMode(rpcBestCU, rpcTempCU, splitRdCost, uiDepth);
			rpcTempCU->initEstData(uiDepth);
		}
		return minRdCostD;
	}

	return minRdCostD;

}

#else
#if F_MHPSKIP_SYC
Double TEncCu::xCheckRDCostMHPSkip(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double minRdCostD, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag)
{

	for (UInt i = 0; i < MH_PSKIP_NUM; i++)
	{

		UInt uiDepth = rpcTempCU->getDepth(0);

		rpcTempCU->setInterSkipmodeSubParts(4 + i, 0, uiDepth);

		rpcTempCU->setDepthSubParts(uiDepth, 0);
		rpcTempCU->setPredModeSubParts(MODE_DIRECT, 0, uiDepth);
		rpcTempCU->setPartSizeSubParts(SIZE_2Nx2N, 0, uiDepth);

		m_pcPredSearch->predSkipSearch(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp);
		m_pcPredSearch->encodeResAndCalcRdInterCU(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp, m_pcResiYuvTemp, m_pcResiYuvBest, m_pcRecoYuvTemp, true);
		if (Flag)
			rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion()) + split_cost_up;
		else
			rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion()) + split_cost_up;

		if (!(minRdCostD > 1e100) && Flag)
		{
			rpcBestCU->getTotalCost() = minRdCostD + split_cost_down;
		}
		else
			rpcBestCU->getTotalCost() = minRdCostD;

		//test
		curr_cbfY = rpcTempCU->getCbf(0, TEXT_LUMA, 0);
		curr_cbfU = rpcTempCU->getCbf(0, TEXT_CHROMA_U, 0);
		curr_cbfV = rpcTempCU->getCbf(0, TEXT_CHROMA_V, 0);

#if DIRECTSKIP_BUG_YQH
		if (!(rpcTempCU->getCbf(0, TEXT_LUMA, 0) || rpcTempCU->getCbf(0, TEXT_CHROMA_U, 0) || rpcTempCU->getCbf(0, TEXT_CHROMA_V, 0)))
		{
			rpcTempCU->setPredModeSubParts(MODE_SKIP, 0, uiDepth);
			rpcTempCU->setPartSizeSubParts(SIZE_2Nx2N, 0, uiDepth);

		}
#endif

		minRdCostD = xCheckBestMode(rpcBestCU, rpcTempCU, splitRdCost, uiDepth);
		rpcTempCU->initEstData(uiDepth);
}

	return minRdCostD;

}
Double TEncCu::xCheckRDCostMHPSkip_NO_QT(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double minRdCostD, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag)
//Void TEncCu::xCheckRDCostSkip_NO_QT(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiSplitbits)
{
	if (curr_cbfY || curr_cbfU || curr_cbfV)
		//if (rpcBestCU->getCbf(0, TEXT_LUMA, 0) || rpcBestCU->getCbf(0, TEXT_CHROMA_U, 0) || rpcBestCU->getCbf(0, TEXT_CHROMA_V, 0))
	{
		for (UInt i = 0; i < MH_PSKIP_NUM; i++)
		{
			UInt uiDepth = rpcTempCU->getDepth(0);

			rpcTempCU->setInterSkipmodeSubParts(4 + i, 0, uiDepth);

			rpcTempCU->setDepthSubParts(uiDepth, 0);
			rpcTempCU->setPredModeSubParts(MODE_SKIP, 0, uiDepth);
			rpcTempCU->setPartSizeSubParts(SIZE_2Nx2N, 0, uiDepth);


			m_pcPredSearch->predSkipSearch(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp);
			m_pcPredSearch->encodeResAndCalcRdInterCU_NO_QT(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp, m_pcResiYuvTemp, m_pcResiYuvBest, m_pcRecoYuvTemp, true);
			if (Flag)
				rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion()) + split_cost_up;
			else
				rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion()) + split_cost_up;


			if (!(minRdCostD > 1e100) && Flag)
			{
				rpcBestCU->getTotalCost() = minRdCostD + split_cost_down;
			}
			else
				rpcBestCU->getTotalCost() = minRdCostD;

			minRdCostD = xCheckBestMode(rpcBestCU, rpcTempCU, splitRdCost, uiDepth);
			rpcTempCU->initEstData(uiDepth);
		}
		return minRdCostD;
	}

	return minRdCostD;

}
#endif
#endif
#if WLQ_CUSplitFlag_inter
#if WLQ_rdcost
	Double TEncCu::xCheckRDCostInter(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double minRdCostD, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag)
#else
Void TEncCu::xCheckRDCostInter(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag)
#endif
#else
Void TEncCu::xCheckRDCostInter( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, UInt uiSplitbits )
#endif
{
  UInt uiDepth = rpcTempCU->getDepth( 0 );

  rpcTempCU->setDepthSubParts( uiDepth, 0 );
  rpcTempCU->setPartSizeSubParts  ( ePartSize,  0, uiDepth );
  rpcTempCU->setPredModeSubParts  ( MODE_INTER, 0, uiDepth );
#if wlq_avs2x_debug
  int sign = 0;
#endif
  m_pcPredSearch->predInterSearch ( rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp );
#if DMH
	Double bestCost = MAX_DOUBLE;
	Int bestDMHMode = 0;
	if (m_pcPredSearch->checkDMHMode(rpcTempCU, 0))
	{
		for (Int dmh_mode = 0; dmh_mode < 2 * DMH_MODE_NUM - 1; dmh_mode++)
		{
			rpcTempCU->setDMHModeSubPart(dmh_mode, 0, uiDepth);
			if (m_pcPredSearch->dmhInterPred(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp, dmh_mode))
			{
				m_pcPredSearch->encodeResAndCalcRdInterCU(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp, m_pcResiYuvTemp, m_pcResiYuvBest, m_pcRecoYuvTemp, false);
			}
			if (rpcTempCU->getTotalCost() < bestCost)
			{
				bestCost = rpcTempCU->getTotalCost();
				bestDMHMode = dmh_mode;
			}
		}
		rpcTempCU->setDMHModeSubPart(bestDMHMode, 0, uiDepth);
		m_pcPredSearch->dmhInterPred(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp, bestDMHMode);
		m_pcPredSearch->encodeResAndCalcRdInterCU(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp, m_pcResiYuvTemp, m_pcResiYuvBest, m_pcRecoYuvTemp, false);
	}
	else
	{
		m_pcPredSearch->encodeResAndCalcRdInterCU(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp, m_pcResiYuvTemp, m_pcResiYuvBest, m_pcRecoYuvTemp, false);
	}
#else
	m_pcPredSearch->encodeResAndCalcRdInterCU(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp, m_pcResiYuvTemp, m_pcResiYuvBest, m_pcRecoYuvTemp, false);
#endif
#if wlq_avs2x_debug
  if (sign)
  {
	  printf("\n");
	  printf("%.16lf\n", m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion()));
	  printf("%.16lf\n", split_cost_up);
	  printf("%.16lf\n", minRdCostD);
	  printf("%.16lf\n", split_cost_down);
	  printf("%.16lf\n", splitRdCost);
	  printf("\n");
  }
#endif
#if WLQ_rdcost

  if (Flag)
	  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion()) + split_cost_up;
  else
#if niu_multimode
	  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion()) + split_cost_up;
#else
	  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion());
#endif

  if (!(minRdCostD > 1e100) && Flag)
  {
	  rpcBestCU->getTotalCost() = minRdCostD + split_cost_down;
  }
  else
	  rpcBestCU->getTotalCost() = minRdCostD;

#else
#if WLQ_CUSplitFlag_inter
  if (!(rpcBestCU->getTotalCost() > 1e100) && Flag)
  {
	  rpcBestCU->getTotalBits() += split_cost_down;
	  rpcBestCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcBestCU->getTotalBits(), rpcBestCU->getTotalDistortion());
  }
  rpcTempCU->getTotalBits() += split_cost_up;
#else
  rpcTempCU->getTotalBits() += uiSplitbits;
#endif

  rpcTempCU->getTotalCost()  = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );
#endif
#if !INTERTEST
//  printf("\n%dx%d\t", rpcTempCU->getWidth(0), rpcTempCU->getHeight(0));
  printf("bits_all :%4d\t", rpcTempCU->getTotalBits());
  printf("distortion_all :%4d\t", rpcTempCU->getTotalDistortion());
  printf("cost:%4f\n", rpcTempCU->getTotalCost());
#endif

#if WLQ_rdcost
  minRdCostD = xCheckBestMode(rpcBestCU, rpcTempCU, splitRdCost, uiDepth);
  return minRdCostD;
#else
  xCheckBestMode( rpcBestCU, rpcTempCU, uiDepth );
#endif
}

#if RD_DIRECT
Void TEncCu::xCheckRDCostDirect(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiSplitbits)
{
	UInt uiDepth = rpcTempCU->getDepth(0);

	rpcTempCU->setDepthSubParts(uiDepth, 0);
	rpcTempCU->setPredModeSubParts(MODE_DIRECT, 0, uiDepth);
	rpcTempCU->setPartSizeSubParts(SIZE_2Nx2N, 0, uiDepth);

	m_pcPredSearch->predSkipSearch(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp);
	m_pcPredSearch->encodeResAndCalcRdInterCUDirect(rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp, m_pcResiYuvTemp, m_pcResiYuvBest, m_pcRecoYuvTemp);

	rpcTempCU->getTotalBits() += uiSplitbits;

	rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion());
#if WLQ_rdcost
	xCheckBestMode(rpcBestCU, rpcTempCU, 0, uiDepth);  //0, 需斟酌
#else
	xCheckBestMode(rpcBestCU, rpcTempCU, uiDepth);
#endif

}
#endif
#if WLQ_CUSplitFlag
#if WLQ_rdcost
Double TEncCu::xCheckRDCostIntra(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double minRdCostD, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag)
#else
Void TEncCu::xCheckRDCostIntra(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, Double split_cost_down, Double split_cost_up, Double splitRdCost, Bool Flag)
#endif
#else
Void TEncCu::xCheckRDCostIntra( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize, UInt uiSplitbits )
#endif
{//rpcBestCU and rpcTempCU are all the sub-cu of LCU
  UInt uiDepth = rpcTempCU->getDepth( 0 );
  rpcTempCU->setPartSizeSubParts( ePartSize, 0, uiDepth );
  rpcTempCU->setPredModeSubParts( MODE_INTRA, 0, uiDepth );

  m_pcPredSearch->predIntraSearch( rpcTempCU, m_pcOrigYuv, m_pcPredYuvTemp, m_pcResiYuvTemp, m_pcRecoYuvTemp );

#if niu_state_test
  m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
#else
  m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
#endif

  m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[ uiDepth ][ CI_CURR_BEST ] );

  m_pcEntropyCoder->resetBits();
#if YQH_RDO_InitEstBits_BUG
  UInt bits_init = m_pcEntropyCoder->getNumberOfWrittenBits() + 0;
#endif
#if INTER_GROUP
#else
	m_pcEntropyCoder->encodePredMode(rpcTempCU, 0, true);
#endif
  m_pcEntropyCoder->encodePartSize( rpcTempCU, 0, true );
#if ZHANGYI_INTRA
  #if !YQH_INTRA
  m_pcEntropyCoder->encodeIntraPuTypeIndex(rpcTempCU, 0, true);
  #endif
#endif
  m_pcEntropyCoder->encodePredInfo( rpcTempCU, 0, true );
#if niu_rate_divide_2
  UInt a = m_pcEntropyCoder->getNumberOfWrittenBits() - bits_init;
#if niu_write_cbf
  m_pcEntropyCoder->encodeCbfY(rpcTempCU, 0, TEXT_LUMA, 0);
#else
  m_pcEntropyCoder->encodeCbf(rpcTempCU, 0, TEXT_LUMA, 0);
#endif
  UInt b = m_pcEntropyCoder->getNumberOfWrittenBits() - bits_init;
#endif
  m_pcEntropyCoder->encodeCoeff   ( rpcTempCU, 0, uiDepth, rpcTempCU->getLog2CUSize(0) );

  m_pcEntropyCoder->encodeDBKIdx  ( rpcTempCU, 0 );

#if niu_state_test
  m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);
#else
  m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);
#endif

#if YQH_RDO_InitEstBits_BUG
#if niu_rate_divide_2
#if WLQ_CUSplitFlag
  rpcTempCU->getTotalBits() =  m_pcEntropyCoder->getNumberOfWrittenBits()  - bits_init + (b-a)/2 + (b-a)/2 - (b-a);
#else
  rpcTempCU->getTotalBits() = m_pcEntropyCoder->getNumberOfWrittenBits() + uiSplitbits - bits_init + (b - a) / 2 + (b - a) / 2 - (b - a);
#endif
#else
  rpcTempCU->getTotalBits() = m_pcEntropyCoder->getNumberOfWrittenBits() + uiSplitbits - bits_init;
#endif
#else
  rpcTempCU->getTotalBits() = m_pcEntropyCoder->getNumberOfWrittenBits() + uiSplitbits;
#endif
#if WLQ_CUSplitFlag
#if WLQ_rdcost
  if (Flag)
	  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion()) + split_cost_up;
  else
	  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion());
  if (!(minRdCostD > 1e100) && Flag)
  {
	  rpcBestCU->getTotalCost() = minRdCostD + split_cost_down;
  }
  else
	  rpcBestCU->getTotalCost() = minRdCostD;
#else
  rpcTempCU->getTotalBits() += split_cost_up;
  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion());
  if (!(rpcBestCU->getTotalCost() > 1e100) && Flag)
  {
	  rpcBestCU->getTotalBits() += split_cost_down;
	  rpcBestCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcBestCU->getTotalBits(), rpcBestCU->getTotalDistortion());
  }
#endif
#else
  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );
#endif
#if WLQ_CUcost_print
  Double temp;
  if (rpcBestCU->getTotalCost() > 1e6)
  {
	  temp = 1073741824;
  }
  else
	  temp = rpcBestCU->getTotalCost() - rpcBestCU->getPicture()->getLambda()*split_cost_down;
  printf("U: %.0f\tUs: %.0f\tD: %.0f\tDs: %.0f\n", rpcTempCU->getTotalCost() - split_cost_up*rpcTempCU->getPicture()->getLambda(), split_cost_up*rpcTempCU->getPicture()->getLambda(), temp, rpcBestCU->getPicture()->getLambda()*split_cost_down);
#endif
#if WLQ_rdcost
  minRdCostD = xCheckBestMode(rpcBestCU, rpcTempCU, splitRdCost, uiDepth);
  return minRdCostD;
#else
  xCheckBestMode( rpcBestCU, rpcTempCU, uiDepth );
#endif
}


// check whether current try is the best
#if WLQ_rdcost
Double TEncCu::xCheckBestMode(TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, Double splitRdCost, UInt uiDepth, Bool bSplitMode)
#else
Void TEncCu::xCheckBestMode( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiDepth, Bool bSplitMode )
#endif
{
#if WLQ_rdcost
	//test
  if (rpcTempCU->getTotalCost() + 0 < rpcBestCU->getTotalCost() + 0)
	//if (Int(rpcTempCU->getTotalCost()*100) < (rpcBestCU->getTotalCost() >1e20 ? rpcBestCU->getTotalCost() : Int(rpcBestCU->getTotalCost()*100)))
#else
  if( rpcTempCU->getTotalCost() < rpcBestCU->getTotalCost() )
#endif
  {
    TComYuv* pcYuv;
    
    // Change Information data
    TComDataCU* pcCU = rpcBestCU;
    rpcBestCU = rpcTempCU;
    rpcTempCU = pcCU;

    if( !bSplitMode )
#if ZHANGYI_INTRA_SDIP
		if (rpcBestCU->getPredictionMode(0) == MODE_INTRA)
		{
			xCopyYuv2PicIntra(rpcBestCU->getPic(), rpcBestCU->getAddr(), rpcBestCU->getZorderIdxInCU(), uiDepth);   // Copy Yuv data to picture Yuv
		}
		else
		{
			xCopyYuv2Pic(rpcBestCU->getPic(), rpcBestCU->getAddr(), rpcBestCU->getZorderIdxInCU(), uiDepth);   // Copy Yuv data to picture Yuv
		}
#else
      xCopyYuv2Pic( rpcBestCU->getPic(), rpcBestCU->getAddr(), rpcBestCU->getZorderIdxInCU(), uiDepth );   // Copy Yuv data to picture Yuv
#endif


    pcYuv = NULL;
    pcCU  = NULL;
    
    m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]->store(m_pppcRDSbacCoder[uiDepth][CI_NEXT_BEST]);
#if WLQ_rdcost
	return rpcBestCU->getTotalCost();
#endif
  }
#if WLQ_CUSplitFlag
  else
  {
	  m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_NEXT_BEST]);
#if WLQ_rdcost
	  return  rpcBestCU->getTotalCost();
#endif
  }
#endif
}

Void TEncCu::xCopyYuv2Pic(TComPic* rpcPic, UInt uiCUAddr, UInt uiAbsZorderIdx, UInt uiDepth)
{
  m_pcRecoYuvTemp->copyToPicYuv( rpcPic->getPicYuvRec (), uiCUAddr, uiAbsZorderIdx, uiDepth, uiAbsZorderIdx );
}

#if ZHANGYI_INTRA_SDIP
Void TEncCu::xCopyYuv2PicIntra(TComPic* rpcPic, UInt uiCUAddr, UInt uiAbsZorderIdx, UInt uiDepth)
{
	/*TComDataCU* pcCU = rpcPic->getCU(uiCUAddr);
	pcCU->copyToPicLuma(pcCU, rpcPic->getPicYuvRec (), m_pcRecoYuvTemp, uiCUAddr, uiAbsZorderIdx, uiDepth, uiAbsZorderIdx);
	m_pcRecoYuvTemp->copyToPicChroma(rpcPic->getPicYuvRec(), uiCUAddr, uiAbsZorderIdx, uiDepth, uiAbsZorderIdx);*/ //zhangyiModify: 0609
	m_pcRecoYuvTemp->copyToPicYuv(rpcPic->getPicYuvRec(), uiCUAddr, uiAbsZorderIdx, uiDepth, uiAbsZorderIdx); //zhangyiModify: 0609
}
#endif