#include "eeprom_at24c32.h"
#include "pico/stdlib.h"

//==============================
// Inicialização
//==============================
void at24c32_init() {
    i2c_init(AT24C32_I2C_PORT, 100 * 1000);
}

//==============================
// Escrita de 1 byte
//==============================
bool at24c32_write_byte(uint16_t addr, uint8_t data)
{
    uint8_t buf[3];
    buf[0] = (addr >> 8) & 0xFF; // High
    buf[1] = (addr & 0xFF);      // Low
    buf[2] = data;

    int ret = i2c_write_blocking(AT24C32_I2C_PORT, AT24C32_ADDR, buf, 3, false);

    sleep_ms(5); // tempo de escrita interna

    return ret >= 0;
}

//==============================
// Leitura de 1 byte
//==============================
bool at24c32_read_byte(uint16_t addr, uint8_t *data)
{
    uint8_t addr_buf[2];
    addr_buf[0] = (addr >> 8) & 0xFF;
    addr_buf[1] = (addr & 0xFF);

    if (i2c_write_blocking(AT24C32_I2C_PORT, AT24C32_ADDR, addr_buf, 2, true) < 0)
        return false;

    if (i2c_read_blocking(AT24C32_I2C_PORT, AT24C32_ADDR, data, 1, false) < 0)
        return false;

    return true;
}

//==============================
// Escrita de bloco (com controle de páginas)
//==============================
bool at24c32_write_block(uint16_t addr, const uint8_t *data, uint16_t len)
{
    while (len > 0)
    {
        uint16_t page_offset = addr % AT24C32_PAGE_SIZE;
        uint16_t bytes_to_write = AT24C32_PAGE_SIZE - page_offset;

        if (bytes_to_write > len)
            bytes_to_write = len;

        // buffer: [AH][AL][bytes...]
        uint8_t buf[2 + AT24C32_PAGE_SIZE];
        buf[0] = (addr >> 8) & 0xFF;
        buf[1] = (addr & 0xFF);

        for (uint16_t i = 0; i < bytes_to_write; i++)
            buf[2 + i] = data[i];

        if (i2c_write_blocking(AT24C32_I2C_PORT, AT24C32_ADDR, buf, 2 + bytes_to_write, false) < 0)
            return false;

        sleep_ms(5);

        addr += bytes_to_write;
        data += bytes_to_write;
        len -= bytes_to_write;
    }

    return true;
}

//==============================
// Leitura de bloco
//==============================
bool at24c32_read_block(uint16_t addr, uint8_t *data, uint16_t len)
{
    uint8_t addr_buf[2];
    addr_buf[0] = (addr >> 8) & 0xFF;
    addr_buf[1] = (addr & 0xFF);

    if (i2c_write_blocking(AT24C32_I2C_PORT, AT24C32_ADDR, addr_buf, 2, true) < 0)
        return false;

    if (i2c_read_blocking(AT24C32_I2C_PORT, AT24C32_ADDR, data, len, false) < 0)
        return false;

    return true;
}
