#include "syscall.h"
#include "thread.h"
#include "mini_uart.h"
#include "exception.h"
#include "mailbox.h"
#include "initramfs.h"


void getpid(uint64_t* sp){
    sp[0] = get_cur_tid();
    // kprintf("syscall return pid %d\n", sp[0]);
}

void uart_read(uint64_t* sp){
    char* buf = (char*)sp[0];
    size_t size = sp[1];
    for(int i=0; i<size; ++i)
        buf[i] = uart_recv();
    // return size
    sp[0] = size;
}

void uart_write(uint64_t* sp){
    char *buf = (char*)sp[0];
    size_t size = sp[1];
    int i=0;
    for(i=0; i<size && buf[i] != '\0'; ++i)
        uart_send(buf[i]);
    // return i
    sp[0] = i;
}

void exec(uint64_t* sp){
    char* name = (char*)sp[0];
    // char** argv = (char **)sp[1];
    
    // // access user program image
    cpio_path path = cpio_search(name);
    if(path.namesize == 0)
        throw(, "file not found: %s", name);
    else if(path.mode != CPIO_FILE)
        throw(, "Not a file: %s", name);
    
    return thread_exec(path);
}


void mailbox_syscall(uint64_t* sp){
    unsigned long ch = sp[0];
    unsigned int* mbox = (void*)sp[1];
    mailbox_call(ch, mbox);
}


void syscall_handler(uint64_t* sp, void *pc){
    uint64_t syscall_num = sp[8];
    switch (syscall_num) {
    case 0:
        // kprintf("syscall getpid\n");
        getpid(sp);
        break;
    case 1:
        // kprintf("syscall getpid\n");
        uart_read(sp);
        break;
    case 2:
        // kprintf("syscall getpid\n");
        uart_write(sp);
        break;
    case 3:
        // kprintf("syscall exec\n");
        exec(sp);
        break;
    case 4:
        // kprintf("syscall fork\n");
        thread_fork(sp, pc);
        break;
    case 5:
        // kprintf("syscall exit\n");
        thread_exit();
        break;
    case 6:
        // kprintf("syscall mailbox\n");
        mailbox_syscall(sp);
        break;
    case 7:
        // kprintf("syscall kill\n");
        thread_kill(sp);
        break;
    case 8:
        // kprintf("syscall schedule\n");
        thread_schedule();
        break;
    default:
        kprintf("syscall unknown\n");
        while(1);
        break;
    }
}
