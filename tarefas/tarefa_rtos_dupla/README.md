# Sistema de Controle de Caldeira com FreeRTOS

**Autores:** Jorge Wilker Mamede de Andrade e Roger De Lima Araújo De Melo 
**Curso:** Residência Tecnológica em Sistemas Embarcados  
**Instituição:** EmbarcaTech - HBr  
**Campinas, 18 de junho de 2025**

## Link do projeto no Youtube: https://youtu.be/Jm8qSbdRBkM?si=09aVbqoJJM6A6uH1

---

## 🎯 Objetivo do Projeto

Sistema de controle de caldeira implementado em Raspberry Pi Pico usando **FreeRTOS** com simulação de estados via joystick e visualização em matriz de LEDs RGB 5x5 e display OLED.

O projeto simula um sistema real de controle de caldeira com **4 estados críticos** organizados por **prioridades de segurança**, conforme especificações industriais.

## 🔥 Estados da Caldeira

### 1. **Estado OK** (Verde) - Prioridade 1 (baixa)
- **Controle**: Joystick → Direita
- **Pressão**: 300 kPa | **Temperatura**: 90°C | **Nível**: 54%
- **Atuadores**: Aquecedor Ligado, Bomba Desligada, Alívio Desligado

### 2. **Nível Baixo** (Amarelo) - Prioridade 2 (baixa)
- **Controle**: Joystick → Esquerda  
- **Pressão**: 310 kPa | **Temperatura**: 95°C | **Nível**: 19%
- **Atuadores**: Aquecedor Desligado, Bomba Ligada, Alívio Desligado

### 3. **Temperatura Alta** (Laranja) - Prioridade 3 (média)
- **Controle**: Joystick → Baixo
- **Pressão**: 330 kPa | **Temperatura**: 150°C | **Nível**: 5%
- **Atuadores**: Aquecedor Desligado, Bomba Desligada, Alívio Desligado

### 4. **Pressão Alta** (Vermelho) - Prioridade 4 (MÁXIMA)
- **Controle**: Joystick → Cima
- **Pressão**: 500 kPa | **Temperatura**: 120°C | **Nível**: 54%
- **Atuadores**: Aquecedor Ligado, Bomba Ligada, Alívio Ligado
- **⚠️ EMERGÊNCIA AUTOMÁTICA**: Duração fixa de 5 segundos com preempção natural
- **🔄 PREEMPÇÃO**: Permite execução de outras tarefas durante delays internos

## 🧩 Componentes Utilizados

- **Microcontrolador**: Raspberry Pi Pico (RP2040)
- **Display**: SSD1306 OLED 128x64 I2C
- **Joystick**: Analógico 2 eixos
- **Matriz LED**: NeoPixel WS2812B 5x5 (25 LEDs)
- **RTOS**: FreeRTOS com 6 tarefas e preempção natural

## ⚡ Pinagem dos Dispositivos

### **Joystick**
- **VRX**: GPIO 26 (ADC0) - Eixo X
- **VRY**: GPIO 27 (ADC1) - Eixo Y

### **Display SSD1306 (I2C)**
- **SDA**: GPIO 14
- **SCL**: GPIO 15

### **Matriz LED NeoPixel**
- **DIN**: GPIO 7 (controle PIO)

## 🔧 Arquitetura FreeRTOS

### **Tarefas e Prioridades**
1. **Tarefa Joystick** - Prioridade 5 (máxima responsividade de entrada)
2. **Tarefa Pressão Alta** - Prioridade 4 (emergência crítica com preempção)
3. **Tarefa Temperatura Alta** - Prioridade 3 (média)
4. **Tarefa Nível Baixo** - Prioridade 2 (baixa)
5. **Tarefa Estado OK** - Prioridade 1 (baixa)
6. **Tarefa Display** - Prioridade 1 (baixa)

### **Comunicação Inter-Tarefas**
- **Queue**: Fila para transmissão de estados entre tarefas
- **Preempção Natural**: Escalonador FreeRTOS controla execução baseada em prioridades

### **Sistema de Preempção Inteligente**

#### **Comportamento da Emergência (Prioridade 4)**
```c
// Ciclo de 5 segundos com preempção natural
for (int i = 5; i > 0; i--) {
    printf(">>> EMERGENCIA: %d segundos restantes <<<\n", i);
    vTaskDelay(pdMS_TO_TICKS(1000)); // ← Outras tarefas executam aqui
    exibir_cor_matriz(255, 0, 0);    // ← Retoma e atualiza LED
}
```

#### **Fluxo de Preempção**
1. **Tarefa de Emergência Inicia**: Prioridade 4 assume controle
2. **Durante `vTaskDelay(1000)`**: Kernel permite execução de outras tarefas
3. **Tarefa de Emergência Retoma**: Automaticamente após 1 segundo
4. **Ciclo se Repete**: Por 5 iterações (5 segundos total)
5. **Auto-Finalização**: Retorna ao Estado OK automaticamente

