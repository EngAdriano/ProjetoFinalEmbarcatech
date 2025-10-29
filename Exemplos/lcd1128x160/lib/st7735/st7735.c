
#include "stdlib.h"
#include "stdbool.h"
#include "st7735.h"
#include "hardware/gpio.h"
#include "font5x7.h"

// -------------------- Comandos ST7735 --------------------
#define ST77XX_SWRESET 0x01
#define ST77XX_SLPOUT  0x11
#define ST77XX_DISPON  0x29
#define ST77XX_CASET   0x2A
#define ST77XX_RASET   0x2B
#define ST77XX_RAMWR   0x2C
#define ST77XX_MADCTL  0x36
#define ST77XX_COLMOD  0x3A

// -------------------- Funções internas --------------------
static inline void cs_select()   { gpio_put(ST7735_PIN_CS, 0); }
static inline void cs_deselect() { gpio_put(ST7735_PIN_CS, 1); }
static inline void dc_command()  { gpio_put(ST7735_PIN_DC, 0); }
static inline void dc_data()     { gpio_put(ST7735_PIN_DC, 1); }

static void write_cmd(uint8_t cmd) {
    dc_command();
    cs_select();
    spi_write_blocking(ST7735_SPI_PORT, &cmd, 1);
    cs_deselect();
}

static void write_data(const uint8_t *data, size_t len) {
    dc_data();
    cs_select();
    spi_write_blocking(ST7735_SPI_PORT, data, len);
    cs_deselect();
}

// -------------------- Inicialização --------------------
void st7735_init(void) {
    // GPIOs
    gpio_init(ST7735_PIN_CS);  gpio_set_dir(ST7735_PIN_CS, GPIO_OUT);
    gpio_init(ST7735_PIN_DC);  gpio_set_dir(ST7735_PIN_DC, GPIO_OUT);
    gpio_init(ST7735_PIN_RST); gpio_set_dir(ST7735_PIN_RST, GPIO_OUT);

    // SPI
    spi_init(ST7735_SPI_PORT, 8000 * 1000);
    gpio_set_function(ST7735_PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(ST7735_PIN_MOSI, GPIO_FUNC_SPI);

    // Reset
    gpio_put(ST7735_PIN_RST, 0);
    sleep_ms(50);
    gpio_put(ST7735_PIN_RST, 1);
    sleep_ms(120);

    // Inicialização básica
    write_cmd(ST77XX_SWRESET); sleep_ms(150);
    write_cmd(ST77XX_SLPOUT);  sleep_ms(150);

    uint8_t data = 0x05; // 16 bits por pixel
    write_cmd(ST77XX_COLMOD);
    write_data(&data, 1);

    data = 0xC0; // rotação e ordem RGB
    write_cmd(ST77XX_MADCTL);
    write_data(&data, 1);

    write_cmd(ST77XX_DISPON);
    sleep_ms(100);

    st7735_fill_screen(ST7735_BLACK);
}

// -------------------- Janela de escrita --------------------
static void set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t data[4];
    write_cmd(ST77XX_CASET);
    data[0] = 0; data[1] = x0;
    data[2] = 0; data[3] = x1;
    write_data(data, 4);

    write_cmd(ST77XX_RASET);
    data[0] = 0; data[1] = y0;
    data[2] = 0; data[3] = y1;
    write_data(data, 4);

    write_cmd(ST77XX_RAMWR);
}

// -------------------- Desenho --------------------
void st7735_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= ST7735_WIDTH || y >= ST7735_HEIGHT) return;
    set_addr_window(x, y, x, y);
    uint8_t data[2] = { color >> 8, color & 0xFF };
    write_data(data, 2);
}

void st7735_fill_screen(uint16_t color) {
    st7735_fill_rect(0, 0, ST7735_WIDTH, ST7735_HEIGHT, color);
}

void st7735_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if ((x >= ST7735_WIDTH) || (y >= ST7735_HEIGHT)) return;
    if ((x + w - 1) >= ST7735_WIDTH) w = ST7735_WIDTH - x;
    if ((y + h - 1) >= ST7735_HEIGHT) h = ST7735_HEIGHT - y;

    set_addr_window(x, y, x + w - 1, y + h - 1);
    uint8_t data[2] = { color >> 8, color & 0xFF };

    dc_data();
    cs_select();
    for (uint32_t i = 0; i < (w * h); i++)
        spi_write_blocking(ST7735_SPI_PORT, data, 2);
    cs_deselect();
}

void st7735_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    st7735_draw_line(x, y, x + w - 1, y, color);
    st7735_draw_line(x, y, x, y + h - 1, color);
    st7735_draw_line(x + w - 1, y, x + w - 1, y + h - 1, color);
    st7735_draw_line(x, y + h - 1, x + w - 1, y + h - 1, color);
}

// Algoritmo de Bresenham
void st7735_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        st7735_draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

// Círculo
void st7735_draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
    int f = 1 - r;
    int ddF_x = 1;
    int ddF_y = -2 * r;
    int x = 0;
    int y = r;

    st7735_draw_pixel(x0, y0 + r, color);
    st7735_draw_pixel(x0, y0 - r, color);
    st7735_draw_pixel(x0 + r, y0, color);
    st7735_draw_pixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        st7735_draw_pixel(x0 + x, y0 + y, color);
        st7735_draw_pixel(x0 - x, y0 + y, color);
        st7735_draw_pixel(x0 + x, y0 - y, color);
        st7735_draw_pixel(x0 - x, y0 - y, color);
        st7735_draw_pixel(x0 + y, y0 + x, color);
        st7735_draw_pixel(x0 - y, y0 + x, color);
        st7735_draw_pixel(x0 + y, y0 - x, color);
        st7735_draw_pixel(x0 - y, y0 - x, color);
    }
}

void st7735_fill_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x * x + y * y <= r * r)
                st7735_draw_pixel(x0 + x, y0 + y, color);
        }
    }
}

// -------------------- Texto --------------------
void st7735_draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg) {
    if (c < 32 || c > 127) c = '?';
    for (int i = 0; i < 5; i++) {
        uint8_t line = font5x7[c - 32][i];
        for (int j = 0; j < 8; j++) {
            st7735_draw_pixel(x + i, y + j, (line & 1) ? color : bg);
            line >>= 1;
        }
    }
}

void st7735_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg) {
    while (*str) {
        st7735_draw_char(x, y, *str, color, bg);
        x += 6;
        str++;
    }
}
