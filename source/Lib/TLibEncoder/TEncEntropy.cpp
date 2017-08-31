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

/** \file     TEncEntropy.cpp
    \brief    entropy encoder class
*/

#include "TEncEntropy.h"

#if AVS3_PIC_HEADER_ZL
Void TEncEntropy::setEntropyCoder ( TEncSbac* e, TComPicture* pcPicture, TComPicHeader* pcPicHeader )
#else
Void TEncEntropy::setEntropyCoder ( TEncSbac* e, TComPicture* pcPicture )
#endif
{
  m_pcEntropyCoderIf = e;
  m_pcEntropyCoderIf->setPicture ( pcPicture );
#if AVS3_PIC_HEADER_ZL
  m_pcEntropyCoderIf->setPicHeader ( pcPicHeader );
#endif
}

Void TEncEntropy::encodePictureHeader ( TComPicture* pcPicture )
{
  m_pcEntropyCoderIf->codePictureHeader( pcPicture );
  return;
}
#if wlq_AEC
Void TEncEntropy::encodeEntropyFinish         ()
{
	int i;
	m_pcEntropyCoderIf->getBitstream()->put_one_bit_plus_outstanding((m_pcEntropyCoderIf->getElow() >> (B_BITS_AVS2 - 1)) & 1,m_pcEntropyCoderIf->getEbits_to_follow());        //Ð´ÈëEbits_to_follow+1Î»
	m_pcEntropyCoderIf->getBitstream()->write((m_pcEntropyCoderIf->getElow() >> (B_BITS_AVS2 - 2)) & 1,1);        //1bit
	//put_one_bit(0);   //1 bit         //multiple slice, yuanyuan, XZHENG
	m_pcEntropyCoderIf->getBitstream()->write(1,1);
	for (i = 0; i < 7;
		i++) {   //just encoder method to guarantee the last lps in a slice can be decoded successfully, 2015.07.02
			m_pcEntropyCoderIf->getBitstream()->write(0,1);
	}
}
#endif

#if AVS3_PIC_HEADER_ZL
Void TEncEntropy::encodeIPicHeader( TComPicHeader* pcPicHeader )
{
  m_pcEntropyCoderIf->codeIPicHeader( pcPicHeader );
#if !AVS3_EXTENTION_OFF_SYC
#if AVS3_EXTENSION_LWJ
  m_pcEntropyCoderIf->codeUserData(pcPicHeader->getUserData());
  m_pcEntropyCoderIf->codeReservedExt();
#endif
#if AVS3_EXTENSION_CYH
  m_pcEntropyCoderIf->codeExtCopyright(pcPicHeader->getExtCopyright());
  m_pcEntropyCoderIf->codeExtCameraPra(pcPicHeader->getExtCameraPra());
#endif
#if AVS3_EXTENSION_LYT
  m_pcEntropyCoderIf->codeExtPicDisplay(pcPicHeader);
  m_pcEntropyCoderIf->codeExtRoiPra(pcPicHeader);
#if !AVS3_EXTENSION_DEBUG_SYC
  m_pcEntropyCoderIf->xWriteCodeVlc(0x1B0, 32);				//32,暂时以此作为extension_and_user_data的结束标志
#endif
#endif
#endif
  return;
}
Void TEncEntropy::encodePBPicHeader( TComPicHeader* pcPicHeader )
{
  m_pcEntropyCoderIf->codePBPicHeader( pcPicHeader );
#if !AVS3_EXTENTION_OFF_SYC
#if AVS3_EXTENSION_LWJ
  m_pcEntropyCoderIf->codeUserData(pcPicHeader->getUserData());
  m_pcEntropyCoderIf->codeReservedExt();
#endif
#if AVS3_EXTENSION_CYH
  m_pcEntropyCoderIf->codeExtCopyright(pcPicHeader->getExtCopyright());
  m_pcEntropyCoderIf->codeExtCameraPra(pcPicHeader->getExtCameraPra());
#endif
#if AVS3_EXTENSION_LYT
  m_pcEntropyCoderIf->codeExtPicDisplay(pcPicHeader);
  m_pcEntropyCoderIf->codeExtRoiPra(pcPicHeader);
#if !AVS3_EXTENSION_DEBUG_SYC
  m_pcEntropyCoderIf->xWriteCodeVlc(0x1B0, 32);				//32,暂时以此作为extension_and_user_data的结束标志
#endif
#endif

#endif
  return;
}
#endif
Void TEncEntropy::encodeTerminatingBit      ( UInt uiIsLast )
{
  m_pcEntropyCoderIf->codeTerminatingBit( uiIsLast );
  
  return;
}

Void TEncEntropy::encodePictureFinish()
{
#if wlq_AEC
	encodeEntropyFinish         ();
#endif
  m_pcEntropyCoderIf->codePictureFinish();
}

#if !AVS3_DELETE_PPS_SYC
Void TEncEntropy::encodePPS( TComPPS* pcPPS )
{
  m_pcEntropyCoderIf->codePPS( pcPPS );
  return;

#endif

Void TEncEntropy::encodeSPS( TComSPS* pcSPS )
{
  m_pcEntropyCoderIf->codeSPS( pcSPS );
#if !AVS3_EXTENTION_OFF_SYC
#if AVS3_EXTENSION_LWJ
  //interface
  m_pcEntropyCoderIf->codeUserData		( pcSPS->getUserData() );
  m_pcEntropyCoderIf->codeExtSeqDisplay	( pcSPS );
  m_pcEntropyCoderIf->codeExtScalability( pcSPS );
  m_pcEntropyCoderIf->codeReservedExt	();
#endif
#if AVS3_EXTENSION_CYH
  m_pcEntropyCoderIf->codeExtCopyright(pcSPS->getExtCopyright());
  m_pcEntropyCoderIf->codeExtCameraPra(pcSPS->getExtCameraPra());
#endif
#if AVS3_EXTENSION_LYT	
  m_pcEntropyCoderIf->codeExtMetadata(pcSPS->getExtMetadata());

#if !AVS3_EXTENSION_DEBUG_SYC
  m_pcEntropyCoderIf->xWriteCodeVlc(0x1B0, 32);				//32,暂时以此作为extension_and_user_data的结束标志
#endif
#endif
#endif
  return;
}

#include "../TLibCommon/TypeDef.h"

Void TEncEntropy::encodePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;
  
  if ( pcCU->getPicture()->isIntra() )
  {
    return;
  }

  m_pcEntropyCoderIf->codePredMode( pcCU, uiAbsPartIdx );
}

