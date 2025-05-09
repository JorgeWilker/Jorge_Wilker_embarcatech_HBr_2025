/**
 * @file galton.h
 * @brief API para simulação de Galton Board
 *
 * Define estruturas, constantes e funções para simulação em microcontrolador.
 *
 * @author Jorge Wilker
 * @date Maio 2025
 */

#ifndef GALTON_H
#define GALTON_H

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"

/**
 * @brief Configurações da simulação
 */
#define NUM_LEVELS 7              /**< Número de níveis no Galton Board (otimizado para display) */
#define NUM_BINS (NUM_LEVELS + 1) /**< Número de coletores (bins) na base, sempre é NUM_LEVELS + 1 */
#define NUM_BALLS 75              /**< Quantidade total de bolas na simulação */
#define DELAY_MS 20               /**< Intervalo entre atualizações visuais (ms) */

/**
 * @brief Estados possíveis da simulação
 */
typedef enum
{
    STATE_WELCOME, /**< Tela de boas-vindas, aguardando início */
    STATE_RUNNING, /**< Simulação em execução, bolas caindo */
    STATE_COMPLETE /**< Simulação concluída, exibindo resultados */
} simulation_state_t;

/**
 * @brief Estrutura para acompanhar a posição da bolinha durante a queda
 */
typedef struct
{
    bool active;       /**< Indica se há uma bolinha ativa caindo no momento */
    int current_level; /**< Nível atual (0 a NUM_LEVELS) */
    int position;      /**< Posição horizontal no nível atual */
    int direction;     /**< Última direção de movimento (0 = esquerda, 1 = direita) */
    int steps;         /**< Passos entre níveis (para animação) */
} ball_position_t;

/**
 * @brief Inicializa o módulo de simulação Galton Board
 *
 * Configura o gerador de números aleatórios e inicializa
 * o estado da simulação para valores padrão.
 */
void galton_init(void);

/**
 * @brief Reinicia a simulação para o estado inicial
 *
 * Zera todos os contadores de bolas, limpa os coletores e
 * retorna ao estado de boas-vindas.
 */
void galton_reset(void);

/**
 * @brief Simula o caminho completo de uma bola através do Galton Board
 *
 * Determina aleatoriamente o caminho da bola através de todos os níveis
 * usando probabilidade de 50% para cada direção.
 *
 * @return Posição final da bola (índice do coletor)
 */
int galton_simulate_ball_path(void);

/**
 * @brief Obtém o valor máximo nos bins
 *
 * Útil para normalizar a exibição do histograma.
 *
 * @return Maior quantidade de bolas em um único coletor
 */
int galton_get_max_bin_value(void);

/**
 * @brief Obtém o array dos coletores
 *
 * @return Ponteiro para o array de contagem de bolas em cada coletor
 */
int *galton_get_bins(void);

/**
 * @brief Obtém o número de coletores
 *
 * @return Número total de coletores (NUM_BINS)
 */
int galton_get_num_bins(void);

/**
 * @brief Atualiza o estado da simulação
 *
 * Função principal que deve ser chamada periodicamente para processar
 * a queda das bolas, atualizar contadores e gerenciar estados.
 */
void galton_update(void);

/**
 * @brief Obtém o estado atual da simulação
 *
 * @return Estado atual (welcome, running ou complete)
 */
simulation_state_t galton_get_state(void);

/**
 * @brief Define o estado da simulação
 *
 * @param state Novo estado para a simulação
 */
void galton_set_state(simulation_state_t state);

/**
 * @brief Obtém o número da bola atual
 *
 * @return Contador da bola atual (0 a NUM_BALLS-1)
 */
int galton_get_current_ball(void);

/**
 * @brief Obtém o número total de bolas
 *
 * @return Número total de bolas na simulação (NUM_BALLS)
 */
int galton_get_total_balls(void);

/**
 * @brief Obtém a posição atual da bolinha ativa
 *
 * @return Ponteiro para a estrutura com informações da bolinha em queda
 */
const ball_position_t *galton_get_ball_position(void);

#endif // GALTON_H