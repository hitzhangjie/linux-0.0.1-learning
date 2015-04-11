/*
 * 'tty.h' defines some structures used by tty_io.c and some defines.
 *
 * NOTE! Don't touch this without checking that nothing in rs_io.s or
 * con_io.s breaks. Some constants are hardwired into the system (mainly
 * offsets into 'tty_queue'
 */

#ifndef _TTY_H
#define _TTY_H

#include <termios.h>

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-11 10:22:22 AM
 * 
 * tty buffer size is 1024.
 * buffer type: line buffer, full buffer, most of terminals are line-buffered
 * by default.
 */
#define TTY_BUF_SIZE 1024

struct tty_queue {
	unsigned long data;
	unsigned long head;
	unsigned long tail;
	
	/**
	 * hit.zhangjie@gmail.com 
	 * 2015-04-11 10:24:46 AM
	 * 
	 * tty_struct has read\write queue implemented by tty_queue linked lists.
	 * why in tty_queue struct, task_struct linked list is defined, what does
	 * it mean? 
	 * i think, each tty_queue represents a message that carry read\write info,
	 * should it contains a proc_list? i mean, the same read\write info can be
	 * shared among processes?
	 * fixme.
	 */
	struct task_struct * proc_list;
	char buf[TTY_BUF_SIZE];
};

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-11 10:48:34 AM
 * 
 * here, param 'a' is an index of tty_queue.buf, so INC(a) is used to
 * increment the index of buf, and DEC(a) is used to decrement the index of
 * buf, in order to traverse the buf.
 * notice that, because '& (TTY_BUF_SIZE-1)' is appended, INC(a)\DEC(a) won't
 * access memory violation, it will always be in the range 0~1023.
 */
#define INC(a) ((a) = ((a)+1) & (TTY_BUF_SIZE-1))
#define DEC(a) ((a) = ((a)-1) & (TTY_BUF_SIZE-1))
/**
 * hit.zhangjie@gmail.com 
 * 2015-04-11 10:51:46 AM
 * 
 * here, param 'a' is a tty_queue struct.
 *
 * before i state the function's purpose, i have to describe the ugly queue
 * data struct used by linus torvards.
 *
 * here's the buf:
 *    0 1 2 3 ...                        1023
 *    ---------------------------------------
 *    | | | | | | | | | | | | | | ...
 *    ---------------------------------------
 *    |                         |
 *    |->tail                   |->head
 *
 *    get data from tail, and put new data in head, it is just contrary as we
 *    learned before, we always put new data at the queue tail end, and get
 *    data from the queue head end. torvards, you're just a contrary man.
 *    if i implemented this circular queue, maybe i will use smaller index as
 *    the head ptr, and the bigger index as the tail index. and i put data use
 *    tail ptr, and get data using head ptr.
 *
 *    but torvards implemented it contrarily on the concept, but it worked.
 *
 * 1) EMPTY(a) is used to test whether the tty_queue.buf is empty,
 *    tty_queue.buf can be treated as a circular queue.
 * 2) LEFT(a) is used to test whether the circular buf has remained/left
 *    positions to hold new elements.
 * 3) LAST(a) is used to get the index of last element in circular buf.
 * 4) FULL(a) is used to test whether the circular buf is full.
 * 5) CHARS(a) is used to get how many chars is stored in this buf.
 * 6) GETCH/PUTCH is used to get/put element in the circular buf, here i want
 *    to ask linus torvards, what's the concept of queue head/tail, hahaha,
 * 7) EOF_CHAR\INTR_CHAR\STOP_CHAR\START_CHAR\ERASE_CHAR are used to get the
 *    relevant control character.
 *
 */
#define EMPTY(a) ((a).head == (a).tail)
#define LEFT(a) (((a).tail-(a).head-1)&(TTY_BUF_SIZE-1))
#define LAST(a) ((a).buf[(TTY_BUF_SIZE-1)&((a).head-1)])
#define FULL(a) (!LEFT(a))
#define CHARS(a) (((a).head-(a).tail)&(TTY_BUF_SIZE-1))
#define GETCH(queue,c) \
(void)({c=(queue).buf[(queue).tail];INC((queue).tail);})
#define PUTCH(c,queue) \
(void)({(queue).buf[(queue).head]=(c);INC((queue).head);})

#define EOF_CHAR(tty) ((tty)->termios.c_cc[VEOF])
#define INTR_CHAR(tty) ((tty)->termios.c_cc[VINTR])
#define STOP_CHAR(tty) ((tty)->termios.c_cc[VSTOP])
#define START_CHAR(tty) ((tty)->termios.c_cc[VSTART])
#define ERASE_CHAR(tty) ((tty)->termios.c_cc[VERASE])

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-11 10:29:54 AM
 * 
 * each tty_struct represents a tty device. it contains a read queue and write
 * queue. am i right? fixme
 * 
 */
struct tty_struct {
	struct termios termios;
	int pgrp;
	int stopped;
	void (*write)(struct tty_struct * tty);
	struct tty_queue read_q;
	struct tty_queue write_q;
	struct tty_queue secondary;
	};

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-11 10:29:21 AM
 * 
 * all ttys are stored in tty_table.
 */
extern struct tty_struct tty_table[];

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-11 11:28:56 AM
 * 
 * some common terminal control characters are defined in the following
 * comments body.
 */

/*	intr=^C		quit=^|		erase=del	kill=^U
	eof=^D		vtime=\0	vmin=\1		sxtc=\0
	start=^Q	stop=^S		susp=^Y		eol=\0
	reprint=^R	discard=^U	werase=^W	lnext=^V
	eol2=\0
*/
/**
 * hit.zhangjie@gmail.com 
 * 2015-04-11 11:34:01 AM
 * 
 * the following line defines the terminal control characters' ascii code in
 * octal system, for example, \003 reprensents ^C.
 */
#define INIT_C_CC "\003\034\177\025\004\0\1\0\021\023\031\0\022\017\027\026\0"

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-11 11:35:50 AM
 * 
 * other functions that'll be explained later.
 * fixme.
 */
void rs_init(void);
void con_init(void);
void tty_init(void);

int tty_read(unsigned c, char * buf, int n);
int tty_write(unsigned c, char * buf, int n);

void rs_write(struct tty_struct * tty);
void con_write(struct tty_struct * tty);

void copy_to_cooked(struct tty_struct * tty);

#endif
