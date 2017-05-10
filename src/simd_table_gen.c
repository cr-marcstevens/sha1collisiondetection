/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow <danshu@microsoft.com>
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/

#include "../lib/simd/config.h"

/* TODO: set SIMD_MAX_WORD_ALIGNMENT and MAX_SIMD_EXPONENT in ../lib/simd/config.h depending on platform and features */

/* #define MAX_SIMD_EXPONENT (4) // max SIMD width 2^4=16 in words */
#if defined(SHA1DC_HAVE_AVX512)
#define MAX_SIMD_EXPONENT 4
#elif defined(SHA1DC_HAVE_AVX256)
#define MAX_SIMD_EXPONENT 3
#elif (defined(SHA1DC_HAVE_SSE128) || defined(SHA1DC_HAVE_NEON128))
#define MAX_SIMD_EXPONENT 2
#elif defined(SHA1DC_HAVE_MMX64)
#define MAX_SIMD_EXPONENT 1
#else
#define MAX_SIMD_EXPONENT 0
#endif

#define SIMD_MAX_WORD_ALIGNMENT (4) /* max alignment required is 4 words (even for 16 word vectors) */
#define SIMD_MAX_CASE_PADDING (SIMD_MAX_WORD_ALIGNMENT-1) /* max padding between cases in words to try to improve alignment */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct
{
	int dvType;
	int dvK;
	int dvB;
	int ok58;
	int ok65;
	uint32_t dv[80];
	uint32_t dm[80];
} DV_info_t;

typedef struct
{
	int off1, len1, pad1, off2, len2, pad2, finalpad;
	int simd_off2[MAX_SIMD_EXPONENT+1];
	int first58;
} DV_order_info_t;

static uint32_t rotate_left(uint32_t x, unsigned n) { return (x<<n)|(x>>(32-n)); }
static uint32_t rotate_right(uint32_t x, unsigned n) { return (x>>n)|(x<<(32-n)); }

void expand_me(uint32_t v[80], int offset)
{
	int i;
	for (i = offset - 1; i >= 0; --i)
		v[i] = rotate_right(v[i+16], 1) ^ v[i+13] ^ v[i+8] ^ v[i+2];
	for (i = offset + 16; i < 80; ++i)
		v[i] = rotate_left( v[i-3] ^ v[i-8] ^ v[i-14] ^ v[i-16], 1);
}

void init_DV(DV_info_t* DV)
{
	int i;
	int K = DV->dvK, B = DV->dvB, T = DV->dvType;
	uint32_t* dv = DV->dv;
	uint32_t* dm = DV->dm;

	/* initialize 16 words of dv */
	for (i = 0; i < 80; ++i)
		DV->dv[i] = 0;
	DV->dv[K+15] = 1 << B;
	if (T == 2)
	{
		DV->dv[K+1] = 1 << ((31+B) & 31);
		DV->dv[K+3] = 1 << ((31+B) & 31);
	}
	/* expand to entire dv */
	expand_me(dv, K);

	/* initialize 16 words of dm */
	for (i = 16; i < 32; ++i)
		dm[i] = dv[i-0] ^ rotate_left(dv[i-1],5) ^ dv[i-2]
			 ^ rotate_left(dv[i-3],30) ^ rotate_left(dv[i-4],30) ^ rotate_left(dv[i-5],30);
	/* expand to entire dm */
	expand_me(dm, 16);
}

int parse_error(char* str)
{
	fprintf(stderr, "Parse error: %s", str);
	return 1;
}

DV_order_info_t DV_order_info;

int eval_align(int off1, int len1, int off2, int len2)
{
	int i, j, weight = 0, maxalign;
	int newoff, pad2;

	DV_order_info.off1 = off1;
	DV_order_info.len1 = len1;
	DV_order_info.pad1 = off2 - (off1 + len1);
	DV_order_info.off2 = off2;
	DV_order_info.len2 = len2;
	DV_order_info.pad2 = 0;
	DV_order_info.simd_off2[0] = off2;

	for (j = 1; j <= MAX_SIMD_EXPONENT; ++j)
	{
		maxalign = (1<<j);
		if (maxalign > SIMD_MAX_WORD_ALIGNMENT)
			maxalign = SIMD_MAX_WORD_ALIGNMENT;

		/* count how many function calls are needed for this SIMD size */
		newoff = off1 & ~(maxalign-1);
		for (i = newoff; i < off1 + len1; i += 1<<j)
			++weight;

		newoff = off2 & ~(maxalign-1);
		for (i = newoff; i < off2 + len2; i += 1<<j)
			++weight;

		/* compute offset for second case and extra padding required */
		pad2 = i - (off2 + len2);
		while (pad2 >= maxalign && newoff >= maxalign)
		{
			pad2 -= maxalign;
			newoff -= maxalign;
		}

		DV_order_info.simd_off2[j] = newoff;
		/* for this SIMD size we read pad2 words after the last DV, use final padding to ensure we read in allocated memory */
		if (pad2 > DV_order_info.finalpad)
			DV_order_info.finalpad = pad2;
		/* but except for the last row, we can wrap-around, so we can remove padding in multiples of maxalign */
		pad2 %= maxalign;
		if (pad2 > DV_order_info.pad2)
			DV_order_info.pad2 = pad2;

	}
	/* remove regular row padding from final table padding */
	DV_order_info.finalpad -= DV_order_info.pad2;

	return weight;
}

