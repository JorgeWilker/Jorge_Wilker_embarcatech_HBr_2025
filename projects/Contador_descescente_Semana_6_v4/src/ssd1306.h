#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define WIDTH 128  // Largura padrão do display SSD1306
#define HEIGHT 64 // Altura padrão do display SSD1306

// Enumeração dos comandos comuns do SSD1306
typedef enum
{
  SET_CONTRAST = 0x81,
  SET_ENTIRE_ON = 0xA4,
  SET_NORM_INV = 0xA6,
  SET_DISP = 0xAE,
  SET_MEM_ADDR = 0x20,
  SET_COL_ADDR = 0x21,
  SET_PAGE_ADDR = 0x22,
  SET_DISP_START_LINE = 0x40,
  SET_SEG_REMAP = 0xA0,
  SET_MUX_RATIO = 0xA8,
  SET_COM_OUT_DIR = 0xC0,
  SET_DISP_OFFSET = 0xD3,
  SET_COM_PIN_CFG = 0xDA,
  SET_DISP_CLK_DIV = 0xD5,
  SET_PRECHARGE = 0xD9,
  SET_VCOM_DESEL = 0xDB,
  SET_CHARGE_PUMP = 0x8D
} ssd1306_command_t;

// Estrutura para gerenciar o estado e configuração do display SSD1306
typedef struct
{
  uint8_t width;       // Largura do display em pixels
  uint8_t height;      // Altura do display em pixels
  uint8_t pages;       // Número de páginas (altura / 8)
  uint8_t address;     // Endereço I2C do display
  i2c_inst_t *i2c_port; // Ponteiro para a instância I2C usada
  bool external_vcc;   // Indica se VCC é externo (não usado nesta implementação)
  uint8_t *ram_buffer; // Buffer em RAM para armazenar o conteúdo do display
  size_t bufsize;      // Tamanho do ram_buffer (pages * width + 1 para o byte de controle)
  uint8_t port_buffer[2]; // Buffer temporário para enviar comandos I2C (byte de controle + comando)
} ssd1306_t;

// --- Protótipos das Funções --- 

// Inicializa a estrutura ssd1306_t e aloca memória para o buffer
void ssd1306_init(ssd1306_t *ssd, uint8_t width, uint8_t height, bool external_vcc, uint8_t address, i2c_inst_t *i2c);
// Envia a sequência de comandos de configuração inicial para o display
void ssd1306_config(ssd1306_t *ssd);
// Envia um único byte de comando para o display via I2C
void ssd1306_command(ssd1306_t *ssd, uint8_t command);
// Envia o conteúdo do ram_buffer para a memória do display via I2C
void ssd1306_send_data(ssd1306_t *ssd);

// Define o estado de um pixel individual no ram_buffer
void ssd1306_pixel(ssd1306_t *ssd, uint8_t x, uint8_t y, bool value);
// Preenche todo o ram_buffer com um valor (liga ou desliga todos os pixels)
void ssd1306_fill(ssd1306_t *ssd, bool value);
// Desenha um retângulo no ram_buffer (com ou sem preenchimento)
void ssd1306_rect(ssd1306_t *ssd, uint8_t top, uint8_t left, uint8_t width, uint8_t height, bool value, bool fill);
// Desenha uma linha entre dois pontos no ram_buffer (algoritmo de Bresenham)
void ssd1306_line(ssd1306_t *ssd, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool value);
// Desenha uma linha horizontal no ram_buffer
void ssd1306_hline(ssd1306_t *ssd, uint8_t x0, uint8_t x1, uint8_t y, bool value);
// Desenha uma linha vertical no ram_buffer
void ssd1306_vline(ssd1306_t *ssd, uint8_t x, uint8_t y0, uint8_t y1, bool value);
// Desenha um caractere (fonte 8x8) no ram_buffer na posição especificada
void ssd1306_draw_char(ssd1306_t *ssd, char c, uint8_t x, uint8_t y);
// Desenha uma string (fonte 8x8) no ram_buffer, com quebra de linha automática
void ssd1306_draw_string(ssd1306_t *ssd, const char *str, uint8_t x, uint8_t y);
// Desenha um caractere com tamanho dobrado (16x16) no ram_buffer
void ssd1306_draw_char_large(ssd1306_t *ssd, char c, uint8_t x, uint8_t y);
// Desenha uma string com tamanho dobrado (16x16) no ram_buffer
void ssd1306_draw_string_large(ssd1306_t *ssd, const char *str, uint8_t x, uint8_t y);
