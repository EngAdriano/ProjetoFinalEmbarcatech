# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(teste_HLW8032 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Cria o executável principal (usa seu arquivo fonte)
add_executable(teste_HLW8032
    teste_HLW8032.c
)

# Nome e versão do programa
pico_set_program_name(teste_HLW8032 "teste_HLW8032")
pico_set_program_version(teste_HLW8032 "0.1")

# Habilita saída USB (desativa UART padrão se quiser usar GPIO0/1)
pico_enable_stdio_usb(teste_HLW8032 1)
pico_enable_stdio_uart(teste_HLW8032 0)

# Inclui bibliotecas padrão e de hardware necessárias
target_link_libraries(teste_HLW8032
    pico_stdlib
)

# Inclui diretório local (para achar o .h do PIO)
target_include_directories(teste_HLW8032 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Gera os arquivos extras (UF2, BIN, ELF, etc.)
pico_add_extra_outputs(teste_HLW8032)
