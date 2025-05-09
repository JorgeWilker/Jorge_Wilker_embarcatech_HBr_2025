#include "ssd1306_i2c.h"

// Fonte 5x8 para caracteres ASCII (básica)
static const uint8_t font5x8[] = {
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
    0x08, 0x2A, 0x1C, 0x2A, 0x08, // *
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
    0x00, 0x08, 0x14, 0x22, 0x41, // <
    0x14, 0x14, 0x14, 0x14, 0x14, // =
    0x41, 0x22, 0x14, 0x08, 0x00, // >
    0x02, 0x01, 0x51, 0x09, 0x06, // ?
    0x32, 0x49, 0x79, 0x41, 0x3E, // @
    0x7E, 0x11, 0x11, 0x11, 0x7E, // A
    0x7F, 0x49, 0x49, 0x49, 0x36, // B
    0x3E, 0x41, 0x41, 0x41, 0x22, // C
    0x7F, 0x41, 0x41, 0x22, 0x1C, // D
    0x7F, 0x49, 0x49, 0x49, 0x41, // E
    0x7F, 0x09, 0x09, 0x01, 0x01, // F
    0x3E, 0x41, 0x41, 0x49, 0x3A, // G
    0x7F, 0x08, 0x08, 0x08, 0x7F, // H
    0x00, 0x41, 0x7F, 0x41, 0x00, // I
    0x20, 0x40, 0x41, 0x3F, 0x01, // J
    0x7F, 0x08, 0x14, 0x22, 0x41, // K
    0x7F, 0x40, 0x40, 0x40, 0x40, // L
    0x7F, 0x02, 0x04, 0x02, 0x7F, // M
    0x7F, 0x04, 0x08, 0x10, 0x7F, // N
    0x3E, 0x41, 0x41, 0x41, 0x3E, // O
    0x7F, 0x09, 0x09, 0x09, 0x06, // P
    0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
    0x7F, 0x09, 0x19, 0x29, 0x46, // R
    0x46, 0x49, 0x49, 0x49, 0x31, // S
    0x01, 0x01, 0x7F, 0x01, 0x01, // T
    0x3F, 0x40, 0x40, 0x40, 0x3F, // U
    0x1F, 0x20, 0x40, 0x20, 0x1F, // V
    0x7F, 0x20, 0x18, 0x20, 0x7F, // W
    0x63, 0x14, 0x08, 0x14, 0x63, // X
    0x03, 0x04, 0x78, 0x04, 0x03, // Y
    0x61, 0x51, 0x49, 0x45, 0x43, // Z
    0x00, 0x00, 0x7F, 0x41, 0x41, // [
    0x02, 0x04, 0x08, 0x10, 0x20, // \
    0x41, 0x41, 0x7F, 0x00, 0x00, // ]
    0x04, 0x02, 0x01, 0x02, 0x04, // ^
    0x40, 0x40, 0x40, 0x40, 0x40, // _
    0x00, 0x01, 0x02, 0x04, 0x00, // `
    0x20, 0x54, 0x54, 0x54, 0x78, // a
    0x7F, 0x48, 0x44, 0x44, 0x38, // b
    0x38, 0x44, 0x44, 0x44, 0x20, // c
    0x38, 0x44, 0x44, 0x48, 0x7F, // d
    0x38, 0x54, 0x54, 0x54, 0x18, // e
    0x08, 0x7E, 0x09, 0x01, 0x02, // f
    0x08, 0x14, 0x54, 0x54, 0x3C, // g
    0x7F, 0x08, 0x04, 0x04, 0x78, // h
    0x00, 0x44, 0x7D, 0x40, 0x00, // i
    0x20, 0x40, 0x44, 0x3D, 0x00, // j
    0x00, 0x7F, 0x10, 0x28, 0x44, // k
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
    0x44, 0x64, 0x54, 0x4C, 0x44  // z
};

// Envia um comando para o display
static void ssd1306_command(ssd1306_t *oled, uint8_t command)
{
    uint8_t buf[2] = {0x00, command}; // 0x00 indica comando
    i2c_write_blocking(oled->i2c_port, oled->i2c_addr, buf, 2, false);
}

