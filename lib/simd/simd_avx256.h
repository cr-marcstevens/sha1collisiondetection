/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow (danshu@microsoft.com)
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/

/*
 * this header defines SIMD MACROS for avx256 intrinsics
 * used to generate avx256 code from generic SIMD code (sha1_simd.cinc, ubc_check_simd.cinc)
 */

#ifndef SIMD_AVX256_HEADER
#define SIMD_AVX256_HEADER
#ifdef SHA1DC_HAVE_AVX256
/* requires AVX2 not just AVX */
#define SIMD_VERSION	avx256
#define SIMD_VECSIZE	8

#ifdef __GNUC__

/* TODO */
#include <immintrin.h>

#define SIMD_WORD			__m256i

#define SIMD_ZERO			_mm256_setzero_si256()
#define SIMD_WTOV(l)		_mm256_set1_epi32(l)
#define SIMD_ADD_VV(l,r)	_mm256_add_epi32(l,r)
#define SIMD_ADD_VW(l,r)	_mm256_add_epi32(l, _mm256_set1_epi32(r))
#define SIMD_SUB_VV(l,r)	_mm256_sub_epi32(l,r)
#define SIMD_SUB_VW(l,r)	_mm256_sub_epi32(l, _mm256_set1_epi32(r))
#define SIMD_AND_VV(l,r)	_mm256_and_si256(l,r)
#define SIMD_AND_VW(l,r)	_mm256_and_si256(l, _mm256_set1_epi32(r))
#define SIMD_OR_VV(l,r)		_mm256_or_si256(l,r)
#define SIMD_OR_VW(l,r)		_mm256_or_si256(l, _mm256_set1_epi32(r))
#define SIMD_XOR_VV(l,r)	_mm256_xor_si256(l,r)
#define SIMD_XOR_VW(l,r)	_mm256_xor_si256(l, _mm256_set1_epi32(r))
/*#define SIMD_NOT_V(l)		_mm256_andnot_si256(l,l)*/
#define SIMD_SHL_V(l,i)		_mm256_slli_epi32(l,i)
#define SIMD_SHR_V(l,i)		_mm256_srli_epi32(l,i)
/*#define SIMD_ROL_V(l,i)		_mm256_rol_epi32(l,i)*/
/*#define SIMD_ROR_V(l,i)		_mm256_ror_epi32(l,i)*/
#define SIMD_CLEANUP

#else /* __GNUC__ */

/* VISUAL STUDIO */

#include <immintrin.h>

#define SIMD_WORD			__m256i

#define SIMD_ZERO			_mm256_setzero_si256()
#define SIMD_WTOV(l)		_mm256_set1_epi32(l)
#define SIMD_ADD_VV(l,r)	_mm256_add_epi32(l,r)
#define SIMD_ADD_VW(l,r)	_mm256_add_epi32(l, _mm256_set1_epi32(r))
#define SIMD_SUB_VV(l,r)	_mm256_sub_epi32(l,r)
#define SIMD_SUB_VW(l,r)	_mm256_sub_epi32(l, _mm256_set1_epi32(r))
#define SIMD_AND_VV(l,r)	_mm256_and_si256(l,r)
#define SIMD_AND_VW(l,r)	_mm256_and_si256(l, _mm256_set1_epi32(r))
#define SIMD_OR_VV(l,r)		_mm256_or_si256(l,r)
#define SIMD_OR_VW(l,r)		_mm256_or_si256(l, _mm256_set1_epi32(r))
#define SIMD_XOR_VV(l,r)	_mm256_xor_si256(l,r)
#define SIMD_XOR_VW(l,r)	_mm256_xor_si256(l, _mm256_set1_epi32(r))
/*#define SIMD_NOT_V(l)		_mm256_andnot_si256(l,l)*/
#define SIMD_SHL_V(l,i)		_mm256_slli_epi32(l,i)
#define SIMD_SHR_V(l,i)		_mm256_srli_epi32(l,i)
/*#define SIMD_ROL_V(l,i)		_mm256_rol_epi32(l,i)*/
/*#define SIMD_ROR_V(l,i)		_mm256_ror_epi32(l,i)*/
#define SIMD_CLEANUP

#endif /* __GNUC__ */


/* these are general definitions for lacking SIMD operations */

#ifndef SIMD_NOT_V
#define SIMD_NOT_V(l)		SIMD_XOR_VW(l,0xFFFFFFFF)
#endif

#ifndef SIMD_NEG_V
#define SIMD_NEG_V(l)		SIMD_SUB_VV(SIMD_ZERO,l)
#endif

#ifndef SIMD_ROL_V
#define SIMD_ROL_V(l,i)		SIMD_OR_VV(SIMD_SHL_V(l,i),SIMD_SHR_V(l,32-i))
#define SIMD_ROR_V(l,i)		SIMD_OR_VV(SIMD_SHR_V(l,i),SIMD_SHL_V(l,32-i))
#endif
#endif /* SHA1DC_HAVE_AVX256 */
#endif /* SIMD_AVX256_HEADER */
