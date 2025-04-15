#include "ssd1306.h"
#include "font.h"

/**
 * @brief Inicializa a estrutura ssd1306_t, aloca o buffer e define configurações básicas
 *
 * @param ssd Ponteiro para a estrutura ssd1306_t a ser inicializada.
 * @param width Largura do display em pixels.
 * @param height Altura do display em pixels.
 * @param external_vcc true se VCC for externo, false se usar charge pump interno.
 * @param address Endereço I2C do display.
 * @param i2c Ponteiro para a instância I2C (i2c0 ou i2c1).
 */
void ssd1306_init(ssd1306_t *ssd, uint8_t width, uint8_t height, bool external_vcc, uint8_t address, i2c_inst_t *i2c)
{
  ssd->width = width;
  ssd->height = height;
  ssd->pages = height / 8U;
  ssd->address = address;
  ssd->i2c_port = i2c;
  ssd->bufsize = ssd->pages * ssd->width + 1; // +1 para o byte de controle 0x40
  ssd->ram_buffer = calloc(ssd->bufsize, sizeof(uint8_t));
  ssd->ram_buffer[0] = 0x40; // Byte de controle I2C para envio de dados
  ssd->port_buffer[0] = 0x00; // Byte de controle I2C para envio de comando (não Co e D/C=0)
                               // Alternativamente, 0x00 também funciona para D/C=0
  ssd->external_vcc = external_vcc;
}

/**
 * @brief Envia a sequência de comandos de configuração inicial para o display
 *
 * Configura modo de endereçamento, mapeamento de segmentos/COM, clock, contraste, etc.
 * Baseado nas sequências comuns encontradas em datasheets e bibliotecas.
 *
 * @param ssd Ponteiro para a estrutura ssd1306_t inicializada.
 */
void ssd1306_config(ssd1306_t *ssd)
{
  ssd1306_command(ssd, SET_DISP | 0x00);           // 0xAE: Display OFF
  ssd1306_command(ssd, SET_MEM_ADDR);              // 0x20: Define modo de endereçamento
  ssd1306_command(ssd, 0x00);                      // 0x00: Modo Horizontal
  ssd1306_command(ssd, SET_PAGE_ADDR);             // 0x22: Define intervalo de páginas (não usado no modo horizontal)
  ssd1306_command(ssd, 0);                         // Página inicial 0
  ssd1306_command(ssd, ssd->pages - 1);            // Página final
  ssd1306_command(ssd, SET_DISP_START_LINE | 0x00); // 0x40: Linha inicial 0
  ssd1306_command(ssd, SET_SEG_REMAP | 0x01);       // 0xA1: Mapeamento SEG col 127 -> SEG0
  ssd1306_command(ssd, SET_MUX_RATIO);             // 0xA8: Define MUX ratio
  ssd1306_command(ssd, ssd->height - 1);           // Altura - 1
  ssd1306_command(ssd, SET_COM_OUT_DIR | 0x08);     // 0xC8: Scan direction COM N-1 -> COM0
  ssd1306_command(ssd, SET_DISP_OFFSET);           // 0xD3: Define display offset
  ssd1306_command(ssd, 0x00);                      // Sem offset
  ssd1306_command(ssd, SET_COM_PIN_CFG);           // 0xDA: Configuração dos pinos COM
  ssd1306_command(ssd, 0x12);                      // Configuração alternativa (pode variar com o display)
  ssd1306_command(ssd, SET_DISP_CLK_DIV);          // 0xD5: Define clock/osc frequency
  ssd1306_command(ssd, 0x80);                      // Valor padrão
  ssd1306_command(ssd, SET_PRECHARGE);             // 0xD9: Define período de pré-carga
  ssd1306_command(ssd, ssd->external_vcc ? 0x22 : 0xF1); // Depende do VCC (externo ou charge pump)
  ssd1306_command(ssd, SET_VCOM_DESEL);            // 0xDB: Define nível VCOMH deselect
  ssd1306_command(ssd, 0x30);                      // Ajuste comum (pode variar)
  ssd1306_command(ssd, SET_CONTRAST);              // 0x81: Define contraste
  ssd1306_command(ssd, 0xFF);                      // Contraste máximo
  ssd1306_command(ssd, SET_ENTIRE_ON);             // 0xA4: Display segue conteúdo da RAM
  ssd1306_command(ssd, SET_NORM_INV);              // 0xA6: Display normal (não invertido)
  ssd1306_command(ssd, SET_CHARGE_PUMP);           // 0x8D: Configuração do Charge Pump
  ssd1306_command(ssd, ssd->external_vcc ? 0x10 : 0x14); // Habilita se VCC não for externo
  ssd1306_command(ssd, SET_DISP | 0x01);           // 0xAF: Display ON
}

