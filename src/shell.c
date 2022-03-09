#include "alloc.h"
#include "cpio.h"
#include "mbox.h"
#include "reset.h"
#include "stdio.h"
#include "string.h"
#include "trap.h"

void read_string(char *cmd) {
  char now;
  cmd[0] = 0;
  int now_cur = 0;
  while ((now = getchar()) != '\n') {
    if ((int)now == 240) {
      continue;
    } else if (now == 127) { // delete
      now_cur -= 1;
      if (now_cur >= 0) {
        printf("\b \b");
        now_cur -= 1;
      }
    } else {
      cmd[now_cur] = now;
      printf("%c", now);
    }
    now_cur++;
  }
  printf("\n");
  cmd[now_cur] = 0;
}

void shell() {
  printf("# ");
  char cmd[256];
  read_string(cmd);
  if (!strcmp(cmd, "help")) {
    printf("help      : print this help menu\n"
           "hello     : print Hello World!\n"
           "clear     : clear the screen!\n"
           "ls        : list files\n"
           "cat       : cat files\n"
           "cur       : print current el\n"
           "reboot    : reboot the device\n");
  } else if (!strcmp(cmd, "hello")) {
    printf("Hello World!\n");
  } else if (!strcmp(cmd, "reboot")) {
    reset(10);
  } else if (!strcmp(cmd, "timer")) {
    asm volatile("svc 1");
  } else if (!strcmp(cmd, "exc")) {
    asm volatile("svc 0");
  } else if (!strcmp(cmd, "ls")) {
    cpio_list();
  } else if (!strcmp(cmd, "show")) {
    lfb();
  } else if (!strcmp(cmd, "salloc")) {
    void *ptr = simple_alloc(1);
    printf("Allocated address: %lx\n", (uint64_t)ptr);
  } else if (!strcmp(cmd, "alloc")) {
    struct page *p1 = page_alloc(1);
    struct page *p2 = page_alloc(1);
    struct page *p3 = page_alloc(3);
    page_free(p1);
    page_free(p2);
    page_free(p3);
  } else if (!strcmp(cmd, "list")) {
    show_free_list();
  } else if (!strcmp(cmd, "kmalloc")) {
    for (int i = 0; i < 146; i++) {
      void *addr1 = kmalloc(4);
      printf("Allocated address: %lx\n", (uint64_t)addr1);
    }
  } else if (!strcmp(cmd, "cat")) {
    printf("Filename: ");
    read_string(cmd);
    void *file = get_cpio_file(cmd);
    printf("%s\n", (char *)get_file_data(file));
  } else if (!strcmp(cmd, "clear")) {
    printf("\033[2J\033[1;1H");
  } else {
    printf("command not found: %s\n", cmd);
  }
}
