/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow (danshu@microsoft.com)
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/
#ifndef SHA1DC_SHA1_SIMD_H
#define SHA1DC_SHA1_SIMD_H

#include "config.h"

typedef enum {
    simd_type_mmx64 = 0,
    simd_type_sse128,
    simd_type_neon128,
    simd_type_avx256,
    simd_type_avx512,
    simd_type_unknown = 0xff
} simd_type;

typedef void (*sha1_recompression_simd_fn)(void*, void*,  const void*, const void*);
typedef void (*sha1_apply_message_differences_simd_fn)(const uint32_t me[80], const void*, void*);
typedef void (*sha1_compare_digests_simd_fn)(const SHA1_CTX* ctx, const void*, const void*, void*);
typedef void (*sha1_set_lanes_simd_fn)(uint32_t, void*);

typedef struct {
    simd_type                                   type;
    size_t                                      cnt_lanes;
    sha1_recompression_simd_fn                  sha1_recompression_fast_58;
    sha1_recompression_simd_fn                  sha1_recompression_fast_65;
    sha1_apply_message_differences_simd_fn      sha1_apply_message_differences;
    sha1_compare_digests_simd_fn                sha1_compare_digests;
    sha1_set_lanes_simd_fn                      sha1_set_lanes;
} sha1_simd_implementation_t;

#ifdef SHA1DC_HAVE_MMX64
    #define SHA1DC_MMX64 (1)
    extern sha1_simd_implementation_t sha1_simd_mmx64_implementation;
#else
    #define SHA1DC_MMX64 (0)
#endif

#ifdef SHA1DC_HAVE_SSE128
    #define SHA1DC_SSE128 (1)
    extern sha1_simd_implementation_t sha1_simd_sse128_implementation;
#else
    #define SHA1DC_SSE128 (0)
#endif

#ifdef SHA1DC_HAVE_NEON128
    #define SHA1DC_NEON128 (1)
    extern sha1_simd_implementation_t sha1_simd_neon128_implementation;
#else
    #define SHA1DC_NEON128 (0)
#endif

#ifdef SHA1DC_HAVE_AVX256
    #define SHA1DC_AVX256 (1)
    extern sha1_simd_implementation_t sha1_simd_avx256_implementation;
#else
    #define SHA1DC_AVX256 (0)
#endif

#ifdef SHA1DC_HAVE_AVX512
    #define SHA1DC_AVX512 (1)
    extern sha1_simd_implementation_t sha1_simd_avx512_implementation;
#else
    #define SHA1DC_AVX512 (0)
#endif

#define SIMD_IMPLEMENTATION_CNT (SHA1DC_MMX64 + SHA1DC_SSE128 + SHA1DC_NEON128 + SHA1DC_AVX256 + SHA1DC_AVX512)




#endif /* SHA1DC_SHA1_SIMD_H */
