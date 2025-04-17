# Leituras Joystick - Semana 6 v3

## Objetivo

Este projeto tem como objetivo ler os valores do joystick da placa BitDogLab (com Raspberry Pi Pico) e exibir as leituras dos eixos X e Y em um display OLED. O projeto demonstra a interação com periféricos analógicos (joystick) e digitais (display OLED).

## Componentes Usados

*   **BitDogLab com (Raspberry Pi Pico)**
*   **Joystick Analógico** (conectado aos pinos ADC GPIO27 e GPIO26)
*   **Display OLED I2C SSD1306** (endereço I2C `0x3C`, conectado aos pinos GPIO14 - SDA e GPIO15 - SCL)

## Pinagem dos Dispositivos

| Componente            | Conexão na BitDogLab      |
| :-------------------- | :------------------------ |
| Joystick (Eixo X)     | ADC1 (GPIO 27)            |
| Joystick (Eixo Y)     | ADC0 (GPIO 26)            |
| Display OLED (SDA)    | GPIO 14                   |
| Display OLED (SCL)    | GPIO 15                   |


## Como Compilar e Executar

1.  **Configure o ambiente de desenvolvimento para Raspberry Pi Pico.** Isso envolve a instalação do SDK do Raspberry Pi Pico e das ferramentas de compilação (CMake, compilador ARM).
2.  **Abra o projeto no seu ambiente de desenvolvimento** (ex: VS Code com as extensões apropriadas).
3.  **Compile o código-fonte** (`main.c` e quaisquer outros arquivos necessários) utilizando o processo de build do seu ambiente (ex: `cmake .` seguido de `make`).
4.  **Conecte sua BitDogLab** ao computador via cabo USB enquanto mantém pressionado o botão `BOOTSEL` para colocá-la no modo de bootloader.
5.  **Copie o arquivo `.uf2` gerado** (encontrado na pasta `build`) para a unidade de armazenamento removível que aparecerá no seu sistema (chamada `RPI-RP2`).
6.  **A BitDogLab irá reiniciar automaticamente** e começar a executar o código.
7.  **As leituras dos eixos X e Y do joystick serão exibidas** no display OLED. Você pode observar os valores mudando ao mover o joystick.
8.  **(Opcional)** A inicialização do sistema e os valores brutos do ADC podem ser impressos no terminal serial se estiver conectado.

## Resultados Esperados ou Observados

Ao executar o projeto, espera-se que:

*   O display OLED seja inicializado e limpo.
*   Os valores de leitura do eixo X do joystick sejam exibidos na parte superior do display OLED, precedidos por "X:".
*   Os valores de leitura do eixo Y do joystick sejam exibidos na parte inferior do display OLED, precedidos por "Y:".
*   Ao mover o joystick, os valores numéricos exibidos no OLED devem mudar em tempo real, refletindo a posição do joystick.

## Arquivos

*   `main.c`: Contém o código principal do projeto para leitura do joystick e controle do display OLED. 
*   `ssd1306.h` / `ssd1306.c`: Arquivos para a biblioteca de controle do display OLED SSD1306.
*   `font.h`: Arquivo contendo a definição da fonte a ser utilizada no display OLED.
*   `CMakeLists.txt`: Arquivo de configuração para o sistema de build CMake.

## Licença

Este projeto está licenciado sob a **MIT License**.

```
MIT License

[Leia mais sobre a licença aqui ](https://pt.wikipedia.org/wiki/Licen%C3%A7a_MIT#:~:text=A%20licen%C3%A7a%20MIT%2C%20tamb%C3%A9m%20chamada,livre%20quanto%20em%20software%20propriet%C3%A1rio.)