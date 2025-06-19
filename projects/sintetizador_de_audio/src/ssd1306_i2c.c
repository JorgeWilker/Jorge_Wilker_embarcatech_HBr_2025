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

#include "ssd1306_i2c.h"
#include <string.h>
#include <stdlib.h>

// Definições específicas do projeto para compatibilidade
#ifndef SSD1306_WIDTH
#define SSD1306_WIDTH 128
#endif

#ifndef SSD1306_HEIGHT  
#define SSD1306_HEIGHT 64
#endif

#ifndef SSD1306_PAGES
#define SSD1306_PAGES (SSD1306_HEIGHT / 8)
#endif

// Definições adicionais de comandos para compatibilidade com código fornecido
#ifndef SSD1306_DISPLAY_ALL_ON_RESUME
#define SSD1306_DISPLAY_ALL_ON_RESUME 0xA4
#endif

#ifndef SSD1306_NORMAL_DISPLAY
#define SSD1306_NORMAL_DISPLAY 0xA6
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

// Buffer global do display OLED
static uint8_t display_buffer[SSD1306_WIDTH * SSD1306_PAGES];
static bool display_initialized = false;

// Sequência de inicialização otimizada do controlador SSD1306
static const uint8_t init_sequence[] = {
    SSD1306_DISPLAY_OFF,
    SSD1306_SET_DISPLAY_CLOCK_DIV, 0x80,
    SSD1306_SET_MULTIPLEX, 0x3F,
    SSD1306_SET_DISPLAY_OFFSET, 0x00,
    SSD1306_SET_START_LINE | 0x00,
    SSD1306_CHARGE_PUMP, 0x14,
    SSD1306_MEMORY_MODE, 0x00,
    SSD1306_SEG_REMAP | 0x01,
    SSD1306_COM_SCAN_DEC,
    SSD1306_SET_COM_PINS, 0x12,
    SSD1306_SET_CONTRAST, 0xCF,
    SSD1306_SET_PRECHARGE, 0xF1,
    SSD1306_SET_VCOM_DETECT, 0x40,
    SSD1306_DISPLAY_ALL_ON_RESUME,
    SSD1306_NORMAL_DISPLAY,
    SSD1306_DISPLAY_ON
};

