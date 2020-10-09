/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow (danshu@microsoft.com)
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/

#include "../config.h"

#include "sha1.h"
#include "sha1_simd.h"
#include "dvs_simd.h"
#include "simd_config.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#if defined(SHA1DC_HAVE_MMX64) || defined(SHA1DC_HAVE_SSE128) || defined(SHA1DC_HAVE_AVX256) || defined(SHA1DC_HAVE_AVX512)
#ifdef __GNUC__
#include <x86intrin.h>
#elif _MSC_VER
#include <intrin.h>
#endif

static void sha1dc_cpuid(uint32_t level, uint32_t sublevel, uint32_t result[4])
{
#ifdef _MSC_VER
	__cpuidex(result, level, sublevel);
#else
	result[0] = level;
	result[1] = sublevel;
	result[2] = 0;
	result[3] = 0;
	__asm__ ("cpuid" : "+a" (result[0]), "+b" (result[1]), "+c" (result[2]), "+d" (result[3]));
#endif
}

#endif

/* volatile char should have atomic writes. */
static volatile char simd_index = (char)simd_type_unknown;

static sha1_simd_implementation_t *simd_implementation_table[5] =
{
#ifdef SHA1DC_HAVE_MMX64
	&sha1_simd_mmx64_implementation,
#else
	NULL,
#endif
#ifdef SHA1DC_HAVE_NEON128
	&sha1_simd_neon128_implementation,
#else
	NULL,
#endif
#ifdef SHA1DC_HAVE_SSE128
	&sha1_simd_sse128_implementation,
#else
	NULL,
#endif
#ifdef SHA1DC_HAVE_AVX256
	&sha1_simd_avx256_implementation,
#else
	NULL,
#endif
#ifdef SHA1DC_HAVE_AVX512
	&sha1_simd_avx512_implementation
#else
	NULL
#endif
};


typedef struct {
	size_t offset58;
	size_t offset65;
} dv_table_info_t;

static dv_table_info_t dv_table_info[5] = 
{
#ifdef SHA1DC_SIMD_2_OFFSET58
    { SHA1DC_SIMD_2_OFFSET58, SHA1DC_SIMD_2_OFFSET65 }, /* MMX64 */
#else
    { 0, 0 },
#endif
#ifdef SHA1DC_SIMD_4_OFFSET58
    { SHA1DC_SIMD_4_OFFSET58, SHA1DC_SIMD_4_OFFSET65 }, /* NEON128 */
    { SHA1DC_SIMD_4_OFFSET58, SHA1DC_SIMD_4_OFFSET65 }, /* SSE128 */
#else
    { 0, 0 },
    { 0, 0 },
#endif
#ifdef SHA1DC_SIMD_8_OFFSET58
    { SHA1DC_SIMD_8_OFFSET58, SHA1DC_SIMD_8_OFFSET65 }, /* AVX256 */
#else
    { 0, 0 },
#endif
#ifdef SHA1DC_SIMD_16_OFFSET58
    { SHA1DC_SIMD_16_OFFSET58, SHA1DC_SIMD_16_OFFSET65 } /* AVX512 */
#else
    { 0, 0 }
#endif
};

static void initialize_simd()
{
	/* TODO Put configuration code here. */
#if defined(SHA1DC_HAVE_MMX64) || defined(SHA1DC_HAVE_SSE128) || defined(SHA1DC_HAVE_AVX256) || defined(SHA1DC_HAVE_AVX512)
	uint32_t abcd[4];
	uint32_t maxlevel;
	sha1dc_cpuid(0,0,abcd);
	maxlevel = abcd[0];
#ifdef SHA1DC_HAVE_AVX512
	if (7 <= maxlevel && (sha1dc_cpuid(7,0,abcd),1) && abcd[1]&(1<<17))
	{
		simd_index = (char)simd_type_avx512;
		return;
	}
#endif
#ifdef SHA1DC_HAVE_AVX256
	if (7 <= maxlevel && (sha1dc_cpuid(7,0,abcd),1) && abcd[1]&(1<<5))
	{
		simd_index = (char)simd_type_avx256;
		return;
	}
#endif
#ifdef SHA1DC_HAVE_SSE128
	if (1 <= maxlevel && (sha1dc_cpuid(1,0,abcd),1) && abcd[3]&(1<<26))
	{
		simd_index = (char)simd_type_sse128;
		return;
	}
#endif
#ifdef SHA1DC_HAVE_MMX64
	if (1 <= maxlevel && (sha1dc_cpuid(1,0,abcd),1) && abcd[3]&(1<<23))
	{
		simd_index = (char)simd_type_mmx64;
		return;
	}
#endif

#endif
#if defined(SHA1DC_HAVE_NEON128)
	/* Do not detect NEON, just enable it if it is supported */
	if (simd_index == (char)simd_type_unknown || simd_index < (char)simd_type_neon128)
	{
		simd_index = (char)simd_type_neon128;
		return;
	}
#endif
	simd_index = (char)simd_type_disabled;
}

int SHA1DC_get_simd()
{
	if (simd_index == simd_type_unknown)
	{
		initialize_simd();
	}

	return (int)simd_index;
}

static size_t get_dv_table_offset_58()
{
    size_t i;

    size_t ret = (size_t)-1;

    i = SHA1DC_get_simd();

    if ((i < SIMD_IMPLEMENTATION_CNT))
    {
        ret = dv_table_info[i].offset58;
    }

    return ret;
}

size_t get_dv_table_offset_65()
{
    size_t i;

    size_t ret = (size_t)-1;

    i = SHA1DC_get_simd();

    if ((i < SIMD_IMPLEMENTATION_CNT))
    {
        ret = dv_table_info[i].offset65;
    }

    return ret;
}

