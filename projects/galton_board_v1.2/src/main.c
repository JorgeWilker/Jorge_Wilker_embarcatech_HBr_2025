/**
 * @file main.c
 * @brief Aplicação principal para simulação de Galton Board com display OLED
 *
 * Implementa simulação visual usando display SSD1306 e botões para controle.
 *
 * @author Jorge Wilker
 * @date Maio 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"

/* Inclusão dos módulos do projeto */
#include "../include/ssd1306_i2c.h"
#include "../include/galton.h"

/**
 * @brief Definição dos pinos GPIO para os botões
 */
const uint BUTTON_PIN_A = 5; /**< Pino GPIO para o botão A (iniciar) */
const uint BUTTON_PIN_B = 6; /**< Pino GPIO para o botão B (reiniciar) */

/**
 * @brief Variáveis para controle e debounce dos botões
 */
static absolute_time_t last_press_time_a = {0}; /**< Timestamp da última pressão do botão A */
static absolute_time_t last_press_time_b = {0}; /**< Timestamp da última pressão do botão B */
static bool button_last_state_a = false;        /**< Estado anterior do botão A */
static bool button_last_state_b = false;        /**< Estado anterior do botão B */
static bool button_a_was_pressed = false;       /**< Flag indicando evento de pressão do botão A */
static bool button_b_was_pressed = false;       /**< Flag indicando evento de pressão do botão B */

/**
 * @brief Array local para armazenar contadores de bins
 * @note Este array é um cache local que reflete o estado dos bins do módulo galton
 */
int bins[NUM_BINS] = {0};

/**
 * @brief Configurações do hardware para comunicação I2C com o display OLED
 */
#define I2C_PORT i2c1         /**< Instância I2C utilizada */
#define I2C_SDA_PIN 14        /**< Pino GPIO para dados I2C (SDA) */
#define I2C_SCL_PIN 15        /**< Pino GPIO para clock I2C (SCL) */
#define SSD1306_I2C_ADDR 0x3C /**< Endereço I2C do display OLED SSD1306 */

/**
 * @brief Instância global do display OLED
 */
ssd1306_t display;

/**
 * @brief Inicializa os botões de controle
 *
 * Configura os pinos GPIO dos botões como entradas com pull-up
 * e inicializa as variáveis de estado.
 */
void buttons_init(void)
{
    /* Configura os pinos dos botões como entrada com resistor de pull-up interno */
    gpio_init(BUTTON_PIN_A);
    gpio_set_dir(BUTTON_PIN_A, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_A); /* Habilita o resistor pull-up interno */

    gpio_init(BUTTON_PIN_B);
    gpio_set_dir(BUTTON_PIN_B, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_B); /* Habilita o resistor pull-up interno */

    printf("Botões inicializados (A: GPIO%d, B: GPIO%d).\n", BUTTON_PIN_A, BUTTON_PIN_B);
}

/**
 * @brief Atualiza o estado dos botões
 *
 * Lê o estado atual dos botões, implementa debounce para evitar leituras
 * falsas, e atualiza os flags de evento de pressão.
 *
 * @note Deve ser chamada periodicamente no loop principal
 */
void buttons_update(void)
{
    /* Verifica o estado atual dos botões (pressionado = LOW, liberado = HIGH) */
    bool button_pressed_a = !gpio_get(BUTTON_PIN_A); /* Botão A pressionado = nível baixo (0) */
    bool button_pressed_b = !gpio_get(BUTTON_PIN_B); /* Botão B pressionado = nível baixo (0) */

    /* Reseta os flags de evento de pressão */
    button_a_was_pressed = false;
    button_b_was_pressed = false;

    /* Verifica se o botão A foi pressionado e realiza o debounce */
    if (button_pressed_a && !button_last_state_a &&
        absolute_time_diff_us(last_press_time_a, get_absolute_time()) > 200000)
    {                                            /* 200 ms de debounce */
        last_press_time_a = get_absolute_time(); /* Atualiza o tempo da última pressão */
        button_last_state_a = true;              /* Atualiza o estado do botão */
        button_a_was_pressed = true;             /* Indica que o botão foi pressionado */
    }
    else if (!button_pressed_a)
    {
        button_last_state_a = false; /* Atualiza o estado do botão como liberado */
    }

    /* Verifica se o botão B foi pressionado e realiza o debounce */
    if (button_pressed_b && !button_last_state_b &&
        absolute_time_diff_us(last_press_time_b, get_absolute_time()) > 200000)
    {                                            /* 200 ms de debounce */
        last_press_time_b = get_absolute_time(); /* Atualiza o tempo da última pressão */
        button_last_state_b = true;              /* Atualiza o estado do botão */
        button_b_was_pressed = true;             /* Indica que o botão foi pressionado */
    }
    else if (!button_pressed_b)
    {
        button_last_state_b = false; /* Atualiza o estado do botão como liberado */
    }
}

