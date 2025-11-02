/* hlw8032.h
 * Biblioteca C para HLW8032 (parser UART + cálculos de energia)
 * Autor: ChatGPT (exemplo entregue ao usuário)
 * Versão: 1.0
 * Resumo: biblioteca portátil em C para receber os 24 bytes de cada pacote UART
 * do HLW8032, validar pacote, expor valores brutos (registros) e funções de
 * cálculo: Vrms, Irms, Power active, apparent, power factor e energia (Wh).
 */

#ifndef HLW8032_H
#define HLW8032_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HLW_FRAME_BYTES 24
#define HLW_REG_COUNT 11

typedef struct {
    uint8_t raw[HLW_FRAME_BYTES];
    uint32_t reg[HLW_REG_COUNT+1];
    bool valid;
    uint32_t last_update_ms;
} hlw_frame_t;

typedef struct {
    hlw_frame_t frame;
    double voltage_coeff;
    double current_coeff;
    double power_coeff;
    double energy_Wh;
    uint32_t acc_last_ms;
    uint32_t pf_pulse_count;
    uint32_t (*hal_millis)(void);
} hlw_ctx_t;

void hlw_init(hlw_ctx_t *ctx, uint32_t (*millis_func)(void));
void hlw_feed_byte(hlw_ctx_t *ctx, uint8_t byte);
bool hlw_frame_ready(hlw_ctx_t *ctx);
double hlw_get_voltage(hlw_ctx_t *ctx);
double hlw_get_current(hlw_ctx_t *ctx);
double hlw_get_active_power(hlw_ctx_t *ctx);
double hlw_get_apparent_power(hlw_ctx_t *ctx);
double hlw_get_power_factor(hlw_ctx_t *ctx);
double hlw_get_energy_Wh(hlw_ctx_t *ctx);
void hlw_reset_energy(hlw_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif // HLW8032_H
