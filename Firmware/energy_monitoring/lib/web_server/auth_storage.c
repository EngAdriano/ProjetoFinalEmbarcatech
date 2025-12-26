#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "auth_storage.h"
#include "eeprom_at24c32.h"

/* =========================
 * Layout da EEPROM
 * ========================= */
#define EEPROM_ADDR_USER   0x0000   // 32 bytes
#define EEPROM_ADDR_PASS   0x0020   // 32 bytes
#define EEPROM_MAX_LEN     32

static const char default_user[] = "admin";
static const char default_pass[] = "1234";

/* =========================
 * Limpa região da EEPROM
 * ========================= */
static void auth_clear_region(void)
{
    uint8_t empty[EEPROM_MAX_LEN];

    memset(empty, 0xFF, EEPROM_MAX_LEN);

    at24c32_write_block(EEPROM_ADDR_USER, empty, EEPROM_MAX_LEN);
    at24c32_write_block(EEPROM_ADDR_PASS, empty, EEPROM_MAX_LEN);
}


/* =========================
 * Carrega credenciais
 * ========================= */
void auth_load(char *user, char *pass)
{
    at24c32_init();

    /* Limpa buffers antes de ler */
    memset(user, 0, EEPROM_MAX_LEN);
    memset(pass, 0, EEPROM_MAX_LEN);

    at24c32_read_block(EEPROM_ADDR_USER, (uint8_t *)user, EEPROM_MAX_LEN);
    at24c32_read_block(EEPROM_ADDR_PASS, (uint8_t *)pass, EEPROM_MAX_LEN);

    /* Garante terminação */
    user[EEPROM_MAX_LEN - 1] = '\0';
    pass[EEPROM_MAX_LEN - 1] = '\0';

    /* Validação simples: string ASCII imprimível */
    bool invalid = false;

    if (user[0] == 0x00 || user[0] == (char)0xFF)
        invalid = true;

    for (int i = 0; i < EEPROM_MAX_LEN && user[i] != '\0'; i++)
    {
        if (user[i] < 32 || user[i] > 126)
        {
            invalid = true;
            break;
        }
    }

    if (invalid)
    {
        printf("[AUTH] EEPROM invalida, recriando credenciais\n");

        memset(user, 0, EEPROM_MAX_LEN);
        memset(pass, 0, EEPROM_MAX_LEN);

        strcpy(user, default_user);
        strcpy(pass, default_pass);

        /* Limpa região antes de gravar */
        uint8_t ff[EEPROM_MAX_LEN];
        memset(ff, 0xFF, EEPROM_MAX_LEN);
        at24c32_write_block(EEPROM_ADDR_USER, ff, EEPROM_MAX_LEN);
        at24c32_write_block(EEPROM_ADDR_PASS, ff, EEPROM_MAX_LEN);

        at24c32_write_block(EEPROM_ADDR_USER,
                            (const uint8_t *)user,
                            EEPROM_MAX_LEN);
        at24c32_write_block(EEPROM_ADDR_PASS,
                            (const uint8_t *)pass,
                            EEPROM_MAX_LEN);
    }

    printf("[AUTH] Usuario EEPROM: '%s'\n", user);
}

