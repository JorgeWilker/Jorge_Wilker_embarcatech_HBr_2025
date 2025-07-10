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

// Sistema embarcado de controle de motores baseado em dados inerciais
// Implementa aquisição IMU, controle de motores TB6612FNG e display OLED
// Arquitetura multi-periférico para plataforma BitDogLab com Raspberry Pi Pico
// Autor: Jorge Wilker Mamede de Andrade - 2025

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "ssd1306_i2c.h"
#include "ssd1306.h"
#include "tb6612fng.h"

// Configuração de hardware para arquitetura multi-periférico BitDogLab
// MPU-6050: I2C0 (GPIO 0/1) para aquisição de dados inerciais
// OLED SSD1306: I2C1 (GPIO 14/15) para interface visual
// TB6612FNG: PWM/GPIO para controle de motores DC independentes

// Definições de interface I2C0 para sensor inercial MPU-6050
#define BITDOGLAB_I2C_SDA_PIN 0         // Linha de dados I2C0
#define BITDOGLAB_I2C_SCL_PIN 1         // Linha de clock I2C0
#define BITDOGLAB_I2C_PORT i2c0         // Controlador I2C0 do RP2040

// Definições de interface I2C1 para display OLED SSD1306
#define OLED_I2C_SDA_PIN 14             // Linha de dados I2C1
#define OLED_I2C_SCL_PIN 15             // Linha de clock I2C1
#define OLED_I2C_PORT i2c1              // Controlador I2C1 do RP2040

// Endereçamento I2C de dispositivos periféricos
#define MPU6050_ADDR 0x68               // Endereço padrão MPU-6050 (AD0=LOW)
#define OLED_ADDR 0x3C                  // Endereço padrão SSD1306 (SA0=LOW)

// Buffer de framebuffer para display OLED SSD1306
static uint8_t oled_buffer[ssd1306_buffer_length];
static struct render_area area_display = {
    .start_column = 0,
    .end_column = ssd1306_width - 1,    // Largura completa: 128 pixels (0-127)
    .start_page = 0,
    .end_page = ssd1306_n_pages - 1     // Altura completa: 8 páginas (64 pixels)
};

// Driver para controle de motores TB6612FNG
static tb6612fng_t motor_driver;

// Parâmetros de controle dos motores
#define ACCEL_THRESHOLD 5000    // Limiar de aceleração para movimento
#define GYRO_THRESHOLD 3000     // Limiar de giroscópio para rotação
#define MAX_MOTOR_SPEED 80      // Velocidade máxima dos motores (0-100%)
#define MIN_MOTOR_SPEED 30      // Velocidade mínima dos motores (0-100%)

// Inicialização e configuração do sensor MPU-6050 via I2C
static void mpu6050_reset() {
    // Reset completo do dispositivo através do registro 0x6B
    uint8_t buf[] = {0x6B, 0x80};
    i2c_write_blocking(BITDOGLAB_I2C_PORT, MPU6050_ADDR, buf, 2, false);
    sleep_ms(100); // Estabilização após reset de hardware

    // Saída do modo sleep para operação normal
    buf[1] = 0x00;  // Ativa o MPU-6050 removendo flag de sleep
    i2c_write_blocking(BITDOGLAB_I2C_PORT, MPU6050_ADDR, buf, 2, false); 
    sleep_ms(10); // Estabilização após ativação do sensor
}