int generate_code(DV_info_t* DVS, int nrdvs)
{
	int i,j;
	int totok58 = 0, totok65 = 0;
	int overlap, bestoverlap, bestweight, cnt58, cnt65, weight;
	int best58first = 0, bestpadding = 0, nrcols;
	FILE* fd;
	DV_info_t* ordered_DVS[256];
	DV_info_t pad_DV;

	pad_DV.dvType = 0;
	pad_DV.dvK = 0;
	pad_DV.dvB = 0;
	for (i = 0; i < 80; ++i)
		pad_DV.dm[i] = 0;
	pad_DV.ok58 = 0;
	pad_DV.ok65 = 0;
	for (i=0; i < 256; ++i)
		ordered_DVS[i] = & pad_DV;

	/* Compute overlap */
	for (i = 0; i < nrdvs; ++i)
	{
		totok58 += DVS[i].ok58;
		totok65 += DVS[i].ok65;
	}
	printf("totDVs=%i totok58=%i totok65=%i\n", nrdvs, totok58, totok65);

	overlap = totok58 + totok65 - nrdvs;
	if (overlap < 0) parse_error("overlap negative");

	/* Analyze best division */
	bestoverlap = 0;
	bestweight = 1<<20;
	printf("\nAnalyzing overlap:\n");
	for (i = 0; i <= overlap; ++i)
	{
		cnt58 = totok58 - overlap + i;
		cnt65 = totok65 - i;
		weight = 0;
		for (j = 1; j <= MAX_SIMD_EXPONENT; ++j)
			weight += ((cnt58+(1<<j)-1)>>j) + ((cnt65+(1<<j)-1)>>j);
		printf("cnt58=%i cnt65=%i weight=%i\n", cnt58, cnt65, weight);
		if (weight < bestweight)
		{
			bestoverlap = i;
			bestweight = weight;
		}
	}
	cnt58 = totok58 - overlap + bestoverlap;
	cnt65 = totok65 - bestoverlap;

	printf("Using cnt58=%i cnt65=%i weight=%i\n", cnt58, cnt65, bestweight);

	/* Apply best division */
	j = bestoverlap;
	for (i = 0; i < nrdvs; ++i)
	{
		if (DVS[i].ok58 + DVS[i].ok65 == 2)
		{
			if (j > 0)
			{
				DVS[i].ok65 = 0;
				--j;
			}
			else
			{
				DVS[i].ok58 = 0;
			}
		}
	}

	bestweight = 1<<20;
	best58first = 1;
	bestpadding = 0;
	for (i = 0; i < SIMD_MAX_WORD_ALIGNMENT && i <= SIMD_MAX_CASE_PADDING; ++i)
	{
		weight = eval_align(0, cnt58, cnt58+i, cnt65);
		printf("Eval align %i %i %i = %i\n", cnt58, i, cnt65, weight);
		if (weight < bestweight)
		{
			best58first = 1;
			bestpadding = i;
			bestweight = weight;
		}

		weight = eval_align(0, cnt65, cnt65+i, cnt58);
		printf("Eval align %i %i %i = %i\n", cnt65, i, cnt58, weight);
		if (weight < bestweight)
		{
			best58first = 0;
			bestpadding = i;
			bestweight = weight;
		}
	}

	if (best58first)
	{
		DV_order_info.first58 = 1;
		/* recompute optimal padding for this case */
		eval_align(0, cnt58, cnt58 + bestpadding, cnt65);
		nrcols = cnt58 + cnt65 + DV_order_info.pad1 + DV_order_info.pad2;
		printf("Using [case58(%i) padding(%i) case65(%i) padding(%i)]*80 + finalpadding(%i)\n", cnt58, bestpadding, cnt65, DV_order_info.pad2, DV_order_info.finalpad);
		for (i=0,j=0; i < nrdvs; ++i)
			if (DVS[i].ok58)
				ordered_DVS[j++] = DVS+i;
		j += DV_order_info.pad1;
		for (i=0; i < nrdvs; ++i)
			if (DVS[i].ok65)
				ordered_DVS[j++] = DVS+i;
		j += DV_order_info.pad2;
		if (j != nrcols)
			parse_error("j != nrdvs");
	} else {
		DV_order_info.first58 = 0;
		/* recompute optimal padding for this case */
		eval_align(0, cnt65, cnt65 + bestpadding, cnt58);
		nrcols = cnt58 + cnt65 + DV_order_info.pad1 + DV_order_info.pad2;
		printf("Using [case65(%i) padding(%i) case58(%i) padding(%i)]*80 + finalpadding(%i)\n", cnt65, bestpadding, cnt58, DV_order_info.pad2, DV_order_info.finalpad);
		for (i=0,j=0; i < nrdvs; ++i)
			if (DVS[i].ok65)
				ordered_DVS[j++] = DVS+i;
		j += DV_order_info.pad1;
		for (i=0; i < nrdvs; ++i)
			if (DVS[i].ok58)
				ordered_DVS[j++] = DVS+i;
		j += DV_order_info.pad2;
		if (j != nrcols)
			parse_error("j != nrdvs");
	}
	for (j = 1; j <= MAX_SIMD_EXPONENT; ++j)
	{
		printf("SIMD %4i: off1=0 off2=%i\n", (32<<j), DV_order_info.simd_off2[j]);
	}

	/* Output code */
	fd = fopen("lib/simd/dvs_simd.h", "w");
	if (fd == NULL)
		parse_error("Cannot open output file to write");
	fprintf(fd,
		"/***\n"
		"* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow <danshu@microsoft.com>\n"
		"* Distributed under the MIT Software License.\n"
		"* See accompanying file LICENSE.txt or copy at\n"
		"* https://opensource.org/licenses/MIT\n"
		"***/\n\n"
		"#ifndef SHA1DC_DVS_SIMD_HEADER\n"
		"#define SHA1DC_DVS_SIMD_HEADER\n\n"
		"#include <stdlib.h>\n"
		"#include <stdint.h>\n\n"
		"#define SHA1DC_SIMD_NRDVS (%i)\n" /*nrdvs*/
		"#define SHA1DC_SIMD_TABLESIZE (%i)\n" /*nrdvs+pad1+pad2*/
		"#define SHA1DC_SIMD_FINALPADDING (%i)\n" /*finalpad*/
		,
		nrdvs, nrdvs+DV_order_info.pad1+DV_order_info.pad2, DV_order_info.finalpad
		);
	for (j = 1; j <= MAX_SIMD_EXPONENT; ++j)
	{
		fprintf(fd,
			"#define SHA1DC_SIMD_%i_OFFSET58 (%i)\n" /* off1 / off2 */
			"#define SHA1DC_SIMD_%i_OFFSET65 (%i)\n" /* off2 / off1 */
			,
			(1<<j), DV_order_info.first58 ? 0 : DV_order_info.simd_off2[j],
			(1<<j), DV_order_info.first58 ? DV_order_info.simd_off2[j] : 0
			);
	}
	fprintf(fd,
		"\ntypedef struct {\n"
		"    uint32_t dm[80][SHA1DC_SIMD_TABLESIZE];\n"
		"    uint32_t mask58[SHA1DC_SIMD_TABLESIZE+SHA1DC_SIMD_FINALPADDING];\n"
		"    uint32_t mask65[SHA1DC_SIMD_TABLESIZE+SHA1DC_SIMD_FINALPADDING];\n"
		);
	fprintf(fd,
		"    int dvType[SHA1DC_SIMD_TABLESIZE+SHA1DC_SIMD_FINALPADDING];\n"
		"    int dvK[SHA1DC_SIMD_TABLESIZE+SHA1DC_SIMD_FINALPADDING];\n"
		"    int dvB[SHA1DC_SIMD_TABLESIZE+SHA1DC_SIMD_FINALPADDING];\n"
		"    } sha1_dvs_interleaved_t;\n"
		"extern const sha1_dvs_interleaved_t sha1_dvs_interleaved;\n\n"
		"#endif /* SHA1DC_DVS_SIMD_HEADER */\n"
		);
	fclose(fd);

	fd = fopen("lib/simd/dvs_simd.c", "w");
	if (fd == NULL)
		parse_error("Cannot open output file to write");
	fprintf(fd,
		"/***\n"
		"* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow <danshu@microsoft.com>\n"
		"* Distributed under the MIT Software License.\n"
		"* See accompanying file LICENSE.txt or copy at\n"
		"* https://opensource.org/licenses/MIT\n"
		"***/\n\n"
		"#include \"dvs_simd.h\"\n"
		"#include <stdlib.h>\n"
		"#include <stdint.h>\n\n"
		"const sha1_dvs_interleaved_t sha1_dvs_interleaved = {\n"
		"    {\n"
		);
	for (i = 0; i < 80; ++i)
	{
		fprintf(fd, "        {");
		for (j = 0; j < nrcols; ++j)
			if (ordered_DVS[j]->ok58+ordered_DVS[j]->ok65==0)
				fprintf(fd, ", 0");
			else
				fprintf(fd, "%s 0x%08x", j==0?"":",", ordered_DVS[j]->dm[i]);
		fprintf(fd, " }%s\n", i<79 ? "," : "");
	}
	fprintf(fd, "    },\n");

	fprintf(fd, "    {");
	for (j=0; j < nrcols+DV_order_info.finalpad; ++j)
		fprintf(fd, "%s %s", j==0?"":",", ordered_DVS[j]->ok58 ? "0xFFFFFFFF" : (ordered_DVS[j]->ok65?"0x00000000":"0"));
	fprintf(fd, " },\n");

	fprintf(fd, "    {");
	for (j=0; j < nrcols+DV_order_info.finalpad; ++j)
		fprintf(fd, "%s %s", j==0?"":",", ordered_DVS[j]->ok65 ? "0xFFFFFFFF" : (ordered_DVS[j]->ok58?"0x00000000":"0"));
	fprintf(fd, " },\n");

	fprintf(fd, "    {");
	for (j=0; j < nrcols+DV_order_info.finalpad; ++j)
		fprintf(fd, "%s %2i", j==0?"":",", ordered_DVS[j]->dvType);
	fprintf(fd, " },\n");

	fprintf(fd, "    {");
	for (j=0; j < nrcols+DV_order_info.finalpad; ++j)
		fprintf(fd, "%s %2i", j==0?"":",", ordered_DVS[j]->dvK);
	fprintf(fd, " },\n");

	fprintf(fd, "    {");
	for (j=0; j < nrcols+DV_order_info.finalpad; ++j)
		fprintf(fd, "%s %2i", j==0?"":",", ordered_DVS[j]->dvB);
	fprintf(fd, " }\n");
	fprintf(fd, "};\n");

	fclose(fd);

	return 0;
}

