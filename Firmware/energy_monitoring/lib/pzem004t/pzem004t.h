#ifndef PZEM004T_H
#define PZEM004T_H

#include <stdbool.h>
#include <stdint.h>
#include "pico/stdlib.h"

// UART config (UART0, GPIO0 TX / GPIO1 RX)
#define PZEM_UART_ID     uart0
#define PZEM_UART_TX_PIN 0
#define PZEM_UART_RX_PIN 1
#define PZEM_BAUDRATE    9600

// Enable to print RAW response bytes for debugging
// #define PZEM_DEBUG

typedef struct {
    float voltage;      // V
    float current;      // A
    float power;        // W
    float energy;       // kWh
    float frequency;    // Hz
    float pf;           // power factor
} pzem_data_t;

void pzem_init(void);
bool pzem_read(pzem_data_t *out);

#endif // PZEM004T_H
