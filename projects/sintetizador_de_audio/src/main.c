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

// Projeto 2 - Sintetizador de Áudio
// Implementa um sintetizador de áudio para a BitDogLab com:
// - Gravação de áudio via ADC (microfone)
// - Reprodução de áudio via PWM (buzzer)
// - Controle por botões (A e B)
// - Feedback visual com LEDs RGB
// - Visualização no display OLED SSD1306
// Autor: Jorge Wilker Mamede de Andrade - 2025

// Includes do SDK da Pico
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

// Includes da biblioteca padrão
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Incluir todas as bibliotecas do projeto
#include "audio_pwm.h"
#include "buttons.h"
#include "led_rgb.h"
#include "ssd1306_i2c.h"

// Definições específicas para compatibilidade com display OLED
#ifndef SSD1306_WIDTH
#define SSD1306_WIDTH 128
#endif

#ifndef SSD1306_HEIGHT
#define SSD1306_HEIGHT 64
#endif

// Estados do sistema
typedef enum {
    SYSTEM_STARTUP,
    SYSTEM_IDLE,
    SYSTEM_RECORDING,
    SYSTEM_PLAYING,
    SYSTEM_ERROR
} system_state_t;

// Variáveis globais
static system_state_t current_system_state = SYSTEM_STARTUP;
static absolute_time_t last_display_update = {0};
static absolute_time_t recording_start_time = {0};
static uint32_t recording_duration = 0;

// Cores dos LEDs para cada estado
static const rgb_color_t LED_COLOR_IDLE = {0, 0, 1};      // Azul
static const rgb_color_t LED_COLOR_RECORDING = {1, 0, 0}; // Vermelho
static const rgb_color_t LED_COLOR_PLAYING = {0, 1, 0};   // Verde
static const rgb_color_t LED_COLOR_ERROR = {1, 1, 0};     // Amarelo
static const rgb_color_t LED_COLOR_OFF = {0, 0, 0};       // Desligado

// Protótipos das funções
void system_init(void);
void system_update(void);
void handle_button_events(void);
void update_display(void);
void update_leds(void);
void show_startup_screen(void);
void show_error_message(const char* message);
void calculate_recording_time(void);

// Função principal
int main() {
    // Inicializar comunicação serial
    stdio_init_all();
    sleep_ms(1000);  // Aguardar estabilização
    
    printf("\n=== BitDogLab Sintetizador de Áudio ===\n");
    printf("Iniciando sistema...\n");
    
    // Inicializar todos os subsistemas
    system_init();
    
    // Mostrar tela de inicialização
    show_startup_screen();
    sleep_ms(2000);
    
    // Transição para estado idle
    current_system_state = SYSTEM_IDLE;
    printf("Sistema pronto!\n");
    
    // Loop principal
    while (true) {
        system_update();
        sleep_ms(10);  // Pequeno delay para evitar uso excessivo de CPU
    }
    
    return 0;
}

// Inicializar todos os subsistemas
void system_init(void) {
    printf("Inicializando subsistemas:\n");
    
    // Inicializar botões
    printf("- Botões... ");
    buttons_init();
    printf("OK\n");
    
    // Inicializar LEDs RGB
    printf("- LEDs RGB... ");
    led_rgb_init();
    led_rgb_set_color(LED_COLOR_OFF);
    printf("OK\n");
    
    // Inicializar display OLED
    printf("- Display OLED... ");
    if (ssd1306_init()) {
        printf("OK\n");
    } else {
        printf("ERRO\n");
        current_system_state = SYSTEM_ERROR;
        return;
    }
    
    // Inicializar sistema de áudio
    printf("- Sistema de áudio... ");
    audio_init();
    printf("OK\n");
    
    // Teste inicial dos LEDs
    printf("- Testando LEDs... ");
    led_rgb_test_sequence(200);
    printf("OK\n");
    
    printf("Inicialização concluída!\n");
}

// Atualizar sistema principal
void system_update(void) {
    // Atualizar estado dos botões
    buttons_update();
    
    // Processar eventos dos botões
    handle_button_events();
    
    // Atualizar callback de áudio para gravação
    if (current_system_state == SYSTEM_RECORDING) {
        audio_timer_callback();
        calculate_recording_time();
        
        // Visualizar forma de onda em tempo real durante gravação
        // Ler valor atual do ADC para visualização
        uint16_t adc_sample = adc_read();
        ssd1306_draw_waveform(adc_sample);
        ssd1306_display();  // Atualizar display com nova amostra
    }
    
    // Atualizar display periodicamente (a cada 100ms)
    absolute_time_t now = get_absolute_time();
    if (absolute_time_diff_us(last_display_update, now) >= 100000) {
        if (current_system_state != SYSTEM_RECORDING) {  // Não sobrescrever visualização da forma de onda
            update_display();
        }
        update_leds();
        last_display_update = now;
    }
}

