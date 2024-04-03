#ifndef __EXCEPTION_H
#define __EXCEPTION_H
#include "mini_uart.h"

// LAB3-1
// implement in assembly
extern void el2_to_el1();
extern void branch_el1_to_el0(char *addr, char *sp);
extern void set_exception_vector_table();

void set_interrupt_el1(int enable);


#endif