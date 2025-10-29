#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "main.h"
#include "st7735.h"
#include "fonts.h"
#include "testimg.h"

// Casa
//#define WIFI_SSID   "Lu e Deza"
//#define WIFI_PASSWORD   "liukin1208"
// Laboratório
#define WIFI_SSID   "ITSelf"
#define WIFI_PASSWORD   "code2020"

// Protótipo das funções
uint16_t rgb888_convert_rgb565(uint8_t r, uint8_t g, uint8_t b);
void DemoTFT(void);

//
uint8_t rotat=0;

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

    // intialize the SPI0 of Raspberry Pi
        // This example will use SPI0 at 4MHz.
    spi_init(SPI_PORT, 4000 * 1000);
    //gpio_set_function(LCD_MISO, GPIO_FUNC_SPI);
    gpio_set_function(LCD_SCK, GPIO_FUNC_SPI);
    gpio_set_function(LCD_MOSI, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(LCD_RST);
    gpio_set_dir(LCD_RST, GPIO_OUT);
    gpio_init(LCD_CS);
    gpio_set_dir(LCD_CS, GPIO_OUT);
    gpio_init(LCD_DC); //RS PIn
    gpio_set_dir(LCD_DC, GPIO_OUT);

    // call the LCD initialization
    ST7735_Init();

    ST7735_SetRotation(1);
    ST7735_FillScreen(ST7735_BLACK);

	int r = 0;
	int g = 0;
	int b = 0;

    while (true) {
		uint16_t cor = 0;

        ST7735_DrawString(20, 0, "EMBARCATECH", Font_11x18, ST7735_BLUE, ST7735_BLACK);
        ST7735_DrawString(35, 2*10, "Projeto Final", Font_7x10, ST7735_GREEN, ST7735_BLACK);
		ST7735_DrawLine(0, 33, ST7735_GetWidth(), 33, ST7735_YELLOW);

			cor = rgb888_convert_rgb565(r, g, 200);
			printf("Valor em decimal: %d\n", cor);
			printf("Valor em decimal: %d\n", r);
		 	// Para letras maiúsculas (A-F), use `%04X`.
  			printf("Valor em hexadecimal (maiúsculo): %04X\n", cor);
			
        ST7735_DrawString(20, 5*10, "IP: 192.168.1.227", Font_7x10, cor, ST7735_BLACK);
        ST7735_DrawString(12, 8*10, "EM CONSTRUCAO", Font_11x18, ST7735_RED, ST7735_BLACK);
        sleep_ms(500);
        ST7735_DrawString(12, 8*10, "               ", Font_11x18, ST7735_RED, ST7735_BLACK);
         sleep_ms(500);

		 r = r + 10;

		 if(r >= 256)
		 {
			r = 0;
			g = g + 10;
			if(g >= 256)
			{
				g = 0;
			}
		 }
    }
}


uint16_t rgb888_convert_rgb565(uint8_t r, uint8_t g, uint8_t b) {
	uint16_t cor_16bit = 0;

	// Extrai os bits da cor vermelha e monta na variável de 16 bits
	cor_16bit |= ((r >> 3) << 11);
	// Extrai os bits da cor verde e monta na variável de 16 bits
	cor_16bit |= ((g >> 2) << 5);
	// Extrai os bits da cor azul e monta na variável de 16 bits
	cor_16bit |= (b >> 3);

	return cor_16bit;
}

void DemoTFT(void)
{
	ST7735_SetRotation(rotat);

	ST7735_FillScreen(ST7735_BLACK);

	for(int x = 0; x < ST7735_GetWidth(); x++)
	{
	  ST7735_DrawPixel(x, 0, ST7735_WHITE);
	  ST7735_DrawPixel(x, ST7735_GetHeight() - 1, ST7735_WHITE);
	}

	for(int y = 0; y < ST7735_GetHeight(); y++)
	{
	  ST7735_DrawPixel(0, y, ST7735_WHITE);
	  ST7735_DrawPixel(ST7735_GetWidth() - 1, y, ST7735_WHITE);
	}

	ST7735_DrawLine(0, 0, ST7735_GetWidth(), ST7735_GetHeight(), ST7735_WHITE);
	ST7735_DrawLine(ST7735_GetWidth(), 0, 0, ST7735_GetHeight(), ST7735_WHITE);

	sleep_ms(2000);

	ST7735_FillScreen(ST7735_BLACK);

	for (int i = 0; i < ST7735_GetHeight(); i += 4)
	{
		ST7735_DrawFastHLine(0, i, ST7735_GetWidth() - 1, ST7735_WHITE);
	}

	for (int i = 0; i < ST7735_GetWidth(); i += 4)
	{
		ST7735_DrawFastVLine(i, 0, ST7735_GetHeight() - 1, ST7735_WHITE);
	}

	sleep_ms(2000);

	// Check fonts
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_DrawString(0, 0, "Font_7x10, Embarcatech", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 3*10, "Font_11x18, Eng. Adriano", Font_11x18, ST7735_GREEN, ST7735_BLACK);
	ST7735_DrawString(0, 3*10+3*18, "Font_16x26", Font_16x26, ST7735_BLUE, ST7735_BLACK);
	sleep_ms(2000);

	// Check colors
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_DrawString(0, 0, "PRETO", Font_11x18, ST7735_WHITE, ST7735_BLACK);
	sleep_ms(500);

	ST7735_FillScreen(ST7735_BLUE);
	ST7735_DrawString(0, 0, "AZUL", Font_11x18, ST7735_BLACK, ST7735_BLUE);
	sleep_ms(500);

	ST7735_FillScreen(ST7735_RED);
	ST7735_DrawString(0, 0, "VERMELHO", Font_11x18, ST7735_BLACK, ST7735_RED);
	sleep_ms(500);

	ST7735_FillScreen(ST7735_GREEN);
	ST7735_DrawString(0, 0, "VERDE", Font_11x18, ST7735_BLACK, ST7735_GREEN);
	sleep_ms(500);

	ST7735_FillScreen(ST7735_CYAN);
	ST7735_DrawString(0, 0, "AZULADO CLARO", Font_11x18, ST7735_BLACK, ST7735_CYAN);
	sleep_ms(500);

	ST7735_FillScreen(ST7735_MAGENTA);
	ST7735_DrawString(0, 0, "ROSA", Font_11x18, ST7735_BLACK, ST7735_MAGENTA);
	sleep_ms(500);

	ST7735_FillScreen(ST7735_YELLOW);
	ST7735_DrawString(0, 0, "AMARELO", Font_11x18, ST7735_BLACK, ST7735_YELLOW);
	sleep_ms(500);

	ST7735_FillScreen(ST7735_WHITE);
	ST7735_DrawString(0, 0, "BRANCO", Font_11x18, ST7735_BLACK, ST7735_WHITE);
	sleep_ms(500);

	// Draw circles
	ST7735_FillScreen(ST7735_BLACK);
	for (int i = 0; i < ST7735_GetHeight() / 2; i += 2)
	{
		ST7735_DrawCircle(ST7735_GetWidth() / 2, ST7735_GetHeight() / 2, i, ST7735_YELLOW);
	}
	sleep_ms(1000);

	ST7735_FillScreen(ST7735_BLACK);
	ST7735_FillTriangle(0, 0, ST7735_GetWidth() / 2, ST7735_GetHeight(), ST7735_GetWidth(), 0, ST7735_RED);
	sleep_ms(1000);

	ST7735_FillScreen(ST7735_BLACK);
	ST7735_DrawImage(10, 30, 100, 100, (uint16_t*) test_img);
	sleep_ms(3000);

	rotat++;
}
