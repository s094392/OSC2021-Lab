#include "syscall.h"
#include "alloc.h"
#include "cpio.h"
#include "stdio.h"
#include "string.h"
#include "task.h"
#include "trap.h"

int sys_getpid() {
  struct task *task = get_current_task();
  return task->pid;
}

size_t sys_uartread(char buf[], size_t size) {
  for (int i = 0; i < size; i++) {
    buf[i] = getchar();
  }
  return size;
}

size_t sys_uartwrite(const char buf[], size_t size) {
  for (int i = 0; i < size; i++) {
    putchar(buf[i]);
  }
  return size;
}

int sys_exec(const char *name, char *const argv[]) {
  struct cpio_newc_header *cpio_file = get_cpio_file(name);
  int cpio_filesize = get_file_size(cpio_file);
  void *cpio_data = get_file_data(cpio_file);
  struct task *task = get_current_task();
  task->code = page_alloc(1);
  void *code_addr = (void *)get_page_addr(task->code);
  memcpy(code_addr, cpio_data, cpio_filesize);
  el1_entry(code_addr);
  return 1;
}

int sys_fork() { return 0; }

void sys_exit() {}
