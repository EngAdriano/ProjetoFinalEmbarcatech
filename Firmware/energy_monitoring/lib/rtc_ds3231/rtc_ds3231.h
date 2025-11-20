#ifndef DS3231_H
#define DS3231_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"

// Endereço I2C do DS3231
#define DS3231_ADDR          0x68

// I2C1 – GPIO2 (SDA) e GPIO3 (SCL)
#define DS3231_I2C_PORT      i2c1
#define DS3231_SDA_PIN       2
#define DS3231_SCL_PIN       3

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day_of_week;
    uint8_t day;
    uint8_t month;
    uint16_t year;   // Ex.: 2025
} ds3231_time_t;

void ds3231_init();
bool ds3231_get_time(ds3231_time_t *t);
bool ds3231_set_time(ds3231_time_t *t);
float ds3231_get_temperature();

#endif
