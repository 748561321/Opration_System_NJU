#include "lib.h"
#include "types.h"
#define N 5
void philosopher()
{
	sem_t forkarray[N];

	for(int i=0;i<N;i++)
		sem_init(&forkarray[i],1);//value = 1
	
	for(int i=0;i<N-1;i++)//最多四个子进程
	{
		int ret = fork();
		if(ret == -1)
		{
			printf("error\n");
		}
		if(ret == 0)//child
		{
			int id = get_id();
			id -=1;//哲学家号是0-4
			while(1)
			{
				if(id%2 == 1)
				{
					
					sem_wait(&forkarray[id]);            // 去拿左边的叉子
					sleep(128);
					sem_wait(&forkarray[(id+1)%N]);      // 去拿右边的叉子
					sleep(128);
					printf("Philosopher %d: eat\n", id);
					sleep(128);
					printf("Philosopher %d: think\n", id);
					sleep(128);
					sem_post(&forkarray[id]);            // 放下左边的叉子
					sleep(128);
					sem_post(&forkarray[(id+1)%N]);      // 放下右边的叉子
					sleep(128);
				}
				else
				{
					
					sem_wait(&forkarray[(id+1)%N]);      // 去拿右边的叉子
					sleep(128);
					sem_wait(&forkarray[id]);            // 去拿左边的叉子
					sleep(128);
					printf("Philosopher %d: eat\n", id);
					sleep(128);
					printf("Philosopher %d: think\n", id);
					sleep(128);
					sem_post(&forkarray[id]);            // 放下左边的叉子
					sleep(128);
					sem_post(&forkarray[(id+1)%N]);      // 放下右边的叉子
					sleep(128);
				}
			}
		}
	}
	int id = 0;
	while(1)
	{
		sem_wait(&forkarray[(id+1)%N]);      // 去拿右边的叉子
		sleep(128);
		sem_wait(&forkarray[id]);            // 去拿左边的叉子
		sleep(128);
		printf("Philosopher %d: eat\n", id);
		sleep(128);
		printf("Philosopher %d: think\n", id);
		sleep(128);
		sem_post(&forkarray[id]);            // 放下左边的叉子
		sleep(128);
		sem_post(&forkarray[(id+1)%N]);      // 放下右边的叉子
		sleep(128);	
	}
}

