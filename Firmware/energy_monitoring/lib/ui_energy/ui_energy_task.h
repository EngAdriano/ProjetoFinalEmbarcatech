#ifndef UI_ENERGY_TASK_H
#define UI_ENERGY_TASK_H

#include "FreeRTOS.h"
#include "task.h"

/* Task de atualização do display (criada no main) */
void vTaskDisplay(void *pv);

#endif
