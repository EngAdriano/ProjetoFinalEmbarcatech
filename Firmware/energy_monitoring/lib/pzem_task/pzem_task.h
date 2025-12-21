#ifndef PZEM_TASK_H
#define PZEM_TASK_H

#include "FreeRTOS.h"
#include "task.h"

/* Task de leitura do PZEM (criada no main) */
void vTaskPZEMReader(void *pv);

#endif