void pro_con()
{
	// class BoundedBuffer { //buffer
    // mutex = new Semaphore(1);
    // fullBuffers = new Semaphore(0);
    // emptyBuffers = new Semaphore(n);}
	sem_t mutex,fullBuffers,emptyBuffers;
	sem_init(&mutex,1);
	sem_init(&fullBuffers,0);
	sem_init(&emptyBuffers,N);

	for(int i=0;i<4;i++)
	{
		int ret = fork();
		if(ret == -1)
		{
			printf("error\n");
		}
		if(ret == 0)//child
		{
			while(1)
			{
				int id = get_id();
				id -=1;//1-4
				sem_wait(&emptyBuffers);
				sleep(128);
				sem_wait(&mutex);
				sleep(128);
				printf("Producer %d: produce\n", id);
				sleep(128);
				sem_post(&mutex);
				sleep(128);
				sem_post(&fullBuffers);
				sleep(128);
			}
		}
	}
	while(1)
	{
		//int id = 0;
		sem_wait(&fullBuffers);
		sleep(128);
		sem_wait(&mutex);
		sleep(128);
		printf("Consumer : consume\n");
		sleep(128);
		sem_post(&mutex);
		sleep(128);
		sem_post(&emptyBuffers);
		sleep(128);
	}
// 	BoundedBuffer::Deposit(c){               BoundedBuffer::Remove(c){
//   emptyBuffers->P();                       fullBuffers->P();
//   mutex->P();                              mutex->P();
//   Add c to the buffer;                     Remove c from buffer;
//   mutex->V();                              mutex->V();
//   fullBuffers->V();                        emptyBuffers->V();
// }                                        }
}
void read_write()
{
// 信号量WriteMutex，控制读写操作的互斥，初始化为1
// 读者计数Rcount，正在进行读操作的读者数目，初始化为0
// 信号量CountMutex，控制对读者计数的互斥修改，初始化为1，只允许一个线程修改Rcount计数
	sem_t WriteMutex,CountMutex;
	sem_init(&WriteMutex,1);
	sem_init(&CountMutex,1);
	writeRcount(0);
 	for(int i=0;i<5;i++)//2 - 6
	{
		int ret = fork();
		if(ret == -1)
		{
			printf("error");
		}
		if(ret == 0){
			int id = get_id(); 
			id--;//1-5
			//printf("%d:initial\n",id);
			if(id <= 2)
			{
				while(1)
				{
					sem_wait(&CountMutex);
					sleep(128);
					int Rcount = readRcount();
					//printf("Reader %d: total %d reader\n", id, Rcount);
					if (Rcount == 0)
					{
						sem_wait(&WriteMutex);
						//printf("Reader %d: dedaoxiezhesuo\n", id);
						sleep(128);
					}
					Rcount++;
					writeRcount(Rcount);
					sem_post(&CountMutex);
					sleep(128);
					printf("Reader %d: read, total %d reader\n", id, Rcount);
					sleep(128);
					sem_wait(&CountMutex);
					sleep(128);
					Rcount = readRcount();
					writeRcount(Rcount-1);
					if (Rcount-1 == 0)
					{
						sem_post(&WriteMutex);
						//printf("Reader %d: shiquxiezhesuo\n", id);
						sleep(128);
					}
					sem_post(&CountMutex);
					sleep(128);
				}
			}
			else
			{
				while(1)
				{
					sem_wait(&WriteMutex);
					sleep(128);
					printf("Writer %d: write\n", id);
					sleep(128);
					sem_post(&WriteMutex);
					sleep(128);
				}
			}
			
		}
	}
	int id = 0;
	//printf("%d:initial\n",id);
	while(1)
	{
		sem_wait(&CountMutex);
		sleep(128);
		int Rcount = readRcount();
		//printf("Reader %d: total %d reader\n", id, Rcount);
		if (Rcount == 0)
		{
			sem_wait(&WriteMutex);
			//printf("Reader %d: dedaoxiezhesuo\n", id);
			sleep(128);
		}
		Rcount++;
		writeRcount(Rcount);
		sem_post(&CountMutex);
		sleep(128);
		printf("Reader %d: read, total %d reader\n", id, Rcount);
		sleep(128);
		sem_wait(&CountMutex);
		sleep(128);
		Rcount = readRcount();
		writeRcount(Rcount-1);
		if (Rcount-1 == 0)
		{
			sem_post(&WriteMutex);
			//printf("Reader %d: shiquxiezhesuo\n", id);
			sleep(128);
		}
		sem_post(&CountMutex);
		sleep(128);
	}
}

int uEntry(void) {

	// // 测试scanf	
	// int dec = 0;
	// int hex = 0;
	// char str[6];
	// char cha = 0;
	// int ret = 0;
	// while(1){
	// 	printf("Input:\" Test %%c Test %%6s %%d %%x\"\n");
	// 	ret = scanf(" Test %c Test %6s %d %x", &cha, str, &dec, &hex);
	// 	printf("Ret: %d; %c, %s, %d, %x.\n", ret, cha, str, dec, hex);
	// 	if (ret == 4)
	// 		break;
	// }
	
	// // 测试信号量
	// int i = 4;
	// sem_t sem;
	// printf("Father Process: Semaphore Initializing.\n");
	// ret = sem_init(&sem, 0);
	// if (ret == -1) {
	// 	printf("Father Process: Semaphore Initializing Failed.\n");
	// 	exit();
	// }

	// ret = fork();
	// if (ret == 0) {
	// 	while( i != 0) {
	// 		i --;
	// 		printf("Child Process: Semaphore Waiting.\n");
	// 		sem_wait(&sem);
	// 		printf("Child Process: In Critical Area.\n");
	// 	}
	// 	printf("Child Process: Semaphore Destroying.\n");
	// 	sem_destroy(&sem);
	// 	exit();
	// }
	// else if (ret != -1) {
	// 	while( i != 0) {
	// 		i --;
	// 		printf("Father Process: Sleeping.\n");
	// 		sleep(128);
	// 		printf("Father Process: Semaphore Posting.\n");
	// 		sem_post(&sem);
	// 	}
	// 	printf("Father Process: Semaphore Destroying.\n");
	// 	sem_destroy(&sem);
	// 	exit();
	// }

	// For lab4.3
	// TODO: You need to design and test the philosopher problem.
	// Note that you can create your own functions.
	// Requirements are demonstrated in the guide.
	
	//哲学家
	//philosopher();
	//生产者消费者问题
	//pro_con();
	//读者写者问题
	read_write();

	exit(0);
	return 0;
}
