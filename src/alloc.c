#include "alloc.h"
#include "list.h"
#include "stdio.h"
#include "util.h"
#include <stdint.h>

void *simple_alloc_offset;
struct page *frame_array;
struct list_head **free_list;
struct list_head *slabs_free;
struct list_head *slabs_full;
int MAX_PAGE_ORDER;

void simple_alloc_init() { simple_alloc_offset = (void *)&_end; }

// simple allocator that simply increase the offset
void *simple_alloc(size_t size) {
  void *result = simple_alloc_offset;
  simple_alloc_offset += size;
  return result;
}

void buddy_system_init() {
  int page_frame_num = (BUDDY_END - BUDDY_START) / 0x1000;
  MAX_PAGE_ORDER = log_2(page_frame_num);
  free_list = simple_alloc((MAX_PAGE_ORDER + 1) * sizeof(struct list_head *));
  frame_array = simple_alloc(page_frame_num * sizeof(struct page));
  // initialize free_list
  for (int i = 0; i <= MAX_PAGE_ORDER; i++) {
    free_list[i] = simple_alloc(sizeof(struct list_head));
  }
  // initialize free_list head of all orders
  for (int i = 0; i < MAX_PAGE_ORDER + 1; i++) {
    INIT_LIST_HEAD(free_list[i]);
  }
  // initialize page_frame
  for (int i = 0; i < page_frame_num; i += 1) {
    INIT_LIST_HEAD(&(frame_array[i].list));
    frame_array[i].status = USED_PAGE;
    frame_array[i].val = 1;
  }
  // push first frame
  frame_array[0].status = FREE_PAGE;
  frame_array[0].val = MAX_PAGE_ORDER;
  list_add(&frame_array[0].list, free_list[MAX_PAGE_ORDER]);
}

uint64_t get_page_id(struct page *page) { return (page - frame_array); }

uint64_t get_page_addr(struct page *page) {
  return get_page_id(page) * 0x1000 + BUDDY_START;
}

struct page *get_buddy(struct page *buddy) {
  return &frame_array[(get_page_id(buddy)) ^ (1 << buddy->val)];
}

struct page *page_alloc(int order) {
  struct page *result;
  // alloc larger page when the free list of current order is empty
  if (list_empty(free_list[order])) {
    result = page_alloc(order + 1);
    result->val -= 1;
    struct page *buddy = get_buddy(result);
    buddy->val = result->val;
    buddy->status = FREE_PAGE;
    list_add(&buddy->list, free_list[buddy->val]);
  } else {
    // grab the first pages in free list and pop it
    result = list_entry(free_list[order]->next, struct page, list);
    __list_del(free_list[order], free_list[order]->next->next);
  }
  result->status = ALLOCATED_PAGE;
  return result;
}

void page_free(struct page *page) {
  struct page *buddy = get_buddy(page);
  int order = page->val;
  // find buddy and merge
  if (page->val < MAX_PAGE_ORDER && buddy->status == FREE_PAGE) {
    // find first buddy
    struct page *first_buddy =
        &frame_array[(buddy - frame_array) & (~(1 << page->val))];
    // reset both current page and its buddy
    buddy->status = USED_PAGE;
    page->status = USED_PAGE;
    buddy->val = 0;
    page->val = 0;
    // push first buddy and remove buddy from previous free list
    first_buddy->val = order + 1;
    __list_del(buddy->list.prev, buddy->list.next);
    // recursivly free merged pages
    page_free(first_buddy);
  } else {
    // push page into free list
    page->status = FREE_PAGE;
    list_add(&page->list, free_list[page->val]);
  }
}

void show_free_list() {
  for (int i = 0; i <= MAX_PAGE_ORDER; i++) {
    struct list_head *pos;
    printf("Order %d: ", i);
    list_for_each(pos, free_list[i]) {
      struct page *free_block = list_entry(pos, struct page, list);
      printf("%lx ", (uint64_t)free_block);
    }
    printf("\n");
  }
}

void slabs_init() {
  slabs_free = simple_alloc(sizeof(struct slab));
  slabs_full = simple_alloc(sizeof(struct slab));
  INIT_LIST_HEAD(slabs_free);
  INIT_LIST_HEAD(slabs_full);
}
size_t avaliable_sizes[] = {16, 32, 48, 96, 128, 256, 512};

void *kmalloc(size_t size) {
  struct slab *slab = 0;
  // determine the target size
  size_t target_size;
  for (int i = 0; i < NUM_OF_AVALIABLE_SIZES; i++) {
    if (avaliable_sizes[i] >= size)
      target_size = avaliable_sizes[i];
  }
  // find slab with requested size
  struct list_head *pos;
  list_for_each(pos, slabs_free) {
    struct slab *tmp_slab = list_entry(pos, struct slab, list);
    if (tmp_slab->size == target_size && !list_empty(&tmp_slab->obj_list)) {
      slab = list_entry(pos, struct slab, list);
    }
  }
  if (!slab) {
    // allocate a page to create a new slab
    struct page *page = page_alloc(1);
    uint64_t base_addr = get_page_addr(page);
    slab = (struct slab *)base_addr;
    slab->page = page;
    slab->size = target_size;
    slab->status = SLAB_FREE;
    INIT_LIST_HEAD(&slab->obj_list);
    list_add(&slab->list, slabs_free);
    page->slab = slab;
    // initialize slab structure and slab objects
    base_addr += sizeof(struct slab);
    int num_of_obj = (0x1000 - sizeof(struct slab)) /
                     (sizeof(struct slab_obj) + target_size);
    uint64_t slab_obj_base =
        (uint64_t)base_addr + num_of_obj * sizeof(struct slab_obj);
    // push slab objects into slab->obj_list
    for (int i = 0; i < num_of_obj; i++) {
      struct slab_obj *slab_obj =
          (struct slab_obj *)(base_addr + sizeof(struct slab_obj) * i);
      slab_obj->addr = (void *)(slab_obj_base + target_size * i);
      list_add(&slab_obj->list, &slab->obj_list);
    }
  }
  // get slab object's address and remove it from obj_list
  void *result;
  result = list_entry(slab->obj_list.next, struct slab_obj, list)->addr;
  __list_del(&slab->obj_list, slab->obj_list.next->next);
  // move slab from slabs_free to slabs_full when empty
  if (list_empty(&slab->obj_list)) {
    __list_del(slab->list.prev, slab->list.next);
    list_add(&slab->list, slabs_full);
  }
  return result;
}

void kfree(void *addr) {
  // get page of current address
  uint64_t page_base_addr = ((uint64_t)addr >> 12 << 12);
  struct page *page = &frame_array[page_base_addr - BUDDY_START];
  // get slab of current page
  struct slab *slab = page->slab;
  // get slab objects of current address
  int num_of_obj =
      (0x1000 - sizeof(struct slab)) / (sizeof(struct slab_obj) + slab->size);
  int slab_offset = ((uint64_t)addr - page_base_addr - sizeof(struct slab) -
                     sizeof(struct slab_obj) * num_of_obj) /
                    slab->size;
  struct slab_obj *slab_obj =
      (struct slab_obj *)(get_page_addr(page) + sizeof(struct slab) +
                          sizeof(struct slab_obj) * slab_offset);
  // push slab object into obj_list
  list_add(&slab_obj->list, &slab->obj_list);
  if (slab->status == SLAB_FULL) {
    slab->status = SLAB_FREE;
    __list_del(slab->list.prev, slab->list.next);
    list_add(&slab->list, slabs_free);
  }
}
