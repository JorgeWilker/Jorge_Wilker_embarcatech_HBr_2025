// Sistema de Controle de Caldeira Industrial com FreeRTOS
// Implementa simulação de caldeira com 4 estados críticos e prioridades diferenciadas
// Utiliza joystick analógico, display OLED SSD1306 e matriz LED NeoPixel 5x5
// Arquitetura RTOS pura com preempção natural para gerenciamento de emergências
// Autor: Jorge Wilker Mamede de Andrade e Roger - EmbarcaTech 2025

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include <stdint.h>
#include <stdbool.h>
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ssd1306.h"

// Programa PIO para controle de matriz LED WS2812B
#include "ws2818b.pio.h"

// =============================================================================
// CONFIGURAÇÕES DE HARDWARE E CONSTANTES DO SISTEMA
// =============================================================================

// Configuração dos pinos do joystick analógico
#define VRX_PIN         26      // Eixo X do joystick conectado ao ADC0
#define VRY_PIN         27      // Eixo Y do joystick conectado ao ADC1

// Configuração do display OLED SSD1306 via I2C
#define SDA_PIN         14      // Linha de dados I2C
#define SCL_PIN         15      // Linha de clock I2C

// Configuração da matriz de LEDs NeoPixel WS2812B
#define LED_PIN         7       // Pino de controle PIO para matriz LED
#define LED_COUNT       25      // Matriz 5x5 = 25 LEDs individuais

// Canais ADC para leitura do joystick
#define ADC_CH_X        0       // Canal ADC para eixo X
#define ADC_CH_Y        1       // Canal ADC para eixo Y

// Calibração de sensibilidade do joystick analógico
// Valores baseados em ADC de 12 bits (0-4095) do RP2040
#define JOY_CENTER_MIN  1800    // Limite inferior da zona morta central
#define JOY_CENTER_MAX  2300    // Limite superior da zona morta central
#define JOY_THRESHOLD   1000    // Sensibilidade para detecção de movimento

// =============================================================================
// ESTRUTURAS E TIPOS DE DADOS DO SISTEMA
// =============================================================================

// Enumeração dos estados críticos da caldeira industrial
// Organizados por prioridade crescente para escalonamento RTOS
typedef enum {
    CALDEIRA_OK = 0,           // Prioridade 1: operação normal (baixa)
    CALDEIRA_NIVEL_BAIXO,      // Prioridade 2: água insuficiente (baixa)
    CALDEIRA_TEMP_ALTA,        // Prioridade 3: superaquecimento (média)
    CALDEIRA_PRESSAO_ALTA      // Prioridade 4: emergência crítica (máxima)
} estado_caldeira_t;

// Estrutura de dados completa do estado da caldeira
// Contém telemetria e status dos atuadores em tempo real
typedef struct {
    estado_caldeira_t estado;  // Estado operacional atual
    float pressao;             // Pressão interna em kPa
    float temperatura;         // Temperatura do vapor em °C
    float nivel_agua;          // Nível do reservatório em %
    bool aquecedor;            // Status do sistema de aquecimento
    bool bomba;                // Status da bomba de alimentação
    bool alivio;               // Status da válvula de alívio
} dados_caldeira_t;

// Estrutura de pixel para matriz LED WS2812B
// Ordenação GRB conforme protocolo nativo do controlador
typedef struct {
    uint8_t G, R, B;           // Verde, Vermelho, Azul (8 bits cada)
} pixel_t;

// Enumeração das direções do joystick analógico
// Mapeamento direto para comandos de estado da caldeira
typedef enum {
    JOY_CENTER = 0,            // Posição neutra (sem comando)
    JOY_RIGHT,                 // Direita: aciona estado OK
    JOY_LEFT,                  // Esquerda: simula nível baixo
    JOY_DOWN,                  // Baixo: simula temperatura alta
    JOY_UP                     // Cima: simula pressão alta (emergência)
} joystick_dir_t;

// =============================================================================
// VARIÁVEIS GLOBAIS DO SISTEMA FREERTOS
// =============================================================================

