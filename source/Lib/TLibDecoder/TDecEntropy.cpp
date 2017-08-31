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

/** \file     TDecEntropy.cpp
    \brief    entropy decoder class
*/

#include "TDecEntropy.h"

Void TDecEntropy::setEntropyDecoder         ( TDecSbac* p )
{
  m_pcEntropyDecoderIf = p;
}

Void TDecEntropy::decodeSplitFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseSplitFlag( pcCU, uiAbsPartIdx, uiDepth );
}

#if INTER_GROUP
Void TDecEntropy::decodeInterCUTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	m_pcEntropyDecoderIf->parseInterCUTypeIndex(pcCU, uiAbsPartIdx, uiDepth);
}

Void TDecEntropy::decodeShapeOfPartitionIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	m_pcEntropyDecoderIf->parseShapeOfPartitionIndex(pcCU, uiAbsPartIdx, uiDepth);
}

Void TDecEntropy::decodeBPUTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	assert(!pcCU->isIntra(uiAbsPartIdx));

	if (pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N)
	{
		m_pcEntropyDecoderIf->parseB2Nx2NInterDir(pcCU, uiAbsPartIdx, uiDepth);
	}
	else if ((pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxN && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_Nx2N) ||
		(pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxnU && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N))
	{
		m_pcEntropyDecoderIf->parseB2NxnNInterDir(pcCU, uiAbsPartIdx, uiDepth);
	}
}

Void TDecEntropy::decodeWeightedSkipMode(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	return;
}

Void TDecEntropy::decodeCUSubTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	return;
}

Void TDecEntropy::decodeBPUTypeIndex2(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	m_pcEntropyDecoderIf->parseBNxNInterDir(pcCU, uiAbsPartIdx, uiDepth);
}

Void TDecEntropy::decodeFPUTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	assert(!pcCU->isIntra(uiAbsPartIdx));

	//UInt uiPartOffset = (pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1)) >> 2;

	if (pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N)
	{
		m_pcEntropyDecoderIf->parseF2Nx2NInterDir(pcCU, uiAbsPartIdx, uiDepth);
	}
	else if ((pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxN && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_Nx2N) ||
		(pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxnU && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N))
	{
		m_pcEntropyDecoderIf->parseF2NxnNInterDir(pcCU, uiAbsPartIdx, uiDepth);
	}
}

Void TDecEntropy::decodeFPUTypeIndex2(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{

	m_pcEntropyDecoderIf->parseFNxNInterDir(pcCU, uiAbsPartIdx, uiDepth);

}

Void TDecEntropy::decodeDMHMode(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{

	m_pcEntropyDecoderIf->parseDMHMode(pcCU, uiAbsPartIdx, uiDepth);

}
#endif

Void TDecEntropy::decodePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parsePredMode( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parsePartSize( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodeTransformIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseTransformIdx( pcCU, uiAbsPartIdx, uiDepth );
}

#if ZHANGYI_INTRA
Void TDecEntropy::decodeIntraPuTypeIndex( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) 
{
	PictureType cuPicType = pcCU->getPicture()->getPictureType();
	if ((pcCU->isSkip(uiAbsPartIdx) && (cuPicType == P_PICTURE || cuPicType == F_PICTURE || cuPicType == B_PICTURE)) || !(pcCU->isIntra(uiAbsPartIdx)))
		// cuType != 'P_SKIP' && cuType != 'F_SKIP' && cuType != 'B_SKIP' && IntraCuFlag == 1
	{
		return;
	}
	m_pcEntropyDecoderIf->parseIntraPuTypeIndex(pcCU, uiAbsPartIdx, uiDepth);
}
#endif

Void TDecEntropy::decodePredInfo    ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{

  if( pcCU->isSkip( uiAbsPartIdx ) )
  {
    // clear motion data
#if RPS
    TComMvField cMvFieldZero;
    pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvFieldZero, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, 0, uiDepth);
    pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMvFieldZero, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, 0, uiDepth);
#else
    TComMv cMvZero;

    pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvZero, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, 0, uiDepth);
    pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMvZero, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, 0, uiDepth);
#endif

    // set inter direction
    if ( pcCU->getPicture()->isInterB() )
    {
#if RPS
#if 0//ZHANGYI_INTRA_SDIP
		TComMv cMv0 = pcCU->getMvPredDec(uiAbsPartIdx, REF_PIC_0);
		TComMv cMv1 = pcCU->getMvPredDec(uiAbsPartIdx, REF_PIC_1);
#endif
#if B_RPS_BUG_819
		TComMvField cMvField0;            //暂时B帧的skip模式还没有进来
		TComMvField cMvField1;
		cMvField0.setMvField(TComMv(), 0);
		cMvField1.setMvField(TComMv(), 0);
#else
      TComMvField cMvField0 = pcCU->getMvFieldPred(uiAbsPartIdx, REF_PIC_0, 0);
      TComMvField cMvField1 = pcCU->getMvFieldPred(uiAbsPartIdx, REF_PIC_1, 0);
#endif
      pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvField0, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, 0, uiDepth);
      pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMvField1, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, 0, uiDepth);
#else
#if ZHANGYI_INTRA_SDIP
		TComMv cMv0 = pcCU->getMvPredDec( uiAbsPartIdx, REF_PIC_0 );
		TComMv cMv1 = pcCU->getMvPredDec( uiAbsPartIdx, REF_PIC_1 );
#else
		TComMv cMv0 = pcCU->getMvPred(uiAbsPartIdx, REF_PIC_0);
		TComMv cMv1 = pcCU->getMvPred(uiAbsPartIdx, REF_PIC_1);
#endif
      pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMv0, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, 0, uiDepth);
      pcCU->getCUMvField(REF_PIC_1)->setAllMvField(cMv1, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, 0, uiDepth);
#endif
#if rd_mvd
#if  B_RPS_BUG_820
	  pcCU->setInterDirSubParts(1, uiAbsPartIdx, uiDepth, 0);
#else
	  pcCU->setInterDirSubParts(INTER_BID, uiAbsPartIdx, uiDepth, 0);
#endif
#else
	  pcCU->setInterDirSubParts(3, uiAbsPartIdx, uiDepth, 0);
