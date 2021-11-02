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

void shell() {
    printf("# ");
    char cmd[256];
    read_cmd(cmd);
    if (!strcmp(cmd, "help")) {
        printf(
            "help      : print this help menu\n"
            "hello     : print Hello World!\n"
            "reboot    : reboot the device\r\n");
    } else if (!strcmp(cmd, "hello")) {
        printf("Hello World!\r\n");
    } else if (!strcmp(cmd, "reboot")) {
        reset(10);
    } else if (!strcmp(cmd, "clear")) {
        printf("\033[2J\033[1;1H");
    } else if (!strcmp(cmd, "load")) {
    } else {
        printf("command not found: %s\r\n", cmd);
    }
}

void move(char* base_start, char* start, char* bootloader_end) {
    long offset = start - base_start;
    for (char* i = (char*)start; i <= bootloader_end; i++) {
        *(i - offset) = *i;
    }
}

int main() {
    while (1) shell();
}

void _load() {
    uart_init();
    char* base_start = (char*)0x60000;
    char* bootloader_start = (char*)0x80000;
    move(base_start, bootloader_start, (char*)&_end);
    ((void (*)())((char*)main - bootloader_start + base_start))();
}
