/* hlw8032.c
 * Implementação da biblioteca HLW8032
 */

#include "hlw8032.h"
#include <string.h>

void hlw_init(hlw_ctx_t *ctx, uint32_t (*millis_func)(void)) {
    memset(ctx, 0, sizeof(hlw_ctx_t));
    ctx->hal_millis = millis_func;
    ctx->voltage_coeff = 1.0;
    ctx->current_coeff = 1.0;
    ctx->power_coeff = 1.0;
}

static uint8_t hlw_checksum(uint8_t *buf) {
    uint16_t sum = 0;
    for (int i = 0; i < 23; i++) sum += buf[i];
    return (uint8_t)(sum & 0xFF);
}

void hlw_feed_byte(hlw_ctx_t *ctx, uint8_t byte) {
    static uint8_t buffer[HLW_FRAME_BYTES];
    static uint8_t idx = 0;

    buffer[idx++] = byte;
    if (idx >= HLW_FRAME_BYTES) {
        idx = 0;
        if (buffer[0] == 0x55 && buffer[1] == 0x5A) {
            uint8_t chk = hlw_checksum(buffer);
            if (chk == buffer[23]) {
                ctx->frame.valid = true;
                memcpy(ctx->frame.raw, buffer, HLW_FRAME_BYTES);
                ctx->frame.last_update_ms = ctx->hal_millis ? ctx->hal_millis() : 0;

                for (int i = 0; i < HLW_REG_COUNT; i++) {
                    int j = 2 + i * 2;
                    ctx->frame.reg[i+1] = ((uint32_t)buffer[j] << 8) | buffer[j+1];
                }
            }
        }
    }
}

bool hlw_frame_ready(hlw_ctx_t *ctx) {
    return ctx->frame.valid;
}

double hlw_get_voltage(hlw_ctx_t *ctx) {
    return ctx->frame.valid ? ctx->frame.reg[2] * ctx->voltage_coeff : 0.0;
}

double hlw_get_current(hlw_ctx_t *ctx) {
    return ctx->frame.valid ? ctx->frame.reg[3] * ctx->current_coeff : 0.0;
}

double hlw_get_active_power(hlw_ctx_t *ctx) {
    return ctx->frame.valid ? ctx->frame.reg[4] * ctx->power_coeff : 0.0;
}

double hlw_get_apparent_power(hlw_ctx_t *ctx) {
    return hlw_get_voltage(ctx) * hlw_get_current(ctx);
}

double hlw_get_power_factor(hlw_ctx_t *ctx) {
    double p = hlw_get_active_power(ctx);
    double s = hlw_get_apparent_power(ctx);
    return (s > 0.0) ? (p / s) : 0.0;
}

double hlw_get_energy_Wh(hlw_ctx_t *ctx) {
    if (!ctx->hal_millis) return ctx->energy_Wh;
    uint32_t now = ctx->hal_millis();
    double dt_h = (now - ctx->acc_last_ms) / 3600000.0;
    ctx->energy_Wh += hlw_get_active_power(ctx) * dt_h;
    ctx->acc_last_ms = now;
    return ctx->energy_Wh;
}

void hlw_reset_energy(hlw_ctx_t *ctx) {
    ctx->energy_Wh = 0;
    ctx->acc_last_ms = ctx->hal_millis ? ctx->hal_millis() : 0;
}
