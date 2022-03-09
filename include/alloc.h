#ifndef ALLOC_H
#define ALLOC_H
#include "list.h"
#include <stddef.h>

#define BUDDY_START 0x100000
#define BUDDY_END 0x200000

enum Page_status {
  FREE_PAGE,
  ALLOCATED_PAGE,
  USED_PAGE,
};

struct page {
  int val;
  enum Page_status status;
  struct list_head list;
};

struct slab {
  size_t size;
  struct page *page;
  struct list_head ob_list;
  struct list_head list;
};

struct slab_obj {
  void *addr;
  struct list_head list;
};

extern char _end;
extern void *simple_alloc_offset;
extern struct page *frame_array;

void simple_alloc_init();
void *simple_alloc(size_t size);
void buddy_system_init();
struct page *page_alloc(int order);
void page_free(struct page *page);
void show_free_list();
void slabs_init();
void *kmalloc(size_t size);

#endif
