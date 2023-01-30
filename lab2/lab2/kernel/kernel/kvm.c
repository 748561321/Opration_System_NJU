#include "x86.h"
#include "device.h"

#define PT_LOAD 1
SegDesc gdt[NR_SEGMENTS];       // the new GDT, NR_SEGMENTS=7, defined in x86/memory.h
TSS tss;

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


//init GDT and LDT
void initSeg() { // setup kernel segements
	gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_KERN);
	gdt[SEG_KDATA] = SEG(STA_W,         0,       0xffffffff, DPL_KERN);
	//gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_USER);
	gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0x00200000,0x000fffff, DPL_USER);
	//gdt[SEG_UDATA] = SEG(STA_W,         0,       0xffffffff, DPL_USER);
	gdt[SEG_UDATA] = SEG(STA_W,         0x00200000,0x000fffff, DPL_USER);
	gdt[SEG_TSS] = SEG16(STS_T32A,      &tss, sizeof(TSS)-1, DPL_KERN);
	gdt[SEG_TSS].s = 0;
	setGdt(gdt, sizeof(gdt)); // gdt is set in bootloader, here reset gdt in kernel

	/*
	 * 初始化TSS
	 */
	tss.esp0 = 0x1fffff;
	tss.ss0 = KSEL(SEG_KDATA);
	asm volatile("ltr %%ax":: "a" (KSEL(SEG_TSS)));

	/*设置正确的段寄存器*/
	asm volatile("movw %%ax,%%ds":: "a" (KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax,%%es":: "a" (KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax,%%fs":: "a" (KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax,%%gs":: "a" (KSEL(SEG_KDATA)));
	asm volatile("movw %%ax,%%ss":: "a" (KSEL(SEG_KDATA)));

	lLdt(0);
	
}

void enterUserSpace(uint32_t entry) {
	/*
	 * Before enter user space 
	 * you should set the right segment registers here
	 * and use 'iret' to jump to ring3
	 */
	uint32_t EFLAGS = 0;
	asm volatile("pushl %0":: "r" (USEL(SEG_UDATA))); // push ss
	asm volatile("pushl %0":: "r" (0x2fffff)); 
	asm volatile("pushfl"); //push eflags, sti
	asm volatile("popl %0":"=r" (EFLAGS));
	asm volatile("pushl %0"::"r"(EFLAGS|0x200));
	asm volatile("pushl %0":: "r" (USEL(SEG_UCODE))); // push cs
	asm volatile("pushl %0":: "r" (entry)); 
	asm volatile("iret");
}

/*
kernel is loaded to location 0x100000, i.e., 1MB
size of kernel is not greater than 200*512 bytes, i.e., 100KB
user program is loaded to location 0x200000, i.e., 2MB
size of user program is not greater than 200*512 bytes, i.e., 100KB
*/

void loadUMain(void) {
	// TODO: 参照bootloader加载内核的方式，具体加载到哪里请结合手册提示思考！
	
	int i = 0;
	//int phoff = 0x0;
	//int offset = 0x0;
	uint32_t elf = 0x200000;
	uint32_t uMainEntry = 0;//read to 0x200000

	for (i=0;i<200;i++)
	{
		readSect((void*)(elf+i*512),201+i);
	}

	//ELF头
	struct ELFHeader* elfhdr =  ((struct ELFHeader*)elf);
	uMainEntry = (uint32_t)elfhdr->entry;
	//phoff = elfhdr->phoff;
	struct ProgramHeader *ph, *eph;
	//程序头表
	ph = (ProgramHeader *)((unsigned int)elfhdr + elfhdr->phoff);
	//offset = ph->off;
	//表项的多少
    eph = ph + elfhdr->phnum;

	for (; ph < eph; ph++)
	{
		if (ph->type == PT_LOAD)
		{
			//若Type为LOAD，则ELF文件中从文件Offset开始位置，连续FileSiz个字节的内容需要被装载
			//装载到内存VirtAddr开始，连续MemSiz个字节的区域中
            memcpy((void*)ph->vaddr,(void*)elf+ph->off,ph->filesz);
			
			//MemSiz可能大于FileSiz
            if(ph->memsz>ph->filesz)
            memset((void*)(ph->vaddr+ph->filesz),0,ph->memsz-ph->filesz);
		}
	}


	enterUserSpace(uMainEntry);
	
}