#endif
    }
    else
    {
#if 0//ZHANGYI_INTRA_SDIP
		TComMv cMv0 = pcCU->getMvPredDec(uiAbsPartIdx, REF_PIC_0);

#endif

#if RPS
#if B_RPS_BUG_819
		TComMvField cMvField0;
		cMvField0.setMvField(TComMv(), 0);
		pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvField0, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, 0, uiDepth);
#else
      TComMvField cMv0 = pcCU->getMvFieldPred(uiAbsPartIdx, REF_PIC_0, 0);
#endif
#else
      TComMv cMv0 = pcCU->getMvPred(uiAbsPartIdx, REF_PIC_0);
#endif

#if RPS

#if	inter_direct_skip_bug2

	  #if  inter_direct_skip_bug3
	  if(pcCU->getPicture()->getPictureType()==F_PICTURE)
		  decodeInterWSM(pcCU, uiAbsPartIdx, uiDepth);
#else
	  decodeInterWSM(pcCU, uiAbsPartIdx, uiDepth);
#endif

#endif
#if F_MHPSKIP_SYC
	  if (pcCU->getPicture()->getPictureType() == F_PICTURE && pcCU->getInterSkipmode(uiAbsPartIdx) == 0)
		  decodeInterMHPSKIP(pcCU, uiAbsPartIdx, uiDepth);
#endif

#if F_MHPSKIP_SYC_DEBUG
	  {
		  //UInt uiPartAddr = pcCU->getZorderIdxInCU();

		  if (pcCU->getInterSkipmode(uiAbsPartIdx) > 3)//MHPSKIP
		  {
			  pcCU->setPSkipMvField(uiAbsPartIdx); // 2NX2N
			  Int refIdx[2];
			  TComMv tempMV[2];
#if F_MHPSKIP_SYC_FIXED_MV
			  switch (pcCU->getInterSkipmode(uiAbsPartIdx) - 3)
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
			  switch (pcCU->getInterSkipmode(uiAbsPartIdx) - 3)
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
				  refIdx[0] = pcCU->getTmpFirstMvPred(FW_P_FST).getRefIdx();
				  tempMV[0] = pcCU->getTmpFirstMvPred(FW_P_FST).getMv();
				  break;
			  default:
				  assert(0);
			  }
#endif


			  if (pcCU->getInterSkipmode(uiAbsPartIdx) - 3 < FW_P_FST)
			  {
				  pcCU->setInterDirSubParts(INTER_FORWARD, uiAbsPartIdx, uiDepth, 0);
				  pcCU->getCUMvField(REF_PIC_0)->setAllMv(tempMV[0], SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);
				  pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(refIdx[0], SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);

				  // m_pcPrediction->motionCompensation(pcCU, &m_acMHPSkipYuvPred[REF_PIC_0], REF_PIC_0, 0);

				  pcCU->setInterDirSubParts(INTER_FORWARD, uiAbsPartIdx, uiDepth, 0);
				  pcCU->getCUMvField(REF_PIC_0)->setAllMv(tempMV[1], SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);
				  pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(refIdx[1], SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);
				  //m_pcPrediction->motionCompensation(pcCU, &m_acMHPSkipYuvPred[REF_PIC_1], REF_PIC_0, 0);

				  //m_pcYuvReco->addAvg(&m_acMHPSkipYuvPred[0], &m_acMHPSkipYuvPred[1], uiPartAddr + pcCU->getZorderIdxInCU(), iRoiWidth, iRoiHeight, uiPartAddr + pcCU->getZorderIdxInCU());
			  }
			  else
			  {
				  pcCU->setInterDirSubParts(INTER_FORWARD, uiAbsPartIdx, uiDepth, 0);
				  pcCU->getCUMvField(REF_PIC_0)->setAllMv(tempMV[0], SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);
				  pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(refIdx[0], SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);

				  //m_pcPrediction->motionCompensation(pcCU, &m_acMHPSkipYuvPred[REF_PIC_0], REF_PIC_0, 0);

				  //m_acMHPSkipYuvPred[REF_PIC_0].copyPartToPartYuv(m_pcYuvReco, uiPartAddr + pcCU->getZorderIdxInCU(), uiPartAddr + pcCU->getZorderIdxInCU(), iRoiWidth, iRoiHeight);
			  }

		  }
#if F_MHPSKIP_SYC_DEBUG_2
		  else
		  {
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
          UInt uiPartAddr = 0;
          //uiBitSize = 
          UInt blockshape_block_x, blockshape_block_y;
          UInt  uiAbsPartIdxR = g_auiZscanToRaster[uiAbsPartIdx];
          blockshape_block_x = pcCU->getWidth(uiAbsPartIdx) >> MIN_BLOCK_SIZE_IN_BIT;
          blockshape_block_y = pcCU->getHeight(uiAbsPartIdx) >> MIN_BLOCK_SIZE_IN_BIT;
          Int iPartIdx = 0;
          Int iRoiWidth, iRoiHeight;
          for (Int i = 0; i < 2; i++)
          {
            for (Int j = 0; j < 2; j++)
            {
              pcCU->getPartIndexAndSize(iPartIdx, uiPartAddr, iRoiWidth, iRoiHeight);
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

                cMvPredL0.setHor(Clip3(-32768, 32767, ((long long int)(curT)* Colmv.getHor() * (MULTI / colT) + HALF_MULTI) >> OFFSET));
                cMvPredL0.setVer(Clip3(-32768, 32767, ((long long int)(curT)* Colmv.getVer() * (MULTI / colT) + HALF_MULTI) >> OFFSET));

                //scalingMV(cMvPredL0, Colmv, curT, colT);
                cMvPredL0.setVer(cMvPredL0.getVer() - delta2);
                cMvFieldPredL0.setMvField(cMvPredL0, 0);
              }
              else
              {
                cMvPredL0.setZero();
                cMvFieldPredL0.setMvField(cMvPredL0, 0);
              }
              pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvFieldPredL0, SIZE_NxN, uiAbsPartIdx + uiPartAddr, 0, uiDepth);
              iPartIdx++;
            }
          }
        }
#if	inter_direct_skip_bug3
			  Int           iNumPredDir = pcCU->getPicture()->isInterB() ? 2 : 1;
