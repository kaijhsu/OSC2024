#ifndef __TIMER_H
#define __TIMER_H

#include "mm.h"

extern uint64_t get_cpu_cycles();
extern uint64_t get_cpu_frequency();
extern void set_core_timer(int enable);
extern void set_timer_expired(uint64_t cycles);
extern void set_timer_limit(uint64_t cycles);
extern void set_cpu_cycles(uint64_t cycles);

uint64_t get_cpu_time();
void print_cpu_time();

void timer_handler();
void timer_init();
int timer_add(unsigned long sec, void (*callback)(void *), void* arg);

void delay_ms(uint64_t ms);

#endif