// Envia dados para o display
static void ssd1306_data(ssd1306_t *oled, const uint8_t *data, size_t len)
{
    // Buffer para enviar os dados (1 byte de controle + len bytes de dados)
    uint8_t *buf = malloc(len + 1);
    if (buf == NULL)
        return;

    buf[0] = 0x40; // 0x40 indica dados
    memcpy(buf + 1, data, len);

    i2c_write_blocking(oled->i2c_port, oled->i2c_addr, buf, len + 1, false);

    free(buf);
}

// Inicializa o display OLED
void ssd1306_init(ssd1306_t *oled, i2c_inst_t *i2c_port, uint8_t i2c_addr)
{
    oled->i2c_port = i2c_port;
    oled->i2c_addr = i2c_addr;

    // Inicialização do display
    sleep_ms(100); // Garante que o display tenha tempo para inicializar

    ssd1306_command(oled, SSD1306_DISPLAY_OFF);
    ssd1306_command(oled, SSD1306_SET_DISPLAY_CLOCK_DIV);
    ssd1306_command(oled, 0x80); // valor padrão
    ssd1306_command(oled, SSD1306_SET_MULTIPLEX);
    ssd1306_command(oled, OLED_HEIGHT - 1);
    ssd1306_command(oled, SSD1306_SET_DISPLAY_OFFSET);
    ssd1306_command(oled, 0x00);
    ssd1306_command(oled, SSD1306_SET_START_LINE | 0x00);
    ssd1306_command(oled, SSD1306_CHARGE_PUMP);
    ssd1306_command(oled, 0x14); // Habilita fonte de alimentação interna
    ssd1306_command(oled, SSD1306_MEMORY_MODE);
    ssd1306_command(oled, 0x00);                     // Modo horizontal
    ssd1306_command(oled, SSD1306_SEG_REMAP | 0x01); // Rotação horizontal
    ssd1306_command(oled, SSD1306_COM_SCAN_DEC);     // Rotação vertical
    ssd1306_command(oled, SSD1306_SET_COM_PINS);
    ssd1306_command(oled, 0x12); // Configuração de pinos para 128x64
    ssd1306_command(oled, SSD1306_SET_CONTRAST);
    ssd1306_command(oled, 0xCF); // Contraste máximo
    ssd1306_command(oled, SSD1306_SET_PRECHARGE);
    ssd1306_command(oled, 0xF1); // Valor pré-carga
    ssd1306_command(oled, SSD1306_SET_VCOM_DETECT);
    ssd1306_command(oled, 0x40); // Nível VCOM
    ssd1306_command(oled, SSD1306_DISPLAY_RAM);
    ssd1306_command(oled, SSD1306_DISPLAY_NORMAL);
    ssd1306_command(oled, SSD1306_DISPLAY_ON);

    // Limpa o buffer
    ssd1306_clear(oled);

    // Envia buffer limpo para o display
    ssd1306_display(oled);

    // Espera um pouco para estabilizar
    sleep_ms(100);
}

// Limpa o buffer do display
void ssd1306_clear(ssd1306_t *oled)
{
    memset(oled->buffer, 0, sizeof(oled->buffer));
}

// Envia o buffer para o display
void ssd1306_display(ssd1306_t *oled)
{
    // Configura área de endereço
    ssd1306_command(oled, SSD1306_COLUMN_ADDR);
    ssd1306_command(oled, 0);              // Coluna inicial
    ssd1306_command(oled, OLED_WIDTH - 1); // Coluna final

    ssd1306_command(oled, SSD1306_PAGE_ADDR);
    ssd1306_command(oled, 0);              // Página inicial
    ssd1306_command(oled, OLED_PAGES - 1); // Página final

    // Envia o buffer inteiro
    ssd1306_data(oled, oled->buffer, sizeof(oled->buffer));
}

// Define um pixel no buffer
void ssd1306_draw_pixel(ssd1306_t *oled, int x, int y, bool color)
{
    if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT)
    {
        return; // Fora dos limites
    }

    // Encontra o byte no buffer
    int page = y / 8;
    int bit = y % 8;
    int index = x + page * OLED_WIDTH;

    // Define ou limpa o bit
    if (color)
    {
        oled->buffer[index] |= (1 << bit);
    }
    else
    {
        oled->buffer[index] &= ~(1 << bit);
    }
}

