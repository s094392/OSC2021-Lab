#include "trap.h"

#include <stdint.h>

#include "stdio.h"

extern void core_timer_handler();
extern void core_timer_enable(int s);

int get_current_el() {
  int el;
  asm volatile("mrs %0, CurrentEL" : "=r"(el));
  return el >> 2;
}

void synchronize_handler(uint64_t esr, uint64_t elr,
                         struct trap_frame_t *trap_frame) {
  printf("Current el: %d\n", get_current_el());
  int iss = esr & ((1 << 24) - 1);
  printf("0x%lx 0x%lx, iss: %d\n", esr, elr, iss);
  core_timer_enable(3);
}

void irq_handler(uint64_t esr, uint64_t elr, struct trap_frame_t *trap_frame) {
  printf("Timer!");
  core_timer_handler();
}
