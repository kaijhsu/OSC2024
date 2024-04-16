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

void el_spelx_irq(){
    set_interrupt_el1(0);
    unsigned long long irq = *(unsigned long long *)CORE0_IRQ_SRC;
    if(irq & SYSTEM_TIMER_IRQ_1){
        set_core_timer(0);
        timer_handler();
        set_core_timer(1);
        irq &= ~SYSTEM_TIMER_IRQ_1;
    }

    if(irq & GPU_IRQ){
        unsigned int pending = get32(IRQ_PENDING_1);
        if(pending & AUX_INT){
            unsigned int irq_src = get32(AUX_MU_IIR_REG);
            if(irq_src & AUX_MU_IIR_REG_WRITE){
                uart_irq_write();
            }
            else if(irq_src & AUX_MU_IIR_REG_READ){
                *(unsigned int *)AUX_MU_IER_REG &= ~(AUX_MU_IER_REG_RECEIVE_INTERRUPT);
                uart_irq_read();
            }
        }
        else{
            debug("unknown gpu irq pending %l", pending);
        }
        irq &= ~GPU_IRQ;
    }

    *(unsigned long long *)CORE0_IRQ_SRC = 0;
    if(irq) debug("unprocess irq %d", irq);
    set_interrupt_el1(1);
}

void el_spel0_sync(){
    debug("");
}

void el_spel0_irq(){
    debug("");
}

void el_spel0_fiq(){
    debug("");
}

void el_spel0_error(){
    debug("");
}

void el_spelx_sync(){
    set_interrupt_el1(0);
    static int cnt = 0;
    cnt ++;
    if(cnt > 3) while(1) asm volatile("nop");
    unsigned long long irq = *(unsigned long long *)CORE0_IRQ_SRC;
    debug("irq: %l", irq);
    debug("time: %l", *(unsigned long long *)CORE0_TIMER_CNT);
    debug("mailbox: %l", *(unsigned long long *)CORE0_MAILBOX_CNT);
    *(unsigned long long *)CORE0_IRQ_SRC = 0;
    set_interrupt_el1(1);
}

void el_spelx_fiq(){
    debug("");
}

void el_spelx_error(){
    debug("");
}

void elh_64_sync(){
    debug("");
}

void elh_64_irq(){
    debug("");
}

void elh_64_fiq(){
    debug("");
}

void elh_64_error(){
    debug("");
}

void elh_32_sync(){
    debug("");
}

void elh_32_irq(){
    debug("");
}

void elh_32_fiq(){
    debug("");
}

void elh_32_error(){
    debug("");
}
