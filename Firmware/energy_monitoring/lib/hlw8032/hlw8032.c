#include "hlw8032.h"
#include <string.h>

#define HLW8032_FRAME_HEADER 0x55
#define HLW8032_FRAME_LEN    24  // Pacote completo (header + dados + checksum)

// Função de inicialização
void hlw8032_init(hlw8032_config_t *cfg) {
    uart_init(cfg->uart, cfg->baudrate);
    gpio_set_function(cfg->tx_pin, GPIO_FUNC_UART);
    gpio_set_function(cfg->rx_pin, GPIO_FUNC_UART);
}

// Função auxiliar para ler pacote bruto
static bool hlw8032_read_raw(hlw8032_config_t *cfg, uint8_t *buf) {
    // Espera pelo header 0x55
    int ch;
    do {
        ch = uart_getc(cfg->uart);
    } while (ch != HLW8032_FRAME_HEADER);

    buf[0] = (uint8_t)ch;

    // Lê o resto do pacote
    for (int i = 1; i < HLW8032_FRAME_LEN; i++) {
        buf[i] = uart_getc(cfg->uart);
    }

    // Verifica checksum
    uint8_t checksum = 0;
    for (int i = 0; i < HLW8032_FRAME_LEN - 1; i++) {
        checksum += buf[i];
    }
    if ((checksum & 0xFF) != buf[HLW8032_FRAME_LEN - 1]) {
        return false;
    }

    return true;
}

// Faz parsing dos dados do HLW8032
bool hlw8032_read_packet(hlw8032_config_t *cfg, hlw8032_data_t *data) {
    uint8_t buf[HLW8032_FRAME_LEN];

    if (!hlw8032_read_raw(cfg, buf)) {
        return false;
    }

    // Cada valor vem em 3 bytes (24 bits)
    uint32_t voltage_reg = (buf[4] << 16) | (buf[5] << 8) | buf[6];
    uint32_t current_reg = (buf[10] << 16) | (buf[11] << 8) | buf[12];
    uint32_t power_reg   = (buf[16] << 16) | (buf[17] << 8) | buf[18];
    uint32_t pf_reg      = buf[20];  // fator de potência em %

    // Conversão (exemplo: precisa ajustar conforme calibração e datasheet!)
    data->voltage      = (float)voltage_reg / 100.0f;   // ~0.01 V por unidade
    data->current      = (float)current_reg / 1000.0f;  // ~0.001 A por unidade
    data->power        = (float)power_reg / 100.0f;     // ~0.01 W por unidade
    data->power_factor = (float)pf_reg / 100.0f;

    return true;
}
