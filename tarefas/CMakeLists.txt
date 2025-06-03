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
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

# IMPORTANTE: Define a placa como Pico W para ter suporte a WiFi
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Habilita MQTT (para lwIP)
set(PICO_ENABLE_MQTT 1)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(bitdoglab_wifi C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable
add_executable(bitdoglab_wifi
    main.c
    wifi_conn.c
    mqtt_comm.c
)

pico_set_program_name(bitdoglab_wifi "bitdoglab_wifi")
pico_set_program_version(bitdoglab_wifi "0.1")

# Configuração de E/S - ajuste conforme necessidade
pico_enable_stdio_uart(bitdoglab_wifi 1)
pico_enable_stdio_usb(bitdoglab_wifi 1)

# Inclui diretórios
target_include_directories(bitdoglab_wifi PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PICO_SDK_PATH}/lib/lwip/src/include
    ${PICO_SDK_PATH}/lib/lwip/contrib/
)

# IMPORTANTE: Bibliotecas necessárias para lwIP/WiFi e MQTT
target_link_libraries(bitdoglab_wifi
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
)

# Gera arquivos adicionais (.uf2, etc)
pico_add_extra_outputs(bitdoglab_wifi) 