/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow (danshu@microsoft.com)
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/

#include <stdlib.h>
#include <stdint.h>

#include "sha1.h"
#include "sha1_simd.h"
#include "dvs_simd.h"

/* volatile char should have atomic writes. */
volatile char simd_index = -1;

sha1_simd_implementation_t *simd_implementation_table[SIMD_IMPLEMENTATION_CNT+1] =
{
#ifdef SHA1DC_HAVE_MMX64
	&sha1_simd_mmx64_implementation,
#endif
#ifdef SHA1DC_HAVE_SSE128
	&sha1_simd_sse128_implementation,
#endif
#ifdef SHA1DC_HAVE_NEON128
	&sha1_simd_neon128_implementation,
#endif
#ifdef SHA1DC_HAVE_AVX256
	&sha1_simd_avx256_implementation,
#endif
#ifdef SHA1DC_HAVE_AVX512
	&sha1_simd_avx512_implementation,
#endif
	NULL
};


typedef struct {
	size_t offset58;
	size_t offset65;
} dv_table_info_t;

dv_table_info_t dv_table_info[SIMD_IMPLEMENTATION_CNT+1] = 
{
#ifdef SHA1DC_HAVE_MMX64
    {
        SHA1DC_SIMD_2_OFFSET58,
        SHA1DC_SIMD_2_OFFSET65
    },
#endif
#ifdef SHA1DC_HAVE_SSE128
    {
        SHA1DC_SIMD_4_OFFSET58,
        SHA1DC_SIMD_4_OFFSET65
    },
#endif
#ifdef SHA1DC_HAVE_NEON128
    {
        SHA1DC_SIMD_4_OFFSET58,
        SHA1DC_SIMD_4_OFFSET65
    },
#endif
#ifdef SHA1DC_HAVE_AVX256
    {
        SHA1DC_SIMD_8_OFFSET58,
        SHA1DC_SIMD_8_OFFSET65
    },
#endif
#ifdef SHA1DC_HAVE_AVX512
    {
        SHA1DC_SIMD_16_OFFSET58,
        SHA1DC_SIMD_16_OFFSET65
    },
#endif
	{0}

};

void initialize_simd()
{
	/* TODO Put configuration code here. */
}

size_t SHA1DC_get_simd()
{
	if ((char)-1 == simd_index)
	{
		initialize_simd();
	}

	return (size_t)simd_index;
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

void sha1_process_simd(SHA1_CTX* ctx, const uint32_t block[16])
{
	ctx->ihv1[0] = ctx->ihv[0];
	ctx->ihv1[1] = ctx->ihv[1];
	ctx->ihv1[2] = ctx->ihv[2];
	ctx->ihv1[3] = ctx->ihv[3];
	ctx->ihv1[4] = ctx->ihv[4];

	ctx->simd = block[1]; /* DUMMY OPERATION: REMOVE ASAP */
#if 0

	sha1_compression_states(ctx->ihv, block, ctx->m1, ctx->states);

	if (ctx->detect_coll)
	{
		for (i = 0; sha1_dvs[i].dvType != 0; ++i)
		{
			for (j = 0; j < 80; ++j)
				ctx->m2[j] = ctx->m1[j] ^ sha1_dvs[i].dm[j];

			sha1_recompression_step(sha1_dvs[i].testt, ctx->ihv2, ihvtmp, ctx->m2, ctx->states[sha1_dvs[i].testt]);

			// to verify SHA-1 collision detection code with collisions for reduced-step SHA-1
			if ((0 == ((ihvtmp[0] ^ ctx->ihv[0]) | (ihvtmp[1] ^ ctx->ihv[1]) | (ihvtmp[2] ^ ctx->ihv[2]) | (ihvtmp[3] ^ ctx->ihv[3]) | (ihvtmp[4] ^ ctx->ihv[4])))
				|| (ctx->reduced_round_coll && 0==((ctx->ihv1[0] ^ ctx->ihv2[0]) | (ctx->ihv1[1] ^ ctx->ihv2[1]) | (ctx->ihv1[2] ^ ctx->ihv2[2]) | (ctx->ihv1[3] ^ ctx->ihv2[3]) | (ctx->ihv1[4] ^ ctx->ihv2[4]))))
			{
				ctx->found_collision = 1;

				if (ctx->safe_hash)
				{
					sha1_compression_W(ctx->ihv, ctx->m1);
					sha1_compression_W(ctx->ihv, ctx->m1);
				}
			}
		}
	}
#endif
}
