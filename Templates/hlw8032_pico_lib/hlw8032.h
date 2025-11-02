#ifndef HLW8032_H
#define HLW8032_H

#include "pico/stdlib.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float v_ref;
    float i_ref;
    float p_ref;
    uint32_t v_coef;
    uint32_t i_coef;
    uint32_t p_coef;
} hlw8032_calibration_t;

typedef struct {
    uart_inst_t *uart;
    uint tx_pin;
    uint rx_pin;
    uint32_t voltage_raw;
    uint32_t current_raw;
    uint32_t power_raw;
    uint32_t energy_raw;
    uint8_t pf_raw;
    float voltage;
    float current;
    float power;
    float pf;
    float energy_wh;
    hlw8032_calibration_t calib;
    absolute_time_t last_update;
} hlw8032_t;

void hlw8032_init(hlw8032_t *dev, uart_inst_t *uart_id, uint rx_pin, uint baudrate);
bool hlw8032_read_frame(hlw8032_t *dev);
void hlw8032_process_data(hlw8032_t *dev);
void hlw8032_set_calibration(hlw8032_t *dev, float v_ref, float i_ref, float p_ref);
void hlw8032_print_data(hlw8032_t *dev);
float hlw8032_get_voltage(hlw8032_t *dev);
float hlw8032_get_current(hlw8032_t *dev);
float hlw8032_get_power(hlw8032_t *dev);
float hlw8032_get_energy(hlw8032_t *dev);
float hlw8032_get_pf(hlw8032_t *dev);

#endif
