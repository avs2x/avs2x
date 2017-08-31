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

/** \file     TComPrediction.cpp
    \brief    prediction class
*/

#include <memory.h>
#include "TComPrediction.h"

//kolya
enum PassType
{
  TWO_PASSES = -2, ONE_PASS = -1, NO_PASS = 0
};

#if ZHANGYI_INTRA
const unsigned char g_aucXYflg[NUM_INTRA_MODE] = {
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	1, 1, 1, 1, 1,
	1, 1, 1
};

const char g_aucDirDx[NUM_INTRA_MODE] = {
	0, 0, 0, 11, 2,
	11, 1, 8, 1, 4,
	1, 1, 0, 1, 1,
	4, 1, 8, 1, 11,
	2, 11, 4, 8, 0,
	8, 4, 11, 2, 11,
	1, 8, 1
};

const char g_aucDirDy[NUM_INTRA_MODE] = {
	0, 0, 0, -4, -1,
	-8, -1, -11, -2, -11,
	-4, -8, 0, 8, 4,
	11, 2, 11, 1, 8,
	1, 4, 1, 1, 0,
	-1, -1, -4, -1, -8,
	-1, -11, -2
};

const char g_aucSign[NUM_INTRA_MODE] = {
	0, 0, 0, -1, -1,
	-1, -1, -1, -1, -1,
	-1, -1, 0, 1, 1,
	1, 1, 1, 1, 1,
	1, 1, 1, 1, 0,
	-1, -1, -1, -1, -1,
	-1, -1, -1
};
const char g_aucDirDxDy[2][NUM_INTRA_MODE][2] = {
	{
		// dx/dy
		{0, 0}, {0, 0}, {0, 0}, {11, 2}, {2, 0},
		{11, 3}, {1, 0}, {93, 7}, {1, 1}, {93, 8},
		{1, 2}, {1, 3}, {0, 0}, {1, 3}, {1, 2},
		{93, 8}, {1, 1}, {93, 7}, {1, 0}, {11, 3},
		{2, 0}, {11, 2}, {4, 0}, {8, 0}, {0, 0},
		{8, 0}, {4, 0}, {11, 2}, {2, 0}, {11, 3},
		{1, 0}, {93, 7}, {1, 1},
	},
	{
		// dy/dx
		{0, 0}, {0, 0}, {0, 0}, {93, 8}, {1, 1},
		{93, 7}, {1, 0}, {11, 3}, {2, 0}, {11, 2},
		{4, 0}, {8, 0}, {0, 0}, {8, 0}, {4, 0},
		{11, 2}, {2, 0}, {11, 3}, {1, 0}, {93, 7},
		{1, 1}, {93, 8}, {1, 2}, {1, 3}, {0, 0},
		{1, 3}, {1, 2}, {93, 8}, {1, 1}, {93, 7},
		{1, 0}, {11, 3}, {2, 0}
		}
};
#endif

// ====================================================================================================================
// Constructor / destructor / initialize
// ====================================================================================================================

TComPrediction::TComPrediction()
{
  m_piYuvExt = NULL;
}

TComPrediction::~TComPrediction()
{

#if YQH_BUFF_RELEASE

 for(UInt ch=0; ch<MAX_NUM_COMPONENT; ch++)
  {
    for(UInt buf=0; buf<NUM_PRED_BUF; buf++)
    {
      delete [] m_piYuvExt2g[ch][buf];
      m_piYuvExt2g[ch][buf] = NULL;
    }
  }

#endif

  m_cYuvExt.destroy();
#if rd_sym
  m_cYuvExtSym.destroy();
#endif
  delete[] m_piYuvExt;

  m_acYuvPred[0].destroy();
  m_acYuvPred[1].destroy();
  
  m_cYuvPredTemp.destroy();

#if F_DHP_SYC_OP
  m_acYuvPredDual[0]->destroy();
  delete m_acYuvPredDual[0];
  m_acYuvPredDual[0] = NULL;
  m_acYuvPredDual[1]->destroy();
  delete m_acYuvPredDual[1];
  m_acYuvPredDual[1] = NULL;
#endif

#if F_MHPSKIP_SYC
  m_acMHPSkipYuvPred[0].destroy();
  m_acMHPSkipYuvPred[1].destroy();
#endif
#if DCTIF || chazhi_bug
	Int i, j;
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			m_filteredBlock[i][j].destroy();
		}
		m_filteredBlockTmp[i].destroy();
	}
#endif
}

