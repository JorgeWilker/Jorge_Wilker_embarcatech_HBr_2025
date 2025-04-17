# Contador_descescente_Semana_6_v4

## 🎯 Objetivo do projeto

Este projeto implementa um contador decrescente visualizado em um display OLED, com interações através de dois botões: um para iniciar/reiniciar a contagem e outro para registrar cliques durante a contagem.

## 🔧 Componentes usados

*   Placa de desenvolvimento BitDogLab com Raspberry Pi Pico RP2040.
*   Display OLED SSD1306 (conectado via I2C) (já incluídos na BitDogLab).
*   Dois botões (conectados aos pinos GPIO 5 e 6) (já incluídos na BitDogLab).


## 💾 Como compilar e executar o código

1.  Certifique-se de ter o ambiente de desenvolvimento configurado para Raspberry Pi Pico (com CMake e compilador ARM).
2.  Salve o código fornecido em um arquivo `.c` (por exemplo, `main.c`).
3.  Crie um arquivo `CMakeLists.txt` para configurar o processo de build.
4.  Compile o projeto usando o CMake e o comando `make` no terminal. Isso irá gerar o arquivo `.uf2`. Ou use o SDK da Raspberry PI Pico.
5.  Conecte sua placa Pico ao computador via cabo USB enquanto mantém pressionado o botão BOOTSEL para colocá-la no modo de boot.
6.  Uma unidade de armazenamento removível chamada `RPI-RP2` deverá aparecer. Copie o arquivo `.uf2` para esta unidade.
7.  A placa Pico irá reiniciar automaticamente e começar a executar o código.

## ⚡ Pinagem dos dispositivos utilizados

| Componente        | Pino da BitDogLab (ou RP2040) |
| :---------------- | :--------------------------- |
| Botão A           | GPIO 5                       |
| Botão B           | GPIO 6                       |
| Display OLED SDA  | GPIO 14                      |
| Display OLED SCL  | GPIO 15                      |

**Observação:** A biblioteca `ssd1306.h` utiliza a interface I2C. As configurações do I2C são feitas para a porta `i2c1` com os pinos SDA em GPIO 14 e SCL em GPIO 15.

## 📈 Resultados esperados ou obtidos

Ao executar o código, espera-se que:

*   O display OLED inicialmente mostre a mensagem "Pressione A".
*   Ao pressionar o Botão A, um contador decrescente de 9 a 0 será iniciado no display OLED, com atualizações a cada segundo.
*   Durante a contagem, cada vez que o Botão B for pressionado, um contador de cliques será incrementado e exibido no display OLED.
*   Ao final da contagem (quando o contador chega a 0), a contagem para e o display mostra o valor final do contador e o número de cliques do Botão B.
*   Pressionar o Botão A novamente reinicia a contagem e o contador de cliques.
*   Pressionar o Botão B fora do período de contagem não terá efeito no contador.
*   Informações de depuração sobre o estado do contador e dos botões são impressas no terminal serial.

## 📜 Licença

MIT License

[Leia mais sobre a licença aqui ](https://pt.wikipedia.org/wiki/Licen%C3%A7a_MIT#:~:text=A%20licen%C3%A7a%20MIT%2C%20tamb%C3%A9m%20chamada,livre%20quanto%20em%20software%20propriet%C3%A1rio.)