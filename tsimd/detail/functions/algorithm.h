// ========================================================================== //
// The MIT License (MIT)                                                      //
//                                                                            //
// Copyright (c) 2017 Jefferson Amstutz                                       //
//                                                                            //
// Permission is hereby granted, free of charge, to any person obtaining a    //
// copy of this software and associated documentation files (the "Software"), //
// to deal in the Software without restriction, including without limitation  //
// the rights to use, copy, modify, merge, publish, distribute, sublicense,   //
// and/or sell copies of the Software, and to permit persons to whom the      //
// Software is furnished to do so, subject to the following conditions:       //
//                                                                            //
// The above copyright notice and this permission notice shall be included in //
// in all copies or substantial portions of the Software.                     //
//                                                                            //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    //
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    //
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        //
// DEALINGS IN THE SOFTWARE.                                                  //
// ========================================================================== //

#pragma once

#include "../pack.h"

namespace tsimd {

  template <typename T, int W, typename FCN_T>
  TSIMD_INLINE void foreach(pack<T, W> &p, FCN_T && fcn)
  {
    for (int i = 0; i < W; ++i)
      fcn(p[i], i);
  }

  template <
      typename BOOL_T,
      int W,
      typename FCN_T,
      typename = traits::is_bool_t<BOOL_T>>
  TSIMD_INLINE void foreach_active(const pack<BOOL_T, W> &m, FCN_T &&fcn)
  {
    for (int i = 0; i < W; ++i)
      if (m[i])
        fcn(i);
  }

  template <typename T, int W, typename FCN_T>
  TSIMD_INLINE void foreach_active(const mask<T, W>  &m,
                                         pack<T, W>  &p,
                                         FCN_T      &&fcn)
  {
    for (int i = 0; i < W; ++i)
      if (m[i])
        fcn(p[i]);
  }

  // any() ////////////////////////////////////////////////////////////////////

  // 1-wide //

  template <typename T, typename = traits::is_bool_t<T>>
  TSIMD_INLINE bool any(const pack<T, 1> &a)
  {
    return a[0];
  }

  // 4-wide //

  TSIMD_INLINE bool any(const vboolf4 &a)
  {
#if defined(__SSE__)
    return _mm_movemask_ps(a) != 0x0;
#else
    for (int i = 0; i < 4; ++i) {
      if (a[i])
        return true;
    }

    return false;
#endif
  }

  // 8-wide //

  TSIMD_INLINE bool any(const vboolf8 &a)
  {
#if defined(__AVX512F__) || defined(__AVX2__) || defined(__AVX__)
    return !_mm256_testz_ps(a, a);
#else
    for (int i = 0; i < 8; ++i) {
      if (a[i])
        return true;
    }

    return false;
#endif
  }

  // 16-wide //

  TSIMD_INLINE bool any(const vboolf16 &a)
  {
#if defined(__AVX512F__)
    return _mm512_kortestz(a, a) == 0;
#else
    for (int i = 0; i < 16; ++i) {
      if (a[i])
        return true;
    }

    return false;
#endif
  }

  // none() ///////////////////////////////////////////////////////////////////

  template <typename MASK_T, typename = traits::is_mask_t<MASK_T>>
  TSIMD_INLINE bool none(const MASK_T &m)
  {
    return !any(m);
  }

  // all() ////////////////////////////////////////////////////////////////////

  // 1-wide //

  TSIMD_INLINE bool all(const vboolf1 &a)
  {
    return any(a);
  }

  TSIMD_INLINE bool all(const vboold1 &a)
  {
    return any(a);
  }

  // 4-wide //

  TSIMD_INLINE bool all(const vboolf4 &a)
  {
#if defined(__SSE__)
    return _mm_movemask_ps(a) == 0xf;
#else
    for (int i = 0; i < 4; ++i) {
      if (!a[i])
        return false;
    }

    return true;
#endif
  }

  // 8-wide //

