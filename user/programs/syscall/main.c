#include "stdio.h"
#include "syscall.h"

int main() {
  int pid = getpid();
  printf("Pid is; %d\n", pid);
}
