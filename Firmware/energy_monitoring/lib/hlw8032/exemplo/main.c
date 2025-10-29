#include <stdio.h>
#include "pico/stdlib.h"
#include "hlw8032.h"

int main() {
    stdio_init_all();

    hlw8032_config_t hlw = {
        .uart = uart0,
        .tx_pin = 0,       // GPIO0 = TX
        .rx_pin = 1,       // GPIO1 = RX
        .baudrate = 4800
    };

    hlw8032_init(&hlw);

    hlw8032_data_t data;

    while (true) {
        if (hlw8032_read_packet(&hlw, &data)) {
            printf("Tensão: %.2f V | Corrente: %.3f A | Potência: %.2f W | FP: %.2f\n",
                   data.voltage, data.current, data.power, data.power_factor);
        } else {
            printf("Erro na leitura do pacote!\n");
        }

        sleep_ms(1000);
    }
}
