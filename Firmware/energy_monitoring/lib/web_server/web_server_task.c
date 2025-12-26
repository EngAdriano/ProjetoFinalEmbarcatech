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
 * Página HTML (Dashboard)
 * ========================= */
static const char html_index[] =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html\r\n\r\n"
"<!DOCTYPE html>"
"<html>"
"<head>"
"<meta charset='utf-8'>"
"<title>Embarcatech Dashboard</title>"
"<style>"
"body{background:#111;color:#eee;font-family:Arial;margin:0;padding:0;}"
"h1{background:#0a74da;padding:10px;margin:0;text-align:center;}"
".container{display:flex;justify-content:center;margin-top:20px;}"
".card{background:#222;border-radius:8px;padding:20px;margin:10px;width:200px;}"
".label{color:#0a74da;font-size:14px;}"
".value{font-size:28px;margin-top:10px;}"
"</style>"
"</head>"
"<body>"
"<h1>Embarcatech - Dashboard</h1>"
"<div class='container'>"
" <div class='card'>"
"  <div class='label'>Temperatura</div>"
"  <div class='value' id='temp'>--</div>"
" </div>"
" <div class='card'>"
"  <div class='label'>Umidade</div>"
"  <div class='value' id='hum'>--</div>"
" </div>"
" <div class='card'>"
"  <div class='label'>Luminosidade</div>"
"  <div class='value' id='lux'>--</div>"
" </div>"
"</div>"
"<script>"
"function updateEnv(){"
" fetch('/env')"
"  .then(r=>r.json())"
"  .then(d=>{"
"    temp.innerText=d.temperature.toFixed(1)+' °C';"
"    hum.innerText=d.humidity.toFixed(1)+' %';"
"    lux.innerText=d.lux.toFixed(1)+' lx';"
"  });"
"}"
"setInterval(updateEnv,1000);"
"updateEnv();"
"</script>"
"</body>"
"</html>";

/* =========================
 * Prototipos
 * ========================= */
static err_t http_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t http_recv(void *arg, struct tcp_pcb *tpcb,
                       struct pbuf *p, err_t err);

/* =========================
 * Handler HTTP
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

    char *req = calloc(p->len + 1, 1);
    memcpy(req, p->payload, p->len);

    /* ========= JSON /env ========= */
    if (strstr(req, "GET /env"))
    {
        const env_sensor_data_t *env = env_get_last();
        char resp[256];

        snprintf(resp, sizeof(resp),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n\r\n"
            "{"
            "\"temperature\":%.2f,"
            "\"humidity\":%.2f,"
            "\"lux\":%.2f"
            "}",
            env->temperature,
            env->humidity,
            env->lux
        );

        tcp_write(tpcb, resp, strlen(resp), TCP_WRITE_FLAG_COPY);
    }
    else
    {
        /* Página principal */
        tcp_write(tpcb, html_index,
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
 * Nova conexão
 * ========================= */
static err_t http_accept(void *arg,
                         struct tcp_pcb *newpcb,
                         err_t err)
{
    tcp_recv(newpcb, http_recv);
    return ERR_OK;
}

/* =========================
 * Task Web Server
 * ========================= */
void vTaskWebServer(void *pv)
{
    (void) pv;

    vTaskDelay(pdMS_TO_TICKS(3000));

    struct tcp_pcb *pcb = tcp_new();
    if (!pcb)
    {
        printf("[WEB] ERRO: tcp_new()\n");
        vTaskDelete(NULL);
    }

    tcp_bind(pcb, IP_ADDR_ANY, WEB_PORT);
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, http_accept);

    printf("[WEB] Dashboard web ativo na porta %d\n", WEB_PORT);

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
