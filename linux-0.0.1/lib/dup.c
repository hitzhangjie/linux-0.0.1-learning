#define __LIBRARY__
#include <unistd.h>

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-12 09:06:50 PM
 * 
 * syscall1 is a macro that will expand the dup to:
 * int dup(int fd);
 *
 * dup is used to copy the fd, then we will get a new fd.
 */
_syscall1(int,dup,int,fd)
