#include "common.h"
#include "x86.h"
#include "device.h"

void kEntry(void) {

	// Interruption is disabled in bootloader

	initSerial();// initialize serial port
	//putChar('x');
	initIdt(); // initialize idt
	//putChar('c');
	initIntr(); // iniialize 8259a
	//putChar('y');
	initSeg(); // initialize gdt, tss
	//putChar('z');
	initVga(); // initialize vga device
	//putChar('d');
	initTimer(); // initialize timer device
	//putChar('e');
	initProc();
}
