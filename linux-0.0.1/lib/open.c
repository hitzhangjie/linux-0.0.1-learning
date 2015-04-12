#define __LIBRARY__
#include <unistd.h>
#include <stdarg.h>

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-12 09:35:38 PM
 * 
 * this function is used to open a file named filename. open mode will be
 * limited by flag.
 */
int open(const char * filename, int flag, ...)
{
	register int res;
	va_list arg;

	va_start(arg,flag);
	__asm__("int $0x80"
		:"=a" (res)
		/* __NR_open is defined as a system call number */
		:"0" (__NR_open),"b" (filename),"c" (flag),
		"d" (va_arg(arg,int)));
	if (res>=0)
		return res;
	errno = -res;
	return -1;
}
