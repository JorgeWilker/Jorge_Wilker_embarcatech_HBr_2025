# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

# Definições de padrão para C e C++
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Inicializa o pico_sdk
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

# Configura placa
set(PICO_BOARD pico CACHE STRING "Board type")

# Importa SDK do Pico
include(pico_sdk_import.cmake)

project(pico_temp_unity_test C CXX ASM)

# Inicializa o SDK do Pico
pico_sdk_init()

# Força unity.c a ser compilado como C (evita erro de typedef float)
set_source_files_properties(src/unity.c PROPERTIES LANGUAGE C)

# Define o executável de teste
add_executable(temperature_test_runner
    src/temperature.c
    src/unity.c
    test/test_temperature.c
)

# Linka bibliotecas necessárias
target_link_libraries(temperature_test_runner PRIVATE
    pico_stdlib
    m  # math.h (para roundf, etc.)
)

# Define UNITY_INCLUDE_CONFIG_H para permitir configurações personalizadas
# e UNITY_EXCLUDE_STDINT_H para evitar conflito com Pico SDK
target_compile_definitions(temperature_test_runner PRIVATE 
    UNITY_INCLUDE_CONFIG_H
    UNITY_EXCLUDE_STDINT_H
)

# Diretórios com headers (temperature.h, unity.h, etc.)
target_include_directories(temperature_test_runner PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

# Ativa stdio para saída via USB/UART
pico_enable_stdio_usb(temperature_test_runner 1)
pico_enable_stdio_uart(temperature_test_runner 1)

# Gera .uf2 e outros artefatos
pico_add_extra_outputs(temperature_test_runner)