// Split mode
#if WLQ_CUSplitFlag
Void TEncEntropy::encodeSplitFlag(TComDataCU* pcCU, UInt value, Bool key, UInt value2)
{
	m_pcEntropyCoderIf->codeSplitFlag(pcCU, value, key, value2);
}
#else
Void TEncEntropy::encodeSplitFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;
  
  m_pcEntropyCoderIf->codeSplitFlag( pcCU, uiAbsPartIdx, uiDepth );
}
#endif

#if ZHANGYI_INTRA

Void TEncEntropy::encodeIntraPuTypeIndex (TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD ) 
{
	PictureType cuPicType = pcCU->getPicture()->getPictureType();
	if (bRD)
	{
		uiAbsPartIdx = 0;
	}
	if ((pcCU->isSkip(uiAbsPartIdx) && (cuPicType == P_PICTURE || cuPicType == F_PICTURE || cuPicType == B_PICTURE)) || !(pcCU->isIntra(uiAbsPartIdx)))
		// cuType != 'P_SKIP' && cuType != 'F_SKIP' && cuType != 'B_SKIP' && IntraCuFlag == 1
	{
		return;
	}
	m_pcEntropyCoderIf->codeIntraPuTypeIdx( pcCU, uiAbsPartIdx );
}
#endif

Void TEncEntropy::encodePredInfo( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

#if RPS
  if (pcCU->isSkip(uiAbsPartIdx)) //B帧skip 没写
  {
	  #if	  inter_direct_skip_bug1
	  #if  inter_direct_skip_bug3
	 if( pcCU->getPicture()->isInterF())
	  encodeInterWSM(pcCU, uiAbsPartIdx, bRD);
#else
    encodeInterWSM(pcCU, uiAbsPartIdx, bRD);
#endif
#endif
#if F_MHPSKIP_SYC
	if (pcCU->getPicture()->getPictureType() == F_PICTURE && (pcCU->getInterSkipmode(uiAbsPartIdx) > 3 || pcCU->getInterSkipmode(uiAbsPartIdx) == 0))
		encodeInterMHPSKIP(pcCU, uiAbsPartIdx, bRD);
#endif
#if B_MHBSKIP_SYC
	else if (pcCU->getPicture()->getPictureType() == B_PICTURE )
		encodeInterMHBSKIP(pcCU, uiAbsPartIdx, bRD);
#endif
#if  !B_RPS_BUG_818
    encodeInterDir(pcCU, uiAbsPartIdx, bRD);
#endif
#if F_RPS
	if (pcCU->getPicture()->isInterP() || pcCU->getPicture()->isInterF())
#else
	if (pcCU->getPicture()->isInterP())
#endif
    {
      if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(REF_PIC_0) > 0)
      {
#if	  inter_direct_skip_bug1
#else
        encodeRef(pcCU, uiAbsPartIdx, REF_PIC_0);
#endif
      }
    }
    return;
  }
#else
  if (pcCU->isSkip(uiAbsPartIdx)) return;
#endif





#if inter_direct_skip_bug1
  if (pcCU->isDirect(uiAbsPartIdx)) //B帧skip 没写
  {
#if	  inter_direct_skip_bug1
#if  inter_direct_skip_bug3
	  if (pcCU->getPicture()->isInterF())
		  encodeInterWSM(pcCU, uiAbsPartIdx, bRD);
#else
	  encodeInterWSM(pcCU, uiAbsPartIdx, bRD);
#endif
#endif
#if F_MHPSKIP_SYC
	  if (pcCU->getPicture()->getPictureType() == F_PICTURE && (pcCU->getInterSkipmode(uiAbsPartIdx) > 3 || pcCU->getInterSkipmode(uiAbsPartIdx) == 0))
		  encodeInterMHPSKIP(pcCU, uiAbsPartIdx, bRD);
#endif
#if B_MHBSKIP_SYC
	  else if (pcCU->getPicture()->getPictureType() == B_PICTURE)
		  encodeInterMHBSKIP(pcCU, uiAbsPartIdx, bRD);
#endif
#if WRITE_INTERDIR
	  encodeInterDirRD(pcCU, uiAbsPartIdx, bRD);
#else
	  encodeInterDir(pcCU, uiAbsPartIdx, bRD);
#endif
#if F_RPS
	  if (pcCU->getPicture()->isInterP() || pcCU->getPicture()->isInterF())
#else
	  if (pcCU->getPicture()->isInterP())
#endif
	  {
		  if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(REF_PIC_0) > 0)
		  {
#if	  inter_direct_skip_bug1
#else
			  encodeRef(pcCU, uiAbsPartIdx, REF_PIC_0);
#endif
		  }
	  }
	  return;
  }

