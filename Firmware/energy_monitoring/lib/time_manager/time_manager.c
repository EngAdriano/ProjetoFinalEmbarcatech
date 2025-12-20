#include <stdio.h>
#include "pico/stdlib.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* RTC DS3231 */
#include "rtc_ds3231.h"
#include "time_manager.h"

/* Queue criada no main */
extern QueueHandle_t timeQueue;

/* =====================================
 * Inicialização do RTC
 * ===================================== */
void time_manager_init(void)
{
    ds3231_init();
    printf("RTC DS3231 inicializado\n");
}

/* =====================================
 * Leitura da data/hora
 * ===================================== */
void time_manager_get(sys_datetime_t *dt)
{
    ds3231_time_t rtc;

    ds3231_get_time(&rtc);

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
    ds3231_time_t rtc;

    rtc.year    = dt->year;
    rtc.month   = dt->month;
    rtc.day     = dt->day;
    rtc.hours   = dt->hour;
    rtc.minutes = dt->min;
    rtc.seconds = dt->sec;

    ds3231_set_time(&rtc);
}

/* =====================================
 * Task FreeRTOS – Tempo do sistema
 * ===================================== */
void task_time(void *pv)
{
    sys_datetime_t now;

    printf("Task TIME iniciada\n");

    while (true) {
        time_manager_get(&now);
        xQueueOverwrite(timeQueue, &now);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
