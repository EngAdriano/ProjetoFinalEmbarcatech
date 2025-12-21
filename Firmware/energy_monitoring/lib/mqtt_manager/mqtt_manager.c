#include <stdio.h>
#include <string.h>

#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "wifi_manager.h"
#include "mqtt_manager.h"

#include "payload_builder.h"
#include "pzem004t.h"
#include "env_sensors.h"

/* Filas criadas no main */
extern QueueHandle_t xQueuePZEM_MQTT;
extern QueueHandle_t xEnvSensorQueue;

/* ===============================
   Configurações MQTT
   =============================== */
#define MQTT_BROKER        "broker.hivemq.com"
#define MQTT_BROKER_PORT   1883
#define MQTT_CLIENT_ID     "pico_freertos_client"
#define MQTT_KEEPALIVE     60
#define MQTT_DNS_RETRY_MS  5000

#define MQTT_TOPIC_PZEM "embarcartech/energy/pzem"


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
static TickType_t last_dns_try = 0;

/* ===============================
   Prototypes internos
   =============================== */
static void mqtt_dns_callback(const char *name,
                              const ip_addr_t *ipaddr,
                              void *callback_arg);

static void mqtt_connection_cb(mqtt_client_t *client,
                               void *arg,
                               mqtt_connection_status_t status);

/* ===============================
   Inicialização do módulo
   =============================== */
void mqtt_manager_init(void)
{
    mqtt_client = mqtt_client_new();
    if (!mqtt_client) {
        printf("[MQTT] ERRO: mqtt_client_new() retornou NULL\n");
        return;
    }

    mqttQueue = xQueueCreate(1, sizeof(mqtt_message_t));
    if (!mqttQueue) {
        printf("[MQTT] ERRO: Falha ao criar fila MQTT\n");
    }

    g_mqtt_connected = false;
}

/* ===============================
   Publicação assíncrona
   =============================== */
void mqtt_publish_async(const char *topic, const char *payload)
{
    if (!mqttQueue) return;

    mqtt_message_t msg;
    snprintf(msg.topic, sizeof(msg.topic), "%s", topic);
    snprintf(msg.payload, sizeof(msg.payload), "%s", payload);

    //xQueueSend(mqttQueue, &msg, 0);
    xQueueOverwrite(mqttQueue, &msg);

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
   Task: Gerenciamento de conexão
   =============================== */
void vTaskMQTTConnection(void *pv)
{
    (void) pv;

    for (;;)
    {
        if (!g_wifi_connected || !mqtt_client) {
            g_mqtt_connected = false;
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        if (!g_mqtt_connected &&
            (xTaskGetTickCount() - last_dns_try) > pdMS_TO_TICKS(MQTT_DNS_RETRY_MS))
        {
            printf("[MQTT] Resolvendo DNS...\n");
            last_dns_try = xTaskGetTickCount();

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

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* ===============================
   Task: Publicação MQTT
   =============================== */
void vTaskMQTTPublisher(void *pv)
{
    (void) pv;

    mqtt_message_t msg;
    err_t err;

    for (;;)
    {
        if (!g_mqtt_connected) {
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        if (xQueueReceive(mqttQueue, &msg, portMAX_DELAY)) {

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

            if (err != ERR_OK) {
            /* Buffer cheio ou conexão instável */
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
    }

            /* OBRIGATÓRIO */
            vTaskDelay(pdMS_TO_TICKS(300));
        }
    }
}
