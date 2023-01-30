#include "boot.h"

#define SECTSIZE 512
#define PT_LOAD 1
#define NULL 0

void *memcpy(void *dst, const void *src, unsigned int n)
{
    if (dst == NULL || src == NULL || n <= 0)
        return NULL;

    char * pdst = (char *)dst;
    char * psrc = (char *)src;
	while (n--)
		*pdst++ = *psrc++;

    return dst;
}

void *memset(void *s,unsigned int c, unsigned int n)
{
	if (NULL == s || n < 0)
		return NULL;
	char * tmpS = (char *)s;
	while(n-- > 0)
		*tmpS++ = c;
	return s; 
}

void bootMain(void) {
	int i = 0;
	//int phoff = 0x34;
	//int offset = 0x1000;
	unsigned int elf = 0x100000;
	void (*kMainEntry)(void);
	kMainEntry = (void(*)(void))0x100000;

	for (i = 0; i < 200; i++) {
		readSect((void*)(elf + i*512), 1+i);
	}

	// TODO: 填写kMainEntry、phoff、offset...... 然后加载Kernel（可以参考NEMU的某次lab）
	
	// 先通过readSect函数把ELF文件整体读入固定的位置。
	// 找到ELF可执行文件的ELF头。
	// 通过ELF可执行文件的头，找到程序头表的位置，并获知表项的多少（实际上程序头表就是个结构数组）。
	// 将type为LOAD的段装载到合适的位置上去。（注意，不要覆盖掉还未装载的段所占据的内存！！！）

	
	//ELF头
	struct ELFHeader* elfhdr =  ((struct ELFHeader*)elf);
	kMainEntry = (void(*)(void))(elfhdr->entry);
	//phoff = elfhdr->phoff;
	struct ProgramHeader *ph, *eph;
	ph = (void *)elfhdr + elfhdr->phoff;
	//offset = ph->off;
    eph = ph + elfhdr->phnum;

	for (; ph < eph; ph++)
	{
		if (ph->type == PT_LOAD)
		{
			//若Type为LOAD，则ELF文件中从文件Offset开始位置，连续FileSiz个字节的内容需要被装载
			//装载到内存VirtAddr开始，连续MemSiz个字节的区域中

            memcpy((void*)ph->vaddr,(void*)elf + ph->off,ph->filesz);
			
			//MemSiz可能大于FileSiz
            if(ph->memsz>ph->filesz)
            memset((void*)(ph->vaddr+ph->filesz),0,ph->memsz-ph->filesz);
		}
	}

	kMainEntry();
}

void waitDisk(void) { // waiting for disk
	while((inByte(0x1F7) & 0xC0) != 0x40);
}

void readSect(void *dst, int offset) { // reading a sector of disk
	int i;
	waitDisk();
	outByte(0x1F2, 1);
	outByte(0x1F3, offset);
	outByte(0x1F4, offset >> 8);
	outByte(0x1F5, offset >> 16);
	outByte(0x1F6, (offset >> 24) | 0xE0);
	outByte(0x1F7, 0x20);

	waitDisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = inLong(0x1F0);
	}
}
