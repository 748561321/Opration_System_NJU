#include"thread.h"


void test1(int n){
    for(int i = 1; i <= n; i++){
        printf("ping");
        yield_thread(i);
    }
    destory_thread();
}

void test2(int n){
    for(int i = 1; i <= n; i++){
        printf("pong");
        yield_thread(i);
    }
    destory_thread();
}

int main(){
    init();
    int id1 = create_thread(test1, 1, 10);
    int id2 = create_thread(test2, 1, 5);
    for(int i = 0; i <= 10; i++){
        if (getThreadStatus(id1) != status_empty) printf(" %d\n", launch_thread(id1));
        if (getThreadStatus(id2) != status_empty) printf(" %d\n", launch_thread(id2));
    }
    return 0;
}
