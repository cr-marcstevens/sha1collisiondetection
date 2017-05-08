/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow (danshu@microsoft.com)
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/

/*
 * this header defines SIMD MACROS for avx512 intrinsics
 * used to generate avx512 code from generic SIMD code (sha1_simd.cinc, ubc_check_simd.cinc)
 */

#ifndef SIMD_AVX512_HEADER
#define SIMD_AVX512_HEADER
#ifdef SHA1DC_HAVE_AVX512
/* requires AVX512 not just AVX */
#define SIMD_VERSION	avx512
#define SIMD_VECSIZE	16

#ifdef __GNUC__

/* TODO */
#include <immintrin.h>

#define SIMD_WORD			__m512i

#define SIMD_ZERO			_mm512_setzero_si512()
#define SIMD_WTOV(l)		_mm512_set1_epi32(l)
#define SIMD_ADD_VV(l,r)	_mm512_add_epi32(l,r)
#define SIMD_ADD_VW(l,r)	_mm512_add_epi32(l, _mm512_set1_epi32(r))
#define SIMD_SUB_VV(l,r)	_mm512_sub_epi32(l,r)
#define SIMD_SUB_VW(l,r)	_mm512_sub_epi32(l, _mm512_set1_epi32(r))
#define SIMD_AND_VV(l,r)	_mm512_and_si512(l,r)
#define SIMD_AND_VW(l,r)	_mm512_and_si512(l, _mm512_set1_epi32(r))
#define SIMD_OR_VV(l,r)		_mm512_or_si512(l,r)
#define SIMD_OR_VW(l,r)		_mm512_or_si512(l, _mm512_set1_epi32(r))
#define SIMD_XOR_VV(l,r)	_mm512_xor_si512(l,r)
#define SIMD_XOR_VW(l,r)	_mm512_xor_si512(l, _mm512_set1_epi32(r))
/*#define SIMD_NOT_V(l)		_mm512_andnot_si512(l,l)*/
#define SIMD_SHL_V(l,i)		_mm512_slli_epi32(l,i)
#define SIMD_SHR_V(l,i)		_mm512_srli_epi32(l,i)
/*#define SIMD_ROL_V(l,i)		_mm512_rol_epi32(l,i)*/
/*#define SIMD_ROR_V(l,i)		_mm512_ror_epi32(l,i)*/
#define SIMD_CLEANUP

#else /* __GNUC__ */

/* VISUAL STUDIO */

#include <immintrin.h>

#define SIMD_WORD			__m512i

#define SIMD_ZERO			_mm512_setzero_si512()
#define SIMD_WTOV(l)		_mm512_set1_epi32(l)
#define SIMD_ADD_VV(l,r)	_mm512_add_epi32(l,r)
#define SIMD_ADD_VW(l,r)	_mm512_add_epi32(l, _mm512_set1_epi32(r))
#define SIMD_SUB_VV(l,r)	_mm512_sub_epi32(l,r)
#define SIMD_SUB_VW(l,r)	_mm512_sub_epi32(l, _mm512_set1_epi32(r))
#define SIMD_AND_VV(l,r)	_mm512_and_si512(l,r)
#define SIMD_AND_VW(l,r)	_mm512_and_si512(l, _mm512_set1_epi32(r))
#define SIMD_OR_VV(l,r)		_mm512_or_si512(l,r)
#define SIMD_OR_VW(l,r)		_mm512_or_si512(l, _mm512_set1_epi32(r))
#define SIMD_XOR_VV(l,r)	_mm512_xor_si512(l,r)
#define SIMD_XOR_VW(l,r)	_mm512_xor_si512(l, _mm512_set1_epi32(r))
/*#define SIMD_NOT_V(l)		_mm512_andnot_si512(l,l)*/
#define SIMD_SHL_V(l,i)		_mm512_slli_epi32(l,i)
#define SIMD_SHR_V(l,i)		_mm512_srli_epi32(l,i)
/*#define SIMD_ROL_V(l,i)		_mm512_rol_epi32(l,i)*/
/*#define SIMD_ROR_V(l,i)		_mm512_ror_epi32(l,i)*/
#define SIMD_CLEANUP

#endif /* __GNUC__ */

/* these are general definitions for lacking SIMD operations */

#ifndef SIMD_NOT_V
#define SIMD_NOT_V(l)		SIMD_XOR_VW(l,-1)
#endif

#ifndef SIMD_NEG_V
#define SIMD_NEG_V(l)		SIMD_SUB_VV(SIMD_ZERO,l)
#endif

#ifndef SIMD_ROL_V
#define SIMD_ROL_V(l,i)		_mm512_rol_epi32(l, i) /*SIMD_OR_VV(SIMD_SHL_V(l,i),SIMD_SHR_V(l,32-i))*/
#define SIMD_ROR_V(l,i)		_mm512_ror_epi32(l, i) /*SIMD_OR_VV(SIMD_SHR_V(l,i),SIMD_SHL_V(l,32-i))*/
#endif
#endif /* SHA1DC_HAVE_AVX512 */
#endif /* SIMD_AVX512_HEADER */
