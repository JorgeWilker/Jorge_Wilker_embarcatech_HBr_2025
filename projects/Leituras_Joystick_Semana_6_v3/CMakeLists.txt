# Arquivo CMake gerado para o projeto Pico

cmake_minimum_required(VERSION 3.13)

# Define os padrões de C e C++
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Inicializa o SDK do Pico a partir da localização instalada
# (note que isso pode vir de variáveis de ambiente, cache do CMake, etc.)

# == NÃO EDITE AS LINHAS A SEGUIR para que a extensão do VS Code funcione ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})  # Define o diretório do usuário no Windows
else()
    set(USERHOME $ENV{HOME})          # Define o diretório do usuário em outros sistemas
endif()
set(sdkVersion 2.1.1)                 # Versão do SDK
set(toolchainVersion 14_2_Rel1)       # Versão da toolchain
set(picotoolVersion 2.1.1)            # Versão do picotool
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake) # Caminho para o arquivo de configuração do VS Code
if (EXISTS ${picoVscode})
    include(${picoVscode})            # Inclui o arquivo de configuração se existir
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Tipo de placa")  # Define o tipo de placa

# Importa o SDK do Raspberry Pi Pico (deve ser antes do projeto)
include(pico_sdk_import.cmake)

# Define o nome do projeto e os tipos de linguagem
project(Leituras_Joystick_Semana_6_v3 C CXX ASM)

# Inicializa o SDK do Raspberry Pi Pico
pico_sdk_init()

# Adiciona o executável. O nome padrão é o nome do projeto, versão 0.1
add_executable(Leituras_Joystick_Semana_6_v3 src/main.c src/ssd1306.c)

# Define o nome e a versão do programa
pico_set_program_name(Leituras_Joystick_Semana_6_v3 "Leituras_Joystick_Semana_6_v3")
pico_set_program_version(Leituras_Joystick_Semana_6_v3 "0.1")

# Modifique as linhas abaixo para habilitar/desabilitar a saída via UART/USB
pico_enable_stdio_uart(Leituras_Joystick_Semana_6_v3 0)  # Desabilita saída UART
pico_enable_stdio_usb(Leituras_Joystick_Semana_6_v3 0)   # Desabilita saída USB

# Adiciona a biblioteca padrão ao build
target_link_libraries(Leituras_Joystick_Semana_6_v3
        pico_stdlib
        hardware_i2c
        hardware_adc
)

# Adiciona os diretórios de include padrão ao build
target_include_directories(Leituras_Joystick_Semana_6_v3 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}  # Diretório atual
        ${PICO_SDK_PATH}/src/common/pico_stdlib/include  # Diretório da biblioteca padrão
        ${PICO_SDK_PATH}/src/rp2_common/hardware_i2c/include  # Diretório da biblioteca I2C
        ${PICO_SDK_PATH}/src/rp2_common/hardware_adc/include  # Diretório da biblioteca ADC
)

# Adiciona quaisquer bibliotecas solicitadas pelo usuário
# target_link_libraries(Leituras_Joystick_Semana_6_v3) 

# Adiciona saídas extras para o executável
pico_add_extra_outputs(Leituras_Joystick_Semana_6_v3)

