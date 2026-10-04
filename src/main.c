#include "uart.h"
#include <stdio.h>

int main() {
    uart_init();
    while (1) {
        process_keypress();
    }
}
