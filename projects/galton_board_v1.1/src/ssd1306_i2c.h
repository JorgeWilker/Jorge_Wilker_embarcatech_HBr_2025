/**
 * @file ssd1306_i2c.h
 * @brief Driver para display OLED SSD1306 via I2C
 *
 * Fornece funções para controlar o display SSD1306 e desenhar gráficos.
 *
 * @author Jorge Wilker
 * @date Maio 2025
 */

#ifndef SSD1306_I2C_H
#define SSD1306_I2C_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

/**
 * @brief Dimensões do display
 */
#define OLED_WIDTH 128               /**< Largura do display em pixels */
#define OLED_HEIGHT 64               /**< Altura do display em pixels */
#define OLED_PAGES (OLED_HEIGHT / 8) /**< Número de páginas verticais (cada página tem 8 pixels) */

/**
 * @brief Comandos do controlador SSD1306
 */
#define SSD1306_SET_CONTRAST 0x81          /**< Define o contraste do display */
#define SSD1306_DISPLAY_RAM 0xA4           /**< Exibe o conteúdo da RAM */
#define SSD1306_DISPLAY_NORMAL 0xA6        /**< Modo de exibição normal (não invertido) */
#define SSD1306_DISPLAY_OFF 0xAE           /**< Desliga o display */
#define SSD1306_DISPLAY_ON 0xAF            /**< Liga o display */
#define SSD1306_SET_DISPLAY_OFFSET 0xD3    /**< Define o offset vertical do display */
#define SSD1306_SET_COM_PINS 0xDA          /**< Configura os pinos COM */
#define SSD1306_SET_VCOM_DETECT 0xDB       /**< Configura detecção VCOM */
#define SSD1306_SET_DISPLAY_CLOCK_DIV 0xD5 /**< Define o divisor do clock */
#define SSD1306_SET_PRECHARGE 0xD9         /**< Define período de pré-carga */
#define SSD1306_SET_MULTIPLEX 0xA8         /**< Configura ratio de multiplexação */
#define SSD1306_SET_LOW_COLUMN 0x00        /**< Define coluna baixa */
#define SSD1306_SET_HIGH_COLUMN 0x10       /**< Define coluna alta */
#define SSD1306_SET_START_LINE 0x40        /**< Define linha inicial de exibição */
#define SSD1306_MEMORY_MODE 0x20           /**< Define modo de endereçamento de memória */
#define SSD1306_COLUMN_ADDR 0x21           /**< Define endereço de coluna */
#define SSD1306_PAGE_ADDR 0x22             /**< Define endereço de página */
#define SSD1306_COM_SCAN_INC 0xC0          /**< Varredura COM incremental */
#define SSD1306_COM_SCAN_DEC 0xC8          /**< Varredura COM decremental */
#define SSD1306_SEG_REMAP 0xA0             /**< Remapeamento de segmento */
#define SSD1306_CHARGE_PUMP 0x8D           /**< Configuração da bomba de carga */

/**
 * @brief Estrutura que representa um display SSD1306
 */
typedef struct
{
    i2c_inst_t *i2c_port;                    /**< Instância do periférico I2C utilizado */
    uint8_t i2c_addr;                        /**< Endereço I2C do display (geralmente 0x3C ou 0x3D) */
    uint8_t buffer[OLED_WIDTH * OLED_PAGES]; /**< Buffer de framebuffer (1 bit por pixel) */
} ssd1306_t;

/**
 * @brief Inicializa o display OLED
 *
 * Configura o hardware e inicializa o controlador SSD1306 com os parâmetros padrão.
 *
 * @param oled Ponteiro para estrutura do display a ser inicializada
 * @param i2c_port Instância I2C a ser utilizada
 * @param i2c_addr Endereço I2C do display (geralmente 0x3C)
 */
void ssd1306_init(ssd1306_t *oled, i2c_inst_t *i2c_port, uint8_t i2c_addr);

/**
 * @brief Limpa o buffer do display
 *
 * Preenche o buffer com zeros (pixels apagados). Não atualiza o display físico
 * até que ssd1306_display() seja chamada.
 *
 * @param oled Ponteiro para estrutura do display
 */
void ssd1306_clear(ssd1306_t *oled);

/**
 * @brief Atualiza o display físico com o conteúdo do buffer
 *
 * Envia o conteúdo do buffer interno para o controlador SSD1306 via I2C,
 * fazendo com que as alterações se tornem visíveis no display físico.
 *
 * @param oled Ponteiro para estrutura do display
 */
void ssd1306_display(ssd1306_t *oled);

/**
 * @brief Desenha um pixel no buffer
 *
 * @param oled Ponteiro para estrutura do display
 * @param x Coordenada X do pixel (0 a OLED_WIDTH-1)
 * @param y Coordenada Y do pixel (0 a OLED_HEIGHT-1)
 * @param color Estado do pixel (true = aceso, false = apagado)
 */
void ssd1306_draw_pixel(ssd1306_t *oled, int x, int y, bool color);

/**
 * @brief Desenha uma linha no buffer
 *
 * Implementa o algoritmo de Bresenham para desenhar linhas.
 *
 * @param oled Ponteiro para estrutura do display
 * @param x0 Coordenada X inicial
 * @param y0 Coordenada Y inicial
 * @param x1 Coordenada X final
 * @param y1 Coordenada Y final
 * @param color Estado dos pixels da linha (true = aceso, false = apagado)
 */
void ssd1306_draw_line(ssd1306_t *oled, int x0, int y0, int x1, int y1, bool color);

/**
 * @brief Desenha um retângulo no buffer
 *
 * @param oled Ponteiro para estrutura do display
 * @param x Coordenada X do canto superior esquerdo
 * @param y Coordenada Y do canto superior esquerdo
 * @param w Largura do retângulo
 * @param h Altura do retângulo
 * @param filled Se true, preenche o retângulo; se false, desenha apenas o contorno
 * @param color Estado dos pixels do retângulo (true = aceso, false = apagado)
 */
void ssd1306_draw_rect(ssd1306_t *oled, int x, int y, int w, int h, bool filled, bool color);

/**
 * @brief Desenha um círculo no buffer
 *
 * Implementa o algoritmo de Bresenham para círculos.
 *
 * @param oled Ponteiro para estrutura do display
 * @param x0 Coordenada X do centro
 * @param y0 Coordenada Y do centro
 * @param r Raio do círculo
 * @param filled Se true, preenche o círculo; se false, desenha apenas o contorno
 * @param color Estado dos pixels do círculo (true = aceso, false = apagado)
 */
void ssd1306_draw_circle(ssd1306_t *oled, int x0, int y0, int r, bool filled, bool color);

/**
 * @brief Desenha um caractere no buffer
 *
 * Usa fonte 5x7 embutida.
 *
 * @param oled Ponteiro para estrutura do display
 * @param c Caractere a ser desenhado
 * @param x Coordenada X inicial
 * @param y Coordenada Y inicial
 * @param color Estado dos pixels do caractere (true = aceso, false = apagado)
 */
void ssd1306_draw_char(ssd1306_t *oled, char c, int x, int y, bool color);

/**
 * @brief Desenha uma string no buffer
 *
 * @param oled Ponteiro para estrutura do display
 * @param str String a ser desenhada (terminada em nulo)
 * @param x Coordenada X inicial
 * @param y Coordenada Y inicial
 * @param color Estado dos pixels da string (true = aceso, false = apagado)
 */
void ssd1306_draw_string(ssd1306_t *oled, const char *str, int x, int y, bool color);

#endif // SSD1306_I2C_H