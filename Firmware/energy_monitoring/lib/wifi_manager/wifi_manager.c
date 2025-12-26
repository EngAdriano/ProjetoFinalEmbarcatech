#include <stdio.h>

#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "lwip/netif.h"
#include "lwip/ip4_addr.h"

#include "wifi_manager.h"

/* ===============================
   Configurações Wi-Fi
   =============================== */
#define WIFI_SSID      "Lu e Deza"
#define WIFI_PASSWORD  "liukin1208"

/* ===============================
   Estado global
   =============================== */
volatile bool g_wifi_connected = false;

/* ===============================
   Recursos internos
   =============================== */
static SemaphoreHandle_t xWiFiMutex;

/* ===============================
   Inicialização do Wi-Fi Manager
   =============================== */
void wifi_manager_init(void)
{
    xWiFiMutex = xSemaphoreCreateMutex();

    if (xWiFiMutex == NULL)
    {
        printf("[WiFi] Erro ao criar mutex\n");
    }
}

/* ===============================
   Task de gerenciamento Wi-Fi
   =============================== */
void vTaskWiFiManager(void *pv)
{
    struct netif *netif = &cyw43_state.netif[0];

    while (1)
    {
        if (xSemaphoreTake(xWiFiMutex, portMAX_DELAY))
        {
            if (!g_wifi_connected || netif->ip_addr.addr == 0)
            {
                //printf("[WiFi] Conectando em '%s'...\n", WIFI_SSID);

                int r = cyw43_arch_wifi_connect_timeout_ms(
                    WIFI_SSID,
                    WIFI_PASSWORD,
                    CYW43_AUTH_WPA2_AES_PSK,
                    15000
                );

                if (r == 0)
                {
                    g_wifi_connected = true;
                    printf("[WiFi] Conectado! IP: %s\n",
                           ip4addr_ntoa(&netif->ip_addr));
                }
                else
                {
                    g_wifi_connected = false;
                    printf("[WiFi] Falha ao conectar.\n");
                }
            }

            xSemaphoreGive(xWiFiMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/* =========================
 * Status Wi-Fi
 * ========================= */
bool wifi_is_connected(void)
{
    return (cyw43_tcpip_link_status(&cyw43_state,
            CYW43_ITF_STA) == CYW43_LINK_UP);
}

bool wifi_get_ip(char *ip_str, size_t len)
{
    if (!wifi_is_connected())
        return false;

    struct netif *netif = netif_default;
    if (!netif)
        return false;

    ip4addr_ntoa_r(netif_ip4_addr(netif), ip_str, len);
    return true;
}
