#include "mqtt_aggregator.h"
#include "queue.h"
#include <stdio.h>

#include "pzem004t.h"
#include "payload_builder.h"
#include "mqtt_manager.h"
#include "time_manager.h"

/* Filas criadas no main */
extern QueueHandle_t xQueuePZEM_MQTT;
extern QueueHandle_t xEnvSensorQueue;

#define MQTT_TOPIC_PZEM "embarcartech/energy/pzem"

static env_sensor_data_t env_last = {0};

const env_sensor_data_t *env_get_last(void)
{
    return &env_last;
}



void vTaskMQTTAggregator(void *pv)
{
    (void) pv;

    pzem_data_t pzem;
    env_sensor_data_t env_raw;

    payload_energy_t energy;
    payload_environment_t env;
    sys_datetime_t now;

    char payload[512];
    char timestamp[32];

    //printf("[MQTT][AGG] Task iniciada\n");

    for (;;)
    {
        if (xQueueReceive(xQueuePZEM_MQTT, &pzem, portMAX_DELAY))
        {
            /* Energia */
            energy.voltage   = pzem.voltage;
            energy.current   = pzem.current;
            energy.power     = pzem.power;
            energy.energy    = pzem.energy;
            energy.frequency = pzem.frequency;
            energy.pf        = pzem.pf;

            /* Ambiente */
            if (xQueueReceive(xEnvSensorQueue, &env_raw, 0) == pdTRUE)
                {
                    env_last = env_raw;   /* Atualiza cache */
                }

                /* Usa SEMPRE o último valor válido */
                env.temperature = env_last.temperature;
                env.humidity    = env_last.humidity;
                env.lux         = env_last.lux;


            /* Timestamp */
            time_manager_get(&now);
            snprintf(timestamp, sizeof(timestamp),
                     "%04d-%02d-%02dT%02d:%02d:%02d",
                     now.year,
                     now.month,
                     now.day,
                     now.hour,
                     now.min,
                     now.sec);

            /* JSON */
            if (payload_build_energy_json(
                    payload,
                    sizeof(payload),
                    &energy,
                    &env,
                    timestamp
                ))
            {
                mqtt_publish_async(MQTT_TOPIC_PZEM, payload);
                //printf("[MQTT] %s\n", payload);
            }
        }
    }
}
