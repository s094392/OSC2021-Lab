#include "alloc.h"

void* simple_alloc_offset;

void simple_alloc_init() { simple_alloc_offset = (void*)&_end; }

void* simple_alloc(size_t size) {
    void* result = simple_alloc_offset;
    simple_alloc_offset += size;
    return result;
}

void buddy_system_init() {}
