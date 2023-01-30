#include "lib.h"
#include "types.h"



int uEntry(void) {
	uint16_t selector;
	//uint16_t selector = 16;
	asm volatile("movw %%ss, %0":"=m"(selector)); //XXX necessary or not, iret may reset ds in QEMU
	asm volatile("movw %%ax, %%ds"::"a"(selector));
	
	int data = 0;	
	
	int ret = fork();
	printf("%d",ret);
	int i = 8;
	if (ret == 0) {
		data = 2;
		while( i != 0) {
			i --;
			printf("Child Process: Pong %d, %d;\n", data, i);
			//sleep(1);
			sleep(60);
		}
		int argc = 2;
		char *argv[] = {"oslab","2022"};
		exec2(221, 20,argc,argv);
		exit();
	}
	else if (ret != -1) {
		data = 1;
		while( i != 0) {
			i --;
			printf("Father Process: Ping %d, %d;\n", data, i);
			sleep(60);
		}
		exit();
	}

	return 0;
}