// Processar eventos dos botões
void handle_button_events(void) {
    bool button_a = button_a_pressed();
    bool button_b = button_b_pressed();
    bool both_buttons = button_a_held() && button_b_held();
    
    // Verificar se ambos os botões estão pressionados (limpar buffer)
    if (both_buttons && current_system_state == SYSTEM_IDLE) {
        printf("Limpando buffer de áudio...\n");
        audio_clear_buffer();
        ssd1306_clear();
        ssd1306_draw_string_centered(20, "BUFFER LIMPO", true);
        ssd1306_display();
        sleep_ms(1000);
        return;
    }
    
    // Processar botão A (gravação)
    if (button_a) {
        switch (current_system_state) {
            case SYSTEM_IDLE:
                if (audio_start_recording()) {
                    current_system_state = SYSTEM_RECORDING;
                    recording_start_time = get_absolute_time();
                    recording_duration = 0;
                    
                    // Inicializar visualização da forma de onda
                    ssd1306_waveform_init();
                    printf("Gravação iniciada com visualização da forma de onda\n");
                } else {
                    show_error_message("Erro ao iniciar gravacao");
                }
                break;
                
            case SYSTEM_RECORDING:
                printf("Parando gravação...\n");
                audio_stop_recording();
                current_system_state = SYSTEM_IDLE;
                printf("Gravação finalizada - %d amostras\n", audio_get_buffer_usage());
                break;
                
            case SYSTEM_PLAYING:
                // Não fazer nada durante reprodução
                break;
                
            default:
                break;
        }
    }
    
    // Processar botão B (reprodução)
    if (button_b) {
        switch (current_system_state) {
            case SYSTEM_IDLE:
                if (audio_get_buffer_usage() > 0) {
                    printf("Iniciando reprodução...\n");
                    if (audio_start_playback()) {
                        current_system_state = SYSTEM_PLAYING;
                    } else {
                        show_error_message("Erro ao iniciar reproducao");
                    }
                } else {
                    show_error_message("Nenhum audio gravado");
                    sleep_ms(1500);
                }
                break;
                
            case SYSTEM_PLAYING:
                printf("Parando reprodução...\n");
                audio_stop_playback();
                current_system_state = SYSTEM_IDLE;
                break;
                
            case SYSTEM_RECORDING:
                // Não fazer nada durante gravação
                break;
                
            default:
                break;
        }
    }
    
    // Verificar se reprodução terminou automaticamente
    if (current_system_state == SYSTEM_PLAYING && !audio_is_playing()) {
        current_system_state = SYSTEM_IDLE;
        printf("Reprodução finalizada\n");
    }
}