/**
 * @brief Envia um único byte de comando via I2C
 *
 * @param ssd Ponteiro para a estrutura ssd1306_t.
 * @param command O byte de comando a ser enviado.
 */
void ssd1306_command(ssd1306_t *ssd, uint8_t command)
{
  // Monta a mensagem I2C: [Byte de Controle Comando, Byte de Comando]
  ssd->port_buffer[1] = command;
  // Envia os 2 bytes via I2C de forma bloqueante
  i2c_write_blocking(
      ssd->i2c_port,    // Instância I2C
      ssd->address,     // Endereço do display
      ssd->port_buffer, // Buffer com [Control Byte, Command Byte]
      2,                // Número de bytes a enviar
      false);           // false: não envia STOP ao final (permite sequências)
}

/**
 * @brief Envia o conteúdo do ram_buffer para a memória do display via I2C
 *
 * Atualiza a tela com os dados desenhados no buffer local.
 * Define a janela de escrita para toda a tela antes de enviar.
 *
 * @param ssd Ponteiro para a estrutura ssd1306_t.
 */
void ssd1306_send_data(ssd1306_t *ssd)
{
  // Define a janela de escrita para abranger toda a tela
  // Necessário no modo de endereçamento horizontal
  ssd1306_command(ssd, SET_COL_ADDR);
  ssd1306_command(ssd, 0);             // Coluna inicial 0
  ssd1306_command(ssd, ssd->width - 1); // Coluna final (largura - 1)
  ssd1306_command(ssd, SET_PAGE_ADDR);
  ssd1306_command(ssd, 0);             // Página inicial 0
  ssd1306_command(ssd, ssd->pages - 1); // Página final (páginas - 1)

  // Envia o buffer de dados completo via I2C de forma bloqueante
  // O primeiro byte do ram_buffer (0x40) indica que são dados
  i2c_write_blocking(
      ssd->i2c_port,    // Instância I2C
      ssd->address,     // Endereço do display
      ssd->ram_buffer,  // Buffer com [Control Byte, Data Byte 1, ...]
      ssd->bufsize,     // Tamanho total do buffer (incluindo byte de controle)
      false);           // false: não envia STOP ao final
}

/**
 * @brief Define ou apaga um pixel no ram_buffer nas coordenadas (x, y)
 *
 * @param ssd Ponteiro para a estrutura ssd1306_t.
 * @param x Coordenada X do pixel.
 * @param y Coordenada Y do pixel.
 * @param value true para ligar o pixel (branco), false para desligar (preto).
 */
void ssd1306_pixel(ssd1306_t *ssd, uint8_t x, uint8_t y, bool value)
{
  // Verifica limites (opcional, mas bom para segurança)
  if (x >= ssd->width || y >= ssd->height) return;

  // Calcula o índice do byte no buffer e o bit dentro do byte
  // O buffer começa no índice 1 (índice 0 é o byte de controle)
  uint16_t index = (y / 8) * ssd->width + x + 1;
  uint8_t bit_mask = 1 << (y % 8);

  if (value)
    ssd->ram_buffer[index] |= bit_mask;  // Liga o bit
  else
    ssd->ram_buffer[index] &= ~bit_mask; // Desliga o bit
}

/**
 * @brief Preenche todo o ram_buffer (exceto o byte de controle) com 0x00 (apagado) ou 0xFF (aceso)
 *
 * @param ssd Ponteiro para a estrutura ssd1306_t.
 * @param value true para preencher com 0xFF (aceso), false para 0x00 (apagado).
 */
void ssd1306_fill(ssd1306_t *ssd, bool value)
{
  uint8_t fill_byte = value ? 0xFF : 0x00;
  for (uint16_t i = 1; i < ssd->bufsize; ++i)
  {
    ssd->ram_buffer[i] = fill_byte;
  }
}

// --- Funções de Desenho (Rect, Line, Hline, Vline) ---
// Os comentários existentes aqui são razoáveis e explicam a lógica.
// Vou mantê-los como estão para evitar excesso de comentários.

/**
 * @brief Desenha um retângulo no ram_buffer.
 *
 * @param ssd Ponteiro para a estrutura ssd1306_t.
 * @param top Coordenada Y do canto superior esquerdo.
 * @param left Coordenada X do canto superior esquerdo.
 * @param width Largura do retângulo.
 * @param height Altura do retângulo.
 * @param value Cor do contorno (true=branco, false=preto).
 * @param fill true para preencher o retângulo, false para desenhar apenas o contorno.
 */
