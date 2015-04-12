#define __LIBRARY__
#include <unistd.h>

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-12 09:09:41 PM
 * 
 * _syscall1 is a macro, close will be expanded to:
 * int close(int fd);
 *
 * this function is used to close the fd.
 *
 * 1) everytime call open(), a new file descriptor table item will be created.
 * 2) everytime call dup(), a new file descriptor will be created, but the new
 * created fd and old fd are both pointing to the same file descriptor table
 * item.
 * 3) there's only one copy of i-node info of specific file on disk.
 */
_syscall1(int,close,int,fd)
