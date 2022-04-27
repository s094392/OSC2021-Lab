#include "task.h"
#include "alloc.h"
#include "list.h"
#include "mem.h"
#include "stdio.h"

int pid_now;
struct list_head *readyqueue;
struct list_head *waitqueue;
struct list_head *deadqueue;
struct task *idle_task;

void idle() {
  while (1) {
    while (!list_empty(deadqueue)) {
      struct task *task = list_entry(deadqueue->next, struct task, list);
      page_free(task->stack);
      kfree(task);
      __list_del(task->list.prev, task->list.next);
    }
    schedule();
  }
}

void multitasking_init() {
  pid_now = 1;
  readyqueue = kmalloc(sizeof(struct list_head));
  waitqueue = kmalloc(sizeof(struct list_head));
  deadqueue = kmalloc(sizeof(struct list_head));
  INIT_LIST_HEAD(readyqueue);
  INIT_LIST_HEAD(waitqueue);
  INIT_LIST_HEAD(deadqueue);

  idle_task = task_create((uint64_t)&idle);
  __list_del(idle_task->list.prev, idle_task->list.next);
}

struct task *task_create(uint64_t addr) {
  // initialize a task and push it into readyqueue
  struct task *task = kmalloc(sizeof(struct task));
  task->pid = pid_now++;
  task->lr = addr;
  task->code = page_alloc(0);
  task->stack = page_alloc(0);
  task->ustack = page_alloc(0);
  task->sp = task->fp = get_page_addr(task->stack) + 0x1000;
  task->status = TASK_READY;
  task->pagetable = KA2PA(get_page_addr(page_alloc(0)));

  // map stack and mmio
  mappages(PA2KA((void *)task->pagetable), 0x0000ffffffffe000, 4096,
           get_page_addr(task->ustack), PT_AF | PT_USER | PT_MEM | PT_RW);
  mappages(PA2KA((void *)task->pagetable), 0x3c100000, 0x200000,
           PA2KA(0x3c100000), PT_AF | PT_USER | PT_MEM | PT_RW);

  list_add(&task->list, readyqueue);
  return task;
}

void task_run(struct task *task) {
  struct task tmp_task;
  switch_to(&tmp_task, task);
}

void schedule() {
  // check readyqueue
  struct task *current_task = get_current_task();
  if (list_empty(readyqueue)) {
    // printf("No avaliable task in readyqueue!\n");
    switch_to(current_task, idle_task);
    return;
  }
  // get next task and switch to it
  struct task *next_task = list_entry(readyqueue->next, struct task, list);
  __list_del(next_task->list.prev, next_task->list.next);
  list_add_tail(&next_task->list, readyqueue);
  // printf("Schedule! form %d to %d\n", current_task->pid, next_task->pid);
  switch_to(current_task, next_task);
}
