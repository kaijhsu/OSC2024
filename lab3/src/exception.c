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

void gpu_irq_handler(){
    unsigned int pending = get32(IRQ_PENDING_1);
    if(pending & AUX_INT)
        uart_irq_handler();
}


void el_spel0_sync(){
    debug();
}

void el_spel0_irq(){
    debug();
}

void el_spel0_fiq(){
    debug();
}

void el_spel0_error(){
    debug();
}

void el_spelx_sync(){
    debug();
    set_interrupt_el1(0);
    unsigned int irq = get32(CORE0_IRQ_SRC);
    switch (irq){
        case SYSTEM_TIMER_IRQ_1:
            timer_handler();
            break;
        default:
            uart_printf("unknown irq %x\n", irq);
            break;
    }
    set_interrupt_el1(1);
}

void el_spelx_irq(){
    set_interrupt_el1(0);
    unsigned int irq = get32(CORE0_IRQ_SRC);
    switch (irq){
        case SYSTEM_TIMER_IRQ_1:
            timer_handler();
            break;
        default:
            uart_printf("unknown irq %x\n", irq);
            debug();
            break;
    }
    set_interrupt_el1(1);
}

void el_spelx_fiq(){
    debug();
}

void el_spelx_error(){
    debug();
}

void elh_64_sync(){
    debug();
}

void elh_64_irq(){
    debug();
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

void elh_64_fiq(){
    debug();
}

void elh_64_error(){
    debug();
}

void elh_32_sync(){
    debug();
}

void elh_32_irq(){
    debug();
}

void elh_32_fiq(){
    debug();
}

void elh_32_error(){
    debug();
}
