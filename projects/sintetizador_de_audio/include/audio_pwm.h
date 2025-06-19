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

// Sistema de captura e reprodução de áudio via PWM
// Implementa funcionalidades de gravação via ADC e reprodução via PWM
// com processamento digital para melhoria da qualidade sonora
// Autor: Jorge Wilker Mamede de Andrade - 2025

#ifndef AUDIO_PWM_H
#define AUDIO_PWM_H

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include <stdio.h>
#include <string.h>

// Configurações do sistema de áudio digital
#define SAMPLE_RATE 22050          // Taxa de amostragem otimizada para qualidade
#define AUDIO_BUFFER_SIZE 32768    // Tamanho do buffer (~1.5 segundos de áudio)
#define ADC_CHANNEL_MIC 2          // Canal ADC para microfone (GPIO 28)
#define PWM_GPIO_BUZZER 10         // GPIO para saída de áudio via buzzer
#define PWM_COUNT_MAX 1023         // Resolução de 10 bits para dinâmica melhorada
#define PWM_CLOCK_DIV 4.0f         // Divisor de clock para frequência PWM otimizada

// Parâmetros de processamento de sinal digital
#define AUDIO_GAIN 1               // Ganho unitário para preservar dinâmica original
#define DC_OFFSET_FILTER 0.99f     // Coeficiente do filtro passa-alta para remoção DC
#define NOISE_GATE_THRESHOLD 8     // Threshold para supressão de ruído de fundo
#define DYNAMIC_RANGE_COMPRESS 0.8f // Fator de compressão dinâmica

// Estados operacionais do sistema de áudio
#ifndef AUDIO_STATE_T_DEFINED
#define AUDIO_STATE_T_DEFINED
typedef enum {
    AUDIO_IDLE,      // Sistema em repouso, PWM em alta impedância
    AUDIO_RECORDING, // Captura ativa via ADC
    AUDIO_PLAYING    // Reprodução ativa via PWM
} audio_state_t;
#endif

// Estrutura de dados do sistema de áudio
typedef struct {
    uint8_t* buffer;        // Buffer principal de dados de áudio
    uint32_t size;          // Tamanho total do buffer alocado
    uint32_t current_pos;   // Posição atual no buffer (gravação/reprodução)
    audio_state_t state;    // Estado operacional atual do sistema
    bool recording_complete; // Flag de finalização de gravação
    bool playback_complete;  // Flag de finalização de reprodução
} audio_data_t;

// Inicializa o sistema de áudio PWM
void audio_init(void);

// Inicia processo de gravação de áudio
bool audio_start_recording(void);

// Finaliza processo de gravação
void audio_stop_recording(void);

// Verifica se o sistema está gravando
bool audio_is_recording(void);

// Inicia reprodução do áudio gravado
bool audio_start_playback(void);

// Finaliza processo de reprodução
void audio_stop_playback(void);

// Verifica se o sistema está reproduzindo
bool audio_is_playing(void);

// Limpa o buffer de áudio
void audio_clear_buffer(void);

// Obtém utilização atual do buffer
uint32_t audio_get_buffer_usage(void);

// Calcula tempo de gravação
float audio_get_recording_time(void);

// Extrai dados da forma de onda para visualização
void audio_get_waveform_data(uint8_t* display_buffer, uint8_t width, uint8_t height);

// Callback de timer (compatibilidade)
void audio_timer_callback(void);

// Processa sinal do microfone
int16_t process_microphone_signal(uint16_t adc_raw);

// Define pino PWM em alta impedância
void audio_set_pwm_high_impedance(void);

// Ativa configuração PWM do buzzer
void audio_set_pwm_active(void);

#endif // AUDIO_PWM_H 