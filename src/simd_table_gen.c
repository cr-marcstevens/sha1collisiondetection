/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow <danshu@microsoft.com>
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/

#define MAX_SIMD_EXPONENT (4) /* max SIMD width 2^4=16 in words */

#define SIMD_MAX_WORD_ALIGNMENT (4) /* max alignment required is 4 words (even for 16 word vectors) */
#define SIMD_MAX_CASE_PADDING (0) /* max padding between cases in words to try to improve alignment */

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

int eval_align(int off1, int len1, int off2, int len2)
{
	int i, j, weight = 0, maxalign;
	int newoff;
	for (j = 1; j <= MAX_SIMD_EXPONENT; ++j)
	{
		maxalign = (1<<j);
		if (maxalign > SIMD_MAX_WORD_ALIGNMENT)
			maxalign = SIMD_MAX_WORD_ALIGNMENT;

		newoff = off1 & ~(maxalign-1);
		for (i = newoff; i < off1 + len1; i += 1<<j)
			++weight;

		newoff = off2 & ~(maxalign-1);
		for (i = newoff; i < off2 + len2; i += 1<<j)
			++weight;
	}
	return weight;
}

int generate_code(DV_info_t* DVS, int nrdvs)
{
	int i,j;
	int totok58 = 0, totok65 = 0;
	int overlap, bestoverlap, bestweight, cnt58, cnt65, weight;
	int best58first = 0, bestpadding = 0, endpadding;
	FILE* fd;

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
	/* TODO: compute better endpadding based on MAX_SIMD_WORD_ALIGNMENT instead of 1<<MAX_SIMD_EXPONENT */
	endpadding = ((1<<20) - (cnt58 + cnt65 + bestpadding)) & ((1<<MAX_SIMD_EXPONENT)-1);
	if (best58first)
	{
		printf("Using case58(%i) padding(%i) case65(%i) padding(%i)\n", cnt58, bestpadding, cnt65, endpadding);
	} else {
		printf("Using case65(%i) padding(%i) case58(%i) padding(%i)\n", cnt65, bestpadding, cnt58, endpadding);
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
		"#include <stdlib.h>\n"
		"#include <stdint.h>\n"
		"#define SHA1DC_SIMD_NRDVS (%i)\n"
		"#define SHA1DC_SIMD_TABLESIZE (%i)\n"
		"extern uint32_t sha1_dm_interleaved[80][SHA1DC_SIMD_TABLESIZE];\n"
		, nrdvs, nrdvs+16);

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
		"#include <stdint.h>\n"
		);
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
