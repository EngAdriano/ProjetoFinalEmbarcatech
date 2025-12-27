#ifndef MQTT_AGGREGATOR_H
#define MQTT_AGGREGATOR_H

#include "env_sensors.h"

/* =====================================================
 * Estrutura pública – Energia (cache)
 * ===================================================== */
typedef struct
{
    float voltage;
    float current;
    float power;
    float energy;
    float frequency;
    float pf;
} energy_data_t;

/* =====================================================
 * Task MQTT
 * ===================================================== */
void vTaskMQTTAggregator(void *pv);

/* =====================================================
 * Acesso aos últimos dados (cache)
 * ===================================================== */
const env_sensor_data_t   *env_get_last(void);
const energy_data_t       *energy_get_last(void);

#endif /* MQTT_AGGREGATOR_H */
