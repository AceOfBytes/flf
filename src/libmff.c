/**
 * license: BSD-3-Clause
*/

#include <libmff.h>

int cpnblk(char *dest, char *src, size_t n, size_t bufsiz, int sync)
{
	int ifd;
	int ofd;
	double prcnt;
	int i = 0;
	unsigned long wrt = 0;
	char *buff;

	/* allocate the buffer */
	buff = (char *)malloc(bufsiz);
	if (buff == 0)
		return -1;

	/* Zero fill buffer */
	for (i = 0; i < bufsiz; i++)
		buff[i] = 0;

	ifd = open(src, O_RDONLY);
	if (sync != 0)
		ofd = open(dest, O_WRONLY | O_SYNC);
	else
		ofd = open(dest, O_WRONLY);

	if (ifd == -1 || ofd == -1)
		return -1;

	while (read(ifd, &buff, bufsiz) > 0 && wrt <= n)
	{
		if (write(ofd, &buff, bufsiz) == -1)
			return -1;

		for (i = 0; i < bufsiz; i++)
			buff[i] = 0;
		wrt++;
		prcnt = ((double)wrt / (double)n) * 100.0;
		fprintf(stderr, "\rWritten %lu of %lu blocks (%.2f%%) to %s", wrt, n, prcnt, dest);
	}

	if (sync == 0)
		fsync(ofd);

	/* clear the buffer just in case any sensitive information was held */
	for (i = 0; i < bufsiz; i++)
		buff[i] = 0;

	/* release the resources */
	free(buff);
	close(ifd);
	close(ofd);

	return wrt == n ? 0 : 1;
}

int tellsz(char *path, struct dev_info *dinf)
{
	int fd;
	unsigned long nblk = 0;
	struct stat st;

	dinf = malloc(sizeof(struct dev_info));
	if (dinf == 0)
	{
		return -1;
	}

	fd = open(path, O_RDONLY | O_NONBLOCK);
	if (fd <= 0)
		/* failed to open a raw file descriptor so no cleanup is needed */
		return -1;

	if (fstat(fd, &st) == -1)
		return -1;
	if (S_ISBLK(st.st_mode))
	{
		/* file is block device make the ioctl call as appropriate */
		if (ioctl(fd, BLKGETSIZE, &nblk) == -1)
			goto fail;
	}
	else
		nblk = st.st_blocks;

	close(fd);

	return nblk;
fail:
	close(fd);
	return -1;
}

/**
 * WARNING: this function may allocate memory,
 * remember to free the returned buffer when convenient!
*/
char *s_strcpy(char *dest, char *src)
{
	size_t srclen = 0;
	size_t destlen = 0;

	/**
     * guard against null buffers
    */
	if (src != 0)
		srclen = strlen(src);
	else
		return 0;

	if (dest != 0)
		destlen = strlen(dest);

	if (srclen > destlen)
	{
		free(dest);
		dest = malloc(srclen);
		if (dest == NULL)
			return NULL;
	}

	return strncpy(dest, src, srclen);
}