/**
 * @brief Verifica se o botão A foi pressionado
 *
 * @return true se o botão A foi pressionado desde a última chamada a buttons_update()
 */
bool button_a_pressed(void)
{
    return button_a_was_pressed;
}

/**
 * @brief Verifica se o botão B foi pressionado
 *
 * @return true se o botão B foi pressionado desde a última chamada a buttons_update()
 */
bool button_b_pressed(void)
{
    return button_b_was_pressed;
}

/**
 * @brief Inicializa o display OLED
 *
 * Configura a comunicação I2C e inicializa o display OLED SSD1306.
 */
void display_init(void)
{
    printf("Inicializando display OLED...\n");

    /* Inicializa I2C para comunicação com o display */
    i2c_init(I2C_PORT, 400 * 1000); /* 400 kHz (Fast Mode) */
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    /* Inicializa o display OLED */
    ssd1306_init(&display, I2C_PORT, SSD1306_I2C_ADDR);
    ssd1306_clear(&display);
    ssd1306_display(&display);

    printf("Display OLED inicializado (Addr: 0x%X).\n", SSD1306_I2C_ADDR);
}

/**
 * @brief Desenha texto na posição especificada
 *
 * Função auxiliar para simplificar a exibição de texto no display.
 *
 * @param text String a ser desenhada (terminada em nulo)
 * @param x Coordenada X do texto
 * @param y Coordenada Y do texto
 */
void display_draw_text(const char *text, int x, int y)
{
    ssd1306_draw_string(&display, text, x, y, true);
}

/**
 * @brief Desenha a estrutura do Galton Board no display
 *
 * Renderiza os pinos, canaletas e bolas do Galton Board, bem como
 * a bolinha em movimento quando aplicável.
 *
 * @param num_levels Número de níveis do Galton Board
 */
void display_draw_galton_board(int num_levels)
{
    /* Configurações de posicionamento do Galton Board */
    int start_x = 42; /* Posição central em X */
    int start_y = 12; /* Margem superior */
    int spacing = 6;  /* Espaçamento entre pinos */

    /* Desenha os pinos de cada nível em formato triangular */
    for (int level = 0; level < num_levels; level++)
    {
        int y = start_y + level * spacing;
        int pins_in_level = level + 1;
        int level_width = pins_in_level * spacing;
        int level_start_x = start_x - level_width / 2;

        for (int pin = 0; pin < pins_in_level; pin++)
        {
            int x = level_start_x + pin * spacing;
            ssd1306_draw_circle(&display, x, y, 1, true, true);
        }
    }

    /* Desenha as canaletas (bins) abaixo dos pinos */
    int canaleta_y = start_y + num_levels * spacing;
    int num_canaletas = num_levels + 1;
    int canaleta_width = spacing - 1;
    int canaletas_start_x = start_x - (num_canaletas * spacing) / 2 + 1;

    /* Obtém contagem atual de bolas nos bins para visualização */
    int *bins = galton_get_bins();

    /* Desenha cada canaleta com indicação visual se contiver bolas */
    for (int i = 0; i < num_canaletas; i++)
    {
        int x = canaletas_start_x + i * spacing;

        /* Desenha a canaleta como um retângulo vertical */
        ssd1306_draw_rect(&display, x, canaleta_y, canaleta_width, 8, true, false);

        /* Desenha linhas verticais para os lados da canaleta */
        ssd1306_draw_line(&display, x, canaleta_y, x, canaleta_y + 8, true);
        ssd1306_draw_line(&display, x + canaleta_width - 1, canaleta_y, x + canaleta_width - 1, canaleta_y + 8, true);

        /* Adiciona uma bolinha na canaleta se houver bolas neste bin */
        if (bins[i] > 0)
        {
            int ball_x = x + canaleta_width / 2;
            int ball_y = canaleta_y + 4; /* Centraliza na canaleta */
            ssd1306_draw_circle(&display, ball_x, ball_y, 1, true, true);
        }
    }

    /* Desenha a bolinha em queda, se estiver no estado de execução */
    if (galton_get_state() == STATE_RUNNING)
    {
        const ball_position_t *ball = galton_get_ball_position();

        if (ball->active)
        {
            /* Obtém informações de posição da bolinha */
            int level = ball->current_level;
            int position = ball->position;
            int steps = ball->steps;

            /* Calcula a posição y inicial do nível */
            int level_y = start_y + (level * spacing);

            /* Calcula posição y interpolada entre níveis baseada nos passos */
            int y = level_y - spacing + (steps * spacing / 3);

            /* Calcula quantidade de pinos no nível atual */
            int pins_in_level = level + 1;
            int level_width = pins_in_level * spacing;
            int level_start_x = start_x - level_width / 2;

            /* Calcula a posição x baseada na posição dentro do nível */
            int x = level_start_x + (position * spacing);

            /* Desenha a bolinha em movimento com tamanho maior para destaque */
            ssd1306_draw_circle(&display, x, y, 2, true, true);
        }
    }
}

