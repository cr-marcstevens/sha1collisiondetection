/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow (danshu@microsoft.com)
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/

/* this file generates avx512 code using avx512 MACROS (simd_avx512.h) and generic SIMD code (ubc_check_simd.cinc) */
#include "simd_config.h"
#ifdef SHA1DC_HAVE_AVX512
#include "ubc_check.h"
#include "simd_avx512.h"

#define UBC_CHECK_SIMD ubc_check_avx512

#include "ubc_check_simd.cinc"

#else

#pragma message "The file: ubc_check_simd_avx512.c is not compiled for this architecture."

#endif /* SHA1DC_HAVE_AVX512 */
