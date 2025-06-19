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

// Módulo de controle do LED RGB da BitDogLab
// Implementa controle do LED RGB permitindo feedback visual
// para diferentes estados do sistema com cores personalizadas
// Autor: Jorge Wilker Mamede de Andrade - 2025

#ifndef LED_RGB_H
#define LED_RGB_H

#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Forward declaration para evitar dependência circular
#ifndef AUDIO_STATE_T_DEFINED
#define AUDIO_STATE_T_DEFINED
typedef enum {
    AUDIO_IDLE,
    AUDIO_RECORDING,
    AUDIO_PLAYING
} audio_state_t;
#endif

// Configurações do LED RGB da BitDogLab
#define LED_R_PIN 11    // GPIO 11 - LED Vermelho
#define LED_G_PIN 12    // GPIO 12 - LED Verde  
#define LED_B_PIN 13    // GPIO 13 - LED Azul

// Compatibilidade com nomes alternativos
#define LED_RED_PIN   LED_R_PIN
#define LED_GREEN_PIN LED_G_PIN
#define LED_BLUE_PIN  LED_B_PIN

// Estrutura para controle de cor RGB
typedef struct {
    bool red;   // Estado do LED vermelho
    bool green; // Estado do LED verde
    bool blue;  // Estado do LED azul
} rgb_color_t;

// Cores predefinidas
#define COLOR_OFF     ((rgb_color_t){0, 0, 0})  // Desligado
#define COLOR_RED     ((rgb_color_t){1, 0, 0})  // Vermelho
#define COLOR_GREEN   ((rgb_color_t){0, 1, 0})  // Verde
#define COLOR_BLUE    ((rgb_color_t){0, 0, 1})  // Azul
#define COLOR_YELLOW  ((rgb_color_t){1, 1, 0})  // Amarelo (R+G)
#define COLOR_MAGENTA ((rgb_color_t){1, 0, 1})  // Magenta (R+B)
#define COLOR_CYAN    ((rgb_color_t){0, 1, 1})  // Ciano (G+B)
#define COLOR_WHITE   ((rgb_color_t){1, 1, 1})  // Branco (R+G+B)

// Inicialização do LED RGB
void led_rgb_init(void);

// Funções de controle RGB
void led_rgb_set_color(rgb_color_t color);
rgb_color_t led_rgb_get_color(void);
void led_rgb_blink(rgb_color_t color, uint32_t interval_ms);

// Funções de controle individual
void led_set_red(bool state);
void led_set_green(bool state);
void led_set_blue(bool state);

// Funções de utilidade
void led_rgb_off(void);
rgb_color_t led_rgb_create_color(bool red, bool green, bool blue);

// Funções para feedback visual do estado do áudio
void led_rgb_set_audio_feedback(audio_state_t audio_state);
void led_rgb_set_audio_level(uint16_t level);

// Funções de teste e efeitos
void led_rgb_test_sequence(uint32_t delay_ms);
void led_rgb_fade_to_color(rgb_color_t target_color, uint32_t duration_ms);

#endif // LED_RGB_H 