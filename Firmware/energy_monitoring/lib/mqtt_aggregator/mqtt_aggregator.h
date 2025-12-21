#ifndef MQTT_AGGREGATOR_H
#define MQTT_AGGREGATOR_H

#include "FreeRTOS.h"
#include "task.h"

/* Task agregadora MQTT (criada no main) */
void vTaskMQTTAggregator(void *pv);

#endif