#endif




  PartSize eSize = pcCU->getPartitionSize( uiAbsPartIdx );

  if( !pcCU->isIntra( uiAbsPartIdx ) )                                 // If it is Intra mode, encode intra prediction mode.
  {
#if WRITE_INTERDIR
	  encodeInterDirRD(pcCU, uiAbsPartIdx, bRD);
#else
    encodeInterDir( pcCU, uiAbsPartIdx, bRD );
#endif
#if RPS
    for (UInt uiRefListIdx = 0; uiRefListIdx < 2; uiRefListIdx++)
    {
      if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(RefPic(uiRefListIdx)) > 0)
      {
        encodeRef(pcCU, uiAbsPartIdx, RefPic(uiRefListIdx));
#if RD_DIRECT
        if(!pcCU->isDirect(uiAbsPartIdx))
        {
#if F_DHP_SYC
			if (pcCU->getPicture()->isInterF())
				encodeInterDHP(pcCU, uiAbsPartIdx, bRD);
#endif

#if DMH
					if (pcCU->getPicture()->isInterF())
					{
						encodeDMHMode(pcCU, uiAbsPartIdx);
					}				
#endif
#if rd_mvd
#else
          encodeMvd(pcCU, uiAbsPartIdx, RefPic(uiRefListIdx), bRD);
#endif
        }
#else
        encodeMvd(pcCU, uiAbsPartIdx, RefPic(uiRefListIdx), bRD);
#endif
       
      }
     }
#if rd_mvd
	if (!pcCU->isDirect(uiAbsPartIdx))
	{
		encodeMvd(pcCU, uiAbsPartIdx, bRD);
	}
#endif
#else
    if( pcCU->getPicture()->getRefPic(REF_PIC_0) != NULL )
    {
      encodeMvd(pcCU, uiAbsPartIdx, REF_PIC_0, bRD);
    }

    if (pcCU->getPicture()->getRefPic(REF_PIC_1) != NULL)
    {
      encodeMvd(pcCU, uiAbsPartIdx, REF_PIC_1, bRD);
    }
#endif
   
  }
  else
  {
#if ZHANGYI_INTRA
	  PictureType cuPicType = pcCU->getPicture()->getPictureType();
	  if ( pcCU->isSkip( uiAbsPartIdx ) && ( cuPicType == P_PICTURE || cuPicType == B_PICTURE || cuPicType == F_PICTURE ))
	  {
		  return;
	  }
#endif
#if ZHANGYI_INTRA_SDIP
	  if (eSize == SIZE_2NxhN || eSize == SIZE_hNx2N || eSize == SIZE_NxN)
#else
	  if (eSize == SIZE_NxN)
#endif
    {
      UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( pcCU->getDepth(uiAbsPartIdx) << 1 ) ) >> 2;
      encodeIntraDir( pcCU, uiAbsPartIdx                  );
      encodeIntraDir( pcCU, uiAbsPartIdx + uiPartOffset   );
      encodeIntraDir( pcCU, uiAbsPartIdx + uiPartOffset*2 );
      encodeIntraDir( pcCU, uiAbsPartIdx + uiPartOffset*3 );
    }
    else
    {
      encodeIntraDir( pcCU, uiAbsPartIdx, bRD );
    }
#if ZHANGYI_INTRA
	if (pcCU->getPicture()->getSPS()->getChromaFormat() != CHROMA_400 )
	{
		encodeIntraDirCb( pcCU, uiAbsPartIdx, bRD);
#if !ZHANGYI_INTRA_MODIFY
		encodeIntraDirCr( pcCU, uiAbsPartIdx, bRD);
#endif
	}
#endif
    return;
  }
}

#if INTER_GROUP
Void TEncEntropy::encodeInterCUTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD)
{
	if (bRD)
	{
		uiAbsPartIdx = 0;
	}
#if !WLQ_noCUtype
	m_pcEntropyCoderIf->codeInterCUTypeIndex(pcCU, uiAbsPartIdx);
#endif
}

Void TEncEntropy::encodeShapeOfPartitionIndex(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
	m_pcEntropyCoderIf->codeShapeOfPartitionIndex(pcCU, uiAbsPartIdx);
}

Void TEncEntropy::encodeBPUTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD)
{
	assert(!pcCU->isIntra(uiAbsPartIdx));
	if (bRD)
		uiAbsPartIdx = 0;

	if (pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N)
	{
		m_pcEntropyCoderIf->codeB2Nx2NInterDir(pcCU, uiAbsPartIdx);
	}
	else if ((pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxN && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_Nx2N) ||
		(pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxnU && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N))
	{
		m_pcEntropyCoderIf->codeB2NxnNInterDir(pcCU, uiAbsPartIdx);
	}
}

Void TEncEntropy::encodeWeightedSkipMode(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
	return;
}

Void TEncEntropy::encodeCUSubTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
	return;
}

Void TEncEntropy::encodeBPUTypeIndex2(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
	m_pcEntropyCoderIf->codeBNxNInterDir(pcCU, uiAbsPartIdx);
}

Void TEncEntropy::encodeFPUTypeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD)
{
	assert(!pcCU->isIntra(uiAbsPartIdx));
	if (bRD)
		uiAbsPartIdx = 0;

	//UInt uiPartOffset = (pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1)) >> 2;

	if (pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N)
	{
		m_pcEntropyCoderIf->codeF2Nx2NInterDir(pcCU, uiAbsPartIdx);
	}
	else if ((pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxN && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_Nx2N) ||
		(pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxnU && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N))
	{
		m_pcEntropyCoderIf->codeF2NxnNInterDir(pcCU, uiAbsPartIdx);
	}
}

Void TEncEntropy::encodeFPUTypeIndex2(TComDataCU* pcCU, UInt uiAbsPartIdx)
{

	m_pcEntropyCoderIf->codeFNxNInterDir(pcCU, uiAbsPartIdx);

}

Void TEncEntropy::encodeDMHMode(TComDataCU* pcCU, UInt uiAbsPartIdx)
{

	m_pcEntropyCoderIf->codeDMHMode(pcCU, uiAbsPartIdx);

}
///AVS3
#endif

