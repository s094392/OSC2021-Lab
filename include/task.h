#ifndef TASK_H
#define TASK_H
#include "alloc.h"
#include "list.h"
#include "trap.h"
#include <stdint.h>

extern int pid_now;

enum TASK_STATUS {
  TASK_READY,
  TASK_WAIT,
  TASK_DEAD,
};

struct task {
  uint64_t x19, x20, x21, x22, x23, x24, x25, x26, x27, x28;
  uint64_t fp, lr, sp;
  uint64_t pagetable;
  int pid;
  enum TASK_STATUS status;
  struct page *stack;
  struct page *ustack;
  struct page *code;
  struct trap_frame *trap_frame;
  struct list_head list;
};

void switch_to(struct task *prev, struct task *next);
struct task *get_current_task();

void multitasking_init();
struct task *task_create(uint64_t addr);
void task_run(struct task *task);
void schedule();

extern struct list_head *readyqueue;
extern struct list_head *waitqueue;
extern struct list_head *deadqueue;

extern struct task *idle_task;

#endif