// Aquisição de dados inerciais do sensor MPU-6050 via protocolo I2C
static void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3]) {
    // Leitura sequencial otimizada com auto-incremento de registros
    // Minimiza transações I2C para melhor performance
    
    uint8_t buffer[6];

    // Leitura dos registros de aceleração (0x3B-0x40) em bloco único
    uint8_t val = 0x3B;
    i2c_write_blocking(BITDOGLAB_I2C_PORT, MPU6050_ADDR, &val, 1, true); // Mantém barramento ativo
    i2c_read_blocking(BITDOGLAB_I2C_PORT, MPU6050_ADDR, buffer, 6, false);

    // Conversão de dados big-endian para int16_t dos eixos X, Y, Z
    for (int i = 0; i < 3; i++) {
        accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
    }

    // Leitura dos registros de giroscópio (0x43-0x48) em bloco único
    val = 0x43;
    i2c_write_blocking(BITDOGLAB_I2C_PORT, MPU6050_ADDR, &val, 1, true);
    i2c_read_blocking(BITDOGLAB_I2C_PORT, MPU6050_ADDR, buffer, 6, false);  // Libera barramento

    // Conversão de dados big-endian para int16_t dos eixos X, Y, Z
    for (int i = 0; i < 3; i++) {
        gyro[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
    }
}

// Renderização de dados inerciais e status dos motores no display OLED
static void display_sensor_data(int16_t accel[3], int16_t gyro[3]) {
    char line_buffer[16];
    
    // Limpa o buffer do display
    ssd1306_clear(oled_buffer);
    
    // Seção Acelerômetro (compacta)
    ssd1306_draw_string(oled_buffer, 0, 0, "ACEL:");
    snprintf(line_buffer, sizeof(line_buffer), "X:%5d", accel[0]);
    ssd1306_draw_string(oled_buffer, 0, 8, line_buffer);
    snprintf(line_buffer, sizeof(line_buffer), "Y:%5d", accel[1]);
    ssd1306_draw_string(oled_buffer, 64, 8, line_buffer);
    
    // Seção Giroscópio (compacta)
    ssd1306_draw_string(oled_buffer, 0, 16, "GIRO:");
    snprintf(line_buffer, sizeof(line_buffer), "Z:%5d", gyro[2]);
    ssd1306_draw_string(oled_buffer, 0, 24, line_buffer);
    
    // Status do driver de motores
    ssd1306_draw_string(oled_buffer, 0, 40, "MOTORS:");
    if (tb6612fng_is_ready(&motor_driver)) {
        ssd1306_draw_string(oled_buffer, 0, 48, "READY");
    } else {
        ssd1306_draw_string(oled_buffer, 0, 48, "OFF");
    }
    
    // Indicador de movimento detectado
    bool movement_detected = (abs(accel[0]) > ACCEL_THRESHOLD || 
                             abs(accel[1]) > ACCEL_THRESHOLD || 
                             abs(gyro[2]) > GYRO_THRESHOLD);
    if (movement_detected) {
        ssd1306_draw_string(oled_buffer, 64, 48, "ACTIVE");
    } else {
        ssd1306_draw_string(oled_buffer, 64, 48, "IDLE");
    }
    
    // Transfere framebuffer para controlador SSD1306 via I2C
    render_on_display(oled_buffer, &area_display);
}

// Controle inteligente de motores baseado em dados inerciais do IMU
static void control_motors_from_imu(int16_t accel[3], int16_t gyro[3]) {
    // Extração de componentes principais para controle direcional
    int16_t accel_x = accel[0];  // Componente lateral do movimento
    int16_t accel_y = accel[1];  // Componente frontal/traseiro do movimento  
    int16_t gyro_z = gyro[2];    // Velocidade angular em torno do eixo Z
    
    // Inicializa controles dos motores
    motor_direction_t dir_a = MOTOR_STOP;
    motor_direction_t dir_b = MOTOR_STOP;
    uint8_t speed_a = 0;
    uint8_t speed_b = 0;
    
    // Detecção de movimento através de comparação com limites configurados
    bool has_accel_movement = (abs(accel_x) > ACCEL_THRESHOLD || abs(accel_y) > ACCEL_THRESHOLD);
    bool has_gyro_movement = abs(gyro_z) > GYRO_THRESHOLD;
    
    if (has_accel_movement || has_gyro_movement) {
        // Cálculo de velocidade base proporcional à intensidade do movimento
        uint8_t base_speed = MIN_MOTOR_SPEED;
        
        if (has_accel_movement) {
            // Velocidade baseada na magnitude vetorial da aceleração
            int32_t accel_magnitude = sqrt(accel_x * accel_x + accel_y * accel_y);
            base_speed = MIN_MOTOR_SPEED + 
                        ((accel_magnitude - ACCEL_THRESHOLD) * (MAX_MOTOR_SPEED - MIN_MOTOR_SPEED)) / 
                        (32000 - ACCEL_THRESHOLD);
        }
        
        // Limita velocidade ao máximo permitido
        if (base_speed > MAX_MOTOR_SPEED) base_speed = MAX_MOTOR_SPEED;
        
        // Implementação da lógica de controle direcional
        if (abs(gyro_z) > GYRO_THRESHOLD) {
            // Rotação diferencial: motores em direções opostas
            if (gyro_z > 0) {
                // Rotação horária através de direções opostas
                dir_a = MOTOR_FORWARD;
                dir_b = MOTOR_BACKWARD;
            } else {
                // Rotação anti-horária através de direções opostas
                dir_a = MOTOR_BACKWARD;
                dir_b = MOTOR_FORWARD;
            }
            speed_a = base_speed;
            speed_b = base_speed;
        } 
        else if (abs(accel_y) > ACCEL_THRESHOLD) {
            // Movimento frontal/traseiro
            if (accel_y > 0) {
                // Movimento para frente
                dir_a = MOTOR_FORWARD;
                dir_b = MOTOR_FORWARD;
            } else {
                // Movimento para trás
                dir_a = MOTOR_BACKWARD;
                dir_b = MOTOR_BACKWARD;
            }
            speed_a = base_speed;
            speed_b = base_speed;
            
            // Ajuste de direção baseado no acelerômetro X
            if (abs(accel_x) > ACCEL_THRESHOLD / 2) {
                if (accel_x > 0) {
                    // Curva para direita (reduz velocidade do motor direito)
                    speed_b = speed_b * 0.6;
                } else {
                    // Curva para esquerda (reduz velocidade do motor esquerdo)
                    speed_a = speed_a * 0.6;
                }
            }
        }
    }
    
    // Aplica comandos aos motores
    tb6612fng_control_both_motors(&motor_driver, dir_a, speed_a, dir_b, speed_b);
    
    // Debug via serial
    if (speed_a > 0 || speed_b > 0) {
        printf("Motores: A[%s:%d%%] B[%s:%d%%]\n", 
               (dir_a == MOTOR_FORWARD) ? "FWD" : (dir_a == MOTOR_BACKWARD) ? "BWD" : "STP",
               speed_a,
               (dir_b == MOTOR_FORWARD) ? "FWD" : (dir_b == MOTOR_BACKWARD) ? "BWD" : "STP",
               speed_b);
    }
}

// Função principal do sistema embarcado
int main() {
    stdio_init_all();

    printf("Sistema MPU-6050 + OLED + Motores TB6612FNG BitDogLab\n");
    printf("MPU-6050 I2C: SDA=GPIO%d, SCL=GPIO%d\n", BITDOGLAB_I2C_SDA_PIN, BITDOGLAB_I2C_SCL_PIN);
    printf("OLED I2C: SDA=GPIO%d, SCL=GPIO%d\n", OLED_I2C_SDA_PIN, OLED_I2C_SCL_PIN);
    printf("Motores: A(PWM=GP%d,IN1=GP%d,IN2=GP%d) B(PWM=GP%d,IN1=GP%d,IN2=GP%d)\n", 
           MOTOR_A_PWM_PIN, MOTOR_A_IN1_PIN, MOTOR_A_IN2_PIN,
           MOTOR_B_PWM_PIN, MOTOR_B_IN1_PIN, MOTOR_B_IN2_PIN);

    // Configuração da interface I2C para MPU-6050 (i2c0)
    printf("\nConfigurando I2C0 (MPU-6050)...\n");
    i2c_init(BITDOGLAB_I2C_PORT, 400 * 1000);
    gpio_set_function(BITDOGLAB_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(BITDOGLAB_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(BITDOGLAB_I2C_SDA_PIN);
    gpio_pull_up(BITDOGLAB_I2C_SCL_PIN);
    printf("I2C0 configurado: SDA=GPIO%d, SCL=GPIO%d\n", BITDOGLAB_I2C_SDA_PIN, BITDOGLAB_I2C_SCL_PIN);
    
    // Configuração da interface I2C para OLED SSD1306 (i2c1)
    printf("\nConfigurando I2C1 (OLED)...\n");
    i2c_init(OLED_I2C_PORT, ssd1306_i2c_clock * 1000);
    gpio_set_function(OLED_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(OLED_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(OLED_I2C_SDA_PIN);
    gpio_pull_up(OLED_I2C_SCL_PIN);
    printf("I2C1 configurado: SDA=GPIO%d, SCL=GPIO%d\n", OLED_I2C_SDA_PIN, OLED_I2C_SCL_PIN);
    
    // Disponibiliza os pinos I2C para ferramenta picotool
    bi_decl(bi_2pins_with_func(BITDOGLAB_I2C_SDA_PIN, BITDOGLAB_I2C_SCL_PIN, GPIO_FUNC_I2C));
    bi_decl(bi_2pins_with_func(OLED_I2C_SDA_PIN, OLED_I2C_SCL_PIN, GPIO_FUNC_I2C));

    // Aguarda estabilização
    sleep_ms(1000);
    
    // Inicialização do sensor MPU-6050
    printf("Inicializando MPU-6050...\n");
    mpu6050_reset();
    
    // Inicialização do display OLED (conforme projeto de referência)
    printf("Inicializando OLED...\n");
    ssd1306_init();
    calculate_render_area_buffer_length(&area_display);
    ssd1306_clear(oled_buffer);
    
    // Inicialização do driver de motores TB6612FNG
    printf("Inicializando driver de motores TB6612FNG...\n");
    if (tb6612fng_init(&motor_driver)) {
        printf("Driver de motores inicializado com sucesso!\n");
    } else {
        printf("ERRO: Falha na inicialização do driver de motores!\n");
    }

    // Variáveis para armazenamento dos dados capturados
    int16_t acceleration[3], gyro[3];

    printf("Iniciando sistema de controle de motores baseado em IMU...\n");

    // Loop principal de aquisição, exibição e controle
    while (1) {
        mpu6050_read_raw(acceleration, gyro);

        // Exibição dos dados no terminal serial
        printf("\n=== LEITURA MPU-6050 ===\n");
        printf("Acelerômetro:\n");
        printf("  X = %7d  Y = %7d  Z = %7d\n", acceleration[0], acceleration[1], acceleration[2]);
        printf("Giroscópio:\n");
        printf("  X = %7d  Y = %7d  Z = %7d\n", gyro[0], gyro[1], gyro[2]);
        printf("========================\n");
        
        // Controle inteligente dos motores baseado em dados inerciais
        control_motors_from_imu(acceleration, gyro);
        
        // Exibição dos dados no display OLED
        display_sensor_data(acceleration, gyro);

        sleep_ms(500); // Taxa de amostragem de 2Hz para controle responsivo
    }
    
    return 0;
}