// Fonte bitmap 5x7 pixels para renderização de texto
static const uint8_t font5x7[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, // Espaço
    0x00, 0x00, 0x5F, 0x00, 0x00, // !
    0x00, 0x07, 0x00, 0x07, 0x00, // "
    0x14, 0x7F, 0x14, 0x7F, 0x14, // #
    0x24, 0x2A, 0x7F, 0x2A, 0x12, // $
    0x23, 0x13, 0x08, 0x64, 0x62, // %
    0x36, 0x49, 0x55, 0x22, 0x50, // &
    0x00, 0x05, 0x03, 0x00, 0x00, // '
    0x00, 0x1C, 0x22, 0x41, 0x00, // (
    0x00, 0x41, 0x22, 0x1C, 0x00, // )
    0x14, 0x08, 0x3E, 0x08, 0x14, // *
    0x08, 0x08, 0x3E, 0x08, 0x08, // +
    0x00, 0x50, 0x30, 0x00, 0x00, // ,
    0x08, 0x08, 0x08, 0x08, 0x08, // -
    0x00, 0x60, 0x60, 0x00, 0x00, // .
    0x20, 0x10, 0x08, 0x04, 0x02, // /
    0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
    0x00, 0x42, 0x7F, 0x40, 0x00, // 1
    0x42, 0x61, 0x51, 0x49, 0x46, // 2
    0x21, 0x41, 0x45, 0x4B, 0x31, // 3
    0x18, 0x14, 0x12, 0x7F, 0x10, // 4
    0x27, 0x45, 0x45, 0x45, 0x39, // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
    0x01, 0x71, 0x09, 0x05, 0x03, // 7
    0x36, 0x49, 0x49, 0x49, 0x36, // 8
    0x06, 0x49, 0x49, 0x29, 0x1E, // 9
    0x00, 0x36, 0x36, 0x00, 0x00, // :
    0x00, 0x56, 0x36, 0x00, 0x00, // ;
    0x08, 0x14, 0x22, 0x41, 0x00, // <
    0x14, 0x14, 0x14, 0x14, 0x14, // =
    0x00, 0x41, 0x22, 0x14, 0x08, // >
    0x02, 0x01, 0x51, 0x09, 0x06, // ?
    0x32, 0x49, 0x79, 0x41, 0x3E, // @
    0x7E, 0x11, 0x11, 0x11, 0x7E, // A
    0x7F, 0x49, 0x49, 0x49, 0x36, // B
    0x3E, 0x41, 0x41, 0x41, 0x22, // C
    0x7F, 0x41, 0x41, 0x22, 0x1C, // D
    0x7F, 0x49, 0x49, 0x49, 0x41, // E
    0x7F, 0x09, 0x09, 0x09, 0x01, // F
    0x3E, 0x41, 0x49, 0x49, 0x7A, // G
    0x7F, 0x08, 0x08, 0x08, 0x7F, // H
    0x00, 0x41, 0x7F, 0x41, 0x00, // I
    0x20, 0x40, 0x41, 0x3F, 0x01, // J
    0x7F, 0x08, 0x14, 0x22, 0x41, // K
    0x7F, 0x40, 0x40, 0x40, 0x40, // L
    0x7F, 0x02, 0x0C, 0x02, 0x7F, // M
    0x7F, 0x04, 0x08, 0x10, 0x7F, // N
    0x3E, 0x41, 0x41, 0x41, 0x3E, // O
    0x7F, 0x09, 0x09, 0x09, 0x06, // P
    0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
    0x7F, 0x09, 0x19, 0x29, 0x46, // R
    0x46, 0x49, 0x49, 0x49, 0x31, // S
    0x01, 0x01, 0x7F, 0x01, 0x01, // T
    0x3F, 0x40, 0x40, 0x40, 0x3F, // U
    0x1F, 0x20, 0x40, 0x20, 0x1F, // V
    0x3F, 0x40, 0x38, 0x40, 0x3F, // W
    0x63, 0x14, 0x08, 0x14, 0x63, // X
    0x07, 0x08, 0x70, 0x08, 0x07, // Y
    0x61, 0x51, 0x49, 0x45, 0x43, // Z
    0x00, 0x7F, 0x41, 0x41, 0x00, // [
    0x02, 0x04, 0x08, 0x10, 0x20, // \
    0x00, 0x41, 0x41, 0x7F, 0x00, // ]
    0x04, 0x02, 0x01, 0x02, 0x04, // ^
    0x40, 0x40, 0x40, 0x40, 0x40, // _
    0x00, 0x01, 0x02, 0x04, 0x00, // `
    0x20, 0x54, 0x54, 0x54, 0x78, // a
    0x7F, 0x48, 0x44, 0x44, 0x38, // b
    0x38, 0x44, 0x44, 0x44, 0x20, // c
    0x38, 0x44, 0x44, 0x48, 0x7F, // d
    0x38, 0x54, 0x54, 0x54, 0x18, // e
    0x08, 0x7E, 0x09, 0x01, 0x02, // f
    0x0C, 0x52, 0x52, 0x52, 0x3E, // g
    0x7F, 0x08, 0x04, 0x04, 0x78, // h
    0x00, 0x44, 0x7D, 0x40, 0x00, // i
    0x20, 0x40, 0x44, 0x3D, 0x00, // j
    0x7F, 0x10, 0x28, 0x44, 0x00, // k
    0x00, 0x41, 0x7F, 0x40, 0x00, // l
    0x7C, 0x04, 0x18, 0x04, 0x78, // m
    0x7C, 0x08, 0x04, 0x04, 0x78, // n
    0x38, 0x44, 0x44, 0x44, 0x38, // o
    0x7C, 0x14, 0x14, 0x14, 0x08, // p
    0x08, 0x14, 0x14, 0x18, 0x7C, // q
    0x7C, 0x08, 0x04, 0x04, 0x08, // r
    0x48, 0x54, 0x54, 0x54, 0x20, // s
    0x04, 0x3F, 0x44, 0x40, 0x20, // t
    0x3C, 0x40, 0x40, 0x20, 0x7C, // u
    0x1C, 0x20, 0x40, 0x20, 0x1C, // v
    0x3C, 0x40, 0x30, 0x40, 0x3C, // w
    0x44, 0x28, 0x10, 0x28, 0x44, // x
    0x0C, 0x50, 0x50, 0x50, 0x3C, // y
    0x44, 0x64, 0x54, 0x4C, 0x44, // z
    0x00, 0x08, 0x36, 0x41, 0x00, // {
    0x00, 0x00, 0x7F, 0x00, 0x00, // |
    0x00, 0x41, 0x36, 0x08, 0x00, // }
    0x10, 0x08, 0x08, 0x10, 0x08, // ~
    0x00, 0x00, 0x00, 0x00, 0x00  // DEL
};

