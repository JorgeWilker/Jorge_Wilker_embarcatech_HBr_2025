# Arquivo CMake gerado para o projeto Pico

cmake_minimum_required(VERSION 3.13)  # Define a versão mínima do CMake necessária

set(CMAKE_C_STANDARD 11)  # Define o padrão C para o projeto
set(CMAKE_CXX_STANDARD 17)  # Define o padrão C++ para o projeto
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # Exporta os comandos de compilação

# Inicializa o SDK do Pico a partir da localização instalada
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})  # Define o diretório do usuário no Windows
else()
    set(USERHOME $ENV{HOME})  # Define o diretório do usuário em outros sistemas
endif()
set(sdkVersion 2.1.1)  # Versão do SDK
set(toolchainVersion 14_2_Rel1)  # Versão da toolchain
set(picotoolVersion 2.1.1)  # Versão da ferramenta Pico
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)  # Caminho para o arquivo de configuração do VS Code
if (EXISTS ${picoVscode})
    include(${picoVscode})  # Inclui o arquivo de configuração do VS Code se existir
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Tipo de placa")  # Define o tipo de placa como pico_w

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Ler_temp_interna_semana_6_v1 C CXX ASM)  # Define o nome do projeto e as linguagens utilizadas

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Ler_temp_interna_semana_6_v1 "C:/Users/Jorge Wilker Tech/Desktop/EmbarcaTech/Jorge_Wilker_embarcatech_HBr_2025/projects/Ler_temp_interna_semana_6/src/main.c")

pico_set_program_name(Ler_temp_interna_semana_6_v1 "Ler_temp_interna_semana_6_v1")  # Define o nome do programa
pico_set_program_version(Ler_temp_interna_semana_6_v1 "0.1")  # Define a versão do programa

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Ler_temp_interna_semana_6_v1 0)  # Desabilita a saída UART
pico_enable_stdio_usb(Ler_temp_interna_semana_6_v1 1)  # Habilita a saída USB

# Add the standard library to the build
target_link_libraries(Ler_temp_interna_semana_6_v1
        pico_stdlib  # Biblioteca padrão do Pico
        hardware_adc)  # Biblioteca do ADC

# Add the standard include files to the build
target_include_directories(Ler_temp_interna_semana_6_v1 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}  # Diretório atual do CMake
)

# Add any user requested libraries
target_link_libraries(Ler_temp_interna_semana_6_v1 
        # Adicione aqui outras bibliotecas, se necessário
)

pico_add_extra_outputs(Ler_temp_interna_semana_6_v1)  # Adiciona saídas extras para o projeto

