#include "ui_energy_task.h"

/* UI */
#include "ui_energy.h"
#include "ui_environment.h"

/* Módulos */
#include "env_sensors.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "time_manager.h"
#include "mqtt_aggregator.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Hardware */
#include "hardware/gpio.h"

#include <stdbool.h>
#include <stdio.h>

#define UI_BUTTON_GPIO 5

/* =========================
 * Controle de telas
 * ========================= */
typedef enum {
    UI_SCREEN_ENERGY = 0,
    UI_SCREEN_ENV
} ui_screen_t;

static ui_screen_t current_screen = UI_SCREEN_ENERGY;

/* =========================
 * Queues externas (criadas no main)
 * ========================= */
extern QueueHandle_t xQueuePZEM_Display;
extern QueueHandle_t xEnvSensorQueue;
extern QueueHandle_t timeQueue;

/* =========================
 * Inicialização do botão
 * ========================= */
static void ui_button_init(void)
{
    gpio_init(UI_BUTTON_GPIO);
    gpio_set_dir(UI_BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(UI_BUTTON_GPIO);   /* botão ligado ao GND */
}

/* =========================
 * Leitura do botão (toggle tela)
 * ========================= */
static void ui_handle_button(void)
{
    static bool last_state = true;
    bool current_state = gpio_get(UI_BUTTON_GPIO);

    /* Detecta borda de descida */
    if (last_state && !current_state)
    {
        current_screen =
            (current_screen == UI_SCREEN_ENERGY)
            ? UI_SCREEN_ENV
            : UI_SCREEN_ENERGY;

        vTaskDelay(pdMS_TO_TICKS(300)); /* debounce */
    }

    last_state = current_state;
}

/* =========================
 * Task principal do display
 * ========================= */
void vTaskDisplay(void *pv)
{
    (void) pv;

    pzem_data_t       pzem_data;
    env_sensor_data_t env_data = {0};

    sys_datetime_t now;

    int hour   = -1;
    int minute = -1;

    ui_screen_t last_screen = UI_SCREEN_ENERGY;

    char ip_str[16] = "---.---.---.---";

    /* Inicializações */
    ui_button_init();

    UI_Energy_ShowSplash();
    UI_Energy_Init();

    for (;;)
    {
        /* ================= BOTÃO ================= */
        ui_handle_button();

        /* ================= TROCA DE TELA ================= */
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

        /* ================= TELA DE ENERGIA ================= */
        if (current_screen == UI_SCREEN_ENERGY)
        {
            if (xQueueReceive(xQueuePZEM_Display, &pzem_data, 0) == pdPASS)
            {
                UI_Energy_Update(&pzem_data);
            }
        }

        /* ================= TELA AMBIENTAL ================= */
        else if (current_screen == UI_SCREEN_ENV)
        {
            /* Dados ambientais (não bloqueante) */
            //xQueueReceive(xEnvSensorQueue, &env_data, 0);

            const env_sensor_data_t *env_ptr = env_get_last();

            if (env_ptr)
            {
                env_data = *env_ptr;
            }


            /* Hora atual (não bloqueante) */
            if (xQueueReceive(timeQueue, &now, 0) == pdPASS)
            {
                hour   = now.hour;
                minute = now.min;   /* ATENÇÃO: é 'min' */
            }

            /* IP atual */
            wifi_get_ip(ip_str, sizeof(ip_str));

            /* Atualiza UI ambiental */
            UI_Env_Update(&env_data,
                          wifi_is_connected(),
                          ip_str,
                          hour,
                          minute);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