int process_dv_list(char* filename, int maxDVs)
{
	FILE* fd;
	char buffer[1<<16];
	size_t size;
	char* ptr;
	char* ptrend;
	DV_info_t DVS[256];
	int nrdvs = 0;
	DV_info_t* DV = DVS+0;
	char* DVtypestr[3] = { "err", "I", "II" };
	int K;

	fd = fopen(filename, "r");
	if (fd == NULL)
		return parse_error("Cannot open file");
	size = fread(buffer,1,65536,fd);
	if (size >= 65536)
		return parse_error("File larger than 65536 bytes!");
	printf("Parsing at most %i DVs...\n", maxDVs);

	ptrend = buffer+size;
	for (ptr = buffer; ptr < ptrend;)
	{
		/* I_48_0 : compl=2^64.5138 (prob=2^-71.5138) */

		DV->dvType = 0;

		while (ptr < ptrend && *ptr != 'I')
			++ptr;
		if (ptr >= ptrend)
			break;
		for (; *ptr == 'I'; ++ptr,++DV->dvType)
			;
		if (DV->dvType > 2) return parse_error("there is no DV type III");
		if (*ptr++ != '_') return parse_error("expected _ after I");
		K = DV->dvK = atoi(ptr);
		ptr += 2;
		if (*ptr++ != '_') return parse_error("expected _ after K");
		DV->dvB = atoi(ptr);

		/* compute dv and dm tables in DV */
		init_DV(DV);

                if (DV->dvType == 1)
                {
			DV->ok58 = ((58 >= K+5) & (58 <= K+15)) ? 1 : 0;
			DV->ok65 = ((65 >= K+5) & (65 <= K+15)) ? 1 : 0;
                }
                else
                {
			DV->ok58 = ((58 >= K+9) & (58 <= K+15)) ? 1 : 0;
			DV->ok65 = ((65 >= K+9) & (65 <= K+15)) ? 1 : 0;
                }

		printf("Parsed DV: %s(%i,%i) ok58=%i ok65=%i\n", DVtypestr[DV->dvType], DV->dvK, DV->dvB, DV->ok58, DV->ok65);

		++DV; ++nrdvs;
		if (DV != DVS+nrdvs)
			parse_error("huh?!?");
		if (nrdvs >= maxDVs)
			break;
	}
	fclose(fd);
	return generate_code(DVS, nrdvs);
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("Usage: %s <file> [<nr>]\n", argv[0]);
		return 1;
	}
	if (argc == 2)
		return process_dv_list(argv[1],256);
	else
		return process_dv_list(argv[1],atoi(argv[2]));
}