// Handles das tarefas FreeRTOS para controle de ciclo de vida
// Cada tarefa possui prioridade específica conforme criticidade
TaskHandle_t xJoystickTaskHandle = NULL;      // Tarefa de entrada: prioridade 5
TaskHandle_t xCaldeiraOKTaskHandle = NULL;    // Estado normal: prioridade 1
TaskHandle_t xCaldeiraNivelTaskHandle = NULL; // Nível baixo: prioridade 2
TaskHandle_t xCaldeiraTempTaskHandle = NULL;  // Temperatura: prioridade 3
TaskHandle_t xCaldeiraPressaoTaskHandle = NULL; // Emergência: prioridade 4
TaskHandle_t xDisplayTaskHandle = NULL;       // Interface: prioridade 1

// Fila de comunicação inter-tarefas para comandos de estado
// Implementa padrão producer-consumer thread-safe
QueueHandle_t xEstadoQueue;

// Buffer de framebuffer para display OLED SSD1306
// Área de renderização configurada para tela completa 128x64
uint8_t display_buffer[ssd1306_buffer_length];
struct render_area area_display = {
    .start_column = 0,
    .end_column = ssd1306_width - 1,    // 127 pixels largura
    .start_page = 0,
    .end_page = ssd1306_n_pages - 1     // 7 páginas altura
};

// Buffer de pixels para matriz LED 5x5 NeoPixel
// Cada pixel contém componentes GRB de 8 bits
pixel_t leds[LED_COUNT];

// Recursos de hardware PIO para controle WS2812B
// PIO permite comunicação de alta velocidade com protocolo proprietário
PIO np_pio;                           // Instância PIO (0 ou 1)
uint sm;                              // Máquina de estado PIO

// Estado de telemetria atual da caldeira industrial
// Inicializado com valores de operação normal segura
dados_caldeira_t estado_atual = {
    .estado = CALDEIRA_OK,            // Operação normal
    .pressao = 300.0,                 // 300 kPa (pressão segura)
    .temperatura = 90.0,              // 90°C (temperatura controlada)
    .nivel_agua = 54.0,               // 54% (nível adequado)
    .aquecedor = true,                // Aquecimento ativo
    .bomba = false,                   // Bomba inativa
    .alivio = false                   // Válvula fechada
};

// =============================================================================
// FUNÇÕES DE CONTROLE DA MATRIZ LED NEOPIXEL WS2812B
// =============================================================================

// Fator de atenuação de brilho para conforto visual
// Reduz intensidade luminosa para 1% da potência máxima
#define BRIGHTNESS_FACTOR 0.01f  // 1% para evitar ofuscamento em ambientes

// Aplica redução de brilho proporcional nos componentes RGB
// Implementa controle de intensidade luminosa por software
uint8_t apply_brightness(uint8_t color_value) {
    return (uint8_t)(color_value * BRIGHTNESS_FACTOR);
}

// Inicializa interface PIO para comunicação com matriz WS2812B
// Configura máquina de estado PIO e define frequência de transmissão
void neopixel_init(uint pin) {
    // Carrega programa PIO para protocolo WS2812B no bloco PIO0
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;
    
    // Tenta alocar máquina de estado no PIO0, fallback para PIO1
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0) {
        np_pio = pio1;
        sm = pio_claim_unused_sm(np_pio, true);  // Força alocação em PIO1
    }
    
    // Inicializa programa PIO com frequência de 800 kHz (padrão WS2812B)
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
    
    // Zera buffer de pixels para estado inicial apagado
    for (uint i = 0; i < LED_COUNT; ++i) {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

// Define cor de um LED específico na matriz com controle de brilho
// Aplica automaticamente fator de atenuação configurado
void neopixel_set_led(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    if (index < LED_COUNT) {
        leds[index].R = apply_brightness(r);  // Componente vermelho atenuado
        leds[index].G = apply_brightness(g);  // Componente verde atenuado
        leds[index].B = apply_brightness(b);  // Componente azul atenuado
    }
}

// Apaga todos os LEDs da matriz definindo cor preta (0,0,0)
// Utilizada para reset visual entre estados da caldeira
void neopixel_clear() {
    for (uint i = 0; i < LED_COUNT; ++i) {
        neopixel_set_led(i, 0, 0, 0);
    }
}

// Transmite buffer de pixels via PIO para matriz WS2812B
// Converte formato RGB para GRB conforme protocolo do controlador
void neopixel_write() {
    for (uint i = 0; i < LED_COUNT; ++i) {
        // Reorganiza componentes para formato GRB nativo
        uint32_t pixel_grb = (leds[i].G << 16) | (leds[i].R << 8) | leds[i].B;
        // Transmite pixel com shift de 8 bits conforme protocolo
        pio_sm_put_blocking(np_pio, sm, pixel_grb << 8u);
    }
    sleep_ms(1);  // Delay mínimo para estabilização do sinal
}

// Converte coordenadas cartesianas (x,y) para índice linear da matriz
// Implementa mapeamento serpentina para disposição física dos LEDs
int get_led_index(int x, int y) {
    if (y % 2 == 0) {
        // Linhas pares: da esquerda para direita
        return 24 - (y * 5 + x);
    } else {
        // Linhas ímpares: da direita para esquerda (padrão serpentina)
        return 24 - (y * 5 + (4 - x));
    }
}

// Exibe cor sólida em toda matriz LED para indicação de estado
// Utiliza brilho padrão configurado globalmente
void exibir_cor_matriz(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < LED_COUNT; i++) {
        neopixel_set_led(i, r, g, b);
    }
    neopixel_write();  // Efetiva transmissão para hardware
}

