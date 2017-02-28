/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow (danshu@microsoft.com)
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/

// this file generates avx256 code using avx256 MACROS (simd_avx256.h) and generic SIMD code (sha1_simd.cinc) 

#ifdef HAVE_AVX
#include "sha1.h"

#include "simd_avx256.h"

#define SHA1_MESSAGE_EXPANSION_SIMD					sha1_message_expansion_avx256
#define SHA1_COMPRESSION_SIMD						sha1_avx256
#define SHA1_COMPRESSION_W_SIMD						sha1_W_avx256
#define SHA1_COMPRESSION_STATES_SIMD				sha1_states_avx256
#define SHA1_RECOMPRESSION_SIMD(t)					sha1recompress_fast_ ## t ## _avx256
#define SHA1_RECOMPRESSION_TABLE_SIMD				sha1_recompression_step_avx256

#include "sha1_simd.cinc"

#else

#pragma message "The file: sha1_simd_avx256.c is not compiled for this architecture."

#endif //HAVE_AVX
