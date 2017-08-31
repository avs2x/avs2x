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

/** \file     TDecCu.cpp
    \brief    CU decoder class
*/

#include "TDecCu.h"
#if ZHANGYI_INTRA
#include "../TLibCommon/TComPrediction.h"
#include "../TLibCommon/TypeDef.h"
#endif

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TDecCu::TDecCu()
{
  m_pcYuvResi = NULL;
  m_pcYuvReco = NULL;
  m_ppcCU      = NULL;

}

TDecCu::~TDecCu()
{
}

Void TDecCu::init( TDecEntropy* pcEntropyDecoder, TComTrQuant* pcTrQuant, TComPrediction* pcPrediction)
{
  m_pcEntropyDecoder  = pcEntropyDecoder;
  m_pcTrQuant         = pcTrQuant;
  m_pcPrediction      = pcPrediction;
}

/**
 \param    uiMaxDepth    total number of allowable depth
 \param    uiMaxWidth    largest CU width
 \param    uiMaxHeight   largest CU height
 */
Void TDecCu::create( UInt uiMaxDepth, UInt uiLog2MaxCUSize )
{
  m_uiMaxDepth = uiMaxDepth+1;
  
  m_pcYuvResi = new TComYuv; m_pcYuvResi->create( uiLog2MaxCUSize );
  m_pcYuvReco = new TComYuv; m_pcYuvReco->create( uiLog2MaxCUSize );
  m_ppcCU      = new TComDataCU*[uiMaxDepth];
#if inter_direct_skip_bug2
  m_acYuvPred[0].create(g_uiLog2MaxCUSize);
  m_acYuvPred[1].create(g_uiLog2MaxCUSize);
#endif
#if F_MHPSKIP_SYC
  m_acMHPSkipYuvPred[0].create(g_uiLog2MaxCUSize);
  m_acMHPSkipYuvPred[1].create(g_uiLog2MaxCUSize);
#endif
  UInt uiNumPartitions;
  for ( UInt ui = 0; ui < m_uiMaxDepth-1; ui++ )
  {
    uiNumPartitions = 1<<( ( uiMaxDepth - ui )<<1 );
    UInt uiLog2CuSize  = uiLog2MaxCUSize - ui;
    
    m_ppcCU     [ui] = new TComDataCU; m_ppcCU     [ui]->create( uiNumPartitions, uiLog2CuSize, true );
  }
  
  // initialize partition order.
  UInt* piTmp = &g_auiZscanToRaster[0];
  initZscanToRaster(m_uiMaxDepth, 1, 0, piTmp);
  initRasterToZscan( uiLog2MaxCUSize, m_uiMaxDepth );
  
  // initialize conversion matrix from partition index to pel
  initRasterToPelXY( uiLog2MaxCUSize, m_uiMaxDepth );
}

Void TDecCu::destroy()
{
  for ( UInt ui = 0; ui < m_uiMaxDepth-1; ui++ )
  {
    m_ppcCU     [ui]->destroy(); delete m_ppcCU     [ui]; m_ppcCU     [ui] = NULL;
  }
  
  m_pcYuvResi->destroy(); delete m_pcYuvResi; m_pcYuvResi = NULL;
  m_pcYuvReco->destroy(); delete m_pcYuvReco; m_pcYuvReco = NULL;
  delete [] m_ppcCU     ; m_ppcCU      = NULL;

#if inter_direct_skip_bug2
  m_acYuvPred[0].destroy();
  m_acYuvPred[1].destroy();
#endif

}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** \param    pcCU        pointer of CU data
 \param    ruiIsLast   last data?
 */
Void TDecCu::decodeCU( TComDataCU* pcCU, UInt& ruiIsLast )
{
  // start from the top level CU
  xDecodeCU( pcCU, 0, 0 );
  
  //--- Read terminating bit ---
  m_pcEntropyDecoder->decodeTerminatingBit( ruiIsLast );
}

/** \param    pcCU        pointer of CU data
 */
Void TDecCu::decompressCU( TComDataCU* pcCU )
{
  xDecompressCU( pcCU, pcCU, 0,  0 );
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================
Int split_flag = 0;
Void TDecCu::xDecodeCU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  TComPic* pcPic = pcCU->getPic();
  UInt uiCurNumParts = pcPic->getNumPartInCU() >> (uiDepth<<1);
  UInt uiQNumParts   = uiCurNumParts>>2;
  
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (1<<(g_uiLog2MaxCUSize-uiDepth)) - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (1<<(g_uiLog2MaxCUSize-uiDepth)) - 1;
  
  if( ( uiRPelX < pcCU->getPicture()->getSPS()->getWidth() ) && ( uiBPelY < pcCU->getPicture()->getSPS()->getHeight() ) )
  {
#if YQH_SPLIGFLAG_DEC
	  if ((g_uiLog2MaxCUSize - uiDepth) >MIN_CU_SIZE_IN_BIT)
		  m_pcEntropyDecoder->decodeSplitFlag(pcCU, uiAbsPartIdx, uiDepth);
	  else
		  pcCU->setDepthSubParts(uiDepth + 0, uiAbsPartIdx);
	     
#else

	  m_pcEntropyDecoder->decodeSplitFlag(pcCU, uiAbsPartIdx, uiDepth);
#endif

  }
  else
  {
    bBoundary = true;
  }
  





  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < g_uiMaxCUDepth - 1 ) ) || bBoundary )
 
  {
    UInt uiIdx = uiAbsPartIdx;

    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++ )
    {
      uiLPelX = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiIdx] ];
      uiTPelY = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiIdx] ];
      
      if( ( uiLPelX < pcCU->getPicture()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getPicture()->getSPS()->getHeight() ) )
        xDecodeCU( pcCU, uiIdx, uiDepth+1 );
      
      uiIdx += uiQNumParts;
    }
    
    return;
  }
#if INTER_GROUP
	//AVS3
	//m_pcEntropyDecoder->decodePredMode(pcCU, uiAbsPartIdx, uiDepth);

	m_pcEntropyDecoder->decodeInterCUTypeIndex(pcCU, uiAbsPartIdx, uiDepth);
		

#else
  //m_pcEntropyDecoder->decodePredMode( pcCU, uiAbsPartIdx, uiDepth );

  //m_pcEntropyDecoder->decodePartSize( pcCU, uiAbsPartIdx, uiDepth );
#endif
//


#if niu_predmode_dec
#if  inter_intra_1
  if (pcCU->isIntra(uiAbsPartIdx))
#else
  if (pcCU->getPicture()->isIntra())
#endif
#endif
	m_pcEntropyDecoder->decodePredMode( pcCU, uiAbsPartIdx, uiDepth );


  UInt uiLog2CurrSize = pcCU->getLog2CUSize(uiAbsPartIdx);

#if niu_predmode_dec
#if	inter_intra_1
  if (pcCU->isIntra(uiAbsPartIdx))
#else
  if (pcCU->getPicture()->isIntra())
#endif
#endif
	m_pcEntropyDecoder->decodePartSize(pcCU, uiAbsPartIdx, uiDepth);


#if ZHANGYI_INTRA
#if !YQH_INTRA
  m_pcEntropyDecoder->decodeIntraPuTypeIndex(pcCU, uiAbsPartIdx, uiDepth);
#endif
#endif
  m_pcEntropyDecoder->decodePredInfo( pcCU, uiAbsPartIdx, uiDepth );

  // Coefficient decoding
  m_pcEntropyDecoder->decodeCoeff( pcCU, uiAbsPartIdx, uiDepth, uiLog2CurrSize );
