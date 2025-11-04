#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

// ====================== CONFIGURAÇÃO ======================
#define UART_ID         uart0
#define UART_TX_PIN     0
#define UART_RX_PIN     1
#define BAUD_RATE       4800
#define DATA_BITS       8
#define STOP_BITS       1
#define PARITY          UART_PARITY_EVEN
#define FRAME_SIZE      24
// ==========================================================

// Buffer de recepção e flags
static volatile uint8_t rx_buffer[FRAME_SIZE];
static volatile int rx_index = 0;
static volatile bool frame_ready = false;

// -------------------- DECODIFICAÇÃO DE STATUS --------------------
typedef struct {
    bool voltage_overflow;
    bool current_overflow;
    bool power_overflow;
    bool param_invalid;
} HLW8032Status;

HLW8032Status hlw_decode_status(uint8_t status) {
    HLW8032Status s;
    s.voltage_overflow = (status >> 3) & 0x01;
    s.current_overflow = (status >> 2) & 0x01;
    s.power_overflow   = (status >> 1) & 0x01;
    s.param_invalid    = (status >> 0) & 0x01;
    return s;
}

void hlw_print_status(uint8_t status) {
    HLW8032Status s = hlw_decode_status(status);

    printf("\n[HLW8032 STATUS] 0x%02X\n", status);

    if (status == 0x55) {
        printf("✅ Estado normal: parâmetros e medições válidos.\n");
        return;
    }

    if (status == 0xAA) {
        printf("❌ Erro de correção interna: parâmetros inválidos!\n");
        return;
    }

    if ((status & 0xF0) == 0xF0) {
        printf("⚠️ Overflow detectado:\n");
        if (s.voltage_overflow) printf("  - Tensão (Voltage REG) overflow\n");
        if (s.current_overflow) printf("  - Corrente (Current REG) overflow\n");
        if (s.power_overflow)   printf("  - Potência (Power REG) overflow\n");
        if (s.param_invalid)    printf("  - Parâmetros (calibração) inválidos\n");

        if (status == 0xF2 || status == 0xFA)
            printf("💡 Interpretação: condição de NO-LOAD (sem carga)\n");
        return;
    }

    printf("Estado não documentado: 0x%02X\n", status);
}

// -------------------- INTERRUPÇÃO UART RX --------------------
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        rx_buffer[rx_index++] = ch;

        if (rx_index >= FRAME_SIZE) {
            rx_index = 0;
            frame_ready = true;
        }
    }
}

// -------------------- FUNÇÃO PRINCIPAL --------------------
int main() {
    stdio_usb_init();
    sleep_ms(1500);

    printf("\n=== HLW8032 via UART com interrupção ===\n");
    printf("Configuração: %d bps, 8E1, RX=GPIO%d, TX=GPIO%d\n\n",
           BAUD_RATE, UART_RX_PIN, UART_TX_PIN);

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    uart_set_fifo_enabled(UART_ID, false);

    int UART_IRQ = (UART_ID == uart0 ? UART0_IRQ : UART1_IRQ);
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);

    printf("Aguardando frames do HLW8032...\n");

    while (true) {
        if (frame_ready) {
            frame_ready = false;

            printf("\nFrame recebido (%d bytes):\n", FRAME_SIZE);
            for (int i = 0; i < FRAME_SIZE; i++) {
                printf("0x%02X ", rx_buffer[i]);
                if ((i + 1) % 10 == 0) printf("\n");
            }
            printf("\n");

            uint8_t status = rx_buffer[0];
            hlw_print_status(status);

            HLW8032Status s = hlw_decode_status(status);
            if (s.param_invalid) {
                printf("❌ Parâmetros inválidos, descartando frame.\n");
                rx_index = 0;
                continue;
            }

            if (s.power_overflow || s.voltage_overflow) {
                printf("⚠️ Overflow detectado — tratar como sem carga.\n");
                // Aqui pode definir corrente/potência = 0
            }

            rx_index = 0;
        }

        sleep_ms(10);
    }
}
