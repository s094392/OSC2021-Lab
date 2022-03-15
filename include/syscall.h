#ifndef SYSCALL_H
#define SYSCALL_H
#include <stddef.h>

int sys_getpid();
size_t sys_uartread(char buf[], size_t size);
size_t sys_uartwrite(const char buf[], size_t size);
int sys_exec(const char *name, char *const argv[]);
int sys_fork();
void sys_exit();
#endif
