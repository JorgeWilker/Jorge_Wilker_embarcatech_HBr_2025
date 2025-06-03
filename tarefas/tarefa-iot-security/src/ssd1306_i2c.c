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

/**
 * @file ssd1306_i2c.c
 * @brief Implementação do driver para display OLED SSD1306 via I2C
 * 
 * @author Jorge Wilker
 * @date Maio 2025
 */

#include "../include/ssd1306_i2c.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Comandos do SSD1306
 */
#define SSD1306_SET_CONTRAST        0x81
#define SSD1306_SET_ENTIRE_ON       0xA4
#define SSD1306_SET_NORM_INV        0xA6
#define SSD1306_SET_DISP            0xAE
#define SSD1306_SET_MEM_ADDR        0x20
#define SSD1306_SET_COL_ADDR        0x21
#define SSD1306_SET_PAGE_ADDR       0x22
#define SSD1306_SET_DISP_START_LINE 0x40
#define SSD1306_SET_SEG_REMAP       0xA0
#define SSD1306_SET_MUX_RATIO       0xA8
#define SSD1306_SET_COM_OUT_DIR     0xC0
#define SSD1306_SET_DISP_OFFSET     0xD3
#define SSD1306_SET_COM_PIN_CFG     0xDA
#define SSD1306_SET_DISP_CLK_DIV    0xD5
#define SSD1306_SET_PRECHARGE       0xD9
#define SSD1306_SET_VCOM_DESEL      0xDB
#define SSD1306_SET_CHARGE_PUMP     0x8D

/**
 * @brief Fonte bitmap 6x8 pixels (ASCII 32-127)
 */