// Exibe cor sólida com controle de brilho personalizado
// Permite ajuste dinâmico de intensidade luminosa (0.0 a 1.0)
void exibir_cor_matriz_com_brilho(uint8_t r, uint8_t g, uint8_t b, float brightness) {
    // Aplica fator de brilho customizado nos componentes RGB
    uint8_t adj_r = (uint8_t)(r * brightness);
    uint8_t adj_g = (uint8_t)(g * brightness);
    uint8_t adj_b = (uint8_t)(b * brightness);
    
    for (int i = 0; i < LED_COUNT; i++) {
        // Define pixel diretamente para evitar dupla atenuação
        leds[i].R = adj_r;
        leds[i].G = adj_g;
        leds[i].B = adj_b;
    }
    neopixel_write();  // Transmite dados para matriz LED
}

// =============================================================================
// FUNÇÕES DE LEITURA E PROCESSAMENTO DO JOYSTICK ANALÓGICO
// =============================================================================

// Lê posição atual do joystick e retorna direção detectada
// Implementa zona morta central para evitar comandos espúrios
joystick_dir_t ler_joystick() {
    // Seleciona canal ADC do eixo X e aguarda estabilização
    adc_select_input(ADC_CH_X);
    sleep_us(2);  // Tempo de settling do multiplexer ADC
    uint16_t x = adc_read();
    
    // Seleciona canal ADC do eixo Y e aguarda estabilização
    adc_select_input(ADC_CH_Y);
    sleep_us(2);  // Tempo de settling do multiplexer ADC
    uint16_t y = adc_read();
    
    // Análise de direção baseada em thresholds calibrados
    // Implementa zona morta central para estabilidade
    if (x > (JOY_CENTER_MAX + JOY_THRESHOLD)) {
        return JOY_RIGHT;  // Comando: acionar estado OK
    }
    if (x < (JOY_CENTER_MIN - JOY_THRESHOLD)) {
        return JOY_LEFT;   // Comando: simular nível baixo
    }
    if (y > (JOY_CENTER_MAX + JOY_THRESHOLD)) {
        return JOY_UP;     // Comando: simular pressão alta (emergência)
    }
    if (y < (JOY_CENTER_MIN - JOY_THRESHOLD)) {
        return JOY_DOWN;   // Comando: simular temperatura alta
    }
    
    return JOY_CENTER;     // Posição neutra: sem comando ativo
}

// =============================================================================
// FUNÇÕES DE INTERFACE VISUAL COM DISPLAY OLED SSD1306
// =============================================================================

