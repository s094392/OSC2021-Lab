#include "syscall.h"
#include <stdint.h>

int getpid() {
  asm volatile("svc 0");
  uint64_t result;
  asm volatile("mov %0, x0" : "=r"(result)::);
  return result;
}

size_t uartread(char buf[], size_t size) {}
size_t uartwrite(const char buf[], size_t size) {
  asm volatile("mov x8, #2");
  asm volatile("svc 0");
  uint64_t result;
  asm volatile("mov %0, x0" : "=r"(result)::);
  return result;
}
int exec(const char *name, char *const argv[]) {}
int fork() {}
void exit() {}