Void TComPrediction::initTempBuff()
{
  if( m_piYuvExt == NULL )
  {
#if DCTIF || chazhi_bug
		Int extWidth = MAX_CU_SIZE + 16;
		Int extHeight = MAX_CU_SIZE + 1;
		Int i, j;
		for (i = 0; i < 4; i++)
		{
			m_filteredBlockTmp[i].create(extWidth, extHeight + 7);
			for (j = 0; j < 4; j++)
			{
				m_filteredBlock[i][j].create(extWidth, extHeight);
			}
		}
#endif
    m_iYuvExtStride = 1<<(g_uiLog2MaxCUSize+4);
    
    m_cYuvExt.create( g_uiLog2MaxCUSize+4 );
#if rd_sym
	m_cYuvExtSym.create(g_uiLog2MaxCUSize + 4);
#endif
#if ZHANGYI_INTRA
	for(UInt ch=0; ch<MAX_NUM_COMPONENT; ch++) //MAX_NUM_COMPONENT
	{
		for(UInt buf=0; buf<NUM_PRED_BUF; buf++)//NUM_PRED_BUF
		{
			m_piYuvExt2g[ch][buf] = new Pel[ (MAX_CU_SIZE*2+1) * (MAX_CU_SIZE*2+1) ];
		}
	}
#endif
    m_piYuvExt = new Int[ m_iYuvExtStride*m_iYuvExtStride ];

    m_piYuvExt2 = m_piYuvExt + m_iYuvExtStride * (m_iYuvExtStride >> 1);

    // new structure
    m_acYuvPred[0] .create( g_uiLog2MaxCUSize );
    m_acYuvPred[1] .create( g_uiLog2MaxCUSize );
    
    m_cYuvPredTemp.create( g_uiLog2MaxCUSize );
#if F_DHP_SYC_OP
	m_acYuvPredDual[0] = new TComYuv;
	m_acYuvPredDual[0]->create(g_uiLog2MaxCUSize);
	m_acYuvPredDual[1] = new TComYuv;
	m_acYuvPredDual[1]->create(g_uiLog2MaxCUSize);
#endif
#if F_MHPSKIP_SYC

	m_acMHPSkipYuvPred[0].create(g_uiLog2MaxCUSize);
	m_acMHPSkipYuvPred[1].create(g_uiLog2MaxCUSize);
#endif

  }
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

#if ZHANGYI_INTRA
Void TComPrediction::xPredIntraVertAdi(Pel* ptrSrc, UInt srcStride, Pel* ptrDst, UInt dstStride, UInt uiWidth, UInt uiHeight ) {
	Pel* ptrSrcTemp = ptrSrc + 1;
	Pel* ptrDstTemp = ptrDst;
	int x, y;
	Pel predValue;
	for ( y = 0; y < uiWidth; y++ ) //every col
	{
		predValue = ptrSrcTemp[y];
		for (x = 0; x < uiHeight; x++ )
		{
			ptrDstTemp[x*dstStride + y] = predValue;
		}
	}
}

Void TComPrediction::xPredIntraHorAdi(Pel* ptrSrc, UInt srcStride, Pel* ptrDst, UInt dstStride, UInt uiWidth, UInt uiHeight ) {
	Pel* ptrSrcTemp = ptrSrc + srcStride;
	Pel* ptrDstTemp = ptrDst;
	int x, y;
	Pel predValue;
	for ( y = 0; y < uiHeight; y++ ) //for every row
	{
		predValue = (*ptrSrcTemp);
		for (x = 0; x < uiWidth; x++ )
		{
			ptrDstTemp[x] = predValue;
		}
		ptrSrcTemp += srcStride;
		ptrDstTemp += dstStride;
	}
}

Void TComPrediction::xPredIntraDCAdi(Pel* ptrSrc, UInt srcStride, Pel* ptrDst, UInt dstStride, UInt uiWidth, UInt uiHeight, int bAboveAvail, int bLeftAvail, UInt bitDepth ) {
	int x, y;
	int iDCValue = 0;
	Pel* ptrSrcTemp;

	if (bLeftAvail)
	{
		ptrSrcTemp = ptrSrc + srcStride;
		for (y = 0; y < uiHeight; y++)
		{
			iDCValue += (*ptrSrcTemp);
			ptrSrcTemp += srcStride;
		}
		
		if (bAboveAvail)
		{
			ptrSrcTemp = ptrSrc + 1;
			for ( x = 0; x < uiWidth; x++ )
			{
				iDCValue += ptrSrcTemp[x];
			}
			iDCValue += ((uiWidth + uiHeight) >> 1);
			iDCValue = (iDCValue * (512 / (uiWidth + uiHeight))) >> 9;
		}
		else
		{
			#if	ZHANGYI_INTRA_SDIP_BUG_YQH
			iDCValue += uiHeight / 2;
			iDCValue /= uiHeight;
#else
			iDCValue += uiWidth / 2;
			iDCValue /= uiWidth;
#endif
		}
	}
	else
	{
		if (bAboveAvail)
		{
			ptrSrcTemp = ptrSrc + 1;
			for (x = 0; x < uiWidth; x++)
			{
				iDCValue += ptrSrcTemp[x];
			}
			iDCValue += uiWidth / 2;
			iDCValue /= uiWidth;
		}
		else
		{
			iDCValue = (1 << (bitDepth - 1));
		}
	}
	for (y = 0; y < uiHeight; y++)
	{
		for (x = 0; x < uiWidth; x++)
		{
			ptrDst[y * dstStride + x] = iDCValue;
		}
	}
}
Void TComPrediction::xPredIntraPlaneAdi(Pel* ptrSrc, UInt srcStride, Pel* ptrDst, UInt dstStride, UInt uiWidth, UInt uiHeight, UInt bitDepth ) {
	int iH = 0;
	int iV = 0;
	int iA, iB, iC;
	int i, j, x, y;
	int iW2 = uiWidth >> 1;
	int iH2 = uiHeight >> 1;
	int ib_mult[5] = {13, 17, 5, 11, 23};
	int ib_shift[5] = {7, 10, 11, 15, 19};

	int log2uiWidth = (uiWidth == 4)?2:((uiWidth == 8)?3:((uiWidth == 16)?4:((uiWidth == 32)?5:((uiWidth == 64)?6:7)))); //zhangyiCheck: what if the biggest lcu size is larger than 128
	int log2uiHeight = (uiHeight == 4)?2:((uiHeight == 8)?3:((uiHeight == 16)?4:((uiHeight == 32)?5:((uiHeight == 64)?6:7))));

	int im_h = ib_mult[log2uiWidth - 2];
	int is_h = ib_shift[log2uiWidth - 2]; 
	int im_v = ib_mult[log2uiHeight - 2];
	int is_v = ib_shift[log2uiHeight - 2];
	int iTmp, iTmp2;

	

	for (i = 0; i < iW2; i++)
	{
		iH += (i+1) * (ptrSrc[(uiWidth >> 1) + 1 + i] - ptrSrc[(uiWidth >> 1) - 1 - i]);
	}
	for (j = 0; j < iH2; j++)
	{
		iV += (j+1) * (ptrSrc[((uiHeight >> 1) + 1 + j) * srcStride] - ptrSrc[((uiHeight >> 1) - 1 - j) * srcStride]);
	}
	iA = (ptrSrc[uiWidth] + ptrSrc[srcStride * uiHeight]) << 4;
	iB = ((iH << 5) * im_h + (1 << (is_h - 1))) >> is_h;
	iC = ((iV << 5) * im_v + (1 << (is_v - 1))) >> is_v;

	iTmp = iA - (iH2 - 1) * iC - (iW2 - 1) * iB + 16;
	for (y = 0; y < uiHeight; y++) {
		iTmp2 = iTmp;
		for (x = 0; x < uiWidth; x++) {
			//img->mprr[PLANE_PRED][y][x] = Clip( iTmp2 >> 5 );
			ptrDst[y * dstStride + x] = Clip3(0, (1 << bitDepth) - 1, iTmp2 >> 5);
			iTmp2 += iB;
		}
		iTmp += iC;
	}
}
Void TComPrediction::xPredIntraBiAdi(Pel* ptrSrc, UInt srcStride, Pel* ptrDst, UInt dstStride, UInt uiWidth, UInt uiHeight, UInt bitDepth ) {
	int x, y;
	int log2uiWidth = (uiWidth == 4)?2:((uiWidth == 8)?3:((uiWidth == 16)?4:((uiWidth == 32)?5:((uiWidth == 64)?6:7)))); //zhangyiCheck: what if the biggest lcu size is larger than 128
	int log2uiHeight = (uiHeight == 4)?2:((uiHeight == 8)?3:((uiHeight == 16)?4:((uiHeight == 32)?5:((uiHeight == 64)?6:7))));
	int ishift_x = log2uiWidth;
	int ishift_y = log2uiHeight;
	int ishift = min(ishift_x, ishift_y);
	int ishift_xy = ishift_x + ishift_y + 1;
	int offset = 1 << (ishift_x + ishift_y);
	int a, b, c, w, wxy, tmp;
	int predx;

#if ZY_MODIFY_DATA_STRUCTURE
	int pTop[MAX_CU_SIZE] = { 0 }, pLeft[MAX_CU_SIZE] = { 0 }, pT[MAX_CU_SIZE] = { 0 }, pL[MAX_CU_SIZE] = { 0 }, wy[MAX_CU_SIZE] = { 0 };
#else
	int pTop[MAX_CU_SIZE], pLeft[MAX_CU_SIZE], pT[MAX_CU_SIZE], pL[MAX_CU_SIZE], wy[MAX_CU_SIZE];
#endif

	for (x = 0; x < uiWidth; x++)
	{
		pTop[x] = ptrSrc[1 + x];
	}
	for (y = 0; y < uiHeight; y++)
	{
		pLeft[y] = ptrSrc[srcStride * (y+1)];
	}
	a = pTop[uiWidth - 1];
	b = pLeft[uiHeight - 1];
	c = (uiWidth == uiHeight) ? (a + b + 1) >> 1 :
		(((a << ishift_x) + (b << ishift_y)) * 13 + (1 << (ishift + 5))) >> (ishift + 6);
	w = (c << 1) - a - b;

#if ZY_MODIFY_DATA_STRUCTURE
	for (x = 0; x < uiWidth; x++)
#else
	for (x = 0; x < uiHeight; x++)
#endif
	{
		pT[x] = b - pTop[x];
		pTop[x] <<= ishift_y;
	}
	tmp = 0;
	for (y = 0; y < uiHeight; y++)
	{
		pL[y] = a - pLeft[y];
		pLeft[y] <<= ishift_x;
		wy[y] = tmp;
		tmp += w;
	}

	for (y = 0; y < uiHeight; y++)
	{
		predx = pLeft[y];
		wxy = 0;
		for (x = 0; x < uiWidth; x++)
		{
			predx += pL[y];
#if niu_IntraBiAdi_FIX
			pTop[x] += pT[x];
#else
			pTop[x] += pL[x];
#endif
			ptrDst[y * dstStride + x] = Clip3(0, ((1<<bitDepth) - 1), (((predx << ishift_y) + (pTop[x] << ishift_x) + wxy + offset) >> ishift_xy));
			wxy += wy[y];
		}
	}
}
Void TComPrediction::xPredIntraAngAdi(Pel* ptrSrc, UInt srcStride, Pel* ptrDst, UInt dstStride, UInt uiDirMode, UInt uiWidth, UInt uiHeight) {
	int  iDx, iDy, i, j, iTempDx, iTempDy, iXx, iXy, iYx, iYy;
	int  uixyflag = 0; // 0 for x axis 1 for y axis
	int offset, offsetx, offsety;
	int iX, iY, iXn, iYn, iXnN1, iYnN1, iXnP2, iYnP2;
	int iDxy;
	int iWidth2 = uiWidth << 1;
	int iHeight2 = uiHeight << 1;
#if ZY_INTRA_MODIFY_PRED
	Pel* refereceSamples = new Pel[(uiWidth << 1) + (uiHeight << 1) + 1];
	Pel* pMidRef = refereceSamples + (uiHeight << 1) + 1;
	for (j = 0; j <= (uiWidth << 1); j++) //above ref pixels
	{
		pMidRef[j] = ptrSrc[j];
	}
	for (j = 1; j <= (uiHeight << 1); j++)
	{
		pMidRef[-j] = ptrSrc[j * srcStride];
	}
	short *rpSrc = pMidRef;
#elif niu_intra_data_struct
#if ZY_MODIFY_DATA_STRUCTURE
	Pel tmp[MAX_CU_SIZE * 4 + 1] = { 0 };
#else
	Pel tmp[MAX_CU_SIZE * 4 + 1];
#endif
	for (i = 0; i < iHeight2; i++)
		tmp[i] = ptrSrc[(iHeight2 - i) * (iWidth2 + 1)];
	tmp[iHeight2] = ptrSrc[0];
#if ZY_MODIFY_DATA_STRUCTURE
	for (i = 0; i < iWidth2; i++)
#else
	for (i = 0; i < iHeight2; i++)
#endif
		tmp[i + 1 + iHeight2] = ptrSrc[i + 1];

	short *EP = tmp;
	//EP = EP + (iHeight2);
	short *rpSrc = EP;
#else
	short  *rpSrc = ptrSrc;
#endif

	iDx      = g_aucDirDx[uiDirMode];
	iDy      = g_aucDirDy[uiDirMode];
	uixyflag = g_aucXYflg[uiDirMode];
	iDxy     = g_aucSign [uiDirMode];

	for (j = 0; j < uiHeight; j++)
	{
		for (i = 0; i < uiWidth; i++)
		{
			if (iDxy < 0) //select context pixel based on uixyflag
			{
				if (uixyflag == 0) //refer to the pixels above
				{
					// case x-line
					iTempDy = j - (-1);
					iTempDx = getContextPixel(uiDirMode, 0, iTempDy, &offset);
					iX = i + iTempDx;
					iY = -1;
				}
				else //refer to the left pixels
				{
					iTempDx = i + 1;
					iTempDy = getContextPixel(uiDirMode, 1, iTempDx, &offset);
					iX = -1;
					iY = j + iTempDy;
				}
			}
			else // select context pixel based on the distance
			{
				iTempDy = j + 1;
				iTempDx = getContextPixel(uiDirMode, 0, iTempDy, &offsetx);
				iTempDx = -iTempDx;
				iXx     = i + iTempDx;
				iYx     = -1;

				iTempDx = i - (-1);
				iTempDy = getContextPixel(uiDirMode, 1, iTempDx, &offsety);
				iTempDy = -iTempDy;
				iXy     = -1;
				iYy     = j + iTempDy;

				if (iYy <= -1) {
					iX = iXx;
					iY = iYx;
					offset = offsetx;
				} else {
					iX = iXy;
					iY = iYy;
					offset = offsety;
				}
			}
			if (iY == -1) {
#if ZY_INTRA_MODIFY_PRED
				rpSrc = pMidRef + 1;
#elif niu_intra_data_struct
				rpSrc = EP + iHeight2 + 1;
#else
				rpSrc = ptrSrc + 1;
#endif

				if (iDxy < 0) {
					iXnN1 = iX - 1;
					iXn   = iX + 1;
					iXnP2 = iX + 2;
				} else {
					iXnN1 = iX + 1;
					iXn   = iX - 1;
					iXnP2 = iX - 2;
				}
				
				iXnN1 = min(iWidth2 - 1, iXnN1);
				iX    = min(iWidth2 - 1, iX);
				iXn   = min(iWidth2 - 1, iXn);
				iXnP2 = min(iWidth2 - 1, iXnP2);

				ptrDst[j * dstStride + i] = (rpSrc[iXnN1] * (32 - offset) + rpSrc[iX] * (64 - offset) + rpSrc[iXn] * (32 + offset) + rpSrc[iXnP2] * offset + 64) >> 7;
			}
			else if (iX == -1) {
#if ZY_INTRA_MODIFY_PRED
				rpSrc = pMidRef - 1;
#elif niu_intra_data_struct
				rpSrc = EP + iHeight2 - 1;
#else
				rpSrc = ptrSrc - 1;
#endif

				if (iDxy < 0) {
					iYnN1 = iY - 1;
					iYn   = iY + 1;
					iYnP2 = iY + 2;
				} else {
					iYnN1 = iY + 1;
					iYn   = iY - 1;
					iYnP2 = iY - 2;
				}
				iYnN1 = min(iHeight2 - 1, iYnN1);
				iY    = min(iHeight2 - 1, iY);
				iYn   = min(iHeight2 - 1, iYn);
				iYnP2 = min(iHeight2 - 1, iYnP2);

				ptrDst[j * dstStride + i] = (rpSrc[-iYnN1] * (32 - offset) + rpSrc[-iY] * (64 - offset) + rpSrc[-iYn] * (32 + offset) + rpSrc[-iYnP2] * offset + 64) >> 7;
			}
		}
	}
}

Int TComPrediction::getContextPixel(int uiDirMode, int uiXYflag, int iTempD, int *offset)
{
	int imult = g_aucDirDxDy[uiXYflag][uiDirMode][0];
	int ishift = g_aucDirDxDy[uiXYflag][uiDirMode][1];

	int iTempDn = iTempD * imult >> ishift;
	*offset = ((iTempD * imult * 32) >> ishift) - iTempDn * 32;
	return iTempDn;
}

#if ZHANGYI_INTRA_SDIP
Void  TComPrediction::predIntraAngAVS(const TextType compID, UInt uiDirMode, UInt uiOrgStride, Pel* piPred, UInt uiStride, UInt uiWidth, UInt uiHeight, Int bAbove, Int bLeft, UInt bitDepth) {
#else
Void  TComPrediction::predIntraAngAVS(const TextType compID, UInt uiDirMode, UInt uiOrgStride, Pel* piPred, UInt uiStride, UInt uiSize, Int bAbove, Int bLeft, UInt bitDepth) {
#endif
	Pel *pDst = piPred;
	// get starting pixel in block
#if ZHANGYI_INTRA_SDIP
	const Int sw = (2 * uiWidth + 1);
#else
	const Int sw = (2 * uiSize + 1);
#endif

	Pel *ptrSrc;
	if (compID == TEXT_CHROMA_U)
	{
		ptrSrc = m_piYuvExt2g[COMPONENT_Cb][PRED_BUF_UNFILTERED];
	}
	else if (compID == TEXT_CHROMA_V)
	{
		ptrSrc = m_piYuvExt2g[COMPONENT_Cr][PRED_BUF_UNFILTERED];
	}
	else
	{
		ptrSrc = m_piYuvExt2g[COMPONENT_Y][PRED_BUF_UNFILTERED];
	}
#if ZHANGYI_INTRA_SDIP
	switch (uiDirMode)
	{
	case INTRA_VER_IDX:
		xPredIntraVertAdi(ptrSrc, sw, pDst, uiStride, uiWidth, uiHeight);
		break;
	case INTRA_HOR_IDX:
		xPredIntraHorAdi(ptrSrc, sw, pDst, uiStride, uiWidth, uiHeight);
		break;
	case INTRA_DC_IDX:
		xPredIntraDCAdi(ptrSrc, sw, pDst, uiStride, uiWidth, uiHeight, bAbove, bLeft, bitDepth);
		break;
	case INTRA_PLANAR_IDX:
		xPredIntraPlaneAdi(ptrSrc, sw, pDst, uiStride, uiWidth, uiHeight, bitDepth);
		break;
	case INTRA_BI_IDX:
		xPredIntraBiAdi(ptrSrc, sw, pDst, uiStride, uiWidth, uiHeight, bitDepth);
		break;
	default:
		xPredIntraAngAdi(ptrSrc, sw, pDst, uiStride, uiDirMode, uiWidth, uiHeight);
		break;
	}
#else
	switch (uiDirMode)
	{
	case INTRA_VER_IDX:
		xPredIntraVertAdi( ptrSrc, sw, pDst, uiStride, uiSize, uiSize );
		break;
	case INTRA_HOR_IDX:
		xPredIntraHorAdi( ptrSrc, sw, pDst, uiStride, uiSize, uiSize);
		break;
	case INTRA_DC_IDX:
		xPredIntraDCAdi( ptrSrc, sw, pDst, uiStride, uiSize, uiSize, bAbove, bLeft, bitDepth);
		break;
	case INTRA_PLANAR_IDX:
		xPredIntraPlaneAdi( ptrSrc, sw, pDst, uiStride, uiSize, uiSize, bitDepth );
		break;
	case INTRA_BI_IDX:
		xPredIntraBiAdi( ptrSrc, sw, pDst, uiStride, uiSize, uiSize, bitDepth);
		break;
	default:
		xPredIntraAngAdi( ptrSrc, sw, pDst, uiStride, uiDirMode, uiSize, uiSize);
		break;
	}
#endif
}

#endif

#if ZHANGYI_INTRA
Void TComPrediction::predIntraAng( const TextType compID, UInt uiDirMode, Pel* piOrg , UInt uiOrgStride, Pel* piPred, UInt uiStride, UInt uiSize, Bool bAbove, Bool bLeft)
{
	
	Pel *pDst = piPred;

	// get starting pixel in block
	const Int sw = (2 * uiSize + 1);

	Pel *ptrSrc;
	if (compID == TEXT_CHROMA_U)
	{
		ptrSrc = m_piYuvExt2g[COMPONENT_Cb][PRED_BUF_UNFILTERED];
	}
	else if (compID == TEXT_CHROMA_V)
	{
		ptrSrc = m_piYuvExt2g[COMPONENT_Cr][PRED_BUF_UNFILTERED];
	}
	else
	{
		ptrSrc = m_piYuvExt2g[COMPONENT_Y][PRED_BUF_UNFILTERED];
	}

	if ( uiDirMode == INTRA_PLANAR_IDX )
	{
		xPredIntraPlanar( ptrSrc+sw+1, sw, pDst, uiStride, uiSize, uiSize);//, channelType, format );
	}
	else
	{
		// Create the prediction
		//TComDataCU *const pcCU              = rTu.getCU();
		//const UInt              uiAbsPartIdx      = rTu.GetAbsPartIdxTU();
		//const Bool              enableEdgeFilters = !(pcCU->isRDPCMEnabled(uiAbsPartIdx) && pcCU->getCUTransquantBypass(uiAbsPartIdx));

		xPredIntraAng( 8, ptrSrc+sw+1, sw, pDst, uiStride, uiSize, uiSize, uiDirMode, bAbove, bLeft);//, enableEdgeFilters );

		/*if(( uiDirMode == INTRA_DC_IDX ) && bAbove && bLeft )
		{
			xDCPredFiltering( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight, channelType );
		}*/ //滤波
	}

}

Pel TComPrediction::predIntraGetPredValDC( const Pel* pSrc, Int iSrcStride, UInt iWidth, UInt iHeight, Bool bAbove, Bool bLeft )
{
	//assert(iWidth > 0 && iHeight > 0);
	Int iInd, iSum = 0;
	Pel pDcVal;

	if (bAbove)
	{
		for (iInd = 0;iInd < iWidth;iInd++)
		{
			iSum += pSrc[iInd-iSrcStride];
		}
	}
	if (bLeft)
	{
		for (iInd = 0;iInd < iHeight;iInd++)
		{
			iSum += pSrc[iInd*iSrcStride-1];
		}
	}

	if (bAbove && bLeft)
	{
		pDcVal = (iSum + iWidth) / (iWidth + iHeight);
	}
	else if (bAbove)
	{
		pDcVal = (iSum + iWidth/2) / iWidth;
	}
	else if (bLeft)
	{
		pDcVal = (iSum + iHeight/2) / iHeight;
	}
	else
	{
		pDcVal = pSrc[-1]; // Default DC value already calculated and placed in the prediction array if no neighbors are available
	}

	return pDcVal;
}
Void TComPrediction::xPredIntraAng( Int bitDepth, const Pel* pSrc, Int srcStride,Pel* pTrueDst, Int dstStrideTrue,UInt uiWidth, UInt uiHeight,
	UInt dirMode, Bool blkAboveAvailable, Bool blkLeftAvailable)
{
	Int width=Int(uiWidth);
	Int height=Int(uiHeight);

	// Map the mode index to main prediction direction and angle
	//assert( dirMode != PLANAR_IDX ); //no planar
	const Bool modeDC        = dirMode==INTRA_DC_IDX;

	// Do the DC prediction
	if (modeDC)
	{
		const Pel dcval = predIntraGetPredValDC(pSrc, srcStride, width, height, blkAboveAvailable, blkLeftAvailable);

		for (Int y=height;y>0;y--, pTrueDst+=dstStrideTrue)
		{
			for (Int x=0; x<width;) // width is always a multiple of 4.
			{
				pTrueDst[x++] = dcval;
			}
		}
	}
	else // Do angular predictions
	{
		const Bool       bIsModeVer         = (dirMode >= 18);
		const Int        intraPredAngleMode = (bIsModeVer) ? (Int)dirMode - INTRA_VER_IDX :  -((Int)dirMode - INTRA_HOR_IDX);
		const Int        absAngMode         = abs(intraPredAngleMode);
		const Int        signAng            = intraPredAngleMode < 0 ? -1 : 1;
		//const Bool       edgeFilter         = bEnableEdgeFilters && isLuma(channelType) && (width <= MAXIMUM_INTRA_FILTERED_WIDTH) && (height <= MAXIMUM_INTRA_FILTERED_HEIGHT);

		// Set bitshifts and scale the angle parameter to block size
		static const Int angTable[9]    = {0,    2,    5,   9,  13,  17,  21,  26,  32};
		static const Int invAngTable[9] = {0, 4096, 1638, 910, 630, 482, 390, 315, 256}; // (256 * 32) / Angle
		Int invAngle                    = invAngTable[absAngMode];
		Int absAng                      = angTable[absAngMode];
		Int intraPredAngle              = signAng * absAng;

		Pel* refMain;
		Pel* refSide;

		Pel  refAbove[2*MAX_CU_SIZE+1];
		Pel  refLeft[2*MAX_CU_SIZE+1];

		// Initialise the Main and Left reference array.
		if (intraPredAngle < 0)
		{
			const Int refMainOffsetPreScale = (bIsModeVer ? height : width ) - 1;
			const Int refMainOffset         = height - 1;
			for (Int x=0;x<width+1;x++)
			{
				refAbove[x+refMainOffset] = pSrc[x-srcStride-1];
			}
			for (Int y=0;y<height+1;y++)
			{
				refLeft[y+refMainOffset] = pSrc[(y-1)*srcStride-1];
			}
			refMain = (bIsModeVer ? refAbove : refLeft)  + refMainOffset;
			refSide = (bIsModeVer ? refLeft  : refAbove) + refMainOffset;

			// Extend the Main reference to the left.
			Int invAngleSum    = 128;       // rounding for (shift by 8)
			for (Int k=-1; k>(refMainOffsetPreScale+1)*intraPredAngle>>5; k--)
			{
				invAngleSum += invAngle;
				refMain[k] = refSide[invAngleSum>>8];
			}
		}
		else
		{
			for (Int x=0;x<2*width+1;x++)
			{
				refAbove[x] = pSrc[x-srcStride-1];
			}
			for (Int y=0;y<2*height+1;y++)
			{
				refLeft[y] = pSrc[(y-1)*srcStride-1];
			}
			refMain = bIsModeVer ? refAbove : refLeft ;
			refSide = bIsModeVer ? refLeft  : refAbove;
		}

		// swap width/height if we are doing a horizontal mode:
		Pel tempArray[MAX_CU_SIZE*MAX_CU_SIZE];
		const Int dstStride = bIsModeVer ? dstStrideTrue : MAX_CU_SIZE;
		Pel *pDst = bIsModeVer ? pTrueDst : tempArray;
		if (!bIsModeVer)
		{
			std::swap(width, height);
		}

		if (intraPredAngle == 0)  // pure vertical or pure horizontal
		{
			for (Int y=0;y<height;y++)
			{
				for (Int x=0;x<width;x++)
				{
					pDst[y*dstStride+x] = refMain[x+1];
				}
			}

			/*if (edgeFilter)
			{
				for (Int y=0;y<height;y++)
				{
					pDst[y*dstStride] = Clip3 (0, ((1 << bitDepth) - 1), pDst[y*dstStride] + (( refSide[y+1] - refSide[0] ) >> 1) );
				}
			}*/
		}
		else
		{
			Pel *pDsty=pDst;

			for (Int y=0, deltaPos=intraPredAngle; y<height; y++, deltaPos+=intraPredAngle, pDsty+=dstStride)
			{
				const Int deltaInt   = deltaPos >> 5;
				const Int deltaFract = deltaPos & (32 - 1);

				if (deltaFract)
				{
					// Do linear filtering
					const Pel *pRM=refMain+deltaInt+1;
					Int lastRefMainPel=*pRM++;
					for (Int x=0;x<width;pRM++,x++)
					{
						Int thisRefMainPel=*pRM;
						pDsty[x+0] = (Pel) ( ((32-deltaFract)*lastRefMainPel + deltaFract*thisRefMainPel +16) >> 5 );
						lastRefMainPel=thisRefMainPel;
					}
				}
				else
				{
					// Just copy the integer samples
					for (Int x=0;x<width; x++)
					{
						pDsty[x] = refMain[x+deltaInt+1];
					}
				}
			}
		}

		// Flip the block if this is the horizontal mode
		if (!bIsModeVer)
		{
			for (Int y=0; y<height; y++)
			{
				for (Int x=0; x<width; x++)
				{
					pTrueDst[x*dstStrideTrue] = pDst[x];
				}
				pTrueDst++;
				pDst+=dstStride;
			}
		}
	}
}

Void TComPrediction::xPredIntraPlanar( const Pel* pSrc, Int srcStride, Pel* rpDst, Int dstStride, UInt width, UInt height)//, ChannelType channelType, ChromaFormat format )
{
	assert(width <= height);

	Int leftColumn[MAX_CU_SIZE+1], topRow[MAX_CU_SIZE+1], bottomRow[MAX_CU_SIZE], rightColumn[MAX_CU_SIZE];
	UInt shift1Dhor = g_aucConvertToBit[ width ] + 2;
	UInt shift1Dver = g_aucConvertToBit[ height ] + 2;

	// Get left and above reference column and row
	for(Int k=0;k<width+1;k++)
	{
		topRow[k] = pSrc[k-srcStride];
	}

	for (Int k=0; k < height+1; k++)
	{
		leftColumn[k] = pSrc[k*srcStride-1];
	}

	// Prepare intermediate variables used in interpolation
	Int bottomLeft = leftColumn[height];
	Int topRight   = topRow[width];

	for(Int k=0;k<width;k++)
	{
		bottomRow[k]  = bottomLeft - topRow[k];
		topRow[k]     <<= shift1Dver;
	}

	for(Int k=0;k<height;k++)
	{
		rightColumn[k]  = topRight - leftColumn[k];
		leftColumn[k]   <<= shift1Dhor;
	}

	const UInt topRowShift = 0;

	// Generate prediction signal
	for (Int y=0;y<height;y++)
	{
		Int horPred = leftColumn[y] + width;
		for (Int x=0;x<width;x++)
		{
			horPred += rightColumn[y];
			topRow[x] += bottomRow[x];

			// NOTE: RExt - The intermediate shift right could be rolled into the final shift right,
			//              thereby increasing the accuracy of the calculation
			// eg  rpDst[y*dstStride+x] = ( (horPred<<topRowShift) + topRow[x] ) >> (shift1Dver+1);
			Int vertPred = ((topRow[x] + topRowShift)>>topRowShift);
			rpDst[y*dstStride+x] = ( horPred + vertPred ) >> (shift1Dhor+1);
		}
	}
}

Void TComPrediction::getIntraNeighborAvailabilities(TComDataCU* pcCU, int maxCUSizeInBit, int img_x, int img_y, int bsx, int bsy, int* p_avail) 
{
	Int imgWidth = pcCU->getPicture()->getSPS()->getWidth();
	Int imgHeight = pcCU->getPicture()->getSPS()->getHeight();
	/*检查相邻块是否属于一个slice——没写(此时还没有slice)*/
	p_avail[NEIGHBOR_INTRA_LEFT]      = true;
	p_avail[NEIGHBOR_INTRA_UP]        = true;
	p_avail[NEIGHBOR_INTRA_UP_LEFT]   = true;
	p_avail[NEIGHBOR_INTRA_LEFT_DOWN] = true;
	p_avail[NEIGHBOR_INTRA_UP_RIGHT]  = true;
	/*检查是否是边界块（因为不清楚这里是否会padding）*/
	if (img_x == 0 && img_y == 0 ) //左上角第一个块
	{
		p_avail[NEIGHBOR_INTRA_LEFT]      = false;
		p_avail[NEIGHBOR_INTRA_UP]        = false;
		p_avail[NEIGHBOR_INTRA_UP_LEFT]   = false;
		p_avail[NEIGHBOR_INTRA_LEFT_DOWN] = false;
		p_avail[NEIGHBOR_INTRA_UP_RIGHT]  = false;
	} else if (img_x == 0) //左边界的块
	{
		p_avail[NEIGHBOR_INTRA_LEFT]      = false;
		p_avail[NEIGHBOR_INTRA_UP_LEFT]   = false;
		p_avail[NEIGHBOR_INTRA_LEFT_DOWN] = false;
	} else if (img_y == 0) //上边界的块
	{
		p_avail[NEIGHBOR_INTRA_UP_LEFT]   = false;
		p_avail[NEIGHBOR_INTRA_UP_RIGHT]  = false;
		p_avail[NEIGHBOR_INTRA_UP]        = false;
	} else if (img_x + bsx > imgWidth)
	{
		p_avail[NEIGHBOR_INTRA_UP] = false;
	} else if (img_y + bsy > imgHeight)
	{
		p_avail[NEIGHBOR_INTRA_LEFT] = false;
	} else if (img_x + 2 * bsx > imgWidth )
	{
		p_avail[NEIGHBOR_INTRA_UP_RIGHT] = false;
	} else if (img_y + 2 * bsy > imgHeight )
	{
		p_avail[NEIGHBOR_INTRA_LEFT_DOWN] = false;
	}
	/*检查相邻块是否在当前块之前重构*/
	int leftdown = 0;
	int upright = 0;
	int b8_x = img_x >> 2;
	int b8_y = img_y >> 2;
	if (maxCUSizeInBit == (1<<B64X64_IN_BIT))
	{
		leftdown = g_Left_Down_Avail_Matrix64[(b8_y + (bsy >> 2) - 1) & 15][(b8_x) & 15];
		upright  =  g_Up_Right_Avail_Matrix64[(b8_y) & 15][(b8_x + (bsx >> 2) - 1) & 15];
	} else if (maxCUSizeInBit == (1<<B32X32_IN_BIT)) {
		leftdown = g_Left_Down_Avail_Matrix32[(b8_y + (bsy >> 2) - 1) & 7][(b8_x) & 7];
		upright  =  g_Up_Right_Avail_Matrix32[(b8_y) & 7][(b8_x + (bsx >> 2) - 1) & 7];
	} else if (maxCUSizeInBit == (1<<B16X16_IN_BIT)) {
		leftdown = g_Left_Down_Avail_Matrix16[(b8_y + (bsy >> 2) - 1) & 3][(b8_x) & 3];
		upright  =  g_Up_Right_Avail_Matrix16[(b8_y) & 3][(b8_x + (bsx >> 2) - 1) & 3];
	} else if (maxCUSizeInBit == (1<<B8X8_IN_BIT)) {
		leftdown = g_Left_Down_Avail_Matrix8[(b8_y + (bsy >> 2) - 1) & 1][(b8_x) & 1];
		upright  =  g_Up_Right_Avail_Matrix8[(b8_y) & 1][(b8_x + (bsx >> 2) - 1) & 1];
	}

	p_avail[NEIGHBOR_INTRA_LEFT_DOWN] = (leftdown && p_avail[NEIGHBOR_INTRA_LEFT_DOWN]);
	p_avail[NEIGHBOR_INTRA_UP_RIGHT]  = (upright  && p_avail[NEIGHBOR_INTRA_UP_RIGHT]);
}

Void TComPrediction::fillReferenceSamples(const Int bitDepth, TComDataCU* pcCU, const Pel* piRoiOrigin, Pel* piAdiTemp, const UInt uiCuWidth, const UInt uiCuHeight, const Int iPicStride, Int* p_avail, Int img_x, Int img_y) 
{ //参考像素存在piAdiTemp中， uiWidth: 2*puWidth + 1     uiHeight: 2 * puHeight + 1   uiCuWidth: puWidth   piRoiOrigin: 当前PU（TU）重建值指针(当前PU左上角第一个像素)   img_x: 当前PU左上角像素坐标
	UInt uiHeight = (uiCuHeight << 1) + 1;
	UInt uiWidth = (uiCuWidth << 1) + 1;
	const Pel* piRoiTemp;
	Int i, j;
	Int iDCValue = (1 << (bitDepth - 1));
	UInt imgWidth = pcCU->getPicture()->getSPS()->getWidth();
	UInt imgHeight = pcCU->getPicture()->getSPS()->getHeight();

	for (i = 0; i < uiWidth; i++ ) //全部初始化为128
	{
		piAdiTemp[i] = iDCValue;
	}
	for (i = 1; i < uiHeight; i++)
	{
		piAdiTemp[i * uiWidth] = iDCValue;
	}

	if (p_avail[NEIGHBOR_INTRA_UP]) //上边块可得，处理与上边块相邻的参考像素
	{
		piRoiTemp = piRoiOrigin - iPicStride - 1;
		for ( i = 1; i <= uiCuWidth; i++ )
		{
			piAdiTemp[i] = piRoiTemp[i];
		}
	}

	if (p_avail[NEIGHBOR_INTRA_UP_RIGHT])//右上角块可得
	{
		piRoiTemp = piRoiOrigin - iPicStride + uiCuWidth;
		for ( i = 0; i < uiCuWidth; i++ )
		{
			if (img_x + uiCuWidth + i >= imgWidth) //右上角参考像素有部分超过图像边界
			{
				piAdiTemp[uiCuWidth + i + 1] = piAdiTemp[imgWidth - img_x];
			}
			else
			{
				piAdiTemp[uiCuWidth + 1 + i] = piRoiTemp[i];
			}
		}
	}
	else//右上角块不可得
	{
		for (i = 0; i < uiCuWidth; i++ )
		{
			piAdiTemp[uiCuWidth + 1 + i] = piAdiTemp[uiCuWidth];
		}
	}

	if (p_avail[NEIGHBOR_INTRA_LEFT])//左边块可得，处理与左边块相邻的参考像素
	{
		piRoiTemp = piRoiOrigin - 1;
		for (j = 1; j <= uiCuHeight; j++ )
		{
			piAdiTemp[j * uiWidth] = *piRoiTemp;
			piRoiTemp += iPicStride;
		}
	}

	if (p_avail[NEIGHBOR_INTRA_LEFT_DOWN])//左下角块可得
	{
		piRoiTemp = piRoiOrigin + uiCuHeight * iPicStride - 1;
		for (j = 0; j < uiCuHeight; j++ )
		{
			if (img_y + uiCuHeight + j >= imgHeight) //左下角参考像素超过图像边界
			{
				piAdiTemp[(uiCuHeight + 1 + j) * uiWidth] = piAdiTemp[(imgHeight - img_y) * iPicStride];
			}
			else
			{
				piAdiTemp[(uiCuHeight + 1 + j) * uiWidth] = *piRoiTemp;
				piRoiTemp += iPicStride;
			}
		}
	}
	else
	{
		for (j = 0; j < uiCuHeight; j++ ) 
		{
			piAdiTemp[(uiCuHeight + 1 + j) * uiWidth] = piAdiTemp[uiCuHeight * uiWidth];
		}
	}
	
	if (p_avail[NEIGHBOR_INTRA_UP_LEFT])//左上角块存在
	{
		piRoiTemp = piRoiOrigin - iPicStride - 1;
		piAdiTemp[0] = *piRoiTemp;
	} else if (p_avail[NEIGHBOR_INTRA_UP])
	{
		piRoiTemp = piRoiOrigin - iPicStride;
		piAdiTemp[0] = *piRoiTemp;
	} else if (p_avail[NEIGHBOR_INTRA_LEFT])
	{
		piRoiTemp = piRoiOrigin - 1;
		piAdiTemp[0] = *piRoiTemp;
	}
}
#endif

// Function for calculating DC value of the reference samples used in Intra prediction
Void TComPrediction::predIntra( Pel* pSrc, Int iSrcStride, Pel* pDst, Int iDstStride, UInt uiSize, UInt uiDir )
{
  Int x, y;
  
  if( uiDir == 1 ) // Horizontal
  {
    for( y=0; y<uiSize; y++ )
    {
      Pel sVal = pSrc[-1];
      for( x=0; x<uiSize; x++ )
      {
        pDst[x] = sVal;
      }
      pSrc += iSrcStride;
      pDst += iDstStride;
    }
    return;
  }
  else if( uiDir == 2 ) // Vertical
  {
    pSrc -= iSrcStride;
    for( y=0; y<uiSize; y++ )
    {
      memcpy(pDst, pSrc, sizeof(Pel)*uiSize);
      pDst += iDstStride;
    }
    return;
  }

  Int iInd, iSum = 0;
  Pel pDcVal;

  // DC Mode
  pSrc-=iSrcStride;
  for (iInd = 0;iInd < uiSize;iInd++){
    iSum += pSrc[iInd];
  }
  pSrc += iSrcStride - 1;
  for (iInd = 0;iInd < uiSize;iInd++){
    iSum += pSrc[0];
    pSrc += iSrcStride;
  }

  pDcVal = (iSum + uiSize) / (uiSize<<1);

  for ( y=0; y<uiSize; y++ )
  {
    for ( x=0; x<uiSize; x++ )
    {
      pDst[x] = pDcVal;
    }
    pDst += iDstStride;
  }
  return;
}

#if DCTIF
Void TComPrediction::motionCompensation ( TComDataCU* pcCU, TComYuv* pcYuvPred, RefPic eRefPic, Int iPartIdx )
{
	Int         iWidth;
	Int         iHeight;
	UInt        uiPartAddr;

	if( iPartIdx >= 0)
	{
		pcCU->getPartIndexAndSize( iPartIdx, uiPartAddr, iWidth, iHeight );
		if ( eRefPic != REF_PIC_X )
		{
			xPredInterUni (pcCU, uiPartAddr, iWidth, iHeight, eRefPic, pcYuvPred, iPartIdx );
		}
		else
		{
			xPredInterBi  (pcCU, uiPartAddr, iWidth, iHeight, pcYuvPred, iPartIdx );
		}

		return;

	}

	for ( iPartIdx = 0; iPartIdx < pcCU->getNumPartInter(); iPartIdx++ )
	{
		pcCU->getPartIndexAndSize( iPartIdx, uiPartAddr, iWidth, iHeight );

		if ( eRefPic != REF_PIC_X )
		{
			xPredInterUni (pcCU, uiPartAddr, iWidth, iHeight, eRefPic, pcYuvPred, iPartIdx );
		}
		else
		{
			xPredInterBi  (pcCU, uiPartAddr, iWidth, iHeight, pcYuvPred, iPartIdx );
		}
	}

	return;
}
#else
#if WLQ_set_bug
#if DMH
Void TComPrediction::recDmhMode(TComDataCU *pcCU, Int uiPartIdx, RefPic iRefPic, TComYuv *pcYuvRec)
{
	Int  iWidth;
	Int  iHeight;
	UInt uiPartAddr;
	TComYuv pcYuvPred[2];
	Int uiSize = 1 << pcCU->getLog2CUSize(0);
	for (uiPartIdx = 0; uiPartIdx < pcCU->getNumPartInter(); uiPartIdx++)
	{
		pcCU->getPartIndexAndSize(uiPartIdx, uiPartAddr, iWidth, iHeight);

		xPredDmhMode(pcCU, uiPartIdx, uiPartAddr, iWidth, iHeight, REF_PIC_0);

		pcCU->getPartIndexAndSize(uiPartIdx, uiPartAddr, iWidth, iHeight);

		m_acYuvPred[0].copyPartToPartYuv(pcYuvRec, pcCU->getZorderIdxInCU() + uiPartAddr, pcCU->getZorderIdxInCU() + uiPartAddr, iWidth, iHeight);
	}
}

Void TComPrediction::xPredDmhMode(TComDataCU *pcCU, Int uiPartIdx, UInt uiPartAddr, Int iWidth, Int iHeight, RefPic iRefPic)
{
	Int  dmh_mode = pcCU->getDMHMode(0);
	TComYuv* pcYuvPredFinal = &m_acYuvPred[iRefPic];
	TComYuv pcYuvPred[2];

	TComMv   orgMv = pcCU->getCUMvField(iRefPic)->getMv(uiPartAddr);
	TComMv   tmpMvL(orgMv.getHor() + dmh_pos[dmh_mode][0][0], orgMv.getVer() + dmh_pos[dmh_mode][0][1]);
	TComMv   tmpMvR(orgMv.getHor() + dmh_pos[dmh_mode][1][0], orgMv.getVer() + dmh_pos[dmh_mode][1][1]);

	pcYuvPred[0].create(g_uiLog2MaxCUSize);
	pcCU->getCUMvField(iRefPic)->setAllMv(tmpMvL, pcCU->getPartitionSize(0), uiPartAddr, uiPartIdx, 0);
	motionCompensation(pcCU, &pcYuvPred[0], iRefPic, uiPartIdx);

	pcYuvPred[1].create(g_uiLog2MaxCUSize);
	pcCU->getCUMvField(iRefPic)->setAllMv(tmpMvR, pcCU->getPartitionSize(0), uiPartAddr, uiPartIdx, 0);
	motionCompensation(pcCU, &pcYuvPred[1], iRefPic, uiPartIdx);

	pcYuvPredFinal->addAvg(&pcYuvPred[0], &pcYuvPred[1], pcCU->getZorderIdxInCU() + uiPartAddr, iWidth, iHeight, pcCU->getZorderIdxInCU() + uiPartAddr);

	pcCU->getCUMvField(iRefPic)->setAllMv(orgMv, pcCU->getPartitionSize(0), uiPartAddr, uiPartIdx, 0);

	pcYuvPred[0].destroy();
	pcYuvPred[1].destroy();
}
#endif

Void TComPrediction::motionCompensation(TComDataCU* pcCU, TComYuv* pcYuvPred, RefPic eRefPic, Int iPartIdx)
{
	Int         iWidth;
	Int         iHeight;
	UInt        uiPartAddr;

	if (iPartIdx >= 0)
	{
		pcCU->getPartIndexAndSize(iPartIdx, uiPartAddr, iWidth, iHeight);

#if F_DHP_SYC
		if (pcCU->getInterDir(uiPartAddr) == INTER_DUAL)
		{
			xPredInterDual(pcCU, uiPartAddr, iWidth, iHeight, pcYuvPred, iPartIdx);
		}
		else
		{
			if (eRefPic != REF_PIC_X)
			{
				xPredInterUni(pcCU, uiPartAddr, iWidth, iHeight, eRefPic, pcYuvPred, iPartIdx);
			}
			else
			{
				xPredInterBi(pcCU, uiPartAddr, iWidth, iHeight, pcYuvPred, iPartIdx);
			}
		}
#else
		if (eRefPic != REF_PIC_X)
		{
			xPredInterUni(pcCU, uiPartAddr, iWidth, iHeight, eRefPic, pcYuvPred, iPartIdx);
		}
		else
		{
			xPredInterBi(pcCU, uiPartAddr, iWidth, iHeight, pcYuvPred, iPartIdx);
		}

#endif
		return;

	}

	for (iPartIdx = 0; iPartIdx < pcCU->getNumPartInter(); iPartIdx++)//For decoder
	{
		pcCU->getPartIndexAndSize(iPartIdx, uiPartAddr, iWidth, iHeight);

#if F_DHP_SYC
		if (pcCU->getInterDir(uiPartAddr) == INTER_DUAL)
		{
			xPredInterDual(pcCU, uiPartAddr, iWidth, iHeight, pcYuvPred, iPartIdx);
		}
		else
		{
			if (eRefPic != REF_PIC_X)
			{
				xPredInterUni(pcCU, uiPartAddr, iWidth, iHeight, eRefPic, pcYuvPred, iPartIdx);
			}
			else
			{
				xPredInterBi(pcCU, uiPartAddr, iWidth, iHeight, pcYuvPred, iPartIdx);
			}
		}
#else
		if (eRefPic != REF_PIC_X)
		{
			xPredInterUni(pcCU, uiPartAddr, iWidth, iHeight, eRefPic, pcYuvPred, iPartIdx);
		}
		else
		{
			xPredInterBi(pcCU, uiPartAddr, iWidth, iHeight, pcYuvPred, iPartIdx);
		}

#endif
	}

	return;
}
#else
Void TComPrediction::motionCompensation ( TComDataCU* pcCU, TComYuv* pcYuvPred, RefPic eRefPic, Int iPartIdx )
{
  Int         iWidth;
  Int         iHeight;
  UInt        uiPartAddr;

  if( iPartIdx >= 0)
  {
    pcCU->getPartIndexAndSize( iPartIdx, uiPartAddr, iWidth, iHeight );
    if ( eRefPic != REF_PIC_X )
    {
      xPredInterUni (pcCU, pcCU->getZorderIdxInCU()+uiPartAddr, iWidth, iHeight, eRefPic, pcYuvPred, iPartIdx );
    }
    else
    {
      xPredInterBi  (pcCU, pcCU->getZorderIdxInCU()+uiPartAddr, iWidth, iHeight, pcYuvPred, iPartIdx );
    }

    return;

  }

  for ( iPartIdx = 0; iPartIdx < pcCU->getNumPartInter(); iPartIdx++ )
  {
    pcCU->getPartIndexAndSize( iPartIdx, uiPartAddr, iWidth, iHeight );

    if ( eRefPic != REF_PIC_X )
    {
      xPredInterUni (pcCU, pcCU->getZorderIdxInCU()+uiPartAddr, iWidth, iHeight, eRefPic, pcYuvPred, iPartIdx );
    }
    else
    {
      xPredInterBi  (pcCU, pcCU->getZorderIdxInCU()+uiPartAddr, iWidth, iHeight, pcYuvPred, iPartIdx );
    }
  }

  return;
}
#endif
#endif

Void TComPrediction::xPredInterUni ( TComDataCU* pcCU, UInt uiPartAddr, Int iWidth, Int iHeight, RefPic eRefPic, TComYuv*& rpcYuvPred, Int iPartIdx )
{


#if 0//INTERBUG
	UInt PartAddr = 0;
	pcCU->getPartIndexAndSize(iPartIdx, PartAddr, iWidth, iHeight);
	TComMv      cMv = pcCU->getCUMvField(eRefPic)->getMv(PartAddr);

	pcCU->clipMv(cMv);
	Int i = pcCU->getCUMvField(eRefPic)->getRefIdx(PartAddr);
	assert(i >= 0);
	xPredInterLumaBlk(pcCU, pcCU->getPicture()->getPicHeader()->getRefPic(eRefPic, pcCU->getCUMvField(eRefPic)->getRefIdx(PartAddr))->getPicYuvRec(), uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred);
	xPredInterChromaBlk(pcCU, pcCU->getPicture()->getPicHeader()->getRefPic(eRefPic, pcCU->getCUMvField(eRefPic)->getRefIdx(PartAddr))->getPicYuvRec(), uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred);
#else

#if compensation_bug
  TComMv      cMv = pcCU->getCUMvField(eRefPic)->getMv(uiPartAddr);
#if rd_sym
  if (pcCU->getInterDir(uiPartAddr) == INTER_SYM)
  {
	  if (eRefPic = REF_PIC_1)
	  {
		  Int deltaP, TRp, DistanceIndexFw, DistanceIndexBw, refframe, deltaPB;
		  if (pcCU->getPicture()->getPictureType() == B_PICTURE)//需要确认是否正确
		  {
			  refframe = 0; //B帧参考帧
			  deltaP = 2 * (pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_1, 0) - pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_0, 0));
			  deltaP = (deltaP + 512) % 512;

			  TRp = (refframe + 1) * deltaP;
			  deltaPB = 2 * (pcCU->getPicture()->getPOC() - pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_0, 0));
			  TRp = (TRp + 512) % 512;
			  deltaPB = (deltaPB + 512) % 512;

			  DistanceIndexFw = deltaPB;
			  DistanceIndexBw = (TRp - DistanceIndexFw + 512) % 512;

		  }
		  else if (pcCU->getPicture()->getPictureType() == F_PICTURE)//需要确认是否正确
		  {
			  DistanceIndexFw = 2 * (pcCU->getPicture()->getPOC() - pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_0, 0));
			  DistanceIndexFw = (DistanceIndexFw + 512) % 512;
			  DistanceIndexBw = 2 * (pcCU->getPicture()->getPOC() - pcCU->getPicture()->getPicHeader()->getRefPOC(REF_PIC_1, 0));
			  DistanceIndexBw = (DistanceIndexBw + 512) % 512;
			  //if (m_pcEncCfg->getBackgroundEnable())
			  if (pcCU->getPicture()->getSPS()->getBackgroundEnableFlag())
			  {
				  DistanceIndexFw = 1;
				  DistanceIndexBw = 1;
			  }
		  }
		  cMv = pcCU->getCUMvField(REF_PIC_0)->getMv(uiPartAddr);
		  if (pcCU->getPicture()->getPictureType() == B_PICTURE)
		  {
			  cMv.setHor(-Clip3(-32768, 32767, (((long long int)(cMv.getHor())* DistanceIndexBw * (MULTI / DistanceIndexFw) + HALF_MULTI) >> OFFSET)));
			  cMv.setVer(-Clip3(-32768, 32767, (((long long int)(cMv.getVer())* DistanceIndexBw * (MULTI / DistanceIndexFw) + HALF_MULTI) >> OFFSET)));
		  }
		  else if (pcCU->getPicture()->getPictureType() == F_PICTURE)
		  {
			  cMv.setHor(Clip3(-32768, 32767, (((long long int)(cMv.getHor())* DistanceIndexBw * (MULTI / DistanceIndexFw) + HALF_MULTI) >> OFFSET)));
			  cMv.setVer(Clip3(-32768, 32767, (((long long int)(cMv.getVer())* DistanceIndexBw * (MULTI / DistanceIndexFw) + HALF_MULTI) >> OFFSET)));
		  }
	  }
  }