// Atualiza interface do display com telemetria atual da caldeira
// Exibe 7 linhas de informações conforme especificação do sistema
void atualizar_display(dados_caldeira_t *dados) {
    char texto[32];                    // Buffer temporário para formatação de texto
    ssd1306_clear(display_buffer);     // Limpa framebuffer antes da renderização
    
    // Linha 1: Identificação do estado operacional atual
    const char* estados[] = {"OK", "Nv Low", "Tp High", "Pr high"};
    sprintf(texto, "Estado: %s", estados[dados->estado]);
    ssd1306_draw_string(display_buffer, 0, 0, texto);
    
    // Linha 2: Pressão interna do sistema em kPa
    sprintf(texto, "Pressao:%.0f kPa", dados->pressao);
    ssd1306_draw_string(display_buffer, 0, 8, texto);
    
    // Linha 3: Temperatura do vapor em graus Celsius
    sprintf(texto, "Temp:   %.0f C", dados->temperatura);
    ssd1306_draw_string(display_buffer, 0, 16, texto);
    
    // Linha 4: Nível percentual do reservatório de água
    sprintf(texto, "Nivel:  %.0f%%", dados->nivel_agua);
    ssd1306_draw_string(display_buffer, 0, 24, texto);
    
    // Linha 5: Status do sistema de aquecimento
    sprintf(texto, "Aquec:  %s", dados->aquecedor ? "On" : "Off");
    ssd1306_draw_string(display_buffer, 0, 32, texto);
    
    // Linha 6: Status da bomba de alimentação de água
    sprintf(texto, "Bomba:  %s", dados->bomba ? "On" : "Off");
    ssd1306_draw_string(display_buffer, 0, 40, texto);
    
    // Linha 7: Status da válvula de alívio de pressão
    sprintf(texto, "Alivio: %s", dados->alivio ? "On" : "Off");
    ssd1306_draw_string(display_buffer, 0, 48, texto);
    
    render_on_display(display_buffer, &area_display);  // Transfere para hardware
}

// =============================================================================
// TAREFAS CONCORRENTES DO SISTEMA FREERTOS
// =============================================================================

// Tarefa de entrada: monitoramento contínuo do joystick analógico
// Prioridade 5 (máxima): garante resposta imediata aos comandos do usuário
void tarefa_joystick(void *pvParameters) {
    joystick_dir_t dir_anterior = JOY_CENTER;  // Estado anterior para detecção de mudança
    estado_caldeira_t novo_estado;             // Comando a ser enviado via fila
    
    while (true) {
        joystick_dir_t dir_atual = ler_joystick();  // Leitura ADC dos eixos X/Y
        
        // Detecta transição de estado (evita comandos repetitivos)
        if (dir_atual != dir_anterior && dir_atual != JOY_CENTER) {
            // Mapeamento de direções para estados críticos da caldeira
            switch (dir_atual) {
                case JOY_RIGHT:
                    novo_estado = CALDEIRA_OK;
                    printf("Joystick: Estado OK\n");
                    break;
                case JOY_LEFT:
                    novo_estado = CALDEIRA_NIVEL_BAIXO;
                    printf("Joystick: Nível baixo\n");
                    break;
                case JOY_DOWN:
                    novo_estado = CALDEIRA_TEMP_ALTA;
                    printf("Joystick: Temperatura alta\n");
                    break;
                case JOY_UP:
                    novo_estado = CALDEIRA_PRESSAO_ALTA;
                    printf("Joystick: Pressão alta\n");
                    break;
                default:
                    novo_estado = CALDEIRA_OK;
                    break;
            }
            
            // Envia comando via fila thread-safe para tarefas de estado
            xQueueSend(xEstadoQueue, &novo_estado, 0);
        }
        
        dir_anterior = dir_atual;                   // Atualiza estado anterior
        vTaskDelay(pdMS_TO_TICKS(100));             // Período de amostragem: 100ms
    }
}

