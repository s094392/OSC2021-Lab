#include "trap.h"
#include "stdio.h"
#include "syscall.h"
#include "task.h"
#include "timer.h"
#include <stdint.h>

int get_current_el() {
  int el;
  asm volatile("mrs %0, CurrentEL" : "=r"(el));
  return el >> 2;
}

void syscall_handler(struct trap_frame *trap_frame) {
  // enable_timer_interrupt();
  struct task *task = get_current_task();
  task->trap_frame = trap_frame;
  switch (trap_frame->x8) {

  case 0: // getpid
    trap_frame->x0 = sys_getpid();
    break;

  case 1: // uartread
    trap_frame->x0 = sys_uartread((char *)trap_frame->x0, trap_frame->x1);
    break;

  case 2: // uartwrite
    trap_frame->x0 =
        sys_uartwrite((const char *)trap_frame->x0, trap_frame->x1);
    break;

  case 3: // exec
    trap_frame->x0 =
        sys_exec((const char *)trap_frame->x0, (char *const(*))trap_frame->x1);
    break;

  case 4: // fork
    sys_fork();
    break;

  case 5: // exit
    sys_exit();
    break;

  case 6: // mbox_call
    trap_frame->x0 =
        sys_mbox_call(trap_frame->x0, (unsigned int *)trap_frame->x1);
    break;

  case 7: // exit
    sys_kill(trap_frame->x0);
    break;
  }
  // disable_timer_interrupt();
}

void synchronize_handler(uint64_t esr, uint64_t elr,
                         struct trap_frame *trap_frame) {

  int iss = esr & ((1 << 24) - 1);
  switch (iss) {
  case 0:
    enable_timer_interrupt();
    syscall_handler(trap_frame);
    disable_timer_interrupt();
    break;
  }
}

void irq_handler(uint64_t esr, uint64_t elr, struct trap_frame *trap_frame) {
  core_timer_handler(5);
  schedule();
}
