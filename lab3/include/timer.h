#ifndef __TIMER_H
#define __TIMER_H

extern unsigned long long get_cpu_cycles();
extern unsigned long long get_cpu_frequency();
extern void enable_core_timer();
extern void set_timer_expired(unsigned long cycles);
extern void set_timer_limit(unsigned long cycles);
extern void set_cpu_cycles(unsigned long cycles);

unsigned long long get_cpu_time();
void print_cpu_time();

void timer_handler();
void timer_init();
int timer_add(unsigned long sec, void (*callback)(void *), void* arg);


#endif