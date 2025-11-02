#include <stdio.h>
#include "pico/stdlib.h"
#include "hlw8032.h"

hlw8032_t sensor;

int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("Iniciando HLW8032...\n");

    hlw8032_init(&sensor, uart1, 5, 4800);

    while (true) {
        if (hlw8032_read_frame(&sensor)) {
            hlw8032_print_data(&sensor);
        }
        sleep_ms(1000);
    }
}
