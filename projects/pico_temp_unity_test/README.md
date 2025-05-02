# Projeto Pico - Teste de Sensor de Temperatura com Unity

🎯 Objetivo
Este projeto utiliza um Raspberry Pi Pico para ler a temperatura de um sensor (simulado ou real) e usa o framework Unity para testes unitários da lógica de conversão de temperatura.

🔧 Componentes Usados
*   Raspberry Pi Pico (utilizando seu conversor ADC interno ou sensor externo)
*   Framework Unity para testes unitários em C

⚡ Estrutura do Projeto
*   **Código Fonte Principal (`src/`):** Contém a implementação da lógica de temperatura e o framework Unity.
*   **Testes Unitários (`test/`):** Contém os testes para validar a funcionalidade do sensor de temperatura.

💾 Como Compilar e Executar
1.  **Configure o ambiente:** Certifique-se de ter o SDK do Raspberry Pi Pico e as ferramentas de compilação (CMake, GCC ARM Compiler, etc.) instaladas e configuradas corretamente no seu sistema.
2.  **Abra o projeto:** Utilize um ambiente de desenvolvimento como o VS Code com as extensões para Pico configuradas.
3.  **Compile:**
    *   Crie um diretório `build`: `mkdir build`
    *   Navegue até ele: `cd build`
    *   Execute o CMake: `cmake -G "Ninja" ..`
    *   Compile o projeto: `cmake --build .`
4.  **Transfira para o Pico:**
    *   Conecte o Pico ao computador via USB enquanto segura o botão `BOOTSEL`.
    *   O Pico aparecerá como um dispositivo de armazenamento USB (RPI-RP2).
    *   Copie o arquivo `.uf2` gerado para dentro dessa unidade de armazenamento.
5.  **Execução:** A placa irá reiniciar automaticamente e começar a executar os testes.

📈 Problemas Encontrados na Compilação
*   **Erros de Tipo:** Durante o desenvolvimento e configuração deste projeto, foram encontrados erros persistentes de compilação relacionados a tipos conflitantes (`conflicting types`) para `int32_t`.
*   **Tentativas de Solução:**
    *   Ajustes no arquivo `src/unity_config.h` para evitar a inclusão de `<stdint.h>`.
    *   Adição da flag `UNITY_EXCLUDE_STDINT_H` diretamente no `CMakeLists.txt`.
*   **Resultado:** Apesar dessas tentativas, o erro de compilação persistiu, indicando um conflito mais profundo entre as definições de tipo do Pico SDK e as definições internas ou incluídas pelo framework Unity.
*   **Recomendação:** Consulte o professor ou um colega mais experiente para investigar e corrigir este erro de compilação antes de prosseguir.

📂 Arquivos
*   `src/temperature.c`: Implementação da leitura/conversão de temperatura.
*   `src/temperature.h`: Header para a lógica de temperatura.
*   `src/unity.c`: Código fonte do framework Unity.
*   `src/unity.h`: Header principal do Unity.
*   `src/unity_config.h`: Arquivo de configuração customizado do Unity.
*   `src/unity_internals.h`: Arquivo interno do Unity.
*   `test/test_temperature.c`: Testes unitários para temperature.c usando Unity.
*   `CMakeLists.txt`: Arquivo de configuração do projeto para o CMake.
*   `pico_sdk_import.cmake`: Script CMake para importar o SDK do Pico.

📜 Licença
GPL-3.0 License