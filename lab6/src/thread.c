#include "thread.h"
#include "initramfs.h"
#include "exception.h"

struct list_head ready_queue;
struct list_head exit_queue;
thread_info* cur_thread;
thread_info* threads[MAX_THREADS];
uint8_t enable_schedule = 0;


int32_t get_next_tid(){
    for(int i=0; i<MAX_THREADS; ++i)
        if(threads[i] == NULL) return i;
    kprintf("no availiable threads");
    return -1;
}

void thread_schedule(){
    if(!enable_schedule)
        return;

    thread_info* next_thread;
    // choose next thread
    
    // no other thread, keep running
    if(list_empty(&ready_queue)){
        if(cur_thread->state == EXIT) debug("error");
        return;
    }
    
    next_thread = list_entry(ready_queue.next, thread_info, node);
    list_del(&next_thread->node);
    
    // place current thread to exit or ready or block
    if(cur_thread->state == RUN){
        list_add_tail(&cur_thread->node, &ready_queue);
        cur_thread->state = READY;
    }
    else if(cur_thread->state == EXIT)
        list_add_tail(&cur_thread->node, &exit_queue);
    else
        kprintf("shouldn't go here\n");

    // context switch
    thread_info* prev_thread = cur_thread;
    cur_thread = next_thread;
    cur_thread->state = RUN;
    
    // kprintf("schedule %x -> %x\n", prev_thread->id, next_thread->id);
    // switch
    switch_to(prev_thread, next_thread, next_thread->id);
}

void thread_wrapper(){
    branch_el1_to_el0(cur_thread->context, cur_thread->el0_sp);
    cur_thread->state = EXIT;
    asm("mov x8, 8");
    asm("svc 0");
}



int thread_create(void *fptr){
    kprintf(STYLE("not implement yet", FONT_RED));
    
    // thread_info* new_thread = malloc(sizeof(thread_info));
    // if(new_thread == 0)
    //     throw(-1, "fail to allocate mem for new thread");

    // // initailze id lr mem sp fp
    // new_thread->id  = get_next_tid();
    // if(new_thread->id < 0) 
    //     throw(-1, "fail to get new tid");
    
    // new_thread->lr   = thread_wrapper;
    // new_thread->stack_size = cur_thread->;
    // new_thread->stack  = mframe_alloc(4096);
    
    // if(new_thread->stack == 0)
    //     throw(-1, "fail to get page frame");
    
    // new_thread->sp = new_thread->mem + 4096;
    // new_thread->fp = new_thread->sp;

    // threads[new_thread->id] = new_thread;

    // // set state
    // new_thread->state = READY;
    // list_add_tail(&(new_thread->node), &ready_queue);

    // return new_thread->id;
    return 0;
}

void thread_exit(){
    cur_thread->state = EXIT;
    thread_schedule();
}

void thread_kill(uint64_t* sp){
    uint32_t target = sp[0];
    if(target >= 64 || threads[target] == NULL)
        throw(, "no such thread id %d", target);
    threads[target]->state = EXIT;
    list_del(&threads[target]->node);
    list_add_tail(&threads[target]->node, &exit_queue);
}

int thread_fork_return(){
    return 0;
}

extern void entry_elh_64_sync();


void thread_fork(uint64_t *sp, void *pc){
    set_interrupt_el1(0);

    // prepare require date
    int new_id = get_next_tid();
    if(new_id < 0) throw(, "thread full");
    
    thread_info* new_thread = (thread_info*)malloc(sizeof(thread_info));
    if(new_thread == NULL) throw(, "malloc out of memory");
    
    void* new_stack = mframe_alloc(cur_thread->stack_size);
    if(new_stack == NULL) throw(, "page out of memeory");
    
    void* new_kernel_stack = mframe_alloc(cur_thread->kernel_stack_size);
    if(new_kernel_stack == NULL) throw(, "page out of memory free %x", mframe_free(new_stack));


    void *sp_el0;
    asm volatile("mrs %0, sp_el0": "=r"(sp_el0));
    new_thread->fp =     new_stack + (sp_el0 - cur_thread->stack);
    new_thread->el0_sp = new_stack + (sp_el0 - cur_thread->stack);

    new_thread->el1_sp = (void*)((uint64_t)new_kernel_stack + (uint64_t)sp - (uint64_t)cur_thread->kernel_stack);
    new_thread->context = cur_thread->context;
    new_thread->context_size = cur_thread->context_size;
    new_thread->kernel_stack = new_kernel_stack;
    new_thread->kernel_stack_size = cur_thread->kernel_stack_size;  
    new_thread->stack = new_stack;
    new_thread->stack_size = cur_thread->stack_size;


    new_thread->id = new_id;
    new_thread->parent = cur_thread->id;
    new_thread->state = READY;

    // access global data;
    threads[new_thread->id] = new_thread;
    list_add_tail(&new_thread->node, &ready_queue);

    // for mother thread return child id
    sp[0] = new_thread->id;

    memcopy(cur_thread->x19_28, new_thread->x19_28, sizeof(new_thread->x19_28));
    memcopy(cur_thread->stack, new_thread->stack, cur_thread->stack_size);
    memcopy(cur_thread->kernel_stack, new_thread->kernel_stack, cur_thread->kernel_stack_size);

    // for child thread return 0
    *(uint64_t *)(new_thread->kernel_stack + ((void*)sp - cur_thread->kernel_stack)) = 0;
    
    // decide next pc child thread run 
    new_thread->lr = pc+8;
    set_interrupt_el1(1);
}