  TSIMD_INLINE bool all(const vboolf8 &a)
  {
#if defined(__AVX512F__) || defined(__AVX2__) || defined(__AVX__)
    return _mm256_movemask_ps(a) == (unsigned int)0xff;
#else
    for (int i = 0; i < 8; ++i) {
      if (!a[i])
        return false;
    }

    return true;
#endif
  }

  // 16-wide //

  TSIMD_INLINE bool all(const vboolf16 &a)
  {
#if defined(__AVX512F__)
    return _mm512_kortestc(a, a) != 0;
#else
    for (int i = 0; i < 16; ++i) {
      if (!a[i])
        return false;
    }

    return true;
#endif
  }

  // select() /////////////////////////////////////////////////////////////////

  // 1-wide //

  template <typename T>
  TSIMD_INLINE pack<T, 1> select(const pack<bool_t<T>, 1> &m,
                                 const pack<T, 1> &t,
                                 const pack<T, 1> &f)
  {
    return pack<T, 1>(m[0] ? t[0] : f[0]);
  }

  // 4-wide //

  TSIMD_INLINE vfloat4 select(const vboolf4 &m,
                              const vfloat4 &t,
                              const vfloat4 &f)
  {
#if defined(__SSE4_1__)
      return _mm_blendv_ps(f, t, m);
#elif defined(__SSE__)
      return _mm_or_ps(_mm_and_ps(m, t), _mm_andnot_ps(m, f));
#else
    vfloat4 result;

    for (int i = 0; i < 4; ++i)
      result[i] = m[i] ? t[i] : f[i];

    return result;
#endif
  }

  TSIMD_INLINE vint4 select(const vboolf4 &m, const vint4 &t, const vint4 &f)
  {
#if defined(__SSE4_1__)
      return _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(f),
                                            _mm_castsi128_ps(t), m));
#elif defined(__SSE__)
      return _mm_or_si128(_mm_and_si128(m, t), _mm_andnot_si128(m, f));
#else
    vint4 result;

    for (int i = 0; i < 4; ++i)
      result[i] = m[i] ? t[i] : f[i];

    return result;
#endif
  }

  // 8-wide //

  TSIMD_INLINE vfloat8 select(const vboolf8 &m,
                              const vfloat8 &t,
                              const vfloat8 &f)
  {
#if defined(__AVX512F__) || defined(__AVX2__) || defined(__AVX__)
    return _mm256_blendv_ps(f, t, m);
#else
    vfloat8 result;

    for (int i = 0; i < 8; ++i)
      result[i] = m[i] ? t[i] : f[i];

    return result;
#endif
  }

  TSIMD_INLINE vint8 select(const vboolf8 &m, const vint8 &t, const vint8 &f)
  {
#if defined(__AVX512F__) || defined(__AVX2__) || defined(__AVX__)
    return _mm256_castps_si256(
        _mm256_blendv_ps(_mm256_castsi256_ps(f), _mm256_castsi256_ps(t), m));
#else
    vint8 result;

    for (int i = 0; i < 8; ++i)
      result[i] = m[i] ? t[i] : f[i];

    return result;
#endif
  }

  // 16-wide //

  TSIMD_INLINE vfloat16 select(const vboolf16 &m,
                               const vfloat16 &t,
                               const vfloat16 &f)
  {
#if defined(__AVX512F__)
    return _mm512_mask_blend_ps(m, f, t);
#else
    vfloat16 result;

    for (int i = 0; i < 16; ++i)
      result[i] = m[i] ? t[i] : f[i];

    return result;
#endif
  }

  TSIMD_INLINE vint16 select(const vboolf16 &m,
                             const vint16 &t,
                             const vint16 &f)
  {
#if defined(__AVX512F__)
    return _mm512_mask_or_epi32(f, m, t, t);
#else
    vint16 result;

    for (int i = 0; i < 16; ++i)
      result[i] = m[i] ? t[i] : f[i];

    return result;
#endif
  }

}  // namespace tsimd