/**
 * @brief Desenha o histograma com as contagens de bolas
 *
 * Cria uma visualização gráfica da distribuição das bolas nos bins,
 * com valores numéricos e barras proporcionais.
 *
 * @param bins Array com contagem de bolas em cada bin
 * @param num_bins Número de bins a serem exibidos
 * @param max_balls Valor máximo de bolas em um único bin (para normalização)
 */
void display_draw_bins(int bins[], int num_bins, int max_balls)
{
    /* Dimensões e posição do histograma no display */
    int hist_width = 30;  /* Largura da área do histograma */
    int hist_height = 35; /* Altura da área do histograma */
    int hist_x = 94;      /* Posição X inicial (otimizado para layout) */
    int hist_y = 15;      /* Posição Y inicial */
    int bar_gap = 2;      /* Espaçamento entre barras */
    int bar_width = 2;    /* Largura de cada barra */

    /* Calcula a largura total necessária para todas as barras */
    int total_width = (bar_width * num_bins) + (bar_gap * (num_bins - 1));

    /* Centraliza o histograma dentro do espaço disponível */
    hist_x = 128 - total_width - 4; /* 4 pixels de margem à direita */

    /* Desenha o quadro do histograma */
    ssd1306_draw_rect(&display, hist_x - 2, hist_y, total_width + 4, hist_height, true, false);

    /* Desenha uma linha horizontal na base */
    ssd1306_draw_line(&display, hist_x - 2, hist_y + hist_height - 1,
                      hist_x + total_width + 1, hist_y + hist_height - 1, true);

    /* Desenha as barras do histograma */
    for (int bin = 0; bin < num_bins; bin++)
    {
        int value = bins[bin];

        /* Calcula altura proporcional para a barra (otimizado para display OLED) */
        int pixels_per_ball = 1;              /* Fator de escala: 1 pixel por bola */
        int max_bar_height = hist_height - 5; /* Altura máxima para visualização */

        int bar_height = value * pixels_per_ball;

        /* Limita a altura máxima para caber no display */
        if (bar_height > max_bar_height)
            bar_height = max_bar_height;

        /* Posição X de cada barra */
        int bar_x = hist_x + (bin * (bar_width + bar_gap));

        /* Desenha a barra (de baixo para cima) se houver valor */
        if (bar_height > 0)
        {
            ssd1306_draw_rect(&display, bar_x, hist_y + hist_height - 2 - bar_height,
                              bar_width, bar_height, true, true);
        }

        /* Adiciona o valor numérico do bin como legenda */
        if (value > 0)
        {
            char num_str[4];
            snprintf(num_str, sizeof(num_str), "%d", value);

            /* Alterna posição vertical para evitar sobreposição de números */
            int text_y;
            if (bin % 2 == 0)
            {
                text_y = hist_y + hist_height + 1; /* Logo abaixo do histograma */
            }
            else
            {
                text_y = hist_y + hist_height + 8; /* Linha inferior para números alternados */
            }

            /* Centra número em relação à barra */
            display_draw_text(num_str, bar_x - 1, text_y);
        }
    }

    /* Adiciona título ao histograma */
    display_draw_text("HISTOGRAMA", hist_x + 2, hist_y - 9);
}

