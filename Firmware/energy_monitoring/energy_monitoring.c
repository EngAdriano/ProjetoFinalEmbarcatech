#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

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
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "main.h"
#include "ui_energy.h"

/* ===============================
   Queue global
   =============================== */
QueueHandle_t xQueuePZEM;

/* ===============================
   Protótipos da funções
   =============================== */
void vTaskPZEMReader(void *pv);
void vTaskDisplay(void *pv);
void vTaskWiFi(void *pv);

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

    xQueuePZEM = xQueueCreate(5, sizeof(pzem_data_t));

    if (xQueuePZEM == NULL)
    {
        /* Falha crítica */
        while (1) { }
    }

    // Criar tasks
    xTaskCreate(vTaskWiFiManager, "WIFI", 2048, NULL, 3, NULL);
    xTaskCreate(vTaskMQTT, "MQTT", 4096, NULL, 2, NULL);

    xTaskCreate(vTaskPZEMReader, "PZEM", 2048, NULL, 2, NULL);
    xTaskCreate(vTaskDisplay, "DISPLAY", 4096, NULL, 1, NULL);

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
            xQueueOverwrite(xQueuePZEM, &data);
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
        if (xQueueReceive(xQueuePZEM, &data, pdMS_TO_TICKS(300)))
        {
            UI_Energy_Update(&data);
        }

        vTaskDelay(pdMS_TO_TICKS(250));
    }
}