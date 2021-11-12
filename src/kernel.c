#include <byteswap.h>
#include <stdarg.h>

#include "cpio.h"
#include "dtb.h"
#include "mbox.h"
#include "shell.h"
#include "stdio.h"
#include "string.h"
#include "uart.h"
#define GET_BOARD_REVISION 0x00010002
#define REQUEST_CODE 0x00000000
#define REQUEST_SUCCEED 0x80000000
#define REQUEST_FAILED 0x80000001
#define TAG_REQUEST_CODE 0x00000000
#define END_TAG 0x00000000

#define GET_VC_MEMORY 0x00010006
#define UART_ID 0x000000002

extern void el2_entry();
extern void el1_entry();
extern void core_timer_enable(int s);

void get_initramfs(char* key, void* data, int len) {
    if (!strncmp(key, "linux,initrd-start", 18)) {
        cpio_addr = (void*)(size_t)__bswap_32(*(uint32_t*)data);
    }
}

void user() {
    while (1) {
        shell();
    }
}

void get_board_revision() {
    mbox[0] = 7 * 4;  // buffer size in bytes
    mbox[1] = REQUEST_CODE;
    // tags begin
    mbox[2] = GET_BOARD_REVISION;  // tag identifier
    mbox[3] = 4;  // maximum of request and response value buffer's length.
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0;  // value buffer
    // tags end
    mbox[6] = END_TAG;

    mbox_call(
        MBOX_CH_PROP);  // message passing procedure call, you should implement
                        // it following the 6 steps provided above.

    printf("Board revision: %x\n", mbox[5]);
}

void init(struct fdt_header* fdt) {
    uart_init();

    fdt_traverse(fdt, get_initramfs);

    get_board_revision();
    el2_entry();
    el1_entry();
    user();
}
