/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow (danshu@microsoft.com)
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/

/* this file generates avx512 code using avx512 MACROS (simd_avx512.h) and generic SIMD code (sha1_simd.cinc) */

#ifdef SHA1DC_HAVE_AVX512
#include "sha1.h"

#include "simd_avx512.h"

#define SHA1_MESSAGE_EXPANSION_SIMD					sha1_message_expansion_avx512
#define SHA1_COMPRESSION_SIMD						sha1_avx512
#define SHA1_COMPRESSION_W_SIMD						sha1_W_avx512
#define SHA1_COMPRESSION_STATES_SIMD				sha1_states_avx512
#define SHA1_RECOMPRESSION_SIMD(t)					sha1recompress_fast_ ## t ## _avx512
#define SHA1_RECOMPRESSION_TABLE_SIMD				sha1_recompression_step_avx512

#include "sha1_simd.cinc"

#else

#pragma message "The file: sha1_simd_avx512.c is not compiled for this architecture."

#endif /*SHA1DC_HAVE_AVX512*/
