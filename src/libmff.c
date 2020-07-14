/**
 * license: BSD-3-Clause
*/

#include <libmff.h>

int __cpnbytes_mmap(int dest, int src, size_t mem_lim, size_t n, size_t offset, size_t buff_size);

int advstat(char *path, struct fileinf *fi)
{
	int fd;
	struct stat st;
	struct fileinf *_devinf;
	unsigned long nblk = 0;

	_devinf = malloc(sizeof(struct fileinf));
	if (_devinf == 0)
		return -1;

	_devinf->isblk = 0;
	_devinf->ischar = 0;
	_devinf->blkcnt = 0;
	_devinf->blksiz = 0;
	_devinf->path = s_strcpy(_devinf->path, path);

	fd = open(path, O_RDONLY | O_NONBLOCK);
	if (fd < 0)
		/* failed to open a raw file descriptor so no cleanup is needed */
		return -1;

	if (fstat(fd, &st) == -1)
		goto advstat_fail;

	if (S_ISBLK(st.st_mode)) {
		_devinf->isblk = 1;
		/* file is block device make the ioctl call as appropriate */
		if (ioctl(fd, BLKGETSIZE, &nblk) == -1)
			goto advstat_fail;
	} else if (S_ISCHR(st.st_mode)) {
		/* File is char device (ie /dev/[zero or random]) */
		_devinf->ischar = 1;
	} else if (S_ISREG(st.st_mode)) {
		/* If it's a regular file stat will have it's real size */
		nblk = st.st_blocks;
	} else {
		goto advstat_fail;
	}

	_devinf->blkcnt = nblk;
	_devinf->blksiz = st.st_blksize / 8;

	memmove(fi, _devinf, sizeof(struct fileinf));

	close(fd);
	return 0;
advstat_fail:
	close(fd);
	memmove(fi, _devinf, sizeof(struct fileinf));
	return -1;
}

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

	if (srclen > destlen) {
		free(dest);
		dest = malloc(srclen);
		if (dest == NULL)
			return NULL;
	}

	return strncpy(dest, src, srclen);
}

int cpnblk(struct fileinf *dest, struct fileinf *src, unsigned long memlim, unsigned long n, int sync)
{
	int ifd;
	int ofd;
	int i;
	int syncctr = 0;
	unsigned long wrt = 0;
	char *buff;
	unsigned long bufsiz = src->blksiz;
	unsigned long readsiz = 0;

	/* allocate the buffer */
	buff = malloc(sizeof(char) * bufsiz);
	if (!buff)
		return -1;

	/* zero fill buffer */
	for (i = 0; i < bufsiz; i++)
		buff[i] = 0;

	ifd = open(src->path, O_RDONLY);
	if (sync && memlim == 0)
		ofd = open(dest->path, O_WRONLY | O_SYNC);
	else
		ofd = open(dest->path, O_WRONLY);

	if (ifd == -1 || ofd == -1)
		return -1;

	if (memlim > 0) {
		if (__cpnbytes_mmap(ofd, ifd, memlim, n, 0, bufsiz) == -1)
			goto __cpnblk_end;

		wrt = n;
	} else {
		while (wrt < n) {
			readsiz = read(ifd, buff, bufsiz);

			if (readsiz <= 0)
				break;

			if (write(ofd, buff, readsiz) == -1)
				return -1;

			syncctr++;
			wrt++;
			if (syncctr >= 150000) {
				fsync(ofd);
				syncctr = 0;
			}
			fprintf(stderr, "\r{\"written\": %lu, \"total\": %lu}", wrt, n);
		}
	}

	if (sync == 0)
		fsync(ofd);

	/* clear the buffer just in case any sensitive information was held */
	for (i = 0; i < bufsiz; i++)
		buff[i] = 0;

__cpnblk_end:
	/* release the resources */
	free(buff);
	close(ifd);
	close(ofd);
	if (errno != 0) {
		return -1;
	}
	return wrt == n ? 0 : 1;
}

/* this function should not be called directly */
int __cpnbytes_mmap(int dest, int src, size_t mem_lim, size_t n, size_t offset, size_t buff_size)
{
	char *caret;
	char *srcbuff;
	size_t page_size = mem_lim;
	size_t written = 0;
	size_t left = 0;
	size_t src_pos;
	size_t abs_siz;

	srcbuff = mmap(NULL, page_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, src, 0);

	if (srcbuff == MAP_FAILED)
		goto __cpnblk_mmap_end;

	caret = malloc(buff_size);
	if (!caret)
		goto __cpnblk_mmap_end;

	abs_siz = n * buff_size;
	left = abs_siz;

	while (written < n) {
		src_pos = abs_siz - left;
		if (memcpy(caret, srcbuff + src_pos, buff_size) == 0) {
			/* if the memcpy fails try to swap the part of the file that is mapped */
			munmap(srcbuff, page_size);
			srcbuff = mmap(NULL, page_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, src, src_pos);
			if (srcbuff == MAP_FAILED)
				goto __cpnblk_mmap_end;
			goto __cpnblk_mmap_end;
		}
		if (write(dest, caret, buff_size) == -1)
			goto __cpnblk_mmap_end;
		written++;
		left -= buff_size;
		fprintf(stderr,"\r{\"written\": %lu, \"total\": %lu}", written, n);
	}

__cpnblk_mmap_end:
	munmap(srcbuff, page_size);
	free(caret);
	return errno == 0 ? 0 : -1;
}