#endif
#else
  TComMv      cMv         = pcCU->getCUMvField( eRefPic )->getMv( 0 );
#endif
  //test
  
  pcCU->clipMv(cMv);
#if RPS
#if WLQ_set_bug
  Int i = pcCU->getCUMvField(eRefPic)->getRefIdx(uiPartAddr);
  if(i<0)
	  printf("i=%d,uiPartAddr=%d",i,uiPartAddr);
#else
  Int i = pcCU->getCUMvField(eRefPic)->getRefIdx(0);
#endif
  assert(i >= 0);
#if compensation_bug
#if WLQ_set_bug
  xPredInterLumaBlk(pcCU, pcCU->getPicture()->getPicHeader()->getRefPic(eRefPic, pcCU->getCUMvField(eRefPic)->getRefIdx(uiPartAddr))->getPicYuvRec(), pcCU->getZorderIdxInCU()+uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred);
  xPredInterChromaBlk(pcCU, pcCU->getPicture()->getPicHeader()->getRefPic(eRefPic, pcCU->getCUMvField(eRefPic)->getRefIdx(uiPartAddr))->getPicYuvRec(), pcCU->getZorderIdxInCU()+uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred);
#else
  xPredInterLumaBlk(pcCU, pcCU->getPicture()->getPicHeader()->getRefPic(eRefPic, pcCU->getCUMvField(eRefPic)->getRefIdx(uiPartAddr))->getPicYuvRec(), uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred);
  xPredInterChromaBlk(pcCU, pcCU->getPicture()->getPicHeader()->getRefPic(eRefPic, pcCU->getCUMvField(eRefPic)->getRefIdx(uiPartAddr))->getPicYuvRec(), uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred);
