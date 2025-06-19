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

#include "buttons.h"
#include <stdio.h>

// Variáveis globais para controle dos botões
static button_t button_a;
static button_t button_b;

void buttons_init(void) {
    // Configurar botão A
    button_a.gpio_pin = BUTTON_A_PIN;
    button_a.current_state = BUTTON_RELEASED;
    button_a.previous_state = BUTTON_RELEASED;
    button_a.last_change_time = get_absolute_time();
    button_a.debounced = true;
    
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);  // Pull-up interno
    
    // Configurar botão B
    button_b.gpio_pin = BUTTON_B_PIN;
    button_b.current_state = BUTTON_RELEASED;
    button_b.previous_state = BUTTON_RELEASED;
    button_b.last_change_time = get_absolute_time();
    button_b.debounced = true;
    
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);  // Pull-up interno
    
    printf("Botões inicializados\n");
}

void buttons_update(void) {
    absolute_time_t current_time = get_absolute_time();
    
    // Atualizar botão A
    bool button_a_raw = !gpio_get(BUTTON_A_PIN);  // Invertido por causa do pull-up
    
    if (button_a_raw != (button_a.current_state == BUTTON_PRESSED)) {
        if (absolute_time_diff_us(button_a.last_change_time, current_time) >= (DEBOUNCE_TIME_MS * 1000)) {
            button_a.previous_state = button_a.current_state;
            button_a.current_state = button_a_raw ? BUTTON_PRESSED : BUTTON_RELEASED;
            button_a.last_change_time = current_time;
            button_a.debounced = true;
        }
    }
    
    // Atualizar botão B
    bool button_b_raw = !gpio_get(BUTTON_B_PIN);  // Invertido por causa do pull-up
    
    if (button_b_raw != (button_b.current_state == BUTTON_PRESSED)) {
        if (absolute_time_diff_us(button_b.last_change_time, current_time) >= (DEBOUNCE_TIME_MS * 1000)) {
            button_b.previous_state = button_b.current_state;
            button_b.current_state = button_b_raw ? BUTTON_PRESSED : BUTTON_RELEASED;
            button_b.last_change_time = current_time;
            button_b.debounced = true;
        }
    }
}

bool button_a_pressed(void) {
    return is_button_just_pressed(&button_a);
}

bool button_b_pressed(void) {
    return is_button_just_pressed(&button_b);
}

bool button_a_released(void) {
    return is_button_just_released(&button_a);
}

bool button_b_released(void) {
    return is_button_just_released(&button_b);
}

bool button_a_held(void) {
    return button_a.current_state == BUTTON_PRESSED;
}

bool button_b_held(void) {
    return button_b.current_state == BUTTON_PRESSED;
}

button_state_t get_button_state(uint gpio_pin) {
    if (gpio_pin == BUTTON_A_PIN) {
        return button_a.current_state;
    } else if (gpio_pin == BUTTON_B_PIN) {
        return button_b.current_state;
    }
    return BUTTON_RELEASED;
}

bool is_button_just_pressed(button_t* button) {
    if (button->debounced && 
        button->previous_state == BUTTON_RELEASED && 
        button->current_state == BUTTON_PRESSED) {
        button->debounced = false;  // Consumir o evento
        return true;
    }
    return false;
}

bool is_button_just_released(button_t* button) {
    if (button->debounced && 
        button->previous_state == BUTTON_PRESSED && 
        button->current_state == BUTTON_RELEASED) {
        button->debounced = false;  // Consumir o evento
        return true;
    }
    return false;
} 