// Parâmetros da fonte bitmap
#define FONT_WIDTH 5
#define FONT_HEIGHT 7
#define FONT_SPACING 1

// Renderização de caractere individual no framebuffer
void ssd1306_draw_char(uint8_t x, uint8_t y, char c, bool on) {
    if (c < ' ' || c > '~') {
        c = '?'; // Caractere de substituição para valores inválidos
    }

    // Índice na tabela de fonte bitmap
    uint16_t index = (c - ' ') * FONT_WIDTH;

    // Renderização coluna por coluna do caractere
    for (uint8_t col = 0; col < FONT_WIDTH; col++) {
        uint8_t line = font5x7[index + col];
        for (uint8_t row = 0; row < FONT_HEIGHT; row++) {
            if (line & (1 << row)) {
                ssd1306_set_pixel(x + col, y + row, on);
            }
        }
    }
}

// Renderização de string com controle de cursor horizontal
void ssd1306_draw_string(uint8_t x, uint8_t y, const char* str, bool on) {
    uint8_t cursor_x = x;
    while (*str) {
        ssd1306_draw_char(cursor_x, y, *str++, on);
        cursor_x += FONT_WIDTH + FONT_SPACING;
        if (cursor_x >= SSD1306_WIDTH - FONT_WIDTH) break;
    }
}

// Cálculo da largura de string para centralização
uint8_t ssd1306_get_string_width(const char* str) {
    size_t len = strlen(str);
    return len * (FONT_WIDTH + FONT_SPACING) - FONT_SPACING;
}

// Inicialização do controlador SSD1306 via I2C
bool ssd1306_init(void) {
    if (display_initialized) {
        return true;
    }

    // Configuração do barramento I2C
    i2c_init(I2C_PORT, I2C_BAUDRATE);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // Envio da sequência de inicialização
    for (size_t i = 0; i < sizeof(init_sequence); i++) {
        if (!ssd1306_send_command(init_sequence[i])) {
            return false;
        }
    }

    // Limpar framebuffer e enviar para display
    memset(display_buffer, 0, sizeof(display_buffer));
    ssd1306_display();

    display_initialized = true;
    return true;
}

// Limpeza do framebuffer interno
void ssd1306_clear(void) {
    memset(display_buffer, 0, sizeof(display_buffer));
}

// Transferência do framebuffer para o display físico
void ssd1306_display(void) {
    ssd1306_send_command(SSD1306_COLUMN_ADDR);
    ssd1306_send_command(0);
    ssd1306_send_command(SSD1306_WIDTH - 1);
    ssd1306_send_command(SSD1306_PAGE_ADDR);
    ssd1306_send_command(0);
    ssd1306_send_command(SSD1306_PAGES - 1);

    ssd1306_send_data(display_buffer, sizeof(display_buffer));
}

// Definição de pixel individual no framebuffer
void ssd1306_set_pixel(uint8_t x, uint8_t y, bool on) {
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return;
    }

    uint8_t page = y / 8;
    uint8_t bit = y % 8;
    uint16_t index = x + (page * SSD1306_WIDTH);

    if (on) {
        display_buffer[index] |= (1 << bit);
    } else {
        display_buffer[index] &= ~(1 << bit);
    }
}

// Leitura de estado de pixel do framebuffer
bool ssd1306_get_pixel(uint8_t x, uint8_t y) {
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return false;
    }

    uint8_t page = y / 8;
    uint8_t bit = y % 8;
    uint16_t index = x + (page * SSD1306_WIDTH);

    return (display_buffer[index] & (1 << bit)) != 0;
}

