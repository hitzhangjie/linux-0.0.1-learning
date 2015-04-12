#define __LIBRARY__
#include <unistd.h>

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-12 09:17:07 PM
 * 
 * _syscall3 is a macro, execve will be expanded to:
 * int execve(const char *file, char **argv, char **envp);
 *
 * execve is used to execute another program file.
 */
_syscall3(int,execve,const char *,file,char **,argv,char **,envp)
