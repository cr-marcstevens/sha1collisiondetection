#include <string.h>
#include <stdio.h>
#include <algorithm>

#include <iostream>

#include "sha1.h"
#include "ubc_check.h"

#include "test_util_lib.h"

using namespace std;

int GenRandomBytes(
	unsigned char*	pb,
	size_t			cb,
	br::mt19937		&rng)
{
	size_t i;

	for (i = 0; i < cb; i++)
	{
		pb[i] = (unsigned char)rng();
	}

	return i;

}

int HashRandomBytes(
	size_t			cntBytesToHash,
	unsigned int	uiSeed)
{
	unsigned char hash[20];

	unsigned char buffer_to_hash[64];

	SHA1_CTX ctx;

	int i = cntBytesToHash;

	br::mt19937 rng(uiSeed);
	cout << "Hashing...";
	SHA1DCInit(&ctx);

	while (0 < i)
	{
		unsigned int cb = min(64, i);

		cb = GenRandomBytes(buffer_to_hash, cb, rng);

		SHA1DCUpdate(&ctx, (char *)buffer_to_hash, cb);

		i -= cb;
	}

	SHA1DCFinal(hash, &ctx);

	cout << "Done." << endl;

	if (ctx.found_collision)
	{
		cout << "Collision found." << endl;
	}

	return 0;

}

int UBCVerifyRandomBytes(
	size_t			cntBytesToHash,
	unsigned int	uiSeed)
{
	unsigned char buffer_to_hash[64];

	uint32_t expanded_message[80];

	uint32_t dvmask[1] = { 0 };
	uint32_t dvmask_verify[1] = { 0 };

	int i = cntBytesToHash;

	br::mt19937 rng(uiSeed);
	cout << "Unavoidable bit checking...";

	while (0 < i)
	{
		unsigned int cb = min(64, i);

		cb = GenRandomBytes(buffer_to_hash, cb, rng);

		memset(dvmask, 0, sizeof(dvmask));
		memset(dvmask_verify, 0, sizeof(dvmask_verify));

		sha1_compression_W((uint32_t*)buffer_to_hash, expanded_message);

		ubc_check(expanded_message, dvmask);
		//ubc_check_verify(expanded_message, dvmask_verify);

		if (0 != memcmp(dvmask, dvmask_verify, sizeof(dvmask)))
		{
			printf("Calculated dvmask does not equal dvmask verify output.\n");
		}

		i -= cb;
	}

	cout << "Done." << endl;

	return 0;
}
