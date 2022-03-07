#ifndef ALLOC_H
#define ALLOC_H
#include "list.h"
#include <stddef.h>

#define BUDDY_START 0x100000
#define BUDDY_END 0x200000

#define FREE_FRAME -1
#define USED_FRAME -2

struct page {
  struct list_head list;
  int val;
};

extern char _end;
extern void *simple_alloc_offset;
extern int *frame_array;

void simple_alloc_init();
void *simple_alloc(size_t size);
void buddy_system_init();

#endif
