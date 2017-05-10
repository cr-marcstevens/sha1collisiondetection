/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow <danshu@microsoft.com>
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/

/* this file generates mmx64 code using mmx64 MACROS (simd_mmx64.h) and generic SIMD code (sha1_simd.cinc) */

#include "simd_config.h"
#ifdef SHA1DC_HAVE_MMX64
#include "sha1.h"
#include "sha1_simd.h"

#include "simd_mmx64.h"

#define SHA1_MESSAGE_EXPANSION_SIMD     sha1_message_expansion_mmx64
#define SHA1_COMPRESSION_SIMD           sha1_mmx64
#define SHA1_COMPRESSION_W_SIMD         sha1_W_mmx64
#define SHA1_COMPRESSION_STATES_SIMD    sha1_states_mmx64
#define SHA1_RECOMPRESSION_SIMD(t)      sha1_recompress_fast_ ## t ## _mmx64
#define SHA1_RECOMPRESSION_TABLE_SIMD   sha1_recompression_step_mmx64
#define SHA1_APPLY_MESSAGE_DIFFERENCES  sha1_apply_message_differences_mmx64
#define SHA1_COMPARE_DIGESTS            sha1_compare_digests_mmx64
#define SHA1_SET_LANES                  sha1_set_lanes_mmx64

#include "sha1_simd.cinc"

sha1_simd_implementation_t sha1_simd_mmx64_implementation =
{
	simd_type_mmx64,
	SIMD_VECSIZE,
	(sha1_recompression_simd_fn)sha1_recompress_fast_58_mmx64,
	(sha1_recompression_simd_fn)sha1_recompress_fast_65_mmx64,
	(sha1_apply_message_differences_simd_fn)sha1_apply_message_differences_mmx64,
	(sha1_compare_digests_simd_fn)sha1_compare_digests_mmx64,
    (sha1_set_lanes_simd_fn)sha1_set_lanes_mmx64
};

#else

#pragma message "The file: sha1_simd_mmx64.c is not compiled for this architecture."

#endif /* SHA1DC_HAVE_MMX64 */
