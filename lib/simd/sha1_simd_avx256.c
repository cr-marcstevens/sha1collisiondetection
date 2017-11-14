/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow (danshu@microsoft.com)
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/

/* this file generates avx256 code using avx256 MACROS (simd_avx256.h) and generic SIMD code (sha1_simd.cinc) */

#include "../lib/config.h"

#include "simd_config.h"
#ifdef SHA1DC_HAVE_AVX256
#include "sha1.h"

#include "sha1_simd.h"

#include "simd_avx256.h"

#define SHA1_MESSAGE_EXPANSION_SIMD     sha1_message_expansion_avx256
#define SHA1_COMPRESSION_SIMD           sha1_avx256
#define SHA1_COMPRESSION_W_SIMD         sha1_W_avx256
#define SHA1_COMPRESSION_STATES_SIMD	sha1_states_avx256
#define SHA1_RECOMPRESSION_SIMD(t)	    sha1_recompress_fast_ ## t ## _avx256
#define SHA1_RECOMPRESSION_TABLE_SIMD	sha1_recompression_step_avx256
#define SHA1_APPLY_MESSAGE_DIFFERENCES	sha1_apply_message_differences_avx256
#define SHA1_COMPARE_DIGESTS            sha1_compare_digests_avx256
#define SHA1_SET_LANES                  sha1_set_lanes_avx256

#include "sha1_simd.cinc"

sha1_simd_implementation_t sha1_simd_avx256_implementation =
{
	simd_type_avx256,
	SIMD_VECSIZE,
	(sha1_recompression_simd_fn)sha1_recompress_fast_58_avx256,
	(sha1_recompression_simd_fn)sha1_recompress_fast_65_avx256,
	(sha1_apply_message_differences_simd_fn)sha1_apply_message_differences_avx256,
	(sha1_compare_digests_simd_fn)sha1_compare_digests_avx256,
    (sha1_set_lanes_simd_fn)sha1_set_lanes_avx256
};

#else

#pragma message "The file: sha1_simd_avx256.c is not compiled for this architecture."

#endif /* SHA1DC_HAVE_AVX256 */
