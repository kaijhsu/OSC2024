#include "timer.h"
#include "mini_uart.h"

typedef struct timer_t {
    unsigned long long periods;
    void (* callback)(void *);
    struct timer_t* next;
    void* arg;
} timer;

// Define how frequent timer happens in a second
#define TIMER_FREQUENCY 20
unsigned long long timer_period_cnt = 0;
unsigned long long timer_period     = 0;

#define TIMER_POOL_SIZE 32
timer timer_pool[TIMER_POOL_SIZE];
timer* timer_st = 0;

unsigned long long get_cpu_time(){
    return timer_period_cnt/TIMER_FREQUENCY;
}

void print_cpu_time(){
    uart_printf("CPU Time: %d sec\n", get_cpu_time());
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

void timer_init(){
    timer_period_cnt = 0;
    timer_period = get_cpu_frequency()/TIMER_FREQUENCY;
    enable_core_timer();
    set_timer_expired(timer_period);
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
        debug(); // timer_pool is full
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
            t->periods = head->periods;
            t->callback = head->callback;
            t->next = head->next;
            head->periods = tmp_period;
            head->callback = tmp_callback;
            head->next = t;
            return 0;
        }

        // inset at last element
        if(!head->next || !head->next->callback){
            head->next = t;
            return 0;
        }
        head = head->next;
    }
    debug(); // never go here
    return -1;
}