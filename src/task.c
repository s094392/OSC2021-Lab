#include "task.h"
#include "alloc.h"
#include "list.h"
#include "stdio.h"

int pid_now;
struct list_head *readyqueue;
struct list_head *waitqueue;
struct list_head *deadqueue;

void multitasking_init() {
  pid_now = 1;
  readyqueue = kmalloc(sizeof(struct list_head));
  waitqueue = kmalloc(sizeof(struct list_head));
  INIT_LIST_HEAD(readyqueue);
  INIT_LIST_HEAD(waitqueue);
}

struct task *task_create(uint64_t addr) {
  // initialize a task and push it into readyqueue
  struct task *task = kmalloc(sizeof(struct task));
  task->pid = pid_now++;
  task->lr = addr;
  task->stack = page_alloc(1);
  task->sp = task->fp = get_page_addr(task->stack) + 0x1000;
  task->status = TASK_READY;
  list_add(&task->list, readyqueue);
  return task;
}

void task_run(struct task *task) {
  struct task tmp_task;
  switch_to(&tmp_task, task);
}

void schedule() {
  // check readyqueue
  if (list_empty(readyqueue)) {
    printf("No avaliable task in readyqueue!\n");
    return;
  }
  // get next task and switch to it
  struct task *current_task = get_current_task();
  struct task *next_task = list_entry(readyqueue->next, struct task, list);
  __list_del(next_task->list.prev, next_task->list.next->next);
  list_add_tail(&next_task->list, readyqueue);
  switch_to(current_task, next_task);
}