size_t get_simd_lane_count()
{
    size_t i;

    size_t ret = (size_t)-1;

    i = SHA1DC_get_simd();

    if ((i < SIMD_IMPLEMENTATION_CNT) &&
        (NULL != simd_implementation_table[i]))
    {
        ret = simd_implementation_table[i]->cnt_lanes;
    }

    return ret;
}

void sha1_recompress_fast_58_simd(void* ihvin, void* ihvout, const void* me, void* state)
{
	size_t i;
	
	i = SHA1DC_get_simd();

	if ((i < SIMD_IMPLEMENTATION_CNT) &&
		(NULL != simd_implementation_table[i]))
	{		
		simd_implementation_table[i]->sha1_recompression_fast_58(ihvin, ihvout, me, state);
	}
}

void sha1_recompress_fast_65_simd(void* ihvin, void* ihvout, const void* me, void* state)
{
	size_t i;

	i = SHA1DC_get_simd();

	if ((i < SIMD_IMPLEMENTATION_CNT) &&
		(NULL != simd_implementation_table[i]))
	{
		simd_implementation_table[i]->sha1_recompression_fast_65(ihvin, ihvout, me, state);
	}
}
void sha1_apply_message_differences_simd(const uint32_t me[80], const void* dm, void* dme)
{
	size_t i;

	i = SHA1DC_get_simd();

	if ((i < SIMD_IMPLEMENTATION_CNT) &&
		(NULL != simd_implementation_table[i]))
	{
		simd_implementation_table[i]->sha1_apply_message_differences(me, dm, dme);
	}
}

void sha1_compare_digests_simd(const SHA1_CTX* ctx, const void* ihv_full_collision, const void* ihv_reduced_round, void* collision_detected)
{
	size_t i;

	i = SHA1DC_get_simd();

	if ((i < SIMD_IMPLEMENTATION_CNT) &&
		(NULL != simd_implementation_table[i]))
	{
		simd_implementation_table[i]->sha1_compare_digests(ctx, ihv_full_collision, ihv_reduced_round, collision_detected);
	}
}

void sha1_simd_set_lanes(uint32_t x, void* simd_x)
{
    size_t i;

    i = SHA1DC_get_simd();

    if ((i < SIMD_IMPLEMENTATION_CNT) &&
        (NULL != simd_implementation_table[i]))
    {
        simd_implementation_table[i]->sha1_set_lanes(x, simd_x);
    }
}

void sha1_load_state_to_simd(uint32_t state[5], void* simd_state)
{
    size_t simd_lanes;
    size_t i;

    simd_lanes = get_simd_lane_count();

    for (i = 0; i < 5; i++)
    {
        sha1_simd_set_lanes(state[i], &(((uint32_t*)simd_state)[i*simd_lanes]));
    }
}

void sha1_process_simd(SHA1_CTX* ctx, const uint32_t block[16])
{
    uint32_t dme[80][SHA1DC_SIMD_TABLESIZE];

    uint32_t check_results[SHA1DC_SIMD_TABLESIZE + SHA1DC_SIMD_FINALPADDING] = { 0 };

    /*
     * HACK: These buffers are enough to store the largest SIMD registers we have.
     * Probably would be better to have a "MAX_SIMD_SIZE" macro.
     */
    uint32_t simd_states[5 * 16];
    uint32_t simd_ihv_full[5 * 16];
    uint32_t simd_ihv_reduced[5 * 16];

    size_t step_58_offset;
    size_t step_65_offset;

    size_t lane_cnt;

    size_t i;

	ctx->ihv1[0] = ctx->ihv[0];
	ctx->ihv1[1] = ctx->ihv[1];
	ctx->ihv1[2] = ctx->ihv[2];
	ctx->ihv1[3] = ctx->ihv[3];
	ctx->ihv1[4] = ctx->ihv[4];

	sha1_compression_states(ctx->ihv, block, ctx->m1, ctx->states);

	if (ctx->detect_coll)
	{
        lane_cnt = get_simd_lane_count();

        step_58_offset = get_dv_table_offset_58();

        for (i = step_58_offset; i < SHA1DC_SIMD_END58; i += lane_cnt)
        {
            sha1_apply_message_differences_simd(ctx->m1, &(sha1_dvs_interleaved.dm[0][i]), dme);

            sha1_load_state_to_simd(ctx->states[58], simd_states);

            sha1_recompress_fast_58_simd(simd_ihv_reduced, simd_ihv_full, dme, simd_states);

            sha1_compare_digests_simd(ctx, simd_ihv_full, simd_ihv_reduced, &(check_results[i]));
        }

        for (i = step_58_offset; i < SHA1DC_SIMD_END58; i++)
        {
            ctx->found_collision |= (0 == check_results[i]);
        }

        step_65_offset = get_dv_table_offset_65();

        for (i = step_65_offset; i < SHA1DC_SIMD_END65; i += lane_cnt)
        {
            sha1_apply_message_differences_simd(ctx->m1, &(sha1_dvs_interleaved.dm[0][i]), dme);

            sha1_load_state_to_simd(ctx->states[65], simd_states);

            sha1_recompress_fast_65_simd(simd_ihv_reduced, simd_ihv_full, dme, simd_states);

            sha1_compare_digests_simd(ctx, simd_ihv_full, simd_ihv_reduced, &(check_results[i]));
        }

        for (i = step_65_offset; i < SHA1DC_SIMD_END65; i++)
        {
            ctx->found_collision |= (0 == check_results[i]);
        }
   }
}