static const uint8_t font_6x8[][6] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 32 (space)
    {0x00, 0x00, 0x5F, 0x00, 0x00, 0x00}, // 33 !
    {0x00, 0x07, 0x00, 0x07, 0x00, 0x00}, // 34 "
    {0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00}, // 35 #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00}, // 36 $
    {0x23, 0x13, 0x08, 0x64, 0x62, 0x00}, // 37 %
    {0x36, 0x49, 0x56, 0x20, 0x50, 0x00}, // 38 &
    {0x00, 0x08, 0x07, 0x03, 0x00, 0x00}, // 39 '
    {0x00, 0x1C, 0x22, 0x41, 0x00, 0x00}, // 40 (
    {0x00, 0x41, 0x22, 0x1C, 0x00, 0x00}, // 41 )
    {0x2A, 0x1C, 0x7F, 0x1C, 0x2A, 0x00}, // 42 *
    {0x08, 0x08, 0x3E, 0x08, 0x08, 0x00}, // 43 +
    {0x00, 0x80, 0x70, 0x30, 0x00, 0x00}, // 44 ,
    {0x08, 0x08, 0x08, 0x08, 0x08, 0x00}, // 45 -
    {0x00, 0x00, 0x60, 0x60, 0x00, 0x00}, // 46 .
    {0x20, 0x10, 0x08, 0x04, 0x02, 0x00}, // 47 /
    {0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00}, // 48 0
    {0x00, 0x42, 0x7F, 0x40, 0x00, 0x00}, // 49 1
    {0x72, 0x49, 0x49, 0x49, 0x46, 0x00}, // 50 2
    {0x21, 0x41, 0x49, 0x4D, 0x33, 0x00}, // 51 3
    {0x18, 0x14, 0x12, 0x7F, 0x10, 0x00}, // 52 4
    {0x27, 0x45, 0x45, 0x45, 0x39, 0x00}, // 53 5
    {0x3C, 0x4A, 0x49, 0x49, 0x31, 0x00}, // 54 6
    {0x41, 0x21, 0x11, 0x09, 0x07, 0x00}, // 55 7
    {0x36, 0x49, 0x49, 0x49, 0x36, 0x00}, // 56 8
    {0x46, 0x49, 0x49, 0x29, 0x1E, 0x00}, // 57 9
    {0x00, 0x00, 0x14, 0x00, 0x00, 0x00}, // 58 :
    {0x00, 0x40, 0x34, 0x00, 0x00, 0x00}, // 59 ;
    {0x00, 0x08, 0x14, 0x22, 0x41, 0x00}, // 60 <
    {0x14, 0x14, 0x14, 0x14, 0x14, 0x00}, // 61 =
    {0x00, 0x41, 0x22, 0x14, 0x08, 0x00}, // 62 >
    {0x02, 0x01, 0x59, 0x09, 0x06, 0x00}, // 63 ?
    {0x3E, 0x41, 0x5D, 0x59, 0x4E, 0x00}, // 64 @
    {0x7C, 0x12, 0x11, 0x12, 0x7C, 0x00}, // 65 A
    {0x7F, 0x49, 0x49, 0x49, 0x36, 0x00}, // 66 B
    {0x3E, 0x41, 0x41, 0x41, 0x22, 0x00}, // 67 C
    {0x7F, 0x41, 0x41, 0x41, 0x3E, 0x00}, // 68 D
    {0x7F, 0x49, 0x49, 0x49, 0x41, 0x00}, // 69 E
    {0x7F, 0x09, 0x09, 0x09, 0x01, 0x00}, // 70 F
    {0x3E, 0x41, 0x41, 0x51, 0x73, 0x00}, // 71 G
    {0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00}, // 72 H
    {0x00, 0x41, 0x7F, 0x41, 0x00, 0x00}, // 73 I
    {0x20, 0x40, 0x41, 0x3F, 0x01, 0x00}, // 74 J
    {0x7F, 0x08, 0x14, 0x22, 0x41, 0x00}, // 75 K
    {0x7F, 0x40, 0x40, 0x40, 0x40, 0x00}, // 76 L
    {0x7F, 0x02, 0x1C, 0x02, 0x7F, 0x00}, // 77 M
    {0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00}, // 78 N
    {0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00}, // 79 O
    {0x7F, 0x09, 0x09, 0x09, 0x06, 0x00}, // 80 P
    {0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00}, // 81 Q
    {0x7F, 0x09, 0x19, 0x29, 0x46, 0x00}, // 82 R
    {0x26, 0x49, 0x49, 0x49, 0x32, 0x00}, // 83 S
    {0x03, 0x01, 0x7F, 0x01, 0x03, 0x00}, // 84 T
    {0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00}, // 85 U
    {0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00}, // 86 V
    {0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00}, // 87 W
    {0x63, 0x14, 0x08, 0x14, 0x63, 0x00}, // 88 X
    {0x03, 0x04, 0x78, 0x04, 0x03, 0x00}, // 89 Y
    {0x61, 0x59, 0x49, 0x4D, 0x43, 0x00}, // 90 Z
    {0x00, 0x7F, 0x41, 0x41, 0x41, 0x00}, // 91 [
    {0x02, 0x04, 0x08, 0x10, 0x20, 0x00}, // 92 \
    {0x00, 0x41, 0x41, 0x41, 0x7F, 0x00}, // 93 ]
    {0x04, 0x02, 0x01, 0x02, 0x04, 0x00}, // 94 ^
    {0x40, 0x40, 0x40, 0x40, 0x40, 0x00}, // 95 _
    {0x00, 0x03, 0x07, 0x08, 0x00, 0x00}, // 96 `
    {0x20, 0x54, 0x54, 0x78, 0x40, 0x00}, // 97 a
    {0x7F, 0x28, 0x44, 0x44, 0x38, 0x00}, // 98 b
    {0x38, 0x44, 0x44, 0x44, 0x28, 0x00}, // 99 c
    {0x38, 0x44, 0x44, 0x28, 0x7F, 0x00}, // 100 d
    {0x38, 0x54, 0x54, 0x54, 0x18, 0x00}, // 101 e
    {0x00, 0x08, 0x7E, 0x09, 0x02, 0x00}, // 102 f
    {0x18, 0xA4, 0xA4, 0x9C, 0x78, 0x00}, // 103 g
    {0x7F, 0x08, 0x04, 0x04, 0x78, 0x00}, // 104 h
    {0x00, 0x44, 0x7D, 0x40, 0x00, 0x00}, // 105 i
    {0x20, 0x40, 0x40, 0x3D, 0x00, 0x00}, // 106 j
    {0x7F, 0x10, 0x28, 0x44, 0x00, 0x00}, // 107 k
    {0x00, 0x41, 0x7F, 0x40, 0x00, 0x00}, // 108 l
    {0x7C, 0x04, 0x78, 0x04, 0x78, 0x00}, // 109 m
    {0x7C, 0x08, 0x04, 0x04, 0x78, 0x00}, // 110 n
    {0x38, 0x44, 0x44, 0x44, 0x38, 0x00}, // 111 o
    {0xFC, 0x18, 0x24, 0x24, 0x18, 0x00}, // 112 p
    {0x18, 0x24, 0x24, 0x18, 0xFC, 0x00}, // 113 q
    {0x7C, 0x08, 0x04, 0x04, 0x08, 0x00}, // 114 r
    {0x48, 0x54, 0x54, 0x54, 0x24, 0x00}, // 115 s
    {0x04, 0x04, 0x3F, 0x44, 0x24, 0x00}, // 116 t
    {0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00}, // 117 u
    {0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00}, // 118 v
    {0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00}, // 119 w
    {0x44, 0x28, 0x10, 0x28, 0x44, 0x00}, // 120 x
    {0x4C, 0x90, 0x90, 0x90, 0x7C, 0x00}, // 121 y
    {0x44, 0x64, 0x54, 0x4C, 0x44, 0x00}, // 122 z
    {0x00, 0x08, 0x36, 0x41, 0x00, 0x00}, // 123 {
    {0x00, 0x00, 0x77, 0x00, 0x00, 0x00}, // 124 |
    {0x00, 0x41, 0x36, 0x08, 0x00, 0x00}, // 125 }
    {0x02, 0x01, 0x02, 0x04, 0x02, 0x00}, // 126 ~
    {0x3C, 0x26, 0x23, 0x26, 0x3C, 0x00}  // 127 DEL
};

