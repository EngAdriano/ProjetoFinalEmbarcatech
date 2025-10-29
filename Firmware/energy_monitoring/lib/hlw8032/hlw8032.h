#ifndef HLW8032_H
#define HLW8032_H

#include "pico/stdlib.h"
#include "hardware/uart.h"

// Estrutura com os dados medidos
typedef struct {
    float voltage;      // Voltagem em Volts
    float current;      // Corrente em Amperes
    float power;        // Potência ativa em Watts
    float power_factor; // Fator de potência
} hlw8032_data_t;

// Configuração da UART do HLW8032
typedef struct {
    uart_inst_t *uart;   // Ponteiro para a UART usada (ex: uart0)
    uint tx_pin;         // GPIO do TX
    uint rx_pin;         // GPIO do RX
    uint baudrate;       // Velocidade (HLW8032 geralmente usa 4800 bps)
} hlw8032_config_t;

// Funções públicas
void hlw8032_init(hlw8032_config_t *cfg);
bool hlw8032_read_packet(hlw8032_config_t *cfg, hlw8032_data_t *data);

#endif
