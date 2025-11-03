#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#define UART_ID         uart0
#define UART_TX_PIN     0
#define UART_RX_PIN     17
#define BAUD_RATE       4800
#define DATA_BITS       8
#define STOP_BITS       1
#define PARITY          UART_PARITY_EVEN
#define FRAME_SIZE      24

// Buffer de recepção
static volatile uint8_t rx_buffer[FRAME_SIZE];
static volatile int rx_index = 0;
static volatile bool frame_ready = false;

// Interrupção de recepção UART
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        rx_buffer[rx_index++] = ch;

        // Reinicia se ultrapassar o tamanho esperado
        if (rx_index >= FRAME_SIZE) {
            rx_index = 0;
            frame_ready = true;
        }
    }
}

int main() {
    stdio_usb_init();
    sleep_ms(1500);

    printf("\n=== HLW8032 via UART com interrupção ===\n");
    printf("Configuração: 4800 bps, 8E1, RX=GPIO%d, TX=GPIO%d\n\n", UART_RX_PIN, UART_TX_PIN);

    // Inicializa UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    uart_set_fifo_enabled(UART_ID, false);

    // Configura interrupção UART RX
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);

    printf("Aguardando dados do HLW8032...\n");

    while (true) {
        // Quando um frame completo chegar (24 bytes)
        if (frame_ready) {
            frame_ready = false;

            printf("\nFrame recebido (%d bytes):\n", FRAME_SIZE);
            for (int i = 0; i < FRAME_SIZE; i++) {
                printf("0x%02X ", rx_buffer[i]);
                if ((i + 1) % 10 == 0) printf("\n");
            }

            printf("\n");

            // Verifica se os dois bytes iniciais são 0x55 e 0x5A
            if (rx_buffer[0] == 0x55 && rx_buffer[1] == 0x5A) {
                printf("✅ Frame HLW8032 detectado corretamente!\n");
            } else {
                printf("⚠️ Frame inválido: início = 0x%02X 0x%02X (esperado 0x55 0x5A)\n",
                       rx_buffer[0], rx_buffer[1]);
            }

            rx_index = 0; // reinicia leitura
        }

        // Pequeno delay para aliviar a CPU
        sleep_ms(10);
    }
}
