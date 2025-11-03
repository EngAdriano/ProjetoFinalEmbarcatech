#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/pio.h"
#include "uart_rx_inverted.pio.h"

#define UART_RX_PIN 1
#define BAUD_RATE 4800

static inline void uart_rx_inverted_program_init(PIO pio, uint sm, uint offset, uint pin, uint baud) {
    pio_sm_config c = uart_rx_inverted_program_get_default_config(offset);
    sm_config_set_in_pins(&c, pin);
    sm_config_set_jmp_pin(&c, pin);
    sm_config_set_in_shift(&c, true, false, 8);
    pio_gpio_init(pio, pin);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, false);

    float div = (float)clock_get_hz(clk_sys) / (baud * 10);
    sm_config_set_clkdiv(&c, div);

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

int main() {
    stdio_usb_init();
    sleep_ms(1500);

    printf("\n=== HLW8032 via PIO UART RX Invertido (8E1) ===\n");
    printf("RX = GPIO%d, baud = %d\n\n", UART_RX_PIN, BAUD_RATE);

    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &uart_rx_inverted_program);
    uart_rx_inverted_program_init(pio, sm, offset, UART_RX_PIN, BAUD_RATE);

    uint8_t prev = 0;
    uint8_t current = 0;
    int count = 0;

    while (true) {
        if (!pio_sm_is_rx_fifo_empty(pio, sm)) {
            current = pio_sm_get(pio, sm) & 0xFF;
            count++;

            if (prev == 0x55 && current == 0x5A) {
                printf("\n>>> Início de frame detectado (0x55 0x5A) <<<\n");
            }

            printf("0x%02X ", current);
            fflush(stdout);

            if (count % 10 == 0) printf("\n");
            prev = current;
        } else {
            sleep_ms(1);
        }
    }
}
