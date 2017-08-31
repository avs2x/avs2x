/*
 * TComPredFilterChroma.h
 *
 *  Created on: Jan 28, 2011
 *      Author: //kolya
 */

/** \file     TComPredFilterChroma.h
 \brief    Chroma interpolation filter (header)
 */

#ifndef TCOMPREDFILTERCHROMA_H_
#define TCOMPREDFILTERCHROMA_H_

#include "CommonDef.h"
#include "TypeDef.h"

//bitness, internal only
#define C2DVER 0
#define S2DVER 0
#define C2DHOR 2048
#define S2DHOR 12
#define C1D    32
#define S1D    6

// kernels
#define FIR1_CHROMA iSum = (iP0 - iP3) - ((iP0 + iP1) << 2) + (iP2 << 3) + (iP1 << 6);
#define FIR2_CHROMA iSum = (iP1 << 6) + (iP2 << 4) - (iP1 << 3) - (iP0 << 2) - ((iP1 + iP3) << 1);
#define FIR3_CHROMA iTmp0 = iP0 + iP2; \
             iSum = ((iP1 + iP2) << 5) + (iP1 << 4) - ((iTmp0 + iP3) << 2) - (iP1 << 1) - iTmp0;
#define FIR4_CHROMA iTmp0 = iP0 + iP3; \
             iTmp1 = iP1 + iP2; \
             iSum = (iTmp1 << 5) + ((iTmp1 - iTmp0) << 2);

//arguments
#define TAIL const Int iSrcStride, \
             const Int iDstStride, \
             const Int iSize, \
             const Int iWidth, \
             const Int iHeight

#define ARGS_CHROMA  void* piSrcCb, void* piSrcCr, void* piDstCb, void* piDstCr, TAIL

//to be brief
#define READ_PIXELS(pSrc)  iP0 = *(pSrc); \
                           iP1 = *(pSrc + iSize); \
                           iP2 = *(pSrc + 2 * iSize); \
                           iP3 = *(pSrc + 3 * iSize);

#define LOOPANY_CHROMA(xKernel, ...)    Int iSum = 0; \
  IN* piSrcCb1 = (IN*)piSrcCb; \
  IN* piSrcCr1 = (IN*)piSrcCr; \
  OUT* piDstCb1 = (OUT*)piDstCb; \
  OUT* piDstCr1 = (OUT*)piDstCr; \
  Int iP0, iP1, iP2, iP3, ##__VA_ARGS__; \
  for (Int y = 0; y < iHeight; y++) \
    { \
      IN* piSrcTmpCb = piSrcCb1; \
      IN* piSrcTmpCr = piSrcCr1; \
      for (Int x = 0; x < iWidth + (sc_iChromaTaps & (LEFT_LIMIT >> 31)); x++) \
        { \
          READ_PIXELS(piSrcTmpCb); \
          xKernel; \
          piDstCb1[x] = Clip3(LEFT_LIMIT, RIGHT_LIMIT, (iSum + ROUNDING_CONST) >> ROUNDING_SHIFT); \
          READ_PIXELS(piSrcTmpCr); \
          xKernel; \
          piDstCr1[x] = Clip3(LEFT_LIMIT, RIGHT_LIMIT, (iSum + ROUNDING_CONST) >> ROUNDING_SHIFT); \
          piSrcTmpCb++; \
          piSrcTmpCr++; \
        } \
      piSrcCb1 += iSrcStride; \
      piSrcCr1 += iSrcStride; \
      piDstCb1 += iDstStride; \
      piDstCr1 += iDstStride; \
    } \
  return;

typedef Void (*fnChromaFilter)   ( ARGS_CHROMA );

class TComPredFilterChroma
{
protected:

  static const int sc_iChromaLeftMargin, sc_iChromaTaps, sc_iChromaHalfTaps;

  static const fnChromaFilter sc_afnChromaFilter[3][4][5];

  template< class IN, class OUT, int ROUNDING_CONST, int ROUNDING_SHIFT, int LEFT_LIMIT, int RIGHT_LIMIT, int BLOCK_SIZE >
  static inline Void chromaFilter1( ARGS_CHROMA )
  {
    LOOPANY_CHROMA(FIR1_CHROMA);
  }

  template< class IN, class OUT, int ROUNDING_CONST, int ROUNDING_SHIFT, int LEFT_LIMIT, int RIGHT_LIMIT, int BLOCK_SIZE  >
  static inline Void chromaFilter2( ARGS_CHROMA )
  {
    LOOPANY_CHROMA(FIR2_CHROMA);
  }

  template< class IN, class OUT, int ROUNDING_CONST, int ROUNDING_SHIFT, int LEFT_LIMIT, int RIGHT_LIMIT, int BLOCK_SIZE  >
  static inline Void chromaFilter3( ARGS_CHROMA )
  {
    LOOPANY_CHROMA(FIR3_CHROMA, iTmp0);
  }

  template< class IN, class OUT, int ROUNDING_CONST, int ROUNDING_SHIFT, int LEFT_LIMIT, int RIGHT_LIMIT, int BLOCK_SIZE  >
  static inline Void chromaFilter4( ARGS_CHROMA )
  {
    LOOPANY_CHROMA(FIR4_CHROMA, iTmp0, iTmp1);
  }

};
#endif /* TCOMPREDFILTERCHROMA_H_ */