Void TEncEntropy::encodePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if ( bRD )
  {
    uiAbsPartIdx = 0;
  }
  
  if ( pcCU->isSkip( uiAbsPartIdx ) )
  {
    return;
  }

  m_pcEntropyCoderIf->codePartSize( pcCU, uiAbsPartIdx );
}

#if F_DHP_SYC
Void TEncEntropy::encodeInterDHP(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD)
{
	assert(!pcCU->isIntra(uiAbsPartIdx));

	if (bRD)
		uiAbsPartIdx = 0;

	if (pcCU->getPicture()->isInterB())
	{
		return;
	}
	//加上F帧，P帧的限制

	if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(REF_PIC_0) > 1 && pcCU->getPicture()->getSPS()->getDualHypothesisPredictionEnableFlag() == true && (pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2Nx2N && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N))
	{
		m_pcEntropyCoderIf->codeInterDHP(pcCU, uiAbsPartIdx);
	}
}
 Void TEncEntropy::encodeInterDHP_FNXN(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD)//关闭了NxN 没有用到该函数
 {
 	assert(!pcCU->isIntra(uiAbsPartIdx));
 
 	if (bRD)
 		uiAbsPartIdx = 0;
 
 	if (pcCU->getPicture()->isInterB())
 	{
 		return;
 	}
 	//加上F帧，P帧的限制
 	if (pcCU->getPicture()->getPicHeader()->getNumRefIdx(REF_PIC_0) > 1 && pcCU->getPicture()->getSPS()->getDualHypothesisPredictionEnableFlag() == true && (pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2Nx2N && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N))
 	{
 		m_pcEntropyCoderIf->codeInterDHP(pcCU, uiAbsPartIdx);
 	}
 }
#endif


#if inter_direct_skip_bug1
Void TEncEntropy::encodeInterWSM(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD)
{
	assert(!pcCU->isIntra(uiAbsPartIdx));

	if (bRD)
		uiAbsPartIdx = 0;

	if (pcCU->getPicture()->isInterB())
	{
		return;
	}

	//pcPicHeader->getPictureType() == P_PICTURE&&pcPicHeader->getSeqHeader()->getWeightedSkipEnableFlag() == true

	//pcCU->getPicture()->getPicHeader()->getInterSkipmodeNumber()
	//加上F帧，P帧的限制
	if ((pcCU->isSkip(uiAbsPartIdx) || pcCU->isDirect(uiAbsPartIdx)) && pcCU->getPicture()->getPicHeader()->getInterSkipmodeNumber() > 1 && pcCU->getPicture()->getSPS()->getWeightedSkipEnableFlag() == true)
	{
		m_pcEntropyCoderIf->codeInterWSM(pcCU, uiAbsPartIdx);
	}

}
#endif
#if F_MHPSKIP_SYC
Void TEncEntropy::encodeInterMHPSKIP(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD)
{
	assert(!pcCU->isIntra(uiAbsPartIdx));

	if (bRD)
		uiAbsPartIdx = 0;

	if (pcCU->getPicture()->isInterB())
	{
		return;
	}

	//pcPicHeader->getPictureType() == P_PICTURE&&pcPicHeader->getSeqHeader()->getWeightedSkipEnableFlag() == true

	//pcCU->getPicture()->getPicHeader()->getInterSkipmodeNumber()
	//加上F帧，P帧的限制
	if ((pcCU->isSkip(uiAbsPartIdx) || pcCU->isDirect(uiAbsPartIdx)) && pcCU->getPicture()->getSPS()->getMultiHypothesisSkipEnableFlag() == true)
	{
		m_pcEntropyCoderIf->codeInterMHPSKIP(pcCU, uiAbsPartIdx);
	}

}
#endif
#if B_MHBSKIP_SYC
Void TEncEntropy::encodeInterMHBSKIP(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD)
{
	assert(!pcCU->isIntra(uiAbsPartIdx));

	if (bRD)
		uiAbsPartIdx = 0;

	//pcPicHeader->getPictureType() == P_PICTURE&&pcPicHeader->getSeqHeader()->getWeightedSkipEnableFlag() == true

	//pcCU->getPicture()->getPicHeader()->getInterSkipmodeNumber()
	//加上F帧，P帧的限制
	if ((pcCU->isSkip(uiAbsPartIdx) || pcCU->isDirect(uiAbsPartIdx)) )
	{
		m_pcEntropyCoderIf->codeInterMHBSKIP(pcCU, uiAbsPartIdx);
	}

}
#endif

#if WRITE_INTERDIR
Void TEncEntropy::encodeInterDirRD(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD)
{
	assert(!pcCU->isIntra(uiAbsPartIdx));

	if (bRD)
		uiAbsPartIdx = 0;
	//test
	if (!pcCU->getPicture()->isInterB() || (pcCU->isSkip(uiAbsPartIdx)) || (pcCU->isDirect(uiAbsPartIdx)))
	{
		return;
	}

	m_pcEntropyCoderIf->codeInterDirRD(pcCU, uiAbsPartIdx);
}
#else
Void TEncEntropy::encodeInterDir( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  assert( !pcCU->isIntra( uiAbsPartIdx ) );
  
  if( bRD )
    uiAbsPartIdx = 0;
  
  if ( !pcCU->getPicture()->isInterB() )
  {
    return;
  }
  
  UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( pcCU->getDepth(uiAbsPartIdx) << 1 ) ) >> 2;
  
  switch ( pcCU->getPartitionSize( uiAbsPartIdx ) )
  {
    case SIZE_2Nx2N:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      break;
    }
    case SIZE_2NxN:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      uiAbsPartIdx += uiPartOffset << 1;
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      break;
    }
    case SIZE_Nx2N:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      uiAbsPartIdx += uiPartOffset;
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      break;
    }
    case SIZE_NxN:
    {
      for ( Int iPartIdx = 0; iPartIdx < 4; iPartIdx++ )
      {
        m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
        uiAbsPartIdx += uiPartOffset;
      }
      break;
    }
  case SIZE_2NxnU:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx + (uiPartOffset>>1) );
      break;
    }
  case SIZE_2NxnD:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx + (uiPartOffset<<1) + (uiPartOffset>>1) );
      break;
    }
  case SIZE_nLx2N:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx + (uiPartOffset>>2) );
      break;
    }
  case SIZE_nRx2N:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx + uiPartOffset + (uiPartOffset>>2) );
      break;
    }
    default:
      break;
  }
  
  return;
}
#endif
#if ZHANGYI_INTRA
Void TEncEntropy::encodeIntraDirCb( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
	assert( pcCU->isIntra( uiAbsPartIdx ) );

	if( bRD )
		uiAbsPartIdx = 0;

	m_pcEntropyCoderIf->codeIntraDirCb( pcCU, uiAbsPartIdx );

	return;
}

