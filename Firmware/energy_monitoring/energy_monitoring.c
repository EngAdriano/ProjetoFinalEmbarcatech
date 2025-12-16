#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"

#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "pzem004t.h"
#include "rtc_ds3231.h"
#include "eeprom_at24c32.h"

void vTaskSimulatedTemp(void *pvParameters);
void vTaskPZEMReader(void *pvParameters);
void vTaskRTCReader(void *pvParameters);
void vTaskEEPROMTest(void *pvParameters);

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Erro ao iniciar CYW43\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    wifi_init_manager();
    pzem_init();
    //ds3231_init();
    
    // ========================================
    //   DEFINA AQUI A DATA/HORA MANUALMENTE
    // ========================================
    ds3231_time_t init_time = {
        .seconds = 0,
        .minutes = 20,
        .hours   = 10,
        .day     = 20,
        .month   = 11,
        .year    = 2025,
        .day_of_week = 4
    };

    printf("[RTC] Gravando data/hora manual...\n");
    if (ds3231_set_time(&init_time)) {
        printf("[RTC] Data/hora configurada com sucesso!\n");
    } else {
        printf("[RTC] ERRO ao configurar data/hora!\n");
    }

    // Iniciar MQTT
    mqtt_start();

    // Criar tasks
    xTaskCreate(vTaskSimulatedTemp, "TempSimTask", 2048, NULL, 1, NULL);
    xTaskCreate(vTaskPZEMReader,   "PZEMReader",   4096, NULL, 1, NULL);
    xTaskCreate(vTaskRTCReader,  "RTCReader",    2048, NULL, 1, NULL);
    xTaskCreate(vTaskEEPROMTest, "EEPROMTest", 2048, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1) {}
}

// Exemplo de uso da eeprom_at24c32

#include "eeprom_at24c32.h"

void test_eeprom()
{
    at24c32_init();

    // Grava "OLA" na EEPROM
    uint8_t msg[] = "OLA";
    at24c32_write_block(0, msg, 3);

    sleep_ms(10);

    // Lê de volta
    uint8_t buffer[4];
    at24c32_read_block(0, buffer, 3);
    buffer[3] = '\0';

    printf("EEPROM: %s\n", buffer);
}

// Task de teste da EEPROM AT24C32
void vTaskEEPROMTest(void *pvParameters)
{
    // Inicializar EEPROM
    at24c32_init();
    printf("[EEPROM] Inicializada\n");

    uint16_t addr = 0; // endereço inicial
    uint8_t buffer[64];

    while (1)
    {
        // Texto para teste
        const char *msg = "Teste EEPROM OK";
        uint16_t len = strlen(msg);

        // Gravar na EEPROM
        if (at24c32_write_block(addr, (uint8_t*)msg, len))
        {
            printf("[EEPROM] Gravado: %s\n", msg);
        }
        else
        {
            printf("[EEPROM] ERRO ao gravar!\n");
        }

        // Pequena pausa
        vTaskDelay(pdMS_TO_TICKS(20));

        // Ler de volta
        memset(buffer, 0, sizeof(buffer));
        if (at24c32_read_block(addr, buffer, len))
        {
            printf("[EEPROM] Lido: %s\n", buffer);

            // Enviar via MQTT
            char json[128];
            snprintf(json, sizeof(json),
                "{\"eeprom_test\": \"%s\"}", buffer);

            mqtt_publish_async("pico/eeprom/test", json);
        }
        else
        {
            printf("[EEPROM] ERRO ao ler!\n");
        }

        // Aguarda 5 segundos
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}


void vTaskSimulatedTemp(void *pvParameters)
{
    float temp = 25.0f;   // temperatura inicial
    float step = 0.3f;    // variação por ciclo

    while (1)
    {
        // Gera oscilação da temperatura
        temp += step;
        if (temp > 30.0f || temp < 20.0f)
            step = -step;

        // Criar JSON
        char json[128];
        snprintf(json, sizeof(json), "{\"temperature\": %.2f}", temp);

        // Enviar via MQTT
        mqtt_publish_async("pico/sensor/temperature", json);

        printf("[TEMP] Enviado: %s\n", json);

        vTaskDelay(pdMS_TO_TICKS(5000)); // A cada 5 segundos
    }
}

void vTaskPZEMReader(void *pvParameters)
{
   pzem_data_t data;

    while (1) {
        if (pzem_read(&data)) {
            printf("V=%.1fV  I=%.3fA  P=%.1fW  E=%.3fkWh  F=%.1fHz  PF=%.2f\n",
                data.voltage, data.current, data.power, data.energy, data.frequency, data.pf);
        } else {
            printf("Falha leitura\n");
        }

        sleep_ms(2000);
    }
}


void vTaskRTCReader(void *pvParameters)
{
    ds3231_time_t now;

    while (1)
    {
        if (ds3231_get_time(&now))
        {
            float temp = ds3231_get_temperature();

            char json[256];
            snprintf(json, sizeof(json),
                "{"
                "\"date\": \"%02d/%02d/%04d\","
                "\"time\": \"%02d:%02d:%02d\","
                "\"temperature\": %.2f"
                "}",
                now.day,
                now.month,
                now.year,
                now.hours,
                now.minutes,
                now.seconds,
                temp
            );

            mqtt_publish_async("pico/system/rtc", json);
            printf("[RTC] %s\n", json);
        }
        else
        {
            printf("[RTC] Falha ao ler RTC.\n");
        }

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
