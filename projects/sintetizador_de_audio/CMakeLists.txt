cmake_minimum_required(VERSION 3.13)

# -----
# Configuração do SDK da Raspberry Pi Pico
# -----
# Caminho da SDK da Raspberry Pi Pico (ajuste conforme necessário)
# Use variável de ambiente PICO_SDK_PATH ou defina o caminho abaixo
if(DEFINED ENV{PICO_SDK_PATH})
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
else()
set(PICO_SDK_PATH "C:/Users/Jorge Wilker Tech/.pico-sdk/sdk/2.1.1" CACHE PATH "Path to the Raspberry Pi Pico SDK")
endif()

# Configuração do padrão de compilação
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# -----
# Configuração da placa 
# -----
# Por padrão, use a Pico padrão para evitar problemas de compilação em Windows
# Para compilar com Wireless (Pico W), descomente a linha abaixo e comente a linha anterior
set(PICO_BOARD pico)
#set(PICO_BOARD pico_w)

# Opções para configuração do projeto
option(ENABLE_WIRELESS "Habilitar componentes wireless (pode causar erros em Windows)" OFF)

# Importa o SDK da Pico
include(pico_sdk_import.cmake)

# Nome do projeto baseado no nome da pasta
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE "." "_" PROJECT_NAME ${PROJECT_NAME})
project(${PROJECT_NAME} C CXX ASM)

# Inicializa o SDK
pico_sdk_init()

# Adicionar diretório de includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# -----
# Bibliotecas do projeto
# -----
# Biblioteca para áudio PWM
add_library(audio_pwm
    src/audio_pwm.c
)

target_include_directories(audio_pwm PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(audio_pwm
    pico_stdlib
    hardware_adc
    hardware_pwm
    hardware_gpio
    hardware_timer
)

# Biblioteca para interface (botões, LEDs, display)
add_library(interface STATIC
    src/ssd1306_i2c.c
    src/buttons.c
    src/led_rgb.c
)

target_link_libraries(interface
    pico_stdlib
    hardware_gpio
    hardware_i2c
    pico_time
)

# -----
# Executável principal
# -----
add_executable(audio_synth
    src/main.c
)

target_include_directories(audio_synth PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/include
)

# Bibliotecas para o executável principal
if(ENABLE_WIRELESS AND (${PICO_BOARD} STREQUAL "pico_w"))
    # Configuração com suporte wireless (para Pico W)
    target_link_libraries(audio_synth
        pico_stdlib
        hardware_adc
        hardware_pwm
        hardware_dma
        hardware_gpio
        hardware_timer
        hardware_i2c
        hardware_irq
        pico_cyw43_arch_none  # Adiciona suporte wireless básico (sem networking)
        m                     # Biblioteca matemática
        audio_pwm
        interface
    )
    
    message(STATUS "Compilando com suporte wireless para Pico W")
else()
    # Configuração padrão (sem wireless)
    target_link_libraries(audio_synth
        pico_stdlib
        hardware_adc
        hardware_pwm
        hardware_dma
        hardware_gpio
        hardware_timer
        hardware_i2c
        hardware_irq
        m                     # Biblioteca matemática
        audio_pwm
        interface
    )
    
    message(STATUS "Compilando versão padrão sem componentes wireless")
endif()

# Configura a saída via USB e UART
pico_enable_stdio_usb(audio_synth 1)
pico_enable_stdio_uart(audio_synth 1)

# Gera arquivos extras como .uf2
pico_add_extra_outputs(audio_synth)

# -----
# Instruções para compilação
# -----
# Para compilar com suporte wireless (apenas para Pico W):
#   cmake -DENABLE_WIRELESS=ON -DPICO_BOARD=pico_w ..
#
# Para compilar a versão padrão (sem wireless):
#   cmake .. 