Void TEncEntropy::encodeIntraDirCbForRDO( TComDataCU* pcCU, UInt uiAbsPartIdx,Int iIntraDirCbValue )
{
	m_pcEntropyCoderIf->codeIntraDirCbForRDO(pcCU, uiAbsPartIdx, iIntraDirCbValue);
}
Void TEncEntropy::encodeIntraDirCrForRDO( TComDataCU* pcCU, UInt uiAbsPartIdx,Int iIntraDirCbValue )
{
	m_pcEntropyCoderIf->codeIntraDirCrForRDO(pcCU, uiAbsPartIdx, iIntraDirCbValue);
}

Void TEncEntropy::encodeIntraDirCr( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
	assert( pcCU->isIntra( uiAbsPartIdx ) );

	if( bRD )
		uiAbsPartIdx = 0;

	m_pcEntropyCoderIf->codeIntraDirCr( pcCU, uiAbsPartIdx );

	return;
}
#endif
Void TEncEntropy::encodeIntraDir( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  assert( pcCU->isIntra( uiAbsPartIdx ) );

  if( bRD )
    uiAbsPartIdx = 0;

  m_pcEntropyCoderIf->codeIntraDir( pcCU, uiAbsPartIdx );

  return;
}
#if ZHANGYI_INTRA
Void TEncEntropy::encodeIntraDirForRDO( Int iIntraDirValue )
{
	m_pcEntropyCoderIf->codeIntraDirForRDO(  iIntraDirValue );
	return;
}
#endif
#if RPS
Void TEncEntropy::encodeRef(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPic eRefPic)
{
	assert(!pcCU->isIntra(uiAbsPartIdx));
	if ((pcCU->getPicture()->getPicHeader()->getNumRefIdx(eRefPic) == 1))//单参考不写ref码流
	{
		return;
	}
	UInt uiPartOffset = (pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1)) >> 2;
	switch (pcCU->getPartitionSize(uiAbsPartIdx))
	{
	case SIZE_2Nx2N:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, eRefPic);
		}
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
#endif
		break;
	}
	case SIZE_2NxN:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, eRefPic);
		}
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
#endif

		uiAbsPartIdx += uiPartOffset << 1;
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, eRefPic);
		}
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
#endif
		break;
	}

	case SIZE_Nx2N:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, eRefPic);
		}
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
#endif

		uiAbsPartIdx += uiPartOffset;
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, eRefPic);
		}
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, REF_PIC_0);
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
				m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, eRefPic);
			}
#if F_DHP_SYC_REF_DEBUG
			else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			{
				printf("ERROR：F NxN\n");
				m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, REF_PIC_0);
			}
#endif
			uiAbsPartIdx += uiPartOffset;
		}
		break;
	}
	case SIZE_2NxnU:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, eRefPic);
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, REF_PIC_0);
#endif

		uiAbsPartIdx += (uiPartOffset >> 1);

		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, eRefPic);
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, REF_PIC_0);
#endif

		break;
	}
	case SIZE_2NxnD:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, eRefPic);
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, REF_PIC_0);
#endif

		uiAbsPartIdx += (uiPartOffset << 1) + (uiPartOffset >> 1);

		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, eRefPic);
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, REF_PIC_0);
#endif

		break;
	}
	case SIZE_nLx2N:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, eRefPic);
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, REF_PIC_0);
#endif

		uiAbsPartIdx += (uiPartOffset >> 2);

		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, eRefPic);
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, REF_PIC_0);
#endif

		break;
	}
	case SIZE_nRx2N:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, eRefPic);
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, REF_PIC_0);
#endif

		uiAbsPartIdx += uiPartOffset + (uiPartOffset >> 2);

		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, eRefPic);
