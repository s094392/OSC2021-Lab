#include "alloc.h"
#include "list.h"

#include "stdio.h"

void *simple_alloc_offset;
struct page *frame_array;
struct list_head **free_list;
struct list_head *free_slab;
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

struct page *get_buddy(struct page *buddy) {
  return &frame_array[(buddy - frame_array) ^ (1 << buddy->val)];
}

int get_page_id(struct page *buddy) { return (buddy - frame_array); }

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
