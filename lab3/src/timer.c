#include "timer.h"
#include "mini_uart.h"

void print_cpu_time(){
    unsigned long long cycles = get_cpu_cycles();
    unsigned long long freq = get_cpu_frequency();
    uart_printf("CPU Time: %d sec\n", cycles/freq);
}

void set_timer(unsigned long sec){
    unsigned long cycles = get_cpu_frequency() * sec;
    set_timer_expired(cycles);
}

void timer_handler(){
    uart_printf("CPU time: %d sec\n", get_cpu_cycles()/get_cpu_frequency());
    set_timer(1000);
}
