        # Pico W FreeRTOS MQTT ST7735 (Skeleton)

        ## Resumo
        Esqueleto de projeto para Raspberry Pi Pico W usando pico-sdk + FreeRTOS, drivers enviados (AHT10 e ST7735) e cliente MQTT nativo do lwIP (esqueleto de conexão TLS para HiveMQ Cloud).

        ## Como compilar
        1. Instale e configure pico-sdk conforme documentação oficial.
        2. Coloque FreeRTOS em `vendor/freertos` (submodule ou fonte) ou ajuste o `CMakeLists.txt` para sua configuração.
        3. `mkdir build && cd build`
4. `cmake ..`
5. `make -j4`

        ## Observações
        - Implementação TLS ainda precisa ser integrada (mbedTLS). O código já tem placeholders.
        - Substitua `WIFI_SSID` e `WIFI_PASSWORD` em `include/credentials.h` antes de compilar.
