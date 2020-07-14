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
#include <sys/sysinfo.h>
#include <inttypes.h>

int main(int argc, char **argv)
{
	struct fileinf srcfileinf;
	struct fileinf destfileinf;
	struct sysinfo sysinf;
	unsigned long memlim = 0; /* 0 is the same as saying use 50% or if size */
	char *idev = NULL;
	char *odev = NULL;
	int nocopy = 0;
	int sync = 0;
	int cpr;
	int i;

	fprintf(stderr, "license: BSD-3-Clause\nCopyright (c) 2020, Matheus Xavier Silva\nAll rights reserved.\n");
	if (argc <= 1) {
		printf("need at least one argument\n");
		return -1;
	} else {
		for (i = 0; i < argc; i++) {
			if (strcmp(argv[i], "-if") == 0)
				idev = s_strcpy(idev, argv[i + 1]);
			else if (strcmp(argv[i], "-of") == 0)
				odev = s_strcpy(odev, argv[i + 1]);
			else if (strcmp(argv[i], "-sync") == 0)
				sync = 1;
			else if (strcmp(argv[i], "-memlim") == 0) {
				char *rawmemlim = NULL;
				errno = 0;
				rawmemlim = s_strcpy(rawmemlim, argv[i + 1]);
				memlim = strtoul(rawmemlim, NULL, 16);
				free(rawmemlim);
				if (errno != 0)
					return EINVAL;
			} else if (strcmp(argv[i], "-noop") == 0)
				nocopy = 1;
			else if (strcmp(argv[i], "-help") == 0) {
				printf("Mighty fast flasher:\
\nCopy data block by block from -if to -of,\
-sync will do an fsync after each write which is very slow,\
-memlim can be used to set a smaller io buffer value represented in percent bytes HEX (0x notation or plain number)\
(defaults to 50%% of free ram or input file size whichever is smaller).\
\nWorks best with scripted invocations.\n");
				return 0;
			}
		}
		if (idev == NULL || odev == NULL) {
			printf("Usage: -if /dev/xxxx -of /dev/xxxx\nuse -help for more info\n");
			return -1;
		}
	}

	if (advstat(idev, &srcfileinf) < 0) {
		if (errno == 2) {
			fprintf(stderr, "Could not stat %s\n", idev);
		} else if (errno == 13) {
			fprintf(stderr, "Permission denied to %s\n", idev);
		}
		goto cleanup;
	}

	if (advstat(odev, &destfileinf) < 0) {
		if (errno == 2) {
			fprintf(stderr, "Could not stat %s\n", odev);
		} else if (errno == 13) {
			fprintf(stderr, "Permission denied to %s\n", odev);
		}
		goto cleanup;
	}
	/* Free the original path strings as they are no longer needed */
	free(idev);
	free(odev);

	/* checks needed before flashing */
	if (nocopy != 0) {
		printf("\n-noop issued\n memlim: %lu \n", memlim);
		goto cleanup;
	}

	if (srcfileinf.ischar){
		cpr = cpnblk(&destfileinf, &srcfileinf, memlim, destfileinf.blkcnt, sync);
	} else {
		cpr = cpnblk(&destfileinf, &srcfileinf, memlim, srcfileinf.blkcnt, sync);
	}

	if (cpr < 0)
		goto cleanup;

	printf("\n%s\n", (cpr == 0 ? "Done" : "Warn"));
	return 0;
cleanup:
	/* since we allocated memory we must now free it to avoid leaks */
	/*free(&srcfileinf);
	free(&destfileinf);*/
	if (errno != 0) {
		fprintf(stderr, "errno %d\n", errno);
	}
	return errno == 0 ? 0 : errno;
}