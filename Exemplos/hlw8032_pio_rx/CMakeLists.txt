cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(hlw8032_pio_rx C CXX ASM)
pico_sdk_init()

pico_generate_pio_header(hlw8032_pio_rx ${CMAKE_CURRENT_LIST_DIR}/uart_rx_inverted.pio)

add_executable(hlw8032_pio_rx
    main.c
)

target_link_libraries(hlw8032_pio_rx pico_stdlib hardware_pio)
pico_add_extra_outputs(hlw8032_pio_rx)
