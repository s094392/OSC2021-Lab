#include "trap.h"
#include "stdio.h"
#include "syscall.h"
#include <stdint.h>

extern void core_timer_handler();
extern void core_timer_enable(int s);

int get_current_el() {
  int el;
  asm volatile("mrs %0, CurrentEL" : "=r"(el));
  return el >> 2;
}

void syscall_handler(struct trap_frame *trap_frame) {
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
    trap_frame->x0 = sys_fork();
    break;

  case 5: // exit
    sys_exit();
    break;
  }
}

void synchronize_handler(uint64_t esr, uint64_t elr,
                         struct trap_frame *trap_frame) {
  printf("Current el: %d\n", get_current_el());
  int iss = esr & ((1 << 24) - 1);
  printf("0x%lx 0x%lx, iss: %d\n", esr, elr, iss);
  switch (iss) {
  case 0:
    syscall_handler(trap_frame);
    break;
  }
  core_timer_enable(3);
}

void irq_handler(uint64_t esr, uint64_t elr, struct trap_frame *trap_frame) {
  printf("Timer!");
  core_timer_handler();
}
