#ifndef _SCHED_H
#define _SCHED_H

#define NR_TASKS 64
#define HZ 100

#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS-1]

#include <linux/head.h>
#include <linux/fs.h>
#include <linux/mm.h>

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-10 06:10:41 PM
 * 
 * this test macro is used to control how many files can be opened per process.
 * when one file descriptor is marked close-on-exec flag, close(fd) will be
 * automatically called after current process or current child process calls
 * exec* function successfully.
 * and here, because, the close-on-exec flags is stored in one word, one word
 * in 80386 has 32 bits, every flag takes a bit, so we one process can only
 * open 32 files.
 */
#if (NR_OPEN > 32)
#error "Currently the close-on-exec-flags are in one word, max 32 files/proc"
#endif

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-10 06:15:10 PM
 * 
 * defines task' states.
 * what's the differences between task' states and process's states.
 * fixme.
 */
#define TASK_RUNNING		0
#define TASK_INTERRUPTIBLE	1
#define TASK_UNINTERRUPTIBLE	2
#define TASK_ZOMBIE		3
#define TASK_STOPPED		4

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-10 06:18:50 PM
 * 
 * sometimes we mis-write NULL as null, now we can remember why (void *)0 is
 * NULL rather than null, now!
 */
#ifndef NULL
#define NULL ((void *) 0)
#endif

extern int copy_page_tables(unsigned long from, unsigned long to, long size);
extern int free_page_tables(unsigned long from, long size);

extern void sched_init(void);
extern void schedule(void);
extern void trap_init(void);
extern void panic(const char * str);
extern int tty_write(unsigned minor,char * buf,int count);

typedef int (*fn_ptr)();

struct i387_struct {
	long	cwd;
	long	swd;
	long	twd;
	long	fip;
	long	fcs;
	long	foo;
	long	fos;
	long	st_space[20];	/* 8*10 bytes for each FP-reg = 80 bytes */
};

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-10 06:21:14 PM
 * 
 * in 80386, to implement 'protective memory access mode', intel adds some
 * other registers to achieve this.
 * 1) GDTR, short for global descriptor table register, which store a base
 * address which points to an array in which every element is a special struct.
 * this special struct contains segment details, including value of
 * cs or ds, the length of this segment, and the privelege to access this
 * segment.
 * 2) LDTR, short for local descriptor table register, which function
 * similarly to the GDTR. GDTR is shared between kernel and all processes,
 * while LDTR belongs to one specific process.
 * also, in linux, processes don't use LDTR, it's only used in vm86 model, for
 * example, when we use WINE to run some windows applications, LDTR will be
 * used.
 * 3) CS/DS..., now in protective model, value of CS/DS... is not the actual
 * value of code segment/data segment..., the value in them is just the index
 * of the array pointed by the value of GDTR.
 * for example, the struct pointed by (GDTR)[(CS)] is the actual segment
 * details, from this struct, we can get segment's base address and length.
 * combined with IP, we can take instructions from the code segment. 
 * also, notice that, LGDT\SGDT are privilege instructions, any other
 * instruction can't access/modify their value, because of this basic
 * and other mechanisms, protective memory access model is achieved.
 *
 * in GDT, here, GDT is the array pointed by (GDTR). every element in it is
 * either an LDTR, or a task struct. here, terminology 'task' refers to
 * 'process'.
 * every process has 2 structs in the GDT, one is tss struct(tss, short for
 * Task State Segment), one is LDTR whose value points LDT owned by process.
 * GDT[0]\[1] is cleared to zero, it's useless.
 * GDT[2]\[3] is used to store kernel's CS\DS.
 * GDT[4]\[5] is used to store current running task's CS\DS.
 * so you can calculate how many tasks will be supported in 80386 in theory.
 * 4000+!
 *
 * here, we encounter the tss struct definition.
 *
 * we see, it contains some register fields, it is easy to understand, these
 * registers can be used to describe process's running state.
 *
 */
