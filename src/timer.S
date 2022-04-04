#define CORE0_TIMER_IRQ_CTRL 0x40000040

.global core_timer_enable
.global core_timer_handler

core_timer_enable:
  mov x1, 1
  msr cntp_ctl_el0, x1 // enable
  mrs x1, cntfrq_el0
  asr x1, x1, x0
  msr cntp_tval_el0, x1 // set expired time
  mov x1, 2
  ldr x2, =CORE0_TIMER_IRQ_CTRL
  str w1, [x2] // unmask timer interrupt
  ret

core_timer_handler:
  mrs x1, cntfrq_el0
  asr x1, x1, x0
  msr cntp_tval_el0, x1
  ret
