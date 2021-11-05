#include <byteswap.h>

#include "dtb.h"
#include "shell.h"
#include "stdio.h"
#include "string.h"
#include "uart.h"
#include "util.h"
#define pad(x, y) ((((x) + (y)-1) / (y)) * (y))

void describe_fdt_property(const struct fdt_header* fdt, int offset) {
    struct fdt_property* fdt_prop = (struct fdt_property*)(fdt + offset);
    char* key = (char*)(long)((long)fdt + __bswap_32(fdt->off_dt_strings) +
                              __bswap_32(fdt_prop->nameoff));
    int len = __bswap_32(fdt_prop->len);
    char value[len];
    strncpy(value, (fdt_prop->data), (unsigned long)len);
    printf("%s: %s\n", key, fdt_prop->data);
}

void describe_fdt_begin(const struct fdt_header* fdt, int offset) {
    struct fdt_node_header* fdt_begin = (struct fdt_node_header*)(fdt + offset);
    printf("%s\n", fdt_begin->name);
}

uint32_t fdt_next_tag(const void* fdt, int startoffset, int* nextoffset) {
    uint32_t* tag = (uint32_t*)(fdt + startoffset);
    *nextoffset = -1;
    switch (__bswap_32(*tag)) {
        case FDT_BEGIN_NODE:
            *nextoffset = startoffset +
                          max(1, strlen((char*)(tag + 1))) * sizeof(uint32_t) +
                          sizeof(uint32_t);
            break;
        case FDT_PROP:
            *nextoffset =
                startoffset + sizeof(struct fdt_property) +
                __bswap_32(
                    ((struct fdt_property*)((long)fdt + startoffset))->len);
            break;
        case FDT_END:
        case FDT_END_NODE:
        case FDT_NOP:
            break;
        default:
            return FDT_END;
    }
    printf("next: %x\n", *nextoffset);
    return *tag;
}

int next_fdt_node(const void* fdt, int offset, int* depth) {
    printf("offst :%x\n", offset);
    int nextoffset = offset;
    uint32_t tag;
    do {
        offset = nextoffset;
        tag = fdt_next_tag(fdt, offset, &nextoffset);
        printf("tag: %d\n", tag);

        switch (tag) {
            case FDT_PROP:
                for (int i = 0; i < *depth; i++) printf("\t");
                describe_fdt_property(fdt, offset);
            case FDT_NOP:
                break;

            case FDT_BEGIN_NODE:
                for (int i = 0; i < *depth; i++) printf("\t");
                describe_fdt_begin(fdt, offset);
                if (depth) (*depth)++;
                break;

            case FDT_END_NODE:
                if (depth && ((--(*depth)) < 0)) return nextoffset;
                break;

            case FDT_END:
                if ((nextoffset >= 0) || ((nextoffset == -1) && !depth))
                    return -1;
                else
                    return nextoffset;
        }
    } while (tag != FDT_BEGIN_NODE);

    return offset;
}

int init(struct fdt_header* fdt) {
    uart_init();
    int depth = 0;
    for (int i = 0; i < 10; i++) {
        next_fdt_node(fdt, __bswap_32(fdt->off_dt_struct), &depth);
    }
    while (1) {
        shell();
    }
}