#if ZY_DEBUG_PRINT
  FILE* coeff_decoder = fopen("E:\\AVS3 PLATFORM\\AVS3YUQUAHE_V1+EXTENTION\\coeff_decoder.txt", "at+");
  UInt width = pcCU->getWidth(uiAbsPartIdx);
  TCoeff* pCoeff = pcCU->getCoeffY();
  TCoeff* pCoeffCb = pcCU->getCoeffCb();
  TCoeff* pCoeffCr = pcCU->getCoeffCr();
  if (pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N)
  {
	  fprintf(coeff_decoder, "pos: (%d, %d), size: %d, uiAbsPartIdx: %d, partSize: %d, LumaDir: %d, ChromaDir: %d\n", pcCU->getCUPelX(), pcCU->getCUPelY(), pcCU->getWidth(uiAbsPartIdx), uiAbsPartIdx, pcCU->getPartitionSize(uiAbsPartIdx), pcCU->getIntraDir(uiAbsPartIdx), pcCU->getIntraDirCb(uiAbsPartIdx));
  }
  else
  {
	  fprintf(coeff_decoder, "pos: (%d, %d), size: %d, uiAbsPartIdx: %d, partSize: %d, LumaDir: %d %d %d %d, ChromaDir: %d\n", pcCU->getCUPelX(), pcCU->getCUPelY(), pcCU->getWidth(uiAbsPartIdx), uiAbsPartIdx, pcCU->getPartitionSize(uiAbsPartIdx), pcCU->getIntraDir(uiAbsPartIdx),pcCU->getIntraDir(uiAbsPartIdx+1),pcCU->getIntraDir(uiAbsPartIdx+2),pcCU->getIntraDir(uiAbsPartIdx+3), pcCU->getIntraDirCb(uiAbsPartIdx));
  }
  //fprintf(coeff_decoder, "pos: (%d, %d), size: %d, uiAbsPartIdx: %d, partSize: %d, LumaDir: %d, ChromaDir: %d\n", pcCU->getCUPelX(), pcCU->getCUPelY(), pcCU->getWidth(uiAbsPartIdx), uiAbsPartIdx, pcCU->getPartitionSize(uiAbsPartIdx), pcCU->getIntraDir(uiAbsPartIdx), pcCU->getIntraDirCb(uiAbsPartIdx));
  for (UInt y = 0; y < width; y++)
  {
	  for (UInt x = 0; x < width; x++)
	  {
		  fprintf(coeff_decoder, "%d ", pCoeff[y * width + x]);
	  }
	  fputc('\n', coeff_decoder);
  }
  UInt widthC = width / 2;
  for (UInt y = 0; y < widthC; y++)
  {
	  for (UInt x = 0; x < widthC; x++)
	  {
		  fprintf(coeff_decoder, "%d ", pCoeffCb[y * widthC + x]);
	  }
	  fputc('\n', coeff_decoder);
  }
  for (UInt y = 0; y < widthC; y++)
  {
	  for (UInt x = 0; x < widthC; x++)
	  {
		  fprintf(coeff_decoder, "%d ", pCoeffCr[y * widthC + x]);
	  }
	  fputc('\n', coeff_decoder);
  }
  fputc('\n', coeff_decoder);
  fclose(coeff_decoder);
#endif

  // deblock index
  m_pcEntropyDecoder->decodeDBKIdx(pcCU, uiAbsPartIdx);
}

Void TDecCu::xDecompressCU( TComDataCU* pcCU, TComDataCU* pcCUCur, UInt uiAbsPartIdx,  UInt uiDepth )
{
	
  TComPic* pcPic = pcCU->getPic();
  
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (1<<(g_uiLog2MaxCUSize-uiDepth))  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (1<<(g_uiLog2MaxCUSize-uiDepth)) - 1;
  
  if( ( uiRPelX >= pcCU->getPicture()->getSPS()->getWidth() ) || ( uiBPelY >= pcCU->getPicture()->getSPS()->getHeight() ) )
  {
    bBoundary = true;
  }
  
  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < g_uiMaxCUDepth - 1 ) ) || bBoundary )
  {
    UInt uiNextDepth = uiDepth + 1;
    UInt uiQNumParts = pcCU->getTotalNumPart() >> (uiNextDepth<<1);
    UInt uiIdx = uiAbsPartIdx;
    for ( UInt uiPartIdx = 0; uiPartIdx < 4; uiPartIdx++ )
    {
      uiLPelX = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiIdx] ];
      uiTPelY = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiIdx] ];
      
      if( ( uiLPelX < pcCU->getPicture()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getPicture()->getSPS()->getHeight() ) )
        xDecompressCU( pcCU, m_ppcCU[uiNextDepth], uiIdx, uiNextDepth );
      
      uiIdx += uiQNumParts;
    }
    return;
  }
  
  m_ppcCU[uiDepth]->copySubCU( pcCU, uiAbsPartIdx, uiDepth ); //把一个LCU内的CU复制到这个LCU中

  // Residual reconstruction
  m_pcYuvResi->clear(uiAbsPartIdx, 1<<m_ppcCU[uiDepth]->getLog2CUSize(0));
  
  switch( m_ppcCU[uiDepth]->getPredictionMode(0) )
  {
    case MODE_SKIP:
#if RD_DIRECT
		case MODE_DIRECT:
 #if     inter_direct_skip_bug2
			xReconInterSkipDirect(m_ppcCU[uiDepth], uiAbsPartIdx, uiDepth);
#else
			xReconInter(m_ppcCU[uiDepth], uiAbsPartIdx, uiDepth);
#endif
			xCopyToPic(m_ppcCU[uiDepth], pcPic, uiAbsPartIdx, uiDepth);
			break;
#endif
    case MODE_INTER:
      xReconInter( m_ppcCU[uiDepth], uiAbsPartIdx, uiDepth );
	  xCopyToPic( m_ppcCU[uiDepth], pcPic, uiAbsPartIdx, uiDepth );
      break;
    case MODE_INTRA:
      xReconIntra( m_ppcCU[uiDepth], uiAbsPartIdx, uiDepth );
      break;
    default:
      assert(0);
      break;
  }  
}

#if PSKIP
Void TDecCu::scalingMV(TComMv &cMvPredL0, TComMv tempmv, Int curT, Int colT)
{
	cMvPredL0.setHor(Clip3(-32768, 32767, ((long long int)(curT)* tempmv.getHor() * (MULTI / colT) + HALF_MULTI) >> OFFSET));
	cMvPredL0.setVer(Clip3(-32768, 32767, ((long long int)(curT)* tempmv.getVer() * (MULTI / colT) + HALF_MULTI) >> OFFSET));
}
#endif

Void TDecCu::xReconInter(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	// inter prediction
#if DMH
	if (pcCU->getDMHMode(0))
	{
		m_pcPrediction->recDmhMode(pcCU, uiAbsPartIdx, REF_PIC_0, m_pcYuvReco);
	}
	else
	{
#endif
	m_pcPrediction->motionCompensation(pcCU, m_pcYuvReco);
#if DMH
	}
#endif
	// inter recon
	xDecodeInterTexture(pcCU, uiAbsPartIdx, uiDepth);

	// clip for only non-zero cbp case
	if ((pcCU->getCbf(0, TEXT_LUMA)) || (pcCU->getCbf(0, TEXT_CHROMA_U)) || (pcCU->getCbf(0, TEXT_CHROMA_V)))
	{
		m_pcYuvReco->addClip(m_pcYuvReco, m_pcYuvResi, uiAbsPartIdx, 1 << pcCU->getLog2CUSize(0));
	}
}


#if inter_direct_skip_bug2

Void TDecCu::xReconInterSkipDirect( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  // inter prediction
#if inter_direct_skip_bug2
	///////////////////////////////////////////////////////////////////
	UInt uiPartAddr = pcCU->getZorderIdxInCU();
	UInt uiSize = 1 << pcCU->getLog2CUSize(0);
#if RPS
	UInt  uiCostTemp = 0;
	UInt  uiCost = MAX_UINT;
#endif
	// clear data
	m_acYuvPred[0].clear(pcCU->getZorderIdxInCU(), uiSize);
	m_acYuvPred[1].clear(pcCU->getZorderIdxInCU(), uiSize);

	// clear motion data

#if RPS
	TComMvField cMvFieldZero;
	pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvFieldZero, pcCU->getPartitionSize(0), 0, 0, 0);
	pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMvFieldZero, pcCU->getPartitionSize(0), 0, 0, 0);
#else
	TComMv cMvZero;
	pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvZero, pcCU->getPartitionSize(0), 0, 0, 0);
	pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMvZero, pcCU->getPartitionSize(0), 0, 0, 0);
