/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow <danshu@microsoft.com>
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/

// this header defines SIMD MACROS for sse128 intrinsics
// used to generate sse128 code from generic SIMD code (sha1_simd.cinc, ubc_check_simd.cinc) 

#ifndef SIMD_MMX64_HEADER
#define SIMD_MMX64_HEADER
#ifdef HAVE_MMX
#define SIMD_VERSION	mmx64
#define SIMD_VECSIZE	2

#ifdef __GNUC__
#include <mmintrin.h>

#define SIMD_WORD __m64

#define SIMD_ZERO			_mm_setzero_si64()
#define SIMD_WTOV(l)		_mm_set1_pi32(l)	
#define SIMD_ADD_VV(l,r)	_mm_add_pi32(l,r)
#define SIMD_ADD_VW(l,r)	_mm_add_pi32(l, _mm_set1_pi32(r))
#define SIMD_SUB_VV(l,r)	_mm_sub_pi32(l,r)
#define SIMD_SUB_VW(l,r)	_mm_sub_pi32(l, _mm_set1_pi32(r))
#define SIMD_AND_VV(l,r)	_mm_and_si64(l,r)
#define SIMD_AND_VW(l,r)	_mm_and_si64(l, _mm_set1_pi32(r))
#define SIMD_OR_VV(l,r)		_mm_or_si64(l,r)
#define SIMD_OR_VW(l,r)		_mm_or_si64(l, _mm_set1_pi32(r))
#define SIMD_XOR_VV(l,r)	_mm_xor_si64(l,r)
#define SIMD_XOR_VW(l,r)	_mm_xor_si64(l, _mm_set1_pi32(r))
//#define SIMD_NOT_V(l)		_mm_andnot_si64(l,l)
#define SIMD_SHL_V(l,i)		_mm_slli_pi32(l,i)
#define SIMD_SHR_V(l,i)		_mm_srli_pi32(l,i)
//#define SIMD_ROL_V(l,i)		_mm_rol_pi32(l,i)
//#define SIMD_ROR_V(l,i)		_mm_ror_epi32(l,i)
#define SIMD_CLEANUP		_mm_empty()


#else // __GNUC__

// VISUAL STUDIO
#include <mmintrin.h>

#define SIMD_WORD			__m64

#define SIMD_ZERO			_mm_setzero_si64()
#define SIMD_WTOV(l)		_mm_set1_pi32(l)	
#define SIMD_ADD_VV(l,r)	_mm_add_pi32(l,r)
#define SIMD_ADD_VW(l,r)	_mm_add_pi32(l, _mm_set1_pi32(r))
#define SIMD_SUB_VV(l,r)	_mm_sub_pi32(l,r)
#define SIMD_SUB_VW(l,r)	_mm_sub_pi32(l, _mm_set1_pi32(r))
#define SIMD_AND_VV(l,r)	_mm_and_si64(l,r)
#define SIMD_AND_VW(l,r)	_mm_and_si64(l, _mm_set1_pi32(r))
#define SIMD_OR_VV(l,r)		_mm_or_si64(l,r)
#define SIMD_OR_VW(l,r)		_mm_or_si64(l, _mm_set1_pi32(r))
#define SIMD_XOR_VV(l,r)	_mm_xor_si64(l,r)
#define SIMD_XOR_VW(l,r)	_mm_xor_si64(l, _mm_set1_pi32(r))
//#define SIMD_NOT_V(l)		_mm_andnot_si64(l,l)
#define SIMD_SHL_V(l,i)		_mm_slli_pi32(l,i)
#define SIMD_SHR_V(l,i)		_mm_srli_pi32(l,i)
//#define SIMD_ROL_V(l,i)		_mm_rol_pi32(l,i)
//#define SIMD_ROR_V(l,i)		_mm_ror_epi32(l,i)
#define SIMD_CLEANUP		_mm_empty()

#endif // __GNUC__


// these are general definitions for lacking SIMD operations

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
#endif // HAVE_MMX
#endif // SIMD_SSE128_HEADER
