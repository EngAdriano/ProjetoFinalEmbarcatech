#ifndef MQTT_AGGREGATOR_H
#define MQTT_AGGREGATOR_H

#include "env_sensors.h"   // <<< OBRIGATÓRIO

/* Task MQTT */
void vTaskMQTTAggregator(void *pv);

/* Acesso ao último dado ambiental (cache) */
const env_sensor_data_t *env_get_last(void);

#endif /* MQTT_AGGREGATOR_H */
