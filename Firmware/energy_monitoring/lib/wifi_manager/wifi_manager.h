#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include <stddef.h>

/* Estado global do Wi-Fi */
extern volatile bool g_wifi_connected;

/* Inicializa recursos internos do Wi-Fi (mutex, etc.) */
void wifi_manager_init(void);

/* Task do Wi-Fi (criada no main) */
void vTaskWiFiManager(void *pv);

/* Retorna true se Wi-Fi estiver conectado */
bool wifi_is_connected(void);

/* Copia o IP atual para o buffer (retorna true se válido) */
bool wifi_get_ip(char *ip_str, size_t len);

#endif
