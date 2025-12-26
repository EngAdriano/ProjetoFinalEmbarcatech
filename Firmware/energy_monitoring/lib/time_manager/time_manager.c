#include <stdio.h>
#include "pico/stdlib.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* RTC DS3231 */
#include "rtc_ds3231.h"
#include "time_manager.h"

/* =========================
 * Variáveis internas
 * ========================= */
static QueueHandle_t timeQueue = NULL;

/* =====================================
 * Inicialização do RTC
 * ===================================== */
void time_manager_init(void)
{
    ds3231_init();
    // printf("[TIME] RTC DS3231 inicializado\n");
}

/* Permite informar a fila ao módulo */
void time_manager_set_queue(QueueHandle_t queue)
{
    timeQueue = queue;
}

/* =====================================
 * Leitura da data/hora
 * ===================================== */
void time_manager_get(sys_datetime_t *dt)
{
    ds3231_time_t rtc;

    if (!ds3231_get_time(&rtc))
        return;

    dt->year  = rtc.year;
    dt->month = rtc.month;
    dt->day   = rtc.day;
    dt->hour  = rtc.hours;
    dt->min   = rtc.minutes;
    dt->sec   = rtc.seconds;
}

/* =====================================
 * Ajuste manual da data/hora
 * ===================================== */
void time_manager_set(const sys_datetime_t *dt)
{
    ds3231_time_t rtc = {
        .year         = dt->year,
        .month        = dt->month,
        .day          = dt->day,
        .hours        = dt->hour,
        .minutes      = dt->min,
        .seconds      = dt->sec,
        .day_of_week  = 1
    };

    ds3231_set_time(&rtc);
}

/* =====================================
 * Task FreeRTOS – Tempo do sistema
 * ===================================== */
void task_time(void *pv)
{
    (void) pv;

    sys_datetime_t now = {0};

    // printf("[TIME] Task TIME iniciada\n");

    while (true)
    {
        if (timeQueue != NULL)
        {
            time_manager_get(&now);
            xQueueOverwrite(timeQueue, &now);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