// Atualizar display OLED
void update_display(void) {
    ssd1306_clear();
    
    switch (current_system_state) {
        case SYSTEM_STARTUP:
            ssd1306_draw_string_centered(20, "INICIALIZANDO...", true);
            break;
            
        case SYSTEM_IDLE:
            ssd1306_draw_string_centered(5, "MENU PRINCIPAL", true);
            ssd1306_draw_line(0, 15, SSD1306_WIDTH - 1, 15, true);
            ssd1306_draw_string(10, 25, "A - INICIAR GRAVACAO", true);
            ssd1306_draw_string(10, 35, "B - REPRODUZIR AUDIO", true);
            ssd1306_draw_string(10, 45, "A+B - LIMPAR BUFFER", true);
            
            // Mostrar informações do buffer se houver áudio gravado
            if (audio_get_buffer_usage() > 0) {
                char buffer_info[32];
                float duration = audio_get_recording_time();
                snprintf(buffer_info, sizeof(buffer_info), "AUDIO: %.1fS", duration);
                ssd1306_draw_string(10, 55, buffer_info, true);
            }
            break;
            
        case SYSTEM_RECORDING:
            ssd1306_draw_string_centered(0, "GRAVANDO", true);
            ssd1306_draw_line(0, 10, SSD1306_WIDTH - 1, 10, true);
            
            // Mostrar tempo de gravação
            char time_str[16];
            uint32_t minutes = recording_duration / 60;
            uint32_t seconds = recording_duration % 60;
            snprintf(time_str, sizeof(time_str), "%02d:%02d", minutes, seconds);
            ssd1306_draw_string_centered(20, time_str, true);
            
            // Mostrar indicador de gravação
            ssd1306_fill_rect(50, 39, 28, 15, false);
            ssd1306_draw_string_centered(32, "A - PARAR", true);
            
            // Barra de progresso do buffer
            uint32_t buffer_usage = audio_get_buffer_usage();
            uint8_t progress = (buffer_usage * 120) / AUDIO_BUFFER_SIZE;
            ssd1306_fill_rect(4, 55, progress, 6, true);
            ssd1306_draw_rect(3, 54, 122, 8, true);
            break;
            
        case SYSTEM_PLAYING:
            ssd1306_draw_string_centered(0, "REPRODUZINDO", true);
            ssd1306_draw_line(0, 10, SSD1306_WIDTH - 1, 10, true);
            
            ssd1306_draw_string_centered(30, "B - PARAR", true);
            
            // Mostrar informações do áudio
            float duration = audio_get_recording_time();
            char info_str[32];
            snprintf(info_str, sizeof(info_str), "DURACAO: %.1fS", duration);
            ssd1306_draw_string_centered(45, info_str, true);
            break;
            
        case SYSTEM_ERROR:
            ssd1306_draw_string_centered(20, "ERRO DO SISTEMA", true);
            ssd1306_draw_string_centered(30, "REINICIE O DEVICE", true);
            break;
    }
    
    ssd1306_display();
}

// Atualizar LEDs RGB
void update_leds(void) {
    static bool is_blinking = false;
    static uint32_t last_blink = 0;
    static bool blink_state = false;
    uint32_t now = to_ms_since_boot(get_absolute_time());

    switch (current_system_state) {
        case SYSTEM_STARTUP:
            // LED off durante inicialização
            led_rgb_off();
            is_blinking = false;
            break;
            
        case SYSTEM_IDLE:
            led_rgb_set_color(LED_COLOR_IDLE);  // Azul fixo
            is_blinking = false;
            break;
            
        case SYSTEM_RECORDING:
            // LED vermelho piscando
            if (!is_blinking || (now - last_blink >= 500)) {
                blink_state = !blink_state;
                if (blink_state) {
                    led_rgb_set_color(LED_COLOR_RECORDING);
                } else {
                    led_rgb_off();
                }
                last_blink = now;
                is_blinking = true;
            }
            break;
            
        case SYSTEM_PLAYING:
            led_rgb_set_color(LED_COLOR_PLAYING);  // Verde fixo
            is_blinking = false;
            break;
            
        case SYSTEM_ERROR:
            // LED amarelo piscando rápido
            if (!is_blinking || (now - last_blink >= 200)) {
                blink_state = !blink_state;
                if (blink_state) {
                    led_rgb_set_color(LED_COLOR_ERROR);
                } else {
                    led_rgb_off();
                }
                last_blink = now;
                is_blinking = true;
            }
            break;
    }
}

// Mostrar tela de inicialização
void show_startup_screen(void) {
    ssd1306_clear();
    ssd1306_draw_string_centered(5, "BITDOGLAB", true);
    ssd1306_draw_string_centered(15, "SINTETIZADOR", true);
    ssd1306_draw_string_centered(25, "DE AUDIO", true);
    ssd1306_draw_line(20, 35, 108, 35, true);
    ssd1306_draw_string_centered(40, "V1.0", true);
    ssd1306_draw_string_centered(50, "JORGE WILKER", true);
    ssd1306_display();
}

// Mostrar mensagem de erro
void show_error_message(const char* message) {
    ssd1306_clear();
    ssd1306_draw_string_centered(10, "ERRO", true);
    ssd1306_draw_line(30, 20, 98, 20, true);
    
    // Converter a mensagem para maiúsculas
    char upper_message[32];
    size_t i;
    for (i = 0; message[i] && i < sizeof(upper_message)-1; i++) {
        upper_message[i] = (message[i] >= 'a' && message[i] <= 'z') ? 
                          message[i] - 32 : message[i];
    }
    upper_message[i] = '\0';
    
    ssd1306_draw_string_centered(25, upper_message, true);
    ssd1306_display();
    printf("Erro: %s\n", message);
}


// Calcular tempo de gravação

void calculate_recording_time(void) {
    absolute_time_t now = get_absolute_time();
    recording_duration = absolute_time_diff_us(recording_start_time, now) / 1000000;
} 
