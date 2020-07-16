/**
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <libuflash.h>

int cp_nbytes_mmap(int dest, int src, int rprtfd, size_t mem_lim, size_t n, size_t offset, size_t buff_size);

int uf_stat(char *path, struct uf_st *fi)
{
	int fd;
	struct stat st;
	struct uf_st *_devinf;
	unsigned long nblk = 0;

	_devinf = malloc(sizeof(struct uf_st));
	if (_devinf == 0)
		return -1;

	_devinf->isblk = 0;
	_devinf->ischar = 0;
	_devinf->blkcnt = 0;
	_devinf->blksiz = 0;
	_devinf->path = uf_sstrcpy(_devinf->path, path);

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

	memmove(fi, _devinf, sizeof(struct uf_st));

	close(fd);
	return 0;
advstat_fail:
	close(fd);
	memmove(fi, _devinf, sizeof(struct uf_st));
	return -1;
}

char *uf_sstrcpy(char *dest, char *src)
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

int uf_cpy_nblk(struct uf_st *dest, struct uf_st *src, struct uf_cpycfg *cfg)
{
	int ofd;
	int ifd;

	if((ifd = open(src->path, O_RDONLY)) == -1)
		goto __uf_cpy_nblk_err;

	if((ofd = open(src->path, O_RDONLY)) == -1)
		goto __uf_cpy_nblk_err;

	if (cfg == NULL) {
		errno = EINVAL;
		goto __uf_cpy_nblk_err;
	}

	if (cfg->mem_lim > 0)
		cp_nbytes_mmap(ofd, ifd);

__uf_cpy_nblk_err:
	close(ifd);
	close(ofd);
	return -1;
}

/* this function should not be called directly */
int cp_nbytes_mmap(int dest, int src, int rprtfd, size_t mem_lim, size_t n, size_t offset, size_t buff_size)
{
	char *caret = 0;
	char *srcbuff = NULL;
	FILE *fifofs;
	size_t page_size = mem_lim;
	size_t written = 0;
	size_t left;
	size_t src_pos;
	size_t abs_siz;

	if (rprtfd != -1)
		if ((fifofs = fdopen(rprtfd, "w")) != NULL)
			goto __cpnblk_mmap_end;

	srcbuff = mmap(NULL, page_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, src, 0);
	if (srcbuff == MAP_FAILED)
		goto __cpnblk_mmap_end;

	if (!(caret = malloc(buff_size)))
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
		if (rprtfd != -1)
			fprintf(fifofs, "{\"written\": %lu,\"total\": %lu}", written, n);
	}

__cpnblk_mmap_end:
	munmap(srcbuff, page_size);
	free(caret);
	return errno == 0 ? 0 : -1;
}