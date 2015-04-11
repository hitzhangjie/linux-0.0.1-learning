#define __LIBRARY__
#include <unistd.h>
#include <time.h>

/*
 * we need this inline - forking from kernel space will result
 * in NO COPY ON WRITE (!!!), until an execve is executed. This
 * is no problem, but for the stack. This is handled by not letting
 * main() use the stack at all after fork(). Thus, no function
 * calls - which means inline code for fork too, as otherwise we
 * would use the stack upon exit from 'fork()'.
 *
 * Actually only pause and fork are needed inline, so that there
 * won't be any messing with the stack from main(), but we define
 * some others too.
 */

/**
 * hit.zhangjie@gmail.com
 * 2015-04-10 04:35:28 PM
 * 
 * 1) static modifier, changes the functions' linkage to file-linkage, i.e,
 * you'cant see it in other files.
 * 2) _syscall0 is a macro which is defined within file /include/unistd.h.
 * for example, _syscall0(int, fork) will be expanded to:
 *
 *	int fork(void) {
 *		int __res;
 *		__asm__ volatile (
 *			"int $0x80"
 *			: "=a" (__res) \
 *			: "0" (__NR_##fork));
 *		if(__res >= 0) 
 *			return __res;
 *		errno = -__res;
 *		return -1;
 *	}
 *
 */

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-10 05:28:17 PM
 * 
 * here, we defined 4 system calls, including:
 * int fork(void);
 * int pause(void);
 * int setup(void);
 * int sync(void);
 *
 * 1) fork, fork current process to create a new process in which an
 * exec-family function will be called to execute another program.
 * 2) pause, will pause calling process or thread, until another process send
 * a signal to it. maybe it will will catch a kill signal or another signal.
 * 3) setup, this function will only be called once in kernel startup,
 * user-space programme can't call this function. even though it has root
 * privilege, it can't do this, too. this function is used to configure
 * something for the devices and file systems.
 * 4) sync, is used to commit buffer cache to the disk.
 *
 */
static inline _syscall0(int,fork)
static inline _syscall0(int,pause)
static inline _syscall0(int,setup)
static inline _syscall0(int,sync)

#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <asm/system.h>
#include <asm/io.h>

#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include <linux/fs.h>

static char printbuf[1024];

extern int vsprintf();
extern void init(void);
extern void hd_init(void);
extern long kernel_mktime(struct tm * tm);
extern long startup_time;

/*
 * Yeah, yeah, it's ugly, but I cannot find how to do this correctly
 * and this seems to work. I anybody has more info on the real-time
 * clock I'd be interested. Most of this was trial and error, and some
 * bios-listing reading. Urghh.
 */

#define CMOS_READ(addr) ({ \
outb_p(0x80|addr,0x70); \
inb_p(0x71); \
})

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

static void time_init(void)
{
	struct tm time;

	do {
		time.tm_sec = CMOS_READ(0);
		time.tm_min = CMOS_READ(2);
		time.tm_hour = CMOS_READ(4);
		time.tm_mday = CMOS_READ(7);
		time.tm_mon = CMOS_READ(8)-1;
		time.tm_year = CMOS_READ(9);
	} while (time.tm_sec != CMOS_READ(0));
	BCD_TO_BIN(time.tm_sec);
	BCD_TO_BIN(time.tm_min);
	BCD_TO_BIN(time.tm_hour);
	BCD_TO_BIN(time.tm_mday);
	BCD_TO_BIN(time.tm_mon);
	BCD_TO_BIN(time.tm_year);
	startup_time = kernel_mktime(&time);
}

void main(void)		/* This really IS void, no error here. */
{			/* The startup routine assumes (well, ...) this */
/*
 * Interrupts are still disabled. Do necessary setups, then
 * enable them
 */
	time_init();
	tty_init();
	trap_init();
	sched_init();
	buffer_init();
	hd_init();
	sti();
	move_to_user_mode();
	if (!fork()) {		/* we count on this going ok */
		/**
		 * hit.zhangjie@gmail.com 
		 * 2015-04-11 04:59:59 PM
		 * 
		 * child process exec init function, too many times fork() invoked?
		 * why do that? fixme.
		 */
		init();
	}
/*
 *   NOTE!!   For any other task 'pause()' would mean we have to get a
 * signal to awaken, but task0 is the sole exception (see 'schedule()')
 * as task 0 gets activated at every idle moment (when no other tasks
 * can run). For task0 'pause()' just means we go check if some other
 * task can run, and if not we return here.
 */
	for(;;) pause();
}

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-11 03:35:36 PM
 * 
 * printf is implemented based on vsprintf
 */
static int printf(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	write(1,printbuf,i=vsprintf(printbuf, fmt, args));
	va_end(args);
	return i;
}

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-11 04:02:57 PM
 * 
 * default argv/envp configuration
 */
static char * argv[] = { "-",NULL };
static char * envp[] = { "HOME=/usr/root", NULL };

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-11 04:02:37 PM
 * 
 * init function definition
 */
void init(void)
{
	int i,j;

	setup();
	if (!fork())
		/**
		 * hit.zhangjie@gmail.com 
		 * 2015-04-11 04:01:04 PM
		 * 
		 * child process execute /bin/update to do what ? fixme.
		 */
		_exit(execve("/bin/update",NULL,NULL));

	/* parent process */
	(void) open("/dev/tty0",O_RDWR,0);         /* stdin  */
	(void) dup(0);                             /* stdout */
	(void) dup(0);                             /* stderr */
	printf("%d buffers = %d bytes buffer space\n\r",NR_BUFFERS,
		NR_BUFFERS*BLOCK_SIZE);
	printf(" Ok.\n\r");
	if ((i=fork())<0)
		printf("Fork failed in init\r\n");
	else if (!i) {                             /* child process  */
		close(0);close(1);close(2);            /* close tty0 fds */
		setsid();                              /* run program in a new session */
		(void) open("/dev/tty0",O_RDWR,0);     /* reopen         */
		(void) dup(0);
		(void) dup(0);
		/**
		 * hit.zhangjie@gmail.com 
		 * 2015-04-11 04:00:28 PM
		 * 
		 * child process execute shell for user-machine interaction.
		 */
		_exit(execve("/bin/sh",argv,envp));
	}
	j=wait(&i);
	printf("child %d died with code %04x\n",j,i);
	sync();
	_exit(0);	/* NOTE! _exit, not exit() */
}
