#define __LIBRARY__
#include <unistd.h>

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-12 09:36:34 PM
 * 
 * _syscall0 is a macro, setsid will be expanded to:
 * pid_t setsid();
 *
 * this function is used to start a new session.
 */
_syscall0(pid_t,setsid)
