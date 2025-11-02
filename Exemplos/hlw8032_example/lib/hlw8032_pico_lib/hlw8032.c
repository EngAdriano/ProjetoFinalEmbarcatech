#include "hlw8032.h"
#include <stdio.h>
#include <string.h>

#define HLW_FRAME_SIZE 24
#define HLW_HEADER1 0x55
#define HLW_HEADER2 0x5A

#define DEFAULT_V_COEF  1.88f
#define DEFAULT_I_COEF  0.72f
#define DEFAULT_P_COEF  1.0f

static uint8_t frame_buf[HLW_FRAME_SIZE];

void hlw8032_init(hlw8032_t *dev, uart_inst_t *uart_id, uint rx_pin, uint baudrate) {
    dev->uart = uart_id;
    dev->rx_pin = rx_pin;
    dev->tx_pin = 0;
    dev->voltage_raw = 0;
    dev->current_raw = 0;
    dev->power_raw = 0;
    dev->energy_raw = 0;
    dev->pf_raw = 0;
    dev->energy_wh = 0.0f;

    dev->calib.v_ref = 220.0f;
    dev->calib.i_ref = 1.0f;
    dev->calib.p_ref = 220.0f;
    dev->calib.v_coef = DEFAULT_V_COEF;
    dev->calib.i_coef = DEFAULT_I_COEF;
    dev->calib.p_coef = DEFAULT_P_COEF;

    uart_init(dev->uart, baudrate);
    gpio_set_function(rx_pin, GPIO_FUNC_UART);
    uart_set_format(dev->uart, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(dev->uart, false);
}

bool hlw8032_read_frame(hlw8032_t *dev) {
    if (uart_is_readable(dev->uart)) {
        if (uart_getc(dev->uart) != HLW_HEADER1) return false;
        if (!uart_is_readable(dev->uart) || uart_getc(dev->uart) != HLW_HEADER2) return false;

        frame_buf[0] = HLW_HEADER1;
        frame_buf[1] = HLW_HEADER2;

        for (int i = 2; i < HLW_FRAME_SIZE; i++) {
            int timeout = 10000;
            while (!uart_is_readable(dev->uart) && timeout-- > 0) tight_loop_contents();
            frame_buf[i] = uart_getc(dev->uart);
        }

        uint8_t checksum = 0;
        for (int i = 2; i < HLW_FRAME_SIZE - 1; i++)
            checksum += frame_buf[i];
        checksum = (~checksum) + 1;

        if (checksum != frame_buf[HLW_FRAME_SIZE - 1])
            return false;

        dev->voltage_raw = (frame_buf[2] << 16) | (frame_buf[3] << 8) | frame_buf[4];
        dev->current_raw = (frame_buf[5] << 16) | (frame_buf[6] << 8) | frame_buf[7];
        dev->power_raw   = (frame_buf[8] << 16) | (frame_buf[9] << 8) | frame_buf[10];
        dev->energy_raw  = (frame_buf[11] << 16) | (frame_buf[12] << 8) | frame_buf[13];
        dev->pf_raw      = frame_buf[14];

        hlw8032_process_data(dev);
        dev->last_update = get_absolute_time();

        return true;
    }
    return false;
}

void hlw8032_process_data(hlw8032_t *dev) {
    dev->voltage = (float)dev->voltage_raw / dev->calib.v_coef;
    dev->current = (float)dev->current_raw / dev->calib.i_coef;
    dev->power   = (float)dev->power_raw   / dev->calib.p_coef;
    dev->pf      = (float)dev->pf_raw / 100.0f;
    dev->energy_wh += dev->power * 0.001f;
}

void hlw8032_set_calibration(hlw8032_t *dev, float v_ref, float i_ref, float p_ref) {
    dev->calib.v_ref = v_ref;
    dev->calib.i_ref = i_ref;
    dev->calib.p_ref = p_ref;
}

float hlw8032_get_voltage(hlw8032_t *dev) { return dev->voltage; }
float hlw8032_get_current(hlw8032_t *dev) { return dev->current; }
float hlw8032_get_power(hlw8032_t *dev)   { return dev->power; }
float hlw8032_get_energy(hlw8032_t *dev)  { return dev->energy_wh; }
float hlw8032_get_pf(hlw8032_t *dev)      { return dev->pf; }

void hlw8032_print_data(hlw8032_t *dev) {
    printf("Tensão: %.2f V | Corrente: %.3f A | Potência: %.2f W | PF: %.2f | Energia: %.3f Wh\n",
        dev->voltage, dev->current, dev->power, dev->pf, dev->energy_wh);
}
