#include "web_server_task.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

/* Pico / Wi-Fi */
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

/* lwIP RAW TCP */
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"

/* Cache de dados ambientais */
#include "mqtt_aggregator.h"

/* =====================================================
 * Configurações
 * ===================================================== */
#define WEB_PORT 80

#define LOGIN_USER "admin"
#define LOGIN_PASS "1234"

#define SESSION_TIMEOUT_MS   (5 * 60 * 1000)   // 5 minutos

/* =====================================================
 * Controle de sessão
 * ===================================================== */
static bool user_logged = false;
static TickType_t last_activity_tick = 0;

/* =====================================================
 * HTML – Login
 * ===================================================== */
static const char html_login[] =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html\r\n\r\n"
"<!DOCTYPE html>"
"<html><head>"
"<meta charset='utf-8'>"
"<title>Login - Embarcatech</title>"
"<style>"
"body{background:#111;color:#eee;font-family:Arial;"
"display:flex;justify-content:center;align-items:center;height:100vh;}"
".box{background:#222;padding:20px;border-radius:8px;width:260px;}"
"h2{text-align:center;color:#0a74da;}"
"input,button{width:100%;padding:10px;margin-top:10px;"
"border-radius:4px;border:none;}"
"button{background:#0a74da;color:#fff;font-size:16px;}"
".err{color:red;text-align:center;display:none;}"
"</style></head>"
"<body>"
"<div class='box'>"
"<h2>Login</h2>"
"<input id='u' placeholder='Usuário'>"
"<input id='p' type='password' placeholder='Senha'>"
"<button onclick='login()'>Entrar</button>"
"<div id='e' class='err'>Login inválido</div>"
"</div>"
"<script>"
"function login(){"
"fetch('/login',{method:'POST',body:u.value+','+p.value})"
".then(r=>{if(r.status==200)location='/';"
"else e.style.display='block';});}"
"</script>"
"</body></html>";

/* =====================================================
 * HTML – Dashboard
 * ===================================================== */
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
".top{display:flex;justify-content:flex-end;padding:10px;}"
".top a{color:#fff;text-decoration:none;}"
".container{display:flex;justify-content:center;margin-top:20px;}"
".card{background:#222;border-radius:8px;padding:20px;margin:10px;width:200px;}"
".label{color:#0a74da;font-size:14px;}"
".value{font-size:28px;margin-top:10px;}"
"</style>"
"</head>"
"<body>"
"<div class='top'><a href='/logout'>Logout</a></div>"
"<h1>Embarcatech - Dashboard</h1>"
"<div class='container'>"
"<div class='card'><div class='label'>Temperatura</div><div class='value' id='temp'>--</div></div>"
"<div class='card'><div class='label'>Umidade</div><div class='value' id='hum'>--</div></div>"
"<div class='card'><div class='label'>Luminosidade</div><div class='value' id='lux'>--</div></div>"
"</div>"
"<script>"
"function updateEnv(){"
"fetch('/env').then(r=>r.json()).then(d=>{"
"temp.innerText=d.temperature.toFixed(1)+' °C';"
"hum.innerText=d.humidity.toFixed(1)+' %';"
"lux.innerText=d.lux.toFixed(1)+' lx';"
"});}"
"setInterval(updateEnv,1000);updateEnv();"
"</script>"
"</body></html>";

/* =====================================================
 * Protótipos
 * ===================================================== */
static err_t http_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t http_recv(void *arg, struct tcp_pcb *tpcb,
                       struct pbuf *p, err_t err);

/* =====================================================
 * Funções de sessão
 * ===================================================== */
static bool session_is_valid(void)
{
    if (!user_logged)
        return false;

    TickType_t now = xTaskGetTickCount();
    if ((now - last_activity_tick) > pdMS_TO_TICKS(SESSION_TIMEOUT_MS))
    {
        user_logged = false;
        return false;
    }
    return true;
}

static void session_touch(void)
{
    last_activity_tick = xTaskGetTickCount();
}

/* =====================================================
 * Handler HTTP
 * ===================================================== */
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

    /* ---------- LOGIN ---------- */
    if (strstr(req, "POST /login"))
    {
        char user[32] = {0};
        char pass[32] = {0};

        char *body = strstr(req, "\r\n\r\n");
        if (body) body += 4;

        if (body)
            sscanf(body, "%31[^,],%31s", user, pass);

        if (!strcmp(user, LOGIN_USER) &&
            !strcmp(pass, LOGIN_PASS))
        {
            user_logged = true;
            session_touch();

            tcp_write(tpcb,
                "HTTP/1.1 200 OK\r\n\r\n",
                19, TCP_WRITE_FLAG_COPY);
        }
        else
        {
            tcp_write(tpcb,
                "HTTP/1.1 401 Unauthorized\r\n\r\n",
                29, TCP_WRITE_FLAG_COPY);
        }
    }

    /* ---------- LOGOUT ---------- */
    else if (strstr(req, "GET /logout"))
    {
        user_logged = false;

        tcp_write(tpcb,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n\r\n"
            "<html><body><h2>Logout efetuado</h2>"
            "<a href='/'>Voltar</a></body></html>",
            110,
            TCP_WRITE_FLAG_COPY);
    }

    /* ---------- ENV ---------- */
    else if (strstr(req, "GET /env"))
    {
        if (!session_is_valid())
        {
            tcp_write(tpcb,
                "HTTP/1.1 403 Forbidden\r\n\r\n",
                26, TCP_WRITE_FLAG_COPY);
        }
        else
        {
            session_touch();

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

            tcp_write(tpcb, resp,
                      strlen(resp),
                      TCP_WRITE_FLAG_COPY);
        }
    }

    /* ---------- PÁGINAS ---------- */
    else
    {
        if (session_is_valid())
        {
            session_touch();
            tcp_write(tpcb,
                html_index,
                strlen(html_index),
                TCP_WRITE_FLAG_COPY);
        }
        else
        {
            tcp_write(tpcb,
                html_login,
                strlen(html_login),
                TCP_WRITE_FLAG_COPY);
        }
    }

    tcp_output(tpcb);
    tcp_close(tpcb);

    free(req);
    pbuf_free(p);

    return ERR_OK;
}

/* =====================================================
 * Nova conexão
 * ===================================================== */
static err_t http_accept(void *arg,
                         struct tcp_pcb *newpcb,
                         err_t err)
{
    tcp_recv(newpcb, http_recv);
    return ERR_OK;
}

/* =====================================================
 * Task Web Server
 * ===================================================== */
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

    printf("[WEB] Web server ativo na porta %d\n", WEB_PORT);

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
