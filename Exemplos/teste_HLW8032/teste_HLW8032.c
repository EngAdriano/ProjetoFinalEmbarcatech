#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/uart.h"

#define UART_ID        uart0
#define UART_TX_PIN    0
#define UART_RX_PIN    1
#define UART_BAUDRATE  4800

int main() {
    stdio_usb_init();
    sleep_ms(1500);

    printf("\n=== Diagnóstico UART HLW8032 ===\n");
    printf("UART0 RX=GPIO%d, TX=GPIO%d, baud=%d\n\n", UART_RX_PIN, UART_TX_PIN, UART_BAUDRATE);

    uart_init(UART_ID, UART_BAUDRATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_EVEN);
    uart_set_fifo_enabled(UART_ID, false);

    uint8_t frame[24];
    int index = 0;

    while (true) {
        if (uart_is_readable(UART_ID)) {
            frame[index++] = uart_getc(UART_ID);

            if (index >= 24) {
                printf("\nFrame (%d bytes): ", index);
                for (int i = 0; i < index; i++) {
                    printf("0x%02X ", frame[i]);
                    if ((i + 1) % 10 == 0) printf("\n");
                }
                printf("\n");

                // Verifica se começa com 0x55 0x5A
                if (frame[0] == 0x55 && frame[1] == 0x5A)
                    printf(">>> Frame HLW8032 detectado corretamente! <<<\n");
                else
                    printf("⚠️  Frame não inicia com 0x55 0x5A (início: 0x%02X 0x%02X)\n", frame[0], frame[1]);

                index = 0; // reinicia
            }
        } else {
            sleep_ms(1);
        }
    }
}
