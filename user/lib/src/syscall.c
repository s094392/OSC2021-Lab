#include "syscall.h"
#include <stdint.h>

int getpid() {
  uint64_t pid = 0;
  asm volatile("svc 0");
  return pid;
}

size_t uartread(char buf[], size_t size) {}
size_t uartwrite(const char buf[], size_t size) {}
int exec(const char *name, char *const argv[]) {}
int fork() {}
void exit() {}
