#ifndef __TIMER_H
#define __TIMER_H

extern int get_cpu_cycles();
extern int get_cpu_frequency();
extern void enable_core_timer();
extern void set_timer_expired(unsigned long cycles);
extern void set_timer_limit(unsigned long cycles);

void print_cpu_time();
void set_timer(unsigned long sec);
void timer_handler();


#endif