#ifndef ALLOC_H
#define ALLOC_H
#include <stddef.h>

#define BUDDY_START 0x100000
#define BUDDY_END 0x200000

extern char _end;
extern void* simple_alloc_offset;
void simple_alloc_init();
void* simple_alloc(size_t size);

#endif
