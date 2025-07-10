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

// Implementação do driver para controlador de motor TB6612FNG
// Suporta controle de 2 motores DC independentes com PWM e direção
// Configurado para plataforma BitDogLab com Raspberry Pi Pico
// Autor: Jorge Wilker Mamede de Andrade - 2025

#include "tb6612fng.h"

// Configuração interna de um motor individual com inicialização de PWM
static void configure_motor_pins(motor_config_t* config, uint8_t pwm_pin, uint8_t in1_pin, uint8_t in2_pin) {
    config->pwm_pin = pwm_pin;
    config->in1_pin = in1_pin;
    config->in2_pin = in2_pin;
    
    // Configurar pinos de direção como saída
    gpio_init(in1_pin);
    gpio_set_dir(in1_pin, GPIO_OUT);
    gpio_put(in1_pin, false);
    
    gpio_init(in2_pin);
    gpio_set_dir(in2_pin, GPIO_OUT);
    gpio_put(in2_pin, false);
    
    // Configurar PWM
    gpio_set_function(pwm_pin, GPIO_FUNC_PWM);
    config->pwm_slice = pwm_gpio_to_slice_num(pwm_pin);
    config->pwm_channel = pwm_gpio_to_channel(pwm_pin);
    
    // Configurar frequência PWM
    pwm_set_clkdiv(config->pwm_slice, 125.0f); // 125MHz / 125 = 1MHz
    pwm_set_wrap(config->pwm_slice, PWM_MAX_DUTY_CYCLE); // 16-bit resolution
    pwm_set_chan_level(config->pwm_slice, config->pwm_channel, 0); // Iniciar parado
    pwm_set_enabled(config->pwm_slice, true);
}

// Aplica configuração de direção nos pinos IN1 e IN2 do motor
static void apply_direction(const motor_config_t* config, motor_direction_t direction) {
    switch (direction) {
        case MOTOR_STOP:
            gpio_put(config->in1_pin, false);
            gpio_put(config->in2_pin, false);
            break;
            
        case MOTOR_FORWARD:
            gpio_put(config->in1_pin, true);
            gpio_put(config->in2_pin, false);
            break;
            
        case MOTOR_BACKWARD:
            gpio_put(config->in1_pin, false);
            gpio_put(config->in2_pin, true);
            break;
            
        case MOTOR_BRAKE:
            gpio_put(config->in1_pin, true);
            gpio_put(config->in2_pin, true);
            break;
    }
}

// Aplica velocidade PWM no motor através de duty cycle proporcional
static void apply_speed(const motor_config_t* config, uint8_t speed) {
    // Limitar velocidade a 100%
    if (speed > 100) {
        speed = 100;
    }
    
    // Converter porcentagem para duty cycle de 16-bit
    uint16_t duty_cycle = (uint16_t)((speed * PWM_MAX_DUTY_CYCLE) / 100);
    
    pwm_set_chan_level(config->pwm_slice, config->pwm_channel, duty_cycle);
}

bool tb6612fng_init(tb6612fng_t* driver) {
    if (driver == NULL) {
        return false;
    }
    
    // Configurar pino de standby
    driver->standby_pin = MOTOR_STANDBY_PIN;
    gpio_init(driver->standby_pin);
    gpio_set_dir(driver->standby_pin, GPIO_OUT);
    gpio_put(driver->standby_pin, false); // Iniciar em standby
    
    // Configurar Motor A
    configure_motor_pins(&driver->motor_a, MOTOR_A_PWM_PIN, MOTOR_A_IN1_PIN, MOTOR_A_IN2_PIN);
    
    // Configurar Motor B
    configure_motor_pins(&driver->motor_b, MOTOR_B_PWM_PIN, MOTOR_B_IN1_PIN, MOTOR_B_IN2_PIN);
    
    // Parar todos os motores inicialmente
    tb6612fng_stop_all_motors(driver);
    
    // Marcar como inicializado
    driver->initialized = true;
    
    // Habilitar driver (sair do standby)
    tb6612fng_enable(driver, true);
    
    return true;
}

void tb6612fng_enable(tb6612fng_t* driver, bool enable) {
    if (driver == NULL || !driver->initialized) {
        return;
    }
    
    gpio_put(driver->standby_pin, enable);
}

void tb6612fng_set_direction(tb6612fng_t* driver, motor_id_t motor_id, motor_direction_t direction) {
    if (driver == NULL || !driver->initialized) {
        return;
    }
    
    switch (motor_id) {
        case MOTOR_A:
            apply_direction(&driver->motor_a, direction);
            break;
            
        case MOTOR_B:
            apply_direction(&driver->motor_b, direction);
            break;
    }
}

void tb6612fng_set_speed(tb6612fng_t* driver, motor_id_t motor_id, uint8_t speed) {
    if (driver == NULL || !driver->initialized) {
        return;
    }
    
    switch (motor_id) {
        case MOTOR_A:
            apply_speed(&driver->motor_a, speed);
            break;
            
        case MOTOR_B:
            apply_speed(&driver->motor_b, speed);
            break;
    }
}

void tb6612fng_control_motor(tb6612fng_t* driver, motor_id_t motor_id, 
                           motor_direction_t direction, uint8_t speed) {
    if (driver == NULL || !driver->initialized) {
        return;
    }
    
    // Configurar direção primeiro, depois velocidade
    tb6612fng_set_direction(driver, motor_id, direction);
    
    // Se direção é STOP ou BRAKE, definir velocidade como 0
    if (direction == MOTOR_STOP || direction == MOTOR_BRAKE) {
        tb6612fng_set_speed(driver, motor_id, 0);
    } else {
        tb6612fng_set_speed(driver, motor_id, speed);
    }
}

void tb6612fng_stop_all_motors(tb6612fng_t* driver) {
    if (driver == NULL || !driver->initialized) {
        return;
    }
    
    // Parar ambos os motores com freio elétrico
    tb6612fng_control_motor(driver, MOTOR_A, MOTOR_STOP, 0);
    tb6612fng_control_motor(driver, MOTOR_B, MOTOR_STOP, 0);
}

void tb6612fng_control_both_motors(tb6612fng_t* driver, 
                                 motor_direction_t dir_a, uint8_t speed_a,
                                 motor_direction_t dir_b, uint8_t speed_b) {
    if (driver == NULL || !driver->initialized) {
        return;
    }
    
    // Controlar ambos os motores simultaneamente
    tb6612fng_control_motor(driver, MOTOR_A, dir_a, speed_a);
    tb6612fng_control_motor(driver, MOTOR_B, dir_b, speed_b);
}

bool tb6612fng_is_ready(tb6612fng_t* driver) {
    if (driver == NULL) {
        return false;
    }
    
    return driver->initialized && gpio_get(driver->standby_pin);
} 