// Desenha uma linha usando o algoritmo de Bresenham
void ssd1306_draw_line(ssd1306_t *oled, int x0, int y0, int x1, int y1, bool color)
{
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (true)
    {
        ssd1306_draw_pixel(oled, x0, y0, color);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2 * err;
        if (e2 >= dy)
        {
            if (x0 == x1)
                break;
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            if (y0 == y1)
                break;
            err += dx;
            y0 += sy;
        }
    }
}

// Desenha um retângulo
void ssd1306_draw_rect(ssd1306_t *oled, int x, int y, int w, int h, bool filled, bool color)
{
    if (filled)
    {
        // Retângulo preenchido
        for (int i = 0; i < w; i++)
        {
            for (int j = 0; j < h; j++)
            {
                ssd1306_draw_pixel(oled, x + i, y + j, color);
            }
        }
    }
    else
    {
        // Apenas a borda
        ssd1306_draw_line(oled, x, y, x + w - 1, y, color);                 // Topo
        ssd1306_draw_line(oled, x, y + h - 1, x + w - 1, y + h - 1, color); // Base
        ssd1306_draw_line(oled, x, y, x, y + h - 1, color);                 // Esquerda
        ssd1306_draw_line(oled, x + w - 1, y, x + w - 1, y + h - 1, color); // Direita
    }
}

// Desenha um círculo usando o algoritmo do ponto médio
void ssd1306_draw_circle(ssd1306_t *oled, int x0, int y0, int r, bool filled, bool color)
{
    if (filled)
    {
        // Círculo preenchido
        for (int y = -r; y <= r; y++)
        {
            for (int x = -r; x <= r; x++)
            {
                if (x * x + y * y <= r * r)
                {
                    ssd1306_draw_pixel(oled, x0 + x, y0 + y, color);
                }
            }
        }
    }
    else
    {
        // Apenas a borda
        int x = 0;
        int y = r;
        int p = 1 - r;

        ssd1306_draw_pixel(oled, x0, y0 + r, color);
        ssd1306_draw_pixel(oled, x0, y0 - r, color);
        ssd1306_draw_pixel(oled, x0 + r, y0, color);
        ssd1306_draw_pixel(oled, x0 - r, y0, color);

        while (x < y)
        {
            x++;
            if (p < 0)
            {
                p += 2 * x + 1;
            }
            else
            {
                y--;
                p += 2 * (x - y) + 1;
            }

            ssd1306_draw_pixel(oled, x0 + x, y0 + y, color);
            ssd1306_draw_pixel(oled, x0 - x, y0 + y, color);
            ssd1306_draw_pixel(oled, x0 + x, y0 - y, color);
            ssd1306_draw_pixel(oled, x0 - x, y0 - y, color);
            ssd1306_draw_pixel(oled, x0 + y, y0 + x, color);
            ssd1306_draw_pixel(oled, x0 - y, y0 + x, color);
            ssd1306_draw_pixel(oled, x0 + y, y0 - x, color);
            ssd1306_draw_pixel(oled, x0 - y, y0 - x, color);
        }
    }
}

// Desenha um caractere
void ssd1306_draw_char(ssd1306_t *oled, char c, int x, int y, bool color)
{
    // Verifica se o caractere está dentro do intervalo da fonte
    if (c < ' ' || c > 'z') // Suporte de ASCII 32 (espaço) até ASCII 122 (z)
    {
        c = '?'; // Caractere não suportado
    }

    // Ajusta o índice para a fonte
    c -= ' ';

    // Limpa a área do caractere primeiro para evitar resíduos
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            ssd1306_draw_pixel(oled, x + i, y + j, false);
        }
    }

    // Agora desenha o caractere
    for (int i = 0; i < 5; i++)
    {
        uint8_t line = font5x8[c * 5 + i];
        for (int j = 0; j < 8; j++)
        {
            if (line & (1 << j))
            {
                ssd1306_draw_pixel(oled, x + i, y + j, color);
            }
        }
    }
}

// Desenha uma string
void ssd1306_draw_string(ssd1306_t *oled, const char *str, int x, int y, bool color)
{
    int start_x = x;

    while (*str)
    {
        ssd1306_draw_char(oled, *str++, x, y, color);
        x += 6; // 5 pixels de largura + 1 pixel de espaçamento

        // Quebra de linha se necessário
        if (x > OLED_WIDTH - 6)
        {
            x = start_x; // Volta para o início da linha
            y += 8;      // Avança uma linha
            if (y > OLED_HEIGHT - 8)
            {
                break; // Fora da tela
            }
        }
    }
}