#else
			  Int           iNumPredDir = pcCU->getPicture()->isInterP() ? 1 : 2;
#endif

			  for (Int iRefPic = 0; iRefPic < iNumPredDir; iRefPic++)
			  {
				  RefPic eRefPic = (iRefPic ? REF_PIC_1 : REF_PIC_0); // llt
				  Int w = pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic);
				  // pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic)
				  for (Int iRefIdxTemp = 0; iRefIdxTemp < 2; iRefIdxTemp++)
				  {
					  pcCU->setInterDirSubParts(iRefPic + 1, uiAbsPartIdx, uiDepth, 0);
					  Int refIdx = (iRefIdxTemp == 0 ? 0 : pcCU->getInterSkipmode(uiAbsPartIdx));
					  pcCU->getCUMvField(eRefPic)->setAllRefIdx(refIdx, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);
				  }//end of refIdxnum
				  Int refIdx = 0;
				  pcCU->getCUMvField(eRefPic)->setAllRefIdx(refIdx, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);
			  }
		  }
#endif
	}
#endif

#if WRITE_INTERDIR
	  decodeInterDirRD(pcCU, uiAbsPartIdx, uiDepth);
#else
      decodeInterDir(pcCU, uiAbsPartIdx, uiDepth);
#endif
#if DIRECTSKIP_BUG_YQH
	  if (pcCU->isSkip(uiAbsPartIdx)|| pcCU->isDirect(uiAbsPartIdx))
	  {

#if F_MHPSKIP_SYC_DEBUG
		  if (pcCU->getInterSkipmode(uiAbsPartIdx) <= 3)//not MHPSKIP
		  {
#endif

			  Int	iRefIdx = 0;
			  pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(iRefIdx, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);
#if  inter_intra_3

#if F_MHPSKIP_SYC_DEBUG

#if !F_MHPSKIP_SYC_DEBUG_2
        if (pcCU->getInterSkipmode(uiAbsPartIdx) != 0)
        {
          pcCU->getCUMvField(REF_PIC_1)->setAllRefIdx(iRefIdx, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);
        }
#endif		 
#else
			  pcCU->getCUMvField(REF_PIC_1)->setAllRefIdx(iRefIdx, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);
#endif
#endif
#if F_MHPSKIP_SYC_DEBUG
		  }
#endif
	}
	  else
	  {
		  decodeRef(pcCU, uiAbsPartIdx, uiDepth, REF_PIC_0);
	  }
#else
      decodeRef(pcCU, uiAbsPartIdx, uiDepth, REF_PIC_0);
#endif
#else
      pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMv0, pcCU->getPartitionSize(uiAbsPartIdx), uiAbsPartIdx, 0, uiDepth);
#endif
#if rd_mvd
	  pcCU->setInterDirSubParts(INTER_FORWARD, uiAbsPartIdx, uiDepth, 0);
#else
	  pcCU->setInterDirSubParts(1, uiAbsPartIdx, uiDepth, 0);
#endif
    

	}

    return;
  }

  PartSize eMode = pcCU->getPartitionSize( uiAbsPartIdx );

  if( !pcCU->isIntra( uiAbsPartIdx ) )                                 // If it is Intra mode, encode intra prediction mode.
  {
#if	inter_direct_skip_bug2
#if  inter_direct_skip_bug3
	  if (pcCU->getPicture()->getPictureType() == F_PICTURE)
	  decodeInterWSM(pcCU, uiAbsPartIdx, uiDepth);
#else
	  decodeInterWSM(pcCU, uiAbsPartIdx, uiDepth);
#endif
#endif
#if F_MHPSKIP_SYC
	  if (pcCU->getPicture()->getPictureType() == F_PICTURE && pcCU->getInterSkipmode(uiAbsPartIdx) == 0)
		  decodeInterMHPSKIP(pcCU, uiAbsPartIdx, uiDepth);
#endif


#if F_MHPSKIP_SYC_DEBUG
	  {
		  //UInt uiPartAddr = pcCU->getZorderIdxInCU();

		  if (pcCU->getInterSkipmode(uiAbsPartIdx) > 3)//MHPSKIP
		  {
			  pcCU->setPSkipMvField(uiAbsPartIdx); // 2NX2N
			  Int refIdx[2];
			  TComMv tempMV[2];
#if F_MHPSKIP_SYC_FIXED_MV
			  switch (pcCU->getInterSkipmode(uiAbsPartIdx) - 3)
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
			  switch (pcCU->getInterSkipmode(uiAbsPartIdx) - 3)
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
				  refIdx[0] = pcCU->getTmpFirstMvPred(FW_P_FST).getRefIdx();
				  tempMV[0] = pcCU->getTmpFirstMvPred(FW_P_FST).getMv();
				  break;
			  default:
				  assert(0);
			  }
#endif


			  if (pcCU->getInterSkipmode(uiAbsPartIdx) - 3 < FW_P_FST)
			  {
				  pcCU->setInterDirSubParts(INTER_FORWARD, uiAbsPartIdx, uiDepth, 0);
				  pcCU->getCUMvField(REF_PIC_0)->setAllMv(tempMV[0], SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);
				  pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(refIdx[0], SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);

				  // m_pcPrediction->motionCompensation(pcCU, &m_acMHPSkipYuvPred[REF_PIC_0], REF_PIC_0, 0);

				  pcCU->setInterDirSubParts(INTER_FORWARD, uiAbsPartIdx, uiDepth, 0);
				  pcCU->getCUMvField(REF_PIC_0)->setAllMv(tempMV[1], SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);
				  pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(refIdx[1], SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);
				  //m_pcPrediction->motionCompensation(pcCU, &m_acMHPSkipYuvPred[REF_PIC_1], REF_PIC_0, 0);

				  //m_pcYuvReco->addAvg(&m_acMHPSkipYuvPred[0], &m_acMHPSkipYuvPred[1], uiPartAddr + pcCU->getZorderIdxInCU(), iRoiWidth, iRoiHeight, uiPartAddr + pcCU->getZorderIdxInCU());
			  }
			  else
			  {
				  pcCU->setInterDirSubParts(INTER_FORWARD, uiAbsPartIdx, uiDepth, 0);
				  pcCU->getCUMvField(REF_PIC_0)->setAllMv(tempMV[0], SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);
				  pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(refIdx[0], SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);

				  //m_pcPrediction->motionCompensation(pcCU, &m_acMHPSkipYuvPred[REF_PIC_0], REF_PIC_0, 0);

				  //m_acMHPSkipYuvPred[REF_PIC_0].copyPartToPartYuv(m_pcYuvReco, uiPartAddr + pcCU->getZorderIdxInCU(), uiPartAddr + pcCU->getZorderIdxInCU(), iRoiWidth, iRoiHeight);
			  }

		  }
#if F_MHPSKIP_SYC_DEBUG_2
		  else
		  {
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
          UInt uiPartAddr = 0;
          //uiBitSize = 
          UInt blockshape_block_x, blockshape_block_y;
          UInt  uiAbsPartIdxR = g_auiZscanToRaster[uiAbsPartIdx];
          blockshape_block_x = pcCU->getWidth(uiAbsPartIdx) >> MIN_BLOCK_SIZE_IN_BIT;
          blockshape_block_y = pcCU->getHeight(uiAbsPartIdx) >> MIN_BLOCK_SIZE_IN_BIT;
          Int iPartIdx = 0;
          Int iRoiWidth, iRoiHeight;
          for (Int i = 0; i < 2; i++)
          {
            for (Int j = 0; j < 2; j++)
            {
              pcCU->getPartIndexAndSize(iPartIdx, uiPartAddr, iRoiWidth, iRoiHeight);
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

                cMvPredL0.setHor(Clip3(-32768, 32767, ((long long int)(curT)* Colmv.getHor() * (MULTI / colT) + HALF_MULTI) >> OFFSET));
                cMvPredL0.setVer(Clip3(-32768, 32767, ((long long int)(curT)* Colmv.getVer() * (MULTI / colT) + HALF_MULTI) >> OFFSET));

                //scalingMV(cMvPredL0, Colmv, curT, colT);
                cMvPredL0.setVer(cMvPredL0.getVer() - delta2);
                cMvFieldPredL0.setMvField(cMvPredL0, 0);
              }
              else
              {
                cMvPredL0.setZero();
                cMvFieldPredL0.setMvField(cMvPredL0, 0);
              }
              pcCU->getCUMvField(REF_PIC_0)->setAllMvField(cMvFieldPredL0, SIZE_NxN, uiAbsPartIdx + uiPartAddr, 0, uiDepth);
              iPartIdx++;
            }
          }
        }
