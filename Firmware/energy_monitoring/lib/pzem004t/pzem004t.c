#include <stdio.h>
#include "pzem004t.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

#define UART_ID      uart0
#define UART_TX_PIN  0
#define UART_RX_PIN  1
#define BAUDRATE     9600

//======================================================
// CRC-16 MODBUS
//======================================================
static uint16_t modbus_crc(uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;

    for (int pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos];

        for (int i = 8; i != 0; i--) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

//======================================================
// Inicialização UART
//======================================================
void pzem_init()
{
    uart_init(UART_ID, BAUDRATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, true);

    printf("[PZEM] UART pronta (MODBUS 9600 8N1)\n");
}


//======================================================
// Frame Modbus de leitura (função 04)
//======================================================
static uint8_t READ_CMD[8] = {
    0x01,   // Slave ID
    0x04,   // Function: Read Input Registers
    0x00,   // Start addr high
    0x00,   // Start addr low
    0x00,   // Num regs high
    0x0A,   // Num regs low (10 registers)
    0xC5,   // CRC LOW
    0xCD    // CRC HIGH
};


//======================================================
// Função principal de leitura
//======================================================
bool pzem_read(pzem_data_t *out)
{
    // --- Enviar comando modbus ---
    for (int i = 0; i < sizeof(READ_CMD); i++)
        uart_putc_raw(UART_ID, READ_CMD[i]);

    sleep_ms(200);

    // --- Verificar resposta ---
    if (!uart_is_readable_within_us(UART_ID, 200000))
        return false;

    uint8_t buffer[64];
    int len = 0;

    while (uart_is_readable(UART_ID) && len < sizeof(buffer))
        buffer[len++] = uart_getc(UART_ID);

    if (len < 25) {
        printf("[PZEM] Pacote menor que 25 bytes (%d)\n", len);
        return false;
    }

    // --- Validar CRC ---
    uint16_t crc_calc = modbus_crc(buffer, len - 2);
    uint16_t crc_recv = buffer[len - 2] | (buffer[len - 1] << 8);

    if (crc_calc != crc_recv) {
        printf("[PZEM] CRC inválido calc=%04X recv=%04X\n", crc_calc, crc_recv);
        return false;
    }

    // --- Decodificar resposta ---
    out->voltage   = (buffer[ 3] << 8 | buffer[ 4]) / 10.0f;
    out->current   = (buffer[ 5] << 8 | buffer[ 6]) / 100.0f;
    out->power     = (buffer[ 7] << 8 | buffer[ 8]) / 10.0f;
    out->energy    = (buffer[ 9] << 8 | buffer[10]) / 1000.0f;
    out->frequency = (buffer[11] << 8 | buffer[12]) / 10.0f;
    out->pf        = (buffer[13] << 8 | buffer[14]) / 100.0f;

    return true;
}
