/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow (danshu@microsoft.com)
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/

/* this file generates sse128 code using sse128 MACROS (simd_sse128.h) and generic SIMD code (sha1_simd.cinc) */

#include "simd_config.h"
#ifdef SHA1DC_HAVE_SSE128
#include "sha1.h"
#include "sha1_simd.h"

#include "simd_sse128.h"

#define SHA1_MESSAGE_EXPANSION_SIMD     sha1_message_expansion_sse128
#define SHA1_COMPRESSION_SIMD           sha1_sse128
#define SHA1_COMPRESSION_W_SIMD         sha1_W_sse128
#define SHA1_COMPRESSION_STATES_SIMD    sha1_states_sse128
#define SHA1_RECOMPRESSION_SIMD(t)      sha1_recompress_fast_ ## t ## _sse128
#define SHA1_RECOMPRESSION_TABLE_SIMD   sha1_recompression_step_sse128
#define SHA1_APPLY_MESSAGE_DIFFERENCES	sha1_apply_message_differences_sse128
#define SHA1_COMPARE_DIGESTS            sha1_compare_digests_sse128
#define SHA1_SET_LANES                  sha1_set_lanes_sse128

#include "sha1_simd.cinc"

sha1_simd_implementation_t sha1_simd_sse128_implementation =
{
	simd_type_sse128,
	SIMD_VECSIZE,
	(sha1_recompression_simd_fn)sha1_recompress_fast_58_sse128,
	(sha1_recompression_simd_fn)sha1_recompress_fast_65_sse128,
	(sha1_apply_message_differences_simd_fn)sha1_apply_message_differences_sse128,
	(sha1_compare_digests_simd_fn)sha1_compare_digests_sse128,
    (sha1_set_lanes_simd_fn)sha1_set_lanes_sse128
};

#else

#pragma message "The file: sha1_simd_sse128.c is not compiled for this architecture."

#endif /*SHA1DC_HAVE_SSE128*/
