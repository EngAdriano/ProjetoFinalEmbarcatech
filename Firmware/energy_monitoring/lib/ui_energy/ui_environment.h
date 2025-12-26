#ifndef UI_ENVIRONMENT_H
#define UI_ENVIRONMENT_H

#include <stdbool.h>
#include "env_sensors.h"

/* Inicializa e desenha o layout fixo da tela ambiental */
void UI_Env_Init(void);

/* Atualiza os dados dinâmicos da tela ambiental */
void UI_Env_Update(const env_sensor_data_t *env,
                   bool wifi_ok,
                   const char *ip_str,
                   int hour,
                   int minute);

#endif /* UI_ENVIRONMENT_H */
