#include "timer.h"
#include "mini_uart.h"
#include "thread.h"

typedef struct timer_t {
    uint64_t periods;
    void (* callback)(void *);
    struct timer_t* next;
    void* arg;
} timer;

// Define how frequent timer happens in a second
#define TIMER_FREQUENCY (1 << 5)
uint64_t timer_period_cnt = 0;
uint64_t timer_period     = 0;

#define TIMER_POOL_SIZE 8
timer timer_pool[TIMER_POOL_SIZE];
timer* timer_st = 0;

/**
 * get_cpu_time() - Return cpu time in second
*/
uint64_t get_cpu_time(){
    return timer_period_cnt/TIMER_FREQUENCY;
}

void print_cpu_time(){
    uart_printf("CPU Time: %d sec\n", get_cpu_time());
}

void timer_init(){
    uint64_t tmp;
    asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
    
    timer_period_cnt = 0;
    timer_period = get_cpu_frequency()/TIMER_FREQUENCY;
    set_core_timer(1);
    set_timer_expired(timer_period);
}

void timer_handler(){
    timer_period_cnt++;
    while(timer_st && timer_st->callback && timer_period_cnt >= timer_st->periods){
        timer_st->callback(timer_st->arg);
        timer_st->periods = -1;
        timer_st->callback = 0;
        timer_st->arg = 0;
        timer_st = timer_st->next;
    }
    set_timer_expired(timer_period);
    return;
}

int timer_add(unsigned long sec, void (* callback)(void *), void* arg){
    timer *t = 0;
    for(int i=0; i<TIMER_POOL_SIZE; ++i){
        if(timer_pool[i].callback == 0){
            t = &timer_pool[i];
            t->callback = callback;
            t->periods = timer_period_cnt + sec * TIMER_FREQUENCY;
            t->arg = arg;
            t->next = 0;
            break;
        }
    }
    if(t == 0){
        debug("timer_pool is full"); // timer_pool is full
        return -1;
    }

    // if there is no timer, replace timer_st
    if(!timer_st || !timer_st->callback){
        timer_st = t;
        return 0;
    }

    // insert new timer 
    timer *head = timer_st;
    while(head && head->callback){
        if(head->periods > t->periods){
            // swap and insert between head and head->next
            unsigned long tmp_period = t->periods;
            void (* tmp_callback) = t->callback;
            void *tmp_arg = t->arg;
            t->periods = head->periods;
            t->callback = head->callback;
            t->next = head->next;
            t->arg = head->arg;
            head->periods = tmp_period;
            head->callback = tmp_callback;
            head->next = t;
            head->arg = tmp_arg;
            return 0;
        }

        // inset at last element
        if(!head->next || !head->next->callback){
            head->next = t;
            return 0;
        }
        head = head->next;
    }
    debug("Timer unexpected!"); // never go here
    return -1;
}

void delay_ms(uint64_t ms){
    uint64_t start_cycle = get_cpu_cycles();
    uint64_t end_cycle = start_cycle + get_cpu_frequency()/1000 * ms;
    while(get_cpu_cycles() < end_cycle);
    return ;
}