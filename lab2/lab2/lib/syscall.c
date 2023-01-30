#include "lib.h"
#include "types.h"



/*
 * io lib here
 * 库函数写在这
 */


//static inline int32_t syscall(int num, uint32_t a1,uint32_t a2,
int32_t syscall(int num, uint32_t a1,uint32_t a2,
		uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t ret = 0;
	//Generic system call: pass system call number in AX
	//up to five parameters in DX,CX,BX,DI,SI
	//Interrupt kernel with T_SYSCALL
	//
	//The "volatile" tells the assembler not to optimize
	//this instruction away just because we don't use the
	//return value
	//
	//The last clause tells the assembler that this can potentially
	//change the condition and arbitrary memory locations.

	/*
		 Note: ebp shouldn't be flushed
	    May not be necessary to store the value of eax, ebx, ecx, edx, esi, edi
	*/
	uint32_t eax, ecx, edx, ebx, esi, edi;
	uint16_t selector;
	
	asm volatile("movl %%eax, %0":"=m"(eax));
	asm volatile("movl %%ecx, %0":"=m"(ecx));
	asm volatile("movl %%edx, %0":"=m"(edx));
	asm volatile("movl %%ebx, %0":"=m"(ebx));
	asm volatile("movl %%esi,  %0":"=m"(esi));
	asm volatile("movl %%edi,  %0":"=m"(edi));
	asm volatile("movl %0, %%eax": :"m"(num));
	asm volatile("movl %0, %%ecx"::"m"(a1));
	asm volatile("movl %0, %%edx"::"m"(a2));
	asm volatile("movl %0, %%ebx"::"m"(a3));
	asm volatile("movl %0, %%esi" ::"m"(a4));
	asm volatile("movl %0, %%edi" ::"m"(a5));
	asm volatile("int $0x80");
	asm volatile("movl %%eax, %0":"=m"(ret));
	asm volatile("movl %0, %%eax"::"m"(eax));
	asm volatile("movl %0, %%ecx"::"m"(ecx));
	asm volatile("movl %0, %%edx"::"m"(edx));
	asm volatile("movl %0, %%ebx"::"m"(ebx));
	asm volatile("movl %0, %%esi"::"m"(esi));
	asm volatile("movl %0, %%edi"::"m"(edi));
	
	asm volatile("movw %%ss, %0":"=m"(selector)); //%ds is reset after iret
	//selector = 16;
	asm volatile("movw %%ax, %%ds"::"a"(selector));
	
	return ret;
}

char getChar(){ // 对应SYS_READ STD_IN
	// TODO: 实现getChar函数，方式不限
	return syscall(SYS_READ,0,0,0,0,0);
}

void getStr(char *str, int size){ // 对应SYS_READ STD_STR
	// TODO: 实现getStr函数，方式不限
	syscall(SYS_READ,1,(uint32_t)str,(uint32_t)size,0,0);
}

int dec2Str(int decimal, char *buffer, int size, int count);
int hex2Str(uint32_t hexadecimal, char *buffer, int size, int count);
int str2Str(char *string, char *buffer, int size, int count);

