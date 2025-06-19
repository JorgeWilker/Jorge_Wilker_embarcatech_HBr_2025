# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
include(${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
# ====================================================================================
cmake_minimum_required(VERSION 3.21)

# Set any variables required for importing libraries
SET(FREERTOS_PATH ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS)

message("FreeRTOS Kernel located in ${FREERTOS_PATH}")

# Import those libraries
include(pico_sdk_import.cmake)
include(${FREERTOS_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(sistema-caldeira C CXX ASM)

pico_sdk_init()


# Executável do sistema de caldeira
add_executable(caldeira
   caldeira_main.c
   include/ssd1306_i2c.c
)

# Adicionar diretório de include para caldeira
target_include_directories(caldeira PRIVATE 
    include
    ${CMAKE_CURRENT_LIST_DIR}
)

# Gerar arquivos PIO
pico_generate_pio_header(caldeira ${CMAKE_CURRENT_LIST_DIR}/ws2818b.pio)

# Configurar stdio para caldeira
pico_enable_stdio_uart(caldeira 0)
pico_enable_stdio_usb(caldeira 1)

# Linkar bibliotecas necessárias para caldeira
target_link_libraries(caldeira 
    pico_stdlib 
    FreeRTOS-Kernel-Heap4
    hardware_adc 
    hardware_i2c 
    hardware_pio
    hardware_gpio
)

pico_add_extra_outputs(caldeira)


