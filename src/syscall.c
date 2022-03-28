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

int sys_fork() {
  struct task *task = get_current_task();
  struct task *child_task = task_create(get_page_addr(task->code));

  // Copy context
  memcpy(child_task, task, sizeof(uint64_t) * 10);
  child_task->code = task->code;
  child_task->lr = task->lr;
  child_task->sp =
      get_page_addr(task->stack) - task->sp + get_page_addr(child_task->stack);
  child_task->fp =
      get_page_addr(task->stack) - task->fp + get_page_addr(child_task->stack);
  child_task->trap_frame =
      (struct trap_frame *)(get_page_addr(task->stack) -
                            (uint64_t)task->trap_frame +
                            get_page_addr(child_task->stack));
  child_task->trap_frame->x0 = 0;
  task->trap_frame->x0 = child_task->pid;
  return 0;
}

void sys_exit() {
  struct task *task = get_current_task();
  task->status = TASK_DEAD;
  __list_del(task->list.prev, task->list.next);
  list_add(&task->list, deadqueue);
  schedule();
}
