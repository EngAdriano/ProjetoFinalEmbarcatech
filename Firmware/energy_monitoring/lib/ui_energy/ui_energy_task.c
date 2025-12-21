#include "ui_energy_task.h"
#include "ui_energy.h"
#include "queue.h"
#include <stdio.h>

/* Fila criada no main */
extern QueueHandle_t xQueuePZEM_Display;

void vTaskDisplay(void *pv)
{
    (void) pv;

    pzem_data_t data;

    UI_Energy_ShowSplash();
    UI_Energy_Init();

    printf("[UI] Display inicializado\n");

    for (;;)
    {
        if (xQueueReceive(xQueuePZEM_Display, &data, pdMS_TO_TICKS(300)))
        {
            UI_Energy_Update(&data);
        }

        vTaskDelay(pdMS_TO_TICKS(250));
    }
}
