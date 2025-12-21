#ifndef PAYLOAD_BUILDER_H
#define PAYLOAD_BUILDER_H

#include <stddef.h>
#include <stdbool.h>

/* Dados de energia */
typedef struct {
    float voltage;
    float current;
    float power;
    float energy;
    float frequency;
    float pf;
} payload_energy_t;

/* Dados ambientais */
typedef struct {
    float temperature;
    float humidity;
    float lux;
} payload_environment_t;

/* ===============================
   API pública
   =============================== */

/**
 * @brief Monta payload JSON de energia + ambiente
 *
 * @param buffer        Buffer de saída
 * @param buffer_len    Tamanho do buffer
 * @param energy        Dados de energia
 * @param env           Dados ambientais (opcional, pode ser NULL)
 * @param timestamp     Timestamp ISO8601 (opcional, pode ser NULL)
 *
 * @return true se o payload couber no buffer
 */
bool payload_build_energy_json(
    char *buffer,
    size_t buffer_len,
    const payload_energy_t *energy,
    const payload_environment_t *env,
    const char *timestamp
);

#endif
