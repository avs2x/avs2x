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

/** \file     TDecGop.cpp
    \brief    GOP decoder class
*/

#include "TDecGop.h"
#include "TDecSbac.h"

#include <time.h>

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TDecGop::TDecGop()
{
  m_iGopSize = 0;
}

TDecGop::~TDecGop()
{
  
}

Void TDecGop::create()
{
  
}

Void TDecGop::destroy()
{
  
}

Void TDecGop::init( TDecEntropy*            pcEntropyDecoder, 
                   TDecSbac*               pcSbacDecoder, 
                   TDecPicture*              pcPictureDecoder, 
                   TDecLoopFilter*         pcLoopFilter )
{
  m_pcEntropyDecoder      = pcEntropyDecoder;
  m_pcSbacDecoder         = pcSbacDecoder;
  m_pcPictureDecoder        = pcPictureDecoder;
  m_pcLoopFilter          = pcLoopFilter;
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TDecGop::decompressGop (Bool bEos, TComBitstream* pcBitstream, TComPic*& rpcPic)
{
  TComPicture*  pcPicture = rpcPic->getPicture();
  
  //-- For time output for each picture
  long iBeforeTime = clock();
  
  m_pcEntropyDecoder->setEntropyDecoder (m_pcSbacDecoder);
  
  m_pcEntropyDecoder->setBitstream      (pcBitstream);
  m_pcEntropyDecoder->resetEntropy      (pcPicture);
  
  m_pcPictureDecoder->decompressPicture(pcBitstream, rpcPic);
  
  // deblocking filter
  m_pcLoopFilter->setCfg(pcPicture->getLoopFilterDisable());
  m_pcLoopFilter->loopFilterPic( rpcPic );
  
  //-- For time output for each picture
#if F_RPS
  printf("\nPOC %4d ( %c-PICTURE, QP%3d ) ",
	  pcPicture->getPOC(),
	  pcPicture->isIntra() ? 'I' : pcPicture->isInterF() ? 'F' : pcPicture->isInterP() ? 'P' : 'B',
	  pcPicture->getPictureQp());
#else
  printf("\nPOC %4d ( %c-PICTURE, QP%3d ) ",
         pcPicture->getPOC(),
         pcPicture->isIntra() ? 'I' : pcPicture->isInterP() ? 'P' : 'B',
         pcPicture->getPictureQp() );
#endif


  Double dDecTime = (double)(clock()-iBeforeTime) / CLOCKS_PER_SEC;
  printf ("[DT %6.3f] ", dDecTime );
  
#if RPS
  for (Int iRefPic = 0; iRefPic < 2; iRefPic++)
  {
    for (Int iNumRefIdx = 0; iNumRefIdx<pcPicture->getPicHeader()->getNumRefIdx(RefPic(iRefPic)); iNumRefIdx++)
    {
      if (pcPicture->getPicHeader()->getRefPic(RefPic(iRefPic), iNumRefIdx))
        printf("[L%d %d]", iRefPic, pcPicture->getPicHeader()->getRefPOC(RefPic(iRefPic), iNumRefIdx));
    }
  }
#else
  for (Int iRefPic = 0; iRefPic < 2; iRefPic++)
  {
    if (pcPicture->getRefPic(RefPic(iRefPic)))
      printf("[L%d %d]", iRefPic, pcPicture->getRefPOC(RefPic(iRefPic)));
  }
#endif

  rpcPic->setReconMark(true);


#if DPB
  rpcPic->compressMotion();
#endif


}

