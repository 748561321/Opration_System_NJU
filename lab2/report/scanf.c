//scanf的syscall部分代码
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