#include <byteswap.h>

#include "dtb.h"
#include "shell.h"
#include "stdio.h"
#include "string.h"
#include "uart.h"
#include "util.h"
#define pad(x, y) ((((x) + (y)-1) / (y)) * (y))

int init(struct fdt_header* fdt) {
    uart_init();
    int depth = -1;
    int offset = __bswap_32(fdt->off_dt_struct);
    void* fdt_obj = (void*)((long)fdt + offset);

    while (fdt_obj) {
        fdt_obj = next_fdt_obj(fdt, fdt_obj, &depth);
    }
    while (1) {
        shell();
    }
}