void ssd1306_rect(ssd1306_t *ssd, uint8_t top, uint8_t left, uint8_t width, uint8_t height, bool value, bool fill)
{
  for (uint8_t x = left; x < left + width; ++x)
  {
    ssd1306_pixel(ssd, x, top, value);
    ssd1306_pixel(ssd, x, top + height - 1, value);
  }
  for (uint8_t y = top; y < top + height; ++y)
  {
    ssd1306_pixel(ssd, left, y, value);
    ssd1306_pixel(ssd, left + width - 1, y, value);
  }

  if (fill)
  {
    for (uint8_t x = left + 1; x < left + width - 1; ++x)
    {
      for (uint8_t y = top + 1; y < top + height - 1; ++y)
      {
        ssd1306_pixel(ssd, x, y, value);
      }
    }
  }
}

/**
 * @brief Desenha uma linha entre dois pontos usando o algoritmo de Bresenham.
 *
 * @param ssd Ponteiro para a estrutura ssd1306_t.
 * @param x0 Coordenada X do ponto inicial.
 * @param y0 Coordenada Y do ponto inicial.
 * @param x1 Coordenada X do ponto final.
 * @param y1 Coordenada Y do ponto final.
 * @param value Cor da linha (true=branco, false=preto).
 */
void ssd1306_line(ssd1306_t *ssd, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool value)
{
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);

  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;

  int err = dx - dy;

  while (true)
  {
    ssd1306_pixel(ssd, x0, y0, value); // Desenha o pixel atual

    if (x0 == x1 && y0 == y1)
      break; // Termina quando alcança o ponto final

    int e2 = err * 2;

    if (e2 > -dy)
    {
      err -= dy;
      x0 += sx;
    }

    if (e2 < dx)
    {
      err += dx;
      y0 += sy;
    }
  }
}

/**
 * @brief Desenha uma linha horizontal otimizada.
 *
 * @param ssd Ponteiro para a estrutura ssd1306_t.
 * @param x0 Coordenada X inicial.
 * @param x1 Coordenada X final.
 * @param y Coordenada Y da linha.
 * @param value Cor da linha (true=branco, false=preto).
 */
void ssd1306_hline(ssd1306_t *ssd, uint8_t x0, uint8_t x1, uint8_t y, bool value)
{
  for (uint8_t x = x0; x <= x1; ++x)
    ssd1306_pixel(ssd, x, y, value);
}

/**
 * @brief Desenha uma linha vertical otimizada.
 *
 * @param ssd Ponteiro para a estrutura ssd1306_t.
 * @param x Coordenada X da linha.
 * @param y0 Coordenada Y inicial.
 * @param y1 Coordenada Y final.
 * @param value Cor da linha (true=branco, false=preto).
 */
void ssd1306_vline(ssd1306_t *ssd, uint8_t x, uint8_t y0, uint8_t y1, bool value)
{
  for (uint8_t y = y0; y <= y1; ++y)
    ssd1306_pixel(ssd, x, y, value);
}

/**
 * @brief Desenha um caractere 8x8 no ram_buffer usando a fonte de font.h.
 *
 * @param ssd Ponteiro para a estrutura ssd1306_t.
 * @param c O caractere ASCII a ser desenhado.
 * @param x Coordenada X do canto superior esquerdo do caractere.
 * @param y Coordenada Y do canto superior esquerdo do caractere.
 */
void ssd1306_draw_char(ssd1306_t *ssd, char c, uint8_t x, uint8_t y)
{
  // Trata caracteres fora da faixa ASCII visível (32-126)
  if (c < ' ' || c > '~') {
     c = '?'; // Usa '?' como caractere padrão para desconhecidos/inválidos
  }
  
  // Calcula o índice inicial na tabela de fontes (font.h)
  // A fonte fornecida mapeia ' ' (32) para o índice 0, '!' (33) para 1*8, etc.
  uint16_t font_index = (c - ' ') * 8; // Índice baseado no valor ASCII relativo ao espaço

  // Desenha as 8 colunas do caractere
  for (uint8_t i = 0; i < 8; ++i) {
    // Verifica se o índice calculado + i está dentro dos limites do array font
    // Isso evita ler fora da memória se o cálculo do índice estiver errado
    // (Considerando que font.h cobre de ASCII 32 a 126, são 95 caracteres * 8 bytes = 760 bytes)
    // Uma verificação mais robusta seria comparar com o tamanho real do array se disponível.
    // if (font_index + i >= sizeof(font)) continue; // Exemplo se sizeof(font) fosse conhecido

    uint8_t column_data = font[font_index + i]; // Lê os dados da coluna da fonte
    
    // Desenha os 8 pixels da coluna atual
    for (uint8_t j = 0; j < 8; ++j) {
      // Verifica se o j-ésimo bit da coluna está ligado
      if (column_data & (1 << j)) {
        // Desenha o pixel correspondente no buffer
        ssd1306_pixel(ssd, x + i, y + j, true);
      }
      // Opcional: poderia desenhar 'false' se o bit estivesse desligado,
      // mas ssd1306_fill(false) geralmente limpa o fundo antes.
    }
  }
}