#if	inter_direct_skip_bug3
			  Int           iNumPredDir = pcCU->getPicture()->isInterB() ? 2 : 1;
#else
			  Int           iNumPredDir = pcCU->getPicture()->isInterP() ? 1 : 2;
#endif
			  for (Int iRefPic = 0; iRefPic < iNumPredDir; iRefPic++)
			  {
				  RefPic eRefPic = (iRefPic ? REF_PIC_1 : REF_PIC_0); // llt
				  Int w = pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic);
				  // pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic)
				  for (Int iRefIdxTemp = 0; iRefIdxTemp < 2; iRefIdxTemp++)
				  {
					  pcCU->setInterDirSubParts(iRefPic + 1, uiAbsPartIdx, uiDepth, 0);
					  Int refIdx = (iRefIdxTemp == 0 ? 0 : pcCU->getInterSkipmode(uiAbsPartIdx));

					  pcCU->getCUMvField(eRefPic)->setAllRefIdx(refIdx, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);
				  }//end of refIdxnum
				  Int refIdx = 0;
				  pcCU->getCUMvField(eRefPic)->setAllRefIdx(refIdx, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);
			  }
		  }
#endif
  }
#endif
#if WRITE_INTERDIR
	  decodeInterDirRD(pcCU, uiAbsPartIdx, uiDepth);
#else
      decodeInterDir( pcCU, uiAbsPartIdx, uiDepth );
#endif
#if RPS
    for (UInt uiRefListIdx = 0; uiRefListIdx < 2; uiRefListIdx++)
    {
      if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(RefPic(uiRefListIdx)) > 0)
      {

#if DIRECTSKIP_BUG_YQH
		  if (pcCU->isSkip(uiAbsPartIdx) || pcCU->isDirect(uiAbsPartIdx))
		  {
#if F_MHPSKIP_SYC_DEBUG
			  if (pcCU->getInterSkipmode(uiAbsPartIdx) <= 3)//not MHPSKIP
			  {
#endif
				  Int	iRefIdx = 0;

#if F_MHPSKIP_SYC_DEBUG
				  if (uiRefListIdx == 0)
				  {
					  pcCU->getCUMvField(RefPic(uiRefListIdx))->setAllRefIdx(iRefIdx, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);
				  }
				  else
				  {
					  if (pcCU->getInterSkipmode(uiAbsPartIdx) != 0)
					  {
						  pcCU->getCUMvField(RefPic(uiRefListIdx))->setAllRefIdx(iRefIdx, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);
					  }		  
				  }		  
#else


#if B_RPS_BUG_820
				  pcCU->getCUMvField(RefPic(uiRefListIdx))->setAllRefIdx(iRefIdx, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);

#else
				  pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(iRefIdx, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth);
#endif
#endif

#if F_MHPSKIP_SYC_DEBUG
			  }
#endif
		  }
		  else
		  {
			  decodeRef(pcCU, uiAbsPartIdx, uiDepth, RefPic(uiRefListIdx));
		  }
#else
		  decodeRef(pcCU, uiAbsPartIdx, uiDepth, RefPic(uiRefListIdx));
#endif
 
#if RD_DIRECT
        if(!pcCU->isDirect(uiAbsPartIdx))
        {
#if F_DHP_SYC
			if (pcCU->getPicture()->isInterF())
				decodeInterDHP(pcCU, uiAbsPartIdx, uiDepth);
#endif
#if DMH
					if (pcCU->getPicture()->isInterF())
					{
						decodeDMHMode(pcCU, uiAbsPartIdx, uiDepth);
					}
#endif
#if rd_mvd
#else
          decodeMvd(pcCU, uiAbsPartIdx, uiDepth, RefPic(uiRefListIdx));
#endif
        }
#else
        decodeMvd(pcCU, uiAbsPartIdx, uiDepth, RefPic(uiRefListIdx));
#endif
       
      }
    }