#if F_DHP_SYC_REF_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			m_pcEntropyCoderIf->codeRefIdx(pcCU, uiAbsPartIdx, REF_PIC_0);
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
Void TEncEntropy::encodeMvd(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD)
{
	assert(!pcCU->isIntra(uiAbsPartIdx));

	if (bRD)
		uiAbsPartIdx = 0;

	UInt uiPartOffset = (pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1)) >> 2;

	switch (pcCU->getPartitionSize(uiAbsPartIdx))
	{
	case SIZE_2Nx2N:
	{
    if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID || pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_1);
		}
		break;
	}

	case SIZE_2NxN:
	{
    if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID || pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_1);
		}

		uiAbsPartIdx += uiPartOffset << 1;
    if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID || pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_1);
		}
		break;
	}

	case SIZE_Nx2N:
	{
    if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID || pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_1);
		}

		uiAbsPartIdx += uiPartOffset;
    if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID || pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_1);
		}
		break;
	}

	case SIZE_NxN:
	{
		for (Int iPartIdx = 0; iPartIdx < 4; iPartIdx++)
		{
      if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID || pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			{
				m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
			}
			if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
			{
				m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_1);
			}
			uiAbsPartIdx += uiPartOffset;
		}
		break;
	}
	case SIZE_2NxnU:
	{
    if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID || pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_1);
		}

		uiAbsPartIdx += (uiPartOffset >> 1);

    if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID || pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_1);
		}

		break;
	}
	case SIZE_2NxnD:
	{
    if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID || pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_1);
		}

		uiAbsPartIdx += (uiPartOffset << 1) + (uiPartOffset >> 1);

    if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID || pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_1);
		}

		break;
	}
	case SIZE_nLx2N:
	{
    if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID || pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_1);
		}
		uiAbsPartIdx += (uiPartOffset >> 2);
    if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID || pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_1);
		}
		break;
	}
	case SIZE_nRx2N:
	{
    if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID || pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_1);
		}

		uiAbsPartIdx += uiPartOffset + (uiPartOffset >> 2);

    if (pcCU->getInterDir(uiAbsPartIdx) == INTER_FORWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_SYM || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID || pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
		if (pcCU->getInterDir(uiAbsPartIdx) == INTER_BACKWARD || pcCU->getInterDir(uiAbsPartIdx) == INTER_BID)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_1);
		}

		break;
	}
	default:
		break;
	}

	return;
}
#else
Void TEncEntropy::encodeMvd(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPic eRefPic, Bool bRD)
{
	assert(!pcCU->isIntra(uiAbsPartIdx));

	if (bRD)
		uiAbsPartIdx = 0;

	UInt uiPartOffset = (pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(uiAbsPartIdx) << 1)) >> 2;

	switch (pcCU->getPartitionSize(uiAbsPartIdx))
	{
	case SIZE_2Nx2N:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, eRefPic);
		}
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
#endif
		break;
	}

	case SIZE_2NxN:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, eRefPic);
		}
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
#endif

		uiAbsPartIdx += uiPartOffset << 1;
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, eRefPic);
		}
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
#endif
		break;
	}

	case SIZE_Nx2N:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, eRefPic);
		}
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
		}
#endif

		uiAbsPartIdx += uiPartOffset;
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, eRefPic);
		}
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
		{
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
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
				m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, eRefPic);
			}
#if F_DHP_SYC_MVD_DEBUG
			else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			{
				printf("ERROR：F NxN\n");
				assert(0);
				m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
			}
#endif
			uiAbsPartIdx += uiPartOffset;
		}
		break;
	}
	case SIZE_2NxnU:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, eRefPic);
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
#endif

		uiAbsPartIdx += (uiPartOffset >> 1);

		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, eRefPic);
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
#endif

		break;
	}
	case SIZE_2NxnD:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, eRefPic);
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
#endif

		uiAbsPartIdx += (uiPartOffset << 1) + (uiPartOffset >> 1);

		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, eRefPic);
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
#endif

		break;
	}
	case SIZE_nLx2N:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, eRefPic);
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
#endif

		uiAbsPartIdx += (uiPartOffset >> 2);

		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, eRefPic);
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
#endif

		break;
	}
	case SIZE_nRx2N:
	{
		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, eRefPic);
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
#endif

		uiAbsPartIdx += uiPartOffset + (uiPartOffset >> 2);

		if (pcCU->getInterDir(uiAbsPartIdx) & (1 << eRefPic))
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, eRefPic);
#if F_DHP_SYC_MVD_DEBUG
		else if (pcCU->getInterDir(uiAbsPartIdx) == INTER_DUAL)
			m_pcEntropyCoderIf->codeMvd(pcCU, uiAbsPartIdx, REF_PIC_0);
#endif

		break;
	}
	default:
		break;
	}

	return;
}
#endif

Void TEncEntropy::encodeTransformIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  m_pcEntropyCoderIf->codeTransformIdx( pcCU, uiAbsPartIdx );
}

Void TEncEntropy::xEncodeCoeff( TComDataCU* pcCU, TCoeff* pcCoeff, UInt uiAbsPartIdx, UInt uiDepth, UInt uiLog2Size, UInt uiTrIdx, UInt uiCurrTrIdx, TextType eType, Bool bRD )
{
#if ZHANGYI_INTRA_SDIP
	UInt uiSdipFlag = pcCU->getSDIPFlag(uiAbsPartIdx); //zhangyiCheckSDIP
	UInt uiSdipDir = pcCU->getSDIPDirection(uiAbsPartIdx); //zhangyiCheckSDIP
#endif
  UInt uiWidth  = (1<<uiLog2Size);
  UInt uiHeight = (1<<uiLog2Size);
#if RD_NSQT_BUG
#if UV_NO_SPLIT_TRANSFROM
  if ((uiCurrTrIdx == uiTrIdx) || (eType == TEXT_CHROMA_U) || (eType == TEXT_CHROMA_V))
#else
  if( uiCurrTrIdx == uiTrIdx )
#endif
  {
	  if (pcCU->getCbf(uiAbsPartIdx, eType, uiTrIdx))
	  {
#else
  if ( pcCU->getCbf( uiAbsPartIdx, eType, uiTrIdx ) )
  {
#if UV_NO_SPLIT_TRANSFROM
	  if ((uiCurrTrIdx == uiTrIdx) || (eType == TEXT_CHROMA_U) || (eType == TEXT_CHROMA_V))
#else
	  if( uiCurrTrIdx == uiTrIdx )
#endif
	  {
#endif
		  m_pcEntropyCoderIf->codeCoeffNxN(pcCU, pcCoeff, uiAbsPartIdx, uiLog2Size, uiDepth, eType, bRD);
	  }
#if RD_NSQT_BUG
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
#if RD_NSQT
#if RD_NSQT_BUG_YQH
	  if (uiLog2Size<=3)
	  {
		  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD); pcCoeff += uiSize; uiIdx += uiQPartNum;
		  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD); pcCoeff += uiSize; uiIdx += uiQPartNum;
		  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD); pcCoeff += uiSize; uiIdx += uiQPartNum;
		  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD);
		  return;
	  }
