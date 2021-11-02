#include "stdio.h"
#include "string.h"
#include "uart.h"

void read_cmd(char *cmd) {
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
        /*reset(10);*/
    } else if (!strcmp(cmd, "clear")) {
        printf("\033[2J\033[1;1H");
    } else {
        printf("command not found: %s\r\n", cmd);
    }
}

int main() {
    uart_init();
    while (1) shell();
}
