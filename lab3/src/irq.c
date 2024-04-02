#include "exception.h"
#include "peripherals/mini_uart.h"
#include "peripherals/irq.h"
#include "timer.h"
#include "utils.h"
#include "mini_uart.h"

void set_interrupt_el1(int enable){
    if(enable) 
        asm("msr DAIFClr, 0xf");
    else
        asm("msr DAIFSet, 0xf");
}

void irq_default(){
    debug();
}

void irq_64_el0(){
    debug();
}

void gpu_irq_handler(){
    unsigned int pending = get32(IRQ_PENDING_1);
    if(pending & AUX_INT)
        uart_irq_handler();
}

void irq_64_el1(){
    set_interrupt_el1(0);
    unsigned int irq = get32(CORE0_IRQ_SRC);
    switch (irq){
        case SYSTEM_TIMER_IRQ_1:
            timer_handler();
            break;
        case GPU_IRQ:
            gpu_irq_handler();
            break;
        default:
            uart_printf("unknown irq %x\n", irq);
            debug();
            break;
    }
    set_interrupt_el1(1);
}

void irq_system_call(){
    debug();
}