// Tarefa de Estado Operacional Normal da Caldeira
// Prioridade 1 (baixa): operação padrão sem urgência crítica
void tarefa_caldeira_ok(void *pvParameters) {
    while (true) {
        estado_caldeira_t estado_recebido;          // Comando recebido da fila
        
        // Aguarda comando específico para este estado (bloqueante)
        if (xQueueReceive(xEstadoQueue, &estado_recebido, portMAX_DELAY) == pdTRUE) {
            if (estado_recebido == CALDEIRA_OK) {
                printf("=== CALDEIRA OK ===\n");
                
                // Configuração de telemetria para operação normal
                estado_atual.estado = CALDEIRA_OK;
                estado_atual.pressao = 300.0;          // Pressão controlada: 300 kPa
                estado_atual.temperatura = 90.0;       // Temperatura segura: 90°C
                estado_atual.nivel_agua = 54.0;        // Nível adequado: 54%
                estado_atual.aquecedor = true;         // Aquecimento ativo
                estado_atual.bomba = false;            // Bomba desnecessária
                estado_atual.alivio = false;           // Válvula fechada
                
                // Sinalização visual: LED verde (operação normal)
                exibir_cor_matriz(0, 255, 0);
                
                // Log detalhado da telemetria atual
                printf("Pressao: %.0f kPa\n", estado_atual.pressao);
                printf("Temperatura: %.0f C\n", estado_atual.temperatura);
                printf("Nivel: %.0f%%\n", estado_atual.nivel_agua);
                printf("Aquecedor: %s\n", estado_atual.aquecedor ? "Ligado" : "Desligado");
                printf("Bomba: %s\n", estado_atual.bomba ? "Ligado" : "Desligado");
                printf("Alivio: %s\n", estado_atual.alivio ? "Ligado" : "Desligado");
            } else {
                // Rejeita comando: retorna à fila para tarefa apropriada
                xQueueSend(xEstadoQueue, &estado_recebido, 0);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));             // Período de execução: 500ms
    }
}

