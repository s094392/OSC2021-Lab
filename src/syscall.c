#include "syscall.h"
#include "alloc.h"
#include "cpio.h"
#include "mbox.h"
#include "mem.h"
#include "stdio.h"
#include "string.h"
#include "task.h"
#include "trap.h"

extern const uint64_t fork_child_entry[];

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
  // extract data from cpio and move it to new page
  struct cpio_newc_header *cpio_file = get_cpio_file(name);
  int cpio_filesize = get_file_size(cpio_file);
  void *cpio_data = get_file_data(cpio_file);
  struct task *task = get_current_task();
  int order = 1;
  while (cpio_filesize > 0x1000 << order) {
    order++;
  }
  task->code = page_alloc(order);
  void *code_addr = (void *)get_page_addr(task->code);
  memcpy(code_addr, cpio_data, cpio_filesize);
  task->codesize = cpio_filesize;
  mappages((pagetable_t)PA2KA(task->pagetable), 0, cpio_filesize,
           (uint64_t)code_addr, PT_AF | PT_USER | PT_MEM | PT_RW);

  // jump to user program and with user stack
  to_el0(0, 0x0000fffffffff000, task->pagetable);
  return 1;
}

int sys_fork() {
  struct task *task = get_current_task();
  struct task *child_task = task_create(get_page_addr(task->code));

  // copy context
  memcpy(child_task, task, sizeof(uint64_t) * 10);
  memcpy((void *)(get_page_addr(child_task->stack)),
         (void *)(get_page_addr(task->stack)), 0x1000);
  memcpy((void *)(get_page_addr(child_task->ustack)),
         (void *)(get_page_addr(task->ustack)), 0x1000);

  // use the same code section
  child_task->code = task->code;
  void *code_addr = (void *)get_page_addr(task->code);
  mappages((pagetable_t)PA2KA(child_task->pagetable), 0,
           child_task->codesize = task->codesize, (uint64_t)code_addr,
           PT_AF | PT_USER | PT_MEM | PT_RW);

  // calculate new sp, fp and trap_frame
  child_task->sp =
      get_page_addr(child_task->stack) + task->sp - get_page_addr(task->stack);
  child_task->fp =
      get_page_addr(child_task->stack) + task->fp - get_page_addr(task->stack);
  child_task->trap_frame =
      (struct trap_frame *)((uint64_t)task->trap_frame -
                            get_page_addr(task->stack) +
                            get_page_addr(child_task->stack));

  // the return value of fork is the child id or 0
  child_task->trap_frame->x0 = 0;
  task->trap_frame->x0 = child_task->pid;

  // for child process, just jump to the end of exc handler
  child_task->lr = (uint64_t)&fork_child_entry;
  child_task->sp = (uint64_t)child_task->trap_frame;
  return 0;
}

void sys_exit() {
  // mark the status of current task to dead and move it from readyqueue to
  // deadqueue
  struct task *task = get_current_task();
  task->status = TASK_DEAD;
  __list_del(task->list.prev, task->list.next);
  list_add(&task->list, deadqueue);
  schedule();
}

void sys_kill(int pid) {
  // mark the status of current task to dead and move it from readyqueue to
  // deadqueue
  struct list_head *pos;
  struct task *task;
  list_for_each(pos, readyqueue) {
    task = list_entry(pos, struct task, list);
    if (task->pid == pid) {
      task->status = TASK_DEAD;
      break;
    }
  }
  __list_del(task->list.prev, task->list.next);
  list_add(&task->list, deadqueue);
}

int sys_mbox_call(unsigned char ch, unsigned int *mbox) {
  int res = mbox_call(ch, mbox);
  return res;
}