#endif


	// set motion data
	if (pcCU->getPicture()->isInterB())
	{
#if B_SKIP_ZP
#if BSKIP
		// Get_direct
	{
		Int offset;
		Int TempRef;
		TComMv TempMv;
		Int   bw_ref, TRb, TRp, TRd, TRp1;
		Int  FrameNoNextP, FrameNoB;

		Int  DeltaP[MAX_NUM_REF_PICS];
		Int numMBInblock = 1 << (pcCU->getPicture()->getSPS()->getLog2MaxCUSize() - pcCU->getDepth(0) - MIN_CU_SIZE_IN_BIT);   //qyu 0820 add 4:1 5:2 6:4
		TComCUMvField* TComCUMvField = pcCU->getPicture()->getPicHeader()->getRefPic(REF_PIC_0, 0)
			->getPicSym()->getDPBPerCtuData(pcCU->getAddr()).getCUMvField(REF_PIC_0); // fref[0]->refbuf,fref[0]->mvbuf;; 光删扫描存储方式
		TComPic* TempPic = pcCU->getPicture()->getPicHeader()->getRefPic(REF_PIC_0, 0);
		TComMv cMvPredL0;
		TComMv cMvPredL1;
		TComMvField cMvFieldPredL0;
		TComMvField cMvFieldPredL1;
		UInt  uiAbsPartIdxR = g_auiZscanToRaster[uiPartAddr];
		Int iPartIdx = 0;
		Int iRoiWidth, iRoiHeight;
#if ZP_DEBUG_829 
		UInt CurrPartNumQ = (pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(0) << 1)) >> 2;
#endif
		for (Int block_y = 0; block_y < 2; block_y++)
		{
			for (Int block_x = 0; block_x < 2; block_x++)
			{
#if ZP_DEBUG_829 
				uiPartAddr = CurrPartNumQ * iPartIdx;
#else
				pcCU->getPartIndexAndSize(iPartIdx, uiPartAddr, iRoiWidth, iRoiHeight);
#endif 
				//如何设置offset;
				offset = numMBInblock * block_x + pcCU->getPic()->getNumPartInWidth() * numMBInblock * block_y;
				TempRef = TComCUMvField->getRefIdx(uiAbsPartIdxR + offset);//应该可以用uiPartAddr代替
				TempMv = TComCUMvField->getMv(uiAbsPartIdxR + offset);
				if (TempRef = -1)
				{
					cMvPredL0 = pcCU->getMvFieldPred(0, REF_PIC_0, 0).getMv();
					cMvPredL1 = pcCU->getMvFieldPred(0, REF_PIC_1, 0).getMv();
					cMvFieldPredL0.setMvField(cMvPredL0, 0);
					cMvFieldPredL1.setMvField(cMvPredL1, 0);
					pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvFieldPredL0, SIZE_NxN, uiPartAddr, 0, 0);
					pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMvFieldPredL1, SIZE_NxN, uiPartAddr, 0, 0);
					// #if Mv_check_bug 
				}
				else
				{
					//需要定义下img->imgtr_next_P 、、hc->picture_distance  imgtr_fwRefDistance
					FrameNoNextP = 2 * pcCU->getPicture()->getPicHeader()->getTemporalReferenceNextP();
					FrameNoB = 2 * pcCU->getPicture()->getPicHeader()->getPictureDistance();

					if (TempPic->getPicture()->getPictureType() == B_PICTURE)//假设RD下B帧只有2帧可以参考
					{
						DeltaP[0] = 2 * (pcCU->getPicture()->getPicHeader()->getTemporalReferenceNextP()
							- TempPic->getPicSym()->getPicHeader()->getRefPOC(REF_PIC_0, 0));
						DeltaP[1] = 2 * (pcCU->getPicture()->getPicHeader()->getTemporalReferenceNextP()
							- TempPic->getPicSym()->getPicHeader()->getRefPOC(REF_PIC_1, 0));
					}
					else
					{
#if ZP_DEBUG_828
						for (Int i = 0; i < TempPic->getPicHeader()->getNumRefIdx(REF_PIC_0); i++)
#else
						for (Int i = 0; i++; i < TempPic->getPicHeader()->getNumRefIdx(REF_PIC_0))
#endif
						{
							DeltaP[i] = 2 * (pcCU->getPicture()->getPicHeader()->getTemporalReferenceNextP()
								- TempPic->getPicSym()->getPicHeader()->getRefPOC(REF_PIC_0, i));
						}
					}
					TRp = DeltaP[TempRef];
					TRp1 = 2 * (pcCU->getPicture()->getPicHeader()->getTemporalReferenceNextP() -
						TempPic->getPicSym()->getPicHeader()->getTemporalReferenceForwardDistance());
					TRd = FrameNoNextP - FrameNoB;
					TRb = TRp1 - TRd;
					TRp = (TRp + 512) % 512;
					TRp1 = (TRp1 + 512) % 512;
					TRd = (TRd + 512) % 512;
					TRb = (TRb + 512) % 512;
					//scalingDirectMvHor(TempMv.getHor(), TRp, TRb, TRd, &cMvPredL0, &cMvPredL1);
					if (TempMv.getHor() < 0) {
						cMvPredL0.setHor(-((long long int)(MULTI / TRp) * (1 + (-TRb) * TempMv.getHor()) - 1) >> OFFSET);
						cMvPredL1.setHor(((long long int)(MULTI / TRp) * (1 + (-TRd) * TempMv.getHor()) - 1) >> OFFSET);
					}
					else {
						cMvPredL0.setHor(((long long int)(MULTI / TRp) * (1 + TRb * TempMv.getHor()) - 1) >> OFFSET);
						cMvPredL1.setHor(-((long long int)(MULTI / TRp) * (1 + TRd * TempMv.getHor()) - 1) >> OFFSET);
					}
					//scalingDirectMvVer(TempMv.getVer(), FrameNoNextP, TRp, FrameNoB, TRb, TRd, &cMvPredL0, &cMvPredL1);
					if (TempMv.getVer() < 0) {
						cMvPredL0.setVer(-((long long int)(MULTI / TRp) * (1 + (-TRb) * TempMv.getVer()) - 1) >> OFFSET);
						cMvPredL1.setVer(((long long int)(MULTI / TRp) * (1 + (-TRd) * TempMv.getVer()) - 1) >> OFFSET);
					}
					else {
						cMvPredL0.setVer(((long long int)(MULTI / TRp) * (1 + TRb * TempMv.getVer()) - 1) >> OFFSET);
						cMvPredL1.setVer(-((long long int)(MULTI / TRp) * (1 + TRd * TempMv.getVer()) - 1) >> OFFSET);
					}
					cMvFieldPredL0.setMvField(cMvPredL0, TempRef);
					cMvFieldPredL1.setMvField(cMvPredL1, TempRef);
					pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvFieldPredL0, SIZE_NxN, uiPartAddr, 0, 0);
					pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMvFieldPredL1, SIZE_NxN, uiPartAddr, 0, 0);
				}
				iPartIdx++;
				if (pcCU->getPicture()->getSPS()->getLog2MaxCUSize() - pcCU->getDepth(0) == MIN_CU_SIZE_IN_BIT)
				{
					break;
				}
			}
			if (pcCU->getPicture()->getSPS()->getLog2MaxCUSize() - pcCU->getDepth(0) == MIN_CU_SIZE_IN_BIT)
			{
				break;
			}
		}
		if (pcCU->getPicture()->getSPS()->getLog2MaxCUSize() - pcCU->getDepth(0) == MIN_CU_SIZE_IN_BIT)
		{

			//  pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvPredL0, pcCU->getPartitionSize(0), 0, 0, 0);
			//  pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMvPredL1, pcCU->getPartitionSize(0), 0, 0, 0);
			cMvFieldPredL0.setMvField(cMvPredL0, 0);
			cMvFieldPredL1.setMvField(cMvPredL1, 0);
			pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvFieldPredL0, pcCU->getPartitionSize(0), 0, 0, 0);
			pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMvFieldPredL1, pcCU->getPartitionSize(0), 0, 0, 0);
		}
		//pcCU->setSkipMotionVectorPredictor(0);
	}
#else
#if RPS
		TComMvField cMvFieldPredL0 = pcCU->getMvFieldPred(0, REF_PIC_0, 0);
		TComMvField cMvFieldPredL1 = pcCU->getMvFieldPred(0, REF_PIC_1, 0);

		pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvFieldPredL0, pcCU->getPartitionSize(0), 0, 0, 0);
		pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMvFieldPredL1, pcCU->getPartitionSize(0), 0, 0, 0);
#else
		TComMv cMvPredL0 = pcCU->getMvPred(0, REF_PIC_0);
		TComMv cMvPredL1 = pcCU->getMvPred(0, REF_PIC_1);

		pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvPredL0, pcCU->getPartitionSize(0), 0, 0, 0);
		pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMvPredL1, pcCU->getPartitionSize(0), 0, 0, 0);
#endif

#endif
		
#else
#if B_RPS_BUG_818
		TComMvField cMvFieldPredL0;            //暂时B帧的skip模式还没有进来
		TComMvField cMvFieldPredL1;
		cMvFieldPredL0.setMvField(TComMv(), 0);
		cMvFieldPredL1.setMvField(TComMv(), 0);
#else
		TComMvField cMvFieldPredL0 = pcCU->getMvFieldPred(0, REF_PIC_0, 0);
		TComMvField cMvFieldPredL1 = pcCU->getMvFieldPred(0, REF_PIC_1, 0);
#endif

		pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvFieldPredL0, pcCU->getPartitionSize(0), 0, 0, 0);
		pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMvFieldPredL1, pcCU->getPartitionSize(0), 0, 0, 0);
#if rd_mvd
#if  B_RPS_BUG_820
		pcCU->setInterDirSubParts(1, 0, pcCU->getDepth(0), 0);
#else
		pcCU->setInterDirSubParts(INTER_BID, 0, pcCU->getDepth(0), 0);
#endif
#else
		pcCU->setInterDirSubParts(3, 0, pcCU->getDepth(0), 0);
#endif


#endif
#if B_SKIP_ZP
		uiPartAddr = pcCU->getZorderIdxInCU();
		pcCU->setSkipMotionVectorPredictor(0);
		pcCU->setInterDirSubParts(3, 0, pcCU->getDepth(0), 0);
