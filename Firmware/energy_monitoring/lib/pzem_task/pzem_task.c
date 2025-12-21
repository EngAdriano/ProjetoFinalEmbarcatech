#include <stdio.h>
#include "pzem_task.h"
#include "pzem004t.h"
#include "queue.h"

/* Filas criadas no main */
extern QueueHandle_t xQueuePZEM_Display;
extern QueueHandle_t xQueuePZEM_MQTT;

void vTaskPZEMReader(void *pv)
{
    (void) pv;

    pzem_data_t data;

    pzem_init();
    printf("[PZEM] Inicializado\n");

    for (;;)
    {
        if (pzem_read(&data))
        {
            if (xQueuePZEM_Display) {
                xQueueOverwrite(xQueuePZEM_Display, &data);
            }

            if (xQueuePZEM_MQTT) {
                xQueueOverwrite(xQueuePZEM_MQTT, &data);
            }
        }
        else
        {
            printf("[PZEM] Erro de leitura\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
