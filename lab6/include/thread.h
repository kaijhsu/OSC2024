#ifndef _THREAD_H__
#define _THREAD_H__

#include "list.h"
#include "mm.h"
#include "mini_uart.h"
#include "timer.h"
#include "initramfs.h"
#include "mmu.h"

#define MAX_THREADS 64

enum THREAD_STATE {
    RUN,
    READY,
    BLOCK,
    EXIT
};

typedef struct thread_info_t{
    uint64_t x19_28[10];
    void*    fp;
    void*    lr;
    void*    el1_sp;
    void*    el0_sp;
    void*    context;
    uint64_t context_size;
    void*    kernel_stack;
    uint64_t kernel_stack_size;
    void*    stack;
    uint64_t stack_size;
    uint32_t id;
    uint32_t parent;
    uint64_t* ttbr0_el1;
    enum THREAD_STATE state;
    struct list_head node;
} thread_info;

/**
 * thread_init() - initialize thread system
 * Return 0 if success
*/
int thread_init();

/**
 * thread_create() - Create a work thread
 * @fptr: function pointer work thread executed
 * 
 * Return: >=0 thread tid
*/
int thread_create(void* fptr);

/**
 * thread_exit() - Exit a thread
*/
void thread_exit();

/**
 * thread_kill() - Kill a thread
 * @sp: frame pointer
*/
void thread_kill();

/**
 * schedule() - select next thread and do context switch 
*/
void thread_schedule();

/**
 * thread_fork() - fork a new thread, shall only call by syscall
 * @sp: frame pointer
*/
void thread_fork(uint64_t* sp, void* pc);

/**
 * thread_exec() - change program, modify context
 * @path: new program data
*/
void thread_exec(cpio_path path);

/**
 * kernel_load() - kernel declare a space and load program, create thread
 * @path: program file path
 * return: 0 if success, if not, fail`
*/
int kernel_load(cpio_path path);

/**
 * kernel_schedule() - only call by kernel, start schedule until ESC key or ready_queue is empty
*/
int kernel_schedule();


/**
 * switch_to() - switch from @curr to @next
 * @curr: current thread info
 * @next: next thread info
*/
extern void switch_to(thread_info* prev, thread_info* next, uint32_t next_id);


/**
 * get_cur_tid() - get current tid
 * Return
*/
extern uint64_t get_cur_tid();

/**
 * thread_copy() - copy thread
 * @src: source 
*/



#endif