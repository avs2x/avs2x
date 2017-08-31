/*
 * TComPredFilterChroma.cpp
 *
 *  Created on: Jan 28, 2011
 *      Author: //kolya
 */

#include <limits.h>
#include "TComPredFilterChroma.h"

//tapsoidness, used in outer call
const int TComPredFilterChroma::sc_iChromaLeftMargin = 1;
const int TComPredFilterChroma::sc_iChromaTaps = 4;
const int TComPredFilterChroma::sc_iChromaHalfTaps = 2;

//function pointers
const fnChromaFilter
TComPredFilterChroma::sc_afnChromaFilter[3][4][5] =
{
    {
        {
            chromaFilter4< Pel, Pel, C1D, S1D, 0, 255, 2  >,
            chromaFilter4< Pel, Pel, C1D, S1D, 0, 255, 4  >,
            chromaFilter4< Pel, Pel, C1D, S1D, 0, 255, 8  >,
            chromaFilter4< Pel, Pel, C1D, S1D, 0, 255, 16 >,
            chromaFilter4< Pel, Pel, C1D, S1D, 0, 255, 32 >
        },
        {
            chromaFilter3<Pel, Pel, C1D, S1D, 0, 255, 2  >,
            chromaFilter3<Pel, Pel, C1D, S1D, 0, 255, 4  >,
            chromaFilter3<Pel, Pel, C1D, S1D, 0, 255, 8  >,
            chromaFilter3<Pel, Pel, C1D, S1D, 0, 255, 16 >,
            chromaFilter3<Pel, Pel, C1D, S1D, 0, 255, 32 >
        },
        {
            chromaFilter2<Pel, Pel, C1D, S1D, 0, 255, 2  >,
            chromaFilter2<Pel, Pel, C1D, S1D, 0, 255, 4  >,
            chromaFilter2<Pel, Pel, C1D, S1D, 0, 255, 8  >,
            chromaFilter2<Pel, Pel, C1D, S1D, 0, 255, 16 >,
            chromaFilter2<Pel, Pel, C1D, S1D, 0, 255, 32 >
        },
        {
            chromaFilter1<Pel, Pel, C1D, S1D, 0, 255, 2  >,
            chromaFilter1<Pel, Pel, C1D, S1D, 0, 255, 4  >,
            chromaFilter1<Pel, Pel, C1D, S1D, 0, 255, 8  >,
            chromaFilter1<Pel, Pel, C1D, S1D, 0, 255, 16 >,
            chromaFilter1<Pel, Pel, C1D, S1D, 0, 255, 32 >
        }
    },
    {
        {
            chromaFilter4<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 2>,
            chromaFilter4<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 4>,
            chromaFilter4<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 8>,
            chromaFilter4<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 16>,
            chromaFilter4<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 32>
        },
        {
            chromaFilter3<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 2>,
            chromaFilter3<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 4>,
            chromaFilter3<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 8>,
            chromaFilter3<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 16>,
            chromaFilter3<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 32>
        },
        {
            chromaFilter2<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 2>,
            chromaFilter2<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 4>,
            chromaFilter2<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 8>,
            chromaFilter2<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 16>,
            chromaFilter2<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 32>
        },
        {
            chromaFilter1<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 2>,
            chromaFilter1<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 4>,
            chromaFilter1<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 8>,
            chromaFilter1<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 16>,
            chromaFilter1<Pel, Int, C2DVER, S2DVER, INT_MIN, INT_MAX, 32>
        }
    },
    {
        {
            chromaFilter4<Int, Pel, C2DHOR, S2DHOR, 0, 255, 2>,
            chromaFilter4<Int, Pel, C2DHOR, S2DHOR, 0, 255, 4>,
            chromaFilter4<Int, Pel, C2DHOR, S2DHOR, 0, 255, 8>,
            chromaFilter4<Int, Pel, C2DHOR, S2DHOR, 0, 255, 16>,
            chromaFilter4<Int, Pel, C2DHOR, S2DHOR, 0, 255, 32>,
        },
        {
            chromaFilter3<Int, Pel, C2DHOR, S2DHOR, 0, 255, 2>,
            chromaFilter3<Int, Pel, C2DHOR, S2DHOR, 0, 255, 4>,
            chromaFilter3<Int, Pel, C2DHOR, S2DHOR, 0, 255, 8>,
            chromaFilter3<Int, Pel, C2DHOR, S2DHOR, 0, 255, 16>,
            chromaFilter3<Int, Pel, C2DHOR, S2DHOR, 0, 255, 32>,
        },
        {
            chromaFilter2<Int, Pel, C2DHOR, S2DHOR, 0, 255, 2>,
            chromaFilter2<Int, Pel, C2DHOR, S2DHOR, 0, 255, 4>,
            chromaFilter2<Int, Pel, C2DHOR, S2DHOR, 0, 255, 8>,
            chromaFilter2<Int, Pel, C2DHOR, S2DHOR, 0, 255, 16>,
            chromaFilter2<Int, Pel, C2DHOR, S2DHOR, 0, 255, 32>,
        },
        {
            chromaFilter1<Int, Pel, C2DHOR, S2DHOR, 0, 255, 2>,
            chromaFilter1<Int, Pel, C2DHOR, S2DHOR, 0, 255, 4>,
            chromaFilter1<Int, Pel, C2DHOR, S2DHOR, 0, 255, 8>,
            chromaFilter1<Int, Pel, C2DHOR, S2DHOR, 0, 255, 16>,
            chromaFilter1<Int, Pel, C2DHOR, S2DHOR, 0, 255, 32>,
        }
    }
};
