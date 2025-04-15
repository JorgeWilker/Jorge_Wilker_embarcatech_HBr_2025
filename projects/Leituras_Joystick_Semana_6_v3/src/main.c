#include <stdio.h>             
#include "pico/stdlib.h"          
#include "hardware/adc.h"           
#include "hardware/i2c.h"           
#include "ssd1306.h"              
#include "font.h"                 

// --- Configurações do Hardware ---

#define JOYSTICK_X_CHANNEL 1  // Canal ADC para o eixo X (ADC1 = GPIO 27)
#define JOYSTICK_Y_CHANNEL 0  // Canal ADC para o eixo Y (ADC0 = GPIO 26)

#define I2C_PORT i2c1           // Instância I2C a ser usada (i2c1)
#define I2C_SDA_PIN 14          // Pino GPIO para I2C SDA
#define I2C_SCL_PIN 15          // Pino GPIO para I2C SCL

#define SSD1306_I2C_ADDR 0x3C   // Endereço I2C do display OLED

#define ADC_CENTER_X 1998       // Valor ADC central calibrado para o eixo X
#define ADC_CENTER_Y 2018       // Valor ADC central calibrado para o eixo Y

// --- Função Principal ---
int main() {
    stdio_init_all();           // Inicializa stdio (necessário para printf via USB/UART)
    sleep_ms(2000);             // Pausa inicial para estabilização da serial
    printf("Inicializando sistema...\n"); // Mensagem de início

    adc_init();                 // Inicializa o hardware ADC
    adc_gpio_init(26 + JOYSTICK_X_CHANNEL); // Configura pino GPIO do eixo X como entrada ADC
    adc_gpio_init(26 + JOYSTICK_Y_CHANNEL); // Configura pino GPIO do eixo Y como entrada ADC
    printf("ADC inicializado.\n");

    i2c_init(I2C_PORT, 400 * 1000); // Inicializa I2C1 com clock de 400kHz
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C); // Define função do pino SDA como I2C
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C); // Define função do pino SCL como I2C
    gpio_pull_up(I2C_SDA_PIN);      // Habilita pull-up interno no SDA
    gpio_pull_up(I2C_SCL_PIN);      // Habilita pull-up interno no SCL
    printf("I2C inicializado (Porta: %d, SDA: %d, SCL: %d).\n", I2C_PORT == i2c0 ? 0 : 1, I2C_SDA_PIN, I2C_SCL_PIN);

    ssd1306_t ssd;              // Declara a estrutura de controle do display
    ssd1306_init(&ssd, 128, 64, false, SSD1306_I2C_ADDR, I2C_PORT); // Inicializa a estrutura ssd1306
    ssd1306_config(&ssd);       // Envia comandos de configuração para o display
    ssd1306_fill(&ssd, false);  // Limpa o buffer local do display (preenche com 0)
    ssd1306_send_data(&ssd);    // Envia o buffer limpo para o display físico
    printf("Display OLED inicializado e limpo (Addr: 0x%X).\n", SSD1306_I2C_ADDR);

    char buffer[32];            // Buffer para formatar strings de texto

    printf("Entrando no loop principal.\n");
    while (true) {              // Loop infinito principal
        adc_select_input(JOYSTICK_X_CHANNEL); // Seleciona canal ADC do eixo X
        uint16_t x_value_raw = adc_read();      // Lê valor raw do ADC X (0-4095)

        adc_select_input(JOYSTICK_Y_CHANNEL); // Seleciona canal ADC do eixo Y
        uint16_t y_value_raw = adc_read();      // Lê valor raw do ADC Y (0-4095)
        
        // Calcula valores ajustados subtraindo o centro calibrado (resultado pode ser negativo)
        int x_adjusted = (int)x_value_raw - ADC_CENTER_X;
        int y_adjusted = (int)y_value_raw - ADC_CENTER_Y;
        
        // Opcional: Imprimir leituras no terminal serial para depuração
        // printf("ADC Raw(X:%d, Y:%d) Adj(X:%d, Y:%d)\n", x_value_raw, y_value_raw, x_adjusted, y_adjusted);
 
        ssd1306_fill(&ssd, false); // Limpa o buffer local antes de desenhar

        snprintf(buffer, sizeof(buffer), "X:%5d", x_adjusted); // Formata string do eixo X
        ssd1306_draw_string(&ssd, buffer, 0, 0); // Desenha string X no buffer local (Y=0)

        snprintf(buffer, sizeof(buffer), "Y:%5d", y_adjusted); // Formata string do eixo Y
        ssd1306_draw_string(&ssd, buffer, 0, 16);// Desenha string Y no buffer local (Y=16)

        ssd1306_send_data(&ssd);    // Envia o buffer local atualizado para o display OLED via I2C
        
        sleep_ms(200);              // Pausa antes da próxima iteração
    } 
} 
