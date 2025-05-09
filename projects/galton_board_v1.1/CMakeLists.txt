cmake_minimum_required(VERSION 3.13)

# -----------------------------------------------------------------------------
# Configuração do SDK da Raspberry Pi Pico
# -----------------------------------------------------------------------------
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

# -----------------------------------------------------------------------------
# Configuração da placa 
# -----------------------------------------------------------------------------
# Por padrão, use a Pico padrão para evitar problemas de compilação em Windows
# Para compilar com Wireless (Pico W), descomente a linha abaixo e comente a linha anterior
set(PICO_BOARD pico)
#set(PICO_BOARD pico_w)

# Opções para configuração do projeto
option(ENABLE_WIRELESS "Habilitar componentes wireless (pode causar erros em Windows)" ON)

# Importa o SDK da Pico
include(pico_sdk_import.cmake)

# Define o nome e os idiomas do projeto
project(galton_board C CXX ASM)

# Inicializa o SDK
pico_sdk_init()

# -----------------------------------------------------------------------------
# Bibliotecas do projeto
# -----------------------------------------------------------------------------
# Biblioteca simplificada do display SSD1306
add_library(ssd1306_i2c
    include/ssd1306_i2c.c
)

target_include_directories(ssd1306_i2c PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(ssd1306_i2c
    pico_stdlib
    hardware_i2c
)

# -----------------------------------------------------------------------------
# Executável principal
# -----------------------------------------------------------------------------
add_executable(galton_board
    src/main.c
    include/galton.c
)

target_include_directories(galton_board PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/include
)

# Bibliotecas para o executável principal
if(ENABLE_WIRELESS AND (${PICO_BOARD} STREQUAL "pico_w"))
    # Configuração com suporte wireless (para Pico W)
    target_link_libraries(galton_board
        pico_stdlib
        hardware_i2c
        hardware_adc
        pico_cyw43_arch_none  # Adiciona suporte wireless básico (sem networking)
        m                     # Biblioteca matemática
        ssd1306_i2c
    )
    
    message(STATUS "Compilando com suporte wireless para Pico W")
else()
    # Configuração padrão (sem wireless)
    target_link_libraries(galton_board
        pico_stdlib
        hardware_i2c
        hardware_adc
        m                     # Biblioteca matemática
        ssd1306_i2c
    )
    
    message(STATUS "Compilando versão padrão sem componentes wireless")
endif()

# Configura a saída via USB e UART
pico_enable_stdio_usb(galton_board 1)
pico_enable_stdio_uart(galton_board 1)

# Gera arquivos extras como .uf2
pico_add_extra_outputs(galton_board)

# -----------------------------------------------------------------------------
# Instruções para compilação
# -----------------------------------------------------------------------------
# Para compilar com suporte wireless (apenas para Pico W):
#   cmake -DENABLE_WIRELESS=ON -DPICO_BOARD=pico_w ..
#
# Para compilar a versão padrão (sem wireless):
#   cmake .. 