struct tss_struct {
	long	back_link;	/* 16 high bits zero */
	long	esp0;
	long	ss0;		/* 16 high bits zero */
	long	esp1;
	long	ss1;		/* 16 high bits zero */
	long	esp2;
	long	ss2;		/* 16 high bits zero */
	long	cr3;		/**
						 * hit.zhangjie@gmail.com 
						 * 2015-04-10 06:43:35 PM
						 *
						 * cr3 contains the base address points to the Page 
						 * Category Table 
						 */
	long	eip;
	long	eflags;
	long	eax,ecx,edx,ebx;
	long	esp;
	long	ebp;
	long	esi;
	long	edi;
	long	es;		/* 16 high bits zero */
	long	cs;		/* 16 high bits zero */
	long	ss;		/* 16 high bits zero */
	long	ds;		/* 16 high bits zero */
	long	fs;		/* 16 high bits zero */
	long	gs;		/* 16 high bits zero */
	long	ldt;	/* 16 high bits zero */
					/**
					 * hit.zhangjie@gmail.com
					 * 2015-04-10 06:44:29 PM 
					 * 
					 * why ldt is contained here, too? it is already contained
					 * in GDT! ah, only in vm86 model, LDT is used in GDT.
					 * linux won't store LDT in GDT, because it stores ldt in
					 * tss. am i right ?
					 * fixme
					 */
	long	trace_bitmap;	/* bits: trace 0, bitmap 16-31 */
	struct i387_struct i387;
};

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-10 06:50:47 PM
 * 
 * to run a program, we must create a PCB for it. PCB not means Print Circuit
 * Board, it is short for Process Control Block.
 * and, the PCB, actually, refers to the task_struct defined below.
 */
struct task_struct {
/* these are hardcoded - don't touch */
	long state;				/* -1 unrunnable, 0 runnable, >0 stopped */
	long counter;
	long priority;
	long signal;
	fn_ptr sig_restorer;
	fn_ptr sig_fn[32];		/**
							 * hit.zhangjie@gmail.com
							 * 2015-04-10 06:54:04 PM
							 *
							 * maybe, only 32 signal handlers can be registered 
							 */
/* various fields */
	int exit_code;
	/**
	 * hit.zhangjie@gmail.com 
	 * 2015-04-10 06:58:04 PM
	 * 
	 * here, i will try to describe the memory layout.
	 *
	 * |                          |
	 * | code segment             |
	 * |--------------------------|
	 * | data    | .data          |
	 * | segment |----------------|
	 * |         | .bss           |
	 * |---------|----------------|
	 * |         |                |
	 * |         |                |
	 * | heap    |---brk----------|
	 * |         |                |
	 * |         //               |
	 * |--------------------------|
	 * |         |                |
	 * |         | vars/others    |
	 * |         |                |
	 * |         |----------------|
	 * | stack   | pushed %ebp    |
	 * | segment |----------------|
	 * |         | return address |
	 * |         |----------------|
	 * |         | args           |
	 * |--------------------------|
	 */

	unsigned long end_code; /* the end address of code segment */
	unsigned long end_data; /* the end address of data segment */
	unsigned long brk;      /* the current brk edge            */
	unsigned long start_stack;  /* the start address of stack  */
	/**
	 * hit.zhangjie@gmail.com 
	 * 2015-04-10 07:09:58 PM
	 * 
	 * maybe we should review process/process group relevant concepts.
	 */
	long pid,father,pgrp,session,leader;
	unsigned short uid,euid,suid;
	unsigned short gid,egid,sgid;
	long alarm;
	/**
	 * hit.zhangjie@gmail.com 
	 * 2015-04-10 07:10:42 PM
	 * 
	 * profile the process's execution time details.
	 */
	long utime,stime,cutime,cstime,start_time;
	unsigned short used_math;
/* file system info */
	int tty;		/* -1 if no tty, so it must be signed */
	unsigned short umask;
	struct m_inode * pwd;
	struct m_inode * root;
	/**
	 * hit.zhangjie@gmail.com 
	 * 2015-04-10 07:11:49 PM
	 * 
	 * set close_on_exec flags on opened files in filp[NR_OPEN], NR_OPEN's
	 * max value is 32, because in 80386, long is 32bit.
	 */
	unsigned long close_on_exec;
	struct file * filp[NR_OPEN];
	/**
	 * hit.zhangjie@gmail.com 
	 * 2015-04-10 07:14:34 PM
	 * 
	 * ldt, ldt[0] is cleared to zero, and it's useless, ldt[1] is used to
	 * store current task's CS value, ldt[2] is used to store DS/SS value.
	 */
/* ldt for this task 0 - zero 1 - cs 2 - ds&ss */
	struct desc_struct ldt[3];
/* tss for this task */
	/**
	 * hit.zhangjie@gmail.com 
	 * 2015-04-10 07:18:18 PM
	 * 
	 * why ldt is stored both in task_struct and tss_struct? why? 
	 * fixme
	 */
	struct tss_struct tss;
};

