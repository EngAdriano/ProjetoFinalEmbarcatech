cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(hlw8032_example C CXX ASM)
pico_sdk_init()

add_executable(hlw8032_example
    main.c
    hlw8032.c
)

target_include_directories(hlw8032_example PRIVATE .)
target_link_libraries(hlw8032_example pico_stdlib hardware_uart)
pico_add_extra_outputs(hlw8032_example)