#if rd_mvd
	if (!pcCU->isDirect(uiAbsPartIdx))
	{
		decodeMvd(pcCU, uiAbsPartIdx, uiDepth);
	}
#endif

#else
    if (pcCU->getPicture()->getRefPic(REF_PIC_0) != NULL) //if ( ref. frame list0 has at least 1 entry )
    {
      decodeMvd(pcCU, uiAbsPartIdx, uiDepth, REF_PIC_0);
    }

    if (pcCU->getPicture()->getRefPic(REF_PIC_1) != NULL) //if ( ref. frame list1 has at least 1 entry )
    {
      decodeMvd(pcCU, uiAbsPartIdx, uiDepth, REF_PIC_1);
    }
#endif
  
  }
  else
  {
#if ZHANGYI_INTRA_SDIP
	  if (eMode == SIZE_NxN || eMode == SIZE_hNx2N || eMode == SIZE_2NxhN)
#else
	  if (eMode == SIZE_NxN)
#endif
    {
      UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( pcCU->getDepth(uiAbsPartIdx) << 1 ) ) >> 2;
      decodeIntraDir( pcCU, uiAbsPartIdx,                  uiDepth+1 );
      decodeIntraDir( pcCU, uiAbsPartIdx + uiPartOffset,   uiDepth+1 );
      decodeIntraDir( pcCU, uiAbsPartIdx + uiPartOffset*2, uiDepth+1 );
      decodeIntraDir( pcCU, uiAbsPartIdx + uiPartOffset*3, uiDepth+1 );
    }
    else
    {
      decodeIntraDir( pcCU, uiAbsPartIdx, uiDepth );
    }
#if ZHANGYI_INTRA
	if ( pcCU->getPicture()->getSPS()->getChromaFormat() != CHROMA_400 )
	{
#if	YQH_SPLIGFLAG_DEC && !niu_LeftPU_revise
		decodeIntraDirCb(pcCU, uiAbsPartIdx, uiDepth+1);
#else
		decodeIntraDirCb(pcCU, uiAbsPartIdx, uiDepth);
#if !ZHANGYI_INTRA_MODIFY
		decodeIntraDirCr(pcCU, uiAbsPartIdx, uiDepth);
#endif
#endif
	}
#endif
  }
}
#if F_DHP_SYC
Void TDecEntropy::decodeInterDHP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	assert(!pcCU->isIntra(uiAbsPartIdx));


	if (pcCU->getPicture()->isInterB())
	{
		return;
	}
	//加上F帧，P帧的限制
	if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(REF_PIC_0) > 1 && pcCU->getPicture()->getSPS()->getDualHypothesisPredictionEnableFlag() == true && (pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2Nx2N && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N))
	{
		m_pcEntropyDecoderIf->parseInterDHP(pcCU, uiAbsPartIdx, uiDepth);
	}
}
#endif
#if	F_MHPSKIP_SYC
Void TDecEntropy::decodeInterMHPSKIP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	assert(!pcCU->isIntra(uiAbsPartIdx));


	if (pcCU->getPicture()->isInterB())
	{
		return;
	}
	//加上F帧，P帧的限制
	if ((pcCU->isSkip(uiAbsPartIdx) || pcCU->isDirect(uiAbsPartIdx)) && pcCU->getPicture()->getSPS()->getMultiHypothesisSkipEnableFlag() == true)
	{
		m_pcEntropyDecoderIf->parseInterMHPSKIP(pcCU, uiAbsPartIdx, uiDepth);
	}
}
#endif
#if	inter_direct_skip_bug2
Void TDecEntropy::decodeInterWSM(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	assert(!pcCU->isIntra(uiAbsPartIdx));


	if (pcCU->getPicture()->isInterB())
	{
		return;
	}
	//加上F帧，P帧的限制

	if ((pcCU->isSkip(uiAbsPartIdx) || pcCU->isDirect(uiAbsPartIdx)) && pcCU->getPicture()->getPicHeader()->getInterSkipmodeNumber() > 1 && pcCU->getPicture()->getSPS()->getWeightedSkipEnableFlag() == true)
	{
		m_pcEntropyDecoderIf->parseInterWSM(pcCU, uiAbsPartIdx, uiDepth);
	}
}
#endif

#if WRITE_INTERDIR
Void TDecEntropy::decodeInterDirRD(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	//test
	if (pcCU->getPicture()->isInterP() || pcCU->getPicture()->isInterF() || (pcCU->isSkip(uiAbsPartIdx)) || (pcCU->isDirect(uiAbsPartIdx)))
	{
		memset(pcCU->getInterDir() + uiAbsPartIdx, 1, sizeof(UChar)*(pcCU->getTotalNumPart() >> (uiDepth << 1)));
		return;
	}



	m_pcEntropyDecoderIf->parseInterDirRD(pcCU, uiAbsPartIdx, uiDepth);
}
#else