/*
 *  INIT_TASK is used to set up the first task table, touch at
 * your own risk!. Base=0, limit=0x9ffff (=640kB)
 */
#define INIT_TASK \
/* state etc */	{ 0,15,15, \
/* signals */	0,NULL,{(fn_ptr) 0,}, \
/* ec,brk... */	0,0,0,0,0, \
/* pid etc.. */	0,-1,0,0,0, \
/* uid etc */	0,0,0,0,0,0, \
/* alarm */	0,0,0,0,0,0, \
/* math */	0, \
/* fs info */	-1,0133,NULL,NULL,0, \
/* filp */	{NULL,}, \
	{ \
		{0,0}, \
/* ldt */	{0x9f,0xc0fa00}, \ /* why initialized to this constanct, fixme */
		{0x9f,0xc0f200}, \
	}, \
/*tss*/	{0,PAGE_SIZE+(long)&init_task,0x10,0,0,0,0,(long)&pg_dir,\
	 0,0,0,0,0,0,0,0, \
	 0,0,0x17,0x17,0x17,0x17,0x17,0x17, \
	 _LDT(0),0x80000000, \     /* why initialized to this constanct, fixme */
		{} \
	}, \
}

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-10 07:28:44 PM
 * 
 * NR_TASKS is 64, we can create 64 processes at most.
 */
extern struct task_struct *task[NR_TASKS];
extern struct task_struct *last_task_used_math;
extern struct task_struct *current;
extern long volatile jiffies;
extern long startup_time;

#define CURRENT_TIME (startup_time+jiffies/HZ)

extern void sleep_on(struct task_struct ** p);
extern void interruptible_sleep_on(struct task_struct ** p);
extern void wake_up(struct task_struct ** p);

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-10 07:31:21 PM
 * 
 * from the comments by linus torvards, we know that, in linux, in gdt, a task
 * indeed need 2 entries, one for ldt, one for tss.
 * but usage of gdt is a little different from the description by MaoDecao,
 * who give insights into linux kernel 2.6. maybe the gdt's usage had been
 * changed.
 */

/*
 * Entry into gdt where to find first TSS. 0-nul, 1-cs, 2-ds, 3-syscall
 * 4-TSS0, 5-LDT0, 6-TSS1 etc ...
 */
#define FIRST_TSS_ENTRY 4
#define FIRST_LDT_ENTRY (FIRST_TSS_ENTRY+1)
/**
 * hit.zhangjie@gmail.com 
 * 2015-04-10 07:55:17 PM
 * 
 * every gdt element takes up 8 bytes. now you should understand why <<4 and
 * <<3 in following code.
 * _TSS(n), returns the bytes offset of the (n+1)th tss_struct in gdt, while
 * _LDT(n), returns the bytes offset of the (n+1)th ldt struct in gdt.
 */
