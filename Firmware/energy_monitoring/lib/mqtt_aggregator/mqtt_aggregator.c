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

/* =====================================================
 * Cache interno (últimos valores válidos)
 * ===================================================== */
static env_sensor_data_t env_last = {0};
static energy_data_t    energy_last = {0};

/* =====================================================
 * API pública – leitura do cache
 * ===================================================== */
const env_sensor_data_t *env_get_last(void)
{
    return &env_last;
}

const energy_data_t *energy_get_last(void)
{
    return &energy_last;
}

/* =====================================================
 * Task MQTT Aggregator
 * ===================================================== */
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

    for (;;)
    {
        if (xQueueReceive(xQueuePZEM_MQTT, &pzem, portMAX_DELAY))
        {
            /* ---------------- Energia (cache + payload) ---------------- */
            energy_last.voltage   = pzem.voltage;
            energy_last.current   = pzem.current;
            energy_last.power     = pzem.power;
            energy_last.energy    = pzem.energy;
            energy_last.frequency = pzem.frequency;
            energy_last.pf        = pzem.pf;

            energy = (payload_energy_t){
                .voltage   = pzem.voltage,
                .current   = pzem.current,
                .power     = pzem.power,
                .energy    = pzem.energy,
                .frequency = pzem.frequency,
                .pf        = pzem.pf
            };

            /* ---------------- Ambiente (cache) ---------------- */
            if (xQueueReceive(xEnvSensorQueue, &env_raw, 0) == pdTRUE)
            {
                env_last = env_raw;
            }

            env.temperature = env_last.temperature;
            env.humidity    = env_last.humidity;
            env.lux         = env_last.lux;

            /* ---------------- Timestamp ---------------- */
            time_manager_get(&now);
            snprintf(timestamp, sizeof(timestamp),
                     "%04d-%02d-%02dT%02d:%02d:%02d",
                     now.year,
                     now.month,
                     now.day,
                     now.hour,
                     now.min,
                     now.sec);

            /* ---------------- JSON + MQTT ---------------- */
            if (payload_build_energy_json(
                    payload,
                    sizeof(payload),
                    &energy,
                    &env,
                    timestamp))
            {
                mqtt_publish_async(MQTT_TOPIC_PZEM, payload);
            }
        }
    }
}