#endif
#else
  xPredInterLumaBlk(pcCU, pcCU->getPicture()->getPicHeader()->getRefPic(eRefPic, pcCU->getCUMvField(eRefPic)->getRefIdx(0))->getPicYuvRec(), uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred);
  xPredInterChromaBlk(pcCU, pcCU->getPicture()->getPicHeader()->getRefPic(eRefPic, pcCU->getCUMvField(eRefPic)->getRefIdx(0))->getPicYuvRec(), uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred);
#endif
#else
  xPredInterLumaBlk       ( pcCU, pcCU->getPicture()->getRefPic( eRefPic )->getPicYuvRec(), uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
  xPredInterChromaBlk(pcCU, pcCU->getPicture()->getRefPic(eRefPic)->getPicYuvRec(), uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred);
#endif
#endif
}

#if F_DHP_SYC
Void TComPrediction::xPredInterDual(TComDataCU* pcCU, UInt uiPartAddr, Int iWidth, Int iHeight, TComYuv*& rpcYuvPred, Int iPartIdx)
{
	assert(pcCU->getInterDir(uiPartAddr) == INTER_DUAL);
	TComMv        cMvDual[2];
	Int           iRefIdxDual[2];
	Int iRefIdxTemp = pcCU->getCUMvField(REF_PIC_0)->getRefIdx(uiPartAddr);
	iRefIdxDual[REF_PIC_0] = iRefIdxTemp;
	iRefIdxDual[REF_PIC_1] = (iRefIdxTemp == 0) ? 1 : 0;
	cMvDual[REF_PIC_0] = pcCU->getCUMvField(REF_PIC_0)->getMv(uiPartAddr);

	pcCU->MV_Scale(pcCU, cMvDual[REF_PIC_1], cMvDual[REF_PIC_0], iRefIdxDual[REF_PIC_0], iRefIdxDual[REF_PIC_1]);

	//TComYuv* pcYuvPred = &m_acYuvPred[REF_PIC_0];
#if !F_DHP_SYC_OP
	TComYuv* m_acYuvPredDual[2];

	m_acYuvPredDual[0] = new TComYuv;
	m_acYuvPredDual[0]->create(g_uiLog2MaxCUSize);
	m_acYuvPredDual[1] = new TComYuv;
	m_acYuvPredDual[1]->create(g_uiLog2MaxCUSize);
#endif

	//  Set motion

	pcCU->getCUMvField(REF_PIC_0)->setAllMv(cMvDual[REF_PIC_1], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
	pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(iRefIdxDual[REF_PIC_1], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
	//motionCompensation(pcCU, &pcYuvPredDual[REF_PIC_1], REF_PIC_0, iPartIdx);
	xPredInterUni(pcCU, uiPartAddr, iWidth, iHeight, REF_PIC_0, m_acYuvPredDual[REF_PIC_1], iPartIdx);

	pcCU->getCUMvField(REF_PIC_0)->setAllMv(cMvDual[REF_PIC_0], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
	pcCU->getCUMvField(REF_PIC_0)->setAllRefIdx(iRefIdxDual[REF_PIC_0], pcCU->getPartitionSize(0), uiPartAddr, iPartIdx, 0);
	//motionCompensation(pcCU, &pcYuvPredDual[REF_PIC_0], REF_PIC_0, iPartIdx);
	xPredInterUni(pcCU, uiPartAddr, iWidth, iHeight, REF_PIC_0, m_acYuvPredDual[REF_PIC_0], iPartIdx);


	rpcYuvPred->addAvg(m_acYuvPredDual[0], m_acYuvPredDual[1], pcCU->getZorderIdxInCU() + uiPartAddr, iWidth, iHeight, pcCU->getZorderIdxInCU() + uiPartAddr);

#if !F_DHP_SYC_OP
	m_acYuvPredDual[0]->destroy();
	delete m_acYuvPredDual[0];
	m_acYuvPredDual[0] = NULL;
	m_acYuvPredDual[1]->destroy();
	delete m_acYuvPredDual[1];
	m_acYuvPredDual[1] = NULL;
#endif

}
#endif



Void TComPrediction::xPredInterBi ( TComDataCU* pcCU, UInt uiPartAddr, Int iWidth, Int iHeight, TComYuv*& rpcYuvPred, Int iPartIdx )
{
  TComYuv* pcMbYuv;
#if INTERBUG
  UInt PartAddr = 0;
  pcCU->getPartIndexAndSize(iPartIdx, PartAddr, iWidth, iHeight);
#endif
  for ( Int iRefPic = 0; iRefPic < 2; iRefPic++ )
  {
    RefPic eRefPic = (iRefPic ? REF_PIC_1 : REF_PIC_0);
    
#if INTERBUG
	if ((pcCU->getInterDir(PartAddr) == 1 && iRefPic == 1) ||
		(pcCU->getInterDir(PartAddr) == 2 && iRefPic == 0))
#else
	if ((pcCU->getInterDir(0) == 1 && iRefPic == 1) ||
		(pcCU->getInterDir(0) == 2 && iRefPic == 0))
#endif
    { 
      continue; 
    }
    
    pcMbYuv = &m_acYuvPred[iRefPic];
    xPredInterUni ( pcCU, uiPartAddr, iWidth, iHeight, eRefPic, pcMbYuv, iPartIdx );
  }
#if WLQ_set_bug 
#if B_RPS_BUG_819
  xAverage(pcCU, &m_acYuvPred[0], &m_acYuvPred[1], pcCU->getZorderIdxInCU() + uiPartAddr, iWidth, iHeight, rpcYuvPred, PartAddr);
#else
  xAverage( pcCU, &m_acYuvPred[0], &m_acYuvPred[1], pcCU->getZorderIdxInCU()+uiPartAddr, iWidth, iHeight, rpcYuvPred );
#endif
#else
  xAverage( pcCU, &m_acYuvPred[0], &m_acYuvPred[1], uiPartAddr, iWidth, iHeight, rpcYuvPred );
#endif
}

#if DCTIF
Void TComPrediction::xPredInterLumaBlk( TComDataCU *cu, TComPicYuv *refPic, UInt partAddr, TComMv *mv, Int width, Int height, TComYuv *&dstPic )
{
	Bool bi = false; //双向预测,从前面传过来
	Int refStride = refPic->getStride();  
	Int refOffset = ( mv->getHor() >> 2 ) + ( mv->getVer() >> 2 ) * refStride;
	Pel *ref      = refPic->getLumaAddr( cu->getAddr(), cu->getZorderIdxInCU() + partAddr ) + refOffset;

	Int dstStride = dstPic->getRealStride();
	Pel *dst      = dstPic->getRealLumaAddr( partAddr );

	Int xFrac = mv->getHor() & 0x3;
	Int yFrac = mv->getVer() & 0x3;

	if ( yFrac == 0 )
	{
		m_if.filterHorLuma( ref, refStride, dst, dstStride, width, height, xFrac,       !bi );
	}
	else if ( xFrac == 0 )
	{
		m_if.filterVerLuma( ref, refStride, dst, dstStride, width, height, yFrac, true, !bi );
	}
	else
	{
		Int tmpStride = m_filteredBlockTmp[0].getRealStride();
		Short *tmp    = m_filteredBlockTmp[0].getLumaAddr();

		Int filterSize = NTAPS_LUMA;
		Int halfFilterSize = ( filterSize >> 1 );

		m_if.filterHorLuma(ref - (halfFilterSize-1)*refStride, refStride, tmp, tmpStride, width, height+filterSize-1, xFrac, false     );
		m_if.filterVerLuma(tmp + (halfFilterSize-1)*tmpStride, tmpStride, dst, dstStride, width, height,              yFrac, false, !bi);    
	}
}


Void TComPrediction::xPredInterChromaBlk( TComDataCU *cu, TComPicYuv *refPic, UInt partAddr, TComMv *mv, Int width, Int height, TComYuv *&dstPic )
{
	Bool bi = false; //双向预测,从前面传过来
	Int     refStride  = refPic->getCStride();
	Int     dstStride  = dstPic->getCStride();

	Int     refOffset  = (mv->getHor() >> 3) + (mv->getVer() >> 3) * refStride;

	Pel*    refCb     = refPic->getCbAddr( cu->getAddr(), cu->getZorderIdxInCU() + partAddr ) + refOffset;
	Pel*    refCr     = refPic->getCrAddr( cu->getAddr(), cu->getZorderIdxInCU() + partAddr ) + refOffset;

	Pel* dstCb = dstPic->getRealCbAddr( partAddr );
	Pel* dstCr = dstPic->getRealCrAddr( partAddr );

	Int     xFrac  = mv->getHor() & 0x7;
	Int     yFrac  = mv->getVer() & 0x7;
	UInt    cxWidth  = width  >> 1;
	UInt    cxHeight = height >> 1;

	Int     extStride = m_filteredBlockTmp[0].getRealStride();
	Short*  extY      = m_filteredBlockTmp[0].getLumaAddr();

	Int filterSize = NTAPS_CHROMA;

	Int halfFilterSize = (filterSize>>1);

	if ( yFrac == 0 )
	{
		m_if.filterHorChroma(refCb, refStride, dstCb,  dstStride, cxWidth, cxHeight, xFrac, !bi);    
		m_if.filterHorChroma(refCr, refStride, dstCr,  dstStride, cxWidth, cxHeight, xFrac, !bi);    
	}
	else if ( xFrac == 0 )
	{
		m_if.filterVerChroma(refCb, refStride, dstCb, dstStride, cxWidth, cxHeight, yFrac, true, !bi);    
		m_if.filterVerChroma(refCr, refStride, dstCr, dstStride, cxWidth, cxHeight, yFrac, true, !bi);    
	}
	else
	{
		m_if.filterHorChroma(refCb - (halfFilterSize-1)*refStride, refStride, extY,  extStride, cxWidth, cxHeight+filterSize-1, xFrac, false);
		m_if.filterVerChroma(extY  + (halfFilterSize-1)*extStride, extStride, dstCb, dstStride, cxWidth, cxHeight  , yFrac, false, !bi);

		m_if.filterHorChroma(refCr - (halfFilterSize-1)*refStride, refStride, extY,  extStride, cxWidth, cxHeight+filterSize-1, xFrac, false);
		m_if.filterVerChroma(extY  + (halfFilterSize-1)*extStride, extStride, dstCr, dstStride, cxWidth, cxHeight  , yFrac, false, !bi);    
	}
}
#else
Void  TComPrediction::xPredInterLumaBlk( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv )
{

  const Int iRefStride = pcPicYuvRef->getStride();
  const Int iDstStride = rpcYuv->getStride();

  const int iRefOffset = (pcMv->getHor() >> 2) + (pcMv->getVer() >> 2) * iRefStride;
  Pel* piRefY = pcPicYuvRef->getLumaAddr( pcCU->getAddr(), uiPartAddr ) + iRefOffset;
  Pel* piDstY = rpcYuv->getLumaAddr( uiPartAddr );

  int iFlip, iSize, iOffset;

  const Int ixFrac = pcMv->getHor() & 0x3;
  const Int iyFrac = pcMv->getVer() & 0x3;

  const int iXEntire = (-ixFrac) >> (sizeof(Int) * 8 - 1);
  const int iYEntire = (-iyFrac) >> (sizeof(Int) * 8 - 1);

  const int iXHek = 2 - ixFrac;
  const int iYHek = 2 - iyFrac;

  const int iXSymm = iXHek >> (sizeof(Int) * 8 - 1);
  const int iYSymm = iYHek >> (sizeof(Int) * 8 - 1);

  int iSizeTemp = iHeight;

  switch ( (PassType) iXEntire + iYEntire )
  {

  case NO_PASS:

    while ( iSizeTemp-- )
    {
      ::memcpy( piDstY, piRefY, sizeof(Pel) * iWidth );
      piDstY += iDstStride;
      piRefY += iRefStride;
    }
    return;

  case ONE_PASS:

    iSize = (iRefStride & iYEntire) | (1 & iXEntire);
    iFlip = (iRefStride & iYSymm) | (1 & iXSymm);

    iSize = iSize - 2 * iFlip;
    iOffset = iFlip - sc_iLumaLeftMargin * iSize;

    sc_afnLumaFilter[0][(ixFrac | iyFrac) & 1]( piRefY + iOffset, piDstY, iRefStride,
        iDstStride, iSize, iWidth, iHeight );

    return;

  case TWO_PASSES:

    iFlip = iRefStride & iYSymm;
    iSize = iRefStride - 2 * iFlip;
    iOffset = sc_iLumaHalfTaps - iFlip + sc_iLumaLeftMargin * iSize;

    sc_afnLumaFilter[1][iyFrac & 1]( piRefY - iOffset, m_piYuvExt, iRefStride,
        m_iYuvExtStride, iSize, iWidth, iHeight );

    iFlip = 1 & iXSymm;
    iSize = 1 - 2 * iFlip;
    iOffset = sc_iLumaHalfTaps + iFlip - sc_iLumaLeftMargin * iSize;

    sc_afnLumaFilter[2][ixFrac & 1]( m_piYuvExt + iOffset, piDstY, m_iYuvExtStride,
        iDstStride, iSize, iWidth, iHeight );

    return;

  }
}
//--

//kolya
#if chazhi_bug
Void TComPrediction::xPredInterChromaBlk(TComDataCU *cu, TComPicYuv *refPic, UInt partAddr, TComMv *mv, Int width, Int height, TComYuv *&dstPic)
{
	Bool bi = false; //双向预测,从前面传过来
	Int     refStride  = refPic->getCStride();
	Int     dstStride  = dstPic->getCStride();

	Int     refOffset  = (mv->getHor() >> 3) + (mv->getVer() >> 3) * refStride;

#if niu_PredInterChroma_Bug
	Pel*    refCb = refPic->getCbAddr(cu->getAddr(), partAddr) + refOffset;
	Pel*    refCr = refPic->getCrAddr(cu->getAddr(), partAddr) + refOffset;
#else
	Pel*    refCb = refPic->getCbAddr(cu->getAddr(), cu->getZorderIdxInCU() + partAddr) + refOffset;
	Pel*    refCr = refPic->getCrAddr(cu->getAddr(), cu->getZorderIdxInCU() + partAddr) + refOffset;
#endif

	Pel* dstCb = dstPic->getCbAddr( partAddr );
	Pel* dstCr = dstPic->getCrAddr( partAddr );

	Int     xFrac  = mv->getHor() & 0x7;
	Int     yFrac  = mv->getVer() & 0x7;
	UInt    cxWidth  = width  >> 1;
	UInt    cxHeight = height >> 1;

	Int     extStride = m_filteredBlockTmp[0].getStride();
	Short*  extY      = m_filteredBlockTmp[0].getLumaAddr();

	Int filterSize = NTAPS_CHROMA;

	Int halfFilterSize = (filterSize>>1);

	if ( yFrac == 0 )
	{
		m_if.filterHorChroma(refCb, refStride, dstCb,  dstStride, cxWidth, cxHeight, xFrac, !bi);    
		m_if.filterHorChroma(refCr, refStride, dstCr,  dstStride, cxWidth, cxHeight, xFrac, !bi);    
	}
	else if ( xFrac == 0 )
	{
		m_if.filterVerChroma(refCb, refStride, dstCb, dstStride, cxWidth, cxHeight, yFrac, true, !bi);    
		m_if.filterVerChroma(refCr, refStride, dstCr, dstStride, cxWidth, cxHeight, yFrac, true, !bi);    
	}
	else
	{
		m_if.filterHorChroma(refCb - (halfFilterSize-1)*refStride, refStride, extY,  extStride, cxWidth, cxHeight+filterSize-1, xFrac, false);
		m_if.filterVerChroma(extY  + (halfFilterSize-1)*extStride, extStride, dstCb, dstStride, cxWidth, cxHeight  , yFrac, false, !bi);

		m_if.filterHorChroma(refCr - (halfFilterSize-1)*refStride, refStride, extY,  extStride, cxWidth, cxHeight+filterSize-1, xFrac, false);
		m_if.filterVerChroma(extY  + (halfFilterSize-1)*extStride, extStride, dstCr, dstStride, cxWidth, cxHeight  , yFrac, false, !bi);    
	}
}
#else
Void TComPrediction::xPredInterChromaBlk( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv )
{
  const Int iRefStride = pcPicYuvRef->getCStride();
  const Int iDstStride = rpcYuv->getCStride();

  const Int iRefOffset = (pcMv->getHor() >> 3) + (pcMv->getVer() >> 3) * iRefStride;

  Pel* piRefCb = pcPicYuvRef->getCbAddr( pcCU->getAddr(), uiPartAddr ) + iRefOffset;
  Pel* piRefCr = pcPicYuvRef->getCrAddr( pcCU->getAddr(), uiPartAddr ) + iRefOffset;

  Pel* piDstCb = rpcYuv->getCbAddr( uiPartAddr );
  Pel* piDstCr = rpcYuv->getCrAddr( uiPartAddr );

  Int iFlip, iSize, iOffset;

  const Int ixFrac = pcMv->getHor() & 0x7;
  const Int iyFrac = pcMv->getVer() & 0x7;

  const Int iCWidth = iWidth >> 1;
  const Int iCHeight = iHeight >> 1;

  const int iXEntire = (-ixFrac) >> (sizeof(Int) * 8 - 1);
  const int iYEntire = (-iyFrac) >> (sizeof(Int) * 8 - 1);

  const int iXHek = 4 - ixFrac;
  const int iYHek = 4 - iyFrac;

  const int iXSymm = iXHek >> (sizeof(Int) * 8 - 1);
  const int iYSymm = iYHek >> (sizeof(Int) * 8 - 1);

  const int iXGo = ((iXHek + iXSymm) ^ iXSymm);
  const int iYGo = ((iYHek + iYSymm) ^ iYSymm);

  int iDepthIdx = -1;
  int iCSizeTemp = Max(iCHeight, iCWidth);

  while ( iCSizeTemp >>= 1 )
    iDepthIdx++;
  iCSizeTemp = iCHeight;

  switch ( (PassType) iXEntire + iYEntire )
  {

  case NO_PASS:

    while ( iCSizeTemp-- )
    {
      memcpy( (void*) piDstCb, piRefCb, sizeof(Pel) * iCWidth );
      memcpy( (void*) piDstCr, piRefCr, sizeof(Pel) * iCWidth );

      piDstCb += iDstStride;
      piDstCr += iDstStride;
      piRefCb += iRefStride;
      piRefCr += iRefStride;
    }
    return;

  case ONE_PASS:

    iSize = (iRefStride & iYEntire) | (1 & iXEntire);
    iFlip = (iRefStride & iYSymm) | (1 & iXSymm);

    iSize = iSize - 2 * iFlip;
    iOffset = iFlip - sc_iChromaLeftMargin * iSize;

    sc_afnChromaFilter[0][(iXGo | iYGo) & 3][iDepthIdx]( piRefCb + iOffset, piRefCr + iOffset,
        piDstCb, piDstCr, iRefStride, iDstStride, iSize, iCWidth, iCHeight );

    return;

  case TWO_PASSES:

    iFlip = iRefStride & iYSymm;
    iSize = iRefStride - 2 * iFlip;
    iOffset = sc_iChromaHalfTaps - iFlip + sc_iChromaLeftMargin * iSize;

    sc_afnChromaFilter[1][iYGo][iDepthIdx]( piRefCb - iOffset, piRefCr - iOffset, m_piYuvExt,
        m_piYuvExt2, iRefStride, m_iYuvExtStride, iSize, iCWidth, iCHeight );

    iFlip = 1 & iXSymm;
    iSize = 1 - 2 * iFlip;
    iOffset = sc_iChromaHalfTaps + iFlip - sc_iChromaLeftMargin * iSize;

    sc_afnChromaFilter[2][iXGo][iDepthIdx]( m_piYuvExt + iOffset, m_piYuvExt2 + iOffset, piDstCb,
        piDstCr, m_iYuvExtStride, iDstStride, iSize, iCWidth, iCHeight );

    return;

  }
}
#endif
#endif

#if B_RPS_BUG_819
Void TComPrediction::xAverage(TComDataCU* pcCU, TComYuv* pcYuvSrc0, TComYuv* pcYuvSrc1, UInt uiPartIdx, Int iWidth, Int iHeight, TComYuv*& rpcYuvDst, Int PartAddr)
#else
Void TComPrediction::xAverage( TComDataCU* pcCU, TComYuv* pcYuvSrc0, TComYuv* pcYuvSrc1, UInt uiPartIdx, Int iWidth, Int iHeight, TComYuv*& rpcYuvDst )
#endif
{
#if B_RPS_BUG_819
	switch (pcCU->getInterDir(PartAddr))
#else
  switch( pcCU->getInterDir(0) )
#endif
  {
#if F_DHP_SYC_NSQT
  case 0://意味着地一个块是DHP模式,目前不考虑NxN，所以只存在第二个PU块是前向而进入该函数的情况
#endif
  case 1: pcYuvSrc0->copyPartToPartYuv( rpcYuvDst, uiPartIdx, uiPartIdx, iWidth, iHeight ); break;
  case 2: pcYuvSrc1->copyPartToPartYuv( rpcYuvDst, uiPartIdx, uiPartIdx, iWidth, iHeight ); break;
  case 3: rpcYuvDst->addAvg( pcYuvSrc0, pcYuvSrc1, uiPartIdx, iWidth, iHeight, uiPartIdx ); break;
#if rd_mvd
  case 4: rpcYuvDst->addAvg(pcYuvSrc0, pcYuvSrc1, uiPartIdx, iWidth, iHeight, uiPartIdx); break;
#endif
  default: assert(0); break;
  }
}

