#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"
#include "hardware/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Laboratório credenciais Wi-Fi
//#define WIFI_SSID     "ITSelf"
//#define WIFI_PASSWORD "code2020"

// Casa credenciais Wi-Fi
#define WIFI_SSID     "Lu e Deza"
#define WIFI_PASSWORD "liukin1208"


// SPI definições
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

// I2C definições
#define I2C_PORT i2c1
#define I2C_SDA 2
#define I2C_SCL 3


// UART definições
#define UART_ID uart0
#define BAUD_RATE 4800

// Usar pinos 0 e 1 para UART0
// Os pinos podem ser alterados, mas devem ser pinos UART válidos
#define UART_TX_PIN 0
#define UART_RX_PIN 1

volatile bool wifi_connected = false;

// Prototipos de funções
void vTaskWiFiReconnect(void *pvParameters);


int main()
{
    stdio_init_all();

    // Inicializa o chip Wi-Fi
    if (cyw43_arch_init()) {
        printf("Falha na inicialização do Wi-Fi\n");
        return -1;
    }

    // Inicialização do SPI. Este exemplo usará SPI a 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    // I2C Inicialização. Usando-a a 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Habilitar modo estação Wi-Fi
    cyw43_arch_enable_sta_mode();

    printf("Conectando ao Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Falha na conexão.\n");
        return 1;
    } else {
        printf("Conectado.\n");
        // Read the ip address in a human readable way
        uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
        printf("IP address %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    }

    // Configurar nosso UART
    uart_init(UART_ID, BAUD_RATE);
    // Definir os pinos TX e RX usando a seleção de função no GPIO
    // Consulte o datasheet para mais informações sobre a seleção de função
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Usar algumas das várias funções UART para enviar dados
    // Em um sistema padrão, printf também será enviado via UART padrão

    // Enviar uma string, com conversões CR/LF
    uart_puts(UART_ID, " Hello, UART!\n");

    // ------- Criar Tasks -------
    xTaskCreate(vTaskWiFiReconnect, "WiFiReconnect", 512, NULL, 1, NULL);

    // ----- Iniciar scheduler -----
    vTaskStartScheduler();

    while (true) {
        // Nada a fazer aqui, tudo é gerenciado pelas tasks 
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
