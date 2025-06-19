/*
 * Copyright (C) 2025 Jorge Wilker
 *
 * Este programa é software livre: você pode redistribuí-lo e/ou modificá-lo
 * sob os termos da GNU General Public License conforme publicada pela
 * Free Software Foundation, tanto a versão 3 da Licença, ou
 * (a seu critério) qualquer versão posterior.
 *
 * Este programa é distribuído na esperança de que seja útil,
 * mas SEM QUALQUER GARANTIA; sem mesmo a garantia implícita de
 * COMERCIALIZAÇÃO ou ADEQUAÇÃO A UM DETERMINADO FIM. Veja o
 * GNU General Public License para mais detalhes.
 *
 * Você deve ter recebido uma cópia da GNU General Public License
 * junto com este programa. Se não, veja <https://www.gnu.org/licenses/>.
 */

#include "led_rgb.h"
#include "pico/time.h"
#include <stdio.h>

// Estado atual do LED RGB
static rgb_color_t current_color = COLOR_OFF;

// Inicializar LED RGB
void led_rgb_init(void) {
    printf("Inicializando LED RGB...\n");
    
    // Configurar GPIOs como saída
    gpio_init(LED_RED_PIN);
    gpio_init(LED_GREEN_PIN);
    gpio_init(LED_BLUE_PIN);
    
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    
    // Desligar todos os LEDs inicialmente
    led_rgb_off();
    
    printf("LED RGB inicializado\n");
}

// Definir cor do LED RGB
void led_rgb_set_color(rgb_color_t color) {
    current_color = color;
    
    // Aplicar os estados aos pinos
    gpio_put(LED_RED_PIN, color.red);
    gpio_put(LED_GREEN_PIN, color.green);
    gpio_put(LED_BLUE_PIN, color.blue);
}

// Obter cor atual do LED RGB
rgb_color_t led_rgb_get_color(void) {
    return current_color;
}

// Piscar LED com cor específica
void led_rgb_blink(rgb_color_t color, uint32_t interval_ms) {
    led_rgb_set_color(color);
    sleep_ms(interval_ms);
    led_rgb_off();
    sleep_ms(interval_ms);
}

// Controle individual - LED vermelho
void led_set_red(bool state) {
    current_color.red = state;
    gpio_put(LED_RED_PIN, state);
}

// Controle individual - LED verde
void led_set_green(bool state) {
    current_color.green = state;
    gpio_put(LED_GREEN_PIN, state);
}

// Controle individual - LED azul
void led_set_blue(bool state) {
    current_color.blue = state;
    gpio_put(LED_BLUE_PIN, state);
}

// Desligar LED RGB
void led_rgb_off(void) {
    led_rgb_set_color(COLOR_OFF);
}

// Criar cor RGB
rgb_color_t led_rgb_create_color(bool red, bool green, bool blue) {
    rgb_color_t color = {red, green, blue};
    return color;
}

// Feedback visual baseado no estado do áudio
void led_rgb_set_audio_feedback(audio_state_t audio_state) {
    switch (audio_state) {
        case AUDIO_IDLE:
            led_rgb_set_color(COLOR_BLUE);  // Azul quando idle
            break;
            
        case AUDIO_RECORDING:
            led_rgb_set_color(COLOR_RED);  // Vermelho durante gravação
            break;
            
        case AUDIO_PLAYING:
            led_rgb_set_color(COLOR_GREEN);  // Verde durante reprodução
            break;
            
        default:
            led_rgb_set_color(COLOR_OFF);
            break;
    }
}

// Visualização de nível de áudio através da cor
void led_rgb_set_audio_level(uint16_t level) {
    // Converter nível (0-65535) para faixas de cor
    if (level < 8000) {
        led_rgb_set_color(COLOR_BLUE);      // Baixo
    } else if (level < 20000) {
        led_rgb_set_color(COLOR_GREEN);     // Médio
    } else if (level < 40000) {
        led_rgb_set_color(COLOR_YELLOW);    // Alto
    } else {
        led_rgb_set_color(COLOR_RED);       // Muito alto
    }
}

// Sequência de teste do LED RGB
void led_rgb_test_sequence(uint32_t delay_ms) {
    printf("Iniciando teste do LED RGB...\n");
    
    // Teste individual de cada cor
    printf("Vermelho...\n");
    led_rgb_set_color(COLOR_RED);
    sleep_ms(delay_ms);
    
    printf("Verde...\n");
    led_rgb_set_color(COLOR_GREEN);
    sleep_ms(delay_ms);
    
    printf("Azul...\n");
    led_rgb_set_color(COLOR_BLUE);
    sleep_ms(delay_ms);
    
    printf("Amarelo...\n");
    led_rgb_set_color(COLOR_YELLOW);
    sleep_ms(delay_ms);
    
    printf("Magenta...\n");
    led_rgb_set_color(COLOR_MAGENTA);
    sleep_ms(delay_ms);
    
    printf("Ciano...\n");
    led_rgb_set_color(COLOR_CYAN);
    sleep_ms(delay_ms);
    
    printf("Branco...\n");
    led_rgb_set_color(COLOR_WHITE);
    sleep_ms(delay_ms);
    
    // Desligar
    printf("Desligando...\n");
    led_rgb_set_color(COLOR_OFF);
    
    printf("Teste do LED RGB concluído\n");
}

// Efeito de fade para uma cor específica
void led_rgb_fade_to_color(rgb_color_t target_color, uint32_t duration_ms) {
    // Implementação simplificada: piscar na cor alvo
    uint32_t steps = duration_ms / 100;  // Passos de 100ms
    
    for (uint32_t i = 0; i < steps; i++) {
        led_rgb_set_color(target_color);
        sleep_ms(50);
        led_rgb_off();
        sleep_ms(50);
    }
    
    // Finalizar na cor alvo
    led_rgb_set_color(target_color);
} 