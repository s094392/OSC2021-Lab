#ifndef TASK_H
#define TASK_H
#include "alloc.h"
#include "list.h"
#include <stdint.h>

extern int pid_now;

struct task {
  uint64_t x19, x20, x21, x22, x23, x24, x25, x26, x27, x28;
  uint64_t fp, lr, sp;
  int pid;
  struct page *code;
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

#endif
