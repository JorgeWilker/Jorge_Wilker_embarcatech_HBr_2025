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

// Sistema de controle de botões com debounce
// Implementa interface de controle para botões de entrada digital
// com algoritmo de debounce integrado e detecção de eventos
// Autor: Jorge Wilker Mamede de Andrade - 2025

#ifndef BUTTONS_H
#define BUTTONS_H

#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Configurações do hardware de entrada
#define BUTTON_A_PIN 5           // GPIO para botão A da placa
#define BUTTON_B_PIN 6           // GPIO para botão B da placa
#define DEBOUNCE_TIME_MS 50      // Período de debounce em milissegundos

// Estados operacionais do botão
typedef enum {
    BUTTON_RELEASED,    // Botão em estado não pressionado
    BUTTON_PRESSED,     // Botão recém pressionado (evento único)
    BUTTON_HELD         // Botão mantido pressionado (estado contínuo)
} button_state_t;

// Estrutura de controle de botão
typedef struct {
    uint gpio_pin;                    // Número do pino GPIO associado
    button_state_t current_state;     // Estado atual após debounce
    button_state_t previous_state;    // Estado anterior para detecção de mudanças
    absolute_time_t last_change_time; // Timestamp da última mudança de estado
    bool debounced;                   // Flag de conclusão do período de debounce
} button_t;

// Inicializa sistema de controle de botões
void buttons_init(void);

// Verifica se botão A foi pressionado
bool button_a_pressed(void);

// Verifica se botão B foi pressionado
bool button_b_pressed(void);

// Verifica se botão A foi liberado
bool button_a_released(void);

// Verifica se botão B foi liberado
bool button_b_released(void);

// Verifica se botão A está sendo mantido pressionado
bool button_a_held(void);

// Verifica se botão B está sendo mantido pressionado
bool button_b_held(void);

// Atualiza estado de todos os botões
void buttons_update(void);

// Obtém estado atual de um botão específico
button_state_t get_button_state(uint gpio_pin);

// Detecta evento de pressionamento
bool is_button_just_pressed(button_t* button);

// Detecta evento de liberação
bool is_button_just_released(button_t* button);

#endif // BUTTONS_H 