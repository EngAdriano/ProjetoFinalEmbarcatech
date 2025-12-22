#include "pzem004t.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include <stdio.h>

// Modbus command to read 10 input registers (0x0000..0x0009)
static uint8_t READ_CMD[8] = {
    0x01, // slave id
    0x04, // function: Read Input Registers
    0x00, 0x00, // start addr
    0x00, 0x0A, // num regs = 10
    0x00, 0x00  // CRC (filled at init)
};

// ----------------------------------------------------
// CRC-16 MODBUS (polynomial 0xA001), returns 16-bit
// ----------------------------------------------------
static uint16_t modbus_crc(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos];
        for (int i = 0; i < 8; i++) {
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

// ----------------------------------------------------
// Helper: build 32-bit value from four bytes in order:
// [b0,b1,b2,b3] -> b0<<24 | b1<<16 | b2<<8 | b3
// We will pass bytes in the specific order discovered
// from PZEM responses.
// ----------------------------------------------------
static inline uint32_t be32(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3)
{
    return ((uint32_t)b0 << 24) |
           ((uint32_t)b1 << 16) |
           ((uint32_t)b2 << 8)  |
           (uint32_t)b3;
}

// ----------------------------------------------------
// Initialize UART and prepare READ_CMD CRC
// ----------------------------------------------------
void pzem_init(void)
{
    uart_init(PZEM_UART_ID, PZEM_BAUDRATE);
    gpio_set_function(PZEM_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(PZEM_UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(PZEM_UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(PZEM_UART_ID, true);

    // Calculate CRC for READ_CMD (first 6 bytes)
    uint16_t crc = modbus_crc(READ_CMD, 6);
    READ_CMD[6] = crc & 0xFF;       // CRC low
    READ_CMD[7] = (crc >> 8) & 0xFF;// CRC high

    //printf("[PZEM] UART0 ready 9600 8N1\n");
}

// ----------------------------------------------------
// Read function: sends the standard 0x04 read request and
// parses response (expected 25 bytes).
// Returns true on success, false otherwise.
// ----------------------------------------------------
bool pzem_read(pzem_data_t *out)
{
    uint8_t rx[64];
    int rx_len = 0;

    // flush RX
    while (uart_is_readable(PZEM_UART_ID)) (void)uart_getc(PZEM_UART_ID);

    // send command
    for (int i = 0; i < 8; i++) uart_putc_raw(PZEM_UART_ID, READ_CMD[i]);

    // wait a bit for response
    sleep_ms(150);

    // wait readable (timeout)
    if (!uart_is_readable_within_us(PZEM_UART_ID, 200000)) {
        #ifdef PZEM_DEBUG
        printf("[PZEM] no response within timeout\n");
        #endif
        return false;
    }

    // read available bytes (max 64)
    while (uart_is_readable(PZEM_UART_ID) && rx_len < (int)sizeof(rx)) {
        rx[rx_len++] = (uint8_t)uart_getc(PZEM_UART_ID);
    }

    if (rx_len < 25) {
        #ifdef PZEM_DEBUG
        printf("[PZEM] short packet (%d bytes)\n", rx_len);
        #endif
        return false;
    }

    // optional raw dump
    #ifdef PZEM_DEBUG
    printf("[PZEM] RAW (%d): ", rx_len);
    for (int i = 0; i < rx_len; i++) printf("%02X ", rx[i]);
    printf("\n");
    #endif

    // validate CRC (last two bytes: low, high)
    uint16_t crc_calc = modbus_crc(rx, rx_len - 2);
    uint16_t crc_recv = (uint16_t)rx[rx_len - 2] | ((uint16_t)rx[rx_len - 1] << 8);
    if (crc_calc != crc_recv) {
        #ifdef PZEM_DEBUG
        printf("[PZEM] CRC mismatch calc=%04X recv=%04X\n", crc_calc, crc_recv);
        #endif
        return false;
    }

    // verify address & function
    if (rx[0] != 0x01 || rx[1] != 0x04) {
        #ifdef PZEM_DEBUG
        printf("[PZEM] unexpected addr/func: %02X %02X\n", rx[0], rx[1]);
        #endif
        return false;
    }

    // --- PARSING (offsets per PZEM V3/V4 observed layout) ---
    // rx[2] = byte count (should be 20)
    // Voltage: 16-bit at rx[3..4], scale /10
    out->voltage = (float)((rx[3] << 8) | rx[4]) / 10.0f;

    // Current: 32-bit assembled from bytes [5..8] but word-swapped order:
    // observed order in response -> we must assemble as [7][8][5][6]
    uint32_t current_raw = be32(rx[7], rx[8], rx[5], rx[6]);
    out->current = (float)current_raw / 1000.0f; // scale: /1000 (A)

    // Power: 32-bit from bytes [9..12] assembled as [11][12][9][10]
    uint32_t power_raw = be32(rx[11], rx[12], rx[9], rx[10]);
    out->power = (float)power_raw / 10.0f; // scale: /10 (W)

    // Energy: 32-bit from bytes [13..16] assembled as [15][16][13][14]
    uint32_t energy_raw = be32(rx[15], rx[16], rx[13], rx[14]);
    out->energy = (float)energy_raw / 1000.0f; // energy in Wh -> /1000 -> kWh

    // Frequency: 16-bit at rx[17..18] scale /10
    out->frequency = (float)((rx[17] << 8) | rx[18]) / 10.0f;

    // Power factor: 16-bit at rx[19..20] scale /100
    out->pf = (float)((rx[19] << 8) | rx[20]) / 100.0f;

    return true;
}
