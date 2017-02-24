/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow (danshu@microsoft.com)
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/

// this file generates avx256 code using avx256 MACROS (simd_avx256.h) and generic SIMD code (ubc_check_simd.cinc) 
#ifdef HAVE_AVX
#include "ubc_check.h"
#include "simd_avx256.h"

#define UBC_CHECK_SIMD ubc_check_avx256

#include "ubc_check_simd.cinc"

#else

#pragma message "The file: ubc_check_simd_avx256.c is not compiled for this architecture."

#endif // HAVE_AVX
