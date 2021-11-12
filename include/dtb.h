#ifndef DTB_H
#define DTB_H

#include <stdint.h>

#define FDT_BEGIN_NODE 0x1 /* Start node: full name */
#define FDT_END_NODE 0x2   /* End node */
#define FDT_PROP                           \
    0x3             /* Property: name off, \
                       size, content */
#define FDT_NOP 0x4 /* nop */
#define FDT_END 0x9

struct fdt_header {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
};

struct fdt_node_header {
    uint32_t tag;
    char name[0];
};

struct fdt_property {
    uint32_t tag;
    uint32_t len;
    uint32_t nameoff;
    char data[0];
};

void* next_fdt_obj(const struct fdt_header* fdt, void* fdt_obj, int* depth,
                   void (*callback)(char* key, void* data, int len));
void fdt_traverse(struct fdt_header* fdt,
                  void (*callback)(char* key, void* data, int len));

#endif
