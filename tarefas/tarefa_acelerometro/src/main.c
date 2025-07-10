//
// Copyright (C) 2025 Jorge Wilker Mamede de Andrade
//
// Este programa é software livre: você pode redistribuí-lo e/ou modificá-lo
// sob os termos da GNU General Public License conforme publicada pela
// Free Software Foundation, tanto a versão 3 da Licença, ou
// (a seu critério) qualquer versão posterior.
//
// Este programa é distribuído na esperança de que seja útil,
// mas SEM QUALQUER GARANTIA; sem mesmo a garantia implícita de
// COMERCIALIZAÇÃO ou ADEQUAÇÃO A UM DETERMINADO FIM. Veja o
// GNU General Public License para mais detalhes.
//
// Você deve ter recebido uma cópia da GNU General Public License
// junto com este programa. Se não, veja <https://www.gnu.org/licenses/>.
//

// Sistema embarcado de aquisição de dados inerciais MPU-6050
// Implementa comunicação I2C dual para sensor e display OLED
// Arquitetura otimizada para plataforma BitDogLab com Raspberry Pi Pico
// Autor: Jorge Wilker Mamede de Andrade - 2025

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "ssd1306_i2c.h"
#include "ssd1306.h"

// Configuração de hardware para arquitetura dual I2C BitDogLab
// Implementa isolamento de barramento entre sensor inercial e display
// MPU-6050: GPIO 0/1 em I2C0 (400kHz) para aquisição de dados
// OLED SSD1306: GPIO 14/15 em I2C1 (400kHz) para interface visual

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

// Rotina de inicialização e configuração do sensor MPU-6050
static void mpu6050_reset() {
    // Sequência de reset do dispositivo via registro 0x6B
    uint8_t buf[] = {0x6B, 0x80};
    i2c_write_blocking(BITDOGLAB_I2C_PORT, MPU6050_ADDR, buf, 2, false);
    sleep_ms(100); // Tempo de estabilização após reset do dispositivo

    // Desabilita modo sleep através do registro 0x6B
    buf[1] = 0x00;  // Valor 0x00 remove o modo sleep do MPU-6050
    i2c_write_blocking(BITDOGLAB_I2C_PORT, MPU6050_ADDR, buf, 2, false); 
    sleep_ms(10); // Tempo de estabilização após saída do modo sleep
}

// Aquisição de dados inerciais do sensor MPU-6050 via protocolo I2C
static void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3]) {
    // Implementa leitura sequencial com auto-incremento de registros
    // Protocolo otimizado para minimizar transações I2C
    
    uint8_t buffer[6];

    // Leitura dos registros de aceleração (0x3B a 0x40) - 6 bytes
    uint8_t val = 0x3B;
    i2c_write_blocking(BITDOGLAB_I2C_PORT, MPU6050_ADDR, &val, 1, true); // Mantém controle do barramento
    i2c_read_blocking(BITDOGLAB_I2C_PORT, MPU6050_ADDR, buffer, 6, false);

    // Conversão dos dados de aceleração para formato int16_t
    for (int i = 0; i < 3; i++) {
        accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
    }

    // Leitura dos registros de giroscópio (0x43 a 0x48) - 6 bytes
    val = 0x43;
    i2c_write_blocking(BITDOGLAB_I2C_PORT, MPU6050_ADDR, &val, 1, true);
    i2c_read_blocking(BITDOGLAB_I2C_PORT, MPU6050_ADDR, buffer, 6, false);  // Libera controle do barramento

    // Conversão dos dados de giroscópio para formato int16_t
    for (int i = 0; i < 3; i++) {
        gyro[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
    }
}

// Renderização de dados inerciais no display OLED SSD1306
static void display_sensor_data(int16_t accel[3], int16_t gyro[3]) {
    char line_buffer[16];
    
    // Limpa o buffer do display
    ssd1306_clear(oled_buffer);
    
    // Seção Acelerômetro (títulos curtos)
    ssd1306_draw_string(oled_buffer, 0, 0, "ACEL:");
    
    // Linha 1: X e Y do acelerômetro
    snprintf(line_buffer, sizeof(line_buffer), "X:%5d", accel[0]);
    ssd1306_draw_string(oled_buffer, 0, 8, line_buffer);
    snprintf(line_buffer, sizeof(line_buffer), "Y:%5d", accel[1]);
    ssd1306_draw_string(oled_buffer, 64, 8, line_buffer);
    
    // Linha 2: Z do acelerômetro
    snprintf(line_buffer, sizeof(line_buffer), "Z:%5d", accel[2]);
    ssd1306_draw_string(oled_buffer, 0, 16, line_buffer);
    
    // Seção Giroscópio (títulos curtos)
    ssd1306_draw_string(oled_buffer, 0, 32, "GIRO:");
    
    // Linha 1: X e Y do giroscópio
    snprintf(line_buffer, sizeof(line_buffer), "X:%5d", gyro[0]);
    ssd1306_draw_string(oled_buffer, 0, 40, line_buffer);
    snprintf(line_buffer, sizeof(line_buffer), "Y:%5d", gyro[1]);
    ssd1306_draw_string(oled_buffer, 64, 40, line_buffer);
    
    // Linha 2: Z do giroscópio
    snprintf(line_buffer, sizeof(line_buffer), "Z:%5d", gyro[2]);
    ssd1306_draw_string(oled_buffer, 0, 48, line_buffer);
    
    // Transfere framebuffer para controlador SSD1306 via I2C
    render_on_display(oled_buffer, &area_display);
}

// Função principal do sistema embarcado
int main() {
    stdio_init_all();

    printf("Sistema MPU-6050 + OLED BitDogLab - Leitura de dados inerciais\n");
    printf("MPU-6050 I2C: SDA=GPIO%d, SCL=GPIO%d\n", BITDOGLAB_I2C_SDA_PIN, BITDOGLAB_I2C_SCL_PIN);
    printf("OLED I2C: SDA=GPIO%d, SCL=GPIO%d\n", OLED_I2C_SDA_PIN, OLED_I2C_SCL_PIN);

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

    // Variáveis para armazenamento dos dados capturados
    int16_t acceleration[3], gyro[3];

    printf("Iniciando leitura contínua do MPU-6050 com exibição no OLED...\n");

    // Loop principal de aquisição e exibição de dados
    while (1) {
        mpu6050_read_raw(acceleration, gyro);

        // Exibição dos dados no terminal serial
        printf("\n=== LEITURA MPU-6050 ===\n");
        printf("Acelerômetro:\n");
        printf("  X = %7d  Y = %7d  Z = %7d\n", acceleration[0], acceleration[1], acceleration[2]);
        printf("Giroscópio:\n");
        printf("  X = %7d  Y = %7d  Z = %7d\n", gyro[0], gyro[1], gyro[2]);
        printf("========================\n");
        
        // Exibição dos dados no display OLED
        display_sensor_data(acceleration, gyro);

        sleep_ms(1000); // Taxa de amostragem de 1Hz para visualização confortável
    }
    
    return 0;
}