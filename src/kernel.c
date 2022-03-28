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
#include "uart.h"

extern void el2_entry();
extern void el1_entry();
extern void core_timer_enable(int s);

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

void idle() {
  printf("Ideling\n");
  while (!list_empty(deadqueue)) {
    struct task *task = list_entry(deadqueue->next, struct task, list);
    page_free(task->stack);
    kfree(task);
    __list_del(task->list.prev, task->list.next);
  }
  schedule();
}
void first() { sys_exec("syscall.img", NULL); }

void init(struct fdt_header *fdt) {
  uart_init();

  fdt_traverse(fdt, get_initramfs);

  // get_board_revision();
  // get_memory();
  simple_alloc_init();
  buddy_system_init();
  slabs_init();
  multitasking_init();
  el2_entry();
  struct task *idle = task_create((uint64_t)&idle);
  struct task *first_task = task_create((uint64_t)&first);
  task_run(idle);
}
