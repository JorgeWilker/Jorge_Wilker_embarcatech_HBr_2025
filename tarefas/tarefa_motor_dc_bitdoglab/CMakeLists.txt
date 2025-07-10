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
option(ENABLE_WIRELESS "Habilitar componentes wireless (pode causar erros em Windows)" OFF)

# Importa o SDK da Pico
include(pico_sdk_import.cmake)

# Define o nome e os idiomas do projeto
project(tarefa_motor_dc_bitdoglab C CXX ASM)

# Inicializa o SDK
pico_sdk_init()

# -----------------------------------------------------------------------------
# Executável principal
# -----------------------------------------------------------------------------
add_executable(tarefa_motor_dc_bitdoglab
    src/main.c
    include/ssd1306_i2c.c
    include/tb6612fng.c
)

target_include_directories(tarefa_motor_dc_bitdoglab PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include
)

# Bibliotecas para o executável principal - Configuração padrão (sem wireless)
target_link_libraries(tarefa_motor_dc_bitdoglab
    pico_stdlib
    hardware_i2c        # Necessário para comunicação I2C com MPU6050
    hardware_pwm        # Necessário para controle PWM dos motores TB6612FNG
    pico_binary_info    # Para informações binárias
    m                   # Biblioteca matemática para cálculos de temperatura
)

message(STATUS "Compilando projeto controle de motores DC - versão padrão sem componentes wireless")

# Configura a saída via USB e UART
pico_enable_stdio_usb(tarefa_motor_dc_bitdoglab 1)
pico_enable_stdio_uart(tarefa_motor_dc_bitdoglab 1)

# Gera arquivos extras como .uf2
pico_add_extra_outputs(tarefa_motor_dc_bitdoglab)

# -----------------------------------------------------------------------------
# Instruções para compilação
# -----------------------------------------------------------------------------
# Para compilar o projeto:
#   mkdir build
#   cd build
#   cmake ..
#   cmake --build .
#
# O arquivo .uf2 será gerado em build/tarefa_motor_dc_bitdoglab.uf2
# Transfira este arquivo para o Raspberry Pi Pico em modo BOOTSEL