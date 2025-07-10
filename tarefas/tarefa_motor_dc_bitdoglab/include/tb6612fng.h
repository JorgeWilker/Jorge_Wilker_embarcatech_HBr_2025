/*
 * Copyright (C) 2025 Jorge Wilker Mamede de Andrade
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

// Driver para controlador de motor TB6612FNG
// Implementa controle de 2 motores DC independentes com PWM e direção
// Configurado para plataforma BitDogLab com Raspberry Pi Pico
// Autor: Jorge Wilker Mamede de Andrade - 2025

#ifndef TB6612FNG_H
#define TB6612FNG_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

// Configuração de GPIOs conforme tabela de conexão BitDogLab
// Baseado no documento "Ponte H.md" fornecido

// Pinos de controle para Motor A
#define MOTOR_A_PWM_PIN     8    // GP8  - PWM A (controle de velocidade)
#define MOTOR_A_IN1_PIN     4    // GP4  - INA1 (direção)
#define MOTOR_A_IN2_PIN     9    // GP9  - INA2 (direção)

// Pinos de controle para Motor B  
#define MOTOR_B_PWM_PIN     16   // GP16 - PWM B (controle de velocidade)
#define MOTOR_B_IN1_PIN     18   // GP18 - INB1 (direção)
#define MOTOR_B_IN2_PIN     19   // GP19 - INB2 (direção)

// Pino de controle geral
#define MOTOR_STANDBY_PIN   20   // GP20 - Standby (habilita/desabilita driver)

// Configurações de PWM
#define PWM_FREQUENCY_HZ    1000  // Frequência PWM em Hz (1kHz)
#define PWM_MAX_DUTY_CYCLE  65535 // Valor máximo do duty cycle (16-bit)
#define PWM_RESOLUTION_BITS 16    // Resolução do PWM em bits

// Definições de direção dos motores
typedef enum {
    MOTOR_STOP = 0,     // Motor parado (freio elétrico)
    MOTOR_FORWARD,      // Motor girando para frente
    MOTOR_BACKWARD,     // Motor girando para trás
    MOTOR_BRAKE         // Freio ativo (curto-circuito controlado)
} motor_direction_t;

// Identificação dos motores
typedef enum {
    MOTOR_A = 0,        // Motor A (primeiro motor)
    MOTOR_B = 1         // Motor B (segundo motor)
} motor_id_t;

// Estrutura para configuração de um motor
typedef struct {
    uint8_t pwm_pin;    // Pino PWM
    uint8_t in1_pin;    // Pino IN1
    uint8_t in2_pin;    // Pino IN2
    uint8_t pwm_slice;  // Slice PWM do RP2040
    uint8_t pwm_channel; // Canal PWM do RP2040
} motor_config_t;

// Estrutura para controle completo do TB6612FNG
typedef struct {
    motor_config_t motor_a;     // Configuração do Motor A
    motor_config_t motor_b;     // Configuração do Motor B
    uint8_t standby_pin;        // Pino de standby
    bool initialized;           // Flag de inicialização
} tb6612fng_t;

/**
 * @brief Inicializa o driver TB6612FNG
 * @param driver Ponteiro para estrutura do driver
 * @return true se inicialização foi bem-sucedida
 */
bool tb6612fng_init(tb6612fng_t* driver);

/**
 * @brief Habilita ou desabilita o driver TB6612FNG
 * @param driver Ponteiro para estrutura do driver
 * @param enable true para habilitar, false para standby
 */
void tb6612fng_enable(tb6612fng_t* driver, bool enable);

/**
 * @brief Configura a direção de um motor
 * @param driver Ponteiro para estrutura do driver
 * @param motor_id Identificação do motor (MOTOR_A ou MOTOR_B)
 * @param direction Direção do motor
 */
void tb6612fng_set_direction(tb6612fng_t* driver, motor_id_t motor_id, motor_direction_t direction);

/**
 * @brief Configura a velocidade de um motor (duty cycle PWM)
 * @param driver Ponteiro para estrutura do driver
 * @param motor_id Identificação do motor (MOTOR_A ou MOTOR_B)
 * @param speed Velocidade (0-100% como 0-100)
 */
void tb6612fng_set_speed(tb6612fng_t* driver, motor_id_t motor_id, uint8_t speed);

/**
 * @brief Controla um motor com direção e velocidade
 * @param driver Ponteiro para estrutura do driver
 * @param motor_id Identificação do motor
 * @param direction Direção do motor
 * @param speed Velocidade (0-100%)
 */
void tb6612fng_control_motor(tb6612fng_t* driver, motor_id_t motor_id, 
                           motor_direction_t direction, uint8_t speed);

/**
 * @brief Para todos os motores (freio elétrico)
 * @param driver Ponteiro para estrutura do driver
 */
void tb6612fng_stop_all_motors(tb6612fng_t* driver);

/**
 * @brief Controla ambos os motores simultaneamente
 * @param driver Ponteiro para estrutura do driver
 * @param dir_a Direção do Motor A
 * @param speed_a Velocidade do Motor A (0-100%)
 * @param dir_b Direção do Motor B
 * @param speed_b Velocidade do Motor B (0-100%)
 */
void tb6612fng_control_both_motors(tb6612fng_t* driver, 
                                 motor_direction_t dir_a, uint8_t speed_a,
                                 motor_direction_t dir_b, uint8_t speed_b);

/**
 * @brief Obtém o status atual do driver
 * @param driver Ponteiro para estrutura do driver
 * @return true se driver está inicializado e habilitado
 */
bool tb6612fng_is_ready(tb6612fng_t* driver);

#endif // TB6612FNG_H 