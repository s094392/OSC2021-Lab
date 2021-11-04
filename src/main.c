#include "shell.h"
#include "uart.h"

int main() {
    uart_init();
    while (1) shell();
}
