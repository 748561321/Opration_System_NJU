#include "common.h"
#include "x86.h"
#include "device.h"

#define va_to_pa(va) (va + (current + 1) * 0x100000)
#define pa_to_va(pa) (pa - (current + 1) * 0x100000)

extern TSS tss;
extern int displayRow;
extern int displayCol;

extern ProcessTable pcb[MAX_PCB_NUM];
extern int current; // current process


void GProtectFaultHandle(struct StackFrame *sf);

void timerHandle(struct StackFrame *sf);

void syscallHandle(struct StackFrame *sf);

void syscallWrite(struct StackFrame *sf);
void syscallPrint(struct StackFrame *sf);
void syscallFork(struct StackFrame *sf);
void syscallExec(struct StackFrame *sf);
void syscallSleep(struct StackFrame *sf);
void syscallExit(struct StackFrame *sf);


void irqHandle(struct StackFrame *sf) { // pointer sf = esp
	/* Reassign segment register */
	asm volatile("movw %%ax, %%ds"::"a"(KSEL(SEG_KDATA)));
	
	/* Save esp to stackTop */
	//为了中断嵌套
	pcb[current].stackTop=(uint32_t)sf;

	switch(sf->irq) {
		case -1:
			break;
		case 0xd:
			GProtectFaultHandle(sf);
			break;
		case 0x20:
			timerHandle(sf);
			break;
		case 0x80:
			syscallHandle(sf);
			break;
		default:assert(0);
	}
}

void GProtectFaultHandle(struct StackFrame *sf) {
	assert(0);
	return;
}

void timerHandle(struct StackFrame *sf){
	//TODO 完成进程调度，建议使用时间片轮转，按顺序调度
	//switch process
	for(int i=MAX_PCB_NUM-1;i>=1;i--)
	{
		if(pcb[i].state == STATE_BLOCKED )
		{
			pcb[i].sleepTime -= 1;
			if(pcb[i].sleepTime == 0 )
			{
				pcb[i].state = STATE_RUNNABLE;
			}
		}
	}
	//putChar('c');
	//if(flag == -1 )
	//putChar(flag);
	pcb[current].timeCount++;
	if(pcb[current].timeCount >= MAX_TIME_COUNT || pcb[current].state==STATE_DEAD || pcb[current].state==STATE_BLOCKED || current==0)
	{
		int flag = -1;
		for(int i=1;i<MAX_PCB_NUM;i++)
		{
			if(pcb[i].state == STATE_RUNNABLE && i!=current)
			{
				flag = i;
			}
		}
		if(flag != -1 )
		{
			if(pcb[current].state==STATE_RUNNING)
			{
				pcb[current].state = STATE_RUNNABLE;
				pcb[current].timeCount = 0;
			}
			pcb[flag].state = STATE_RUNNING;
			current = flag;
			uint32_t tmpStackTop=pcb[current].stackTop;
			tss.esp0=(uint32_t)&(pcb[current].stackTop);
			asm volatile("movl %0,%%esp"::"m"(tmpStackTop));
			asm volatile("popl %gs");
			asm volatile("popl %fs");
			asm volatile("popl %es");
			asm volatile("popl %ds");
			asm volatile("popal");
			asm volatile("addl $8,%esp");
			asm volatile("iret");
			return;
		}
		else
		{
			if(pcb[current].state==STATE_RUNNING)
			{
				pcb[current].state=STATE_RUNNABLE;;
				pcb[current].timeCount=0;
			}
			current=0;
			uint32_t tmpStackTop=pcb[current].stackTop;
			tss.esp0=(uint32_t)&(pcb[current].stackTop);
			asm volatile("movl %0,%%esp"::"m"(tmpStackTop));
			asm volatile("popl %gs");
			asm volatile("popl %fs");
			asm volatile("popl %es");
			asm volatile("popl %ds");
			asm volatile("popal");
			asm volatile("addl $8,%esp");
			asm volatile("iret");
			return ;
		}
	}

}


void syscallHandle(struct StackFrame *sf) {
	switch(sf->eax) { // syscall number
		case 0:
			syscallWrite(sf);
			break; // for SYS_WRITE
		/* Add Fork,Sleep... */
		case 1:
			syscallFork(sf);
			break;
		case 2:
			syscallExec(sf);
			break;
		case 3:
			syscallSleep(sf);
			break;
		case 4:
			syscallExit(sf);
			break;
		default:break;
	}
}

void syscallWrite(struct StackFrame *sf) {
	switch(sf->ecx) { // file descriptor
		case 0:
			syscallPrint(sf);
			break; // for STD_OUT
		default:break;
	}
}

void syscallPrint(struct StackFrame *sf) {
	int sel = sf->ds; // segment selector for user data, need further modification
	char *str = (char*)sf->edx;
	int size = sf->ebx;
	int i = 0;
	int pos = 0;
	char character = 0;
	uint16_t data = 0;
	asm volatile("movw %0, %%es"::"m"(sel));
	for (i = 0; i < size; i++) {
		asm volatile("movb %%es:(%1), %0":"=r"(character):"r"(str+i));
		if(character == '\n') {
			displayRow++;
			displayCol=0;
			if(displayRow==25){
				displayRow=24;
				displayCol=0;
				scrollScreen();
			}
		}
		else {
			data = character | (0x0c << 8);
			pos = (80*displayRow+displayCol)*2;
			asm volatile("movw %0, (%1)"::"r"(data),"r"(pos+0xb8000));
			displayCol++;
			if(displayCol==80){
				displayRow++;
				displayCol=0;
				if(displayRow==25){
					displayRow=24;
					displayCol=0;
					scrollScreen();
				}
			}
		}

	}
	updateCursor(displayRow, displayCol);
	sf->eax=size;
	return;
}	

