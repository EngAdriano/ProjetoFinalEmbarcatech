#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <stdint.h>

/* =========================
 * Data/hora padronizada do sistema
 * ========================= */
typedef struct {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  min;
    uint8_t  sec;
} sys_datetime_t;

/* =========================
 * API pública
 * ========================= */
void time_manager_init(void);
void time_manager_set_queue(QueueHandle_t queue);
void time_manager_get(sys_datetime_t *dt);
void time_manager_set(const sys_datetime_t *dt);

/* =========================
 * Task FreeRTOS
 * ========================= */
void task_time(void *pv);

#endif