/**
 * @brief Função auxiliar para valor absoluto
 */
static inline int abs_val(int x) {
    return (x < 0) ? -x : x;
}

/**
 * @brief Envia um comando para o SSD1306
 */
static void ssd1306_send_cmd(ssd1306_t *display, uint8_t cmd) {
    uint8_t buf[2] = {0x80, cmd};
    i2c_write_blocking(display->i2c, display->addr, buf, 2, false);
}

/**
 * @brief Envia dados para o SSD1306
 */
static void ssd1306_send_data(ssd1306_t *display, uint8_t *data, size_t len) {
    uint8_t buf[len + 1];
    buf[0] = 0x40;
    memcpy(buf + 1, data, len);
    i2c_write_blocking(display->i2c, display->addr, buf, len + 1, false);
}

bool ssd1306_init(ssd1306_t *display, i2c_inst_t *i2c, uint8_t addr) {
    display->i2c = i2c;
    display->addr = addr;
    
    // Sequência de inicialização do SSD1306
    ssd1306_send_cmd(display, SSD1306_SET_DISP | 0x00);  // Display OFF
    ssd1306_send_cmd(display, SSD1306_SET_MEM_ADDR);     // Set Memory Addressing Mode
    ssd1306_send_cmd(display, 0x00);                     // Horizontal Addressing Mode
    ssd1306_send_cmd(display, SSD1306_SET_DISP_START_LINE | 0x00); // Set Display Start Line
    ssd1306_send_cmd(display, SSD1306_SET_SEG_REMAP | 0x01);       // Set Segment Re-map
    ssd1306_send_cmd(display, SSD1306_SET_MUX_RATIO);              // Set Multiplex Ratio
    ssd1306_send_cmd(display, SSD1306_HEIGHT - 1);
    ssd1306_send_cmd(display, SSD1306_SET_COM_OUT_DIR | 0x08);     // Set COM Output Scan Direction
    ssd1306_send_cmd(display, SSD1306_SET_DISP_OFFSET);            // Set Display Offset
    ssd1306_send_cmd(display, 0x00);
    ssd1306_send_cmd(display, SSD1306_SET_COM_PIN_CFG);            // Set COM Pins hardware configuration
    ssd1306_send_cmd(display, 0x12);
    ssd1306_send_cmd(display, SSD1306_SET_CONTRAST);               // Set Contrast Control
    ssd1306_send_cmd(display, 0xFF);
    ssd1306_send_cmd(display, SSD1306_SET_PRECHARGE);              // Set Pre-charge Period
    ssd1306_send_cmd(display, 0xF1);
    ssd1306_send_cmd(display, SSD1306_SET_VCOM_DESEL);             // Set VCOMH Deselect Level
    ssd1306_send_cmd(display, 0x30);
    ssd1306_send_cmd(display, SSD1306_SET_ENTIRE_ON);              // Entire Display ON
    ssd1306_send_cmd(display, SSD1306_SET_NORM_INV);               // Set Normal Display
    ssd1306_send_cmd(display, SSD1306_SET_CHARGE_PUMP);            // Set Charge Pump
    ssd1306_send_cmd(display, 0x14);                               // Enable charge pump
    ssd1306_send_cmd(display, SSD1306_SET_DISP | 0x01);            // Display ON
    
    // Limpa o buffer
    ssd1306_clear(display);
    
    return true;
}

