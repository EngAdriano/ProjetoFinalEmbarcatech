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

/* Projeto */
#include "mqtt_aggregator.h"
#include "auth_storage.h"
#include "sha256.h"

/* Hardware */
#include "hardware/watchdog.h"

/* =====================================================
 * Configurações
 * ===================================================== */
#define WEB_PORT                80
#define SESSION_TIMEOUT_MS      (5 * 60 * 1000)

#define FACTORY_RESET_GPIO      6
#define FACTORY_RESET_TIME_MS   5000

/* =====================================================
 * Sessão
 * ===================================================== */
static bool user_logged = false;
static TickType_t last_activity_tick = 0;

/* Credenciais */
static char     login_user[32];
static uint8_t  login_pass_hash[32];

/* =====================================================
 * Sessão – utilitários
 * ===================================================== */
static bool session_is_valid(void)
{
    if (!user_logged)
        return false;

    if ((xTaskGetTickCount() - last_activity_tick) >
        pdMS_TO_TICKS(SESSION_TIMEOUT_MS))
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
 * HTML – LOGIN
 * ===================================================== */
static const char html_login[] =
"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
"<!DOCTYPE html><html><head><meta charset='utf-8'>"
"<title>Login</title>"
"<style>"
"body{background:#0f172a;color:#e5e7eb;font-family:Arial;"
"display:flex;justify-content:center;align-items:center;height:100vh}"
".box{background:#1e293b;padding:20px;border-radius:8px;width:260px}"
"h2{text-align:center;color:#38bdf8}"
"input,button{width:100%;padding:10px;margin-top:10px;border-radius:4px;border:none}"
"button{background:#0284c7;color:#fff;font-size:16px}"
".err{color:#f87171;text-align:center;display:none}"
"</style></head>"
"<body><div class='box'>"
"<h2>Login</h2>"
"<input id='u' placeholder='Usuário'>"
"<input id='p' type='password' placeholder='Senha'>"
"<button onclick='login()'>Entrar</button>"
"<div id='e' class='err'>Login inválido</div>"
"</div>"
"<script>"
"function login(){"
"fetch('/login',{method:'POST',body:u.value+','+p.value})"
".then(r=>{if(r.status==200)location='/';else e.style.display='block';});}"
"</script></body></html>";

/* =====================================================
 * HTML – CONFIGURAR LOGIN
 * ===================================================== */
static const char html_auth_cfg[] =
"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
"<!DOCTYPE html><html><head><meta charset='utf-8'>"
"<title>Configurar Login</title>"
"<style>"
"body{background:#0f172a;color:#e5e7eb;font-family:Arial;"
"display:flex;justify-content:center;align-items:center;height:100vh}"
".box{background:#1e293b;padding:20px;border-radius:8px;width:300px}"
"h2{text-align:center;color:#38bdf8}"
"input,button{width:100%;padding:10px;margin-top:10px;border-radius:4px;border:none}"
"button{background:#0284c7;color:#fff;font-size:16px}"
"a{color:#38bdf8;text-align:center;display:block;margin-top:10px}"
"</style></head>"
"<body><div class='box'>"
"<h2>Credenciais</h2>"
"<input id='u' placeholder='Novo usuário'>"
"<input id='p' type='password' placeholder='Nova senha'>"
"<button onclick='save()'>Salvar</button>"
"<p id='m'></p>"
"<a href='/'>Voltar</a>"
"</div>"
"<script>"
"function save(){"
"fetch('/config/auth',{method:'POST',body:u.value+','+p.value})"
".then(r=>r.text()).then(t=>m.innerText=t);}"
"</script></body></html>";

/* =====================================================
 * HTML – DASHBOARD
 * ===================================================== */
static const char html_index[] =
"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
"<!DOCTYPE html><html><head><meta charset='utf-8'>"
"<meta name='viewport' content='width=device-width, initial-scale=1'>"
"<title>Embarcatech</title>"
"<style>"
"body{background:#0f172a;color:#e5e7eb;font-family:Arial;margin:0}"
"h1{background:#0284c7;padding:12px;margin:0;text-align:center}"
".top{display:flex;justify-content:flex-end;padding:10px}"
".top a{color:#e5e7eb;text-decoration:none;margin-left:15px}"
".section{padding:20px}"
".section h2{color:#38bdf8}"
".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(140px,1fr));gap:12px}"
".card{background:#1e293b;border-radius:8px;padding:15px;text-align:center}"
".label{font-size:.85em;color:#94a3b8}"
".value{font-size:1.4em;font-weight:bold;margin-top:6px}"
"</style></head><body>"

"<div class='top'>"
"<a href='/config/auth'>Configurar Login</a>"
"<a href='/logout'>Logout</a>"
"</div>"

"<h1>Monitoramento</h1>"

"<div class='section'><h2>⚡ Energia</h2><div class='grid'>"
"<div class='card'><div class='label'>Tensão</div><div class='value' id='v'>--</div></div>"
"<div class='card'><div class='label'>Corrente</div><div class='value' id='i'>--</div></div>"
"<div class='card'><div class='label'>Potência</div><div class='value' id='p'>--</div></div>"
"<div class='card'><div class='label'>Energia</div><div class='value' id='e'>--</div></div>"
"<div class='card'><div class='label'>Frequência</div><div class='value' id='f'>--</div></div>"
"<div class='card'><div class='label'>FP</div><div class='value' id='pf'>--</div></div>"
"</div></div>"

"<div class='section'><h2>🌱 Ambiente</h2><div class='grid'>"
"<div class='card'><div class='label'>Temperatura</div><div class='value' id='t'>--</div></div>"
"<div class='card'><div class='label'>Umidade</div><div class='value' id='h'>--</div></div>"
"<div class='card'><div class='label'>Luminosidade</div><div class='value' id='l'>--</div></div>"
"</div></div>"

"<script>"
"function upd(){"
"fetch('/energy').then(r=>r.json()).then(d=>{"
"v.innerText=d.voltage+' V';i.innerText=d.current+' A';"
"p.innerText=d.power+' W';e.innerText=d.energy+' kWh';"
"f.innerText=d.frequency+' Hz';pf.innerText=d.pf;});"
"fetch('/env').then(r=>r.json()).then(d=>{"
"t.innerText=d.temperature+' °C';h.innerText=d.humidity+' %';"
"l.innerText=d.lux+' lx';});}"
"setInterval(upd,2000);upd();"
"</script></body></html>";

/* =====================================================
 * HTTP Handler
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

    /* LOGIN */
    if (strstr(req, "POST /login"))
    {
        char u[32] = {0}, pw[32] = {0};
        char *b = strstr(req, "\r\n\r\n");
        if (b) sscanf(b + 4, "%31[^,],%31s", u, pw);

        uint8_t hash[32];
        sha256((uint8_t *)pw, strlen(pw), hash);

        if (!strcmp(u, login_user) &&
            memcmp(hash, login_pass_hash, 32) == 0)
        {
            user_logged = true;
            session_touch();
            tcp_write(tpcb, "HTTP/1.1 200 OK\r\n\r\n", 19, TCP_WRITE_FLAG_COPY);
        }
        else
        {
            tcp_write(tpcb, "HTTP/1.1 401 Unauthorized\r\n\r\n",
                      29, TCP_WRITE_FLAG_COPY);
        }
    }

    /* LOGOUT */
    else if (strstr(req, "GET /logout"))
    {
        user_logged = false;
        tcp_write(tpcb, html_login,
                  strlen(html_login), TCP_WRITE_FLAG_COPY);
    }

    /* CONFIG AUTH – GET */
    else if (strstr(req, "GET /config/auth"))
    {
        if (!session_is_valid())
            tcp_write(tpcb, "HTTP/1.1 403 Forbidden\r\n\r\n",
                      26, TCP_WRITE_FLAG_COPY);
        else
            tcp_write(tpcb, html_auth_cfg,
                      strlen(html_auth_cfg), TCP_WRITE_FLAG_COPY);
    }

    /* CONFIG AUTH – POST */
    else if (strstr(req, "POST /config/auth"))
    {
        if (!session_is_valid())
        {
            tcp_write(tpcb, "HTTP/1.1 403 Forbidden\r\n\r\n",
                      26, TCP_WRITE_FLAG_COPY);
        }
        else
        {
            char user[32] = {0};
            char pass[32] = {0};

            char *body = strstr(req, "\r\n\r\n");
            if (body)
                sscanf(body + 4, "%31[^,],%31s", user, pass);

            if (auth_save(user, pass))
            {
                auth_load(login_user, login_pass_hash);
                user_logged = false;

                tcp_write(tpcb,
                    "HTTP/1.1 200 OK\r\n\r\nCredenciais atualizadas. Faça login novamente.",
                    78, TCP_WRITE_FLAG_COPY);
            }
            else
            {
                tcp_write(tpcb,
                    "HTTP/1.1 400 Bad Request\r\n\r\nErro ao salvar",
                    53, TCP_WRITE_FLAG_COPY);
            }
        }
    }

    /* ENV */
    else if (strstr(req, "GET /env"))
    {
        if (!session_is_valid())
            tcp_write(tpcb, "HTTP/1.1 403 Forbidden\r\n\r\n",
                      26, TCP_WRITE_FLAG_COPY);
        else
        {
            const env_sensor_data_t *env = env_get_last();
            char buf[256];
            snprintf(buf, sizeof(buf),
                "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n"
                "{\"temperature\":%.2f,\"humidity\":%.2f,\"lux\":%.2f}",
                env->temperature, env->humidity, env->lux);
            tcp_write(tpcb, buf, strlen(buf), TCP_WRITE_FLAG_COPY);
        }
    }

    /* ENERGY */
    else if (strstr(req, "GET /energy"))
    {
        if (!session_is_valid())
            tcp_write(tpcb, "HTTP/1.1 403 Forbidden\r\n\r\n",
                      26, TCP_WRITE_FLAG_COPY);
        else
        {
            const energy_data_t *e = energy_get_last();
            char buf[256];
            snprintf(buf, sizeof(buf),
                "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n"
                "{\"voltage\":%.1f,\"current\":%.2f,\"power\":%.1f,"
                "\"energy\":%.3f,\"frequency\":%.1f,\"pf\":%.2f}",
                e->voltage, e->current, e->power,
                e->energy, e->frequency, e->pf);
            tcp_write(tpcb, buf, strlen(buf), TCP_WRITE_FLAG_COPY);
        }
    }

    /* DEFAULT */
    else
    {
        if (session_is_valid())
            tcp_write(tpcb, html_index,
                      strlen(html_index), TCP_WRITE_FLAG_COPY);
        else
            tcp_write(tpcb, html_login,
                      strlen(html_login), TCP_WRITE_FLAG_COPY);
    }

    tcp_output(tpcb);
    tcp_close(tpcb);
    free(req);
    pbuf_free(p);
    return ERR_OK;
}

