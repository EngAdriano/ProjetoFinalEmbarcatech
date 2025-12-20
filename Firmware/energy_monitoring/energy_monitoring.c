#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Drivers e libs do projeto
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "pzem004t.h"
#include "rtc_ds3231.h"
#include "eeprom_at24c32.h"
#include "main.h"
#include "ui_energy.h"
#include "env_sensors.h"


#define MQTT_TOPIC_PZEM "embarcartech/energy/pzem"

/* ===============================
   Queue global
   =============================== */
QueueHandle_t xQueuePZEM_Display;
QueueHandle_t xQueuePZEM_MQTT;
QueueHandle_t xEnvSensorQueue;

/* ===============================
   Protótipos da funções
   =============================== */
void vTaskPZEMReader(void *pv);
void vTaskDisplay(void *pv);
void vTaskMQTTPublisher(void *pv);


int main() {
    stdio_init_all();

    // intialize the SPI0 of Raspberry Pi
    spi_init(SPI_PORT, 4000 * 1000);
    //gpio_set_function(LCD_MISO, GPIO_FUNC_SPI);
    gpio_set_function(LCD_SCK, GPIO_FUNC_SPI);
    gpio_set_function(LCD_MOSI, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(LCD_RST);
    gpio_set_dir(LCD_RST, GPIO_OUT);
    gpio_init(LCD_CS);
    gpio_set_dir(LCD_CS, GPIO_OUT);
    gpio_init(LCD_DC); //RS PIn
    gpio_set_dir(LCD_DC, GPIO_OUT);

    if (cyw43_arch_init()) {
        printf("Erro ao iniciar CYW43\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    wifi_manager_init();
    mqtt_manager_init();

    xQueuePZEM_Display = xQueueCreate(1, sizeof(pzem_data_t));
    xQueuePZEM_MQTT    = xQueueCreate(1, sizeof(pzem_data_t));
    xEnvSensorQueue = xQueueCreate(1, sizeof(env_sensor_data_t));

    if (xQueuePZEM_Display == NULL || xQueuePZEM_MQTT == NULL)
    {
        /* Falha crítica */
        while (1) {}
    }

    env_sensors_set_queue(xEnvSensorQueue);
    env_sensors_init();



    // Criar tasks
    xTaskCreate(vTaskWiFiManager, "WIFI", 2048, NULL, 3, NULL);
    xTaskCreate(vTaskMQTT, "MQTT", 4096, NULL, 2, NULL);
    xTaskCreate(vTaskPZEMReader, "PZEM", 2048, NULL, 2, NULL);
    xTaskCreate(vTaskDisplay, "DISPLAY", 4096, NULL, 1, NULL);
    xTaskCreate(vTaskMQTTPublisher, "MQTT_PUB", 2048, NULL, 2, NULL);
    xTaskCreate(env_sensors_task, "ENV_SENS", 2048, NULL, 3, NULL);
    
    vTaskStartScheduler();

    while (1) {}
}

/* ===============================
   Task de leitura do PZEM
   =============================== */
void vTaskPZEMReader(void *pv)
{
    pzem_data_t data;

    pzem_init();

    while (1)
    {
        if (pzem_read(&data))
        {
            xQueueOverwrite(xQueuePZEM_Display, &data);
            xQueueOverwrite(xQueuePZEM_MQTT, &data);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* ===============================
   Task de Display
   =============================== */
void vTaskDisplay(void *pv)
{
    pzem_data_t data;

    UI_Energy_ShowSplash();
    UI_Energy_Init();

    while (1)
    {
        if (xQueueReceive(xQueuePZEM_Display, &data, pdMS_TO_TICKS(300)))
        {
            UI_Energy_Update(&data);
        }

        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

void vTaskMQTTPublisher(void *pv)
{
    pzem_data_t energy;
    env_sensor_data_t env;
    char payload[512];

    /* Inicializa env com zeros para o primeiro publish */
    memset(&env, 0, sizeof(env));

    while (1)
    {
        /* Gatilho da publicação: novos dados do PZEM */
        if (xQueueReceive(xQueuePZEM_MQTT, &energy, pdMS_TO_TICKS(2000)))
        {
            if (!g_mqtt_connected)
                continue;

            /* Últimos dados ambientais (não bloqueante) */
            xQueuePeek(xEnvSensorQueue, &env, 0);

            /* Monta JSON único */
            snprintf(payload, sizeof(payload),
                     "{"
                     "\"energy\":{"
                     "\"voltage\":%.2f,"
                     "\"current\":%.3f,"
                     "\"power\":%.2f,"
                     "\"energy\":%.3f,"
                     "\"frequency\":%.1f,"
                     "\"pf\":%.2f"
                     "},"
                     "\"environment\":{"
                     "\"temperature\":%.2f,"
                     "\"humidity\":%.2f,"
                     "\"lux\":%.2f"
                     "}"
                     "}",
                     energy.voltage,
                     energy.current,
                     energy.power,
                     energy.energy,
                     energy.frequency,
                     energy.pf,
                     env.temperature,
                     env.humidity,
                     env.lux);

            mqtt_publish_async(MQTT_TOPIC_PZEM, payload);

            printf("[MQTT] %s\n", payload);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