// Renderização de string centralizada horizontalmente
void ssd1306_draw_string_centered(uint8_t y, const char* str, bool on) {
    uint8_t width = ssd1306_get_string_width(str);
    uint8_t x = (SSD1306_WIDTH - width) / 2;
    ssd1306_draw_string(x, y, str, on);
}

// Exibição de estado do sistema de áudio
void ssd1306_show_audio_state(uint8_t state) {
    // Implementação básica de indicação de estado
    char* status = "IDLE";
    switch (state) {
        case 1: status = "GRAVANDO"; break;
        case 2: status = "REPRODUZINDO"; break;
    }
    ssd1306_clear();
    ssd1306_draw_string_centered(32, status, true);
    ssd1306_display();
}

// Transmissão de comando para controlador SSD1306
bool ssd1306_send_command(uint8_t command) {
    uint8_t buf[2] = {0x00, command}; // 0x00 indica modo comando
    int result = i2c_write_blocking(I2C_PORT, SSD1306_I2C_ADDR, buf, 2, false);
    return result == 2; // Validação de transmissão completa
}

// Transmissão de dados para controlador SSD1306
void ssd1306_send_data(uint8_t* data, size_t length) {
    uint8_t* buf = malloc(length + 1);
    if (buf == NULL) return;

    buf[0] = 0x40; // 0x40 indica modo dados
    memcpy(buf + 1, data, length);
    i2c_write_blocking(I2C_PORT, SSD1306_I2C_ADDR, buf, length + 1, false);
    
    free(buf);
}

// Verificação de status de inicialização do display
bool ssd1306_is_ready(void) {
    return display_initialized;
}

// Renderização de linha utilizando algoritmo de Bresenham
void ssd1306_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool on) {
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = (dx > dy ? dx : -dy) / 2;
    int16_t e2;

    while (true) {
        ssd1306_set_pixel(x0, y0, on);
        if (x0 == x1 && y0 == y1) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

// Renderização de retângulo com apenas contorno
void ssd1306_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool on) {
    ssd1306_draw_line(x, y, x + width - 1, y, on);
    ssd1306_draw_line(x + width - 1, y, x + width - 1, y + height - 1, on);
    ssd1306_draw_line(x + width - 1, y + height - 1, x, y + height - 1, on);
    ssd1306_draw_line(x, y + height - 1, x, y, on);
}

// Renderização de retângulo preenchido
void ssd1306_fill_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool on) {
    for (uint8_t i = 0; i < height; i++) {
        for (uint8_t j = 0; j < width; j++) {
            ssd1306_set_pixel(x + j, y + i, on);
        }
    }
}

// Renderização de círculo utilizando algoritmo de Bresenham
void ssd1306_draw_circle(uint8_t x0, uint8_t y0, uint8_t radius, bool on) {
    int16_t f = 1 - radius;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * radius;
    int16_t x = 0;
    int16_t y = radius;

    ssd1306_set_pixel(x0, y0 + radius, on);
    ssd1306_set_pixel(x0, y0 - radius, on);
    ssd1306_set_pixel(x0 + radius, y0, on);
    ssd1306_set_pixel(x0 - radius, y0, on);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        ssd1306_set_pixel(x0 + x, y0 + y, on);
        ssd1306_set_pixel(x0 - x, y0 + y, on);
        ssd1306_set_pixel(x0 + x, y0 - y, on);
        ssd1306_set_pixel(x0 - x, y0 - y, on);
        ssd1306_set_pixel(x0 + y, y0 + x, on);
        ssd1306_set_pixel(x0 - y, y0 + x, on);
        ssd1306_set_pixel(x0 + y, y0 - x, on);
        ssd1306_set_pixel(x0 - y, y0 - x, on);
    }
}

// Exibição do menu principal da aplicação
void ssd1306_show_main_menu(void) {
    ssd1306_clear();
    ssd1306_draw_string_centered(0, "BitDogLab Audio", true);
    ssd1306_draw_line(0, 10, SSD1306_WIDTH - 1, 10, true);
    ssd1306_draw_string(0, 20, "A: Gravar", true);
    ssd1306_draw_string(0, 30, "B: Reproduzir", true);
    ssd1306_draw_string(0, 40, "A+B: Limpar", true);
    ssd1306_display();
}

