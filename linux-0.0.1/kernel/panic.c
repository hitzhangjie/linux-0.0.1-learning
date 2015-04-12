/*
 * This function is used through-out the kernel (includeinh mm and fs)
 * to indicate a major problem.
 */
#include <linux/kernel.h>

volatile void panic(const char * s)
{
	printk("Kernel panic: %s\n\r",s);
	/**
	 * hit.zhangjie@gmail.com 
	 * 2015-04-12 08:52:36 PM
	 * 
	 * oh, my god, now i know why the OS freezes. a infinite loop is used.
	 * maybe when system crash, panic is a solution, because the system works
	 * like freezing, then we can check the error msg printed to the console
	 * to analyze what happened in the past dark seconds.
	 */
	for(;;);
}
