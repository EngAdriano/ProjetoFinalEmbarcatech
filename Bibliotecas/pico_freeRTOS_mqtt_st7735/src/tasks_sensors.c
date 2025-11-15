#include "tasks_sensors.h"
#include <stdio.h>
#include "aht10.h"

void task_sensors(void *pvParameters) {
    (void) pvParameters;
    if (!aht10_init()) {
        printf("AHT10 init failed\n");
    }

    while (1) {
        float temp = 0.0f;
        float hum = 0.0f;
        if (aht10_read(&temp, &hum)) {
            printf("AHT10: T=%.2f C, RH=%.2f%%\n", temp, hum);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
