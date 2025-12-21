#include "payload_builder.h"
#include <stdio.h>

#define CHECK_SNPRINTF(ret, remaining) \
    do {                               \
        if ((ret) < 0 || (ret) >= (remaining)) { \
            return false;              \
        }                              \
    } while (0)

bool payload_build_energy_json(
    char *buffer,
    size_t buffer_len,
    const payload_energy_t *energy,
    const payload_environment_t *env,
    const char *timestamp
)
{
    if (!buffer || !energy || buffer_len == 0) {
        return false;
    }

    size_t len = 0;
    int ret;

    /* Início */
    ret = snprintf(buffer + len, buffer_len - len, "{");
    CHECK_SNPRINTF(ret, buffer_len - len);
    len += ret;

    /* Timestamp (opcional) */
    if (timestamp) {
        ret = snprintf(buffer + len, buffer_len - len,
                       "\"timestamp\":\"%s\",", timestamp);
        CHECK_SNPRINTF(ret, buffer_len - len);
        len += ret;
    }

    /* Energia */
    ret = snprintf(buffer + len, buffer_len - len,
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
    CHECK_SNPRINTF(ret, buffer_len - len);
    len += ret;

    /* Ambiente (opcional) */
    if (env) {
        ret = snprintf(buffer + len, buffer_len - len,
                       ",\"environment\":{"
                           "\"temperature\":%.2f,"
                           "\"humidity\":%.2f,"
                           "\"lux\":%.2f"
                       "}",
                       env->temperature,
                       env->humidity,
                       env->lux);
        CHECK_SNPRINTF(ret, buffer_len - len);
        len += ret;
    }

    /* Final */
    ret = snprintf(buffer + len, buffer_len - len, "}");
    CHECK_SNPRINTF(ret, buffer_len - len);
    len += ret;

    return true;
}
