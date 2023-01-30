#include "x86.h"
#include "device.h"

#define INTERRUPT_GATE_32   0xE
#define TRAP_GATE_32        0xF

/* IDT表的内容 */
struct GateDescriptor idt[NR_IRQ]; // NR_IRQ=256, defined in x86/cpu.h

/* 初始化一个中断门(interrupt gate) */
static void setIntr(struct GateDescriptor *ptr, uint32_t selector, uint32_t offset, uint32_t dpl) {
	/* TODO: 初始化interrupt gate，
		ptr是结构体首地址；
		selector是段选择子，但是似乎不能直接让ptr->selector=selector....
		请理解后再填，弄清楚offset和dpl是啥玩意儿
		结构体声明在某个文件里
	*/
	ptr -> offset_15_0 = offset & 0xffff;
	ptr -> offset_31_16 = offset >> 16;
	ptr -> pad0 = 0;
	ptr -> present = 1;
	ptr -> privilege_level = dpl;
	if (dpl == 0)
	ptr -> segment = KSEL(selector);
	else
	ptr -> segment = USEL(selector);
	ptr -> system = 0;
	ptr -> type = INTERRUPT_GATE_32;

}

/* 初始化一个陷阱门(trap gate) */
static void setTrap(struct GateDescriptor *ptr, uint32_t selector, uint32_t offset, uint32_t dpl) {
	/* TODO: 初始化trap gate
		要求同上
	*/
	ptr -> offset_15_0 = offset & 0xffff;
	ptr -> offset_31_16 = offset >> 16;
	ptr -> pad0 = 0;
	ptr -> present = 1;
	ptr -> privilege_level = dpl;
	if (dpl == 0)
	ptr -> segment = KSEL(selector);
	else
	ptr -> segment = USEL(selector);
	ptr -> system = 0;
	ptr -> type = TRAP_GATE_32;
}

/* 声明函数，这些函数在汇编代码里定义 */
void irqEmpty();
void irqErrorCode();

void irqDoubleFault(); // 0x8
void irqInvalidTSS(); // 0xa
void irqSegNotPresent(); // 0xb
void irqStackSegFault(); // 0xc
void irqGProtectFault(); // 0xd
void irqPageFault(); // 0xe
void irqAlignCheck(); // 0x11
void irqSecException(); // 0x1e
void irqKeyboard(); 

void irqSyscall();


//256 interrupt vector
void initIdt() {
	int i;
	/* 为了防止系统异常终止，所有irq都有处理函数(irqEmpty)。 */
	for (i = 0; i < NR_IRQ; i ++) {
		setTrap(idt + i, SEG_KCODE, (uint32_t)irqEmpty, DPL_KERN);
	}
	/*init your idt here 初始化 IDT 表, 为中断设置中断处理函数*/
	
	/* Exceptions with error code */
	// TODO: 填好剩下的表项，参考上面那个，是不是就会了？
	//这里建议把keyboard设置成Intr，否则如果快速按键的话可能会出现连续中断嵌套的问题；
	//同时syscall指令也可以设置成Intr，以防止被打断。其余均可设置成Trap。
	setTrap(idt + 0x8,SEG_KCODE,(uint32_t)irqDoubleFault,DPL_KERN);
	setTrap(idt + 0xa,SEG_KCODE,(uint32_t)irqInvalidTSS,DPL_KERN);
	setTrap(idt + 0xb,SEG_KCODE,(uint32_t)irqSegNotPresent,DPL_KERN);
	setTrap(idt + 0xc,SEG_KCODE,(uint32_t)irqStackSegFault,DPL_KERN);
	setTrap(idt + 0xd,SEG_KCODE,(uint32_t)irqGProtectFault,DPL_KERN);
	setTrap(idt + 0xe,SEG_KCODE,(uint32_t)irqPageFault,DPL_KERN);
	setTrap(idt + 0x11,SEG_KCODE,(uint32_t)irqAlignCheck,DPL_KERN);
	setTrap(idt + 0x1e,SEG_KCODE,(uint32_t)irqSecException,DPL_KERN);

	setIntr(idt + 0x21,SEG_KCODE,(uint32_t)irqKeyboard,DPL_KERN);

	/* Exceptions with DPL = 3 */
	// TODO: 填好剩下的表项，哪个dpl是3，手册里都说了，别看往年代码，都是错的，小心被抓 ：）
	// 真的有说吗....
	setIntr(idt + 0x80,SEG_KCODE,(uint32_t)irqSyscall,DPL_USER);


	/* 写入IDT */
	saveIdt(idt, sizeof(idt));//use lidt
}
