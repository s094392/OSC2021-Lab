#include "alloc.h"
#include "list.h"
#include "stdio.h"
#include <stdint.h>

void *simple_alloc_offset;
struct page *frame_array;
struct list_head **free_list;
struct list_head *slabs_free;
struct list_head *slabs_full;
int MAX_PAGE_ORDER;

void simple_alloc_init() { simple_alloc_offset = (void *)&_end; }

void *simple_alloc(size_t size) {
  void *result = simple_alloc_offset;
  simple_alloc_offset += size;
  return result;
}

int log_2(int n) {
  int res = -1;
  while (n) {
    n >>= 1;
    res++;
  }
  return res;
}

void buddy_system_init() {
  int page_frame_num = (BUDDY_END - BUDDY_START) / 0x1000;
  MAX_PAGE_ORDER = log_2(page_frame_num);
  free_list = simple_alloc((MAX_PAGE_ORDER + 1) * sizeof(struct list_head *));
  frame_array = simple_alloc(page_frame_num * sizeof(struct page));
  for (int i = 0; i <= MAX_PAGE_ORDER; i++) {
    free_list[i] = simple_alloc(sizeof(struct list_head));
  }
  for (int i = 0; i < MAX_PAGE_ORDER + 1; i++) {
    INIT_LIST_HEAD(free_list[i]);
  }
  for (int i = 0; i < page_frame_num; i += 1) {
    INIT_LIST_HEAD(&(frame_array[i].list));
    frame_array[i].status = USED_PAGE;
    frame_array[i].val = 1;
  }
  frame_array[0].status = FREE_PAGE;
  frame_array[0].val = MAX_PAGE_ORDER;
  list_add(&frame_array[0].list, free_list[MAX_PAGE_ORDER]);
}

uint64_t get_page_id(struct page *page) { return (page - frame_array); }

uint64_t get_page_addr(struct page *page) {
  return get_page_id(page) * 0x100 + BUDDY_START;
}

struct page *get_buddy(struct page *buddy) {
  return &frame_array[(get_page_id(buddy)) ^ (1 << buddy->val)];
}

struct page *page_alloc(int order) {
  struct page *result;
  if (list_empty(free_list[order])) {
    result = page_alloc(order + 1);
    result->val -= 1;
    struct page *buddy = get_buddy(result);
    buddy->val = result->val;
    buddy->status = FREE_PAGE;
    list_add(&buddy->list, free_list[buddy->val]);
  } else {
    result = list_entry(free_list[order]->next, struct page, list);
    __list_del(free_list[order], free_list[order]->next->next);
  }
  result->status = ALLOCATED_PAGE;
  return result;
}

void page_free(struct page *page) {
  struct page *buddy = get_buddy(page);
  int order = page->val;
  if (page->val < MAX_PAGE_ORDER && buddy->status == FREE_PAGE) {
    struct page *first_buddy =
        &frame_array[(buddy - frame_array) & (~(1 << page->val))];
    buddy->status = USED_PAGE;
    page->status = USED_PAGE;
    buddy->val = 0;
    page->val = 0;
    first_buddy->val = order + 1;
    __list_del(buddy->list.prev, buddy->list.next);
    page_free(first_buddy);
  } else {
    page->status = FREE_PAGE;
    list_add(&page->list, free_list[page->val]);
  }
}

void show_free_list() {
  for (int i = 0; i <= MAX_PAGE_ORDER; i++) {
    struct list_head *pos;
    printf("%d: ", i);
    list_for_each(pos, free_list[i]) {
      struct page *free_block = list_entry(pos, struct page, list);
      printf("%x ", free_block);
    }
    printf("\n");
  }
}

void slabs_init() { INIT_LIST_HEAD(slabs_free); }

void *kmalloc(size_t size) {
  struct list_head *pos;
  struct slab *slab = 0;
  size_t target_size = 4;
  printf("kmalloc size: %d\n", target_size);
  list_for_each(pos, slabs_free) {
    struct slab *tmp_slab = list_entry(pos, struct slab, list);
    printf("found slab with size %x\n", tmp_slab->size);
    if (tmp_slab->size == target_size && !list_empty(&tmp_slab->ob_list)) {
      slab = list_entry(pos, struct slab, list);
    }
  }
  if (!slab) {
    printf("No slab found, allocating a new page...\n");
    struct page *page = page_alloc(1);
    uint64_t base_addr = get_page_addr(page);
    slab = (struct slab *)base_addr;
    slab->page = page;
    slab->size = target_size;
    INIT_LIST_HEAD(&slab->ob_list);
    list_add(&slab->list, slabs_free);
    base_addr += sizeof(struct slab);
    int num_of_obj = (0x1000 - sizeof(struct slab)) /
                     (sizeof(struct slab_obj) + target_size);
    uint64_t slab_obj_base =
        (uint64_t)base_addr + num_of_obj * sizeof(struct slab_obj);
    printf("%d %d\n", sizeof(struct slab), sizeof(struct slab_obj));
    printf("num_of_obj: %d, base_addr: %x, slab_obj_base: %x\n", num_of_obj,
           base_addr, slab_obj_base);
    for (int i = 0; i < num_of_obj; i++) {
      struct slab_obj *slab_obj =
          (struct slab_obj *)(base_addr + sizeof(struct slab_obj) * i);
      slab_obj->addr = (void *)(slab_obj_base + target_size * i);
      list_add(&slab_obj->list, &slab->ob_list);
    }
  }
  void *result;
  result = list_entry(slab->ob_list.next, struct slab_obj, list)->addr;
  __list_del(&slab->ob_list, slab->ob_list.next->next);
  if (list_empty(&slab->ob_list)) {
    __list_del(slab->list.prev, slab->list.next);
    list_add(&slab->list, slabs_full);
  }
  return result;
}
