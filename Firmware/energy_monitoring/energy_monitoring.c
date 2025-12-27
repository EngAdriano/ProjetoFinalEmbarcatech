#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Tasks / módulos */
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "mqtt_aggregator.h"
#include "pzem_task.h"
#include "ui_energy_task.h"
#include "env_sensors.h"
#include "time_manager.h"
#include "web_server_task.h"

#include "pzem004t.h"
#include "rtc_ds3231.h"
#include "eeprom_at24c32.h"
#include "main.h"

/* ===============================
   Queues globais
   =============================== */
QueueHandle_t xQueuePZEM_Display;
QueueHandle_t xQueuePZEM_MQTT;
QueueHandle_t xEnvSensorQueue;
QueueHandle_t timeQueue;

/* Mutex para acesso exclusivo ao I2C */
SemaphoreHandle_t xI2CMutex;

int main(void)
{
    stdio_init_all();

    factory_button_init();

    /* ==== SPI / Display ==== */
    spi_init(SPI_PORT, 4000 * 1000);
    gpio_set_function(LCD_SCK, GPIO_FUNC_SPI);
    gpio_set_function(LCD_MOSI, GPIO_FUNC_SPI);

    gpio_init(LCD_RST);
    gpio_set_dir(LCD_RST, GPIO_OUT);
    gpio_init(LCD_CS);
    gpio_set_dir(LCD_CS, GPIO_OUT);
    gpio_init(LCD_DC);
    gpio_set_dir(LCD_DC, GPIO_OUT);

    /* ==== Wi-Fi chip ==== */
    if (cyw43_arch_init()) {
        printf("Erro ao iniciar CYW43\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    /* ==== Init módulos ==== */
    wifi_manager_init();
    mqtt_manager_init();

    /* ==== Filas ==== */
    xQueuePZEM_Display = xQueueCreate(1, sizeof(pzem_data_t));
    xQueuePZEM_MQTT    = xQueueCreate(1, sizeof(pzem_data_t));
    xEnvSensorQueue    = xQueueCreate(1, sizeof(env_sensor_data_t));
    timeQueue = xQueueCreate(1, sizeof(sys_datetime_t));

    if (!xQueuePZEM_Display || !xQueuePZEM_MQTT || !xEnvSensorQueue) {
        while (1) {}
    }

    time_manager_init();
    time_manager_set_queue(timeQueue);

    env_sensors_set_queue(xEnvSensorQueue);
    env_sensors_init();

    xI2CMutex = xSemaphoreCreateMutex();
    if (xI2CMutex == NULL) {
        printf("ERRO: Falha ao criar mutex I2C\n");
        while (1) {}
    }


    /* ==== Criação das tasks ==== */
    xTaskCreate(vTaskWiFiManager,     "WIFI",     2048, NULL, 3, NULL);
    xTaskCreate(vTaskWebServer,       "WEB",      2048, NULL, 1, NULL);

    xTaskCreate(vTaskMQTTConnection,  "MQTT_CONN",2048, NULL, 3, NULL);
    xTaskCreate(vTaskMQTTPublisher,   "MQTT_PUB", 2048, NULL, 2, NULL);
    xTaskCreate(vTaskMQTTAggregator, "MQTT_AGG", 3072, NULL, 2, NULL);

    xTaskCreate(vTaskPZEMReader,      "PZEM",     2048, NULL, 2, NULL);
    xTaskCreate(vTaskDisplay,         "DISPLAY",  4096, NULL, 1, NULL);
    xTaskCreate(env_sensors_task,     "ENV_SENS", 2048, NULL, 3, NULL);
    xTaskCreate(task_time, "TIME", 1024, NULL, 2, NULL);

    xTaskCreate(vTaskFactoryReset, "FactoryReset", 1024, NULL, tskIDLE_PRIORITY + 1, NULL);

    vTaskStartScheduler();

    while (1) {}
}
