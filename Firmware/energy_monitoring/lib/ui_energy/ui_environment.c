#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "ui_environment.h"
#include "st7735.h"
#include "fonts.h"
#include "icons_12x12.h"

/* ================= Layout (igual à tela de energia) ================= */
#define VALUE_W        56
#define VALUE_H        12
#define VALUE_PADDING  6

#define WIFI_ICON_X    148
#define WIFI_ICON_Y    2

/* ================= Estado interno (cache visual) ================= */
static bool last_wifi_state = false;
static char last_ip_str[16] = {0};
static int  last_hour = -1;
static int  last_min  = -1;

/* ================= Protótipos internos ================= */
static void UI_DrawIcon12x12(int x, int y,
                             const uint16_t *icon,
                             uint16_t color);

static void ui_draw_header(void);
static void ui_draw_card(uint16_t x, uint16_t y, const char *title);
static void ui_clear_value(uint16_t x, uint16_t y);
static void ui_update_wifi_icon(bool wifi_ok);
static void ui_update_ip(const char *ip);
static void ui_update_time(int hour, int min);

/* ================= Header fixo ================= */
static void ui_draw_header(void)
{
    ST7735_FillRectangle(0, 0, 160, 16, ST7735_BLUE);

    ST7735_DrawString(28, 3, "DADOS AMBIENTAIS",
                      Font_7x10, ST7735_WHITE, ST7735_BLUE);

    ST7735_DrawLine(0, 16, 159, 16, ST7735_WHITE);
}

/* ================= Card padrão ================= */
static void ui_draw_card(uint16_t x, uint16_t y, const char *title)
{
    ST7735_DrawRoundRect(x, y, 72, 30, 4, ST7735_WHITE);
    ST7735_DrawString(x + 4, y + 3, title,
                      Font_7x10, ST7735_CYAN, ST7735_BLACK);
}

/* ================= Limpa apenas valor ================= */
static void ui_clear_value(uint16_t x, uint16_t y)
{
    ST7735_FillRectangle(x + VALUE_PADDING, y,
                         VALUE_W, VALUE_H, ST7735_BLACK);
}

/* ================= Layout fixo ================= */
static void ui_draw_main_frame(void)
{
    ST7735_FillScreen(ST7735_BLACK);
    ui_draw_header();

    ui_draw_card(4,  22, "Temp");
    ui_draw_card(82, 22, "Umid");

    ui_draw_card(4,  56, "Lux");
    ui_draw_card(82, 56, "Hora");

    ST7735_DrawRoundRect(4, 90, 152, 30, 4, ST7735_WHITE);
    ST7735_DrawString(10, 93, "IP",
                      Font_7x10, ST7735_CYAN, ST7735_BLACK);
}

/* ================= Inicialização ================= */
void UI_Env_Init(void)
{
    last_wifi_state = false;
    last_ip_str[0]  = '\0';
    last_hour = last_min = -1;

    ui_draw_main_frame();
    ui_update_wifi_icon(false);
}

/* ================= Atualização principal ================= */
void UI_Env_Update(const env_sensor_data_t *env,
                   bool wifi_ok,
                   const char *ip_str,
                   int hour,
                   int minute)
{
    char buf[20];

    ui_update_wifi_icon(wifi_ok);
    ui_update_ip(ip_str);
    ui_update_time(hour, minute);

    ui_clear_value(4, 38);
    snprintf(buf, sizeof(buf), "%.1f C", env->temperature);
    ST7735_DrawString(4 + VALUE_PADDING, 38, buf,
                      Font_7x10, ST7735_GREEN, ST7735_BLACK);

    ui_clear_value(82, 38);
    snprintf(buf, sizeof(buf), "%.1f %%", env->humidity);
    ST7735_DrawString(82 + VALUE_PADDING, 38, buf,
                      Font_7x10, ST7735_GREEN, ST7735_BLACK);

    ui_clear_value(4, 72);
    snprintf(buf, sizeof(buf), "%.0f lx", env->lux);
    ST7735_DrawString(4 + VALUE_PADDING, 72, buf,
                      Font_7x10, ST7735_GREEN, ST7735_BLACK);
}

/* ================= Atualizações condicionais ================= */
static void ui_update_wifi_icon(bool wifi_ok)
{
    if (wifi_ok == last_wifi_state) return;

    ST7735_FillRectangle(WIFI_ICON_X, WIFI_ICON_Y, 12, 12, ST7735_BLUE);
    UI_DrawIcon12x12(WIFI_ICON_X, WIFI_ICON_Y,
                     icon_wifi_12x12,
                     wifi_ok ? ST7735_GREEN : ST7735_RED);

    last_wifi_state = wifi_ok;
}

static void ui_update_ip(const char *ip)
{
    if (!ip || strcmp(ip, last_ip_str) == 0) return;

    strncpy(last_ip_str, ip, sizeof(last_ip_str) - 1);
    ST7735_FillRectangle(10, 106, 136, 12, ST7735_BLACK);
    ST7735_DrawString(10, 106, last_ip_str,
                      Font_7x10, ST7735_YELLOW, ST7735_BLACK);
}

static void ui_update_time(int hour, int min)
{
    if (hour == last_hour && min == last_min) return;

    last_hour = hour;
    last_min  = min;

    char buf[8];
    snprintf(buf, sizeof(buf), "%02d:%02d", hour, min);

    ui_clear_value(82, 72);
    ST7735_DrawString(82 + VALUE_PADDING, 72, buf,
                      Font_7x10, ST7735_GREEN, ST7735_BLACK);
}

/* ================= Ícone 12x12 ================= */
static void UI_DrawIcon12x12(int x, int y,
                             const uint16_t *icon,
                             uint16_t color)
{
    for (int r = 0; r < 12; r++)
        for (int c = 0; c < 12; c++)
            if (icon[r] & (1 << (11 - c)))
                ST7735_DrawPixel(x + c, y + r, color);
}