/* =====================================================
 * TCP Accept
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
    vTaskDelay(pdMS_TO_TICKS(3000));
    auth_load(login_user, login_pass_hash);

    struct tcp_pcb *pcb = tcp_new();
    tcp_bind(pcb, IP_ADDR_ANY, WEB_PORT);
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, http_accept);

    printf("[WEB] Servidor ativo na porta %d\n", WEB_PORT);

    while (1)
        vTaskDelay(pdMS_TO_TICKS(10000));
}

/* =====================================================
 * Factory Reset
 * ===================================================== */
void factory_button_init(void)
{
    gpio_init(FACTORY_RESET_GPIO);
    gpio_set_dir(FACTORY_RESET_GPIO, GPIO_IN);
    gpio_pull_up(FACTORY_RESET_GPIO);
}

void vTaskFactoryReset(void *pv)
{
    while (1)
    {
        if (gpio_get(FACTORY_RESET_GPIO) == 0)
        {
            TickType_t start = xTaskGetTickCount();
            while (gpio_get(FACTORY_RESET_GPIO) == 0)
            {
                if ((xTaskGetTickCount() - start) >
                    pdMS_TO_TICKS(FACTORY_RESET_TIME_MS))
                {
                    auth_factory_reset();
                    sleep_ms(500);
                    watchdog_reboot(0, 0, 0);
                }
                vTaskDelay(pdMS_TO_TICKS(50));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
