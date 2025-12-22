/**
 * @file ui_environment.c
 * @brief Interface gráfica da tela de dados ambientais
 *
 * Projeto: Sistema IoT de Monitoramento de Energia
 * Plataforma: Raspberry Pi Pico W + Display TFT ST7735 (1.8")
 * RTOS: FreeRTOS
 *
 * Autor: Eng. Jose Adriano
 */

#include "ui_environment.h"

/* Dependências internas */
#include "st7735.h"
#include "fonts.h"

/* =========================================================
   Header padrão (mesmo estilo da tela de energia)
   ========================================================= */
static void ui_env_draw_header(void)
{
    ST7735_FillRectangle(0, 0, 160, 16, ST7735_BLUE);

    ST7735_DrawString(28, 3,
                      "DADOS AMBIENTAIS",
                      Font_7x10,
                      ST7735_WHITE,
                      ST7735_BLUE);

    ST7735_DrawLine(0, 16, 159, 16, ST7735_WHITE);
}

/* =========================================================
   Layout fixo da tela ambiental
   ========================================================= */
static void ui_env_draw_frame(void)
{
    ST7735_FillScreen(ST7735_BLACK);
    ui_env_draw_header();

    /* Placeholder visual (futuro conteúdo) */
    ST7735_DrawRoundRect(10, 40, 140, 50, 6, ST7735_WHITE);

    ST7735_DrawString(17, 58,
                      "EM DESENVOLVIMENTO",
                      Font_7x10,
                      ST7735_CYAN,
                      ST7735_BLACK);
}

/* =========================================================
   Inicialização pública da tela ambiental
   ========================================================= */
void UI_Env_Init(void)
{
    ui_env_draw_frame();
}
