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
#include <string.h>

struct dev_info
{
	size_t blkcnt;
	size_t bufsiz;
	int isblk;
};

/**
 * returns 0 on success and -1 on error
 **/
int tellsz(char *path, struct dev_info *dinf);
/**
 * takes: two paths, destination and source in that order, number of buffer
 * copies to make.
 * returns: les than, zero, or more if respectively
 * failed to read or write, copied the buffer exactly n times or
 * copied the buffer less than n times.
 * ATTENTION: sync is a boolean anything other than 0 is true
*/
int cpnblk(char *dest, char *src, size_t n, size_t bufsiz, int sync);
char *s_strcpy(char *dest, char *src);
#endif