void memcpy(void* dst,void* src,size_t size){
	for(uint32_t j=0;j<size;j++){
		*(uint8_t*)(dst+j)=*(uint8_t*)(src+j);
	}
}

void syscallFork(struct StackFrame *sf){
	//TODO 完善它

	//TODO 查找空闲pcb，如果没有就返回-1
	int i=0;
	for(;i<MAX_PCB_NUM;i++)
	{
		if(pcb[i].state==STATE_DEAD)
		{
			break;
		}
	}
	if(i == MAX_PCB_NUM) {
		pcb[current].regs.eax=-1;
		return;
	}
	//TODO 拷贝地址空间
//这样就可以通过PID作为索引，用数组下标快速访问相应的进程控制块。
//内核进程会占据0号pcb，剩下的分配给用户进程。同样为了简单，我们默认每个pcb 对应进程的内存空间固定， 
//pcb[i] 对应的进程内存起始地址为 (i + 1) * 0x100000 ，大小为 0x100000
	memcpy((void*)((i + 1) * 0x100000),(void*)((current + 1) * 0x100000),0x100000);


	// 拷贝pcb，这部分代码给出了，请注意理解
	memcpy(&pcb[i],&pcb[current],sizeof(ProcessTable));
	
	pcb[i].regs.eax = 0;
	pcb[i].regs.cs = USEL(1 + i * 2);
	pcb[i].regs.ds = USEL(2 + i * 2);
	pcb[i].regs.es = USEL(2 + i * 2);
	pcb[i].regs.fs = USEL(2 + i * 2);
	pcb[i].regs.gs = USEL(2 + i * 2);
	pcb[i].regs.ss = USEL(2 + i * 2);
	pcb[i].stackTop = (uint32_t)&(pcb[i].regs);
	pcb[i].state = STATE_RUNNABLE;
	pcb[i].timeCount = 0;
	pcb[i].sleepTime = 0;

	pcb[current].regs.eax = i;
	return;
}	


void syscallExec(struct StackFrame *sf) {
	// TODO 完成exec
	// hint: 用loadelf，已经封装好了
	uint32_t entry = 0;
	uint32_t secstart = sf->ecx;
	uint32_t secnum = sf->edx;


	uint32_t argc = sf->ebx;
	char ** argv = (char **)sf->esi;
	char vec[100][100]; 
	int i= 0,j=0;
	if(argc >0)
	{
		asm volatile("movw %0, %%es"::"m"(pcb[current].regs.ds));
		uint32_t str;
		for(i=0;i<argc;i++)
		{
			asm volatile("movl %%es:(%1), %0":"=r"(str):"r"(argv+i));
			char x = 'x';
			for(j=0;;j++)
			{
				asm volatile("movb %%es:(%1), %0":"=r"(x):"r"((char*)str+j));
				putChar(x);
				vec[i][j] = x;
				if(x == 0) {j++;break;}
			}
		}
	}

	for(int x=0;x<i;x++)
	{
		for(int y=0;y<j;y++)
		{
			putChar(vec[x][y]);
		}
	}



	

	//把elf文件加载到Pysaddr开始的地址，即将加载的程序以编号为Secstart的扇区开始，占据Secnum个扇区（LBA），传入的entry是个指针
	int f = loadelf(secstart, secnum,(current+1) * 0x100000,&entry);
	if(f==0) pcb[current].regs.eip = entry;
	pcb[current].regs.eax= 0;

	asm volatile("movw %0, %%es"::"m"(pcb[current].regs.ds));
	for(int x=0;x<i;x++)
	{
		uint32_t str;
		asm volatile("movl %%es:(%1), %0":"=r"(str):"r"(argv+x));
		int y=0;
		for(y=0;y<j;y++)
		{
			asm volatile("movb %0,%%es:(%1)"::"r"(vec[x][y]),"r"((char*)str+y));
			//putChar(vec[x][y]);
			
		}
		asm volatile("movb %0,%%es:(%1)"::"r"(0),"r"((char*)str+y));
	}
	
	if(argc>0)
	{
		uint32_t esp = pcb[current].regs.esp;
		uint32_t ss = pcb[current].regs.ss;
		asm volatile("movw %0, %%es"::"m"(ss));
		asm volatile("movl %0, %%es:(%1)"::"r"(argc),"r"(esp+4));
		asm volatile("movl %0, %%es:(%1)"::"r"(argv),"r"(esp+8));
	}
	return;
}

void syscallSleep(struct StackFrame *sf){
	//TODO:实现它
	assert(sf->ecx >0);
	pcb[current].sleepTime=sf->ecx;
	pcb[current].state=STATE_BLOCKED;
	asm volatile("int $0x20");
}	

void syscallExit(struct StackFrame *sf){
	//TODO 先设置成dead，然后用int 0x20进入调度
	pcb[current].state = STATE_DEAD;
	asm volatile("int $0x20");
}
