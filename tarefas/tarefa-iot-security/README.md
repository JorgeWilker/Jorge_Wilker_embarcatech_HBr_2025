[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/G8V_0Zaq)

# 🔒 IoT Security Lab - Segurança em Comunicação MQTT

**Autores:** Jorge Wilker Mamede de Andrade e Carlos Amaral  
**Curso:** Residência Tecnológica em Sistemas Embarcados  
**Instituição:** EmbarcaTech - HBr  
**Campinas, 02 de junho de 2025**

---

## 🎯 Objetivo

Este projeto implementa um sistema de comunicação IoT seguro utilizando Raspberry Pi Pico W (BitDogLab) com proteção contra ataques de replay e criptografia XOR. O sistema demonstra conceitos fundamentais de segurança em IoT, incluindo confidencialidade de dados, proteção temporal e autenticação via MQTT.

## Descrição
Um servidor mosquitto, instalado em um computador na rede local, recebe um broker com dois tópicos. Um criptografado e outro sem criptografia. Uma das placas BitDogLab registrará a temperatura de uma sala, e enviará esta informação ao broker, diretamente, e criptografada. A outra placa assinará o canal (tópico) e exibirá no display OLED da placa as duas mensagens enviadas após a descriptografia. 

### Tópicos MQTT
| Tópico | Descrição |
|--------|-----------|
| `escola/sala1/temperatura` | Mensagens JSON originais (legíveis) |
| `escola/sala1/temperatura_criptografada` | Timestamp criptografado em formato hexadecimal |

## 🔧 Componentes Usados

### Hardware
- **Raspberry Pi Pico W** (2x) (BitDogLab)
- **Display OLED SSD1306** 128x64 (I2C)
- **Rede WiFi** para conectividade
- **Computador** atuando como broker MQTT

### Software
- **Pico SDK** (versão 2.1.1)
- **CMake** para compilação
- **Mosquitto MQTT Broker** (servidor local)
- **lwIP** (Lightweight IP stack)
- **Bibliotecas:**
  - `wifi_conn.c/h` - Gerenciamento de conexão WiFi
  - `mqtt_comm.c/h` - Comunicação MQTT
  - `xor_cipher.c/h` - Criptografia XOR
  - `ssd1306_i2c.c/h` - Driver para display OLED SSD1306

## 💾 Pré-requisitos
1. **Pico SDK** instalado e configurado
2. **CMake** (versão 3.13 ou superior)
3. **Compilador ARM** (arm-none-eabi-gcc)
4. **Mosquitto MQTT Broker** configurado

## 📈 Resultados Esperados

### Mensagens Originais (Legíveis)
```json
{"valor":26.5,"ts":1735123456}
```

### Mensagens Criptografadas (Timestamp em Hexadecimal)
```
4A2E6B7D    ← Timestamp "1735123456" criptografado com XOR e convertido para hex
```

## Exibição esperada no OLED
```
┌─────────────────────────────┐
│   IOT SECURITY LAB          │
│ WIFI: OK    MQTT: OK        │
│─────────────────────────────│
│ TEMP: 26.5 C                │
│ TS: 1735123456    4A2E      │
│ XOR ATIVO                   │
│                             │
└─────────────────────────────┘
```

## 📂 Arquivos e Estrutura

```
tarefa-iot-security-lab-jorgewilker_-_carlosamaral/
├── src/
│   ├── iot_security_lab.c      # Arquivo principal da aplicação
│   ├── wifi_conn.c             # Implementação da conexão WiFi
│   ├── mqtt_comm.c             # Comunicação MQTT com lwIP
│   ├── xor_cipher.c            # Algoritmo de criptografia XOR
│   └── ssd1306_i2c.c           # Driver para display OLED SSD1306
├── include/
│   ├── wifi_conn.h             # Cabeçalho da conexão WiFi
│   ├── mqtt_comm.h             # Cabeçalho da comunicação MQTT
│   ├── xor_cipher.h            # Cabeçalho da criptografia XOR
│   ├── ssd1306_i2c.h           # Cabeçalho do driver OLED
│   └── lwipopts.h              # Configurações do lwIP
├── build/                      # Arquivos de compilação (gerado)
├── CMakeLists.txt              # Configuração de compilação
├── pico_sdk_import.cmake       # Importação do Pico SDK
├── README.md                   # Este arquivo
├── LICENSE                     # Licença GPL-3.0
└── .gitignore                  # Arquivos ignorados pelo Git
```

## 🔒 Mecanismos de Segurança Implementados

1. **Confidencialidade:** Criptografia XOR aplicada ao timestamp
2. **Integridade Temporal:** Timestamp para ordenação de mensagens
3. **Proteção contra Replay:** Validação de timestamp no subscriber
4. **Autenticação:** Credenciais MQTT (usuário/senha)
5. **Interface Visual:** Display OLED com status em tempo real e comparação visual dos dados

## 📋 Relatório de Implementações

### Implementações Realizadas (Ordem Cronológica)

#### 1. **Configuração de Rede Personalizada**
- Atualização das credenciais WiFi para rede local
- Configuração do broker MQTT com IP específico (192.168.1.130)
- Manutenção das credenciais de autenticação MQTT

#### 2. **Criptografia XOR Aplicada ao JSON**
- Criptografia XOR aplicada à mensagem JSON completa (incluindo timestamp)
- Uso de chave fixa (42) para demonstração
- Buffers separados para dados originais e criptografados

#### 3. **Proteção contra Replay Attack (Etapa 6)**
- Adição de timestamp nas mensagens MQTT
- Formatação de dados em JSON com valor e timestamp
- Estrutura de dados: `{"valor":26.5,"ts":1678886400}`

#### 4. **Publicação Dual para Fins Didáticos**
- Publicação simultânea de dados originais e criptografados
- Uso de tópicos MQTT distintos para comparação
- Manutenção de compatibilidade com configurações existentes

### Diferenças em Relação à Tarefa Inicial

**Antes (Código Original):**
- Publicação de mensagem simples: `"26.5"`
- Sem proteção contra replay
- Criptografia XOR opcional e comentada
- Dados sem estrutura temporal

**Depois (Implementação Atual):**
- Publicação de JSON estruturado: `{"valor":26.5,"ts":1735123456}`
- Proteção contra replay com timestamp
- Criptografia XOR aplicada especificamente ao timestamp numérico
- Conversão para formato hexadecimal legível
- Publicação dual (original + timestamp criptografado) para fins didáticos
- Interface OLED completa com status de conectividade, dados e comparação visual
- Estrutura de dados temporal para validação

## 🎓 Valor Didático

A implementação permite:
- **Visualização clara** da diferença entre dados originais e criptografados
- **Compreensão prática** de proteção contra replay attacks
- **Demonstração real** de comunicação IoT segura
- **Comparação lado a lado** de dados protegidos e não protegidos

## 🚀 Próximos Passos

- Implementação de algoritmos de criptografia mais robustos (AES)
- Adição de certificados digitais para autenticação
- Implementação de subscriber com validação de timestamp
- Integração com sensores reais para dados dinâmicos

## 📜 Licença

Este projeto é licenciado sob GNU General Public License v3.0 (GPL-3.0).
Para mais detalhes, consulte o arquivo [LICENSE](LICENSE) ou visite 
<https://www.gnu.org/licenses/gpl-3.0.html>.

---

**Projeto desenvolvido como parte da Residência Tecnológica em Sistemas Embarcados - EmbarcaTech 2025**
