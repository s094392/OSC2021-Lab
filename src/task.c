#include "task.h"
#include "alloc.h"
#include "list.h"
#include "stdio.h"

int pid_now;
struct list_head *runqueue;
struct list_head *waitqueue;

void multitasking_init() {
  pid_now = 1;
  runqueue = kmalloc(sizeof(struct list_head));
  waitqueue = kmalloc(sizeof(struct list_head));
  INIT_LIST_HEAD(runqueue);
  INIT_LIST_HEAD(waitqueue);
}

struct task *task_create(uint64_t addr) {
  struct task *task = kmalloc(sizeof(struct task));
  task->pid = pid_now++;
  task->lr = addr;
  task->sp = task->fp = (uint64_t)page_alloc(1);
  list_add(&task->list, runqueue);
  return task;
}

void task_run(struct task *task) {
  struct task tmp_task;
  switch_to(&tmp_task, task);
}

void schedule() {
  printf("Schedule\n");
  if (list_empty(runqueue)) {
    printf("No avaliable task in runqueue!\n");
    return;
  }
  struct task *current_task = get_current_task();
  struct task *next_task = list_entry(runqueue->next, struct task, list);
  __list_del(next_task->list.prev, next_task->list.next);
  list_add_tail(&next_task->list, runqueue);
  switch_to(current_task, next_task);
}