// Tarefa de Estado Crítico: Nível de Água Insuficiente
// Prioridade 2 (baixa): ativa bomba de alimentação para correção
void tarefa_caldeira_nivel(void *pvParameters) {
    while (true) {
        estado_caldeira_t estado_recebido;
        
        if (xQueueReceive(xEstadoQueue, &estado_recebido, portMAX_DELAY) == pdTRUE) {
            if (estado_recebido == CALDEIRA_NIVEL_BAIXO) {
                printf("=== NIVEL DE AGUA BAIXO ===\n");
                
                // Atualiza dados da caldeira
                estado_atual.estado = CALDEIRA_NIVEL_BAIXO;
                estado_atual.pressao = 310.0;
                estado_atual.temperatura = 95.0;
                estado_atual.nivel_agua = 19.0;
                estado_atual.aquecedor = false;
                estado_atual.bomba = true;
                estado_atual.alivio = false;
                
                // Sinalização visual: LED amarelo (atenção requerida)
                exibir_cor_matriz(255, 255, 0);
                
                printf("Pressao: %.0f kPa\n", estado_atual.pressao);
                printf("Temperatura: %.0f C\n", estado_atual.temperatura);
                printf("Nivel: %.0f%%\n", estado_atual.nivel_agua);
                printf("Aquecedor: %s\n", estado_atual.aquecedor ? "Ligado" : "Desligado");
                printf("Bomba: %s\n", estado_atual.bomba ? "Ligado" : "Desligado");
                printf("Alivio: %s\n", estado_atual.alivio ? "Ligado" : "Desligado");
            } else {
                xQueueSend(xEstadoQueue, &estado_recebido, 0);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(400));
    }
}

// Tarefa de Estado Crítico: Superaquecimento do Sistema
// Prioridade 3 (média): desliga aquecimento para resfriamento controlado
void tarefa_caldeira_temperatura(void *pvParameters) {
    while (true) {
        estado_caldeira_t estado_recebido;
        
        if (xQueueReceive(xEstadoQueue, &estado_recebido, portMAX_DELAY) == pdTRUE) {
            if (estado_recebido == CALDEIRA_TEMP_ALTA) {
                printf("=== TEMPERATURA ALTA ===\n");
                
                // Atualiza dados da caldeira
                estado_atual.estado = CALDEIRA_TEMP_ALTA;
                estado_atual.pressao = 330.0;
                estado_atual.temperatura = 150.0;
                estado_atual.nivel_agua = 5.0;
                estado_atual.aquecedor = false;
                estado_atual.bomba = false;
                estado_atual.alivio = false;
                
                // Sinalização visual: LED laranja (superaquecimento)
                exibir_cor_matriz(255, 165, 0);
                
                printf("Pressao: %.0f kPa\n", estado_atual.pressao);
                printf("Temperatura: %.0f C\n", estado_atual.temperatura);
                printf("Nivel: %.0f%%\n", estado_atual.nivel_agua);
                printf("Aquecedor: %s\n", estado_atual.aquecedor ? "Ligado" : "Desligado");
                printf("Bomba: %s\n", estado_atual.bomba ? "Ligado" : "Desligado");
                printf("Alivio: %s\n", estado_atual.alivio ? "Ligado" : "Desligado");
            } else {
                xQueueSend(xEstadoQueue, &estado_recebido, 0);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

// Tarefa de Estado de Emergência: Pressão Crítica Excessiva
// Prioridade 4 (máxima): protocolo de segurança com preempção natural
void tarefa_caldeira_pressao(void *pvParameters) {
    while (true) {
        estado_caldeira_t estado_recebido;          // Comando crítico recebido
        
        if (xQueueReceive(xEstadoQueue, &estado_recebido, portMAX_DELAY) == pdTRUE) {
            if (estado_recebido == CALDEIRA_PRESSAO_ALTA) {
                printf("=== PRESSAO ALTA - EMERGENCIA ===\n");
                
                // Configuração crítica: ativação de todos os sistemas de segurança
                estado_atual.estado = CALDEIRA_PRESSAO_ALTA;
                estado_atual.pressao = 500.0;          // Pressão perigosa: 500 kPa
                estado_atual.temperatura = 120.0;      // Temperatura elevada: 120°C
                estado_atual.nivel_agua = 54.0;        // Nível mantido: 54%
                estado_atual.aquecedor = true;         // Aquecimento mantido
                estado_atual.bomba = true;             // Bomba ativa (resfriamento)
                estado_atual.alivio = true;            // Válvula aberta (alívio)
                
                // Sinalização visual crítica: LED vermelho (máxima urgência)
                exibir_cor_matriz(255, 0, 0);
                
                printf("!!! SITUACAO CRITICA !!!\n");
                printf("Pressao: %.0f kPa\n", estado_atual.pressao);
                printf("Temperatura: %.0f C\n", estado_atual.temperatura);
                printf("Nivel: %.0f%%\n", estado_atual.nivel_agua);
                printf("Aquecedor: %s\n", estado_atual.aquecedor ? "Ligado" : "Desligado");
                printf("Bomba: %s\n", estado_atual.bomba ? "Ligado" : "Desligado");
                printf("Alivio: %s\n", estado_atual.alivio ? "Ligado" : "Desligado");
                
                // Protocolo de emergência: 5 segundos com preempção natural do RTOS
                // Durante vTaskDelay(), outras tarefas podem executar brevemente
                // mas esta tarefa retoma controle por sua prioridade máxima
                for (int i = 5; i > 0; i--) {
                    printf(">>> EMERGENCIA: %d segundos restantes <<<\n", i);
                    vTaskDelay(pdMS_TO_TICKS(1000));    // Permite preempção durante delay
                    
                    // Reafirma controle crítico após possível preempção
                    exibir_cor_matriz(255, 0, 0);      // Reforça sinalização vermelha
                }
                
                printf("=== EMERGENCIA FINALIZADA AUTOMATICAMENTE (5s) ===\n");
                
                // Retorno automático para operação segura
                estado_caldeira_t estado_ok = CALDEIRA_OK;
                xQueueSend(xEstadoQueue, &estado_ok, 0);
                
            } else {
                // Rejeita comando não-crítico: retorna à fila
                xQueueSend(xEstadoQueue, &estado_recebido, 0);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(200));             // Período mínimo para emergência
    }
}

// Tarefa de Interface Visual: Atualização Contínua do Display
// Prioridade 1 (baixa): não interfere no controle crítico da caldeira
void tarefa_display(void *pvParameters) {
    while (true) {
        atualizar_display(&estado_atual);          // Renderiza telemetria atual
        vTaskDelay(pdMS_TO_TICKS(1000));           // Período de atualização: 1s
    }
}

// =============================================================================
// FUNÇÃO PRINCIPAL: INICIALIZAÇÃO E ORQUESTRAÇÃO DO SISTEMA
// =============================================================================

int main() {
    // Inicialização do sistema de comunicação UART para debug
    stdio_init_all();
    
    printf("\n=== SISTEMA DE CONTROLE DE CALDEIRA ===\n");
    printf("Inicializando componentes...\n");
    
    // Configuração do subsistema ADC para leitura do joystick analógico
    adc_init();                                     // Inicializa controlador ADC
    adc_gpio_init(VRX_PIN);                        // Configura GPIO 26 como entrada ADC
    adc_gpio_init(VRY_PIN);                        // Configura GPIO 27 como entrada ADC
    
    // Configuração do subsistema I2C para comunicação com display OLED
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);     // Inicializa I2C1 com clock configurado
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);     // Configura GPIO 14 como SDA
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);     // Configura GPIO 15 como SCL
    gpio_pull_up(SDA_PIN);                         // Resistor pull-up interno SDA
    gpio_pull_up(SCL_PIN);                         // Resistor pull-up interno SCL
    
    // Inicialização do display OLED SSD1306 e configuração de framebuffer
    ssd1306_init();
    calculate_render_area_buffer_length(&area_display);
    
    // Inicialização da matriz LED NeoPixel via PIO
    neopixel_init(LED_PIN);                        // Configura programa PIO
    neopixel_clear();                              // Apaga matriz inicial
    neopixel_write();                              // Efetiva estado apagado
    
    printf("Componentes inicializados com sucesso!\n");
    
    // Criação da fila de comunicação inter-tarefas thread-safe
    xEstadoQueue = xQueueCreate(10, sizeof(estado_caldeira_t));
    if (xEstadoQueue == NULL) {
        printf("Erro: Falha na criação da fila\n");
        while(1);  // Falha crítica: sistema não pode operar
    }
    
    // Criação das tarefas concorrentes com prioridades hierárquicas
    // Prioridades baseadas na criticidade dos estados da caldeira
    
    // Tarefa de entrada: detecção de comandos via joystick
    if (xTaskCreate(tarefa_joystick, "Joystick_Task", 512, NULL, 5, &xJoystickTaskHandle) != pdPASS) {
        printf("Erro: Falha na criação da tarefa do joystick\n");
        while(1);
    }
    
    // Tarefa de estado normal: operação segura padrão
    if (xTaskCreate(tarefa_caldeira_ok, "Caldeira_OK_Task", 512, NULL, 1, &xCaldeiraOKTaskHandle) != pdPASS) {
        printf("Erro: Falha na criação da tarefa Estado OK\n");
        while(1);
    }
    
    // Tarefa de estado crítico: nível de água insuficiente
    if (xTaskCreate(tarefa_caldeira_nivel, "Caldeira_Nivel_Task", 512, NULL, 2, &xCaldeiraNivelTaskHandle) != pdPASS) {
        printf("Erro: Falha na criação da tarefa Nível Baixo\n");
        while(1);
    }
    
    // Tarefa de estado crítico: superaquecimento do sistema
    if (xTaskCreate(tarefa_caldeira_temperatura, "Caldeira_Temp_Task", 512, NULL, 3, &xCaldeiraTempTaskHandle) != pdPASS) {
        printf("Erro: Falha na criação da tarefa Temperatura Alta\n");
        while(1);
    }
    
    // Tarefa de emergência: pressão crítica excessiva
    if (xTaskCreate(tarefa_caldeira_pressao, "Caldeira_Pressao_Task", 512, NULL, 4, &xCaldeiraPressaoTaskHandle) != pdPASS) {
        printf("Erro: Falha na criação da tarefa Pressão Alta\n");
        while(1);
    }
    
    // Tarefa de interface: atualização visual contínua
    if (xTaskCreate(tarefa_display, "Display_Task", 512, NULL, 1, &xDisplayTaskHandle) != pdPASS) {
        printf("Erro: Falha na criação da tarefa do display\n");
        while(1);
    }
    
    printf("Todas as tarefas criadas com sucesso!\n");
    printf("\n=== CONTROLES ===\n");
    printf("Joystick Direita: Estado OK (Verde)\n");
    printf("Joystick Esquerda: Nível Baixo (Amarelo)\n");
    printf("Joystick Baixo: Temperatura Alta (Laranja)\n");
    printf("Joystick Cima: Pressão Alta (Vermelho)\n");
    printf("==================\n\n");
    
    // Definição do estado inicial de operação segura
    estado_caldeira_t estado_inicial = CALDEIRA_OK;
    xQueueSend(xEstadoQueue, &estado_inicial, 0);   // Comando inicial para fila
    
    // Transferência de controle para escalonador FreeRTOS
    // A partir deste ponto, o sistema opera através das tarefas concorrentes
    vTaskStartScheduler();
    
    // Ponto crítico: nunca deve ser alcançado em operação normal
    printf("Erro: Escalonador parou inesperadamente\n");
    while(1);                                       // Loop de segurança infinito
} 