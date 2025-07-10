Projetos de Sistemas Embarcados - EmbarcaTech 2025
# 📂 Projeto Controle de Motores DC com IMU v2.0
**Autor**: Jorge Wilker Mamede de Andrade  
**Curso**: Residência Tecnológica em Sistemas Embarcados  
**Instituição**: EmbarcaTech - HBr  
**Campinas, Janeiro de 2025**

## 🎯 Objetivo
Este projeto implementa um sistema inteligente de controle de motores DC baseado em dados inerciais do sensor MPU-6050 usando Raspberry Pi Pico. O sistema utiliza o driver TB6612FNG para controlar dois motores independentes, respondendo aos movimentos detectados pelo acelerômetro e giroscópio. Os dados são exibidos em tempo real no terminal serial e no display OLED SSD1306, junto com o status dos motores.

## 🔧 Componentes Usados
- **Raspberry Pi Pico** (microcontrolador principal)
- **MPU-6050** (sensor acelerômetro/giroscópio de 6 eixos)
- **Display OLED SSD1306** (128x64 pixels, comunicação I2C)
- **Driver TB6612FNG** (controlador de motores DC com ponte H dupla)
- **2x Motores DC** (motores de corrente contínua para controle)
- **Fonte de alimentação 5V** (para alimentação dos motores via VM)
- **Cabos jumper** para conexões I2C e controle de motores


## ⚡ Pinagem dos Dispositivos (BitDogLab)

### MPU-6050 (Sensor de Movimento)
| BitDogLab/Pico | MPU-6050 | Função |
|----------------|----------|---------|
| GPIO 0 | SDA | Comunicação I2C0 - Dados |
| GPIO 1 | SCL | Comunicação I2C0 - Clock |
| 3V3 | VCC | Alimentação 3.3V |
| GND | GND | Terra |

### OLED SSD1306 (Display)
| BitDogLab/Pico | OLED SSD1306 | Função |
|----------------|--------------|---------|
| GPIO 14 | SDA | Comunicação I2C1 - Dados |
| GPIO 15 | SCL | Comunicação I2C1 - Clock |
| 3V3 | VCC | Alimentação 3.3V |
| GND | GND | Terra |

### TB6612FNG (Driver de Motores)
| BitDogLab/Pico | TB6612FNG | Função |
|----------------|-----------|---------|
| GPIO 4 | INA1 | Controle direção Motor A |
| GPIO 8 | PWMA | Controle velocidade Motor A (PWM) |
| GPIO 9 | INA2 | Controle direção Motor A |
| GPIO 16 | PWMB | Controle velocidade Motor B (PWM) |
| GPIO 18 | INB1 | Controle direção Motor B |
| GPIO 19 | INB2 | Controle direção Motor B |
| GPIO 20 | STBY | Standby (habilita/desabilita driver) |
| 3V3 | VCC | Alimentação lógica 3.3V |
| 5V | VM | Alimentação motores 5V |
| GND | GND | Terra |

**Observações importantes:**
- ⚠️ **Configuração específica para BitDogLab**: Sistema multi-periférico
  - I2C0 (GPIO 0/1) para MPU-6050
  - I2C1 (GPIO 14/15) para OLED SSD1306
  - PWM/GPIO para controle de motores TB6612FNG
- Sensores alimentados com 3.3V, **motores** com 5V
- Frequência PWM: 1kHz para controle suave dos motores
- As conexões I2C já possuem pull-up interno habilitado no código
- Endereços I2C: MPU-6050 (0x68), OLED SSD1306 (0x3C)

## 💾 Como Compilar e Executar

### Pré-requisitos
- Raspberry Pi Pico SDK instalado
- CMake (versão 3.13 ou superior)
- GCC ARM Embedded Toolchain
- Ambiente de desenvolvimento configurado para Raspberry Pi Pico

### Passos para Compilação
1. **Criar diretório de build:**
   ```powershell
   mkdir -Force "C:\tarefa_motor_dc_bitdoglab\build"
   cd "C:\tarefa_motor_dc_bitdoglab\build"
   ```

2. **Configurar o projeto com CMake (recomendado usar Ninja):**
   ```powershell
   cmake -G Ninja ..
   ```

3. **Compilar o projeto:**
   ```powershell
   ninja
   ```

4. **Carregar no Raspberry Pi Pico:**
   - Conecte o Raspberry Pi Pico ao computador mantendo o botão BOOTSEL pressionado
   - Identifique a letra do drive (ex: D:, E:, F:)
   - Copie o arquivo usando PowerShell com caminho absoluto:
   ```powershell
   Copy-Item -Path "C:\tarefa_motor_dc_bitdoglab\build\tarefa_motor_dc_bitdoglab.uf2" -Destination "D:\tarefa_motor_dc_bitdoglab.uf2" -Force
   ```
   - O Pico reiniciará automaticamente e executará o programa

### Visualização dos Dados e Controle dos Motores

