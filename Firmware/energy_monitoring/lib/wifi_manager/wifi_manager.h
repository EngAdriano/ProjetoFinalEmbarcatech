#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>

/* Estado global do Wi-Fi */
extern volatile bool g_wifi_connected;

/* Inicializa recursos internos do Wi-Fi (mutex, etc.) */
void wifi_manager_init(void);

/* Task do Wi-Fi (criada no main) */
void vTaskWiFiManager(void *pv);

#endif
