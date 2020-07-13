/**
 * license: BSD-3-Clause
*/

#ifndef __LIBMFF_H
#define __LIBMFF_H

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

struct fileinf
{
	size_t blkcnt;
	size_t blksiz;
	char *path;
	int isblk;
	int ischar;
};

/**
 * Stat path and fill fi with the relevant information
 * @param path
 * @param fi
 * @return 0 on success -1 on failure (fi will have zeroed fields on failure)
 */
int advstat(char *path, struct fileinf *fi);
/**
 * Copy at most n blocks from srcdev to destdev, you are responsible for
 * ensuring that destdev has enough free space.
 * @param src a fileinf struct pointing to the source device/file
 * @param dest
 * @param memlim
 * @param n
 * @param sync
 * @return
 */
int cpnblk(struct fileinf *dest, struct fileinf *src, unsigned long memlim, unsigned long n, int sync);
/**
 * WARNING: this function may allocate memory,
 * remember to free the returned buffer as soon as possible.
*/
char *s_strcpy(char *dest, char *src);
#endif