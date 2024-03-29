.global el2_entry
.global fork_child_entry
.global to_el0
.global enable_timer_interrupt
.global disable_timer_interrupt

enable_timer_interrupt:
    msr DAIFClr, 0xf
    ret

disable_timer_interrupt:
    msr DAIFSet, 0xf
    ret


to_el0:  
    msr     elr_el1, x0
    msr     sp_el0, x1


    mov     x3, #0
    msr     spsr_el1, x3

    dsb ish // ensure write has completed
    msr ttbr0_el1, x2 // switch translation based address.
    tlbi vmalle1is // invalidate all TLB entries
    dsb ish // ensure completion of TLB invalidatation

    // set up exception handlers
    ldr     x2, =exception_table
    msr     vbar_el1, x2

    isb // clear pipeline

    eret

.macro kernel_entry
    str   x0, [sp, #-8]!
    str   x1, [sp, #-8]!
    str   x2, [sp, #-8]!
    str   x3, [sp, #-8]!
    str   x4, [sp, #-8]!
    str   x5, [sp, #-8]!
    str   x6, [sp, #-8]!
    str   x7, [sp, #-8]!
    str   x8, [sp, #-8]!
    str   x9, [sp, #-8]!
    str   x10, [sp, #-8]!
    str   x11, [sp, #-8]!
    str   x12, [sp, #-8]!
    str   x13, [sp, #-8]!
    str   x14, [sp, #-8]!
    str   x15, [sp, #-8]!
    str   x16, [sp, #-8]!
    str   x17, [sp, #-8]!
    str   x18, [sp, #-8]!
    str   x19, [sp, #-8]!
    str   x20, [sp, #-8]!
    str   x21, [sp, #-8]!
    str   x22, [sp, #-8]!
    str   x23, [sp, #-8]!
    str   x24, [sp, #-8]!
    str   x25, [sp, #-8]!
    str   x26, [sp, #-8]!
    str   x27, [sp, #-8]!
    str   x28, [sp, #-8]!
    str   x29, [sp, #-8]!
    str   x30, [sp, #-8]!

    mrs   x21, sp_el0
    str   x21, [sp, #-8]!
    mrs   x21, elr_el1
    str   x21, [sp, #-8]!
    mrs   x21, spsr_el1
    str   x21, [sp, #-8]!
  .endm

.macro kernel_exit
    ldr   x21, [sp], #8  
    msr   spsr_el1, x21
    ldr   x21, [sp], #8  
    msr   elr_el1, x21
    ldr   x21, [sp], #8  
    msr   sp_el0, x21

    ldr   x30, [sp], #8  
    ldr   x29, [sp], #8  
    ldr   x28, [sp], #8  
    ldr   x27, [sp], #8  
    ldr   x26, [sp], #8  
    ldr   x25, [sp], #8  
    ldr   x24, [sp], #8  
    ldr   x23, [sp], #8  
    ldr   x22, [sp], #8  
    ldr   x21, [sp], #8  
    ldr   x20, [sp], #8  
    ldr   x19, [sp], #8  
    ldr   x18, [sp], #8  
    ldr   x17, [sp], #8  
    ldr   x16, [sp], #8  
    ldr   x15, [sp], #8  
    ldr   x14, [sp], #8  
    ldr   x13, [sp], #8  
    ldr   x12, [sp], #8  
    ldr   x11, [sp], #8  
    ldr   x10, [sp], #8  
    ldr   x9, [sp], #8  
    ldr   x8, [sp], #8  
    ldr   x7, [sp], #8  
    ldr   x6, [sp], #8  
    ldr   x5, [sp], #8  
    ldr   x4, [sp], #8  
    ldr   x3, [sp], #8  
    ldr   x2, [sp], #8  
    ldr   x1, [sp], #8  
    ldr   x0, [sp], #8
.endm

_exc_handler:
  kernel_entry
  bl synchronize_handler
  kernel_exit
  eret

_fiq_handler:
  kernel_entry
  bl synchronize_handler
  kernel_exit
  eret

_syn_handler:
  kernel_entry
  mrs x0, esr_el1
  mrs x1, elr_el1
  mov x2, sp
  bl synchronize_handler

fork_child_entry:
  kernel_exit
  eret


_irq_handler:
  kernel_entry
  mrs x0, esr_el1
  mrs x1, elr_el1
  mov x2, sp
  bl irq_handler
  kernel_exit
  eret

.align 11
.global exception_table
exception_table:
    .align  7
    b       _syn_handler
    .align  7
    b       _irq_handler
    .align  7
    b       _fiq_handler
    .align  7
    b       _exc_handler

    .align  7
    b       _syn_handler
    .align  7
    b       _irq_handler
    .align  7
    b       _fiq_handler
    .align  7
    b       _exc_handler
    .align  7

    b       _syn_handler
    .align  7
    b       _irq_handler
    .align  7
    b       _fiq_handler
    .align  7
    b       _exc_handler


    .align  7
    b       _syn_handler
    .align  7
    b       _irq_handler
    .align  7
    b       _fiq_handler
    .align  7
    b       _exc_handler
