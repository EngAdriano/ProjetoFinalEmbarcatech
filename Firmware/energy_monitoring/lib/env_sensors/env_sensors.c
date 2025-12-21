#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Drivers */
#include "aht10.h"
#include "bh1750.h"

/* Módulo */
#include "env_sensors.h"

extern SemaphoreHandle_t xI2CMutex;

/* =========================
 * Configuração de I2C
 * ========================= */
#define I2C_PORT_ENV   i2c1
#define SDA_ENV        2
#define SCL_ENV        3
#define I2C_BAUDRATE   100000

/* =========================
 * Variáveis internas
 * ========================= */
static QueueHandle_t envQueue;

/* =========================
 * Wrappers AHT10
 * ========================= */
extern SemaphoreHandle_t xI2CMutex;

int i2c_write_wrapper(uint8_t addr, const uint8_t *data, uint16_t len)
{
    if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
        int ret = (i2c_write_blocking(I2C_PORT_ENV, addr, data, len, false) < 0) ? -1 : 0;
        xSemaphoreGive(xI2CMutex);
        return ret;
    }
    return -1;
}

int i2c_read_wrapper(uint8_t addr, uint8_t *data, uint16_t len)
{
    if (xSemaphoreTake(xI2CMutex, portMAX_DELAY) == pdTRUE) {
        int ret = (i2c_read_blocking(I2C_PORT_ENV, addr, data, len, false) < 0) ? -1 : 0;
        xSemaphoreGive(xI2CMutex);
        return ret;
    }
    return -1;
}





static void delay_ms_wrapper(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

/* =========================
 * API pública
 * ========================= */
void env_sensors_set_queue(QueueHandle_t queue)
{
    envQueue = queue;
}

void env_sensors_init(void)
{
    /* Inicializa I2C */
    i2c_init(I2C_PORT_ENV, I2C_BAUDRATE);
    gpio_set_function(SDA_ENV, GPIO_FUNC_I2C);
    gpio_set_function(SCL_ENV, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_ENV);
    gpio_pull_up(SCL_ENV);
}

/* =========================
 * Task dos sensores
 * ========================= */
void env_sensors_task(void *pvParameters)
{
    AHT10_Handle aht10 = {
        .iface = {
            .i2c_write = i2c_write_wrapper,
            .i2c_read  = i2c_read_wrapper,
            .delay_ms  = delay_ms_wrapper
        },
        .initialized = false
    };

    printf("[ENV] Inicializando AHT10...\n");
    if (!AHT10_Init(&aht10)) {
        printf("[ENV] ERRO AHT10\n");
        vTaskDelete(NULL);
    }

    printf("[ENV] Inicializando BH1750...\n");
    bh1750_init(I2C_PORT_ENV);

    while (1)
    {
        env_sensor_data_t data = {0};

        if (AHT10_ReadTemperatureHumidity(&aht10,
                                          &data.temperature,
                                          &data.humidity))
        {
            float lux = bh1750_read_lux(I2C_PORT_ENV);
            data.lux = (lux >= 0.0f) ? lux : -1.0f;

            xQueueOverwrite(envQueue, &data);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
