#include <byteswap.h>

#include "dtb.h"
#include "stdio.h"
#include "string.h"
#include "util.h"

size_t process_fdt_property(const struct fdt_header* fdt,
                            struct fdt_property* fdt_prop,
                            void (*callback)(char* key, void* data, int len)) {
    char* key = (char*)(long)((long)fdt + __bswap_32(fdt->off_dt_strings) +
                              __bswap_32(fdt_prop->nameoff));
    int len = __bswap_32(fdt_prop->len);
    char value[len];
    strncpy(value, (fdt_prop->data), (unsigned long)len);
    callback(key, value, len);
    /*printf("%s: %s 0x%x\n", key, fdt_prop->data,*/
    /*__bswap_32(*(uint32_t*)(fdt_prop->data)));*/
    return sizeof(struct fdt_property) +
           (__bswap_32(fdt_prop->len) + sizeof(uint32_t) - 1) /
               sizeof(uint32_t) * sizeof(uint32_t);
}

size_t process_fdt_begin(const struct fdt_header* fdt,
                         struct fdt_node_header* fdt_begin) {
    /*printf("%s\n", fdt_begin->name,*/
    /*strlen(fdt_begin->name) / sizeof(uint32_t) + 2);*/
    size_t name_len = strlen(fdt_begin->name) / sizeof(uint32_t) + 1;
    return name_len * sizeof(uint32_t) + sizeof(uint32_t);
}

void* next_fdt_obj(const struct fdt_header* fdt, void* fdt_obj, int* depth,
                   void (*callback)(char* key, void* data, int len)) {
    uint32_t tag = __bswap_32(((struct fdt_node_header*)fdt_obj)->tag);
    uint32_t* next_fdt_obj;

    switch (tag) {
        case FDT_PROP:
            /*for (int i = 0; i < *depth; i++) printf("\t");*/
            next_fdt_obj =
                fdt_obj + process_fdt_property(
                              fdt, (struct fdt_property*)fdt_obj, callback);
            break;

        case FDT_NOP:
            next_fdt_obj = fdt_obj + sizeof(uint32_t);
            break;

        case FDT_BEGIN_NODE:
            /*for (int i = 0; i < *depth; i++) printf("\t");*/
            if (depth) (*depth)++;
            next_fdt_obj = fdt_obj + process_fdt_begin(
                                         fdt, (struct fdt_node_header*)fdt_obj);
            break;

        case FDT_END_NODE:
            next_fdt_obj = fdt_obj + sizeof(uint32_t);
            if (depth) (*depth)--;
            break;
        case FDT_END:
            return NULL;
    }

    return next_fdt_obj;
}

void fdt_traverse(struct fdt_header* fdt,
                  void (*callback)(char* key, void* data, int len)) {
    int depth = -1;
    int offset = __bswap_32(fdt->off_dt_struct);
    void* fdt_obj = (void*)((long)fdt + offset);
    while (fdt_obj) {
        fdt_obj = next_fdt_obj(fdt, fdt_obj, &depth, callback);
    }
}
