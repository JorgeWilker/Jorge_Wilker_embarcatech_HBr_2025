// Arquivo de cabeçalho para a biblioteca do display OLED SSD1306 via I2C

#ifndef SSD1306_H
#define SSD1306_H

#include <stdlib.h> // Para size_t, calloc
#include "pico/stdlib.h" // Funções padrão do Pico SDK
#include "hardware/i2c.h" // Funções de comunicação I2C

// Definições padrão para um display 128x64 (comuns)
#define SSD1306_WIDTH 128  // Largura do display em pixels
#define SSD1306_HEIGHT 64 // Altura do display em pixels

// Enumeração dos códigos de comando do datasheet do SSD1306
// Usado internamente pela função ssd1306_command()
typedef enum
{
  SET_CONTRAST = 0x81,        // Define o nível de contraste
  SET_ENTIRE_ON = 0xA4,       // 0xA4: usa RAM, 0xA5: acende todos os pixels
  SET_NORM_INV = 0xA6,        // 0xA6: Normal, 0xA7: Invertido
  SET_DISP = 0xAE,            // 0xAE: Display OFF, 0xAF: Display ON
  SET_MEM_ADDR = 0x20,        // Define o modo de endereçamento da memória
  SET_COL_ADDR = 0x21,        // Define o intervalo de colunas
  SET_PAGE_ADDR = 0x22,       // Define o intervalo de páginas
  SET_DISP_START_LINE = 0x40, // Define a linha inicial do display (0-63)
  SET_SEG_REMAP = 0xA0,       // Mapeamento de segmento (coluna) -> SEG0
  SET_MUX_RATIO = 0xA8,       // Define a taxa de multiplexação (altura - 1)
  SET_COM_OUT_DIR = 0xC0,     // Direção de varredura COM (linhas)
  SET_DISP_OFFSET = 0xD3,     // Define o deslocamento vertical
  SET_COM_PIN_CFG = 0xDA,     // Configuração dos pinos COM
  SET_DISP_CLK_DIV = 0xD5,    // Define a frequência do clock / oscilador
  SET_PRECHARGE = 0xD9,       // Define o período de pré-carga
  SET_VCOM_DESEL = 0xDB,      // Define o nível de dessseleção VCOMH
  SET_CHARGE_PUMP = 0x8D      // Configuração da bomba de carga interna
} ssd1306_command_t;

// Estrutura principal para representar e controlar um display SSD1306
typedef struct
{
  uint8_t width;        // Largura real configurada para o display
  uint8_t height;       // Altura real configurada para o display
  uint8_t pages;        // Número de páginas de memória (altura / 8)
  uint8_t address;      // Endereço I2C do display (ex: 0x3C)
  i2c_inst_t *i2c_port; // Ponteiro para a instância I2C (i2c0 ou i2c1)
  bool external_vcc;    // Flag para configuração de charge pump (não essencial aqui)
  uint8_t *ram_buffer;  // Ponteiro para o buffer em RAM (framebuffer)
  size_t bufsize;       // Tamanho total do ram_buffer em bytes
  uint8_t port_buffer[2]; // Buffer pequeno para enviar comandos I2C
} ssd1306_t;

// --- Protótipos das Funções Públicas --- 

// Inicializa a estrutura ssd1306_t, configura pinos e aloca memória para o buffer
void ssd1306_init(ssd1306_t *ssd, uint8_t width, uint8_t height, bool external_vcc, uint8_t address, i2c_inst_t *i2c);

// Envia a sequência de comandos de configuração inicial para o hardware do display
void ssd1306_config(ssd1306_t *ssd);

// Envia um único byte de comando para o display via I2C
void ssd1306_command(ssd1306_t *ssd, uint8_t command);

// Envia o conteúdo completo do ram_buffer (framebuffer) para a memória do display via I2C
void ssd1306_send_data(ssd1306_t *ssd);

// --- Funções de Desenho --- 

// Define o estado (ligado/desligado) de um pixel individual no ram_buffer
void ssd1306_pixel(ssd1306_t *ssd, uint8_t x, uint8_t y, bool value);

// Preenche todo o ram_buffer com um valor (0x00 para apagar, 0xFF para acender)
void ssd1306_fill(ssd1306_t *ssd, bool value);

// Desenha um retângulo (apenas contorno ou preenchido) no ram_buffer
void ssd1306_rect(ssd1306_t *ssd, uint8_t top, uint8_t left, uint8_t width, uint8_t height, bool value, bool fill);

// Desenha uma linha entre dois pontos (x0,y0) e (x1,y1) usando o algoritmo de Bresenham
void ssd1306_line(ssd1306_t *ssd, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool value);

// Desenha uma linha horizontal otimizada no ram_buffer
void ssd1306_hline(ssd1306_t *ssd, uint8_t x0, uint8_t x1, uint8_t y, bool value);

// Desenha uma linha vertical otimizada no ram_buffer
void ssd1306_vline(ssd1306_t *ssd, uint8_t x, uint8_t y0, uint8_t y1, bool value);

// Desenha um caractere (usando a fonte 8x8 de font.h) no ram_buffer na posição (x, y)
void ssd1306_draw_char(ssd1306_t *ssd, char c, uint8_t x, uint8_t y);

// Desenha uma string (usando fonte 8x8) no ram_buffer, com quebra de linha automática
void ssd1306_draw_string(ssd1306_t *ssd, const char *str, uint8_t x, uint8_t y);

// Desenha um caractere com tamanho dobrado (16x16 pixels) no ram_buffer
void ssd1306_draw_char_large(ssd1306_t *ssd, char c, uint8_t x, uint8_t y);

// Desenha uma string com tamanho dobrado (16x16) no ram_buffer, com quebra de linha
void ssd1306_draw_string_large(ssd1306_t *ssd, const char *str, uint8_t x, uint8_t y);

#endif // SSD1306_H