// Buffer circular para armazenamento de amostras da forma de onda
static uint8_t waveform_buffer[SSD1306_WIDTH];
static uint8_t waveform_position = 0;

// Visualização em tempo real da forma de onda no display OLED
void ssd1306_draw_waveform(uint16_t adc_sample) {
    // Mapeamento de valor ADC (0-4095, 12 bits) para altura do display (0-63 pixels)
    // Considerando que o centro da forma de onda está na metade do display
    uint8_t center_y = SSD1306_HEIGHT / 2;  // Linha central = 32 pixels
    uint8_t max_amplitude = center_y - 1;   // Amplitude máxima = 31 pixels
    
    // Normalização de amostra ADC para amplitude visual
    // ADC de 12 bits: 0-4095, centro teórico em 2048
    int16_t sample_offset = (int16_t)adc_sample - 2048;  // Centralização em 0
    
    // Escalonamento para amplitude do display com fator de zoom dobrado
    // Dobrando o fator de escala dividindo por 1024 ao invés de 2048
    int8_t display_amplitude = (sample_offset * max_amplitude) / 1024;
    
    // Limitação de amplitude para evitar overflow
    if (display_amplitude > max_amplitude) display_amplitude = max_amplitude;
    if (display_amplitude < -max_amplitude) display_amplitude = -max_amplitude;
    
    // Cálculo da posição Y final no display
    uint8_t sample_y = center_y + display_amplitude;
    
    // Armazenamento de amostra no buffer circular
    waveform_buffer[waveform_position] = sample_y;
    
    // Limpeza da coluna atual para efeito de scrolling
    for (uint8_t y = 0; y < SSD1306_HEIGHT; y++) {
        ssd1306_set_pixel(waveform_position, y, false);
    }
    
    // Renderização da linha central de referência
    ssd1306_set_pixel(waveform_position, center_y, true);
    
    // Renderização da amostra atual como linha vertical
    uint8_t line_start = center_y;
    uint8_t line_end = sample_y;
    
    // Garantia de que start <= end para renderização correta
    if (line_start > line_end) {
        uint8_t temp = line_start;
        line_start = line_end;
        line_end = temp;
    }
    
    // Renderização da linha vertical representando amplitude
    for (uint8_t y = line_start; y <= line_end; y++) {
        ssd1306_set_pixel(waveform_position, y, true);
    }
    
    // Renderização da conexão com amostra anterior para continuidade
    if (waveform_position > 0) {
        uint8_t prev_y = waveform_buffer[waveform_position - 1];
        ssd1306_draw_line(waveform_position - 1, prev_y, waveform_position, sample_y, true);
    }
    
    // Avanço da posição no buffer com comportamento circular
    waveform_position++;
    if (waveform_position >= SSD1306_WIDTH) {
        waveform_position = 0;
    }
    
    // Renderização do cursor de posição atual
    for (uint8_t y = 0; y < SSD1306_HEIGHT; y += 4) {
        ssd1306_set_pixel(waveform_position, y, true);
    }
}

// Inicialização da visualização da forma de onda
void ssd1306_waveform_init(void) {
    // Limpeza do buffer da forma de onda
    memset(waveform_buffer, SSD1306_HEIGHT / 2, sizeof(waveform_buffer));
    waveform_position = 0;
    
    // Configuração do display para modo forma de onda
    ssd1306_clear();
    
    // Renderização da grade de referência
    // Linha central horizontal pontilhada
    for (uint8_t x = 0; x < SSD1306_WIDTH; x += 8) {
        ssd1306_set_pixel(x, SSD1306_HEIGHT / 2, true);
    }
    
    // Linhas verticais de tempo reduzidas para melhor visualização
    for (uint8_t x = 42; x < SSD1306_WIDTH; x += 84) {  // Apenas em x=42 e x=126
        for (uint8_t y = 1; y <= SSD1306_HEIGHT - 2; y += 6) {  // Espaçamento maior entre pontos
            ssd1306_set_pixel(x, y, true);
        }
    }
    
    // Título da interface de visualização
    ssd1306_draw_string(2, 2, "FORMA DE ONDA", true);
    
    ssd1306_display();
}

// Limpeza da visualização da forma de onda
void ssd1306_waveform_clear(void) {
    ssd1306_clear();
    ssd1306_waveform_init();
}