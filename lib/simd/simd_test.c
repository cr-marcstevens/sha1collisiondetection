#ifdef TEST_MMX64
#include "simd_mmx64.h"
#endif

#ifdef TEST_SSE128
#include "simd_sse128.h"
#endif

#ifdef TEST_AVX256
#include "simd_avx256.h"
#endif

#ifdef TEST_AVX512
#include "simd_avx512.h"
#endif

#ifdef TEST_NEON128
#include "simd_neon128.h"
#endif

#include <stdint.h>

int main(int argc, char** argv)
{
	int i;
	union {
		SIMD_WORD v;
		uint32_t w[SIMD_VECSIZE];
	} x, y, z;
	x.v = SIMD_ZERO;
	y.v = SIMD_WTOV(5);
	z.v = SIMD_WTOV(11);
	for (i = 0; i < SIMD_VECSIZE; ++i)
	{
		if (x.w[i] != 0 || y.w[i] != 5 || z.w[i] != 11)
			return 1;
		z.w[i] = 0;
	}
	x.v = SIMD_ADD_VV(y.v, x.v);
	x.v = SIMD_SUB_VV(x.v, y.v);
	x.v = SIMD_AND_VV(x.v, y.v);
	x.v = SIMD_OR_VV(x.v, y.v);
	x.v = SIMD_XOR_VV(x.v, y.v);
	x.v = SIMD_ADD_VW(y.v, 1);
	x.v = SIMD_SUB_VW(x.v, 2);
	x.v = SIMD_AND_VW(x.v, 3);
	x.v = SIMD_OR_VW(x.v, 4);
	x.v = SIMD_XOR_VW(x.v, 5);
	x.v = SIMD_NOT_V(x.v);
	x.v = SIMD_SHL_V(x.v, 2);
	x.v = SIMD_SHR_V(x.v, 2);
	x.v = SIMD_ROL_V(x.v, 2);
	x.v = SIMD_ROR_V(x.v, 2);
	SIMD_CLEANUP;

	/* dummy code touching argc and argv */
	for (i = 0; i < argc; ++i)
		if (argv[i][0] == 0)
			break;
	return 0;
}
