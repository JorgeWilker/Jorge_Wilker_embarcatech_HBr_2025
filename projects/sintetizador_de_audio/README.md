Projetos de Sistemas Embarcados - EmbarcaTech 2025
# 🎵 Sintetizador de Áudio
**Autor:** Jorge Wilker Mamede de Andrade  
**Curso:** Residência Tecnológica em Sistemas Embarcados  
**Instituição:** EmbarcaTech - HBr  
**Local:** Campinas, SP  
**Data:** Junho de 2025
**Link do video no youtube:** https://www.youtube.com/watch?v=CD-abA8Nj0g

## 🎯 Objetivo

Este projeto implementa um sintetizador de áudio capaz de gravar e reproduzir áudio usando a plataforma BitDogLab com Raspberry Pi Pico. O sistema utiliza conversão analógico-digital (ADC) para captura de voz através do microfone e modulação por largura de pulso (PWM) para reprodução através de buzzer, com recursos avançados de redução de ruído digital e **visualização da forma de onda em tempo real**.



### Funcionalidades Principais
- 📹 **Gravação de Áudio**: Captura voz via microfone com buffer dinâmico
- 🔊 **Reprodução de Áudio**: Reproduz o áudio gravado através do buzzer com qualidade otimizada
- 📊 **Visualização de Forma de Onda**: Exibição em tempo real da forma de onda no display OLED durante gravação
- 🎮 **Controle por Botões**: Interface intuitiva com os botões A e B da BitDogLab
- 💡 **Feedback Visual**: LEDs RGB indicam o estado do sistema (idle/gravando/reproduzindo)
- 📟 **Display OLED**: Interface completa com menu principal e informações em tempo real
- 🔧 **Redução de Ruído PWM**: Sistema de alta impedância para eliminar ruído digital do clock interno
- ⚡ **Processamento Eficiente**: Controle de estados automático e otimização de recursos

## 🔧 Componentes Usados

### Hardware
- **Raspberry Pi Pico**: Microcontrolador principal com ARM Cortex-M0+
- **BitDogLab**: Plataforma educacional com periféricos integrados
- **Display OLED SSD1306**: 128x64 pixels, comunicação I2C
- **Microfone**: Captura de áudio analógico via ADC
- **Buzzer Passivo**: Reprodução de áudio via PWM com redução de ruído
- **LED RGB**: Feedback visual de estado com indicações diferenciadas
- **Botões**: Controle de interface (A e B) com debounce implementado

### Software
- **Pico SDK**: Framework de desenvolvimento oficial
- **CMake**: Sistema de build configurado para o projeto
- **VS Code + Cursor**: Ambiente de desenvolvimento com regras personalizadas
- **Ninja**: Gerador de build para compilação otimizada

## ⚡ Pinagem dos Dispositivos

| Componente | Pino GPIO | Função | Protocolo |
|------------|-----------|---------|-----------|
| **Microfone** | GPIO 28 (ADC2) | Entrada de áudio | ADC |
| **Buzzer** | GPIO 10 | Saída PWM de áudio | PWM |
| **Display OLED** | GPIO 14 (SDA) | Dados I2C | I2C |
| **Display OLED** | GPIO 15 (SCL) | Clock I2C | I2C |
| **LED RGB Vermelho** | GPIO 11 | LED de estado | GPIO |
| **LED RGB Verde** | GPIO 12 | LED de estado | GPIO |
| **LED RGB Azul** | GPIO 13 | LED de estado | GPIO |
| **Botão A** | GPIO 5 | Controle (Gravar) | GPIO |
| **Botão B** | GPIO 6 | Controle (Reproduzir) | GPIO |

## 🛠️ Recursos Técnicos Implementados

### Sistema de Redução de Ruído PWM
- **Alta Impedância Automática**: Pino PWM em alta impedância quando idle
- **Ativação Sob Demanda**: PWM ativo apenas durante reprodução de áudio
- **Eliminação de Ruído Digital**: Remove interferência do clock interno do PWM
- **Controle Inteligente**: Integração automática com estados do sistema

### Interface de Usuário Avançada
- **Menu Principal**: Navegação intuitiva com instruções claras
- **Estados Visuais**: LEDs RGB com cores específicas para cada modo
  - 🔵 **Azul**: Sistema idle (pronto para uso)
  - 🔴 **Vermelho**: Gravação em andamento (piscando)
  - 🟢 **Verde**: Reprodução ativa
- **Feedback em Tempo Real**: Display mostra informações do buffer e tempo de gravação

