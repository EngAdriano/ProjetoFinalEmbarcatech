#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

#include "tasks_wifi.h"
#include "tasks_mqtt.h"
#include "tasks_sensors.h"
#include "tasks_display.h"

int main(void) {
    stdio_init_all();
    printf("Pico W FreeRTOS MQTT ST7735 skeleton\n");

    xTaskCreate(task_wifi_monitor, "wifi", TASK_STACK_WIFI, NULL, TASK_PRIORITY_WIFI, NULL);
    xTaskCreate(task_mqtt_client, "mqtt", TASK_STACK_MQTT, NULL, TASK_PRIORITY_MQTT, NULL);
    xTaskCreate(task_sensors, "sensors", TASK_STACK_SENS, NULL, TASK_PRIORITY_SENSORS, NULL);
    xTaskCreate(task_display, "display", TASK_STACK_DISP, NULL, TASK_PRIORITY_DISPLAY, NULL);

    vTaskStartScheduler();

    while(1) {
        tight_loop_contents();
    }
    return 0;
}