#### **Vantagens da Preempção Natural**
- **Display Continua Atualizando**: Tarefa de display executa durante delays
- **Joystick Permanece Responsivo**: Para futuras mudanças de estado
- **Sistema Nunca Trava**: Princípios RTOS respeitados integralmente
- **Comportamento Previsível**: Escalonador controla tudo automaticamente

## 🖥️ Interface do Display

```
Estado: OK          <- Estado atual
Pressao: 300 kPa    <- Pressão em kPa
Temp: 90 C          <- Temperatura em °C
Nivel: 54%          <- Nível de água em %
Aquec: Ligado       <- Status do aquecedor
Bomba: Deslig       <- Status da bomba
Alivio: Deslig      <- Status da válvula de alívio
```

## 🧪 Como Compilar e Executar

### **Pré-requisitos**
- Raspberry Pi Pico SDK
- CMake 3.21+
- Ninja Build System
- Toolchain ARM GCC

### **Compilação**
```bash
# 1. Clone/baixe o projeto
cd embarcatech-2025-tarefa-robo-dupla

# 2. Criar pasta de build
mkdir build
cd build

# 3. Configurar CMake
cmake -G Ninja ..

# 4. Compilar
ninja caldeira

# 5. Carregar no Pico
# O arquivo caldeira.uf2 será gerado
# Conecte o Pico em modo BOOTSEL e copie o arquivo
```

## 📁 Estrutura do Projeto

```
embarcatech-2025-tarefa-robo-dupla/
├── FreeRTOS/                    # Kernel FreeRTOS completo
├── include/                     # Headers (SSD1306, FreeRTOSConfig.h)
├── caldeira_main.c              # ⭐ Código principal
├── CMakeLists.txt               # Configuração de build
├── ws2818b.pio                  # Programa PIO para NeoPixel
├── pico_sdk_import.cmake        # Import do SDK
└── README.md                    # Esta documentação
```

## 🎮 Controles do Sistema

| Direção do Joystick | Estado Resultante | Cor da Matriz | Prioridade |
|---------------------|-------------------|---------------|------------|
| **→ Direita**       | Estado OK         | 🟢 Verde      | 1 (baixa)  |
| **← Esquerda**      | Nível Baixo       | 🟡 Amarelo    | 2 (baixa)  |
| **↓ Baixo**         | Temperatura Alta  | 🟠 Laranja    | 3 (média)  |
| **↑ Cima**          | Pressão Alta      | 🔴 Vermelho   | 4 (MÁXIMA) |

## 📊 Especificações Técnicas

- **Microcontrolador**: RP2040 dual-core ARM Cortex-M0+ @ 133MHz
- **Memória**: 264KB SRAM, 2MB Flash
- **RTOS**: FreeRTOS Kernel v10.x
- **ADC**: 12-bit (0-4095)
- **Display**: 128x64 OLED I2C @ 0x3C
- **LEDs**: WS2812B 800kHz via PIO
- **Comunicação**: USB Serial (115200 baud)

## 🔒 Recursos de Segurança e RTOS

### **Priorização Inteligente**
- **Escalonamento por Criticidade**: Estados mais críticos têm prioridade natural do RTOS
- **Preempção Cooperativa**: Emergência permite execução de outras tarefas durante delays
- **Resposta Garantida**: Joystick com prioridade 5 garante responsividade máxima

### **Gerenciamento de Emergência**
- **Duração Controlada**: Emergência dura exatamente 5 segundos automaticamente
- **Preempção Natural**: FreeRTOS escalonador controla todas as transições
- **Auto-Recuperação**: Sistema retorna ao Estado OK automaticamente
- **Monitoramento Contínuo**: Display e outras tarefas continuam funcionando

### **Arquitetura RTOS Pura**
- **Sem Bloqueios Artificiais**: Utiliza apenas primitivas nativas do FreeRTOS
- **Escalonamento Natural**: Kernel gerencia prioridades automaticamente
- **Preempção Eficiente**: Tarefas cedem controle durante delays (`vTaskDelay`)
- **Responsividade Mantida**: Sistema nunca trava, sempre responsivo

## 📝 Logs do Sistema

O sistema exibe logs detalhados via USB Serial, incluindo **contador regressivo de emergência**:
```
=== SISTEMA DE CONTROLE DE CALDEIRA ===
Inicializando componentes...
FreeRTOS Kernel inicializado

=== PRESSAO ALTA - EMERGENCIA ===
!!! SITUACAO CRITICA !!!
>>> EMERGENCIA: 5 segundos restantes <<<
>>> EMERGENCIA: 4 segundos restantes <<<
>>> EMERGENCIA: 3 segundos restantes <<<
>>> EMERGENCIA: 2 segundos restantes <<<
>>> EMERGENCIA: 1 segundos restantes <<<
=== EMERGENCIA FINALIZADA AUTOMATICAMENTE (5s) ===

=== CALDEIRA OK ===
Pressao: 300 kPa
Temperatura: 90 C
Nivel: 54%
Aquecedor: Ligado
```

## 👨‍💻 Autor

Desenvolvido como parte do programa **EmbarcaTech 2025** - Tarefa de Robótica em Dupla.

**Sistema de Controle Industrial com FreeRTOS para Raspberry Pi Pico** 🚀