#define _TSS(n) ((((unsigned long) n)<<4)+(FIRST_TSS_ENTRY<<3))
#define _LDT(n) ((((unsigned long) n)<<4)+(FIRST_LDT_ENTRY<<3))
/**
 * hit.zhangjie@gmail.com
 * 2015-04-10 07:50:40 PM 
 * 
 * load the (n+1)th tss or ldt in gdt.
 */
#define ltr(n) __asm__("ltr %%ax"::"a" (_TSS(n)))
#define lldt(n) __asm__("lldt %%ax"::"a" (_LDT(n)))
/**
 * hit.zhangjie@gmail.com 
 * 2015-04-10 08:07:51 PM
 * 
 * store current task into the (n+1)th element in gdt
 */
#define str(n) \
__asm__("str %%ax\n\t" \
	"subl %2,%%eax\n\t" \
	"shrl $4,%%eax" \
	:"=a" (n) \
	:"a" (0),"i" (FIRST_TSS_ENTRY<<3))
/*
 *	switch_to(n) should switch tasks to task n, first checking that n isn't
 *	the current task, in which case it does nothing.  This also clears the
 *	TS-flag if the task we switched to has used tha math co-processor latest.
 */
/**
 * hit.zhangjie@gmail.com 
 * 2015-04-10 07:57:06 PM
 * 
 * swith tasks. wow !!!
 *
 * i almost understand now, but some details i still need thinking more clearly.
 * fixme:
 * _current stores what?
 * entry in gdt stores what?
 */
#define switch_to(n) {\
struct {long a,b;} __tmp; \
__asm__("cmpl %%ecx,_current\n\t" \
	"je 1f\n\t" \
	"xchgl %%ecx,_current\n\t" \         /* exchange value              */
	"movw %%dx,%1\n\t" \
	"ljmp %0\n\t" \
	"cmpl %%ecx,%2\n\t" \
	"jne 1f\n\t" \
	"clts\n" \                           /* clears the task-switch flag */
	"1:" \
	::"m" (*&__tmp.a),"m" (*&__tmp.b), \
	"m" (last_task_used_math),"d" _TSS(n),"c" ((long) task[n])); \
}

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-11 12:59:36 AM
 * 
 * the (n+1)th page's base address
 */
#define PAGE_ALIGN(n) (((n)+0xfff)&0xfffff000)

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-11 01:11:39 AM 
 * 
 * haven't understood the following lines.
 */
#define _set_base(addr,base) \
__asm__("movw %%dx,%0\n\t" \
	"rorl $16,%%edx\n\t" \                  /* rotate right  */
	"movb %%dl,%1\n\t" \
	"movb %%dh,%2" \
	::"m" (*((addr)+2)), \
	  "m" (*((addr)+4)), \
	  "m" (*((addr)+7)), \
	  "d" (base) \
	:"dx")

#define _set_limit(addr,limit) \
__asm__("movw %%dx,%0\n\t" \
	"rorl $16,%%edx\n\t" \
	"movb %1,%%dh\n\t" \
	"andb $0xf0,%%dh\n\t" \
	"orb %%dh,%%dl\n\t" \
	"movb %%dl,%1" \
	::"m" (*(addr)), \
	  "m" (*((addr)+6)), \
	  "d" (limit) \
	:"dx")

#define set_base(ldt,base) _set_base( ((char *)&(ldt)) , base )
#define set_limit(ldt,limit) _set_limit( ((char *)&(ldt)) , (limit-1)>>12 )

#define _get_base(addr) ({\
unsigned long __base; \
__asm__("movb %3,%%dh\n\t" \
	"movb %2,%%dl\n\t" \
	"shll $16,%%edx\n\t" \
	"movw %1,%%dx" \
	:"=d" (__base) \
	:"m" (*((addr)+2)), \
	 "m" (*((addr)+4)), \
	 "m" (*((addr)+7))); \
__base;})

#define get_base(ldt) _get_base( ((char *)&(ldt)) )

#define get_limit(segment) ({ \
unsigned long __limit; \
__asm__("lsll %1,%0\n\tincl %0":"=r" (__limit):"r" (segment)); \
__limit;})

#endif
