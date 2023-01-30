#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#define Size 100

int main()
{
    char buffer[Size]={0};
    int fd[2];
    pipe(fd);
    int ret = fork();
    if(ret)
    {
        read(fd[0],buffer,Size);
        printf("the result is:\n %s\n",buffer);
    }
    else
    {
        dup2(fd[1],STDOUT_FILENO);
        system("ls");
    }
    close(fd[0]);
    close(fd[1]);
    return 0;



}
