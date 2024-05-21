#include "exception.h"
#include "peripherals/mini_uart.h"
#include "peripherals/irq.h"
#include "timer.h"
#include "utils.h"
#include "mini_uart.h"
#include "syscall.h"
#include "arm/sysregs.h"
#include "thread.h"

void set_interrupt_el1(int enable){
    if(enable)  asm("msr DAIFClr, 0xf");
    else        asm("msr DAIFSet, 0xf");
}

void el_spelx_irq(uint64_t* sp){
    set_interrupt_el1(0);
    unsigned long long irq = *(unsigned long long *)CORE0_IRQ_SRC;
    int schedule = 0;

    if(irq & SYSTEM_TIMER_IRQ_1){
        set_core_timer(0);
        timer_handler();
        schedule = 1;
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
    if(schedule)
        thread_schedule();

}

void el_spel0_sync(uint64_t* sp){
    debug("");
}

void el_spel0_irq(uint64_t* sp){
    debug("");
}

void el_spel0_fiq(uint64_t* sp){
    debug("");
}

void el_spel0_error(uint64_t* sp){
    debug("");
}

void el_spelx_sync(uint64_t* sp, void* pc){
    debug("");
    while(1);
}

void el_spelx_fiq(uint64_t* sp){
    debug("");
    while(1);
}

void el_spelx_error(uint64_t* sp){
    debug("");
    while(1);
}

void elh_64_sync(uint64_t* sp, void* pc){
    uint64_t esr;
    asm volatile("mrs %0, esr_el1":"=r"(esr));
    uint64_t ec = ESR_GET_EC(esr); 
    set_interrupt_el1(1);

    if(ec == ESR_EC_SVC)
        syscall_handler(sp, pc);
    else
        kprintf("error, unknown esr class %x\n", ec);
}

void elh_64_irq(uint64_t* sp){
    el_spelx_irq(sp);
}

void elh_64_fiq(uint64_t* sp){
    debug("");
}

void elh_64_error(uint64_t* sp){
    debug("");
}

void elh_32_sync(uint64_t* sp){
    debug("");
}

void elh_32_irq(uint64_t* sp){
    debug("");
}

void elh_32_fiq(uint64_t* sp){
    debug("");
}

void elh_32_error(uint64_t* sp){
    debug("");
}