/**
 * @brief Desenha uma string (fonte 8x8) no ram_buffer, com quebra de linha.
 *
 * @param ssd Ponteiro para a estrutura ssd1306_t.
 * @param str Ponteiro para a string terminada em nulo.
 * @param x Coordenada X inicial.
 * @param y Coordenada Y inicial.
 */
void ssd1306_draw_string(ssd1306_t *ssd, const char *str, uint8_t x, uint8_t y)
{
  uint8_t start_x = x; // Guarda a posição X inicial para quebra de linha
  // Itera pela string até encontrar o caractere nulo
  while (*str) {
    // Desenha o caractere atual
    ssd1306_draw_char(ssd, *str, x, y);
    str++; // Avança para o próximo caractere na string
    x += 8; // Avança a posição X para o próximo caractere (largura 8)
    
    // Verifica se o próximo caractere ultrapassaria a largura do display
    if (x >= ssd->width) { // Verifica se a posição ATUAL já saiu ou está na borda
      x = start_x; // Volta para a margem X inicial
      y += 8;      // Move para a próxima linha (altura 8)
    }
    // Verifica se a próxima linha ultrapassaria a altura do display
    if (y >= ssd->height) { // Verifica se a posição ATUAL já saiu
      break; // Para de desenhar se sair da tela
    }
  }
}

/**
 * @brief Desenha um caractere 16x16 (ampliando a fonte 8x8) no ram_buffer.
 *
 * @param ssd Ponteiro para a estrutura ssd1306_t.
 * @param c O caractere ASCII a ser desenhado.
 * @param x Coordenada X do canto superior esquerdo.
 * @param y Coordenada Y do canto superior esquerdo.
 */
void ssd1306_draw_char_large(ssd1306_t *ssd, char c, uint8_t x, uint8_t y) {
  // Trata caracteres fora da faixa
  if (c < ' ' || c > '~') c = '?';

  // Calcula índice na fonte 8x8 (igual a draw_char)
  uint16_t font_index = (c - ' ') * 8;

  // Itera pelos dados da fonte 8x8
  for (uint8_t i = 0; i < 8; ++i) {
      // if (font_index + i >= sizeof(font)) continue; // Verificação de limites (opcional)
      uint8_t column_data = font[font_index + i];
      for (uint8_t j = 0; j < 8; ++j) {
          // Verifica se o pixel original (8x8) está ligado
          bool pixel_on = (column_data & (1 << j));
          // Desenha um bloco 2x2 para cada pixel original ligado
          if (pixel_on) {
            // Calcula as coordenadas X e Y do bloco 2x2
            uint8_t large_x = x + (i * 2);
            uint8_t large_y = y + (j * 2);
            // Desenha os 4 pixels do bloco, verificando limites da tela
            if (large_x < ssd->width && large_y < ssd->height) 
              ssd1306_pixel(ssd, large_x,     large_y,     true);
            if (large_x + 1 < ssd->width && large_y < ssd->height)
              ssd1306_pixel(ssd, large_x + 1, large_y,     true);
            if (large_x < ssd->width && large_y + 1 < ssd->height)
              ssd1306_pixel(ssd, large_x,     large_y + 1, true);
            if (large_x + 1 < ssd->width && large_y + 1 < ssd->height)
              ssd1306_pixel(ssd, large_x + 1, large_y + 1, true);
          }
      }
  }
}

/**
 * @brief Desenha uma string (fonte 16x16 ampliada) no ram_buffer, com quebra de linha.
 *
 * @param ssd Ponteiro para a estrutura ssd1306_t.
 * @param str Ponteiro para a string terminada em nulo.
 * @param x Coordenada X inicial.
 * @param y Coordenada Y inicial.
 */
void ssd1306_draw_string_large(ssd1306_t *ssd, const char *str, uint8_t x, uint8_t y) {
  uint8_t start_x = x; // Guarda X inicial
  while (*str) {
      // Desenha o caractere ampliado atual
      ssd1306_draw_char_large(ssd, *str, x, y);
      str++; // Próximo caractere
      x += 16; // Avança 16 pixels (largura do caractere ampliado)
      
      // Quebra de linha se necessário
      if (x >= ssd->width) {
          x = start_x;
          y += 16; // Pula 16 pixels para a próxima linha
      }
      // Para se sair da tela
      if (y >= ssd->height) {
          break;
      }
  }
}
