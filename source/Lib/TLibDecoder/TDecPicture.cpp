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

/** \file     TDecPicture.cpp
    \brief    picture decoder class
*/

#include "TDecPicture.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TDecPicture::TDecPicture()
{
}

TDecPicture::~TDecPicture()
{
}

Void TDecPicture::create( TComPicture* pcPicture, Int iWidth, Int iHeight, UInt uiLog2MaxSize, UInt uiMaxDepth )
{
}

Void TDecPicture::destroy()
{
}

Void TDecPicture::init(TDecEntropy* pcEntropyDecoder, TDecCu* pcCuDecoder)
{
  m_pcEntropyDecoder  = pcEntropyDecoder;
  m_pcCuDecoder       = pcCuDecoder;
}

Void TDecPicture::decompressPicture(TComBitstream* pcBitstream, TComPic*& rpcPic)
{
  TComDataCU* pcCU;
  UInt        uiIsLast = 0;

  // decoder don't need prediction & residual frame buffer
  rpcPic->setPicYuvPred( 0 );
  rpcPic->setPicYuvResi( 0 );

  rpcPic->getPicYuvRec()->reset();

  // set QP
  m_pcCuDecoder->setQp( rpcPic->getPicture()->getPictureQp(), rpcPic->getPicture()->getPictureChromaQp() );

  // for all CUs
  for( Int iCUAddr = 0; !uiIsLast && iCUAddr < rpcPic->getNumCUsInFrame(); iCUAddr++ )
  {
    pcCU = rpcPic->getCU( iCUAddr );
    pcCU->initCU( rpcPic, iCUAddr );

    // decode CU
    //printf("\n############################################################\n");
    m_pcCuDecoder->decodeCU( pcCU, uiIsLast );

    // decompress CU
    m_pcCuDecoder->decompressCU ( pcCU );
  }
}
