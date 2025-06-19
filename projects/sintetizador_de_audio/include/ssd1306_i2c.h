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

// Driver para display OLED SSD1306 via I2C
// Fornece funções para controle do display e renderização gráfica
// Inclui implementação de visualização da forma de onda em tempo real

#ifndef SSD1306_I2C_H
#define SSD1306_I2C_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Comandos do controlador SSD1306
#define SSD1306_SET_CONTRAST 0x81          // Define o contraste do display
#define SSD1306_DISPLAY_RAM 0xA4           // Exibe o conteúdo da RAM
#define SSD1306_DISPLAY_NORMAL 0xA6        // Modo de exibição normal (não invertido)
#define SSD1306_DISPLAY_OFF 0xAE           // Desliga o display
#define SSD1306_DISPLAY_ON 0xAF            // Liga o display
#define SSD1306_SET_DISPLAY_OFFSET 0xD3    // Define o offset vertical do display
#define SSD1306_SET_COM_PINS 0xDA          // Configura os pinos COM
#define SSD1306_SET_VCOM_DETECT 0xDB       // Configura detecção VCOM
#define SSD1306_SET_DISPLAY_CLOCK_DIV 0xD5 // Define o divisor do clock
#define SSD1306_SET_PRECHARGE 0xD9         // Define período de pré-carga
#define SSD1306_SET_MULTIPLEX 0xA8         // Configura ratio de multiplexação
#define SSD1306_SET_LOW_COLUMN 0x00        // Define coluna baixa
#define SSD1306_SET_HIGH_COLUMN 0x10       // Define coluna alta
#define SSD1306_SET_START_LINE 0x40        // Define linha inicial de exibição
#define SSD1306_MEMORY_MODE 0x20           // Define modo de endereçamento de memória
#define SSD1306_COLUMN_ADDR 0x21           // Define endereço de coluna
#define SSD1306_PAGE_ADDR 0x22             // Define endereço de página
#define SSD1306_COM_SCAN_INC 0xC0          // Varredura COM incremental
#define SSD1306_COM_SCAN_DEC 0xC8          // Varredura COM decremental
#define SSD1306_SEG_REMAP 0xA0             // Remapeamento de segmento
#define SSD1306_CHARGE_PUMP 0x8D           // Configuração da bomba de carga

// === FUNÇÕES BÁSICAS DO DISPLAY ===

// Inicialização do display OLED
// Configura o hardware e inicializa o controlador SSD1306 com os parâmetros padrão
bool ssd1306_init(void);

// Limpeza do buffer do display
// Preenche o buffer com zeros (pixels apagados)
void ssd1306_clear(void);

// Atualização do display físico com o conteúdo do buffer
// Envia o conteúdo do buffer interno para o controlador SSD1306 via I2C
void ssd1306_display(void);

// Verificação de status de inicialização do display
bool ssd1306_is_ready(void);

// === FUNÇÕES DE RENDERIZAÇÃO ===

// Renderização de pixel individual no framebuffer
void ssd1306_set_pixel(uint8_t x, uint8_t y, bool on);

// Leitura de estado de pixel do framebuffer
bool ssd1306_get_pixel(uint8_t x, uint8_t y);

// Renderização de linha utilizando algoritmo de Bresenham
void ssd1306_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool on);

// Renderização de retângulo com apenas contorno
void ssd1306_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool on);

// Renderização de retângulo preenchido
void ssd1306_fill_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool on);

// Renderização de círculo utilizando algoritmo de Bresenham
void ssd1306_draw_circle(uint8_t x0, uint8_t y0, uint8_t radius, bool on);

// === FUNÇÕES DE TEXTO ===

// Renderização de caractere individual no framebuffer
void ssd1306_draw_char(uint8_t x, uint8_t y, char c, bool on);

// Renderização de string com controle de cursor horizontal
void ssd1306_draw_string(uint8_t x, uint8_t y, const char* str, bool on);

// Cálculo da largura de string para centralização
uint8_t ssd1306_get_string_width(const char* str);

// Renderização de string centralizada horizontalmente
void ssd1306_draw_string_centered(uint8_t y, const char* str, bool on);

// === FUNÇÕES ESPECIAIS DA APLICAÇÃO ===

// Exibição do menu principal da aplicação
void ssd1306_show_main_menu(void);

// Exibição de estado do sistema de áudio
void ssd1306_show_audio_state(uint8_t state);

// === FUNCIONALIDADES DE VISUALIZAÇÃO DA FORMA DE ONDA ===

// Visualização em tempo real da forma de onda no display OLED
// Mapeia valores ADC de 12 bits para coordenadas de display com amplitude dobrada
void ssd1306_draw_waveform(uint16_t adc_sample);

// Inicialização da visualização da forma de onda
// Configura grade de referência e prepara buffer circular
void ssd1306_waveform_init(void);

// Limpeza da visualização da forma de onda
// Reinicia display e recarrega grade de referência
void ssd1306_waveform_clear(void);

// === FUNÇÕES DE COMUNICAÇÃO I2C (INTERNAS) ===

// Transmissão de comando para controlador SSD1306
bool ssd1306_send_command(uint8_t command);

// Transmissão de dados para controlador SSD1306
void ssd1306_send_data(uint8_t* data, size_t length);

// === DEFINIÇÕES DE COMPATIBILIDADE ===

// Definições específicas para compatibilidade entre implementações
#ifndef SSD1306_WIDTH
#define SSD1306_WIDTH 128
#endif

#ifndef SSD1306_HEIGHT  
#define SSD1306_HEIGHT 64
#endif

#ifndef SSD1306_PAGES
#define SSD1306_PAGES (SSD1306_HEIGHT / 8)
#endif

// Configurações de I2C e hardware para compatibilidade
#ifndef I2C_PORT
#define I2C_PORT i2c1
#endif

#ifndef I2C_BAUDRATE
#define I2C_BAUDRATE (400 * 1000)  // 400 kHz
#endif

#ifndef I2C_SDA_PIN
#define I2C_SDA_PIN 14
#endif

#ifndef I2C_SCL_PIN
#define I2C_SCL_PIN 15
#endif

#ifndef SSD1306_I2C_ADDR
#define SSD1306_I2C_ADDR 0x3C
#endif

#endif // SSD1306_I2C_H