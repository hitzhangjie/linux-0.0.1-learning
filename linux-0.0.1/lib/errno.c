/**
 * hit.zhangjie@gmail.com 
 * 2015-04-12 09:14:31 PM
 * 
 * errno is the error number, errno is positive number, from 1 to 255.
 * when we execute system call, if return value is -1, then it indicates some
 * error has occured, then we should check the errno's value.
 * if system call is success, we should neglect errno.
 *
 * now errno is set to global variable, actually, in multi-thread environment,
 * errno should be kept private to each thread.
 */
int errno;
