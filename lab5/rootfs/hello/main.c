#include "stddef.h"

int getpid();
size_t uartread(char buf[], size_t size);
size_t uartwrite(const char buf[], size_t size);
int exec(const char *name, char *const argv[]);
int fork();
void exit();
int mbox_call(unsigned char ch, unsigned int *mbox);
void kill(int pid);
void yield();

int main(){
    char buf[] = "hello world!\n";
    uartwrite(buf, sizeof(buf));
    return 0;
}

int getpid(){
    asm("mov x8, 0");
    asm("svc 0");
    return 0;
}

size_t uartread(char buf[], size_t size){
    asm("mov x8, 1");
    asm("svc 0");
    return 0;
}

size_t uartwrite(const char buf[], size_t size){
    asm("mov x8, 2");
    asm("svc 0");
    return 0;
}

int exec(const char *name, char *const argv[]){
    asm("mov x8, 3");
    asm("svc 0");
    return 0;
}

int fork(){
    asm("mov x8, 4");
    asm("svc 0");
    return 0;
}

void exit(){
    asm("mov x8, 5");
    asm("svc 0");
}

int mbox_call(unsigned char ch, unsigned int *mbox){
    asm("mov x8, 6");
    asm("svc 0");
    return 0;
}

void kill(int pid){
    asm("mov x8, 7");
    asm("svc 0");
    return ;
}

void yield(){
    asm("mov x8, 8");
    asm("svc 0");
    return ;
}

