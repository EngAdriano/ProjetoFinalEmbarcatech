#ifndef ENV_SENSORS_H
#define ENV_SENSORS_H

#include "FreeRTOS.h"
#include "queue.h"

/* =========================
 * Estrutura pública
 * ========================= */
typedef struct {
    float temperature;
    float humidity;
    float lux;
} env_sensor_data_t;

/* =========================
 * API do módulo
 * ========================= */

/* Inicializa I2C e sensores */
void env_sensors_init(void);

/* Task FreeRTOS dos sensores */
void env_sensors_task(void *pvParameters);

/* Permite informar a queue usada pelo módulo */
void env_sensors_set_queue(QueueHandle_t queue);

#endif /* ENV_SENSORS_H */