### Controle de Estados Inteligente
- **Sistema Idle**: Aguarda comandos do usuário
- **Gravação Ativa**: Controle automático de tempo e buffer
- **Reprodução**: Monitoramento automático de finalização
- **Limpeza de Buffer**: Comando combinado (A+B) para reset

## 📈 Resultados Esperados

### Interface do Usuário
- **Tela de Inicialização**: Logo "BITDOGLAB SINTETIZADOR DE AUDIO V1.0"
- **Menu Principal**: Instruções claras para uso dos botões
- **Modo Gravação**: Visualização da forma de onda em tempo real com grade de referência
- **Modo Reprodução**: Informações da duração do áudio gravado

### Comportamento do Sistema
1. **Inicialização**: Teste automático dos LEDs e inicialização dos subsistemas
2. **Estado Idle**: LED azul fixo, menu principal no display
3. **Gravação (Botão A)**: LED vermelho piscando, visualização da forma de onda ativa
4. **Reprodução (Botão B)**: LED verde fixo, reprodução do áudio gravado
5. **Limpeza (A+B)**: Limpa buffer de áudio e mostra confirmação
6. **Redução de Ruído**: PWM em alta impedância quando não reproduzindo

### Qualidade do Áudio
- **Taxa de Amostragem**: 22.050 Hz para qualidade de voz
- **Resolução**: 12 bits de ADC e 10 bits de PWM
- **Buffer Dinâmico**: Gerenciamento inteligente de memória
- **Latência Mínima**: Resposta imediata aos comandos
- **Ausência de Ruído**: Sistema de alta impedância elimina interferências

## 📂 Estrutura do Projeto

```
projeto-orientado-2-sintetizador-de-udio-JorgeWilker/
├── src/                          # Código-fonte principal
│   ├── main.c                    # Controle principal e interface do usuário
│   ├── audio_pwm.c              # Sistema de áudio com redução de ruído
│   ├── buttons.c                # Controle de botões com debounce
│   ├── led_rgb.c                # Controle do LED RGB com estados
│   └── ssd1306_i2c.c           # Driver do display OLED com visualização da forma de onda
├── include/                      # Cabeçalhos das bibliotecas
│   ├── audio_pwm.h              # Interface do sistema de áudio
│   ├── buttons.h                # Interface dos botões
│   ├── led_rgb.h                # Interface do LED RGB
│   └── ssd1306_i2c.h           # Interface do display
├── docs/                        # Documentação técnica
│   
├── build/                        # Arquivos de compilação (gerado)
│   └── audio_synth.uf2          # Arquivo executável (99.840 bytes)
├── CMakeLists.txt               # Configuração de build do projeto
├── pico_sdk_import.cmake        # Importação do Pico SDK
├── README.md                    # Este arquivo
├── LICENSE                      # Licença GPL-3.0 completa
└── .gitignore                   # Arquivos ignorados pelo Git
```

### Descrição dos Módulos

#### `main.c`
- Controle principal do sistema e máquina de estados
- Interface de usuário completa com display OLED
- Coordenação de todos os subsistemas
- Gerenciamento de eventos e feedback visual

#### `audio_pwm.c/h`
- Sistema de captura de áudio via ADC com buffer dinâmico
- Reprodução via PWM com sistema de redução de ruído
- Controle automático de alta impedância para eliminar interferências
- Gerenciamento inteligente de estados de gravação/reprodução

#### `buttons.c/h`
- Leitura e debounce dos botões A e B
- Detecção de pressionamento simples e combinado
- Interface para eventos de botão com estados held/pressed

#### `led_rgb.c/h`
- Controle individual e combinado dos LEDs RGB
- Estados visuais diferenciados para cada modo do sistema
- Sequências de teste e feedback de estado

#### `ssd1306_i2c.c/h`
- Driver completo para display OLED SSD1306
- Funções de desenho (texto centralizado, linhas, retângulos)
- Interface de usuário com menus e informações em tempo real


## 📜 Licença

Este projeto é licenciado sob **GNU General Public License v3.0 (GPL-3.0)**.  
Para mais detalhes, consulte o arquivo [LICENSE](LICENSE) ou visite:  
https://www.gnu.org/licenses/gpl-3.0.html

### Características da Licença GPL-3.0
- ✅ **Código Aberto**: Livre para usar, modificar e distribuir
- ✅ **Copyleft**: Modificações devem manter a mesma licença
- ✅ **Transparência**: Código-fonte deve estar disponível
- ⚠️ **Responsabilidade**: Fornecido "como está", sem garantias

---

**Desenvolvido com ❤️ para EmbarcaTech 2025**  
*Projeto de Sintetizador de Áudio com Redução de Ruído Digital* 