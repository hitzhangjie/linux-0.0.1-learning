#define __LIBRARY__
#include <unistd.h>

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-12 09:20:32 PM
 * 
 * _exit function doesn't return a value.
 */
volatile void _exit(int exit_code)
{
	/**
	 * hit.zhangjie@gmail.com 
	 * 2015-04-12 09:21:36 PM
	 * 
	 * __NR_exit is defined as a system call.
	 */
	__asm__("int $0x80"::"a" (__NR_exit),"b" (exit_code));
}
