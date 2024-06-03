#include "arm/sysregs.h"

// LAB3-1
.globl el2_to_el1
el2_to_el1:
    // disable MMU
    ldr     x1, =SCTLR_VALUE_MMU_DISABLED
    msr     sctlr_el1, x1
    // make el0, el1 can use Floating point and Advanced SIMD
    ldr     x1, =CPACR_EL1_VALUE
    msr     CPACR_EL1, x1
    // set AArch64 for el2
    ldr     x1, =HCR_EL2_VALUE
    msr     hcr_el2, x1
    // mask all interrupt, and set interrupt level to el1h
    ldr     x1, =SPSR_EL2_VALUE
    msr     spsr_el2, x1
    msr     elr_el2, lr
    eret

// LAB3-2
.globl branch_el1_to_el0
branch_el1_to_el0:
    mov x2, 0
    // msr modify special register
    msr spsr_el1, x2
    // x0 is addr of user program
    msr elr_el1, x0
    // x1 is sp of user program
    msr sp_el0, x1
    eret


.macro	ventry	label
.align	7
b	\label
.endm

// save general registers to "kernel"stack
.macro save_all
    sub sp, sp, 32 * 9
    stp x0, x1, [sp ,16 * 0]
    stp x2, x3, [sp ,16 * 1]
    stp x4, x5, [sp ,16 * 2]
    stp x6, x7, [sp ,16 * 3]
    stp x8, x9, [sp ,16 * 4]
    stp x10, x11, [sp ,16 * 5]
    stp x12, x13, [sp ,16 * 6]
    stp x14, x15, [sp ,16 * 7]
    stp x16, x17, [sp ,16 * 8]
    stp x18, x19, [sp ,16 * 9]
    stp x20, x21, [sp ,16 * 10]
    stp x22, x23, [sp ,16 * 11]
    stp x24, x25, [sp ,16 * 12]
    stp x26, x27, [sp ,16 * 13]
    stp x28, x29, [sp ,16 * 14]
    str x30, [sp, 16 * 15]
    // lab3-6-2 enable nested interrupt
    mrs x0, spsr_el1
    mrs x1, elr_el1
    str x0, [sp, 16 * 16]
    str x1, [sp, 16 * 17]
    ldp x0, x1, [sp, 16 * 0]
.endm

// load general registers from stack
.macro load_all
    ldp x0, x1, [sp ,16 * 0]
    ldp x2, x3, [sp ,16 * 1]
    ldp x4, x5, [sp ,16 * 2]
    ldp x6, x7, [sp ,16 * 3]
    ldp x8, x9, [sp ,16 * 4]
    ldp x10, x11, [sp ,16 * 5]
    ldp x12, x13, [sp ,16 * 6]
    ldp x14, x15, [sp ,16 * 7]
    ldp x16, x17, [sp ,16 * 8]
    ldp x18, x19, [sp ,16 * 9]
    ldp x20, x21, [sp ,16 * 10]
    ldp x22, x23, [sp ,16 * 11]
    ldp x24, x25, [sp ,16 * 12]
    ldp x26, x27, [sp ,16 * 13]
    ldp x28, x29, [sp ,16 * 14]
    ldr x30, [sp, 16 * 15]

    // lab3-6-2 enable nested interrupt
    ldr x0, [sp ,16 * 16]
    ldr x1, [sp, 16 * 17]
    msr spsr_el1, x0
    msr elr_el1, x1
    ldp x0, x1, [sp ,16 * 0]
    add sp, sp, 32 * 9
.endm


.macro irq_handler_wrapper name link
.globl \name
\name:
    save_all
    // Lab5: let handler can access frame
    mov x0, sp
    // get current program counter
    adr x1, .
    bl \link
    load_all
    eret
.endm

// LAB3-3
irq_handler_wrapper entry_el_spel0_sync    el_spel0_sync
irq_handler_wrapper entry_el_spel0_irq     el_spel0_irq
irq_handler_wrapper entry_el_spel0_fiq     el_spel0_fiq
irq_handler_wrapper entry_el_spel0_error   el_spel0_error

