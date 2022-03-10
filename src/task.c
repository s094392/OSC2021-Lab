#include "task.h"
#include "alloc.h"
#include "stdio.h"

int pid_now;

void multitasking_init() { pid_now = 1; }

struct task *task_create(uint64_t addr) {
  struct task *task = kmalloc(sizeof(struct task));
  task->pid = pid_now++;
  task->lr = addr;
  task->sp = task->fp = (uint64_t)page_alloc(1);
  return task;
}

void task_run(struct task *task) {
  struct task tmp_task;
  switch_to(&tmp_task, task);
}
