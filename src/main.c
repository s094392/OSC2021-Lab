#include <byteswap.h>

#include "cpio.h"
#include "dtb.h"
#include "shell.h"
#include "string.h"
#include "uart.h"

void get_initramfs(char* key, void* data, int len) {
    if (!strncmp(key, "linux,initrd-start", 18)) {
        cpio_addr = (void*)(size_t)__bswap_32(*(uint32_t*)data);
    }
}

int init(struct fdt_header* fdt) {
    uart_init();
    fdt_traverse(fdt, get_initramfs);

    while (1) {
        shell();
    }
}
