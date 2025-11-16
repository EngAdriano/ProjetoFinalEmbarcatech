#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define WIFI_SSID "Lu e Deza"
#define WIFI_PASSWORD "liukin1208"

volatile bool wifi_connected = false;

// Protótipo de funções
void vTaskWiFiReconnect(void *pvParameters);

int main()
{
    stdio_init_all();

    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    // Enable wifi station
    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return 1;
    } else {
        printf("Connected.\n");
        // Read the ip address in a human readable way
        uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
        printf("IP address %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    }

    // Criação das tarefas FreeRTOS
    xTaskCreate(vTaskWiFiReconnect, "WiFiReconnect", 512, NULL, 1, NULL);

    // ----- Iniciar scheduler -----
    vTaskStartScheduler();

    while (true) {
        tight_loop_contents();
    }
}


// ---- Task: reconexão Wi-Fi ----
void vTaskWiFiReconnect(void *pvParameters) {
    while (1) {
        struct netif *netif = &cyw43_state.netif[0];
        if (ip4_addr_isany_val(netif->ip_addr)) {
            wifi_connected = false;
            printf("Wi-Fi desconectado. Tentando reconectar...\n");

            if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                                   CYW43_AUTH_WPA2_AES_PSK, 10000)) {
                printf("Falha na reconexão.\n");
            } else {
                wifi_connected = true;
                ip4_addr_t ip = netif->ip_addr;
                printf("Reconectado! IP: %s\n", ip4addr_ntoa(&ip));
            }
        } else {
            wifi_connected = true;
        }

        vTaskDelay(pdMS_TO_TICKS(10000)); // Verifica a cada 10 segundos
    }
}