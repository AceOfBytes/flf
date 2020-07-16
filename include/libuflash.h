/**
 * SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef __LIBUFLASH_H
#define __LIBUFLASH_H

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <stdbool.h>

#define __LIB_UFLASH_VERSION "v0.1.0"

struct uf_st {
  size_t blkcnt;
  size_t blksiz;
  char *path;
  int isblk;
  int ischar;
};

struct uf_cpycfg {
  size_t i_offset;
  size_t o_offset;
  size_t at_most;
  size_t mem_lim;
  size_t blk_siz;
  bool sync_f;
  int out_fd;
};

/**
 * Stat path and fill fi with the relevant information
 * @param path
 * @param fi
 * @return 0 on success -1 on failure (fi will have zeroed fields on failure)
 */
int uf_stat(char *path, struct uf_st *fi);
/**
 *
 * @param dest
 * @param src
 * @param cfg
 * @return
 */
int uf_cpy_nblk(struct uf_st *dest, struct uf_st *src, struct uf_cpycfg *cfg);
/**
 * WARNING: this function may allocate memory,
 * remember to free the returned buffer as soon as possible.
*/
char *uf_sstrcpy(char *dest, char *src);
#endif /* !__LIBUFLASH_H */