Void TDecEntropy::decodeInterDir( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
#if F_RPS
	if (pcCU->getPicture()->isInterP() || pcCU->getPicture()->isInterF())
#else
	if (pcCU->getPicture()->isInterP())
#endif
  {
    memset( pcCU->getInterDir() + uiAbsPartIdx, 1, sizeof(UChar)*( pcCU->getTotalNumPart() >> (uiDepth << 1) ) );
    return;
  }
  
  UInt uiInterDir; 
  UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( uiDepth << 1 ) ) >> 2;
  
  switch ( pcCU->getPartitionSize( uiAbsPartIdx ) )
  {
    case SIZE_2Nx2N:
    {
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, uiDepth, 0 );
      break;
    }
    case SIZE_2NxN:
    {
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, uiDepth, 0 );

      uiAbsPartIdx += uiPartOffset << 1;
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, uiDepth, 1 );
      break;
    }
    case SIZE_Nx2N:
    {
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, uiDepth, 0 );

      uiAbsPartIdx += uiPartOffset;
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, uiDepth, 1 );

      break;
    }
    case SIZE_NxN:
    {
      for ( Int iPartIdx = 0; iPartIdx < 4; iPartIdx++ )
      {
        m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
        pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, uiDepth, iPartIdx );
        uiAbsPartIdx += uiPartOffset;
      }
      break;
    }
  case SIZE_2NxnU:
    {
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, uiDepth, 0 );

      uiAbsPartIdx += (uiPartOffset>>1);

      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, uiDepth, 1 );

      break;
    }
  case SIZE_2NxnD:
    {
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, uiDepth, 0 );

      uiAbsPartIdx += (uiPartOffset<<1) + (uiPartOffset>>1);

      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, uiDepth, 1 );

      break;
    }
  case SIZE_nLx2N:
    {
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, uiDepth, 0 );

      uiAbsPartIdx += (uiPartOffset>>2);

      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, uiDepth, 1 );

      break;
    }
  case SIZE_nRx2N:
    {
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, uiDepth, 0 );

      uiAbsPartIdx += uiPartOffset + (uiPartOffset>>2);

      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, uiDepth, 1 );

      break;
    }
    default:
      break;
  }

  return;
}
#endif
#if ZHANGYI_INTRA
Void TDecEntropy::decodeIntraDirCb( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
	m_pcEntropyDecoderIf->parseIntraDirCb( pcCU, uiAbsPartIdx, uiDepth );
	return;
}
Void TDecEntropy::decodeIntraDirCr( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
	m_pcEntropyDecoderIf->parseIntraDirCr( pcCU, uiAbsPartIdx, uiDepth );
	return;
}
#endif
Void TDecEntropy::decodeIntraDir( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseIntraDir( pcCU, uiAbsPartIdx, uiDepth );
  return;
}
#if RPS
Void TDecEntropy::decodeRef(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, RefPic eRefPic)
{
	Int iRefIdx = 0;
	if (!pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
	{
		iRefIdx = NOT_VALID;
	}
	UInt uiPartOffset = (pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1)) >> 2;
	switch (pcCU->getPartitionSize(uiAbsPartIdx))
	{
	case SIZE_2Nx2N:
	{
		if ((pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic)))
		{
			if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic) > 1)
			{
				m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, eRefPic);
			}
			else
			{
				iRefIdx = 0;
			}
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 0, uiDepth);
		}
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)//因为Dec的代码机制，此段不会被用到
		{
			m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, REF_PIC_0);
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 0, uiDepth);
		}

#endif
		break;
	}
	case SIZE_2NxN:
	{
		if ((pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic)))
		{
			if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic) > 1)
			{
				m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, eRefPic);
			}
			else
			{
				iRefIdx = 0;
			}
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 0, uiDepth);
		}
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, REF_PIC_0);
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 0, uiDepth);
		}
#endif
		uiAbsPartIdx += (uiPartOffset << 1);
		if ((pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic)))
		{
			if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic) > 1)
			{
				m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, eRefPic);
			}
			else
			{
				iRefIdx = 0;
			}
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 1, uiDepth);
		}
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, REF_PIC_0);
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 1, uiDepth);
		}
#endif
		break;
	}
	case SIZE_Nx2N:
	{
		if ((pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic)))
		{
			if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic) > 1)
			{
				m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, eRefPic);
			}
			else
			{
				iRefIdx = 0;
			}
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 0, uiDepth);
		}
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, REF_PIC_0);
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 0, uiDepth);
		}
#endif

		uiAbsPartIdx += uiPartOffset;
		if ((pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic)))
		{
			if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic) > 1)
			{
				m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, eRefPic);
			}
			else
			{
				iRefIdx = 0;
			}
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 1, uiDepth);
		}
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, REF_PIC_0);
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 1, uiDepth);
		}
#endif
		break;
	}
	case SIZE_NxN:
	{
		for (Int iPartIdx = 0; iPartIdx < 4; iPartIdx++)
		{
			if ((pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic)))
			{
				if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic) > 1)
				{
					m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, eRefPic);
				}
				else
				{
					iRefIdx = 0;
				}
				PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
				pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, iPartIdx, uiDepth);
			}
#if F_DHP_SYC_REF_DEBUG
			else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			{
				printf("ERROR：F NxN\n");
				assert(0);
				m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, REF_PIC_0);
				PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
				pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, iPartIdx, uiDepth);
			}
#endif
			uiAbsPartIdx += uiPartOffset;
		}

		break;
	}
	case SIZE_2NxnU:
	{
		if ((pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic)))
		{
			if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic) > 1)
			{
				m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, eRefPic);
			}
			else
			{
				iRefIdx = 0;
			}
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 0, uiDepth);
		}
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, REF_PIC_0);
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 0, uiDepth);
		}
#endif
		uiAbsPartIdx += (uiPartOffset >> 1);
		if ((pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic)))
		{
			if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic) > 1)
			{
				m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, eRefPic);
			}
			else
			{
				iRefIdx = 0;
			}
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 1, uiDepth);
		}
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, REF_PIC_0);
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 1, uiDepth);
		}
#endif
		break;
	}
	case SIZE_2NxnD:
	{
		if ((pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic)))
		{
			if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic) > 1)
			{
				m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, eRefPic);
			}
			else
			{
				iRefIdx = 0;
			}
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 0, uiDepth);
		}
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, REF_PIC_0);
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 0, uiDepth);
		}
#endif
		uiAbsPartIdx += (uiPartOffset << 1) + (uiPartOffset >> 1);
		if ((pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic)))
		{
			if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic) > 1)
			{
				m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, eRefPic);
			}
			else
			{
				iRefIdx = 0;
			}
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 1, uiDepth);
		}
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, REF_PIC_0);
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 1, uiDepth);
		}
#endif
		break;
	}
	case SIZE_nLx2N:
	{
		if ((pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic)))
		{
			if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic) > 1)
			{
				m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, eRefPic);
			}
			else
			{
				iRefIdx = 0;
			}
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 0, uiDepth);
		}
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, REF_PIC_0);
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 0, uiDepth);
		}
#endif
		uiAbsPartIdx += (uiPartOffset >> 2);
		if ((pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic)))
		{
			if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic) > 1)
			{
				m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, eRefPic);
			}
			else
			{
				iRefIdx = 0;
			}
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 1, uiDepth);
		}
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, REF_PIC_0);
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 1, uiDepth);
		}
