#ifndef AT24C32_H
#define AT24C32_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"

// Endereço I2C da EEPROM AT24C32
#define AT24C32_ADDR 0x57

// Escolher I2C1 como DS3231 (GPIO2 SDA, GPIO3 SCL)
#define AT24C32_I2C_PORT i2c1

// Página da EEPROM AT24C32 tem 32 bytes
#define AT24C32_PAGE_SIZE 32
#define AT24C32_TOTAL_SIZE 4096

// Funções da EEPROM
void at24c32_init();
bool at24c32_write_byte(uint16_t addr, uint8_t data);
bool at24c32_read_byte(uint16_t addr, uint8_t *data);

bool at24c32_write_block(uint16_t addr, const uint8_t *data, uint16_t len);
bool at24c32_read_block(uint16_t addr, uint8_t *data, uint16_t len);

#endif
