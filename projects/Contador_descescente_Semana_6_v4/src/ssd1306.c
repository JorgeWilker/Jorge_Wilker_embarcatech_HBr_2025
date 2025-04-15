#include "ssd1306.h"
#include "font.h"

// Inicializa a estrutura ssd1306_t, aloca o buffer e define configurações básicas
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
  ssd->port_buffer[0] = 0x80; // Byte de controle I2C para envio de comando (não Co e D/C=0)
                               // Alternativamente, 0x00 também funciona para D/C=0
}

// Envia a sequência de comandos de configuração inicial para o display
void ssd1306_config(ssd1306_t *ssd)
{
  ssd1306_command(ssd, SET_DISP | 0x00); // Display OFF
  ssd1306_command(ssd, SET_MEM_ADDR);    // Define modo de endereçamento de memória
  ssd1306_command(ssd, 0x00);            // Modo horizontal
  ssd1306_command(ssd, SET_PAGE_ADDR);   // Define endereços de página (não usado no modo horizontal)
  ssd1306_command(ssd, 0);               // Página inicial 0
  ssd1306_command(ssd, ssd->pages - 1);  // Página final
  ssd1306_command(ssd, SET_DISP_START_LINE | 0x00); // Linha de início do display em 0
  ssd1306_command(ssd, SET_SEG_REMAP | 0x01); // Mapeamento de segmento: col 127 -> SEG0
  ssd1306_command(ssd, SET_MUX_RATIO);       // Define taxa de multiplexação
  ssd1306_command(ssd, ssd->height - 1);     // Para altura do display
  ssd1306_command(ssd, SET_COM_OUT_DIR | 0x08); // Direção de varredura COM: remapeada C8h
  ssd1306_command(ssd, SET_DISP_OFFSET);     // Define deslocamento vertical do display
  ssd1306_command(ssd, 0x00);                // Sem deslocamento
  ssd1306_command(ssd, SET_COM_PIN_CFG);     // Configuração dos pinos COM
  ssd1306_command(ssd, 0x12);                // Modo sequencial, remapeamento COM E/D desabilitado
  ssd1306_command(ssd, SET_DISP_CLK_DIV);    // Define divisor do clock / oscilador
  ssd1306_command(ssd, 0x80);                // Valor padrão
  ssd1306_command(ssd, SET_PRECHARGE);       // Define período de pré-carga
  ssd1306_command(ssd, ssd->external_vcc ? 0x22 : 0xF1); // Depende de VCC externo ou interno
  ssd1306_command(ssd, SET_VCOM_DESEL);      // Define nível de dessseleção VCOMH
  ssd1306_command(ssd, 0x30);                // ~0.83 x VCC
  ssd1306_command(ssd, SET_CONTRAST);        // Define contraste
  ssd1306_command(ssd, 0xFF);                // Contraste máximo
  ssd1306_command(ssd, SET_ENTIRE_ON);       // Desabilita "display inteiro LIGADO" (usa dados da RAM)
  ssd1306_command(ssd, SET_NORM_INV);        // Display normal (não invertido)
  ssd1306_command(ssd, SET_CHARGE_PUMP);     // Habilita charge pump
  ssd1306_command(ssd, 0x14);                // Habilitar
  ssd1306_command(ssd, SET_DISP | 0x01);     // Display ON
}

// Envia um único byte de comando via I2C
void ssd1306_command(ssd1306_t *ssd, uint8_t command)
{
  // O primeiro byte (0x80 ou 0x00) indica que é um comando
  ssd->port_buffer[1] = command;
  i2c_write_blocking(
      ssd->i2c_port,
      ssd->address,
      ssd->port_buffer, // Envia [Control Byte, Command Byte]
      2,
      false); // Não envia STOP no final (pode ser útil para sequências)
}

// Envia o conteúdo do ram_buffer para a memória do display via I2C
void ssd1306_send_data(ssd1306_t *ssd)
{
  // Define a janela de escrita para toda a tela (necessário no modo horizontal)
  ssd1306_command(ssd, SET_COL_ADDR);
  ssd1306_command(ssd, 0);             // Coluna inicial 0
  ssd1306_command(ssd, ssd->width - 1); // Coluna final
  ssd1306_command(ssd, SET_PAGE_ADDR);
  ssd1306_command(ssd, 0);             // Página inicial 0
  ssd1306_command(ssd, ssd->pages - 1); // Página final

  // Envia o buffer inteiro (incluindo o byte de controle 0x40 no início)
  i2c_write_blocking(
      ssd->i2c_port,
      ssd->address,
      ssd->ram_buffer, // Envia [Control Byte, Data Byte 1, Data Byte 2, ...]
      ssd->bufsize,
      false);
}

// Define ou apaga um pixel no ram_buffer nas coordenadas (x, y)
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

// Preenche todo o ram_buffer (exceto o byte de controle) com 0x00 (apagado) ou 0xFF (aceso)
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

void ssd1306_hline(ssd1306_t *ssd, uint8_t x0, uint8_t x1, uint8_t y, bool value)
{
  for (uint8_t x = x0; x <= x1; ++x)
    ssd1306_pixel(ssd, x, y, value);
}

