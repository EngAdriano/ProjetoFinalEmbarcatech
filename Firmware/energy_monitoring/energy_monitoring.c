#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"

#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "pzem004t.h"
#include "rtc_ds3231.h"
#include "eeprom_at24c32.h"

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "main.h"
#include "st7735.h"
#include "fonts.h"
#include "testimg.h"

/* ===============================
   Queue global
   =============================== */
QueueHandle_t xQueuePZEM;

static void ui_draw_frame_landscape(void);
static void ui_update_pzem_landscape(const pzem_data_t *d);
void vTaskPZEMReader(void *pv);
void vTaskDisplay(void *pv);

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

    // call the LCD initialization
    ST7735_Init();

    if (cyw43_arch_init()) {
        printf("Erro ao iniciar CYW43\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    wifi_init_manager();
    pzem_init();
    //ds3231_init();
    xQueuePZEM = xQueueCreate(5, sizeof(pzem_data_t));
\
    // Iniciar MQTT
    mqtt_start();

    // Criar tasks
    xTaskCreate(vTaskPZEMReader, "PZEM Reader", 2048, NULL, 2, NULL);
    xTaskCreate(vTaskDisplay, "Display", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1) {}
}

/* ===============================
   Funções de UI
   =============================== */
static void ui_draw_frame_landscape(void)
{
    ST7735_FillScreen(ST7735_BLACK);

    ST7735_DrawRect(2, 2, 156, 124, ST7735_WHITE);

    ST7735_DrawString(40, 6, "ENERGY MONITOR",
                      Font_7x10, ST7735_CYAN, ST7735_BLACK);

    ST7735_DrawLine(2, 20, 158, 20, ST7735_WHITE);

    // Labels compactos
    ST7735_DrawString(6, 35,  "V:",  Font_7x10, ST7735_WHITE, ST7735_BLACK);
    ST7735_DrawString(6, 55,  "P:",  Font_7x10, ST7735_WHITE, ST7735_BLACK);
    ST7735_DrawString(6, 75,  "F:",  Font_7x10, ST7735_WHITE, ST7735_BLACK);

    ST7735_DrawString(80, 35, "I:",  Font_7x10, ST7735_WHITE, ST7735_BLACK);
    ST7735_DrawString(80, 55, "E:",  Font_7x10, ST7735_WHITE, ST7735_BLACK);
    ST7735_DrawString(80, 75, "PF:", Font_7x10, ST7735_WHITE, ST7735_BLACK);

    ST7735_DrawLine(2, 95, 158, 95, ST7735_WHITE);
    ST7735_DrawString(6, 105, "STATUS:", Font_7x10,
                      ST7735_WHITE, ST7735_BLACK);
}

static void ui_update_pzem_landscape(const pzem_data_t *d)
{
    char buf[20];

    sprintf(buf, "%4.1fV", d->voltage);
    ST7735_DrawString(22, 35, buf, Font_7x10, ST7735_GREEN, ST7735_BLACK);

    sprintf(buf, "%4.3fA", d->current);
    ST7735_DrawString(96, 35, buf, Font_7x10, ST7735_GREEN, ST7735_BLACK);

    sprintf(buf, "%5.1fW", d->power);
    ST7735_DrawString(22, 55, buf, Font_7x10, ST7735_GREEN, ST7735_BLACK);

    sprintf(buf, "%5.3fk", d->energy);   // kWh abreviado
    ST7735_DrawString(96, 55, buf, Font_7x10, ST7735_GREEN, ST7735_BLACK);

    sprintf(buf, "%4.1fHz", d->frequency);
    ST7735_DrawString(22, 75, buf, Font_7x10, ST7735_GREEN, ST7735_BLACK);

    sprintf(buf, "%3.2f", d->pf);
    ST7735_DrawString(104, 75, buf, Font_7x10, ST7735_GREEN, ST7735_BLACK);

    ST7735_DrawString(70, 105, "ACTIVE",
                      Font_7x10, ST7735_GREEN, ST7735_BLACK);
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

    ST7735_Init();
    ST7735_SetRotation(1);   // paisagem
    ui_draw_frame_landscape();

    while (1)
    {
        if (xQueueReceive(xQueuePZEM, &data, pdMS_TO_TICKS(200)))
        {
            ui_update_pzem_landscape(&data);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}