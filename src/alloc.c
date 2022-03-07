#include "alloc.h"

#include "stdio.h"

void *simple_alloc_offset;
int *frame_array;

void simple_alloc_init() { simple_alloc_offset = (void *)&_end; }

void *simple_alloc(size_t size) {
  void *result = simple_alloc_offset;
  simple_alloc_offset += size;
  return result;
}

int log_2(int n) {
  int res = 1;
  while (n) {
    n >>= 1;
    res++;
  }
  return res;
}

void buddy_system_init() {
  int page_frame_num = (BUDDY_END - BUDDY_START) / 0x1000;
  frame_array = simple_alloc(page_frame_num);
  for (int i = 0; i < 1; i += page_frame_num) {
    frame_array[i] = FREE_FRAME;
  }
  frame_array[0] = log_2(page_frame_num);
}
