/**
 * \file
 * \brief Declaration of TComInterpolationFilter class
 */

#ifndef __HM_TCOMINTERPOLATIONFILTER_H__
#define __HM_TCOMINTERPOLATIONFILTER_H__

#include "TypeDef.h"

//! \ingroup TLibCommon
//! \{

#define NTAPS_LUMA        8 ///< Number of taps for luma
#define NTAPS_CHROMA      4 ///< Number of taps for chroma
#define IF_INTERNAL_PREC 14 ///< Number of bits for internal precision
#define IF_FILTER_PREC    6 ///< Log2 of sum of filter taps
#define IF_INTERNAL_OFFS (1<<(IF_INTERNAL_PREC-1)) ///< Offset used internally

/**
 * \brief Interpolation filter class
 */
class TComInterpolationFilter
{
  static const Short m_lumaFilter[4][NTAPS_LUMA];     ///< Luma filter taps
  static const Short m_chromaFilter[8][NTAPS_CHROMA]; ///< Chroma filter taps
  
  static Void filterCopy(Int bitDepth, const Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Bool isFirst, Bool isLast);
  
  template<Int N, Bool isVertical, Bool isFirst, Bool isLast>
  static Void filter(Int bitDepth, Pel const *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Short const *coeff);

  template<Int N>
  static Void filterHor(Int bitDepth, Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height,               Bool isLast, Short const *coeff);
  template<Int N>
  static Void filterVer(Int bitDepth, Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Bool isFirst, Bool isLast, Short const *coeff);

public:
  TComInterpolationFilter() {}
  ~TComInterpolationFilter() {}

  Void filterHorLuma  (Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Int frac,               Bool isLast );
  Void filterVerLuma  (Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Int frac, Bool isFirst, Bool isLast );
  Void filterHorChroma(Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Int frac,               Bool isLast );
  Void filterVerChroma(Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Int frac, Bool isFirst, Bool isLast );
};

//! \}

#endif
