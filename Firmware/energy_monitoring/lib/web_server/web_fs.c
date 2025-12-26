#include "lwip/apps/httpd.h"
#include <string.h>

/* Página HTML */
static const char index_html[] =
"<!DOCTYPE html>"
"<html>"
"<head>"
"<meta charset='utf-8'>"
"<title>Embarcatech</title>"
"</head>"
"<body style='background:#111;color:#0f0;font-family:Arial;'>"
"<h1>Servidor Web Ativo</h1>"
"<p>Projeto Embarcatech</p>"
"</body>"
"</html>";

/* Implementação mínima do FS */
int fs_open_custom(struct fs_file *file, const char *name)
{
    if (strcmp(name, "/") == 0 || strcmp(name, "/index.html") == 0)
    {
        file->data  = index_html;
        file->len   = strlen(index_html);
        file->index = file->len;
        file->flags = FS_FILE_FLAGS_HEADER_INCLUDED;
        return 1;
    }

    return 0;
}
