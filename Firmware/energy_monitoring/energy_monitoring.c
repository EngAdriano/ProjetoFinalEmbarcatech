#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Erro ao iniciar CYW43\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    wifi_init_manager();
    mqtt_start();

    vTaskStartScheduler();

    while (1) {}
}
