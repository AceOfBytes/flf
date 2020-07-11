/*
license: BSD-3-Clause
Copyright (c) 2020, Matheus Xavier Silva
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

	Redistributions of source code must retain the above copyright notice, this
	list of conditions and the following disclaimer.

	Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

	Neither the name of the copyright holder nor the names of its
	contributors may be used to endorse or promote products derived from
	this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <libmff.h>
#include <string.h>

int main(int argc, char **argv)
{
	struct dev_info iblkct;
	struct dev_info oblkct;
	char *idev = NULL;
	char *odev = NULL;
	int nocopy = 0;
	int cpr;
	int sync = 0;
	int i;

	if (argc <= 1)
	{
		printf("need at least one argument\n");
		return -1;
	}
	else
	{
		for (i = 0; i < argc; i++)
		{
			if (strcmp(argv[i], "-if") == 0)
				idev = s_strcpy(idev, argv[i + 1]);
			else if (strcmp(argv[i], "-of") == 0)
				odev = s_strcpy(odev, argv[i + 1]);
			else if (strcmp(argv[i], "-sync") == 0)
				sync = 1;
			else if (strcmp(argv[i], "-nocopy") == 0)
				nocopy = 1;
			else if (strcmp(argv[i], "-help") == 0)
			{
				printf("Mighty fast flasher in that it simply copies\ndata byte by byte from -if to -of, -sync will do an fsync after each write which is very slow");
				printf("\nWorks best with scripted invocations.\n");
				return 0;
			}
		}
		if (idev == NULL || odev == NULL)
		{
			printf("Usage: -if /dev/xxxx -of /dev/xxxx\nuse -help for more info\n");
			return -1;
		}
	}

	iblkct = tellsz(idev);
	if (iblkct == 0)
	{
		if (errno == 2)
		{
			fprintf(stderr, "Could not stat %s", idev);
			goto cleanup;
		}
	}

	oblkct = tellsz(odev);
	if (oblkct == 0)
	{
		if (errno == 2)
		{
			fprintf(stderr, "Could not stat %s", odev);
			goto cleanup;
		}
	}

	printf("if: %.2f MiB of: %.2f MiB\n", (double)iblkct * BUFF_SIZE / (1024 * 1024),
	       (double)oblkct * BUFF_SIZE / (1024 * 1024));

	/* don't even try if the output file is smaller than the input */
	if (oblkct < iblkct)
		return -1;

	if (nocopy != 0)
	{
		printf("-nocopy issued");
		return 0;
	}

	cpr = cpnblk(odev, idev, iblkct, sync);
	if (cpr < 0)
		goto cleanup;
	printf("\n%s\n", (cpr == 0 ? "Done" : "Warn"));
	/**
 * since we allocated memory we must now free it to avoid leaks
 */
cleanup:
	free(idev);
	free(odev);
	fprintf(stderr, "errno %d", errno);
	return errno == 0 ? 0 : errno;
}