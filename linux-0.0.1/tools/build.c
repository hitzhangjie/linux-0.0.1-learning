#include <stdio.h>	/* fprintf */
#include <stdlib.h>	/* contains exit */
#include <sys/types.h>	/* unistd.h needs this */
#include <unistd.h>	/* contains read/write */
#include <fcntl.h>

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-12 03:59:14 PM
 * 
 * linux had some relations with minix, another story ...
 * firstly, let's see the details of struct exec, maybe this struct is not
 * the same with the header in 'boot', fixme later:
 * struct exec {
 * unsigned long a_magic; // Use macros N_MAGIC, etc for access
 * unsigned a_text;       // length of text, in bytes
 * unsigned a_data;       // length of data, in bytes
 * unsigned a_bss;        // length of uninitialized data area for file, in bytes
 * unsigned a_syms;       // length of symbol table data in file, in bytes
 * unsigned a_entry;      // start address
 * unsigned a_trsize;     // length of relocation info for text, in bytes
 * unsigned a_drsize;     // length of relocation info for data, in bytes
 * };
 *
 * now, we can provide all unsigned int\int\unsigned long are all 4-bytes long.
 * so, it takes up 32 bytes in total which is equal to MINIX_HEADER.
 */
#define MINIX_HEADER 32
/**
 * hit.zhangjie@gmail.com 
 * 2015-04-12 04:05:50 PM
 * 
 * GCC_HEADER, this struct will be explained later, i haven't checked it out
 * so far.
 */
#define GCC_HEADER 1024

void die(char * str)
{
	fprintf(stderr,"%s\n",str);
	exit(1);
}

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-12 03:43:55 PM
 * 
 * this file is used to generate the tool to build the boot system.
 */
void usage(void)
{
	die("Usage: build boot system [> image]");
}

/**
 * hit.zhangjie@gmail.com 
 * 2015-04-12 04:25:15 PM
 * 
 * this tool 'build' is use to generate the file 'image', but it depends on
 * the existed file 'tool' and 'system', where are the 2 files? is it
 * retrieved from this teaching OS, i.e, MINIX?
 * is my guess right ? fixme.
 */
int main(int argc, char ** argv)
{
	int i,c,id;
	char buf[1024];

	if (argc != 3)
		usage();
	/* initialize the buffer to 0 */
	for (i=0;i<sizeof buf; i++) buf[i]=0;

	/* check info of file 'boot' */
	if ((id=open(argv[1],O_RDONLY,0))<0)
		die("Unable to open 'boot'");
	if (read(id,buf,MINIX_HEADER) != MINIX_HEADER)
		die("Unable to read header of 'boot'");
	/* 
	 * minix header info should start with 0x04100301, that is the a_magic of
	 * struct exec 
	 */
	if (((long *) buf)[0]!=0x04100301)
		die("Non-Minix header of 'boot'");
	/* minix header text segment should be 0x20 */
	if (((long *) buf)[1]!=MINIX_HEADER)
		die("Non-Minix header of 'boot'");
	/* minix header data segment should be 0x0 */
	if (((long *) buf)[3]!=0)
		die("Illegal data segment in 'boot'");
	/* minix header bss segment should be 0x0 */
	if (((long *) buf)[4]!=0)
		die("Illegal bss in 'boot'");
	/* ... */
	if (((long *) buf)[5] != 0)
		die("Non-Minix header of 'boot'");
	/* ... */
	if (((long *) buf)[7] != 0)
		die("Illegal symbol table in 'boot'");
	/* read following info in 'boot' whose size should be less than 510 bytes */
	i=read(id,buf,sizeof buf);
	fprintf(stderr,"Boot sector %d bytes.\n",i);
	if (i>510)
		die("Boot block may not exceed 510 bytes");
	/* disk first sector terminating flags, 0x55AA */
	buf[510]=0x55;
	buf[511]=0xAA;
	/* write it to stdout */
	i=write(1,buf,512);
	if (i!=512)
		die("Write call failed");
	close (id);
	
	/* check info of file 'system' */
	if ((id=open(argv[2],O_RDONLY,0))<0)
		die("Unable to open 'system'");
	if (read(id,buf,GCC_HEADER) != GCC_HEADER)
		die("Unable to read header of 'system'");
	if (((long *) buf)[5] != 0)
		die("Non-GCC header of 'system'");
	for (i=0 ; (c=read(id,buf,sizeof buf))>0 ; i+=c )
		/* write 'system' file info stdout */
		if (write(1,buf,c)!=c)
			die("Write call failed");
	close(id);
	fprintf(stderr,"System %d bytes.\n",i);

	return(0);
}
