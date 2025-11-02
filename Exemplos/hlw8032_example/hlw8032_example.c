#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hlw8032.h"

#define WIFI_SSID "Lu e Deza"
#define WIFI_PASSWORD "liukin1208"

hlw8032_t hlw8032;


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

    hlw8032_init(&hlw8032, uart0, 1, 4800);

    while (true) {
        if(hlw8032_read_frame(&hlw8032)) {
            hlw8032_print_data(&hlw8032);
        } else {
            printf("Falha ao ler dados do HLW8032\n");
        }   
        sleep_ms(1000);
    }
}
