#ifndef TASKS_MQTT_H
#define TASKS_MQTT_H

#include "FreeRTOS.h"
#include "task.h"

void task_mqtt_client(void *pvParameters);

#endif // TASKS_MQTT_H
