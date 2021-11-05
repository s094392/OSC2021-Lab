#include <byteswap.h>

#include "dtb.h"
#include "shell.h"
#include "stdio.h"
#include "uart.h"

int init(struct fdt_header* dtb) {
    uart_init();
    struct fdt_node_header* node =
        (struct fdt_node_header*)((long)dtb + __bswap_32(dtb->off_dt_struct));
    printf("0x%x\n", __bswap_32(node->tag));
    while (1) {
        shell();
    }
}
