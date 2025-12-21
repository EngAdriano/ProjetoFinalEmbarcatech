#include "payload_builder.h"
#include <stdio.h>

bool payload_build_energy_json(
    char *buffer,
    size_t buffer_len,
    const payload_energy_t *energy,
    const payload_environment_t *env,
    const char *timestamp
)
{
    if (!buffer || !energy) {
        return false;
    }

    int len = 0;

    /* Início do JSON */
    len += snprintf(buffer + len, buffer_len - len, "{");

    /* Timestamp (opcional) */
    if (timestamp) {
        len += snprintf(buffer + len, buffer_len - len,
                        "\"timestamp\":\"%s\",", timestamp);
    }

    /* Bloco de energia */
    len += snprintf(buffer + len, buffer_len - len,
                    "\"energy\":{"
                        "\"voltage\":%.2f,"
                        "\"current\":%.3f,"
                        "\"power\":%.2f,"
                        "\"energy\":%.3f,"
                        "\"frequency\":%.1f,"
                        "\"pf\":%.2f"
                    "}",
                    energy->voltage,
                    energy->current,
                    energy->power,
                    energy->energy,
                    energy->frequency,
                    energy->pf);

    /* Bloco ambiental (opcional) */
    if (env) {
        len += snprintf(buffer + len, buffer_len - len,
                        ",\"environment\":{"
                            "\"temperature\":%.2f,"
                            "\"humidity\":%.2f,"
                            "\"lux\":%.2f"
                        "}",
                        env->temperature,
                        env->humidity,
                        env->lux);
    }

    /* Final do JSON */
    len += snprintf(buffer + len, buffer_len - len, "}");

    /* Verificação de overflow */
    if (len <= 0 || (size_t)len >= buffer_len) {
        return false;
    }

    return true;
}
