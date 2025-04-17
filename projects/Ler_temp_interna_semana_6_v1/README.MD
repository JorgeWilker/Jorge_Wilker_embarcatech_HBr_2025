# Ler_temp_interna_semana_6_v1

Este projeto implementa a leitura da **temperatura interna do microcontrolador Raspberry Pi Pico RP2040**, utilizando a placa de desenvolvimento **BitDogLab**. A temperatura é exibida via saída serial.

## 🎯 Objetivo do projeto

O objetivo principal deste projeto é demonstrar como **ler e exibir a temperatura interna do Raspberry Pi Pico RP2040** utilizando o sensor ADC integrado.

## 🔧 Componentes usados

*   **Placa de desenvolvimento BitDogLab** com Raspberry Pi Pico RP2040.
*   **Sensor de temperatura interno** do Raspberry Pi Pico RP2040.
*   **Interface serial** para saída dos dados de temperatura.

## 💾 Como compilar e executar o código

1.  Certifique-se de ter o **ambiente de desenvolvimento configurado para Raspberry Pi Pico** (com CMake e compilador ARM).
2.  Salve o código fornecido em um arquivo `.c` (por exemplo, `main.c`).
3.  Crie um arquivo `CMakeLists.txt` para configurar o processo de build.
4.  **Compile o projeto** usando o CMake e o comando `make` no terminal. Isso irá gerar o arquivo `.uf2`. Ou use o SDK da Raspberry Pi Pico.
5.  **Conecte sua placa Pico ao computador** via cabo USB enquanto mantém pressionado o botão BOOTSEL para colocá-la no modo de boot.
6.  Uma unidade de armazenamento removível chamada `RPI-RP2` deverá aparecer. **Copie o arquivo `.uf2` para esta unidade**.
7.  A placa Pico irá reiniciar automaticamente e começar a executar o código.
8.  **Abra um terminal serial** (com a velocidade configurada corretamente, 115200 bps) para visualizar a saída da temperatura.

## ⚡ Pinagem dos dispositivos utilizados

Neste projeto, **não há pinagem externa específica** sendo utilizada para o sensor de temperatura, pois estamos utilizando o sensor interno do Raspberry Pi Pico. A saída da temperatura é feita através da interface serial (USB).

## 📈 Resultados esperados ou obtidos

Ao executar o código, espera-se que a **temperatura interna do Raspberry Pi Pico seja lida a cada segundo e impressa no terminal serial**, no formato "Temperatura interna: XX.XX °C".

## 💾 Arquivos

*   `main.c`: Contém o **código principal** do projeto para leitura e exibição da temperatura.

## 📜 Licença

GPL-3.0 License

[Leia mais sobre a licença aqui ](https://www.gnu.org/licenses/#GPL)