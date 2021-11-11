#include "reset.h"
#include "stdio.h"
#include "string.h"
#include "uart.h"
extern char _end;

void read_cmd(char* cmd) {
    char now;
    cmd[0] = 0;
    int now_cur = 0;
    while ((now = getchar()) != '\n') {
        if ((int)now == 240) {
            continue;
        } else if (now == 127) {  // delete
            now_cur -= 1;
            if (now_cur >= 0) {
                printf("\b \b");
                now_cur -= 1;
            }
        } else {
            cmd[now_cur] = now;
            printf("%c", now);
        }
        now_cur++;
    }
    printf("\n");
    cmd[now_cur] = 0;
}

void load_img(int size) {
    unsigned char* base = (unsigned char*)0x80000;
    for (int i = 0; i < size; i++) {
        base[i] = inbyte();
    }
}

void shell(unsigned long* fdt) {
    printf("# ");
    char cmd[256];
    read_cmd(cmd);
    if (!strcmp(cmd, "help")) {
        printf(
            "help      : print this help menu\n"
            "hello     : print Hello World!\n"
            "load      : load img.!\n"
            "reboot    : reboot the device\n");
    } else if (!strcmp(cmd, "hello")) {
        printf("Hello World!\n");
    } else if (!strcmp(cmd, "reboot")) {
        reset(10);
    } else if (!strcmp(cmd, "clear")) {
        printf("\033[2J\033[1;1H");
    } else if (!strcmp(cmd, "load")) {
        read_cmd(cmd);
        int size = atoi(cmd);
        load_img(size);
        ((void (*)(void* fdt))(0x80000))(fdt);
    } else {
        printf("command not found: %s\n", cmd);
    }
}

void move(char* base_start, char* start, char* bootloader_end) {
    long offset = start - base_start;
    for (char* i = (char*)start; i <= bootloader_end; i++) {
        *(i - offset) = *i;
    }
}

int nmain(void* fdt) {
    while (1) shell(fdt);
}

void _load(void* fdt) {
    uart_init();
    char* base_start = (char*)0x60000;
    char* bootloader_start = (char*)0x80000;
    move(base_start, bootloader_start, (char*)&_end);
    ((void (*)(void* fdt))((char*)nmain - bootloader_start + base_start))(fdt);
}