#endif
	  if (pcCU->getPicture()->getSPS()->getNonsquareQuadtreeTansformEnableFlag())
	  {
		  PartSize ePartSize = pcCU->getPartitionSize(uiAbsPartIdx);
		  switch (ePartSize)
		  {
		  case SIZE_2Nx2N:
		  case SIZE_NxN:
			 // m_pcEntropyCoderIf->codeCbf(pcCU, uiIdx, eType, uiTrIdx);

			  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD); pcCoeff += uiSize; uiIdx += uiQPartNum;

			 // m_pcEntropyCoderIf->codeCbf(pcCU, uiIdx, eType, uiTrIdx);

			  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD); pcCoeff += uiSize; uiIdx += uiQPartNum;

			 // m_pcEntropyCoderIf->codeCbf(pcCU, uiIdx, eType, uiTrIdx);

			  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD); pcCoeff += uiSize; uiIdx += uiQPartNum;

			 // m_pcEntropyCoderIf->codeCbf(pcCU, uiIdx, eType, uiTrIdx);

			  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD);
			  break;
		  case SIZE_2NxN:
		  case SIZE_2NxnU:
		  case SIZE_2NxnD:
		  case SIZE_2NxhN:
			 // m_pcEntropyCoderIf->codeCbf(pcCU, uiIdx, eType, uiTrIdx);

			  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD); pcCoeff += uiSize; uiIdx += uiQPartNum;/* uiIdx += uiQPartNum >> 1;*/

			 // m_pcEntropyCoderIf->codeCbf(pcCU, uiIdx, eType, uiTrIdx);

			  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD); pcCoeff += uiSize;  uiIdx += uiQPartNum;/*uiIdx += uiQPartNum + (uiQPartNum >> 1);*/

			//  m_pcEntropyCoderIf->codeCbf(pcCU, uiIdx, eType, uiTrIdx);

			  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD); pcCoeff += uiSize;   uiIdx += uiQPartNum;/*uiIdx += uiQPartNum >> 1;*/

			 // m_pcEntropyCoderIf->codeCbf(pcCU, uiIdx, eType, uiTrIdx);

			  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD);
			  break;
		  case SIZE_Nx2N:
		  case SIZE_nLx2N:
		  case SIZE_nRx2N:
		  case SIZE_hNx2N:
			//  m_pcEntropyCoderIf->codeCbf(pcCU, uiIdx, eType, uiTrIdx);

			  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD); pcCoeff += uiSize; uiIdx += uiQPartNum;/* uiIdx += uiQPartNum >> 2;*/

			//  m_pcEntropyCoderIf->codeCbf(pcCU, uiIdx, eType, uiTrIdx);

			  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD); pcCoeff += uiSize; uiIdx += uiQPartNum; /*uiIdx += (uiQPartNum >> 2) + (uiQPartNum >> 1);*/

			 // m_pcEntropyCoderIf->codeCbf(pcCU, uiIdx, eType, uiTrIdx);

			  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD); pcCoeff += uiSize; uiIdx += uiQPartNum; /*uiIdx += uiQPartNum >> 2;*/

			  //m_pcEntropyCoderIf->codeCbf(pcCU, uiIdx, eType, uiTrIdx);

			  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD);
			  break;

		  default:
			  break;
		  }
	  }
	  else
	  {
		 // m_pcEntropyCoderIf->codeCbf(pcCU, uiIdx, eType, uiTrIdx);

		  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD); pcCoeff += uiSize; uiIdx += uiQPartNum;

		 // m_pcEntropyCoderIf->codeCbf(pcCU, uiIdx, eType, uiTrIdx);

		  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD); pcCoeff += uiSize; uiIdx += uiQPartNum;

		 // m_pcEntropyCoderIf->codeCbf(pcCU, uiIdx, eType, uiTrIdx);

		  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD); pcCoeff += uiSize; uiIdx += uiQPartNum;

		  //m_pcEntropyCoderIf->codeCbf(pcCU, uiIdx, eType, uiTrIdx);

		  xEncodeCoeff(pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size - 1, uiTrIdx, uiCurrTrIdx, eType, bRD);
	  }
#else
#if !niu_write_cbf
      m_pcEntropyCoderIf->codeCbf( pcCU, uiIdx, eType, uiTrIdx );
#endif
      xEncodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size-1, uiTrIdx, uiCurrTrIdx, eType, bRD ); pcCoeff += uiSize; uiIdx += uiQPartNum;
#if !niu_write_cbf
      m_pcEntropyCoderIf->codeCbf( pcCU, uiIdx, eType, uiTrIdx );
#endif
      xEncodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size-1, uiTrIdx, uiCurrTrIdx, eType, bRD ); pcCoeff += uiSize; uiIdx += uiQPartNum;
#if !niu_write_cbf
      m_pcEntropyCoderIf->codeCbf( pcCU, uiIdx, eType, uiTrIdx );
#endif
      xEncodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size-1, uiTrIdx, uiCurrTrIdx, eType, bRD ); pcCoeff += uiSize; uiIdx += uiQPartNum;
#if wlq_avs2x_debug
	  if (g_sign && eType == TEXT_LUMA)
	  {
		  printf("\n");
		  for (int iii = 0; iii < 4; iii++)
		  {
			  for (int jjj = 0; jjj < 4; jjj++)
			  {
				  printf("%d\t", pcCoeff[iii*4 + jjj]);
			  }
			  printf("\n");
		  }
		  printf("\n");
	  }
#endif
#if !niu_write_cbf
      m_pcEntropyCoderIf->codeCbf( pcCU, uiIdx, eType, uiTrIdx );
