Projetos de Sistemas Embarcados - EmbarcaTech 2025

# 📂 Projeto Galton Board v1.1

Autor: Jorge Wilker Mamede de Andrade

Curso: Residência Tecnológica em Sistemas Embarcados

Instituição: EmbarcaTech - HBr

Campinas, 09 de maio 2025

## 🎯 Objetivo
Este projeto simula um Galton Board usando Raspberry Pi Pico, demonstrando distribuição binomial com visualização em display OLED.

## 🔧 Componentes Usados
- Raspberry Pi Pico
- Display OLED SSD1306 (128x64 pixels, via I2C)
- Dois botões para controle (GPIO 5 e 6)

## ⚡ Pinagem dos Dispositivos
| Pino | Função          |
|------|-----------------|
| GPIO 5 | Botão Iniciar   |
| GPIO 6 | Botão Reiniciar |
| I2C SDA (GPIO 14) | Display OLED    |
| I2C SCL (GPIO 15) | Display OLED    |

## 💾 Como Compilar e Executar
- Certifique-se de que o SDK do Pico está instalado (defina PICO_SDK_PATH).
- Execute: `mkdir build && cd build && cmake .. && cmake --build .` para compilar.
- O arquivo .uf2 será gerado em `build/`; carregue-o no Pico no modo BOOTSEL.

## 📈 Resultados Esperados
Simulação visual de bolas caindo, histograma em tempo real e controle via botões.

## 📂 Arquivos
- `src/`: Contém código-fonte principal (ex: galton.c, main.c, ssd1306_i2c.h).
- `CMakeLists.txt`: Configura a compilação com CMake.
- `build/`: Diretório para arquivos gerados (adicionado ao .gitignore).

## 📜 Licença
Este projeto é licenciado sob GPL-3.0. 