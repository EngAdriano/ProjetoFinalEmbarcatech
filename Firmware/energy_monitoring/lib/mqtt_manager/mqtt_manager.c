#include <stdio.h>
#include <string.h>

#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "wifi_manager.h"
#include "mqtt_manager.h"

/* ===============================
   Configurações MQTT
   =============================== */
#define MQTT_BROKER        "broker.hivemq.com"
#define MQTT_BROKER_PORT   1883
#define MQTT_CLIENT_ID     "pico_freertos_client"
#define MQTT_KEEPALIVE     60

/* ===============================
   Estado global
   =============================== */
volatile bool g_mqtt_connected = false;

/* ===============================
   Objetos internos
   =============================== */
static mqtt_client_t *mqtt_client = NULL;
static ip_addr_t broker_ip;
static QueueHandle_t mqttQueue = NULL;

/* ===============================
   Callbacks
   =============================== */
static void mqtt_dns_callback(const char *name,
                             const ip_addr_t *ipaddr,
                             void *callback_arg);

static void mqtt_connection_cb(mqtt_client_t *client,
                               void *arg,
                               mqtt_connection_status_t status);

/* ===============================
   Inicialização do MQTT
   =============================== */
void mqtt_manager_init(void)
{
    mqtt_client = mqtt_client_new();
    if (!mqtt_client) {
        printf("[MQTT] ERRO: mqtt_client_new() retornou NULL\n");
        return;
    }

    mqttQueue = xQueueCreate(10, sizeof(mqtt_message_t));
    if (!mqttQueue) {
        printf("[MQTT] ERRO: Falha ao criar fila MQTT\n");
    }
}

/* ===============================
   Publicação assíncrona
   =============================== */
void mqtt_publish_async(const char *topic, const char *payload)
{
    if (!mqttQueue || !g_mqtt_connected) return;

    mqtt_message_t msg;
    snprintf(msg.topic, sizeof(msg.topic), "%s", topic);
    snprintf(msg.payload, sizeof(msg.payload), "%s", payload);

    xQueueSend(mqttQueue, &msg, 0);
}

/* ===============================
   DNS callback
   =============================== */
static void mqtt_dns_callback(const char *name,
                              const ip_addr_t *ipaddr,
                              void *callback_arg)
{
    if (!ipaddr) {
        printf("[MQTT] DNS falhou para %s\n", name);
        return;
    }

    broker_ip = *ipaddr;

    struct mqtt_connect_client_info_t ci = {
        .client_id  = MQTT_CLIENT_ID,
        .keep_alive = MQTT_KEEPALIVE
    };

    printf("[MQTT] Conectando ao broker...\n");

    mqtt_client_connect(
        mqtt_client,
        &broker_ip,
        MQTT_BROKER_PORT,
        mqtt_connection_cb,
        NULL,
        &ci
    );
}

/* ===============================
   Callback de conexão
   =============================== */
static void mqtt_connection_cb(mqtt_client_t *client,
                               void *arg,
                               mqtt_connection_status_t status)
{
    if (status == MQTT_CONNECT_ACCEPTED) {
        g_mqtt_connected = true;
        printf("[MQTT] Conectado ao broker\n");
    } else {
        g_mqtt_connected = false;
        printf("[MQTT] Erro de conexão (%d)\n", status);
    }
}

/* ===============================
   Task MQTT
   =============================== */
void vTaskMQTT(void *pv)
{
    while (1)
    {
        if (!g_wifi_connected) {
            g_mqtt_connected = false;
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        if (!g_mqtt_connected) {
            printf("[MQTT] Resolvendo DNS...\n");

            err_t err = dns_gethostbyname(
                MQTT_BROKER,
                &broker_ip,
                mqtt_dns_callback,
                NULL
            );

            if (err == ERR_OK) {
                mqtt_dns_callback(MQTT_BROKER, &broker_ip, NULL);
            }
        }

        if (g_mqtt_connected) {
            mqtt_message_t msg;

            if (xQueueReceive(mqttQueue, &msg, pdMS_TO_TICKS(500))) {
                mqtt_publish(
                    mqtt_client,
                    msg.topic,
                    msg.payload,
                    strlen(msg.payload),
                    0,
                    0,
                    NULL,
                    NULL
                );
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
