#include <stdio.h>
#include <string.h>

/* lwIP */
#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Projeto */
#include "wifi_manager.h"
#include "mqtt_manager.h"

/* =========================================================
 * Configurações MQTT
 * ========================================================= */
#define MQTT_BROKER        "broker.hivemq.com"
#define MQTT_BROKER_PORT   1883
#define MQTT_CLIENT_ID     "pico_freertos_client"
#define MQTT_KEEPALIVE     60
#define MQTT_DNS_RETRY_MS  5000

/* =========================================================
 * Estado global público
 * ========================================================= */
volatile bool g_mqtt_connected = false;

/* =========================================================
 * Objetos internos
 * ========================================================= */
static mqtt_client_t *mqtt_client = NULL;
static ip_addr_t broker_ip;

static QueueHandle_t mqttQueue = NULL;
static TickType_t last_dns_try = 0;

/* Controle de fluxo do lwIP */
static volatile bool mqtt_publish_busy = false;

/* =========================================================
 * Prototipação interna
 * ========================================================= */
static void mqtt_dns_callback(const char *name,
                              const ip_addr_t *ipaddr,
                              void *callback_arg);

static void mqtt_connection_cb(mqtt_client_t *client,
                               void *arg,
                               mqtt_connection_status_t status);

static void mqtt_pub_request_cb(void *arg, err_t err);

/* =========================================================
 * Inicialização do módulo MQTT
 * ========================================================= */
void mqtt_manager_init(void)
{
    mqtt_client = mqtt_client_new();
    if (!mqtt_client) {
        printf("[MQTT] ERRO: mqtt_client_new() retornou NULL\n");
        return;
    }

    mqttQueue = xQueueCreate(4, sizeof(mqtt_message_t));
    if (!mqttQueue) {
        printf("[MQTT] ERRO: Falha ao criar fila MQTT\n");
        return;
    }

    g_mqtt_connected = false;
    mqtt_publish_busy = false;

    //printf("[MQTT] Manager inicializado\n");
}

/* =========================================================
 * Publicação assíncrona (thread-safe)
 * ========================================================= */
void mqtt_publish_async(const char *topic, const char *payload)
{
    if (!mqttQueue || !topic || !payload)
        return;

    mqtt_message_t msg;

    snprintf(msg.topic, sizeof(msg.topic), "%s", topic);
    snprintf(msg.payload, sizeof(msg.payload), "%s", payload);

    xQueueSend(mqttQueue, &msg, 0);
}

/* =========================================================
 * Callback: publicação concluída
 * ========================================================= */
static void mqtt_pub_request_cb(void *arg, err_t err)
{
    (void)arg;
    (void)err;
    mqtt_publish_busy = false;
}

/* =========================================================
 * Callback: DNS resolvido
 * ========================================================= */
static void mqtt_dns_callback(const char *name,
                              const ip_addr_t *ipaddr,
                              void *callback_arg)
{
    (void)name;
    (void)callback_arg;

    if (!ipaddr) {
        printf("[MQTT] DNS falhou\n");
        return;
    }

    broker_ip = *ipaddr;

    struct mqtt_connect_client_info_t ci = {
        .client_id  = MQTT_CLIENT_ID,
        .keep_alive = MQTT_KEEPALIVE
    };

    mqtt_client_connect(
        mqtt_client,
        &broker_ip,
        MQTT_BROKER_PORT,
        mqtt_connection_cb,
        NULL,
        &ci
    );
}

/* =========================================================
 * Callback: conexão MQTT
 * ========================================================= */
static void mqtt_connection_cb(mqtt_client_t *client,
                               void *arg,
                               mqtt_connection_status_t status)
{
    (void)client;
    (void)arg;

    if (status == MQTT_CONNECT_ACCEPTED) {
        g_mqtt_connected = true;
        //printf("[MQTT] Conectado ao broker\n");
    } else {
        g_mqtt_connected = false;
        printf("[MQTT] Falha na conexão (%d)\n", status);
    }
}

/* =========================================================
 * Task: Gerenciamento de conexão MQTT
 * ========================================================= */
void vTaskMQTTConnection(void *pv)
{
    (void) pv;

    for (;;)
    {
        if (!g_wifi_connected || !mqtt_client)
        {
            g_mqtt_connected = false;
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        if (!g_mqtt_connected &&
            (xTaskGetTickCount() - last_dns_try) > pdMS_TO_TICKS(MQTT_DNS_RETRY_MS))
        {
            last_dns_try = xTaskGetTickCount();

            err_t err = dns_gethostbyname(
                MQTT_BROKER,
                &broker_ip,
                mqtt_dns_callback,
                NULL
            );

            if (err == ERR_OK)
            {
                mqtt_dns_callback(MQTT_BROKER, &broker_ip, NULL);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* =========================================================
 * Task: Publicação MQTT
 * ========================================================= */
void vTaskMQTTPublisher(void *pv)
{
    (void) pv;

    mqtt_message_t msg;
    err_t err;

    for (;;)
    {
        if (!g_mqtt_connected)
        {
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        if (xQueueReceive(mqttQueue, &msg, portMAX_DELAY))
        {
            if (mqtt_publish_busy)
            {
                vTaskDelay(pdMS_TO_TICKS(100));
                continue;
            }

            mqtt_publish_busy = true;

            err = mqtt_publish(
                mqtt_client,
                msg.topic,
                msg.payload,
                strlen(msg.payload),
                0,  /* QoS */
                0,  /* retain */
                mqtt_pub_request_cb,
                NULL
            );

            if (err != ERR_OK)
            {
                mqtt_publish_busy = false;
                vTaskDelay(pdMS_TO_TICKS(200));
                continue;
            }

            vTaskDelay(pdMS_TO_TICKS(500)); /* evita flood TCP */
        }
    }
}

/* =========================================================
 * API pública de status (usada pela UI)
 * ========================================================= */
bool mqtt_is_connected(void)
{
    return g_mqtt_connected;
}
