#include "mqtt_aggregator.h"
#include "queue.h"
#include <stdio.h>

#include "pzem004t.h"
#include "env_sensors.h"
#include "payload_builder.h"
#include "mqtt_manager.h"

/* Filas criadas no main */
extern QueueHandle_t xQueuePZEM_MQTT;
extern QueueHandle_t xEnvSensorQueue;

#define MQTT_TOPIC_PZEM "embarcartech/energy/pzem"

void vTaskMQTTAggregator(void *pv)
{
    (void) pv;

    pzem_data_t pzem;
    env_sensor_data_t env_raw;

    payload_energy_t energy;
    payload_environment_t env;

    char payload[512];

    printf("[MQTT][AGG] Task iniciada\n");

    for (;;)
    {
        /* Gatilho: novo dado de energia */
        if (xQueueReceive(xQueuePZEM_MQTT, &pzem, portMAX_DELAY))
        {
            /* Converte dados de energia */
            energy.voltage   = pzem.voltage;
            energy.current   = pzem.current;
            energy.power     = pzem.power;
            energy.energy    = pzem.energy;
            energy.frequency = pzem.frequency;
            energy.pf        = pzem.pf;

            /* Dados ambientais (opcional) */
            if (xQueuePeek(xEnvSensorQueue, &env_raw, 0) == pdTRUE) {
                env.temperature = env_raw.temperature;
                env.humidity    = env_raw.humidity;
                env.lux         = env_raw.lux;
            } else {
                env.temperature = 0;
                env.humidity    = 0;
                env.lux         = 0;
            }

            /* Monta JSON */
            if (payload_build_energy_json(
                    payload,
                    sizeof(payload),
                    &energy,
                    &env,
                    NULL   /* timestamp FUTURO */
                ))
            {
                mqtt_publish_async(MQTT_TOPIC_PZEM, payload);
                printf("[MQTT] %s\n", payload);
            }
        }
    }
}