#endif
		break;
	}
	case SIZE_nRx2N:
	{
		if ((pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic)))
		{
			if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic) > 1)
			{
				m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, eRefPic);
			}
			else
			{
				iRefIdx = 0;
			}
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 0, uiDepth);
		}
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, REF_PIC_0);
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 0, uiDepth);
		}
#endif
		uiAbsPartIdx += uiPartOffset + (uiPartOffset >> 2);
		if ((pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic)))
		{
			if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic) > 1)
			{
				m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, eRefPic);
			}
			else
			{
				iRefIdx = 0;
			}
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 1, uiDepth);
		}
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseRefIdx(pcCU, iRefIdx, REF_PIC_0);
			PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
			pcCU->getCUMvField(eRefPic)->setAllRefIdx(iRefIdx, ePartSize, uiAbsPartIdx, 1, uiDepth);
		}
#endif
		break;
	}
	default:
		break;
	}
	return;
}
#endif


#if rd_mvd
Void TDecEntropy::decodeMvd(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
	UInt uiPartOffset = (pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1)) >> 2;

	switch (pcCU->getPartitionSize(uiAbsPartIdx))
	{
	case SIZE_2Nx2N:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_1);
		}
		break;
	}
	case SIZE_2NxN:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_1);
		}

		uiAbsPartIdx += (uiPartOffset << 1);
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, REF_PIC_1);
		}
		break;
	}
	case SIZE_Nx2N:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_1);
		}

		uiAbsPartIdx += uiPartOffset;
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, REF_PIC_1);
		}
		break;
	}
	case SIZE_NxN:
	{
		for (Int iPartIdx = 0; iPartIdx < 4; iPartIdx++)
		{
			if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
			{
				m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, iPartIdx, uiDepth, REF_PIC_0);
			}
			if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
			{
				m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, iPartIdx, uiDepth, REF_PIC_1);
			}
			uiAbsPartIdx += uiPartOffset;
		}
		break;
	}
	case SIZE_2NxnU:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_1);
		}
		uiAbsPartIdx += (uiPartOffset >> 1);
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, REF_PIC_1);
		}
		break;
	}
	case SIZE_2NxnD:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_1);
		}
		uiAbsPartIdx += (uiPartOffset << 1) + (uiPartOffset >> 1);
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, REF_PIC_1);
		}
		break;
	}
	case SIZE_nLx2N:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_1);
		}
		uiAbsPartIdx += (uiPartOffset >> 2);
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, REF_PIC_1);
		}
		break;
	}
	case SIZE_nRx2N:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_1);
		}
		uiAbsPartIdx += uiPartOffset + (uiPartOffset >> 2);
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, REF_PIC_1);
		}
		break;
	}
	default:
		break;
	}

	return;
}
#else
Void TDecEntropy::decodeMvd(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, RefPic eRefPic)
{
	UInt uiPartOffset = (pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1)) >> 2;

	switch (pcCU->getPartitionSize(uiAbsPartIdx))
	{
	case SIZE_2Nx2N:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, eRefPic);
		}
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_0);
		}
#endif
		break;
	}
	case SIZE_2NxN:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, eRefPic);
		}
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_0);
		}
#endif

		uiAbsPartIdx += (uiPartOffset << 1);
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, eRefPic);
		}
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, REF_PIC_0);
		}
#endif
		break;
	}
	case SIZE_Nx2N:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, eRefPic);
		}
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_0);
		}
#endif

		uiAbsPartIdx += uiPartOffset;
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, eRefPic);
		}
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, REF_PIC_0);
		}
#endif
		break;
	}
	case SIZE_NxN:
	{
		for (Int iPartIdx = 0; iPartIdx < 4; iPartIdx++)
		{
			if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
			{
				m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, iPartIdx, uiDepth, eRefPic);
			}
#if F_DHP_SYC_MVD_DEBUG
			else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			{
				printf("ERROR：F NxN\n");
				assert(0);
				m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, iPartIdx, uiDepth, REF_PIC_0);
			}
#endif
			uiAbsPartIdx += uiPartOffset;
		}
		break;
	}
	case SIZE_2NxnU:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, eRefPic);
		}
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_0);
		}
#endif
		uiAbsPartIdx += (uiPartOffset >> 1);
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, eRefPic);
		}
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, REF_PIC_0);
		}
#endif
		break;
	}
	case SIZE_2NxnD:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, eRefPic);
		}
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_0);
		}
#endif
		uiAbsPartIdx += (uiPartOffset << 1) + (uiPartOffset >> 1);
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, eRefPic);
		}
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, REF_PIC_0);
		}
#endif
		break;
	}
	case SIZE_nLx2N:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, eRefPic);
		}
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_0);
		}
#endif
		uiAbsPartIdx += (uiPartOffset >> 2);
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, eRefPic);
		}
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, REF_PIC_0);
		}
#endif
		break;
	}
	case SIZE_nRx2N:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, eRefPic);
		}
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 0, uiDepth, REF_PIC_0);
		}
#endif
		uiAbsPartIdx += uiPartOffset + (uiPartOffset >> 2);
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, eRefPic);
		}
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyDecoderIf->parseMvd(pcCU, uiAbsPartIdx, 1, uiDepth, REF_PIC_0);
		}
