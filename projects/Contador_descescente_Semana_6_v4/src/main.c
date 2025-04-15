#include <stdio.h>
#include <string.h>
#include <stdlib.h> // Adicionado para calloc na lib ssd1306
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/i2c.h"

// --- Inclusão da Biblioteca OLED ---
#include "ssd1306.h"

// --- Configurações ---
#define BUTTON_A_PIN 5 // Pino para o Botão A (inicia/reinicia contagem)
#define BUTTON_B_PIN 6 // Pino para o Botão B (registra cliques durante contagem)
#define DEBOUNCE_TIME_MS 200 // Tempo (ms) para debounce dos botões

// --- Configurações do OLED ---
#define I2C_PORT i2c1
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15
#define OLED_ADDR 0x3C // Endereço I2C comum para SSD1306
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// Protótipos de funções da biblioteca OLED (exemplo)
// void oled_init(i2c_inst_t *i2c, uint8_t address);
// void oled_clear();
// void oled_draw_string(int x, int y, int font_size, char *text);
// void oled_show();

// --- Variáveis Globais ---
volatile int countdown_value = 0;
volatile int button_b_presses = 0;
volatile bool counting_active = false;
volatile uint32_t last_a_press_time = 0;
volatile uint32_t last_b_press_time = 0;

// Instância da estrutura do display OLED
ssd1306_t oled_display;

struct repeating_timer countdown_timer;

// --- Funções ---

// Atualiza o display OLED com os valores atuais
void update_oled_display() {
    char line1[20];
    char line2[20];

    sprintf(line1, "cont: %d", countdown_value);
    sprintf(line2, "bot b:%d", button_b_presses);

    // Atualiza o buffer do display
    ssd1306_fill(&oled_display, false); // Limpa o buffer (fundo preto)
    ssd1306_draw_string_large(&oled_display, line1, 0, 0); // Linha 1 (Contador)
    ssd1306_draw_string_large(&oled_display, line2, 0, 20); // Linha 2 (Botao B)
    ssd1306_send_data(&oled_display); // Envia o buffer para o display físico

    // Saída de depuração no terminal serial
    printf("Atualizacao do display -> Cont: %d, bot b: %d, Contando: %s\n", countdown_value, button_b_presses, counting_active ? "Sim" : "Nao");
}

// Callback do Timer (executado a cada 1 segundo)
bool repeating_timer_callback(struct repeating_timer *t) {
    if (counting_active) {
        if (countdown_value > 0) {
            countdown_value--;
            update_oled_display();
        }
        if (countdown_value == 0) {
            counting_active = false;
            update_oled_display(); // Mostra o estado final
            printf("Contagem finalizada.\n");
            return false; // Para o timer
        }
    }
    return true; // Continua o timer
}

// Callback de Interrupção GPIO (para ambos os botões)
void gpio_callback(uint gpio, uint32_t events) {
    uint32_t now = time_us_32();

    // Tratamento Botão A
    if (gpio == BUTTON_A_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
        if ((now - last_a_press_time) > (DEBOUNCE_TIME_MS * 1000)) { // Debounce
            last_a_press_time = now;
            printf("Botao A pressionado!\n");

            cancel_repeating_timer(&countdown_timer); // Cancela timer anterior

            // Reinicia estado da contagem
            countdown_value = 9;
            button_b_presses = 0;
            counting_active = true;

            // Inicia novo timer de 1 segundo
            add_repeating_timer_ms(-1000, repeating_timer_callback, NULL, &countdown_timer);

            update_oled_display(); // Atualiza display com estado inicial
        }
    // Tratamento Botão B
    } else if (gpio == BUTTON_B_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
         if ((now - last_b_press_time) > (DEBOUNCE_TIME_MS * 1000)) { // Debounce
            last_b_press_time = now;
             if (counting_active) {
                button_b_presses++;
                printf("Botao B pressionado durante contagem! Total: %d\n", button_b_presses);
                update_oled_display(); // Atualiza display com novo total
             } else {
                 printf("Botao B pressionado fora da contagem (ignorado).\n");
             }
         }
    }
}

// Configuração dos GPIOs para os botões
void setup_gpio() {
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN); // Habilita pull-up interno

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN); // Habilita pull-up interno

    // Configura interrupções para borda de descida (botão pressionado)
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
}

// Configuração do I2C e inicialização do OLED
void setup_oled() {
    printf("Configurando I2C e OLED nos pinos SDA=%d, SCL=%d...\n", I2C_SDA_PIN, I2C_SCL_PIN);

    // Inicializa I2C
    i2c_init(I2C_PORT, 400 * 1000); // Define velocidade I2C para 400kHz
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // Inicializa a biblioteca OLED
    ssd1306_init(&oled_display, OLED_WIDTH, OLED_HEIGHT, false, OLED_ADDR, I2C_PORT);
    ssd1306_config(&oled_display); // Envia comandos de configuração para o display
    ssd1306_fill(&oled_display, false); // Limpa o buffer do display

    // Mostra mensagem inicial no OLED
    ssd1306_draw_string(&oled_display, "Pressione A", 0, 0); // Usa fonte pequena (8x8)
    ssd1306_send_data(&oled_display);

    printf("OLED inicializado. Pressione Botao A.\n");
}

// --- Função Principal ---
int main() {
    stdio_init_all(); // Inicializa STDIO (necessário para printf)
    sleep_ms(2000); // Pausa inicial para estabilização e conexão serial

    printf("Iniciando Contador Decrescente...\n");

    setup_gpio();
    setup_oled();

    printf("Sistema pronto. Aguardando Botao A...\n");

    // Loop principal vazio; a lógica é controlada por interrupções e timers
    while (1) {
        tight_loop_contents(); // Mantém o processador ocupado ou __wfi(); para baixo consumo
    }
}