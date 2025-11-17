#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"

// Protótipo da task
void vTaskSimulatedTemp(void *pvParameters);

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Erro ao iniciar CYW43\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    wifi_init_manager();
    mqtt_start();

    // Criar a task de temperatura simulada
    xTaskCreate(vTaskSimulatedTemp, "TempSimTask", 2048, NULL, 1, NULL);

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
