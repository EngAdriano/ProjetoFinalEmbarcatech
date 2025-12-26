#include "web_pages.h"

/* Página HTML simples */
const char *web_page_index(void)
{
    return
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset='utf-8'>"
        "<title>Embarcatech</title>"
        "<style>"
        "body{font-family:Arial;background:#111;color:#0f0;text-align:center;}"
        "</style>"
        "</head>"
        "<body>"
        "<h1>Servidor Web Ativo</h1>"
        "<p>Projeto Embarcatech</p>"
        "</body>"
        "</html>";
}