void thread_exec(cpio_path path){
    if(cur_thread->context_size < path.filesize * 2)
        throw(,"the program is too large to exec");   
    cur_thread->fp = cur_thread->stack + cur_thread->stack_size;
    cur_thread->lr = thread_wrapper;
    cur_thread->el1_sp = cur_thread->kernel_stack + cur_thread->kernel_stack_size;
    cur_thread->el0_sp = cur_thread->stack + cur_thread->stack_size;

    kprintf(STYLE("no implement yet", FONT_RED));
}

int kernel_load(cpio_path path){
    // allocate memory 
    uint32_t new_tid = get_next_tid();
    if(new_tid < 0) throw(-1, "no tid available");

    size_t context_size = path.filesize << 2;
    void* context = mframe_alloc(context_size);
    if(context == NULL) throw(-1, "page out of memory");

    size_t stack_size = context_size;
    void* stack = mframe_alloc(stack_size);
    if(stack == NULL) throw(-1, "page out of memory, free %x", mframe_free(context));

    size_t kernel_stack_size = PAGE_SIZE << 1;
    void* kernel_stack = mframe_alloc(kernel_stack_size); 
    if(kernel_stack == NULL) throw(-1, "page out of memory free %x %x", mframe_free(context), mframe_free(stack));

    thread_info* new_thread = (thread_info*)malloc(sizeof(thread_info));
    if(new_thread == NULL) throw(-1, "heap out of memory, free %x %x", mframe_free(context), mframe_free(stack), mframe_free(kernel_stack));

    // setting up thread
    new_thread->fp = stack + stack_size;
    new_thread->lr = thread_wrapper;
    new_thread->el1_sp = kernel_stack + kernel_stack_size;
    new_thread->el0_sp = stack + stack_size;
    new_thread->context = context;
    new_thread->context_size = context_size;
    new_thread->kernel_stack = kernel_stack;
    new_thread->kernel_stack_size = kernel_stack_size;
    new_thread->stack = stack;
    new_thread->stack_size = stack_size;
    new_thread->id = new_tid;
    new_thread->parent = new_tid;
    new_thread->state = READY;

    memcopy(path.data, new_thread->context, path.filesize);

    threads[new_thread->id] = new_thread;
    list_add(&new_thread->node, &ready_queue);
    return 0;
}

int thread_clean(){
    while(!list_empty(&exit_queue)){
        thread_info* target = list_entry(exit_queue.next, thread_info, node);
        list_del(&target->node);
        threads[target->id] = NULL;
        // mframe_free(target->context);
        // mframe_free(target->stack);
        // mframe_free(target->kernel_stack);
        kprintf("Clean "STYLE("EXIT", FONT_GREEN)" thread: %d\n", target->id);
        mfree(target);
    }
    return 0;
}

int kernel_schedule(){
    kprintf("kernel start schedule\n");
    enable_schedule = 1;
    while(1){
        thread_schedule();
        enable_schedule = 0;
        thread_clean();
        if(uart_last_input() == 27 || list_empty(&ready_queue)) // 27 == ESC
            break;
        enable_schedule = 1;
    }
    return 0;
}


extern char _heap_begin;

int thread_init(){
    INIT_LIST_HEAD(&ready_queue);
    // INIT_LIST_HEAD(&block_queue);
    INIT_LIST_HEAD(&exit_queue);
    
    // kernel thread
    cur_thread = (thread_info*)malloc(sizeof(thread_info));
    cur_thread->id = 0;
    cur_thread->context = (void*)0x80000;
    cur_thread->context_size = ((size_t)&_heap_begin) - 0x80000;
    cur_thread->kernel_stack = (void*)0x80000;
    cur_thread->kernel_stack_size = 0x40000;
    cur_thread->parent = 0;
    INIT_LIST_HEAD(&cur_thread->node);

    threads[0] = cur_thread;
    cur_thread->state = RUN;

    return 0;
}
