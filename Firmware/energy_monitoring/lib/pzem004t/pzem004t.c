#include "pzem004t.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

#define UART_ID      uart0
#define UART_TX_PIN  0   // GPIO0 = TX para o PZEM
#define UART_RX_PIN  1   // GPIO1 = RX do PZEM
#define BAUDRATE     9600

// ========================
//  Inicialização UART
// ========================
void pzem_init() {
    uart_init(UART_ID, BAUDRATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, true);
}

// ========================
//  Comando de leitura PZEM (versão estável)
// ========================

static const uint8_t READ_COMMAND[8] = {
    0xB0, 0xC0, 0xA8, 0x01, 0x01, 0x00, 0x00, 0x1E
};

// ========================
//  Leitura completa do módulo
// ========================
bool pzem_read(pzem_data_t *out)
{
    // Envia comando
    for (int i = 0; i < sizeof(READ_COMMAND); i++)
        uart_putc_raw(UART_ID, READ_COMMAND[i]);

    sleep_ms(100);

    // Verifica se há dados
    if (!uart_is_readable_within_us(UART_ID, 300000))
        return false;

    uint8_t buffer[32];
    int index = 0;

    // Lê tudo disponível
    while (uart_is_readable(UART_ID) && index < sizeof(buffer)) {
        buffer[index++] = uart_getc(UART_ID);
    }

    // Resposta mínima: 24 bytes (dependendo do firmware)
    if (index < 24)
        return false;

    // Conversão dos dados
    out->voltage   = (buffer[10] << 8 | buffer[11]) / 10.0f;
    out->current   = (buffer[12] << 8 | buffer[13]) / 1000.0f;
    out->power     = (buffer[14] << 8 | buffer[15]) / 10.0f;
    out->energy    = (buffer[16] << 8 | buffer[17]) / 1000.0f;
    out->frequency = (buffer[18] << 8 | buffer[19]) / 10.0f;
    out->pf        = (buffer[20] << 8 | buffer[21]) / 100.0f;

    return true;
}