#### Terminal Serial
- Abra um terminal serial (ex: PuTTY, Arduino IDE Serial Monitor)
- Configure para 115200 baud rate
- Conecte à porta COM do Raspberry Pi Pico
- Os dados serão exibidos a cada 0.5 segundos no formato:
  ```
  === LEITURA MPU-6050 ===
  Acelerômetro:
    X =   valor  Y =   valor  Z =   valor
  Giroscópio:
    X =   valor  Y =   valor  Z =   valor
  ========================
  Motores: A[FWD:80%] B[FWD:80%]
  ```

#### Display OLED SSD1306
- Os dados são exibidos simultaneamente no display OLED 128x64 pixels
- Layout organizado: dados do acelerômetro, giroscópio e status dos motores
- **Nova seção de status**: MOTORS (READY/OFF) e atividade (ACTIVE/IDLE)
- Indicação visual quando movimento é detectado e motores estão ativos
- Atualização automática a cada 0.5 segundos
- Interface visual clara para monitoramento em tempo real

#### Lógica de Controle dos Motores
- **Detecção de movimento**: Baseada em limites configuráveis do acelerômetro e giroscópio
- **Controle direcional**:
  - Aceleração Y > limiar: Movimento frontal/traseiro (ambos motores mesma direção)
  - Giroscópio Z > limiar: Rotação (motores em direções opostas)
  - Aceleração X: Ajuste de curva (reduz velocidade de um motor)
- **Velocidade proporcional**: Baseada na magnitude do movimento detectado
- **Limites de segurança**: Velocidade mínima (30%) e máxima (80%) configuráveis

## 📈 Resultados Esperados
- **Acelerômetro**: Valores próximos a ±16384 para 1g nos eixos correspondentes
- **Giroscópio**: Valores próximos a zero quando em repouso
- **Controle de motores**: Resposta automática aos movimentos detectados
- **Movimento frontal/traseiro**: Ambos motores giram na mesma direção
- **Rotação**: Motores giram em direções opostas para criar torque
- **Parada automática**: Motores param quando não há movimento significativo detectado
- **Frequência de atualização**: 2Hz (atualização a cada 0.5 segundos para controle responsivo)

## 📂 Arquivos
```
tarefa_motor_dc_bitdoglab/
├── build/                       # Arquivos de compilação (gerado)
│   └── tarefa_motor_dc_bitdoglab.uf2  # Arquivo compilado para upload
├── include/                     # Bibliotecas dos periféricos
│   ├── ssd1306.h                # Definições do display SSD1306
│   ├── ssd1306_font.h           # Fonte de caracteres para OLED
│   ├── ssd1306_i2c.c            # Implementação I2C do SSD1306
│   ├── ssd1306_i2c.h            # Interface I2C do SSD1306
│   ├── tb6612fng.h              # Interface do driver TB6612FNG
│   └── tb6612fng.c              # Implementação do driver TB6612FNG
├── src/                         # Código principal
│   └── main.c                   # Programa principal integrado
├── CMakeLists.txt               # Configuração de build do projeto
├── pico_sdk_import.cmake        # Importação do Pico SDK
├── README.md                    # Este arquivo
├── .gitignore                   # Arquivos ignorados pelo Git
└── LICENSE                      # Licença GPL-3.0
```

### Descrição dos Arquivos Principais
- **src/main.c**: Programa principal integrado com MPU-6050, OLED e controle de motores TB6612FNG
- **include/tb6612fng.c**: Implementação completa do driver para controle de motores DC
- **include/tb6612fng.h**: Interface e definições para o driver TB6612FNG (pinos, estruturas, funções)
- **include/ssd1306_i2c.c**: Implementação das funções de comunicação I2C com o display OLED SSD1306
- **include/ssd1306_i2c.h**: Interface e definições para o display OLED (endereços, comandos, estruturas)
- **include/ssd1306.h**: Declarações das funções públicas para controle do display OLED
- **include/ssd1306_font.h**: Fonte de caracteres 8x8 pixels para exibição de texto no OLED
- **CMakeLists.txt**: Configuração do sistema de build incluindo bibliotecas PWM e I2C do Pico SDK
- **pico_sdk_import.cmake**: Script para localização e importação do Pico SDK

### Funcionalidades Implementadas
- ✅ **Leitura de dados inerciais** do MPU-6050 via I2C0
- ✅ **Display visual** no OLED SSD1306 via I2C1 
- ✅ **Controle inteligente de motores** baseado em aceleração e rotação
- ✅ **Driver TB6612FNG** com PWM para controle de velocidade e direção
- ✅ **Sistema modular** com arquitetura limpa e reutilizável
- ✅ **Controle de segurança** com limites de velocidade e detecção de movimento
- ✅ **Interface serial** para monitoramento e debug em tempo real

## 📜 Licença
Este projeto é licenciado sob GNU General Public License v3.0 (GPL-3.0).
Para mais detalhes, consulte o arquivo [LICENSE](LICENSE) ou visite 
<https://www.gnu.org/licenses/gpl-3.0.html>.

---
*Projeto desenvolvido como parte do curso de Residência Tecnológica em Sistemas Embarcados - EmbarcaTech 2025* 