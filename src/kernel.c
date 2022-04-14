#include <byteswap.h>
#include <stdarg.h>

#include "alloc.h"
#include "cpio.h"
#include "dtb.h"
#include "list.h"
#include "mbox.h"
#include "shell.h"
#include "stdio.h"
#include "string.h"
#include "syscall.h"
#include "task.h"
#include "timer.h"
#include "uart.h"

extern void el1_entry();

void get_initramfs(char *key, void *data, int len) {
  if (!strncmp(key, "linux,initrd-start", 18)) {
    cpio_addr = (void *)(size_t)__bswap_32(*(uint32_t *)data);
  }
}

void user() {
  while (1) {
    shell();
  }
}

void first() { sys_exec("syscall.img", NULL); }

void init(struct fdt_header *fdt) {
  uart_init();
  fdt = (struct fdt_header *)((uint64_t)fdt | 0xffff000000000000);

  fdt_traverse(fdt, get_initramfs);

  simple_alloc_init();
  buddy_system_init();
  slabs_init();
  multitasking_init();
  el2_entry();

  uint64_t tmp;
  asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
  tmp |= 1;
  asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));

  core_timer_enable(5);
  enable_timer_interrupt();

  struct task *first_task = task_create((uint64_t)&first);

  task_run(first_task);
}
