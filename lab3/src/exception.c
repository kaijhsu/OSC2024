#include "exception.h"
#include "peripherals/mini_uart.h"
#include "peripherals/irq.h"
#include "timer.h"
#include "utils.h"
#include "mini_uart.h"

typedef struct irq_task{
    unsigned char prio;
    void (* task)(void);
    struct irq_task* next; 
} irq_task;
#define TASK_POOL_SIZE 32
irq_task task_pool[TASK_POOL_SIZE];
irq_task *task_head = 0;
unsigned char task_cur_prio = 255;

int task_create(void(* task)(void), unsigned char prio){
    irq_task *ptr = 0;
    for(int i=0; i<TASK_POOL_SIZE; ++i){
        if(task_pool[i].task == 0){
            ptr = &task_pool[i];
            break;
        }
    }
    
    if(ptr == 0){
        debug("task pool full");
        task();
        return 0;
    }

    ptr->prio = prio;
    ptr->task = task;

    if(task_head == 0 || task_head->task == 0){
        task_head = ptr;
        return 0;
    }

    irq_task *head = task_head;
    while(head && head->task){
        if(head->prio > ptr->prio){
            // swap and insert between head and head->next
            unsigned char tmp_prio = ptr->prio;
            void (*tmp_task) = ptr->task;
            ptr->next = head->next;
            ptr->prio = head->prio;
            ptr->task = head->task;
            head->next = ptr;
            head->prio = tmp_prio;
            head->task = tmp_task;
            return 0;
        }

        // insert at last element
        if(!head->next || !head->next->task){
            head->next = ptr;
            return 0;
        }
        head = head->next;
    }
    debug("Unexpected!");
    task();
    return -1;
}

void task_run(){
    while(1){
        // critical section start
        set_interrupt_el1(0);
        if(task_head == 0 || task_head->task == 0){
            set_interrupt_el1(1);
            break;
        }
        irq_task *task = task_head;
        if(task_cur_prio <= task->prio){
            set_interrupt_el1(1);
            break;
        }
        task_head = task_head->next;
        unsigned char tmp_prio = task_cur_prio;
        set_interrupt_el1(1);
        // critical section end
        
        task->task();

        // critical section;
        set_interrupt_el1(0);
        task_cur_prio = tmp_prio;
        task->task = 0;
        task->next = 0;
        task->prio = 0;
        set_interrupt_el1(1);
    }
}



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
        task_create(timer_handler, TIMER_PRIO);
        irq &= ~SYSTEM_TIMER_IRQ_1;
    }

    if(irq & GPU_IRQ){
        unsigned int pending = get32(IRQ_PENDING_1);
        if(pending & AUX_INT){
            unsigned int irq_src = get32(AUX_MU_IIR_REG);
            if(irq_src & AUX_MU_IIR_REG_WRITE){
                uart_set_transmit_interrupt(0);
                task_create(uart_irq_write, UART_WRITE_PRIO);
            }
            else if(irq_src & AUX_MU_IIR_REG_READ){
                *(unsigned int *)AUX_MU_IER_REG &= ~(AUX_MU_IER_REG_RECEIVE_INTERRUPT);
                task_create(uart_irq_read, UART_READ_PRIO);
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
    task_run();
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