#endif
	}
	else
	{

#if PSKIP
	{
		TComMv cMvPredL0;
		TComMvField cMvFieldPredL0;// = pcCU->getMvFieldPred(0, REF_PIC_0, 0);
		//	pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvFieldPredL0, pcCU->getPartitionSize(0), 0, 0, 0);

		TComCUMvField* ColCUMvField = pcCU->getPicture()->getPicHeader()->getRefPic(REF_PIC_0, 0)
			->getPicSym()->getDPBPerCtuData(pcCU->getAddr()).getCUMvField(REF_PIC_0); //col_ref col_mv 光删扫描存储方式


		//TComMvField testpred;
		//testpred = pcCU->getMvFieldPred(uiAbsPartIdx, eRefPic, iRefIdx);
		//pAboveCU->getCUMvField(eRefPic)->getMv(uiAboveIdx)

		//	TComCUMvField* ColCUMvField = pcCU->getCUMvField(REF_PIC_0);


		Int ColRef = 0;
		TComMv Colmv;
		Int offset = 0;
		Int curT, colT;
		Int delta1, delta2;
		delta1 = delta2 = 0;
		//uiPartAddr
		//uiBitSize = 
		UInt blockshape_block_x, blockshape_block_y;
		UInt  uiAbsPartIdxR = g_auiZscanToRaster[uiPartAddr];
		blockshape_block_x = pcCU->getWidth(0) >> MIN_BLOCK_SIZE_IN_BIT;
		blockshape_block_y = pcCU->getHeight(0) >> MIN_BLOCK_SIZE_IN_BIT;
		Int iPartIdx = 0;
		Int iRoiWidth, iRoiHeight;
#if ZP_DEBUG_829 
		UInt CurrPartNumQ = (pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(0) << 1)) >> 2;
#endif
		for (Int i = 0; i < 2; i++)
		{
			for (Int j = 0; j < 2; j++)
			{
#if ZP_DEBUG_829 
				uiPartAddr = CurrPartNumQ * iPartIdx;
#else
				pcCU->getPartIndexAndSize(iPartIdx, uiPartAddr, iRoiWidth, iRoiHeight);
#endif 
				//如何设置offset;
				offset = blockshape_block_x / 2 * i + pcCU->getPic()->getNumPartInWidth() * blockshape_block_y / 2 * j;
				ColRef = ColCUMvField->getRefIdx(uiAbsPartIdxR + offset);
				Colmv = ColCUMvField->getMv(uiAbsPartIdxR + offset);


				if (ColRef >= 0)
				{
					curT = (2 * (pcCU->getPicture()->getPOC() - pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_0, 0)) + 512) % 512;
					colT = (2 * (pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_0, 0) -
						pcCU->getPicture()->getPicHeader()->getRefPic(REF_PIC_0, 0)->getPicHeader()->getRefPOC(REF_PIC_0, ColRef)) + 512) % 512;
					/* if (0 == img->num_of_references - 1 && he->background_reference_enable) {
					curT = 1;
					colT = 1;
					}
					if (refframe == img->num_of_references - 1 && he->background_reference_enable) {
					colT = 1;
					}*/ // 暂无



					scalingMV(cMvPredL0, Colmv, curT, colT);
					cMvPredL0.setVer(cMvPredL0.getVer() - delta2);
					cMvFieldPredL0.setMvField(cMvPredL0, 0);
				}
				else
				{
					cMvPredL0.setZero();
					cMvFieldPredL0.setMvField(cMvPredL0, 0);
				}
				pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvFieldPredL0, SIZE_NxN, uiPartAddr, 0, 0);
				iPartIdx++;
			}
		}
	}  //完成函数 PskipMV_COL
	//setPSkipMotionVector
	{
		uiPartAddr = pcCU->getZorderIdxInCU();
		pcCU->setPSkipMvField(0); // 2NX2N

	}
#endif
#if rd_mvd
	pcCU->setInterDirSubParts(INTER_FORWARD, 0, pcCU->getDepth(0), 0);
#else
	pcCU->setInterDirSubParts(1, 0, pcCU->getDepth(0), 0);
#endif
	}
	/////////////////////////////////////////////////////////////////
#if 	inter_direct_skip_bug3
#if	inter_direct_skip_bug4
	if (pcCU->getPic()->getPicture()->getPictureType() != B_PICTURE)
#else
	if (pcCU->getPic()->getPicture()->getPictureType() == F_PICTURE)
#endif
#else
	if (((pcCU->getPredictionMode(uiAbsPartIdx) == MODE_DIRECT) || (pcCU->getPredictionMode(uiAbsPartIdx) == MODE_SKIP)))
