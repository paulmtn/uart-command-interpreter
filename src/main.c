#include "uart.h"
#include <stdio.h>

int main() {
    uart_init();
    uart_send_string("Ready\r\n", 6);
    while (1) {
        process_keypress();
    }
}
