#define __LIBRARY__
#include <unistd.h>

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-12 09:43:52 PM
 * 
 * _syscall3 is a macro, write will be expanded to:
 * int write(int fd, const char *buf, off_t count);
 *
 * write is used to write count bytes into file specified by param 'fd', from
 * the buffer specified by param 'buf'.
 */
_syscall3(int,write,int,fd,const char *,buf,off_t,count)
