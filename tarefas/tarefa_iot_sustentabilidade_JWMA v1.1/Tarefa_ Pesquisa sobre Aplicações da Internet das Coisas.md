# Tarefa EmbarcaTech: IoT no Contexto da Sustentabilidade

**Autores:** Jorge Wilker Mamede de Andrade e Mauricio  
**Turma:** Campinas, 17 de maio de 2025

## Tarefa

Pesquisa sobre Aplicações da Internet das Coisas.

## SisCalhas: Sistema de Monitoramento Contínuo para Prevenção de Entupimentos e Transbordamentos

### Introdução ao Problema e Contexto

O sistema de calhas e tubulações é essencial para captar e conduzir água das chuvas, evitando transtornos. No entanto, folhas, plásticos e sujeiras acumuladas podem causar entupimentos, tornando a manutenção periódica necessária, idealmente a cada seis meses ou mais frequentemente em áreas com vegetação densa.

### Requisitos e Descrição da Solução IoT

Propomos um sistema de manutenção preditiva para evitar transbordamentos, monitorando o estado das calhas em tempo real. A arquitetura inclui:

- **Camada de Sensores:** Sensores ultrassônicos resistentes à água (como HC-SR04) para medir níveis de detritos ou água.
- **Camada de Conectividade:** Microcontrolador ESP32 para transmissão de dados via Wi-Fi.
- **Camada de Borda:** Pré-processamento local no ESP32 para detectar riscos e gerar alertas.
- **Camada de Armazenamento:** Armazenamento em nuvem (ex.: Firebase ou AWS) para registro histórico.
- **Camada de Abstração:** Processamento de dados via APIs para gerar relatórios.
- **Camada de Exibição:** Aplicativos móveis ou dashboards para notificações ao usuário.

### Especificações e Operação

Os sensores medem continuamente o nível nas calhas, e o ESP32 processa os dados. Ao atingir um limiar crítico, alertas são enviados para a nuvem e exibidos ao usuário, permitindo manutenção preditiva.

### Impacto Positivo ao Consumidor e Benefícios Adicionais

O sistema elimina visitas técnicas desnecessárias, otimizando recursos para residências, edifícios e instituições públicas, com manutenção baseada em condições reais.

### Soluções Já Existentes

- [**Zensy**](https://zensy.io/whats-zensy/)
- [**Smart Drain Grate Monitoring**](https://blog.semtech.com/smart-drain-grate-monitoring-enhancing-efficiency-with-iot-and-lorawan)
- [**FlexSense**](https://myriota.com/br/flexsense/)

### Sustentabilidade

O sistema contribui para a sustentabilidade ao:
- Reduzir o consumo de recursos e resíduos.
- Prevenir danos ambientais e estruturais.
- Otimizar logística e aumentar a vida útil das calhas.

### Escalabilidade

O projeto é escalável para pequenos e médios portes, com potencial para upgrades (ex.: LoRa), mas tem limitações em redes Wi-Fi para grandes escalas.

### Segurança

Garante proteção via protocolos como TLS, autenticação de dispositivos e assinaturas digitais para atualizações OTA.

### Escolha de Hardware (Camadas 1-3)

- **Camada de Sensores:** HC-SR04 waterproof (5V, resistente à umidade).
- **Camada de Borda e Conectividade:** ESP32 para processamento e comunicação via Wi-Fi.

### Bibliografia

- FERNANDO K TECNOLOGIA. *Sensor ultrassônico com ESP32 - Parte 1*. YouTube. [**Link**](https://www.youtube.com/watch?v=Rx3hkcfdL8Q)
- NACIONAL METAIS. *Cuidados e prevenção: como efetuar a manutenção de calhas*. Blog. [**Link**](https://nacionalmetais.com/cuidados-e-prevencao-como-efetuar-a-manutencao-de-calhas/)
- WR KITS. *SENSOR ULTRASSÔNICO RESISTENTE À ÁGUA*. YouTube. [**Link**](https://www.youtube.com/watch?v=A1zTalvWoGw&t=4s)

