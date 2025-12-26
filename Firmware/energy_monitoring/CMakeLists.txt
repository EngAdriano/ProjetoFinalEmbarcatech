# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/FreeRTOS/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(energy_monitoring C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(energy_monitoring 
        energy_monitoring.c
        lib/wifi_manager/wifi_manager.c
        lib/mqtt_manager/mqtt_manager.c
        lib/pzem004t/pzem004t.c
        lib/rtc_ds3231/rtc_ds3231.c
        lib/eeprom_at24c32/eeprom_at24c32.c
        lib/st7735/st7735.c
        lib/st7735/fonts.c
        lib/aht10/aht10.c
        lib/bh1750/bh1750.c
        lib/ui_energy/ui_energy.c
        lib/time_manager/time_manager.c
        lib/env_sensors/env_sensors.c
        lib/payload_builder/payload_builder.c
        lib/pzem_task/pzem_task.c
        lib/ui_energy/ui_energy_task.c
        lib/mqtt_aggregator/mqtt_aggregator.c
        lib/ui_energy/ui_environment.c
        lib/web_server/web_server_task.c
        lib/web_server/auth_storage.c
        lib/web_server/sha256.c
)

pico_set_program_name(energy_monitoring "energy_monitoring")
pico_set_program_version(energy_monitoring "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(energy_monitoring 0)
pico_enable_stdio_usb(energy_monitoring 1)

# Add the standard library to the build
target_link_libraries(energy_monitoring
        pico_stdlib
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
)

# Add the standard include files to the build
target_include_directories(energy_monitoring PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/wifi_manager
        ${CMAKE_CURRENT_LIST_DIR}/lib/mqtt_manager
        ${CMAKE_CURRENT_LIST_DIR}/lib/pzem004t
        ${CMAKE_CURRENT_LIST_DIR}/lib/rtc_ds3231
        ${CMAKE_CURRENT_LIST_DIR}/lib/eeprom_at24c32
        ${CMAKE_CURRENT_LIST_DIR}/lib/time_manager
        ${CMAKE_CURRENT_LIST_DIR}/lib/st7735
        ${CMAKE_CURRENT_LIST_DIR}/lib/aht10
        ${CMAKE_CURRENT_LIST_DIR}/lib/bh1750
        ${CMAKE_CURRENT_LIST_DIR}/lib/ui_energy
        ${CMAKE_CURRENT_LIST_DIR}/lib/env_sensors
        ${CMAKE_CURRENT_LIST_DIR}/lib/payload_builder
        ${CMAKE_CURRENT_LIST_DIR}/lib/pzem_task
        ${CMAKE_CURRENT_LIST_DIR}/lib/mqtt_aggregator
        ${CMAKE_CURRENT_LIST_DIR}/lib/web_server
)

# Add any user requested libraries
target_link_libraries(energy_monitoring 
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_mqtt
        hardware_i2c
        hardware_spi
        )

pico_add_extra_outputs(energy_monitoring)

target_link_options(energy_monitoring PRIVATE -Wl,--print-memory-usage)