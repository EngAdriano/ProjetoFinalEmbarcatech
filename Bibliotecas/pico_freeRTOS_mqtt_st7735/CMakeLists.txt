cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(pico_freeRTOS_mqtt_st7735)

pico_sdk_init()

add_executable(pico_freeRTOS_mqtt_st7735
  src/main.c
  src/tasks_wifi.c
  src/tasks_mqtt.c
  src/tasks_sensors.c
  src/tasks_display.c
  src/aht10.c
  src/st7735.c
)

target_include_directories(pico_freeRTOS_mqtt_st7735 PRIVATE include src)

target_link_libraries(pico_freeRTOS_mqtt_st7735
  pico_stdlib
  hardware_spi
  hardware_i2c
  hardware_uart
  pico_cyw43_driver
  lwip_core
  lwip_mqtt
)

add_subdirectory(vendor/freertos EXCLUDE_FROM_ALL)

pico_enable_stdio_usb(pico_freeRTOS_mqtt_st7735 1)
pico_enable_stdio_uart(pico_freeRTOS_mqtt_st7735 1)
pico_add_extra_outputs(pico_freeRTOS_mqtt_st7735)
