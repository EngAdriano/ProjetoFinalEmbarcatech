#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "pzem004t.h"

// Protótipo das task's
void vTaskSimulatedTemp(void *pvParameters);
void vTaskPZEMReader(void *pvParameters);

// Função principal
int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Erro ao iniciar CYW43\n");
        return -1;
    }

    // Iniciar WiFi e MQTT
    cyw43_arch_enable_sta_mode();
    wifi_init_manager();
    mqtt_start();

    // Iniciar PZEM
    //pzem_init();

   // Criar a task de temperatura simulada
   //xTaskCreate(vTaskSimulatedTemp, "TempSimTask", 2048, NULL, 1, NULL);

    // Criar a task de leitura do PZEM
    //xTaskCreate(vTaskPZEMReader, "PZEMReader", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1) {}
}


void vTaskSimulatedTemp(void *pvParameters)
{
    float temp = 25.0f;   // temperatura inicial
    float step = 0.3f;    // variação por ciclo

    while (1)
    {
        // Gera oscilação da temperatura
        temp += step;
        if (temp > 30.0f || temp < 20.0f)
            step = -step;

        // Criar JSON
        char json[128];
        snprintf(json, sizeof(json), "{\"temperature\": %.2f}", temp);

        // Enviar via MQTT
        mqtt_publish_async("pico/sensor/temperature", json);

        printf("[TEMP] Enviado: %s\n", json);

        vTaskDelay(pdMS_TO_TICKS(5000)); // A cada 5 segundos
    }
}

void vTaskPZEMReader(void *pvParameters)
{
    pzem_data_t data;

    while (1)
    {
        if (pzem_read(&data))
        {
            char json[256];

            snprintf(json, sizeof(json),
                "{"
                "\"voltage\": %.1f,"
                "\"current\": %.3f,"
                "\"power\": %.1f,"
                "\"energy\": %.3f,"
                "\"frequency\": %.1f,"
                "\"pf\": %.2f"
                "}",
                data.voltage,
                data.current,
                data.power,
                data.energy,
                data.frequency,
                data.pf
            );

            mqtt_publish_async("pico/energy/data", json);

            printf("[PZEM] %s\n", json);
        }
        else
        {
            printf("[PZEM] Falha leitura\n");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

