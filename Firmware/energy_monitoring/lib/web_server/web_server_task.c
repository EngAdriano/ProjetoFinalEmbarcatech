#include "web_server_task.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

/* Pico / Wi-Fi */
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

/* lwIP RAW TCP */
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"

/* Cache de dados */
#include "mqtt_aggregator.h"

/* =========================
 * Configuração
 * ========================= */
#define WEB_PORT 80

/* =========================
 * Página HTML principal
 * ========================= */
static const char html_index[] =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html\r\n\r\n"
"<!DOCTYPE html>"
"<html>"
"<head>"
"<meta charset='utf-8'>"
"<title>Embarcatech</title>"
"</head>"
"<body style='background:#111;color:#0f0;font-family:Arial;'>"
"<h1>Sistema IoT - Embarcatech</h1>"
"<p>Dados Ambientais:</p>"
"<pre id='env'>Carregando...</pre>"
"<script>"
"setInterval(()=>{"
" fetch('/env')"
"  .then(r=>r.text())"
"  .then(t=>document.getElementById('env').innerText=t);"
"},1000);"
"</script>"
"</body>"
"</html>";

/* =========================
 * Prototipos internos
 * ========================= */
static err_t http_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t http_recv(void *arg, struct tcp_pcb *tpcb,
                       struct pbuf *p, err_t err);

/* =========================
 * Handler de recepção HTTP
 * ========================= */
static err_t http_recv(void *arg, struct tcp_pcb *tpcb,
                       struct pbuf *p, err_t err)
{
    if (!p)
    {
        tcp_close(tpcb);
        return ERR_OK;
    }

    tcp_recved(tpcb, p->tot_len);

    /* Copia requisição */
    char *req = calloc(p->len + 1, 1);
    memcpy(req, p->payload, p->len);

    /* =========================
     * Endpoint: /env
     * ========================= */
    if (strstr(req, "GET /env"))
    {
        const env_sensor_data_t *env = env_get_last();
        char resp[256];

        snprintf(resp, sizeof(resp),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n\r\n"
            "Temperatura: %.1f C\n"
            "Umidade: %.1f %%\n"
            "Luminosidade: %.1f lx\n",
            env->temperature,
            env->humidity,
            env->lux
        );

        tcp_write(tpcb, resp, strlen(resp), TCP_WRITE_FLAG_COPY);
    }
    else
    {
        /* Página principal */
        tcp_write(tpcb,
                  html_index,
                  strlen(html_index),
                  TCP_WRITE_FLAG_COPY);
    }

    tcp_output(tpcb);
    tcp_close(tpcb);

    free(req);
    pbuf_free(p);

    return ERR_OK;
}

/* =========================
 * Handler de nova conexão
 * ========================= */
static err_t http_accept(void *arg,
                          struct tcp_pcb *newpcb,
                          err_t err)
{
    tcp_recv(newpcb, http_recv);
    return ERR_OK;
}

/* =========================
 * Task do servidor web
 * ========================= */
void vTaskWebServer(void *pv)
{
    (void) pv;

    /* Aguarda Wi-Fi estabilizar */
    vTaskDelay(pdMS_TO_TICKS(3000));

    struct tcp_pcb *pcb = tcp_new();
    if (!pcb)
    {
        printf("[WEB] ERRO: tcp_new() falhou\n");
        vTaskDelete(NULL);
    }

    tcp_bind(pcb, IP_ADDR_ANY, WEB_PORT);
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, http_accept);

    printf("[WEB] Servidor web ativo na porta %d\n", WEB_PORT);

    /* Task passiva */
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