#endif
		break;
	}
	default:
		break;
	}

	return;
}
#endif
Void TDecEntropy::xDecodeCoeff( TComDataCU* pcCU, TCoeff* pcCoeff, UInt uiAbsPartIdx, UInt uiDepth, UInt uiLog2Size, UInt uiTrIdx, UInt uiCurrTrIdx, TextType eType )
{
#if ZHANGYI_INTRA_SDIP
	UInt uiSdipFlag = pcCU->getSDIPFlag(uiAbsPartIdx); //zhangyiCheckSDIP
	UInt uiSdipDir = pcCU->getSDIPDirection(uiAbsPartIdx); //zhangyiCheckSDIP
#endif

  UInt uiWidth  = (1<<uiLog2Size);
  UInt uiHeight = (1<<uiLog2Size);

#if !niu_NSQT_dec
  if (pcCU->getCbf(uiAbsPartIdx, eType, uiTrIdx))
  {
#endif
#if UV_NO_SPLIT_TRANSFROM
	  if ((uiCurrTrIdx == uiTrIdx) || (eType == TEXT_CHROMA_U) || (eType == TEXT_CHROMA_V))
#if niu_NSQT_dec
	  {
		  if (pcCU->getCbf(uiAbsPartIdx, eType, uiTrIdx))
#endif
#else
	  if (uiCurrTrIdx == uiTrIdx)
#endif
		{
		  m_pcEntropyDecoderIf->parseCoeffNxN( pcCU, pcCoeff, uiAbsPartIdx, uiLog2Size, uiDepth, eType );
		}
#if niu_NSQT_dec
	  }
#endif
    else
    {
      if( uiCurrTrIdx <= uiTrIdx )
        assert(0);
      
      UInt uiSize;
#if ZHANGYI_INTRA_SDIP

#if ZHANGYI_INTRA_SDIP_BUG_YQH
	  if (uiSdipFlag && (eType == TEXT_LUMA))
#else
	  if (uiSdipFlag)
#endif
	  {
		  uiWidth = uiSdipDir ? pcCU->getWidth(uiAbsPartIdx) : ((pcCU->getWidth(uiAbsPartIdx)) >> 2);
		  uiHeight = uiSdipDir ? (pcCU->getHeight(uiAbsPartIdx) >> 2) : pcCU->getHeight(uiAbsPartIdx);
	  }
	  else
	  {
		  uiWidth >>= 1;
		  uiHeight >>= 1;
	  }
#else
	  uiWidth >>= 1;
	  uiHeight >>= 1;
#endif
      uiSize = uiWidth*uiHeight;
      uiDepth++;
      uiTrIdx++;
      
      UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1);
      UInt uiIdx      = uiAbsPartIdx;
#if !niu_write_cbf
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiIdx, eType, uiTrIdx, uiDepth );
#endif
      xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size-1, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;
#if !niu_write_cbf
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiIdx, eType, uiTrIdx, uiDepth );
#endif
      xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size-1, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;
#if !niu_write_cbf
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiIdx, eType, uiTrIdx, uiDepth );
#endif
      xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size-1, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;
#if !niu_write_cbf
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiIdx, eType, uiTrIdx, uiDepth );
#endif
      xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size-1, uiTrIdx, uiCurrTrIdx, eType );
    }
#if !niu_NSQT_dec
}
#endif
}

Void TDecEntropy::decodeCoeff( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiLog2Size )
{
  if( pcCU->isSkip(uiAbsPartIdx) ) 
   {
#if DIRECTSKIP_BUG_YQH
	   UInt uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA);
	   pcCU->setCbfSubParts(uiCbf | (0 << 0), TEXT_LUMA, uiAbsPartIdx, uiDepth);

	   uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U);
	   pcCU->setCbfSubParts(uiCbf | (0 << 0), TEXT_CHROMA_U, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
	   uiCbf = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V);
	   pcCU->setCbfSubParts(uiCbf | (0 << 0), TEXT_CHROMA_V, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
#endif


     return;
   }

  UInt uiMinCoeffSize = 1<<(pcCU->getPic()->getLog2MinCUSize()<<1);
  UInt uiLumaOffset   = uiMinCoeffSize*uiAbsPartIdx;
  UInt uiChromaOffset = uiLumaOffset>>2;

#if !niu_cbf_dec
  if( !pcCU->isIntra(uiAbsPartIdx) )
  {
    UInt uiCoeff = 0;
    m_pcEntropyDecoderIf->parseAllCbf(pcCU, uiAbsPartIdx, uiCoeff);
    if( uiCoeff == 0 )
      return;
  }
#endif

#if niu_write_cbf
  m_pcEntropyDecoderIf->parseCbfY(pcCU, uiAbsPartIdx, TEXT_LUMA, 0, uiDepth);
#else
  m_pcEntropyDecoderIf->parseCbf(pcCU, uiAbsPartIdx, TEXT_LUMA, 0, uiDepth);
  m_pcEntropyDecoderIf->parseCbf(pcCU, uiAbsPartIdx, TEXT_CHROMA_U, 0, uiDepth);
  m_pcEntropyDecoderIf->parseCbf(pcCU, uiAbsPartIdx, TEXT_CHROMA_V, 0, uiDepth);
#endif

#if !niu_cbf_dec
  if( !pcCU->isIntra(uiAbsPartIdx) )
  {
    if( pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0) || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0) || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0) )
    {
      decodeTransformIdx( pcCU, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx) );
    }
  }
#endif


  UInt uiLog2SizeC = uiLog2Size - 1;
  UInt uiTrIdx = pcCU->getTransformIdx(uiAbsPartIdx);
  UInt uiLumaTrMode = uiTrIdx;

  UInt uiChromaTrMode;
  if (pcCU->isIntra(uiAbsPartIdx))
  {
	  uiChromaTrMode = 0;
  }
  else
  {
	  (uiLog2SizeC - uiTrIdx >= 2) ? uiChromaTrMode = uiTrIdx : uiChromaTrMode = uiLog2SizeC - 2;
  }

  // UInt uiChromaTrMode = (uiLog2Size>3) ? uiTrIdx : 0;
  xDecodeCoeff( pcCU, pcCU->getCoeffY()  + uiLumaOffset,   uiAbsPartIdx, uiDepth, uiLog2Size,    0, uiLumaTrMode,   TEXT_LUMA     );
  
#if ZHANGYI_INTRA
  xDecodeCoeff( pcCU, pcCU->getCoeffCb() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiLog2Size-1,  0, uiChromaTrMode, TEXT_CHROMA_U );
  xDecodeCoeff( pcCU, pcCU->getCoeffCr() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiLog2Size-1,  0, uiChromaTrMode, TEXT_CHROMA_V );
#else
  xDecodeCoeff( pcCU, pcCU->getCoeffCb() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiLog2Size-1,  0, uiChromaTrMode, TEXT_CHROMA_U );
  xDecodeCoeff( pcCU, pcCU->getCoeffCr() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiLog2Size-1,  0, uiChromaTrMode, TEXT_CHROMA_V );
#endif
}

Void TDecEntropy::decodeDBKIdx( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  if( pcCU->getPicture()->getLoopFilterDisable() )
    return;
  m_pcEntropyDecoderIf->parseDBKIdx(pcCU, uiAbsPartIdx);
}
