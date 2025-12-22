#include "ui_energy_task.h"
#include "ui_energy.h"
#include "queue.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include <stdbool.h>
#include "ui_environment.h"

#define UI_BUTTON_GPIO 5

typedef enum {
    UI_SCREEN_ENERGY = 0,
    UI_SCREEN_ENV
} ui_screen_t;

static ui_screen_t current_screen = UI_SCREEN_ENERGY;


/* Fila criada no main */
extern QueueHandle_t xQueuePZEM_Display;

static void ui_button_init(void)
{
    gpio_init(UI_BUTTON_GPIO);
    gpio_set_dir(UI_BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(UI_BUTTON_GPIO);   // botão ligado ao GND
}

static void ui_handle_button(void)
{
    static bool last_state = true;
    bool current_state = gpio_get(UI_BUTTON_GPIO);

    // Detecta borda de descida (pressionado)
    if (last_state && !current_state)
    {
        current_screen =
            (current_screen == UI_SCREEN_ENERGY)
            ? UI_SCREEN_ENV
            : UI_SCREEN_ENERGY;

        vTaskDelay(pdMS_TO_TICKS(300)); // debounce
    }

    last_state = current_state;
}


void vTaskDisplay(void *pv)
{
    (void) pv;

    pzem_data_t data;
    ui_screen_t last_screen = UI_SCREEN_ENERGY;

    ui_button_init();

    UI_Energy_ShowSplash();
    UI_Energy_Init();

    //printf("[UI] Display inicializado\n");

    for (;;)
    {
        /* ===== BOTÃO ===== */
        ui_handle_button();

        /* ===== TROCA DE TELA ===== */
        if (current_screen != last_screen)
        {
            if (current_screen == UI_SCREEN_ENERGY)
            {
                UI_Energy_Init();
            }
            else
            {
                UI_Env_Init();
            }

            last_screen = current_screen;
        }

        /* ===== ATUALIZAÇÃO DA TELA ATIVA ===== */
        if (current_screen == UI_SCREEN_ENERGY)
        {
            if (xQueueReceive(xQueuePZEM_Display, &data, 0))
            {
                UI_Energy_Update(&data);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
