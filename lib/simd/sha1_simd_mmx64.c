/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow <danshu@microsoft.com>
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/

/* this file generates sse128 code using sse128 MACROS (simd_sse128.h) and generic SIMD code (sha1_simd.cinc) */
#ifdef SHA1DC_HAVE_MMX64

#include "sha1.h"
#include "simd_mmx64.h"

#define SHA1_MESSAGE_EXPANSION_SIMD					sha1_message_expansion_mmx64
#define SHA1_COMPRESSION_SIMD						sha1_mmx64
#define SHA1_COMPRESSION_W_SIMD						sha1_W_mmx64
#define SHA1_COMPRESSION_STATES_SIMD				sha1_states_mmx64
#define SHA1_RECOMPRESSION_SIMD(t)					sha1recompress_fast_ ## t ## _mmx64
#define SHA1_RECOMPRESSION_TABLE_SIMD				sha1_recompression_step_mmx64

#include "sha1_simd.cinc"

#else

#pragma message "The file: sha1_simd_mmx64.c is not compiled for this architecture."

#endif /* SHA1DC_HAVE_MMX64 */