#endif
      xEncodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiLog2Size-1, uiTrIdx, uiCurrTrIdx, eType, bRD );
#endif 
}
#if RD_NSQT_BUG
#else
  }
#endif
}

Void TEncEntropy::encodeCoeff( TComDataCU* pcCU, TCoeff* pCoeff, UInt uiAbsPartIdx, UInt uiDepth, UInt uiLog2Size, UInt uiMaxTrMode, UInt uiTrMode, TextType eType, Bool bRD )
{
  xEncodeCoeff( pcCU, pCoeff, uiAbsPartIdx, uiDepth, uiLog2Size, uiTrMode, uiMaxTrMode, eType, bRD );
}

Void TEncEntropy::encodeCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;
  
  m_pcEntropyCoderIf->codeCbf( pcCU, uiAbsPartIdx, eType, uiTrDepth );
}
#if niu_write_cbf
Void TEncEntropy::encodeCbfY(TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, Bool bRD)
{
	if (bRD)
		uiAbsPartIdx = 0;
#if WLQ_set_CBP 
	if (pcCU->isIntra(uiAbsPartIdx))
#endif
	m_pcEntropyCoderIf->codeCbfY(pcCU, uiAbsPartIdx, eType, uiTrDepth);
}

Void TEncEntropy::encodeCbfUV(TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, Bool bRD)
{
	if (bRD)
		uiAbsPartIdx = 0;

	m_pcEntropyCoderIf->codeCbfUV(pcCU, uiAbsPartIdx, eType, uiTrDepth);
}
#endif
Void TEncEntropy::encodeCoeff( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiLog2Size )
{

#if inter_direct_skip_bug
#else
  if ( pcCU->isSkip(uiAbsPartIdx) ) return;
#endif
  UInt uiMinCoeffSize = 1<<(pcCU->getPic()->getLog2MinCUSize()<<1);
  UInt uiLumaOffset   = uiMinCoeffSize*uiAbsPartIdx;
  UInt uiChromaOffset = uiLumaOffset>>2;
  
  if( !pcCU->isIntra(uiAbsPartIdx) )
  {
#if !niu_intercbf_revise
	  m_pcEntropyCoderIf->codeAllCbfs(pcCU, uiAbsPartIdx);
#endif
    if( ( pcCU->getCbf(uiAbsPartIdx,TEXT_LUMA,0) == 0 ) && ( pcCU->getCbf(uiAbsPartIdx,TEXT_CHROMA_U,0) == 0 ) && ( pcCU->getCbf(uiAbsPartIdx,TEXT_CHROMA_V,0) == 0 ) )
      return;
  }

#if niu_write_cbf
#if niu_rate_divide_2
#else
  m_pcEntropyCoderIf->codeCbf(pcCU,uiAbsPartIdx, TEXT_LUMA,     0);
#endif
#else
  m_pcEntropyCoderIf->codeCbf(pcCU, uiAbsPartIdx, TEXT_LUMA, 0);
  m_pcEntropyCoderIf->codeCbf(pcCU,uiAbsPartIdx, TEXT_CHROMA_U, 0);
  m_pcEntropyCoderIf->codeCbf(pcCU,uiAbsPartIdx, TEXT_CHROMA_V, 0);
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

  if( !pcCU->isIntra(uiAbsPartIdx) )
  {
    if( pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0) || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0) || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0) )
    {
#if !rdot_bug
        encodeTransformIdx( pcCU, uiAbsPartIdx );
#endif
    }
  }

  xEncodeCoeff( pcCU, pcCU->getCoeffY()  + uiLumaOffset,   uiAbsPartIdx, uiDepth, uiLog2Size,   0, uiLumaTrMode,   TEXT_LUMA     );
#if ZHANGYI_INTRA
#if WLQ_inter_Chroma_TU
  xEncodeCoeff( pcCU, pcCU->getCoeffCb() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiLog2Size-1, 0, 0, TEXT_CHROMA_U );
  xEncodeCoeff( pcCU, pcCU->getCoeffCr() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiLog2Size-1, 0, 0, TEXT_CHROMA_V );
#else
  xEncodeCoeff( pcCU, pcCU->getCoeffCb() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiLog2Size-1, 0, uiChromaTrMode, TEXT_CHROMA_U );
  xEncodeCoeff( pcCU, pcCU->getCoeffCr() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiLog2Size-1, 0, uiChromaTrMode, TEXT_CHROMA_V );
#endif
#else
  xEncodeCoeff( pcCU, pcCU->getCoeffCb() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiLog2Size-1, 0, uiChromaTrMode, TEXT_CHROMA_U );
  xEncodeCoeff( pcCU, pcCU->getCoeffCr() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiLog2Size-1, 0, uiChromaTrMode, TEXT_CHROMA_V );
#endif
}

Void TEncEntropy::encodeCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoeff, UInt uiAbsPartIdx, UInt uiLog2Size, UInt uiDepth, TextType eType, Bool bRD )
{ // This is for Transform unit processing. This may be used at mode selection stage for Inter.
  m_pcEntropyCoderIf->codeCoeffNxN( pcCU, pcCoeff, uiAbsPartIdx, uiLog2Size, uiDepth, eType, bRD );
}

Void TEncEntropy::estimateBit (estBitsSbacStruct* pcEstBitsSbac, UInt uiSize, TextType eTType)
{
  eTType = eTType == TEXT_LUMA ? TEXT_LUMA : ( eTType == TEXT_CHROMA_U ? TEXT_CHROMA : TEXT_CHROMA_U );
  m_pcEntropyCoderIf->estBit ( pcEstBitsSbac, eTType );
}

Void TEncEntropy::encodeDBKIdx( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  if( pcCU->getPicture()->getLoopFilterDisable() )
    return;
  m_pcEntropyCoderIf->codeDBKIdx( pcCU, uiAbsPartIdx );
}