void printf(const char *format,...){
	int i=0; // format index
	char buffer[MAX_BUFFER_SIZE];
	int count=0; // buffer index
	int index=0; // parameter index
	void *paraList=(void*)&format; // address of format in stack
	int state=0; // 0: legal character; 1: '%'; 2: illegal format
	int decimal=0;
	uint32_t hexadecimal=0;
	char *string=0;
	char character=0;
	//void* para=0;
	while(format[i]!=0){
		//buffer[count] = format[i];
		//count++;
		//i++;
		//TODO: 可以借助状态机（回忆数电），辅助的函数已经实现好了，注意阅读手册	
		switch (state)
		{
		case 0:
			switch (format[i])
			{
			case '%':
				state = 1;
				break;
			default:
				state = 0;
				buffer[count++] = format[i];
				break;
			}
			break;
		case 1:
			switch (format[i])
			{
			case '%':
				state = 0;
				buffer[count++] = '%';
				break;
			case 'c':
				state = 0;
				index += 4;
				character = *(char *)(paraList+index);
				buffer[count++] = character;
				break;
			case 's':
				state = 0;
				index += 4;
				string = *(char**)(paraList+index);
				count = str2Str(string,buffer,MAX_BUFFER_SIZE,count);
				break;
			case 'x':
				state = 0;
				index += 4;
				hexadecimal = *(uint32_t *)(paraList+index);
				count = hex2Str(hexadecimal,buffer,MAX_BUFFER_SIZE,count);
				break;
			case 'd':
				state = 0;
				index += 4;
				decimal = *(int *)(paraList + index);
				count = dec2Str(decimal,buffer,MAX_BUFFER_SIZE,count);
				break;
			default:
				state = 2;
				break;
			}
			break;
		case 2:
			break;
		default:
			break;
		}
		if(state == 2)
			break;
		i++;
	}
	if(count!=0)
		syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)count, 0, 0);
}
int Str2str(char * str,char *buffer,int size,int count)
{
	int i =0;

	while(buffer[count] != '\n' && buffer[count] != 0)
	{
		str[i++] =buffer[count++];
	}
	str[--i] = '\0';
	//printf("debug::%s",str);
	return count;
}
void scanf(const char *format,...)
{
	int i=0; // format index
	char buffer[MAX_BUFFER_SIZE];
	int count=0; // buffer index
	int index=0; // parameter index
	void *paraList=(void*)&format; // address of format in stack
	int state=0; // 0: legal character; 1: '%'; 2: illegal format
	buffer[0] = 0;
	while(format[i]!= 0)
	{
		while(buffer[count] == 0)
		{
			//printf("debug:%c\n",buffer[0]);
			syscall(SYS_READ,2,(uint32_t)buffer,(uint32_t)MAX_BUFFER_SIZE,0,0);
			//printf("debug:%c\n",buffer[0]);
			count = 0;
		}
		switch (state)
		{
		case 0:
			switch(format[i])
			{
				case '%':
					state = 1;
					break;
				default:
					state = 0;
					break;
			}
			break;
		case 1:
			switch (format[i])
			{
				case '%':
					state = 0;
					break;
				case 'c':
					state =0;
					index +=4;
					char character = ' ';
					while(character == ' ')
					{
					   character = buffer[count++];
					}
					*(*(char **)(paraList+index)) = character;
					break;
				case 's':
					state = 0;
					index +=4;
					count = Str2str(*((char **)(paraList+index)),buffer,MAX_BUFFER_SIZE,count);
					break;
			default:
				state =2;
				break;
			}
		case 2:
			break;
		default:
			break;
		}
		i++;
	}
}

		


int dec2Str(int decimal, char *buffer, int size, int count) {
	int i=0;
	int temp;
	int number[16];

	if(decimal<0){
		buffer[count]='-';
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		temp=decimal/10;
		number[i]=temp*10-decimal;
		decimal=temp;
		i++;
		while(decimal!=0){
			temp=decimal/10;
			number[i]=temp*10-decimal;
			decimal=temp;
			i++;
		}
	}
	else{
		temp=decimal/10;
		number[i]=decimal-temp*10;
		decimal=temp;
		i++;
		while(decimal!=0){
			temp=decimal/10;
			number[i]=decimal-temp*10;
			decimal=temp;
			i++;
		}
	}

	while(i!=0){
		buffer[count]=number[i-1]+'0';
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		i--;
	}
	return count;
}

int hex2Str(uint32_t hexadecimal, char *buffer, int size, int count) {
	int i=0;
	uint32_t temp=0;
	int number[16];

	temp=hexadecimal>>4;
	number[i]=hexadecimal-(temp<<4);
	hexadecimal=temp;
	i++;
	while(hexadecimal!=0){
		temp=hexadecimal>>4;
		number[i]=hexadecimal-(temp<<4);
		hexadecimal=temp;
		i++;
	}

	while(i!=0){
		if(number[i-1]<10)
			buffer[count]=number[i-1]+'0';
		else
			buffer[count]=number[i-1]-10+'a';
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		i--;
	}
	return count;
}

int str2Str(char *string, char *buffer, int size, int count) {
	int i=0;
	while(string[i]!=0){
		buffer[count]=string[i];
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		i++;
	}
	return count;
}
