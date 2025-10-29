#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "st7735.h"



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
    if (cyw43_arch_wifi_connect_timeout_ms("Lu e Deza", "liukin1208", CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return 1;
    } else {
        printf("Connected.\n");
        // Read the ip address in a human readable way
        uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
        printf("IP address %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    }


    st7735_init();

    st7735_fill_screen(ST7735_BLACK);

    st7735_draw_string(4, 4, "ST7735 128x160", ST7735_YELLOW, ST7735_BLACK);
    st7735_draw_string(4, 16, "Pico W + SPI", ST7735_CYAN, ST7735_BLACK);

    // Desenhos básicos
    st7735_draw_rect(2, 30, 124, 40, ST7735_GREEN);
    st7735_fill_rect(10, 36, 30, 24, ST7735_ORANGE);
    st7735_draw_line(10, 70, 118, 120, ST7735_RED);
    st7735_draw_circle(64, 100, 20, ST7735_WHITE);
    st7735_fill_circle(100, 130, 8, ST7735_BLUE);

    // Texto demo
    st7735_draw_string(4, 140, "Hello, Mundo!", ST7735_MAGENTA, ST7735_BLACK);


    while (true) {
        printf("Running...\n");
        sleep_ms(1000);
    }
}
