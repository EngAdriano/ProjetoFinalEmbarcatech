#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <stdbool.h>

/* ===============================
   Tipo de mensagem MQTT
   =============================== */
typedef struct {
    char topic[128];
    char payload[256];
} mqtt_message_t;

/* ===============================
   Estado global do MQTT
   =============================== */
extern volatile bool g_mqtt_connected;

/* ===============================
   API do MQTT Manager
   =============================== */
void mqtt_manager_init(void);
void vTaskMQTT(void *pv);
void mqtt_publish_async(const char *topic, const char *payload);

#endif
