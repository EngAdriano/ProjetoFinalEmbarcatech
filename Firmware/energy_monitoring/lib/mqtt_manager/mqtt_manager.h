#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"

/* ===============================
   Tipo de mensagem MQTT
   =============================== */
typedef struct {
    char topic[128];
    char payload[512];
} mqtt_message_t;

/* ===============================
   Estado global
   =============================== */
extern volatile bool g_mqtt_connected;

/* ===============================
   API pública
   =============================== */
void mqtt_manager_init(void);

/* Tasks MQTT (criadas no main) */
void vTaskMQTTConnection(void *pv);
void vTaskMQTTPublisher(void *pv);

/* Publicação assíncrona */
void mqtt_publish_async(const char *topic, const char *payload);

#endif
