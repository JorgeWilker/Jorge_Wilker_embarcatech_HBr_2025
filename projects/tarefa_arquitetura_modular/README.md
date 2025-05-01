# Tarefa Arquitetura Modular - Controle de LED Embutido

🎯 Objetivo
Este projeto demonstra a implementação de uma arquitetura de software modular (Aplicação, Driver, HAL - Hardware Abstraction Layer) para controlar o LED embutido de uma placa Raspberry Pi Pico W. O objetivo é piscar o LED embutido como exemplo prático dessa arquitetura.

🔧 Componentes Usados
*   Raspberry Pi Pico W (utilizando seu LED embutido)

⚡ Pinagem dos Dispositivos
*   **LED Embutido (Pico W):** Controlado internamente através do chip CYW43 (geralmente usado para WiFi/Bluetooth), acessado via funções específicas do SDK (`pico_cyw43_arch_none`). Não utiliza um pino GPIO direto padrão para controle.

💾 Como Compilar e Executar
1.  **Configure o ambiente:** Certifique-se de ter o SDK do Raspberry Pi Pico e as ferramentas de compilação (CMake, GCC ARM Compiler, etc.) instaladas e configuradas corretamente no seu sistema.
2.  **Abra o projeto:** Utilize um ambiente de desenvolvimento como o VS Code com as extensões para Pico configuradas.
3.  **Compile:**
    *   Crie um diretório `build`: `mkdir build`
    *   Navegue até ele: `cd build`
    *   Execute o CMake: `cmake ..`
    *   Compile o projeto: `make`
4.  **Transfira para o Pico W:**
    *   Conecte o Pico W ao computador via USB enquanto segura o botão `BOOTSEL`.
    *   O Pico W aparecerá como um dispositivo de armazenamento USB (RPI-RP2).
    *   Copie o arquivo `.uf2` gerado (localizado na pasta `build`, com o nome `tarefa_arquitetura_modular.uf2`) para dentro dessa unidade de armazenamento.
5.  **Execução:** A placa irá reiniciar automaticamente e começar a executar o código.

📈 Resultados Esperados ou Observados
Ao executar o projeto, espera-se que:
*   O LED embutido na placa Raspberry Pi Pico W comece a piscar em intervalos regulares (ligando e desligando). A frequência do piscar é definida no código da aplicação (`app/main.c`).

📂 Arquivos
*   `app/main.c`: Contém a lógica principal da aplicação, definindo como e quando o LED deve piscar.
*   `drivers/led_embutido.c`: Implementação do driver de alto nível para o controle do LED.
*   `hal/hal_led.c`: Implementação da Hardware Abstraction Layer (HAL), interagindo diretamente com as funções do SDK para controlar o hardware do LED.
*   `include/led_embutido.h`: Arquivo de cabeçalho para o driver do LED.
*   `include/hal_led.h`: Arquivo de cabeçalho para a HAL do LED.
*   `CMakeLists.txt`: Arquivo de configuração do projeto para o CMake.
*   `pico_sdk_import.cmake`: Script CMake para importar o SDK do Pico.

📜 Licença
GPL-3.0 License