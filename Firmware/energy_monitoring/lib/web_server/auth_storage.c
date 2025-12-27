#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "auth_storage.h"
#include "eeprom_at24c32.h"
#include "sha256.h"

#define EEPROM_ADDR_USER 0x0000
#define EEPROM_ADDR_PASS 0x0020
#define EEPROM_MAX_LEN   32

static const char default_user[] = "admin";
static const char default_pass[] = "1234";

void auth_load(char *user, uint8_t pass_hash[32])
{
    at24c32_init();

    //printf("[AUTH] RESET FORÇADO EEPROM\n");
    // Limpa EEPROM (FORÇAR RESET DE CREDENCIAIS)
    //uint8_t ff[32];
    //memset(ff, 0xFF, 32);
    //at24c32_write_block(EEPROM_ADDR_USER, ff, 32);
    //at24c32_write_block(EEPROM_ADDR_PASS, ff, 32);

    memset(user, 0, EEPROM_MAX_LEN);
    memset(pass_hash, 0, 32);

    at24c32_read_block(EEPROM_ADDR_USER, (uint8_t *)user, EEPROM_MAX_LEN);
    at24c32_read_block(EEPROM_ADDR_PASS, pass_hash, 32);

    if (user[0] == 0xFF || user[0] == 0x00)
    {
        strcpy(user, default_user);
        sha256((const uint8_t *)default_pass,
               strlen(default_pass),
               pass_hash);

        at24c32_write_block(EEPROM_ADDR_USER,
                            (const uint8_t *)user,
                            EEPROM_MAX_LEN);
        at24c32_write_block(EEPROM_ADDR_PASS,
                            pass_hash,
                            32);
    }

    user[EEPROM_MAX_LEN - 1] = '\0';
}

bool auth_save(const char *user, const char *pass)
{
    uint8_t hash[32];
    sha256((const uint8_t *)pass, strlen(pass), hash);

    at24c32_write_block(EEPROM_ADDR_USER,
                        (const uint8_t *)user,
                        EEPROM_MAX_LEN);
    at24c32_write_block(EEPROM_ADDR_PASS,
                        hash,
                        32);

    return true;
}

void auth_factory_reset(void)
{
    uint8_t ff[EEPROM_MAX_LEN];
    memset(ff, 0xFF, EEPROM_MAX_LEN);

    at24c32_init();

    at24c32_write_block(EEPROM_ADDR_USER, ff, EEPROM_MAX_LEN);
    at24c32_write_block(EEPROM_ADDR_PASS, ff, EEPROM_MAX_LEN);

    printf("[AUTH] Factory reset executado\n");
}
