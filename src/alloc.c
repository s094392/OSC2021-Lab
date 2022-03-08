#include "alloc.h"
#include "list.h"

#include "stdio.h"

void *simple_alloc_offset;
struct page *frame_array;
struct list_head *free_list;

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
  int highest_order = log_2(page_frame_num);
  free_list = simple_alloc((highest_order + 1) * sizeof(struct list_head));
  frame_array = simple_alloc(page_frame_num * sizeof(struct page));
  for (int i = 0; i < highest_order + 1; i++) {
    INIT_LIST_HEAD(&free_list[i]);
  }
  for (int i = 0; i < page_frame_num; i += 1) {
    INIT_LIST_HEAD(&(frame_array[i].list));
    frame_array[i].val = FREE_FRAME;
  }
  frame_array[0].val = highest_order;
  list_add(&frame_array[0].list, &free_list[highest_order]);
}

struct page *get_buddy(struct page *buddy) {
  return &frame_array[(buddy - frame_array) + (1 << buddy->val)];
}

int get_page_id(struct page *buddy) { return (buddy - frame_array); }

struct page *page_alloc(int order) {
  printf("alloc %d\n", order);
  struct page *result;
  if (list_empty(&free_list[order])) {
    printf("empty\n");
    result = page_alloc(order + 1);
    result->val -= 1;
    struct page *buddy = get_buddy(result);
    buddy->val = result->val;
    printf("pushed %x of order %d.\n", buddy, buddy->val);
    list_add(&free_list[buddy->val], &buddy->list);
  } else {
    result = list_entry(free_list[order].next, struct page, list);
    __list_del(&free_list[order], free_list[order].next->next);
  }
  printf("return %x\n", result);
  return result;
}