void ssd1306_vline(ssd1306_t *ssd, uint8_t x, uint8_t y0, uint8_t y1, bool value)
{
  for (uint8_t y = y0; y <= y1; ++y)
    ssd1306_pixel(ssd, x, y, value);
}

// Desenha um caractere 8x8 no ram_buffer
void ssd1306_draw_char(ssd1306_t *ssd, char c, uint8_t x, uint8_t y)
{
  // Trata caracteres fora da faixa suportada pela fonte (pode exibir espaço ou nada)
  if (c < ' ' || c > '~') { // Assumindo que a fonte cobre ASCII 32-126
     // Pode adicionar um caractere padrão aqui se desejar (ex: '?')
     c = ' '; // Exibe espaço para caracteres desconhecidos
  }
  
  // Calcula o índice inicial na tabela de fontes (font.h)
  // Esta lógica depende de como a tabela 'font' está organizada.
  // Assumindo: Espaço, 0-9, A-Z, a-z, Símbolos...
  // O cálculo original parece correto para a fonte fornecida.
  uint16_t index = 0;
  if (c >= 'A' && c <= 'Z') index = (c - 'A' + 11) * 8;
  else if (c >= '0' && c <= '9') index = (c - '0' + 1) * 8;
  else if (c >= 'a' && c <= 'z') index = (c - 'a' + 37) * 8;
  else if (c == ' ') index = 0; // Trata espaço explicitamente
  // Adicionar mapeamento para outros símbolos se necessário, baseado em font.h
  else index = 0; // Padrão para espaço se não mapeado

  // Desenha os 8 bytes (colunas) do caractere
  for (uint8_t i = 0; i < 8; ++i) {
    uint8_t line = font[index + i];
    // Desenha os 8 pixels (linhas) de cada coluna
    for (uint8_t j = 0; j < 8; ++j) {
      // Desenha o pixel apenas se o bit correspondente estiver ligado na fonte
      if (line & (1 << j)) {
        ssd1306_pixel(ssd, x + i, y + j, true);
      }
      // (Opcional: poderia desenhar false se o fundo não for limpo antes)
    }
  }
}

// Desenha uma string (fonte 8x8) no ram_buffer
void ssd1306_draw_string(ssd1306_t *ssd, const char *str, uint8_t x, uint8_t y)
{
  uint8_t start_x = x;
  while (*str) {
    ssd1306_draw_char(ssd, *str++, x, y);
    x += 8; // Avança para a próxima posição de caractere
    // Quebra de linha se atingir a borda direita
    if (x + 8 > ssd->width) { // Verifica se o *próximo* caractere caberia
      x = start_x; // Retorna para a margem esquerda original
      y += 8;      // Move para a próxima linha de caracteres
    }
    // Para de desenhar se sair da parte inferior da tela
    if (y + 8 > ssd->height) {
      break;
    }
  }
}

// Desenha um caractere 16x16 (fonte 8x8 ampliada) no ram_buffer
void ssd1306_draw_char_large(ssd1306_t *ssd, char c, uint8_t x, uint8_t y) {
  // Trata caracteres fora da faixa (similar a draw_char)
  if (c < ' ' || c > '~') c = ' ';

  // Calcula índice na fonte (igual a draw_char)
  uint16_t index = 0;
  if (c >= 'A' && c <= 'Z') index = (c - 'A' + 11) * 8;
  else if (c >= '0' && c <= '9') index = (c - '0' + 1) * 8;
  else if (c >= 'a' && c <= 'z') index = (c - 'a' + 37) * 8;
  else if (c == ' ') index = 0;
  else index = 0;

  // Itera pela fonte 8x8
  for (uint8_t i = 0; i < 8; ++i) {
      uint8_t line = font[index + i];
      for (uint8_t j = 0; j < 8; ++j) {
          // Verifica se o pixel original (8x8) está ligado
          bool pixel_on = (line & (1 << j));
          // Desenha um bloco 2x2 para cada pixel original
          if (pixel_on) {
            ssd1306_pixel(ssd, x + (i * 2),     y + (j * 2),     true);
            ssd1306_pixel(ssd, x + (i * 2) + 1, y + (j * 2),     true);
            ssd1306_pixel(ssd, x + (i * 2),     y + (j * 2) + 1, true);
            ssd1306_pixel(ssd, x + (i * 2) + 1, y + (j * 2) + 1, true);
          }
          // (Opcional: poderia desenhar 4 pixels false se o fundo não for limpo)
      }
  }
}

// Desenha uma string (fonte 16x16 ampliada) no ram_buffer
void ssd1306_draw_string_large(ssd1306_t *ssd, const char *str, uint8_t x, uint8_t y) {
  uint8_t start_x = x;
  while (*str) {
      ssd1306_draw_char_large(ssd, *str++, x, y);
      x += 16; // Avança 16 pixels para o próximo caractere ampliado
      // Quebra de linha
      if (x + 16 > ssd->width) {
          x = start_x;
          y += 16; // Pula 16 pixels para a próxima linha
      }
      // Para se sair da tela
      if (y + 16 > ssd->height) {
          break;
      }
  }
}
