#include "uart.h"
#include <stdio.h>

int main() {
    main_init();
    while (1) {
        if (data_available()) {
            process_keypress();
        }
    }
}