#endif

	{

#if	inter_direct_skip_bug3
		Int           iNumPredDir = pcCU->getPicture()->isInterB() ? 2 : 1;
#else
		Int           iNumPredDir = pcCU->getPicture()->isInterP() ? 1 : 2;
#endif
		Int           iRoiWidth, iRoiHeight;
		//UInt uiPartAddr = pcCU->getZorderIdxInCU();
		//UInt uiSize = 1 << pcCU->getLog2CUSize(0);
		m_acYuvPred[0].clear(pcCU->getZorderIdxInCU(), uiSize);
		m_acYuvPred[1].clear(pcCU->getZorderIdxInCU(), uiSize);
		pcCU->getPartIndexAndSize(0, uiPartAddr, iRoiWidth, iRoiHeight);
		//	if (pcCU->getPic()->getPicture()->getPictureType() == P_PICTURE)
#if F_MHPSKIP_SYC
		if (pcCU->getInterSkipmode(uiPartAddr) > 3)//MHPSKIP
		{
			Int refIdx[2];
			TComMv tempMV[2];
#if F_MHPSKIP_SYC_FIXED_MV
			switch (pcCU->getInterSkipmode(uiPartAddr) - 3)
			{
			case  BID_P_FST:
				refIdx[0] = 0;//pcCU->getTmpFirstMvPred(BID_P_FST).getRefIdx();
				tempMV[0] = TComMv(0, 0);// pcCU->getTmpFirstMvPred(BID_P_FST).getMv();
				refIdx[1] = 0;//pcCU->getTmpSecondMvPred(BID_P_FST).getRefIdx();
				tempMV[1] = TComMv(0, 0);//pcCU->getTmpSecondMvPred(BID_P_FST).getMv();
				break;
			case BID_P_SND:
				refIdx[0] = 0;//pcCU->getTmpFirstMvPred(BID_P_SND).getRefIdx();
				tempMV[0] = TComMv(0, 0);//pcCU->getTmpFirstMvPred(BID_P_SND).getMv();
				refIdx[1] = 0;//pcCU->getTmpSecondMvPred(BID_P_SND).getRefIdx();
				tempMV[1] = TComMv(0, 0);//pcCU->getTmpSecondMvPred(BID_P_SND).getMv();
				break;
			case FW_P_FST:
				refIdx[0] = 0;//pcCU->getTmpFirstMvPred(FW_P_FST).getRefIdx();
				tempMV[0] = TComMv(0, 0);//pcCU->getTmpFirstMvPred(FW_P_FST).getMv();
				break;
			case FW_P_SND:
				refIdx[0] = 0;//pcCU->getTmpFirstMvPred(FW_P_FST).getRefIdx();
				tempMV[0] = TComMv(0, 0);//pcCU->getTmpFirstMvPred(FW_P_FST).getMv();
				break;
			default:
				assert(0);
			}
#else
			switch (pcCU->getInterSkipmode(uiPartAddr) - 3)
			{
			case  BID_P_FST:
				refIdx[0] = pcCU->getTmpFirstMvPred(BID_P_FST).getRefIdx();
				tempMV[0] = pcCU->getTmpFirstMvPred(BID_P_FST).getMv();
				refIdx[1] = pcCU->getTmpSecondMvPred(BID_P_FST).getRefIdx();
				tempMV[1] = pcCU->getTmpSecondMvPred(BID_P_FST).getMv();
				break;
			case BID_P_SND:
				refIdx[0] = pcCU->getTmpFirstMvPred(BID_P_SND).getRefIdx();
				tempMV[0] = pcCU->getTmpFirstMvPred(BID_P_SND).getMv();
				refIdx[1] = pcCU->getTmpSecondMvPred(BID_P_SND).getRefIdx();
				tempMV[1] = pcCU->getTmpSecondMvPred(BID_P_SND).getMv();
				break;
			case FW_P_FST:
				refIdx[0] = pcCU->getTmpFirstMvPred(FW_P_FST).getRefIdx();
				tempMV[0] = pcCU->getTmpFirstMvPred(FW_P_FST).getMv();
				break;
			case FW_P_SND:
#if F_DEBUG_828
				refIdx[0] = pcCU->getTmpFirstMvPred(FW_P_SND).getRefIdx();
				tempMV[0] = pcCU->getTmpFirstMvPred(FW_P_SND).getMv();
#else
				refIdx[0] = pcCU->getTmpFirstMvPred(FW_P_FST).getRefIdx();
				tempMV[0] = pcCU->getTmpFirstMvPred(FW_P_FST).getMv();
#endif
				break;
			default:
				assert(0);
			}
#endif


			if (pcCU->getInterSkipmode(uiPartAddr) - 3 < FW_P_FST)
			{
				pcCU->setInterDirSubParts(INTER_FORWARD, uiPartAddr, pcCU->getDepth(0), 0);
				pcCU->getCUMvField(REF_PIC_0)->setAllMv(tempMV[0], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(refIdx[0], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);

				m_pcPrediction->motionCompensation(pcCU, &m_acMHPSkipYuvPred[REF_PIC_0], REF_PIC_0, 0);
#if F_L1_FOR_MHPSKIP_SYC
				pcCU->setInterDirSubParts(INTER_BACKWARD, uiPartAddr, pcCU->getDepth(0), 0);
				pcCU->getCUMvField(REF_PIC_1)->setAllMv(tempMV[1], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				pcCU->getCUMvField(REF_PIC_1)->setAllRefIdx(refIdx[1], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				m_pcPrediction->motionCompensation(pcCU, &m_acMHPSkipYuvPred[REF_PIC_1], REF_PIC_1, 0);
#else
				pcCU->setInterDirSubParts(INTER_FORWARD, uiPartAddr, pcCU->getDepth(0), 0);
				pcCU->getCUMvField(REF_PIC_0)->setAllMv(tempMV[1], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(refIdx[1], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				m_pcPrediction->motionCompensation(pcCU, &m_acMHPSkipYuvPred[REF_PIC_1], REF_PIC_0, 0);
#endif
				m_pcYuvReco->addAvg(&m_acMHPSkipYuvPred[0], &m_acMHPSkipYuvPred[1], uiPartAddr + pcCU->getZorderIdxInCU(), iRoiWidth, iRoiHeight, uiPartAddr + pcCU->getZorderIdxInCU());
			}
			else
			{
				pcCU->setInterDirSubParts(INTER_FORWARD, uiPartAddr, pcCU->getDepth(0), 0);
				pcCU->getCUMvField(REF_PIC_0)->setAllMv(tempMV[0], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(refIdx[0], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);

				m_pcPrediction->motionCompensation(pcCU, &m_acMHPSkipYuvPred[REF_PIC_0], REF_PIC_0, 0);

				m_acMHPSkipYuvPred[REF_PIC_0].copyPartToPartYuv(m_pcYuvReco, uiPartAddr + pcCU->getZorderIdxInCU(), uiPartAddr + pcCU->getZorderIdxInCU(), iRoiWidth, iRoiHeight);
			}

		}
		else //WSM
		{
#endif
			for (Int iRefPic = 0; iRefPic < iNumPredDir; iRefPic++)
			{
				RefPic eRefPic = (iRefPic ? REF_PIC_1 : REF_PIC_0); // llt
				Int w = pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic);
				// pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic)
				for (Int iRefIdxTemp = 0; iRefIdxTemp < 2; iRefIdxTemp++)
				{
					pcCU->setInterDirSubParts(iRefPic + 1, uiPartAddr, pcCU->getDepth(0), 0);
					Int refIdx = (iRefIdxTemp == 0 ? 0 : pcCU->getInterSkipmode(uiPartAddr));

					pcCU->getCUMvField(eRefPic)->setAllRefIdx(refIdx, pcCU->getPartitionSize(0), uiPartAddr, 0, 0);

					TComYuv*  pcYuvPred = &m_acYuvPred[iRefIdxTemp];
					m_pcPrediction->motionCompensation(pcCU, pcYuvPred);

				}//end of refIdxnum
				Int refIdx = 0;
				pcCU->getCUMvField(eRefPic)->setAllRefIdx(refIdx, pcCU->getPartitionSize(0), uiPartAddr, 0, 0);

				if (pcCU->getInterSkipmode(uiPartAddr) != 0)
				{
					m_pcYuvReco->addAvg(&m_acYuvPred[0], &m_acYuvPred[1], uiPartAddr + pcCU->getZorderIdxInCU(), iRoiWidth, iRoiHeight, uiPartAddr + pcCU->getZorderIdxInCU());
				}
				else
				{
					TComYuv*  pcYuvPred_t = &m_acYuvPred[0];
					pcYuvPred_t->copyPartToPartYuv(m_pcYuvReco, uiPartAddr + pcCU->getZorderIdxInCU(), uiPartAddr + pcCU->getZorderIdxInCU(), iRoiWidth, iRoiHeight);
				}
			}//end of reflist
#if F_MHPSKIP_SYC
		}
#endif
	}
#if B_MHBSKIP_SYC
		else if (pcCU->getInterSkipmode(uiPartAddr) > 3)//MHBSKIP
		{

			Int refIdx[2];
			TComMv tempMV[2];

			switch (pcCU->getInterSkipmode(uiPartAddr) - 3)
			{
			case  DS_BID:
				refIdx[0] = 0;
				tempMV[0] = pcCU->getTempForwardBSkipMvPred(DS_BID);
				refIdx[1] = 0;
				tempMV[1] = pcCU->getTempBackwardBSkipMvPred(DS_BID);
				pcCU->setInterDirSubParts(INTER_BID, uiPartAddr, pcCU->getDepth(0), 0);
				pcCU->getCUMvField(REF_PIC_0)->setAllMv(tempMV[0], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(refIdx[0], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				pcCU->getCUMvField(REF_PIC_1)->setAllMv(tempMV[1], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				pcCU->getCUMvField(REF_PIC_1)->setAllRefIdx(refIdx[1], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				m_pcPrediction->motionCompensation(pcCU, m_pcYuvReco, REF_PIC_X, 0);
				break;
			case DS_BACKWARD:
				refIdx[1] = 0;
				tempMV[1] = pcCU->getTempBackwardBSkipMvPred(DS_BACKWARD);
				pcCU->setInterDirSubParts(INTER_BACKWARD, uiPartAddr, pcCU->getDepth(0), 0);
				pcCU->getCUMvField(REF_PIC_1)->setAllMv(tempMV[1], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				pcCU->getCUMvField(REF_PIC_1)->setAllRefIdx(refIdx[1], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				m_pcPrediction->motionCompensation(pcCU, m_pcYuvReco, REF_PIC_1, 0);
				break;
			case DS_SYM:
				refIdx[0] = 0;
				tempMV[0] = pcCU->getTempForwardBSkipMvPred(DS_SYM);
				refIdx[1] = 0;
				tempMV[1] = pcCU->getTempBackwardBSkipMvPred(DS_SYM);
				pcCU->setInterDirSubParts(INTER_BID, uiPartAddr, pcCU->getDepth(0), 0);//DS_SYM
				pcCU->getCUMvField(REF_PIC_0)->setAllMv(tempMV[0], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(refIdx[0], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				pcCU->getCUMvField(REF_PIC_1)->setAllMv(tempMV[1], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				pcCU->getCUMvField(REF_PIC_1)->setAllRefIdx(refIdx[1], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				m_pcPrediction->motionCompensation(pcCU, m_pcYuvReco, REF_PIC_X, 0);
				break;
			case DS_FORWARD:
				refIdx[0] = 0;
				tempMV[0] = pcCU->getTempForwardBSkipMvPred(DS_FORWARD);
				pcCU->setInterDirSubParts(INTER_FORWARD, uiPartAddr, pcCU->getDepth(0), 0);
				pcCU->getCUMvField(REF_PIC_0)->setAllMv(tempMV[0], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(refIdx[0], pcCU->getPartitionSize(0), uiPartAddr, 0, 0);
				m_pcPrediction->motionCompensation(pcCU, m_pcYuvReco, REF_PIC_0, 0);
				break;
			default:
				assert(0);
			}
		}
#endif
		else
		m_pcPrediction->motionCompensation(pcCU, m_pcYuvReco);




#else

  m_pcPrediction->motionCompensation( pcCU, m_pcYuvReco );
#endif
  // inter recon



  xDecodeInterTexture( pcCU, uiAbsPartIdx, uiDepth );




  // clip for only non-zero cbp case
  if  ( ( pcCU->getCbf( 0, TEXT_LUMA ) ) || ( pcCU->getCbf( 0, TEXT_CHROMA_U ) ) || ( pcCU->getCbf(0, TEXT_CHROMA_V ) ) )
  {
    m_pcYuvReco->addClip( m_pcYuvReco, m_pcYuvResi, uiAbsPartIdx, 1<<pcCU->getLog2CUSize( 0 ) );
  }
}
#endif
#if ZHANGYI_INTRA
Void TDecCu::xIntraLumaBlk(UInt uiPU, TComDataCU* pcCU,
#else
Void
	TDecCu::xIntraLumaBlk(TComDataCU* pcCU,
#endif
                      UInt        uiAbsPartIdx,
                      TComYuv*    pcRecoYuv,
                      TComYuv*    pcPredYuv, 
                      TComYuv*    pcResiYuv,
                      UInt        uiPartOffset)
{
  UInt    uiPartDepth       = (pcCU->getPartitionSize(0) == SIZE_2Nx2N) ? 0 : 1;
  UInt    uiLog2Size        = pcCU     ->getLog2CUSize( 0 ) - uiPartDepth;
  UInt    uiSize            = 1<<uiLog2Size;
  UInt    uiStride          = pcRecoYuv->getStride  ();

  UInt    uiMinCoeffSize    = 1<<(pcCU->getPic()->getLog2MinCUSize()<<1);
  UInt    uiCoeffOffset     = uiMinCoeffSize*(uiPartOffset);

#if ZHANGYI_INTRA_SDIP
  UInt    uiSdipFlag = pcCU->getSDIPFlag(0);
  UInt    uiSdipDir = pcCU->getSDIPDirection(0);
  UInt    uiWidth = pcCU->getWidth(0) >> (uiSdipFlag ? (uiSdipDir ? 0 : uiPartDepth * 2) : uiPartDepth);
  UInt    uiHeight = pcCU->getHeight(0) >> (uiSdipFlag ? (uiSdipDir ? uiPartDepth * 2 : 0) : uiPartDepth);
  //UInt    uiStride          = pcRecoYuv->getStride  ();
  UInt    uiLine = pcCU->convertNonSquareUnitToLine(uiPartOffset);

  Pel*    piReco = uiSdipFlag ? (pcRecoYuv->getLumaAddr(uiAbsPartIdx) + (uiSdipDir ? (uiLine * pcRecoYuv->getStride()) : uiLine)) : pcRecoYuv->getLumaAddr(uiAbsPartIdx + uiPartOffset);
  Pel*    piPred = uiSdipFlag ? (pcPredYuv->getLumaAddr(uiAbsPartIdx) + (uiSdipDir ? (uiLine * pcPredYuv->getStride()) : uiLine)) : pcPredYuv->getLumaAddr(uiAbsPartIdx + uiPartOffset);
  Pel*    piResi = uiSdipFlag ? (pcResiYuv->getLumaAddr(uiAbsPartIdx) + (uiSdipDir ? (uiLine * pcResiYuv->getStride()) : uiLine)) : pcResiYuv->getLumaAddr(uiAbsPartIdx + uiPartOffset);

#else
  Pel*    piReco            = pcRecoYuv->getLumaAddr( uiAbsPartIdx + uiPartOffset );
  Pel*    piResi            = pcResiYuv->getLumaAddr( uiAbsPartIdx + uiPartOffset );
  Pel*    piPred            = pcPredYuv->getLumaAddr( uiAbsPartIdx + uiPartOffset );

#endif
  TCoeff* pcCoeff           = pcCU->getCoeffY() + uiCoeffOffset;
  
  UInt    uiZOrder          = pcCU->getZorderIdxInCU();

#if ZHANGYI_INTRA_SDIP
  Pel*    piRecIPred = pcCU->getPic()->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), uiZOrder);
  if (uiSdipFlag)
  {
	  piRecIPred += (uiSdipDir ? (uiLine * pcCU->getPic()->getPicYuvRec()->getStride()) : uiLine);
  }
  else
  {
	  piRecIPred = pcCU->getPic()->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), uiZOrder + uiPartOffset);
  }
#else
  Pel*    piRecIPred = pcCU->getPic()->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), uiZOrder + uiPartOffset);
#endif
  UInt    uiRecIPredStride  = pcCU->getPic()->getPicYuvRec()->getStride  ();

  //===== inverse transform =====
  m_pcTrQuant->setQPforQuant      ( TEXT_LUMA );
  if ( pcCU->getCbf( uiPartOffset, TEXT_LUMA ) )
  {
#if ZHOULULU_QT
#if ZHANGYI_INTRA_MODIFY_TRANS
#if ZHOULULU_SEC_TRANS
	  m_pcTrQuant->setBlockSize(pcCU, uiLog2Size, TEXT_LUMA, uiPartOffset);
#else
	  m_pcTrQuant->setBlockSize( pcCU, uiLog2Size, TEXT_LUMA, uiAbsPartIdx );
#endif

#if ZHOULULU_SEC_TRANS
	  m_pcTrQuant->invtransformMxN    ( pcCU, piResi, uiStride, pcCoeff, uiLog2Size, TEXT_LUMA, uiPartOffset );
#else
	  m_pcTrQuant->invtransformMxN    (  pcCU, piResi, uiStride, pcCoeff, uiLog2Size, TEXT_LUMA, uiAbsPartIdx);
#endif

#else
	  m_pcTrQuant->setBlockSize( pcCU, uiLog2Size, TEXT_LUMA, uiAbsPartIdx );
	  m_pcTrQuant->invtransformMxN(pcCU, piResi, uiStride, pcCoeff, uiLog2Size, TEXT_LUMA, uiAbsPartIdx);
#endif
#else
    m_pcTrQuant->invtransformNxN    ( piResi, uiStride, pcCoeff, uiLog2Size, TEXT_LUMA );
 #endif
  }
#if ZHANGYI_INTRA
  int p_avail[5];
  int bitDepth = pcCU->getPicture()->getSPS()->getBitDepth();
  Pel* piAdiTemp = m_pcPrediction->getpiYuvExt2g(COMPONENT_Y, PRED_BUF_UNFILTERED);
  Pel* piorg = (Pel*)malloc(sizeof(UInt));;
  int block_x = (uiPU & 1) << uiLog2Size;
  int block_y = (uiPU >> 1) << uiLog2Size;

#if ZHANGYI_INTRA_SDIP
  int pic_pix_x, pic_pix_y;
  //UInt uiWidth, uiHeight;
  //UInt uiSdipFlag = pcCU->getSDIPFlag(0);
  //UInt uiSdipDir = pcCU->getSDIPDirection(0);

  uiWidth = uiSdipFlag ? (uiSdipDir ? (uiSize << 1) : (uiSize >> 1)) : uiSize;
  uiHeight = uiSdipFlag ? (uiSdipDir ? (uiSize >> 1) : (uiSize << 1)) : uiSize;
  pic_pix_x = uiSdipFlag ? (uiSdipDir ? pcCU->getCUPelX() : (pcCU->getCUPelX() + uiPU * (pcCU->getWidth(0) >> 2))) : (pcCU->getCUPelX() + block_x);
  pic_pix_y = uiSdipFlag ? (uiSdipDir ? (pcCU->getCUPelY() + uiPU * (pcCU->getHeight(0) >> 2)) : (pcCU->getCUPelY())) : (pcCU->getCUPelY() + block_y);
  m_pcPrediction->getIntraNeighborAvailabilities(pcCU, (1 << g_uiLog2MaxCUSize), pic_pix_x, pic_pix_y, uiWidth, uiHeight, p_avail);
  m_pcPrediction->fillReferenceSamples(bitDepth, pcCU, piRecIPred, piAdiTemp, uiWidth, uiHeight, uiRecIPredStride, p_avail, pic_pix_x, pic_pix_y);
  m_pcPrediction->predIntraAngAVS(TEXT_LUMA, pcCU->getIntraDir(uiPartOffset), uiStride, piPred, uiStride, uiWidth, uiHeight, p_avail[NEIGHBOR_INTRA_UP], p_avail[NEIGHBOR_INTRA_LEFT], bitDepth);
#else

  int pic_pix_x = pcCU->getCUPelX() + block_x;
  int pic_pix_y = pcCU->getCUPelY() + block_y;
#if niu_intra_pavil
  m_pcPrediction->getIntraNeighborAvailabilities(pcCU, (1 << g_uiLog2MaxCUSize), pic_pix_x, pic_pix_y, uiSize, uiSize, p_avail);
#else
  m_pcPrediction->getIntraNeighborAvailabilities(pcCU, (1<<B64X64_IN_BIT), pic_pix_x, pic_pix_y, uiSize, uiSize, p_avail);
#endif
  m_pcPrediction->fillReferenceSamples(bitDepth, pcCU, piRecIPred, piAdiTemp, uiSize, uiSize, uiRecIPredStride, p_avail, pic_pix_x, pic_pix_y);
  m_pcPrediction->predIntraAngAVS(TEXT_LUMA, pcCU->getIntraDir(uiPartOffset), uiStride, piPred, uiStride, uiSize, p_avail[NEIGHBOR_INTRA_UP], p_avail[NEIGHBOR_INTRA_LEFT], bitDepth);
#endif
#else
  m_pcPrediction->predIntra( piRecIPred, uiRecIPredStride, piPred, uiStride, uiSize, pcCU->getIntraDir(uiPartOffset) );
#endif

  //===== reconstruction =====
  Pel* pResi      = piResi;
  Pel* pReco      = piReco;
  Pel* pPred      = piPred;

#if ZHANGYI_INTRA_SDIP
  for (UInt uiY = 0; uiY < uiHeight; uiY++)
  {
	  for (UInt uiX = 0; uiX < uiWidth; uiX++)
#else
  for (UInt uiY = 0; uiY < uiSize; uiY++)
  {
	  for (UInt uiX = 0; uiX < uiSize; uiX++)
#endif
    {
      pReco    [ uiX ] = Clip( pPred[ uiX ] + pResi[ uiX ] );
    }

    pResi     += uiStride;
    pReco     += uiStride;
    pPred     += uiStride;
  }
}

Void
TDecCu::xIntraChromaBlk( TComDataCU* pcCU,
                         UInt        uiAbsPartIdx,
                         TComYuv*    pcRecoYuv,
                         TComYuv*    pcPredYuv, 
                         TComYuv*    pcResiYuv,
                         UInt        uiPartOffset )
{
  UInt      uiPartDepth       = (pcCU->getPartitionSize(0) == SIZE_2Nx2N) ? 0 : 1;
#if ZHANGYI_INTRA
  UInt      uiLog2Size        = pcCU->getLog2CUSize( 0 ) - 1;
#else
  UInt      uiLog2Size        = pcCU->getLog2CUSize( 0 ) - 1 - ( (pcCU->getLog2CUSize( 0 ) > 3) ? uiPartDepth : 0 );
#endif
  UInt      uiSize            = 1<<uiLog2Size;
  UInt      uiStride          = pcRecoYuv->getCStride ();

  UInt uiMinCoeffSize = 1<<(pcCU->getPic()->getLog2MinCUSize()<<1);
  UInt uiCoeffOffset   = uiMinCoeffSize*(uiPartOffset);

  Pel*      piRecoCb          = pcRecoYuv->getCbAddr( uiAbsPartIdx + uiPartOffset );
  Pel*      piResiCb          = pcResiYuv->getCbAddr( uiAbsPartIdx + uiPartOffset );
  Pel*      piPredCb          = pcPredYuv->getCbAddr( uiAbsPartIdx + uiPartOffset );

  Pel*      piRecoCr          = pcRecoYuv->getCrAddr( uiAbsPartIdx + uiPartOffset );
  Pel*      piResiCr          = pcResiYuv->getCrAddr( uiAbsPartIdx + uiPartOffset );
  Pel*      piPredCr          = pcPredYuv->getCrAddr( uiAbsPartIdx + uiPartOffset );

  uiCoeffOffset >>= 2;
  TCoeff*   pcCoeffCb         = pcCU->getCoeffCb() + uiCoeffOffset;
  TCoeff*   pcCoeffCr         = pcCU->getCoeffCr() + uiCoeffOffset;
  
  UInt      uiZOrder          = pcCU->getZorderIdxInCU();

  Pel*      piRecIPredCb      = pcCU->getPic()->getPicYuvRec()->getCbAddr( pcCU->getAddr(), uiZOrder + uiPartOffset );
  Pel*      piRecIPredCr      = pcCU->getPic()->getPicYuvRec()->getCrAddr( pcCU->getAddr(), uiZOrder + uiPartOffset );
  UInt      uiRecIPredStride  = pcCU->getPic()->getPicYuvRec()->getCStride();

  //===== inverse transform =====
  m_pcTrQuant->setQPforQuant  ( TEXT_CHROMA );
  if ( pcCU->getCbf( uiPartOffset, TEXT_CHROMA_U ) )
  {
#if ZHOULULU_QT
#if ZHANGYI_INTRA_MODIFY_TRANS
#if BUG_819
	  m_pcTrQuant->setBlockSize(pcCU, uiLog2Size, TEXT_CHROMA_U, 0);
#else
	  m_pcTrQuant->setBlockSize(pcCU, uiLog2Size, TEXT_CHROMA_U, uiAbsPartIdx);
#endif
	  m_pcTrQuant->invtransformMxN( pcCU,  piResiCb, uiStride, pcCoeffCb, uiLog2Size, TEXT_CHROMA_U, 0);
#else
	  m_pcTrQuant->setBlockSize( pcCU, uiLog2Size, TEXT_CHROMA_U, uiAbsPartIdx );
	  m_pcTrQuant->invtransformMxN(pcCU, piResiCb, uiStride, pcCoeffCb, uiLog2Size, TEXT_CHROMA_U, uiAbsPartIdx);
#endif
#else
    m_pcTrQuant->invtransformNxN( piResiCb, uiStride, pcCoeffCb, uiLog2Size, TEXT_CHROMA_U );
 #endif
  }
  if ( pcCU->getCbf( uiPartOffset, TEXT_CHROMA_V ) )
  {
#if ZHOULULU_QT
#if ZHANGYI_INTRA_MODIFY_TRANS
#if  BUG_819
	  m_pcTrQuant->setBlockSize(pcCU, uiLog2Size, TEXT_CHROMA_V, 0);
#else
	  m_pcTrQuant->setBlockSize(pcCU, uiLog2Size, TEXT_CHROMA_V, uiAbsPartIdx);
#endif
	  m_pcTrQuant->invtransformMxN( pcCU,  piResiCr, uiStride, pcCoeffCr, uiLog2Size, TEXT_CHROMA_V, uiAbsPartIdx);
#else
	  m_pcTrQuant->setBlockSize(pcCU, uiLog2Size, TEXT_CHROMA_V, uiAbsPartIdx);
	  m_pcTrQuant->invtransformMxN(pcCU, piResiCr, uiStride, pcCoeffCr, uiLog2Size, TEXT_CHROMA_V, uiAbsPartIdx);
#endif
#else
    m_pcTrQuant->invtransformNxN( piResiCr, uiStride, pcCoeffCr, uiLog2Size, TEXT_CHROMA_V );
#endif
  }
#if ZHANGYI_INTRA
  int p_avail[5];
  int bitDepth = pcCU->getPicture()->getSPS()->getBitDepth();
  Pel* piAdiTempCb = m_pcPrediction->getpiYuvExt2g(COMPONENT_Cb, PRED_BUF_UNFILTERED);
  Pel* piAdiTempCr = m_pcPrediction->getpiYuvExt2g(COMPONENT_Cr, PRED_BUF_UNFILTERED);
  Pel* piorg = (Pel*)malloc(sizeof(UInt));
  /*Cb*/
  UChar lumaWidth = pcCU->getWidth(0);
  UChar lumaHeight = pcCU->getHeight(0);
  int pic_pix_x = pcCU->getCUPelX();
  int pic_pix_y = pcCU->getCUPelY();
#if niu_intra_pavil
  m_pcPrediction->getIntraNeighborAvailabilities(pcCU, (1 << g_uiLog2MaxCUSize), pic_pix_x, pic_pix_y, lumaWidth, lumaHeight, p_avail);
#else
  m_pcPrediction->getIntraNeighborAvailabilities(pcCU, (1<<B64X64_IN_BIT), pic_pix_x, pic_pix_y, lumaWidth, lumaHeight, p_avail);
#endif
  m_pcPrediction->fillReferenceSamples(bitDepth, pcCU, piRecIPredCb, piAdiTempCb, lumaWidth>>1, lumaHeight>>1, uiRecIPredStride, p_avail, pic_pix_x>>1, pic_pix_y>>1);
  m_pcPrediction->fillReferenceSamples(bitDepth, pcCU, piRecIPredCr, piAdiTempCr,  lumaWidth>>1, lumaHeight>>1, uiRecIPredStride, p_avail, pic_pix_x>>1, pic_pix_y>>1);
  //change chroma pred mode into intra pred mode
  UInt uiLumaPredModeCb = pcCU->getIntraDirCb(0);
  UInt uiLumaPredModeCr = pcCU->getIntraDirCr(0);
  UInt uiChFinalModeCb;
  UInt uiChFinalModeCr;
  switch (uiLumaPredModeCb)
  {
  case INTRA_DC_PRED_CHROMA: 
	  uiChFinalModeCb = INTRA_DC_IDX;
	  break;
  case INTRA_HOR_PRED_CHROMA:
	  uiChFinalModeCb = INTRA_HOR_IDX;
	  break;
  case INTRA_VERT_PRED_CHROMA:
	  uiChFinalModeCb = INTRA_VER_IDX;
	  break;
  case INTRA_BI_PRED_CHROMA:
	  uiChFinalModeCb = INTRA_BI_IDX;
	  break;
  case INTRA_DM_PRED_CHROMA:
	  uiChFinalModeCb = pcCU->getIntraDir(0);//当前CU的第一个亮度PU的模式
	  break;
  }
  switch (uiLumaPredModeCr)
  {
  case INTRA_DC_PRED_CHROMA: 
	  uiChFinalModeCr = INTRA_DC_IDX;
	  break;
  case INTRA_HOR_PRED_CHROMA:
	  uiChFinalModeCr = INTRA_HOR_IDX;
	  break;
  case INTRA_VERT_PRED_CHROMA:
	  uiChFinalModeCr = INTRA_VER_IDX;
	  break;
  case INTRA_BI_PRED_CHROMA:
	  uiChFinalModeCr = INTRA_BI_IDX;
	  break;
  case INTRA_DM_PRED_CHROMA:
	  uiChFinalModeCr = pcCU->getIntraDir(0);//当前CU的第一个亮度PU的模式
	  break;
  }
#if ZHANGYI_INTRA_SDIP
  m_pcPrediction->predIntraAngAVS(TEXT_CHROMA_U, uiChFinalModeCb, uiStride, piPredCb, uiStride, uiSize, uiSize, p_avail[NEIGHBOR_INTRA_UP], p_avail[NEIGHBOR_INTRA_LEFT], bitDepth);
  m_pcPrediction->predIntraAngAVS(TEXT_CHROMA_V, uiChFinalModeCr, uiStride, piPredCr, uiStride, uiSize,uiSize, p_avail[NEIGHBOR_INTRA_UP], p_avail[NEIGHBOR_INTRA_LEFT], bitDepth);
#else
  m_pcPrediction->predIntraAngAVS(TEXT_CHROMA_U, uiChFinalModeCb, uiStride, piPredCb, uiStride, uiSize, p_avail[NEIGHBOR_INTRA_UP], p_avail[NEIGHBOR_INTRA_LEFT], bitDepth);
  m_pcPrediction->predIntraAngAVS(TEXT_CHROMA_V, uiChFinalModeCr, uiStride, piPredCr, uiStride, uiSize, p_avail[NEIGHBOR_INTRA_UP], p_avail[NEIGHBOR_INTRA_LEFT], bitDepth);
#endif
#else
  m_pcPrediction->predIntra( piRecIPredCb, uiRecIPredStride, piPredCb, uiStride, uiSize, pcCU->getIntraDir(uiPartOffset) );
  m_pcPrediction->predIntra( piRecIPredCr, uiRecIPredStride, piPredCr, uiStride, uiSize, pcCU->getIntraDir(uiPartOffset) );
#endif
  //===== reconstruction =====
  Pel* pResiCb    = piResiCb;
  Pel* pRecoCb    = piRecoCb;
  Pel* pPredCb    = piPredCb;
  Pel* pResiCr    = piResiCr;
  Pel* pRecoCr    = piRecoCr;
  Pel* pPredCr    = piPredCr;
  for( UInt uiY = 0; uiY < uiSize; uiY++ )
  {
    for( UInt uiX = 0; uiX < uiSize; uiX++ )
    {
      pRecoCb  [ uiX ] = Clip( pPredCb[ uiX ] + pResiCb[ uiX ] );
      pRecoCr  [ uiX ] = Clip( pPredCr[ uiX ] + pResiCr[ uiX ] );
    }
    pResiCb   += uiStride;
    pRecoCb   += uiStride;
    pPredCb   += uiStride;
    pResiCr   += uiStride;
    pRecoCr   += uiStride;
    pPredCr   += uiStride;
  }
}

Void TDecCu::xReconIntra( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiPU;
  UInt uiNumPart   = pcCU->getNumPartInter();
  UInt uiQNumParts = pcCU->getTotalNumPart() >> 2;

  UInt uiPartOffset = 0;
  for( uiPU = 0 ; uiPU < uiNumPart; uiPU++ )
  {
#if ZHANGYI_INTRA
	  xIntraLumaBlk  ( uiPU, pcCU, uiAbsPartIdx, m_pcYuvReco, m_pcYuvReco, m_pcYuvResi, uiPartOffset );
#else
	  xIntraLumaBlk  (  pcCU, uiAbsPartIdx, m_pcYuvReco, m_pcYuvReco, m_pcYuvResi, uiPartOffset );
#endif
#if !ZHANGYI_INTRA
    if ( ( pcCU->getLog2CUSize(0) > 3 ) || uiPU == 0 )
    {
      xIntraChromaBlk( pcCU, uiAbsPartIdx, m_pcYuvReco, m_pcYuvReco, m_pcYuvResi, uiPartOffset );
    }
#endif
    UInt    uiPartDepth       = (pcCU->getPartitionSize(0) == SIZE_2Nx2N) ? 0 : 1;
#if ZHANGYI_INTRA
	xCopyToPicIntra( TEXT_LUMA, pcCU, pcCU->getPic(), uiAbsPartIdx+uiPartOffset, uiDepth+uiPartDepth );
#else
	xCopyToPic( pcCU, pcCU->getPic(), uiAbsPartIdx+uiPartOffset, uiDepth+uiPartDepth );
#endif
    uiPartOffset += uiQNumParts;
  }
#if ZHANGYI_INTRA
  uiPartOffset = 0;
  xIntraChromaBlk(pcCU, uiAbsPartIdx, m_pcYuvReco, m_pcYuvReco, m_pcYuvResi, uiPartOffset);
  xCopyToPicIntra( TEXT_CHROMA, pcCU, pcCU->getPic(), uiAbsPartIdx+uiPartOffset, uiDepth+0 );
#endif
}

Void TDecCu::xCopyToPic( TComDataCU* pcCU, TComPic* pcPic, UInt uiZorderIdx, UInt uiDepth )
{
	UInt uiCUAddr = pcCU->getAddr();
	m_pcYuvReco->copyToPicYuv( pcPic->getPicYuvRec(), uiCUAddr, uiZorderIdx, uiDepth, uiZorderIdx );
	return;
}

#if ZHANGYI_INTRA
Void TDecCu::xCopyToPicIntra( TextType ttype, TComDataCU* pcCU, TComPic* pcPic, UInt uiZorderIdx, UInt uiDepth )
{
  UInt uiCUAddr = pcCU->getAddr();
  if(ttype == TEXT_LUMA) 
  {
#if ZHANGYI_INTRA_SDIP
	  pcCU->copyToPicLuma(pcCU, pcPic->getPicYuvRec(), m_pcYuvReco, uiCUAddr, uiZorderIdx, uiDepth, uiZorderIdx);
#else
	  m_pcYuvReco->copyToPicLuma(pcPic->getPicYuvRec(), uiCUAddr, uiZorderIdx, uiDepth, uiZorderIdx);
#endif
  }
  else
  {
	  m_pcYuvReco->copyToPicChroma( pcPic->getPicYuvRec(), uiCUAddr, uiZorderIdx, uiDepth, uiZorderIdx );
  }
  return;
}
#endif

Void TDecCu::xDecodeInterTexture ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt    uiLog2Size    = pcCU->getLog2CUSize ( 0 );
  UInt    uiLog2SizeC   = uiLog2Size - 1;
  TCoeff* piCoeff;
  Pel*    pResi;

  UInt uiTrIdx = pcCU->getTransformIdx(0);
  UInt uiLumaTrMode = uiTrIdx;
  UInt uiChromaTrMode;
  (uiLog2SizeC - uiTrIdx >= 2) ? uiChromaTrMode = uiTrIdx : uiChromaTrMode = uiLog2SizeC - 2;
  // UInt uiChromaTrMode = (uiLog2Size>3) ? uiTrIdx : 0;

  // Y
  piCoeff = pcCU->getCoeffY();
  pResi = m_pcYuvResi->getLumaAddr(uiAbsPartIdx);
  m_pcTrQuant->setQPforQuant( TEXT_LUMA );
  m_pcTrQuant->invRecurTransformNxN ( pcCU, 0, TEXT_LUMA, pResi, 0, m_pcYuvResi->getStride(), uiLog2Size, uiLumaTrMode, 0, piCoeff );

  // Cb and Cr
  m_pcTrQuant->setQPforQuant( TEXT_CHROMA );

  uiLog2Size--;

  piCoeff = pcCU->getCoeffCb(); pResi = m_pcYuvResi->getCbAddr(uiAbsPartIdx);
  m_pcTrQuant->invRecurTransformNxN ( pcCU, 0, TEXT_CHROMA_U, pResi, 0, m_pcYuvResi->getCStride(), uiLog2Size, uiChromaTrMode, 0, piCoeff );
  piCoeff = pcCU->getCoeffCr(); pResi = m_pcYuvResi->getCrAddr(uiAbsPartIdx);
  m_pcTrQuant->invRecurTransformNxN ( pcCU, 0, TEXT_CHROMA_V, pResi, 0, m_pcYuvResi->getCStride(), uiLog2Size, uiChromaTrMode, 0, piCoeff );

}

