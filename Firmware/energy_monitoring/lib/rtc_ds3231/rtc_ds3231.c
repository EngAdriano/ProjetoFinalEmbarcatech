#include "rtc_ds3231.h"
#include "pico/stdlib.h"

//==============================
// Funções auxiliares BCD
//==============================
static uint8_t bcd_to_dec(uint8_t val) {
    return (val >> 4) * 10 + (val & 0x0F);
}

static uint8_t dec_to_bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

//==============================
// Inicialização do I2C + DS3231
//==============================
void ds3231_init() {
    i2c_init(DS3231_I2C_PORT, 100 * 1000);

    gpio_set_function(DS3231_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(DS3231_SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(DS3231_SDA_PIN);
    gpio_pull_up(DS3231_SCL_PIN);

    sleep_ms(50);
}

//==============================
// Leitura da data e hora
//==============================
bool ds3231_get_time(ds3231_time_t *t) {

    uint8_t reg = 0x00;
    uint8_t data[7];

    if (i2c_write_blocking(DS3231_I2C_PORT, DS3231_ADDR, &reg, 1, true) < 0)
        return false;

    if (i2c_read_blocking(DS3231_I2C_PORT, DS3231_ADDR, data, 7, false) < 0)
        return false;

    t->seconds = bcd_to_dec(data[0]);
    t->minutes = bcd_to_dec(data[1]);
    t->hours   = bcd_to_dec(data[2]);
    t->day_of_week = bcd_to_dec(data[3]);
    t->day     = bcd_to_dec(data[4]);
    t->month   = bcd_to_dec(data[5] & 0x1F);
    t->year    = 2000 + bcd_to_dec(data[6]);

    return true;
}

//==============================
// Configuração da data e hora
//==============================
bool ds3231_set_time(ds3231_time_t *t) {

    uint8_t data[8];
    data[0] = 0x00;                       // registro inicial
    data[1] = dec_to_bcd(t->seconds);
    data[2] = dec_to_bcd(t->minutes);
    data[3] = dec_to_bcd(t->hours);
    data[4] = dec_to_bcd(t->day_of_week);
    data[5] = dec_to_bcd(t->day);
    data[6] = dec_to_bcd(t->month);
    data[7] = dec_to_bcd(t->year - 2000);

    if (i2c_write_blocking(DS3231_I2C_PORT, DS3231_ADDR, data, 8, false) < 0)
        return false;

    return true;
}

//==============================
// Leitura da temperatura interna
//==============================
float ds3231_get_temperature() {

    uint8_t reg = 0x11;
    uint8_t buf[2];

    if (i2c_write_blocking(DS3231_I2C_PORT, DS3231_ADDR, &reg, 1, true) < 0)
        return 0;

    if (i2c_read_blocking(DS3231_I2C_PORT, DS3231_ADDR, buf, 2, false) < 0)
        return 0;

    int8_t msb = buf[0];
    uint8_t lsb = buf[1] >> 6;

    return msb + (lsb * 0.25f);
}
