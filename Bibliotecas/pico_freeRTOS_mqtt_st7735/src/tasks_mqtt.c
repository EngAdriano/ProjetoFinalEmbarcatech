#include "tasks_mqtt.h"
#include <stdio.h>
#include <string.h>
#include "lwip/apps/mqtt.h"
#include "lwip/netdb.h"
#include "include/credentials.h"

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("MQTT connected!\n");
    } else {
        printf("MQTT connection failed: %d\n", status);
    }
}

void task_mqtt_client(void *pvParameters) {
    (void) pvParameters;
    vTaskDelay(pdMS_TO_TICKS(5000));

    printf("Starting MQTT client task\n");

    mqtt_client_t *client = mqtt_client_new();
    if (!client) {
        printf("Failed to create mqtt client\n");
        vTaskDelete(NULL);
        return;
    }

    struct mqtt_connect_client_info_t ci;
    memset(&ci, 0, sizeof(ci));
    ci.client_id = DEVICE_ID;
    ci.client_user = MQTT_USER;
    ci.client_pass = MQTT_PASS;

    struct hostent *he = gethostbyname(MQTT_HOST);
    if (!he) {
        printf("DNS resolution failed for %s\n", MQTT_HOST);
        for (;;) { vTaskDelay(pdMS_TO_TICKS(5000)); }
    }

    ip_addr_t ipaddr;
    memcpy(&ipaddr, he->h_addr_list[0], sizeof(ipaddr));

    // NOTE: TLS integration required for secure connection on port 8883
    err_t err = mqtt_client_connect(client, &ipaddr, MQTT_PORT, mqtt_connection_cb, NULL, &ci);
    if (err != ERR_OK) {
        printf("mqtt_client_connect failed: %d\n", err);
    }

    while (1) {
        const char *topic = "home/telemetry/" DEVICE_ID;
        const char *msg = "{\"status\":\"hello from pico\"}";
        err_t pub_err = mqtt_publish(client, topic, msg, strlen(msg), 1, 0, NULL, NULL);
        if (pub_err == ERR_OK) {
            printf("Published sample message\n");
        }
        vTaskDelay(pdMS_TO_TICKS(15000));
    }
}
