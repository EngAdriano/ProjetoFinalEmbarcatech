/**
 * @file ui_energy.c
 * @brief Interface gráfica do sistema de monitoramento de energia
 *
 * Projeto: Sistema IoT de Monitoramento de Energia
 * Plataforma: Raspberry Pi Pico W + Display TFT ST7735 (1.8")
 * RTOS: FreeRTOS
 *
 * Autor: Eng. Jose Adriano
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

/* Interface pública */
#include "ui_energy.h"

/* Dependências internas (encapsuladas) */
#include "st7735.h"
#include "fonts.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

/* =========================================================
   Constantes de layout (ESTÁVEIS)
   ========================================================= */

/* Largura segura para qualquer texto (7x10) */
#define VALUE_W        56
#define VALUE_H        12

/* Margem interna esquerda do card */
#define VALUE_PADDING   6

/* =========================================================
   Controle de inicialização única do display
   ========================================================= */
static bool display_initialized = false;

static void ui_display_init_once(void)
{
    if (!display_initialized)
    {
        ST7735_Init();
        ST7735_SetRotation(1);   /* Paisagem: 160x128 */
        display_initialized = true;
    }
}

/* =========================================================
   Header com fundo invertido
   ========================================================= */
static void ui_draw_header(void)
{
    ST7735_FillRectangle(0, 0, 160, 16, ST7735_BLUE);

    ST7735_DrawString(36, 3, "ENERGY MONITOR",
                      Font_7x10, ST7735_WHITE, ST7735_BLUE);

    ST7735_DrawLine(0, 16, 159, 16, ST7735_WHITE);
}

/* =========================================================
   Função auxiliar: desenha um card
   ========================================================= */
static void ui_draw_card(uint16_t x, uint16_t y, const char *title)
{
    ST7735_DrawRoundRect(x, y, 72, 30, 4, ST7735_WHITE);

    ST7735_DrawString(x + 4, y + 3, title,
                      Font_7x10, ST7735_CYAN, ST7735_BLACK);
}

/* =========================================================
   Função auxiliar: limpa área fixa do valor
   ========================================================= */
static void ui_clear_value(uint16_t x, uint16_t y)
{
    ST7735_FillRectangle(x + VALUE_PADDING,
                         y,
                         VALUE_W,
                         VALUE_H,
                         ST7735_BLACK);
}

/* =========================================================
   Desenho do layout principal
   ========================================================= */
static void ui_draw_main_frame(void)
{
    ST7735_FillScreen(ST7735_BLACK);

    ui_draw_header();

    ui_draw_card(4,  22, "Voltage");
    ui_draw_card(82, 22, "Current");

    ui_draw_card(4,  56, "Power");
    ui_draw_card(82, 56, "Energy");

    ui_draw_card(4,  90, "Freq");
    ui_draw_card(82, 90, "PF");
}

/* =========================================================
   Tela de abertura (Splash Screen)
   ========================================================= */
void UI_Energy_ShowSplash(void)
{
    ui_display_init_once();

    ST7735_FillScreen(ST7735_BLACK);

    /* Cabeçalho */
    ST7735_FillRectangle(0, 0, 160, 22, ST7735_GREEN);
    ST7735_DrawString(22, 6, "EMBARCATECH - IFCE",
                      Font_7x10, ST7735_BLACK, ST7735_GREEN);

    /* Caixa central */
    ST7735_DrawRoundRect(10, 40, 140, 50, 6, ST7735_WHITE);

    ST7735_DrawString(18, 50,
                      "Sistema IoT de",
                      Font_7x10, ST7735_CYAN, ST7735_BLACK);

    ST7735_DrawString(18, 65,
                      "Monitoramento de",
                      Font_7x10, ST7735_CYAN, ST7735_BLACK);

    ST7735_DrawString(18, 80,
                      "Energia",
                      Font_7x10, ST7735_CYAN, ST7735_BLACK);

    ST7735_DrawString(20, 110,
                      "Eng. Jose Adriano",
                      Font_7x10, ST7735_YELLOW, ST7735_BLACK);

    /* Tempo da splash screen */
    vTaskDelay(pdMS_TO_TICKS(2500));
}

/* =========================================================
   Inicialização da interface principal
   ========================================================= */
void UI_Energy_Init(void)
{
    ui_display_init_once();
    ui_draw_main_frame();
}

/* =========================================================
   Atualização dos valores do PZEM-004T (VERSÃO FINAL ESTÁVEL)
   ========================================================= */
void UI_Energy_Update(const pzem_data_t *d)
{
    char buf[20];

    /* Voltage */
    ui_clear_value(4, 38);
    snprintf(buf, sizeof(buf), "%.1f V", d->voltage);
    ST7735_DrawString(4 + VALUE_PADDING, 38, buf,
                      Font_7x10, ST7735_GREEN, ST7735_BLACK);

    /* Current */
    ui_clear_value(82, 38);
    snprintf(buf, sizeof(buf), "%.3f A", d->current);
    ST7735_DrawString(82 + VALUE_PADDING, 38, buf,
                      Font_7x10, ST7735_GREEN, ST7735_BLACK);

    /* Power */
    ui_clear_value(4, 72);
    snprintf(buf, sizeof(buf), "%.1f W", d->power);
    ST7735_DrawString(4 + VALUE_PADDING, 72, buf,
                      Font_7x10, ST7735_GREEN, ST7735_BLACK);

    /* Energy */
    ui_clear_value(82, 72);
    snprintf(buf, sizeof(buf), "%.3f kWh", d->energy);
    ST7735_DrawString(82 + VALUE_PADDING, 72, buf,
                      Font_7x10, ST7735_YELLOW, ST7735_BLACK);

    /* Frequency */
    ui_clear_value(4, 106);
    snprintf(buf, sizeof(buf), "%.1f Hz", d->frequency);
    ST7735_DrawString(4 + VALUE_PADDING, 106, buf,
                      Font_7x10, ST7735_GREEN, ST7735_BLACK);

    /* Power Factor */
    ui_clear_value(82, 106);
    snprintf(buf, sizeof(buf), "%.2f", d->pf);
    ST7735_DrawString(82 + VALUE_PADDING, 106, buf,
                      Font_7x10, ST7735_GREEN, ST7735_BLACK);
}
