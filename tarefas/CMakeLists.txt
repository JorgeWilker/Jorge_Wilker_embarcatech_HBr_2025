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
add_executable(tarefa_motor_dc_bitdoglab
    src/main.c
    include/ssd1306_i2c.c
    include/tb6612fng.c
)

pico_set_program_name(tarefa_motor_dc_bitdoglab "tarefa_motor_dc_bitdoglab")
pico_set_program_version(tarefa_motor_dc_bitdoglab "0.1")

# Configuração de E/S - ajuste conforme necessidade
pico_enable_stdio_uart(tarefa_motor_dc_bitdoglab 1)
pico_enable_stdio_usb(tarefa_motor_dc_bitdoglab 1)

# Inclui diretórios
target_include_directories(tarefa_motor_dc_bitdoglab PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include
)

# Bibliotecas para o executável principal - Configuração padrão (sem wireless)
target_link_libraries(tarefa_motor_dc_bitdoglab
    pico_stdlib
    hardware_i2c        # Necessário para comunicação I2C com display
    hardware_pwm        # Necessário para controle PWM dos motores TB6612FNG
    pico_binary_info    # Para informações binárias
    m                   # Biblioteca matemática
)

# Gera arquivos adicionais (.uf2, etc)
pico_add_extra_outputs(tarefa_motor_dc_bitdoglab) 