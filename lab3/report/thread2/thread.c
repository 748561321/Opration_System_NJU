#include"thread.h"

__asm__                    //声明一个函数，叫做callNext                          
(                                                                                
".global callNext       \n\t"    
"callNext:              \n\t"                                
"   pop (%ecx)          \n\t"
"   jmp *%edx           \n\t"
);

//终止并返回
#define yield(x)  asmV("pushal");\
    asmV("mov %0, %1":"=r"(tcbList[currentId].tarResValue):"r"(x));\
    asmV("mov %%esp,%0":"=r"(tcbList[currentId].tarESP));\
    asmV("mov %0, %%esp"::"r"(tcbList[nextId].tarESP));\
    asmV("mov %0, %%ecx     \n\t"\
        "mov %1, %%edx      \n\t"\
        "call callNext      \n\t"\
        ::"r"(&tcbList[currentId].tarEIP),"r"(tcbList[nextId].tarEIP)\
    );\
    currentId = nextId;\
    asmV("popal");

//销毁线程
#define finish  int temp = currentId;\
    currentId = nextId;\
    if(currentId == 0) exit(0);\
    asmV("pushal");\
    asmV("mov %0, %1":"=r"(tcbList[temp].status):"r"(status_empty));\
    asmV("mov %%esp,%0":"=r"(tcbList[temp].tarESP));\
    asmV("mov %0, %%esp"::"r"(tcbList[nextId].tarESP));\
    asmV("mov %0, %%ecx     \n\t"\
        "mov %1, %%edx      \n\t"\
        "call callNext      \n\t"\
        ::"r"(&tcbList[temp].tarEIP),"r"(tcbList[nextId].tarEIP)\
    );\
    asmV("popal");

//触发线程
#define launch(x) currentId = x;\
    asmV("pushal");\
    asmV("mov %%esp, %0":"=r"(tcbList[0].tarESP));\
    asmV("mov %0, %%esp"::"r"(tcbList[currentId].tarESP));\
    asmV("mov %0, %%ecx \n\t"\
        "mov %1, %%edx \n\t"\
        "call callNext \n\t"\
        ::"r"(&tcbList[0].tarEIP),"r"(tcbList[currentId].tarEIP)\
    );\
    asmV("popal");

tcb tcbList[maxThread];
int currentId;
int nextId;
void init()
{
    currentId = 0;
    tcbList[0].status = status_run;   
}

//创建一个线程，众所周知，一个线程相当于一个函数，第一个参数是【函数指针】，第二个参数是【函数参数多少】，后面的变长参数就是【函数的参数】
int create_thread(__uint32_t func, int argNum, ...){
    
    //下面是查找空闲tcb，那么这个管理方式，查找的时间复杂度是多少？你是否能够设计更加合适的数据结构，来加快访问速度？
    int id;
    for(id = 0; id < maxThread; id++){
        if (tcbList[id].status == status_empty){
            break;
        }
    }
    if (id >= maxThread) return -1;

    //初始化
    printf("***thread(id %d) create***\n", id);
    tcbList[id].status = status_ready;
    tcbList[id].tarESP = maxStackSize;
    
    //函数参数
    va_list valist;
    va_start(valist, argNum);
    tcbList[id].tarESP = tcbList[id].stack + maxStackSize;
    tcbList[id].tarESP -= 4 * argNum;
    for(int i = 0; i < argNum ; i++){
        *(__uint32_t*)(tcbList[id].tarESP + i * 4) = va_arg(valist, int);
    }
    va_end(valist);
    tcbList[id].tarESP -= 4;
    tcbList[id].tarEIP = func;
     //printf("id : %d \nsrcESP : %d srcEIP : %d \ntarESP : %d tarEIP : %d \n",id,tcbList[id].srcESP,tcbList[id].srcEIP,tcbList[id].tarESP,tcbList[id].tarEIP);

    return id;
}

int launch_thread(int id){
   // printf("id : %d \nsrcESP : %d srcEIP : %d \ntarESP : %d tarEIP : %d \n",id,tcbList[id].srcESP,tcbList[id].srcEIP,tcbList[id].tarESP,tcbList[id].tarEIP);
    if (tcbList[id].status == status_empty){
        printf("***thread(id %d) is empty!***\n", id);
        return -1;
    }
    if (tcbList[id].status == status_ready){
        printf("***thread(id %d) begin***\n", id);
        tcbList[id].status = status_run;
    }
    //printf("launch %d\n",id);
    launch(id);
    // if (tcbList[id].status == status_finish){
    //     printf("***thread(id %d) finish***\n", id);
    //     tcbList[id].status = status_empty;
    //     return 0;
    // }
    return tcbList[id].tarResValue;
}

void yield_thread(int value){
    //printf("yield %d\n",value);
    int flag = 0;

    for(int i=1;i<maxThread;i++)
    {
        if(currentId != i && tcbList[i].status == status_run)
        {
            nextId = i;
            flag = 1;
            break;
        }
    }

   

    if(flag == 0) 
    {
      // if(tcbList[currentId].status == status_empty)
      nextId = 0;
       // else
       // nextId = 0;
    }
    printf("\ncurrentId %d nextId %d\n",currentId,nextId);
    //printf("\n0 %d 1 %d 2 %d",tcbList[0].tarESP,tcbList[1].tarESP,tcbList[2].tarESP);

    yield(value)
}

void destory_thread()
{
    //printf("debug %d",currentId);
    int flag = 0;
    for(int i=1;i<maxThread;i++)
    {
        if(currentId != i && tcbList[i].status == status_run)
        {
            nextId = i;
            flag = 1;
            break;
        }
    }
    if(flag == 0) 
    {
       if(tcbList[currentId].status == status_empty)
       nextId = currentId;
       else
        nextId = 0;
    }
    //printf("\n des currentId %d nextId %d\n",currentId,nextId);

    printf("***thread(id %d) finish***\n", currentId);
    tcbList[currentId].status = status_empty;

    finish
    
}

enum threadStatus getThreadStatus(int id){
    return tcbList[id].status;
}

void search()
{
    currentId=0;
    int flag =0;
    for(int i=1;i<maxThread;i++)
    {
        //if(i<=2) printf("\n id %d status %d",i,tcbList[i].status);
        if(tcbList[i].status == status_run)
        {
            nextId = i;
            flag =1;
            break;
        }
    }
    if(flag == 1) 
    {
        //printf("debug: %d %d",currentId,nextId);
        yield(0);
    }
}