/**
 * @file galton.c
 * @brief Implementação da simulação de Galton Board
 *
 * Fornece lógica para simular quedas de bolas e manter contagens de coletores.
 *
 * @author Jorge Wilker
 * @date Maio 2025
 */

#include "galton.h"
#include <time.h>

/** Variáveis estáticas para controle do estado da simulação */
static int bins[NUM_BINS] = {0};                         /**< Contador de bolas em cada coletor */
static int current_ball = 0;                             /**< Contador da bola atual */
static simulation_state_t current_state = STATE_WELCOME; /**< Estado inicial da simulação */
static uint32_t last_update_time = 0;                    /**< Timestamp da última atualização (ms) */

/** Posição da bola em movimento */
static ball_position_t ball_position = {
    .active = false,    /**< Nenhuma bola ativa inicialmente */
    .current_level = 0, /**< Nível 0 (topo) */
    .position = 0,      /**< Posição central */
    .direction = 0,     /**< Sem direção definida */
    .steps = 0          /**< Nenhum passo dado */
};

/**
 * @brief Inicializa a simulação do Galton Board
 *
 * Inicializa o gerador de números aleatórios e configura o estado inicial
 * da simulação chamando galton_reset().
 */
void galton_init(void)
{
    /* Inicializa o gerador de números aleatórios com a hora atual
     * Isso garante sequências diferentes a cada execução */
    srand(time(NULL));

    /* Reseta a simulação para o estado inicial */
    galton_reset();
}

/**
 * @brief Reinicia a simulação para o estado inicial
 *
 * Zera todos os contadores de bolas, limpa os coletores,
 * e retorna ao estado de boas-vindas.
 */
void galton_reset(void)
{
    /* Zera os contadores de bolas em todos os coletores */
    for (int i = 0; i < NUM_BINS; i++)
    {
        bins[i] = 0;
    }

    /* Reinicia o contador de bolas */
    current_ball = 0;

    /* Reseta a posição da bolinha ativa */
    ball_position.active = false;
    ball_position.current_level = 0;
    ball_position.position = 0;
    ball_position.direction = 0;
    ball_position.steps = 0;

    /* Retorna ao estado inicial */
    current_state = STATE_WELCOME;
}

/**
 * @brief Simula o caminho completo de uma bola pelo Galton Board
 *
 * Para cada nível do Galton Board, decide aleatoriamente se a bola
 * vai para a esquerda ou para a direita com 50% de probabilidade
 * para cada lado.
 *
 * @return Posição final da bola (índice do coletor)
 */
int galton_simulate_ball_path(void)
{
    int position = 0;

    /* Simula o caminho da bola através de cada nível */
    for (int level = 0; level < NUM_LEVELS; level++)
    {
        /* Gera 0 ou 1 com 50% de probabilidade (decisão aleatória) */
        int direction = rand() % 2;

        /* Atualiza a posição com base na direção:
         * - Se direction = 0, mantém a posição (vai para esquerda)
         * - Se direction = 1, incrementa a posição (vai para direita) */
        position += direction;
    }

    return position;
}

/**
 * @brief Obtém o valor máximo de bolas em um único coletor
 *
 * Essa função é útil para normalizar a exibição do histograma.
 *
 * @return Maior número de bolas em um único coletor
 */
int galton_get_max_bin_value(void)
{
    int max_value = 0;

    /* Procura o maior valor entre todos os coletores */
    for (int i = 0; i < NUM_BINS; i++)
    {
        if (bins[i] > max_value)
        {
            max_value = bins[i];
        }
    }

    return max_value;
}

/**
 * @brief Obtém acesso ao array de coletores
 *
 * @return Ponteiro para o array de contagem de bolas em cada coletor
 */
int *galton_get_bins(void)
{
    return bins;
}

/**
 * @brief Obtém o número total de coletores
 *
 * @return Número de coletores (NUM_BINS)
 */
int galton_get_num_bins(void)
{
    return NUM_BINS;
}

/**
 * @brief Obtém o estado atual da simulação
 *
 * @return Estado atual (welcome, running ou complete)
 */
simulation_state_t galton_get_state(void)
{
    return current_state;
}

/**
 * @brief Define o estado da simulação
 *
 * Atualiza o estado da simulação e inicializa variáveis relevantes
 * para o novo estado.
 *
 * @param state Novo estado para a simulação
 */
void galton_set_state(simulation_state_t state)
{
    current_state = state;

    /* Ao iniciar a simulação, reinicia o timestamp de atualização */
    if (state == STATE_RUNNING)
    {
        last_update_time = to_ms_since_boot(get_absolute_time());
    }
}

/**
 * @brief Obtém o número da bola atual
 *
 * @return Índice da bola atual (0 a NUM_BALLS-1)
 */
int galton_get_current_ball(void)
{
    return current_ball;
}

/**
 * @brief Obtém o número total de bolas na simulação
 *
 * @return Número total de bolas configurado (NUM_BALLS)
 */
int galton_get_total_balls(void)
{
    return NUM_BALLS;
}

/**
 * @brief Obtém a posição atual da bolinha ativa
 *
 * @return Ponteiro para a estrutura com informações da bolinha em queda
 */
const ball_position_t *galton_get_ball_position(void)
{
    return &ball_position;
}

/**
 * @brief Atualiza o estado da simulação
 *
 * Função principal chamada periodicamente para atualizar o estado
 * da simulação, fazendo as bolas caírem através do Galton Board
 * e atualizando os contadores.
 */
void galton_update(void)
{
    /* Só processa atualizações no estado RUNNING */
    if (current_state != STATE_RUNNING)
    {
        return;
    }

    /* Controle de timing para regulagem da velocidade da animação */
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - last_update_time < DELAY_MS / 3) // Otimização para animação mais suave
    {
        return;
    }

    /* Atualiza o timestamp da última atualização */
    last_update_time = current_time;

    /* Verifica se a simulação chegou ao fim (todas as bolas processadas) */
    if (current_ball >= NUM_BALLS)
    {
        /* Muda para o estado de conclusão */
        current_state = STATE_COMPLETE;
        return;
    }

    /* Se não há bolinha ativa, inicia uma nova */
    if (!ball_position.active)
    {
        /* Configura nova bolinha no topo do Galton Board */
        ball_position.active = true;
        ball_position.current_level = 0;
        ball_position.position = 0;
        ball_position.direction = 0;
        ball_position.steps = 0;
        return;
    }

    /* Atualiza a posição da bolinha ativa */
    ball_position.steps++;

    /* Verifica se a bolinha completou o movimento entre níveis */
    if (ball_position.steps >= 3) // Cada nível requer 3 passos para animação suave
    {
        /* Reinicia contador de passos */
        ball_position.steps = 0;

        /* Verifica se a bolinha chegou ao final do Galton Board */
        if (ball_position.current_level >= NUM_LEVELS)
        {
            /* Incrementa o contador do bin correspondente à posição final */
            bins[ball_position.position]++;

            /* Incrementa contador de bolas processadas */
            current_ball++;

            /* Desativa a bolinha atual para que uma nova seja criada */
            ball_position.active = false;
            return;
        }

        /* Escolhe aleatoriamente a direção para o próximo nível (50% para cada lado) */
        ball_position.direction = rand() % 2;

        /* Atualiza a posição horizontal baseada na direção escolhida
         * - Se direction = 1 (direita), incrementa posição
         * - Se direction = 0 (esquerda), mantém posição */
        if (ball_position.direction == 1)
        {
            ball_position.position++;
        }

        /* Avança para o próximo nível */
        ball_position.current_level++;
    }
}