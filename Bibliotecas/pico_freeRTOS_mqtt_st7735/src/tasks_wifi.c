#include "tasks_wifi.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "include/credentials.h"

void task_wifi_monitor(void *pvParameters) {
    (void) pvParameters;
    if (cyw43_arch_init()) {
        printf("cyw43 init failed\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Enable station mode
    cyw43_arch_enable_sta_mode();

    while (1) {
        if (!cyw43_arch_wifi_connected()) {
            printf("Attempting to connect to Wi-Fi %s\n", WIFI_SSID);
            int ret = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000);
            if (ret == 0) {
                printf("Connected to Wi-Fi\n");
                // break; keep monitoring in loop
            } else {
                printf("Wi-Fi connect failed: %d\n", ret);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
