/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sha1.h"

int main(int argc, char** argv) 
{
	FILE* fd;
	unsigned char hash2[20];
	char buffer[65536];
	size_t size;
	SHA1_CTX ctx2;
	int i,j;

	if (argc < 2) 
	{
		printf("Usage: test <file>\n");
		return 1;
	}

	for (i=1; i < argc; ++i) 
	{
		// initialize SHA-1 context
		SHA1DCInit(&ctx2);

		// if the program name includes the word 'partial' then also test for reduced-round SHA-1 collisions 
		if (NULL != strstr(argv[0], "partial"))
		{
			SHA1DCSetDetectReducedRoundCollision(&ctx2, 1);
		}

		// open file
		fd = fopen(argv[i], "rb");
		if (fd == NULL) 
		{
			printf("cannot open file: %s\n", argv[i]);
			return 1;
		}

		// feed file through SHA-1 update fuction
		while (1) 
		{
			size=fread(buffer,1,65536,fd);
			SHA1DCUpdate(&ctx2, buffer, (unsigned)(size));
			if (size != 65536)
				break;
		}
		if (ferror(fd)) 
		{
			printf("error while reading file: %s\n", argv[i]);
			return 1;
		}
		if (!feof(fd)) 
		{
			printf("not end of file?: %s\n",argv[i]);
			return 1;
		}

		// obtain SHA-1 and print it
		SHA1DCFinal(hash2,&ctx2);
		for (j = 0; j < 20; ++j)
		{ 
			sprintf(buffer+(j*2), "%02x", hash2[j]);
		}
		buffer[20*2] = 0;
		if (ctx2.found_collision) 
		{
			printf("%s *coll* %s\n", buffer, argv[i]);
		} 
		else 
		{
			printf("%s  %s\n", buffer, argv[i]);
		}

		fclose(fd);
	}
}
