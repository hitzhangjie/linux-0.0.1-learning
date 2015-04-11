#ifndef _HEAD_H
#define _HEAD_H

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-11 11:37:09 AM
 * 
 * here, desc_struct is defined, it's the data type that will be used to
 * construct the gdt and ldt.
 *
 * desc_struct only contains two unsigned long fields a and b, in 80386, this
 * construct contains 8 bytes, so our previous guess about the size of element
 * in gdt/ldt is right.
 * here only unsigned long fields a,b is displayed, maybe the 2 long datatypes
 * is used more tricky, for example, use bitfields to store more detailed info.
 *
 * and here, desc_table is defined, too, it is an array of which each element
 * is a desc_struct. but, it only contains 256 elements, it is much smaller
 * compared with MaoDecao said.
 */
typedef struct desc_struct {
	unsigned long a,b;
} desc_table[256];

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-11 11:43:24 AM
 * 
 * linux, this operating system uses page to manage memory, but 80386 uses
 * segment and pages to manage memory because of the intel arch's history
 * reasons. linux has too adapt to the 80386 arch, on this aspect, linux
 * should supply both segment and page memory manage. but actually, linux just
 * play a joke on 80386. 
 * 1) after we write a program in c programming language, then we compile/ld
 * it, we disassemble it using objdump, we can see the first column contains
 * hexdecimals, these hexdecimals are 'virtual mem address' given by ld.
 * 2) then, linux enter 'segment mode transform', the 'virtual mem address'
 * will be transformed to 'linear mem address', why called it 'linear'
 * address? it is caused by the transform algorithm is 'linear'. 80386 uses
 * GDTR to reference the gdt, uses CS/DS to supply the index of gdt. before
 * loads and run one program, linux need to specify the value of some
 * registers. these problem is associated with process schedule.
 *
 * firstly, read following macro definition:
 * #define start_thread(regs, new_eip, new_esp) do {     \
 *    __asm__ ("movl %0, %%fs; movl %0, %%gs": :"r"(0)); \
 *    regs->xds = __USER_DS;                             \
 *    regs->xes = __USER_DS;                             \
 *    regs->xss = __USER_DS;                             \
 *    regs->xss = __USER_DS;                             \
 *    regs->xcs = __USER_CS;                             \
 *    regs->eip = new_eip;                               \
 *    regs->esp = new_esp;                               \
 * }while(0)
 *
 * xds,xes,xss... is the shadow of ds,es,ss...
 * we can see, ds,es,ss are set to __USER_DS, cs is set to __USER_CS. why
 * ds,es,ss are all set to __USER_DS rather than __USER_DS, __USER_ES,
 * __USER_SS, because linux didn't conform to intel design goal. in segment
 * mode mem management, intel want us to separate ds\es\ss\cs segment by set
 * different values for each segment registers. but linux didn't do as intel
 * designed. in segment mode mem management, in linux, stack segment and data
 * segment aren't separated. 
 *
 * linux only had 4 segment registers values:
 * __KERNEL_CS 0x10
 * __KERNEL_DS 0x18
 * __USER_CS   0x23
 * __USER_DS   0x2B
 *
 * let's convert __KERNEL_CS\__KERNEL_DS\__USER_CS\__USER_DS value into binary
 * format:
 * |----------------Index---------------|-TI-|-RPL-|
 * |------------------------------------|----|-----|
 * | __KERNEL_CS 0x10  0000 0000 0001 0 | 0  | 00  |
 * | __KERNEL_DS 0x18  0000 0000 0001 1 | 0  | 00  |
 * | __USER_CS   0x23  0000 0000 0010 0 | 0  | 11  |
 * | __USER_DS   0x2B  0000 0000 0010 1 | 0  | 11  |
 * |------------------------------------|----|-----|
 *
 * also, we list segment register field details:
 * |----------------Index---------------|-TI-|-RPL-|
 * 15                                   3  | 2  1 0
 *                                         |    |--> 00: requested ring0 level
 *                                         |         11: requested ring3 level
 *                                         |
 *                                         |--> 0: using GDT
 *                                              1: using LDT
 *
 * now we can see, __KERNEL_CS\__KERNEL_DS\__USER_CS\__USER_DS, their TI value
 * are TI=0, so they use GDT rather than LDT. __KERNEL_CS\__KERNEL_DS, their
 * RPL=00, so ring0 privilege is requested, while __USER_CS\__USER_DS RPL=11,
 * ring3 privilege is requested.
 * now we have explained segment registers' value, now we check gdt elements.
 *
 *
 * gdt[0]\gdt[1] are not used by linux kernel, gdt[2]\[3]\[4]\[5] are special,
 * gdt[2]\[3] are used for kernel's cs\ds registers, gdt[4]\[5] are used for
 * current running task's cs\ds registers.
 *
 *              |B31-24 |    |L19-16|         |B23-16 |B15-0    |L15-0   |
 * gdt[2] K_CS: |0.....0|1100| 1111 |1001 1010|0.....0|0.......0|1......1|
 * gdt[3] K_DS: |0.....0|1100| 1111 |1001 0010|0.....0|0.......0|1......1|
 * gdt[4] U_CS: |0.....0|1100| 1111 |1111 1010|0.....0|0.......0|1......1|
 * gdt[5] U_DS: |0.....0|1100| 1111 |1111 0010|0.....0|0.......0|1......1|
 * 
 * L19-0, defines the segment limit to avoid memory access violation.
 * B31-0, defines the segment base address.
 * we know, kernel process\user process's segment base address are both 0x0,
 * and segment length are both 2^20=1M.
 *
 * if in our program, there's one instruction 'call ADDRESS_VALUE', here,
 * ${ADDRESS_VALUE} is the 'virtual address', it'll be transformed to linear
 * address. how to do it? first, check gdt[2] to get current running process's
 * code segment base address B31-0, then compare the (${ADDRESS_VALUE})-(B31-0)
 * and (L15-0), if the offset is less than L15-0, it indicates mem access is
 * legal. so the linear address will be (B31-0)+${ADDRESS_VALUE}.
 * actually, it is just used to adapt to 80386 arch implementation. if we use
 * other cpu arch which just has page mode mem manangement, the
 * gdtr\ldtr\cs\ds... this registers are all useless and unneeded.
 *
 * also, if we execute 'objdump -h program', we can see 2 columns, one is
 * VMA(virtual mem address), one is LMA(logical mem address or linear mem
 * address). why it can display LMA before segment mode transform beginning?
 * ah, we analyze the process above, so you should understand that.
 *
 * here i mentioned above is based on linux kernel 2.6 rather than 0.0.1, i
 * just want to list it here to help us understand much more easily.
 */
extern unsigned long pg_dir[1024];
extern desc_table idt,gdt;

#define GDT_NUL 0
#define GDT_CODE 1
#define GDT_DATA 2
#define GDT_TMP 3

#define LDT_NUL 0
#define LDT_CODE 1
#define LDT_DATA 2

#endif
