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
 * @file ssd1306_i2c.h
 * @brief Driver para display OLED SSD1306 via I2C
 * 
 * Driver para comunicação com display OLED SSD1306 usando protocolo I2C
 * otimizado para Raspberry Pi Pico com resolução 128x64 pixels.
 * 
 * @author Jorge Wilker
 * @date Maio 2025
 */

#ifndef SSD1306_I2C_H
#define SSD1306_I2C_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"

/**
 * @brief Configurações do display SSD1306
 */
#define SSD1306_WIDTH       128    /**< Largura do display em pixels */
#define SSD1306_HEIGHT      64     /**< Altura do display em pixels */
#define SSD1306_BUFFER_SIZE ((SSD1306_WIDTH * SSD1306_HEIGHT) / 8) /**< Tamanho do buffer de vídeo */

/**
 * @brief Estrutura do display SSD1306
 */
typedef struct {
    i2c_inst_t *i2c;      /**< Instância I2C utilizada */
    uint8_t addr;          /**< Endereço I2C do display */
    uint8_t buffer[SSD1306_BUFFER_SIZE]; /**< Buffer de vídeo local */
} ssd1306_t;

/**
 * @brief Inicializa o display SSD1306
 * 
 * Configura o display OLED SSD1306 e prepara para operação.
 * 
 * @param display Ponteiro para a estrutura do display
 * @param i2c Instância I2C a ser utilizada
 * @param addr Endereço I2C do display (tipicamente 0x3C ou 0x3D)
 * @return true se a inicialização foi bem-sucedida, false caso contrário
 */
bool ssd1306_init(ssd1306_t *display, i2c_inst_t *i2c, uint8_t addr);

/**
 * @brief Limpa o buffer do display
 * 
 * Apaga todo o conteúdo do buffer de vídeo, tornando a tela em branco.
 * 
 * @param display Ponteiro para a estrutura do display
 */
void ssd1306_clear(ssd1306_t *display);

/**
 * @brief Atualiza o display com o conteúdo do buffer
 * 
 * Transfere o buffer de vídeo local para o display via I2C.
 * 
 * @param display Ponteiro para a estrutura do display
 */
void ssd1306_display(ssd1306_t *display);

/**
 * @brief Define um pixel no buffer
 * 
 * Acende ou apaga um pixel específico no buffer de vídeo.
 * 
 * @param display Ponteiro para a estrutura do display
 * @param x Coordenada X do pixel (0 a 127)
 * @param y Coordenada Y do pixel (0 a 63)
 * @param on true para acender o pixel, false para apagar
 */
void ssd1306_set_pixel(ssd1306_t *display, int x, int y, bool on);

/**
 * @brief Desenha uma linha no buffer
 * 
 * Desenha uma linha reta entre dois pontos.
 * 
 * @param display Ponteiro para a estrutura do display
 * @param x0 Coordenada X inicial
 * @param y0 Coordenada Y inicial
 * @param x1 Coordenada X final
 * @param y1 Coordenada Y final
 * @param on true para linha visível, false para apagar
 */
void ssd1306_draw_line(ssd1306_t *display, int x0, int y0, int x1, int y1, bool on);

/**
 * @brief Desenha um retângulo no buffer
 * 
 * Desenha um retângulo com ou sem preenchimento.
 * 
 * @param display Ponteiro para a estrutura do display
 * @param x Coordenada X do canto superior esquerdo
 * @param y Coordenada Y do canto superior esquerdo
 * @param width Largura do retângulo
 * @param height Altura do retângulo
 * @param on true para contorno visível, false para apagar
 * @param filled true para retângulo preenchido, false apenas contorno
 */
void ssd1306_draw_rect(ssd1306_t *display, int x, int y, int width, int height, bool on, bool filled);

/**
 * @brief Desenha um círculo no buffer
 * 
 * Desenha um círculo com ou sem preenchimento.
 * 
 * @param display Ponteiro para a estrutura do display
 * @param cx Coordenada X do centro
 * @param cy Coordenada Y do centro
 * @param radius Raio do círculo
 * @param on true para contorno visível, false para apagar
 * @param filled true para círculo preenchido, false apenas contorno
 */
void ssd1306_draw_circle(ssd1306_t *display, int cx, int cy, int radius, bool on, bool filled);

/**
 * @brief Desenha uma string no buffer
 * 
 * Renderiza texto usando fonte bitmap 6x8 pixels.
 * 
 * @param display Ponteiro para a estrutura do display
 * @param str String a ser desenhada (terminada em nulo)
 * @param x Coordenada X inicial do texto
 * @param y Coordenada Y inicial do texto
 * @param on true para texto visível, false para apagar
 */
void ssd1306_draw_string(ssd1306_t *display, const char *str, int x, int y, bool on);

/**
 * @brief Desenha um caractere no buffer
 * 
 * Renderiza um único caractere usando fonte bitmap 6x8 pixels.
 * 
 * @param display Ponteiro para a estrutura do display
 * @param c Caractere a ser desenhado
 * @param x Coordenada X inicial
 * @param y Coordenada Y inicial
 * @param on true para caractere visível, false para apagar
 */
void ssd1306_draw_char(ssd1306_t *display, char c, int x, int y, bool on);

#endif // SSD1306_I2C_H 