/**
 * @brief Exibe a tela de boas-vindas
 *
 * Apresenta a tela inicial com o título e instruções para o usuário.
 */
void display_show_welcome_screen(void)
{
    ssd1306_clear(&display);
    display_draw_text("GALTON BOARD", 20, 5);
    display_draw_text("PRESSIONE A", 20, 25);
    display_draw_text("PARA INICIAR", 20, 35);
    ssd1306_display(&display);
}

/**
 * @brief Exibe estatísticas da simulação
 *
 * Mostra o contador de bolas atual e total na parte superior da tela.
 *
 * @param current_ball Número da bola atual
 * @param total_balls Número total de bolas na simulação
 */
void display_show_stats(int current_ball, int total_balls)
{
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "BOLAS: %d/%d", current_ball, total_balls);
    ssd1306_draw_rect(&display, 0, 0, 128, 10, true, false); /* Limpa área de status */
    display_draw_text(buffer, 2, 1);                         /* Exibe contador na parte superior */
}

/**
 * @brief Exibe mensagem de conclusão da simulação
 *
 * Sobrepõe uma mensagem indicando o fim da simulação e instruções
 * para reiniciar.
 */
void display_show_simulation_complete(void)
{
    /* Cria área semitransparente para texto de conclusão */
    ssd1306_draw_rect(&display, 20, 20, 88, 25, true, false);

    /* Exibe mensagens de conclusão e instruções */
    display_draw_text("SIMULACAO", 32, 22);
    display_draw_text("COMPLETA!", 32, 32);
    display_draw_text(" B P/ LIMPAR", 22, 55);
    ssd1306_display(&display);
}

/**
 * @brief Função principal
 *
 * Inicializa hardware, configura periféricos, e implementa o
 * loop principal com a máquina de estados da simulação.
 *
 * @return int Código de retorno (não utilizado no ambiente bare-metal)
 */
int main()
{
    /* Inicializa stdio para saída serial */
    stdio_init_all();

    /* Aguarda estabilização do sistema */
    sleep_ms(2000);

    printf("INICIALIZANDO SIMULACAO DO GALTON BOARD...\n");

    /* Inicializa componentes do sistema */
    buttons_init();
    display_init();
    galton_init();

    /* Mostra a tela de boas-vindas */
    display_show_welcome_screen();

    /* Loop principal */
    while (true)
    {
        /* Atualiza os estados dos botões */
        buttons_update();

        /* Máquina de estados da simulação */
        switch (galton_get_state())
        {
        case STATE_WELCOME:
            /* Estado de boas-vindas: aguarda pressionar botão A para iniciar */
            if (button_a_pressed())
            {
                printf("Iniciando simulação...\n");
                galton_set_state(STATE_RUNNING);
            }
            break;

        case STATE_RUNNING:
            /* Estado de execução: processa a simulação e atualiza o display */
            galton_update();

            /* Renderiza a interface completa a cada ciclo */
            ssd1306_clear(&display);
            display_show_stats(galton_get_current_ball(), galton_get_total_balls());
            display_draw_galton_board(NUM_LEVELS);
            display_draw_bins(galton_get_bins(), galton_get_num_bins(), galton_get_max_bin_value());
            ssd1306_display(&display);

            /* Botão B reseta a simulação mesmo durante a execução */
            if (button_b_pressed())
            {
                printf("Reiniciando simulação...\n");
                galton_reset();
                display_show_welcome_screen();
            }
            break;

        case STATE_COMPLETE:
            /* Estado de conclusão: exibe resultados finais */
            ssd1306_clear(&display); /* Limpa o display para atualização completa */
            display_show_stats(galton_get_current_ball(), galton_get_total_balls());
            display_draw_galton_board(NUM_LEVELS);
            display_draw_bins(galton_get_bins(), galton_get_num_bins(), galton_get_max_bin_value());
            display_show_simulation_complete();
            ssd1306_display(&display); /* Atualiza o display explicitamente */

            /* Botão B reinicia a simulação */
            if (button_b_pressed())
            {
                printf("Reiniciando simulação...\n");
                galton_reset();
                display_show_welcome_screen();
            }
            break;
        }

        /* Pequena pausa para economia de recursos */
        sleep_ms(10);
    }

    return 0;
}