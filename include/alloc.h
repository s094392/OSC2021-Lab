#ifndef ALLOC_H
#define ALLOC_H
#include "list.h"
#include <stdint.h>

#define BUDDY_START 0x100000
#define BUDDY_END 0x200000

// simple allocator
void simple_alloc_init();
void *simple_alloc(size_t size);

extern char _end;
extern void *simple_alloc_offset;

// buddy system
enum Page_status {
  FREE_PAGE,
  ALLOCATED_PAGE,
  USED_PAGE,
};

struct page {
  int val;
  int ref;
  enum Page_status status;
  struct slab *slab;
  struct list_head list;
};

extern struct page *frame_array;

void buddy_system_init();
uint64_t get_page_addr(struct page *page);
uint64_t get_page_id(struct page *page);
struct page *page_alloc(int order);
void page_free(struct page *page);
void show_free_list();

// slab allocator
#define NUM_OF_AVALIABLE_SIZES 7
extern size_t avaliable_sizes[];

enum SLAB_STATUS { SLAB_FULL, SLAB_FREE };

struct slab {
  size_t size;
  struct page *page;
  enum SLAB_STATUS status;
  struct list_head obj_list;
  struct list_head list;
};

struct slab_obj {
  void *addr;
  struct list_head list;
};

void slabs_init();
void *kmalloc(size_t size);
void kfree(void *addr);

#endif