irq_handler_wrapper entry_el_spelx_sync    el_spelx_sync
irq_handler_wrapper entry_el_spelx_irq     el_spelx_irq
irq_handler_wrapper entry_el_spelx_fiq     el_spelx_fiq
irq_handler_wrapper entry_el_spelx_error   el_spelx_error

.global entry_elh_64_sync
entry_elh_64_sync:
    save_all
    mov x0, sp
    adr x1, .
    bl  elh_64_sync
    load_all
    eret

irq_handler_wrapper entry_elh_64_irq       elh_64_irq
irq_handler_wrapper entry_elh_64_fiq       elh_64_fiq
irq_handler_wrapper entry_elh_64_error     elh_64_error

irq_handler_wrapper entry_elh_32_sync      elh_32_sync
irq_handler_wrapper entry_elh_32_irq       elh_32_irq
irq_handler_wrapper entry_elh_32_fiq       elh_32_fiq
irq_handler_wrapper entry_elh_32_error     elh_32_error

// you should notice 

.align 11 // align to 0x800
.globl exception_vector_table
exception_vector_table:
    // el not change, using sp_el0
    ventry      entry_el_spel0_sync             // synchronos exception
    ventry      entry_el_spel0_irq              // IRQ and vIRQ exception
    ventry      entry_el_spel0_fiq              // FIQ and vFIQ exception
    ventry      entry_el_spel0_error            // SError and vSError
    // el not change, using sp_elx
    ventry      entry_el_spelx_sync             // synchronos exception
    ventry      entry_el_spelx_irq              // IRQ and vIRQ exception
    ventry      entry_el_spelx_fiq              // FIQ and vFIQ exception
    ventry      entry_el_spelx_error            // SError and vSError
    // Moves to a higher el, from aarch64
    ventry      entry_elh_64_sync               // synchronos exception
    ventry      entry_elh_64_irq                // IRQ and vIRQ exception
    ventry      entry_elh_64_fiq                // FIQ and vFIQ exception
    ventry      entry_elh_64_error              // SError and vSError
    // Moves to a higher el, from aarch32
    ventry      entry_elh_32_sync               // synchronos exception
    ventry      entry_elh_32_irq                // IRQ and vIRQ exception
    ventry      entry_elh_32_fiq                // FIQ and vFIQ exception
    ventry      entry_elh_32_error              // SError and vSError

.globl set_exception_vector_table
set_exception_vector_table:
    adr x0, exception_vector_table
    msr vbar_el1, x0
    ret



/** switch_to() - switch context and change tpidr_el1
*   @ptid_ptr: addr of context to store
*   @ntix_ptr: addr of context to load
*   @next_tid: next thread id
*/
.global switch_to
switch_to:
    stp x19, x20, [x0, 16 * 0]
    stp x21, x22, [x0, 16 * 1]
    stp x23, x24, [x0, 16 * 2]
    stp x25, x26, [x0, 16 * 3]
    stp x27, x28, [x0, 16 * 4]
    stp  fp,  lr, [x0, 16 * 5]
    mov  x9,  sp
    mrs x10, sp_el0
    stp  x9, x10, [x0, 16 * 6]

    ldp x19, x20, [x1, 16 * 0]
    ldp x21, x22, [x1, 16 * 1]
    ldp x23, x24, [x1, 16 * 2]
    ldp x25, x26, [x1, 16 * 3]
    ldp x27, x28, [x1, 16 * 4]
    ldp  fp,  lr, [x1, 16 * 5]
    ldp  x9, x10, [x1, 16 * 6]
    mov  sp,  x9
    msr    sp_el0, x10
    msr tpidr_el1, x2

    ret

.global get_cur_tid
get_cur_tid:
    mrs x0, tpidr_el1
    ret

.global load_all_sp
load_all_sp:
    load_all
    eret
