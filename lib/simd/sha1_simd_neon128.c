/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow (danshu@microsoft.com)
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/

/* this file generates neon 32x4 code using neon MACROS (arm_neon.h) and generic SIMD code (sha1_simd.cinc)*/

#include "simd_config.h"
#ifdef SHA1DC_HAVE_NEON128
#include "sha1.h"
#include "sha1_simd.h"

#include "simd_neon128.h"

#define SHA1_MESSAGE_EXPANSION_SIMD     sha1_message_expansion_neon128
#define SHA1_COMPRESSION_SIMD           sha1_neon128
#define SHA1_COMPRESSION_W_SIMD         sha1_W_neon128
#define SHA1_COMPRESSION_STATES_SIMD    sha1_states_neon128
#define SHA1_RECOMPRESSION_SIMD(t)      sha1_recompress_fast_ ## t ## _neon128
#define SHA1_RECOMPRESSION_TABLE_SIMD   sha1_recompression_step_neon128
#define SHA1_APPLY_MESSAGE_DIFFERENCES	sha1_apply_message_differences_neon128
#define SHA1_COMPARE_DIGESTS            sha1_compare_digests_neon128

#include "sha1_simd.cinc"

sha1_simd_implementation_t sha1_simd_neon128_implementation =
{
	simd_type_neon128,
	SIMD_VECSIZE,
	(sha1_recompression_simd_fn)sha1_recompress_fast_58_neon128,
	(sha1_recompression_simd_fn)sha1_recompress_fast_65_neon128,
	(sha1_apply_message_differences_simd_fn)sha1_apply_message_differences_neon128,
	(sha1_compare_digests_simd_fn)sha1_compare_digests_neon128
};

#else

#pragma message "The file: sha1_simd_neon128.c is not compiled for this architecture."

#endif /*SHA1DC_HAVE_NEON128*/
