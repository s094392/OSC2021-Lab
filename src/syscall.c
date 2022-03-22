#include "syscall.h"
#include "alloc.h"
#include "cpio.h"
#include "stdio.h"
#include "string.h"
#include "task.h"
#include "trap.h"

int sys_getpid() {
  struct task *task = get_current_task();
  printf("task: %x, %d\n", (uint64_t)task, task->pid);
  while (1)
    asm volatile("nop");
  return get_current_task()->pid;
}

size_t sys_uartread(char buf[], size_t size) {}
size_t sys_uartwrite(const char buf[], size_t size) {}
int sys_exec(const char *name, char *const argv[]) {
  struct cpio_newc_header *cpio_file = get_cpio_file(name);
  int cpio_filesize = get_file_size(cpio_file);
  void *cpio_data = get_file_data(cpio_file);
  void *code = (void *)get_page_addr(page_alloc(1));
  struct task *task = get_current_task();
  task->code = page_alloc(1);
  void *code_addr = (void *)get_page_addr(task->code);
  printf("filesize: %d, 0x%x, 0x%x\n", cpio_filesize, code_addr, task);
  memcpy(code_addr, cpio_data, cpio_filesize);
  el1_entry(code_addr);
}
int sys_fork() {}
void sys_exit() {}
