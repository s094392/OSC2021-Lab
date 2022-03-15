#include "syscall.h"
#include "task.h"

int sys_getpid() { return get_current_task()->pid; }

size_t sys_uartread(char buf[], size_t size) {}
size_t sys_uartwrite(const char buf[], size_t size) {}
int sys_exec(const char *name, char *const argv[]) {}
int sys_fork() {}
void sys_exit() {}
