/**
 * license: BSD-3-Clause
*/

#include <libmff.h>

int cpnblk(char *dest, char *src, struct dev_info *o_devinf, struct dev_info *i_devinf, unsigned long memlim, int sync)
{
	int ifd;
	int ofd;
	double percnt;
	int i = 0;
	int syncctr = 0;
	unsigned long wrt = 0;
	char *mmapbuff = 0;
	char *buff;
	unsigned long bufsiz = o_devinf->bufsiz;
	unsigned long n = i_devinf->blkcnt;
	unsigned long readsiz = 0;

	/* allocate the buffer */
	buff = malloc(sizeof(char) * bufsiz);
	if (!buff)
		return -1;

	/* zero fill buffer */
	for (i = 0; i < bufsiz; i++)
		buff[i] = 0;

	ifd = open(src, O_RDONLY);
	if (sync != 0)
		ofd = open(dest, O_WRONLY | O_SYNC);
	else
		ofd = open(dest, O_WRONLY);

	if (ifd == -1 || ofd == -1)
		return -1;

	if (memlim > 0)
	{
		mmapbuff = mmap(0, memlim, PROT_READ, MAP_SHARED, ifd, 0);
		if (mmapbuff == MAP_FAILED)
			return -1;
	}

	while (wrt < n)
	{
		if (mmapbuff == 0)
		{
			readsiz = read(ifd, buff, bufsiz);

			if (readsiz <= 0)
				break;
		}
		else if (mmapbuff != 0)
		{
			if (wrt < n - 1)
				for (i = 0; i < bufsiz; i++)
					buff[i] = mmapbuff[i];
			readsiz = bufsiz;
		}

		if (write(ofd, buff, readsiz) == -1)
			return -1;

		syncctr++;
		wrt++;
		percnt = ((double)wrt / (double)n) * 100.0;
		if (syncctr >= 150000)
		{
			fsync(ofd);
			syncctr = 0;
		}
		fprintf(stderr, "\rWritten %lu of %lu blocks (%.2f%%) to %s", wrt, n, percnt, dest);
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
	munmap(mmapbuff, memlim);

	return wrt == n ? 0 : 1;
}

int tellsiz(char *path, struct dev_info *devinf)
{
	int fd;
	unsigned long nblk = 0;
	struct stat st;
	struct dev_info *_devinf;

	_devinf = malloc(sizeof(struct dev_info));
	if (_devinf == 0)
		return -1;

	_devinf->isblk = 0;
	_devinf->blkcnt = 0;
	_devinf->bufsiz = 0;

	fd = open(path, O_RDONLY | O_NONBLOCK);
	if (fd < 0)
		/* failed to open a raw file descriptor so no cleanup is needed */
		return -1;

	if (fstat(fd, &st) == -1)
		return -1;
	if (S_ISBLK(st.st_mode))
	{
		_devinf->isblk = 1;
		/* file is block device make the ioctl call as appropriate */
		if (ioctl(fd, BLKGETSIZE, &nblk) == -1)
			goto fail;
	}
	else
		nblk = st.st_blocks;

	_devinf->blkcnt = nblk;
	_devinf->bufsiz = st.st_blksize / 8;

	devinf = memmove(devinf, _devinf, sizeof(struct dev_info));

	close(fd);
	return 0;
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