void ssd1306_clear(ssd1306_t *display) {
    memset(display->buffer, 0, SSD1306_BUFFER_SIZE);
}

void ssd1306_display(ssd1306_t *display) {
    ssd1306_send_cmd(display, SSD1306_SET_COL_ADDR);
    ssd1306_send_cmd(display, 0);   // Column start address
    ssd1306_send_cmd(display, SSD1306_WIDTH - 1); // Column end address
    ssd1306_send_cmd(display, SSD1306_SET_PAGE_ADDR);
    ssd1306_send_cmd(display, 0);   // Page start address
    ssd1306_send_cmd(display, (SSD1306_HEIGHT / 8) - 1); // Page end address
    
    ssd1306_send_data(display, display->buffer, SSD1306_BUFFER_SIZE);
}

void ssd1306_set_pixel(ssd1306_t *display, int x, int y, bool on) {
    if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT) {
        return;
    }
    
    if (on) {
        display->buffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));
    } else {
        display->buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

void ssd1306_draw_line(ssd1306_t *display, int x0, int y0, int x1, int y1, bool on) {
    int dx = abs_val(x1 - x0);
    int dy = abs_val(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        ssd1306_set_pixel(display, x0, y0, on);
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void ssd1306_draw_rect(ssd1306_t *display, int x, int y, int width, int height, bool on, bool filled) {
    if (filled) {
        for (int i = 0; i < height; i++) {
            ssd1306_draw_line(display, x, y + i, x + width - 1, y + i, on);
        }
    } else {
        ssd1306_draw_line(display, x, y, x + width - 1, y, on);
        ssd1306_draw_line(display, x, y + height - 1, x + width - 1, y + height - 1, on);
        ssd1306_draw_line(display, x, y, x, y + height - 1, on);
        ssd1306_draw_line(display, x + width - 1, y, x + width - 1, y + height - 1, on);
    }
}

void ssd1306_draw_circle(ssd1306_t *display, int cx, int cy, int radius, bool on, bool filled) {
    int x = radius;
    int y = 0;
    int err = 0;
    
    while (x >= y) {
        if (filled) {
            ssd1306_draw_line(display, cx - x, cy + y, cx + x, cy + y, on);
            ssd1306_draw_line(display, cx - x, cy - y, cx + x, cy - y, on);
            ssd1306_draw_line(display, cx - y, cy + x, cx + y, cy + x, on);
            ssd1306_draw_line(display, cx - y, cy - x, cx + y, cy - x, on);
        } else {
            ssd1306_set_pixel(display, cx + x, cy + y, on);
            ssd1306_set_pixel(display, cx + y, cy + x, on);
            ssd1306_set_pixel(display, cx - y, cy + x, on);
            ssd1306_set_pixel(display, cx - x, cy + y, on);
            ssd1306_set_pixel(display, cx - x, cy - y, on);
            ssd1306_set_pixel(display, cx - y, cy - x, on);
            ssd1306_set_pixel(display, cx + y, cy - x, on);
            ssd1306_set_pixel(display, cx + x, cy - y, on);
        }
        
        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

void ssd1306_draw_char(ssd1306_t *display, char c, int x, int y, bool on) {
    if (c < 32 || c > 127) {
        c = 127; // Use DEL character for unsupported chars
    }
    
    const uint8_t *char_data = font_6x8[c - 32];
    
    for (int i = 0; i < 6; i++) {
        uint8_t line = char_data[i];
        for (int j = 0; j < 8; j++) {
            if (line & (1 << j)) {
                ssd1306_set_pixel(display, x + i, y + j, on);
            }
        }
    }
}

void ssd1306_draw_string(ssd1306_t *display, const char *str, int x, int y, bool on) {
    int orig_x = x;
    
    while (*str) {
        if (*str == '\n') {
            y += 8;
            x = orig_x;
        } else {
            ssd1306_draw_char(display, *str, x, y, on);
            x += 6;
        }
        str++;
    }
} 