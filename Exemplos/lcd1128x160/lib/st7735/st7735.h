#ifndef _ST7735_H_
#define _ST7735_H_

#include "pico/stdlib.h"
#include "hardware/spi.h"

// -------------------- Configuração de pinos --------------------
#define ST7735_SPI_PORT spi0
#define ST7735_PIN_SCK  18
#define ST7735_PIN_MOSI 19
#define ST7735_PIN_CS   17
#define ST7735_PIN_DC   20
#define ST7735_PIN_RST  9

#define ST7735_WIDTH  128
#define ST7735_HEIGHT 160

// -------------------- Cores RGB565 --------------------
#define ST7735_BLACK   0x0000
#define ST7735_BLUE    0x001F
#define ST7735_RED     0xF800
#define ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0
#define ST7735_WHITE   0xFFFF
#define ST7735_GRAY    0x8410
#define ST7735_ORANGE  0xFD20

// -------------------- Funções públicas --------------------
void st7735_init(void);
void st7735_fill_screen(uint16_t color);
void st7735_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

// Gráficos
void st7735_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void st7735_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void st7735_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void st7735_draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void st7735_fill_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);

// Texto
void st7735_draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg);
void st7735_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg);

#endif
