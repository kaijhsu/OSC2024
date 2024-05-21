#include "stddef.h"

unsigned int getpid();
size_t uartread(char buf[], size_t size);
size_t uartwrite(const char buf[], size_t size);
int exec(const char *name, char *const argv[]);
int fork();
void exit();
int mbox_call(unsigned char ch, unsigned int *mbox);
void kill(int pid);
void yield();
char* m_itoa(int value, char *s);

unsigned int m_vsprintf(char *dst, char *fmt, __builtin_va_list args);
void printf(char* fmt, ...);

int main(){
    printf("\nFork Test, pid %d\n", getpid());
    int cnt = 1;
    int ret = 0;
    if((ret = fork()) == 0){
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        printf("first child pid: %d, cnt: %d, ptr: %x, sp: %x\n", getpid(), cnt, &cnt, cur_sp);
        ++cnt;
        if((ret = fork()) != 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            printf("first child pid: %d, cnt: %d, ptr: %x, sp: %x\n", getpid(), cnt, &cnt, cur_sp);
        }
        else{
            while(cnt < 5){
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
                for(int i=0; i<100000; ++i)
                    asm volatile("nop");
                ++cnt;
            }
        }
        exit();
    }
    else {
        printf("parent here, pid %d, child %d\n", getpid(), ret);
    }
    return 0;
}

char* m_itoa(int value, char *s){
    int idx = 0;
    if(value < 0) {
        value *= -1;
        s[idx++] = '-';
    }

    char tmp[10];
    int tidx = 0;
    // read from least significant digit
    do {
        tmp[tidx++] = '0' + value % 10;
        value /= 10;
    } while(value != 0 && tidx < 11);

    // reverse 
    for(int i = tidx-1; i>=0; i--)
        s[idx++] = tmp[i];

    s[idx] = '\0';
    return s;
}

void printf(char* fmt, ...){
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    char str[2048];
    m_vsprintf(str, fmt, args);
    char *s = str;
    uartwrite(str, sizeof(str));
}


unsigned int m_vsprintf(char *dst, char *fmt, __builtin_va_list args) {
    char *dst_orig = dst;

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            // escape %
            if (*fmt == '%') {
                goto put;
            }
            // string
            else if (*fmt == 's') {
                char *p = __builtin_va_arg(args, char *);
                while (*p) *dst++ = *p++;
            }
            // number
            else if (*fmt == 'd') {
                int arg = __builtin_va_arg(args, int);
                char buf[11];
                char *p = m_itoa(arg, buf);
                while (*p)
                    *dst++ = *p++;
            }
            else if (*fmt == 'l'){
                unsigned long long arg = __builtin_va_arg(args, unsigned long long);
                for(int i=60; i>=0; i-=4){
                    int hex = (arg >> i) & 0xF;
                    *dst++ = (hex > 9)? 'A' - 10 + hex: '0' + hex;
                }
            }
            else if (*fmt == 'x'){
                unsigned int arg = __builtin_va_arg(args, unsigned int);
                for(int i=28; i>=0; i-=4){
                    int hex = (arg >> i) & 0xF;
                    *dst++ = (hex > 9)? 'A' - 10 + hex: '0' + hex;
                }
            }
            else if (*fmt == 'c'){
                int arg = __builtin_va_arg(args, int);
                *dst++ = (char)arg;
            }
            else if (*fmt == 'b'){
                unsigned char arg = __builtin_va_arg(args, int);
                for(int i=4; i>=0; i-=4){
                    int hex = (arg >> i) & 0xf;
                    *dst++ = (hex > 9)? 'A' - 10 + hex: '0' + hex;
                }
            }
        } else {
        put:
            *dst++ = *fmt;
        }
        fmt++;
    }
    *dst = '\0';
    
    return dst - dst_orig;  // return written bytes
}

unsigned int getpid(){
    unsigned int pid;
    asm volatile("mov x8, 0");
    asm volatile("svc 0");
    asm volatile("mov %0, x0" : "=r"(pid));
    return pid;
}

size_t uartread(char buf[], size_t size){
    size_t ret;
    asm("mov x8, 1");
    asm("svc 0");
    asm volatile("mov %0, x0" : "=r"(ret));
    return ret;
}

size_t uartwrite(const char buf[], size_t size){
    size_t ret;
    asm("mov x8, 2");
    asm("svc 0");
    asm volatile("mov %0, x0" : "=r"(ret));
    return 0;
}

int exec(const char *name, char *const argv[]){
    int ret;
    asm("mov x8, 3");
    asm("svc 0");
    asm volatile("mov %0, x0" : "=r"(ret));
    return ret;
}

int fork(){
    int ret;
    asm("mov x8, 4");
    asm("svc 0");
    asm volatile("mov %0, x0" : "=r"(ret));
    return ret;
}

void exit(){
    asm("mov x8, 5");
    asm("svc 0");
    return ;
}

int mbox_call(unsigned char ch, unsigned int *mbox){
    int ret;
    asm("mov x8, 6");
    asm("svc 0");
    asm volatile("mov %0, x0" : "=r"(ret));
    return ret;
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

