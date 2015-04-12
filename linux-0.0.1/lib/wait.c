#define __LIBRARY__
#include <unistd.h>
#include <sys/wait.h>

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-12 09:39:18 PM
 * 
 * _syscall3 is a macro, waitpid will be expanded to:
 * pid_t waitpid(pid_t pid, int *wait_stat, int options);
 *
 * this function is used to suspend the calling process until the process's
 * state has changed, which is specified by param 'pid'.
 *
 * if pid_t is -1, then all its child processes will be the target that
 * calling process waits on.
 */
_syscall3(pid_t,waitpid,pid_t,pid,int *,wait_stat,int,options)

pid_t wait(int * wait_stat)
{
	return waitpid(-1,wait_stat,0);
}
