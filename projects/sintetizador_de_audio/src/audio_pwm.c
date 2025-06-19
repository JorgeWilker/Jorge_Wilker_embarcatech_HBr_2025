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

#include "audio_pwm.h"

// Variáveis globais - Buffer de áudio melhorado
static audio_data_t audio_system;
static uint8_t audio_buffer[AUDIO_BUFFER_SIZE];
static uint pwm_slice;
static repeating_timer_t recording_timer;
static repeating_timer_t playback_timer;
static uint32_t playback_position = 0;

// Variáveis para processamento de áudio
static float dc_filter_state = 0.0f;
static int16_t last_sample = 0;
static uint32_t max_amplitude = 0;

// Variáveis adicionais para filtro anti-chiado do microfone
static int16_t mic_history[8] = {0};  // Histórico para filtro de média móvel
static uint8_t mic_history_index = 0;
static float low_pass_state = 0.0f;   // Estado do filtro passa-baixa
static int16_t prev_filtered = 0;     // Amostra anterior filtrada

// Funções para controle de ruído digital (alta impedância)
void audio_set_pwm_high_impedance(void) {
    // Colocar o pino do buzzer em alta impedância para eliminar ruído digital
    gpio_set_function(PWM_GPIO_BUZZER, GPIO_FUNC_SIO);
    gpio_set_dir(PWM_GPIO_BUZZER, GPIO_IN);  // Alta impedância
    printf("PWM buzzer definido para alta impedância (sem ruído digital)\n");
}

void audio_set_pwm_active(void) {
    // Reconfigurar o pino do buzzer para PWM antes da reprodução
    gpio_set_function(PWM_GPIO_BUZZER, GPIO_FUNC_PWM);
    
    // Reconfigurar PWM (caso necessário após mudança de função)
    pwm_slice = pwm_gpio_to_slice_num(PWM_GPIO_BUZZER);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, PWM_CLOCK_DIV);
    pwm_config_set_wrap(&config, PWM_COUNT_MAX);  // 1023 para 10 bits
    pwm_init(pwm_slice, &config, true);
    
    // Definir nível DC neutro (meio da escala de 10 bits)
    pwm_set_gpio_level(PWM_GPIO_BUZZER, PWM_COUNT_MAX / 2);
    printf("PWM buzzer reativado para reprodução\n");
}

// Filtro passa-alta para remover DC offset
int16_t apply_dc_filter(int16_t input) {
    // Filtro passa-alta de primeira ordem
    dc_filter_state = DC_OFFSET_FILTER * dc_filter_state + (1.0f - DC_OFFSET_FILTER) * input;
    return (int16_t)(input - dc_filter_state);
}

// Compressor dinâmico para melhorar o range
int16_t apply_dynamic_compression(int16_t input) {
    int16_t abs_input = input < 0 ? -input : input;
    
    if (abs_input > 200) {  // Threshold para compressão
        float ratio = DYNAMIC_RANGE_COMPRESS;
        int16_t compressed = (int16_t)(input * ratio);
        return compressed;
    }
    return input;
}

// Noise gate para reduzir ruído de fundo
int16_t apply_noise_gate(int16_t input) {
    int16_t abs_input = input < 0 ? -input : input;
    
    if (abs_input < NOISE_GATE_THRESHOLD) {
        return 0;  // Silenciar ruído baixo
    }
    return input;
}

// Função completamente bruta - apenas conversão ADC
int16_t process_microphone_signal(uint16_t adc_raw) {
    // Apenas converter para signed 16 bits centralizados - NADA MAIS
    return (int16_t)(adc_raw - 2048);
}

// Callback para gravação via timer melhorado
bool recording_timer_callback(repeating_timer_t *rt) {
    if (audio_system.state == AUDIO_RECORDING && audio_system.current_pos < AUDIO_BUFFER_SIZE) {
        // Ler amostra do ADC (12 bits)
        uint16_t adc_raw = adc_read();
        
        // Processar sinal do microfone
        int16_t processed_sample = process_microphone_signal(adc_raw);
        
        // Amplificar o sinal
        int32_t amplified = (int32_t)processed_sample * AUDIO_GAIN;
        
        // Aplicar noise gate
        if (amplified > -NOISE_GATE_THRESHOLD && amplified < NOISE_GATE_THRESHOLD) {
            amplified = 0;
        }
        
        // Aplicar compressão dinâmica
        if (amplified > 800) amplified = 800 + (amplified - 800) * DYNAMIC_RANGE_COMPRESS;
        if (amplified < -800) amplified = -800 + (amplified + 800) * DYNAMIC_RANGE_COMPRESS;
        
        // Limitar para evitar clipping
        if (amplified > 511) amplified = 511;
        if (amplified < -512) amplified = -512;
        
        // Converter para unsigned 8 bits com melhor resolução
        uint8_t final_sample = (uint8_t)((amplified + 512) >> 2);  // Escalar para 0-255
        
        // Rastrear amplitude máxima para normalização posterior
        uint16_t abs_amp = amplified < 0 ? -amplified : amplified;
        if (abs_amp > max_amplitude && abs_amp < 500) {
            max_amplitude = abs_amp;
        }
        
        // Armazenar no buffer
        audio_buffer[audio_system.current_pos] = final_sample;
        audio_system.current_pos++;
        
        return true;  // Continuar timer
    } else {
        // Parar gravação quando buffer cheio
        if (audio_system.current_pos >= AUDIO_BUFFER_SIZE) {
            printf("Amplitude máxima gravada: %d\n", max_amplitude);
            audio_stop_recording();
        }
        return false;  // Parar timer
    }
}

// Callback para reprodução via timer melhorado
bool playback_timer_callback(repeating_timer_t *rt) {
    if (audio_system.state == AUDIO_PLAYING && playback_position < audio_system.current_pos) {
        // Ler sample do buffer
        uint8_t sample = audio_buffer[playback_position];
        
        // Expandir para 10 bits com interpolação suave
        uint16_t sample_10bit = (uint16_t)sample << 2;  // 8 bits -> 10 bits
        
        // Aplicar suavização entre amostras para reduzir chiado
        static uint16_t last_pwm_value = 512;
        uint16_t smooth_sample = (last_pwm_value + sample_10bit) >> 1;
        last_pwm_value = sample_10bit;
        
        // Enviar para PWM com maior resolução
        pwm_set_gpio_level(PWM_GPIO_BUZZER, smooth_sample);
        
        playback_position++;
        return true;  // Continuar timer
    } else {
        // Finalizar reprodução
        pwm_set_gpio_level(PWM_GPIO_BUZZER, 512);  // Nível DC neutro para 10 bits
        audio_system.state = AUDIO_IDLE;
        audio_system.playback_complete = true;
        printf("Reprodução finalizada\n");
        
        // Colocar pino em alta impedância para eliminar ruído digital
        audio_set_pwm_high_impedance();
        
        return false;  // Parar timer
    }
}

void audio_init(void) {
    printf("Inicializando sistema de áudio melhorado...\n");
    
    // Inicializar estrutura do sistema de áudio
    audio_system.buffer = audio_buffer;
    audio_system.size = AUDIO_BUFFER_SIZE;
    audio_system.current_pos = 0;
    audio_system.state = AUDIO_IDLE;
    audio_system.recording_complete = false;
    audio_system.playback_complete = false;
    
    // Resetar variáveis de processamento
    dc_filter_state = 0.0f;
    last_sample = 0;
    max_amplitude = 0;
    
    // Limpar buffer com valor neutro melhorado
    memset(audio_buffer, 128, sizeof(audio_buffer));
    
    // Configurar ADC para microfone (GPIO 28) com melhor precisão
    adc_init();
    adc_gpio_init(28);  // GPIO 28 - ADC2 (microfone BitDogLab)
    adc_select_input(ADC_CHANNEL_MIC);
    
    // Fazer algumas leituras iniciais para estabilizar o ADC (reduzido)
    for (int i = 0; i < 10; i++) {
        adc_read();
        sleep_us(10);
    }
    
    // Configurar PWM para buzzer com resolução melhorada
    gpio_set_function(PWM_GPIO_BUZZER, GPIO_FUNC_PWM);
    pwm_slice = pwm_gpio_to_slice_num(PWM_GPIO_BUZZER);
    
    // Configurar PWM: 10 bits de resolução com frequência otimizada
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, PWM_CLOCK_DIV);
    pwm_config_set_wrap(&config, PWM_COUNT_MAX);  // 1023 para 10 bits
    pwm_init(pwm_slice, &config, true);
    
    // Definir nível DC neutro (meio da escala de 10 bits)
    pwm_set_gpio_level(PWM_GPIO_BUZZER, PWM_COUNT_MAX / 2);
    
    printf("Sistema de áudio inicializado:\n");
    printf("- Taxa: %dHz\n", SAMPLE_RATE);
    printf("- Buffer: %d bytes (~%.1f segundos)\n", 
           AUDIO_BUFFER_SIZE, (float)AUDIO_BUFFER_SIZE / SAMPLE_RATE);
    printf("- Resolução PWM: %d bits\n", 10);
    printf("- Ganho: %dx\n", AUDIO_GAIN);
    
    // Colocar pino em alta impedância para eliminar ruído digital quando idle
    audio_set_pwm_high_impedance();
}

bool audio_start_recording(void) {
    if (audio_system.state != AUDIO_IDLE) {
        printf("Erro: Sistema não está idle (estado=%d)\n", audio_system.state);
        return false;
    }
    
    // Limpar buffer antes de gravar
    memset(audio_buffer, 128, sizeof(audio_buffer));
    audio_system.current_pos = 0;
    audio_system.state = AUDIO_RECORDING;
    audio_system.recording_complete = false;
    
    printf("Iniciando gravação direta na RAM...\n");
    
    // Iniciar timer de gravação
    int64_t period_us = 1000000 / SAMPLE_RATE;  // Período em microssegundos
    if (!add_repeating_timer_us(-period_us, recording_timer_callback, NULL, &recording_timer)) {
        printf("Erro ao iniciar timer de gravação\n");
        audio_system.state = AUDIO_IDLE;
        return false;
    }
    
    return true;
}

void audio_stop_recording(void) {
    if (audio_system.state == AUDIO_RECORDING) {
        cancel_repeating_timer(&recording_timer);
        audio_system.state = AUDIO_IDLE;
        audio_system.recording_complete = true;
        printf("Gravação finalizada - %d amostras em RAM\n", audio_system.current_pos);
    }
}

bool audio_is_recording(void) {
    return audio_system.state == AUDIO_RECORDING;
}

bool audio_start_playback(void) {
    if (audio_system.state != AUDIO_IDLE) {
        printf("Erro: Sistema não está idle (estado=%d)\n", audio_system.state);
        return false;
    }
    
    if (audio_system.current_pos == 0) {
        printf("Erro: Nenhum áudio gravado na RAM\n");
        return false;
    }
    
    // Ativar PWM antes de iniciar reprodução
    audio_set_pwm_active();
    
    audio_system.state = AUDIO_PLAYING;
    audio_system.playback_complete = false;
    playback_position = 0;
    
    printf("Iniciando reprodução direto da RAM - %d amostras\n", audio_system.current_pos);
    
    // Iniciar timer de reprodução
    int64_t period_us = 1000000 / SAMPLE_RATE;  // Período em microssegundos
    if (!add_repeating_timer_us(-period_us, playback_timer_callback, NULL, &playback_timer)) {
        printf("Erro ao iniciar timer de reprodução\n");
        audio_system.state = AUDIO_IDLE;
        // Voltar para alta impedância em caso de erro
        audio_set_pwm_high_impedance();
        return false;
    }
    
    return true;
}

void audio_stop_playback(void) {
    if (audio_system.state == AUDIO_PLAYING) {
        cancel_repeating_timer(&playback_timer);
        pwm_set_gpio_level(PWM_GPIO_BUZZER, PWM_COUNT_MAX / 2);  // Nível DC neutro para 10 bits
        audio_system.state = AUDIO_IDLE;
        audio_system.playback_complete = true;
        printf("Reprodução interrompida\n");
        
        // Colocar pino em alta impedância para eliminar ruído digital
        audio_set_pwm_high_impedance();
    }
}

bool audio_is_playing(void) {
    return audio_system.state == AUDIO_PLAYING;
}

void audio_clear_buffer(void) {
    if (audio_system.state == AUDIO_IDLE) {
        memset(audio_buffer, 128, sizeof(audio_buffer));  // Valor neutro
        audio_system.current_pos = 0;
        audio_system.recording_complete = false;
        audio_system.playback_complete = false;
        // Resetar variáveis de processamento
        dc_filter_state = 0.0f;
        max_amplitude = 0;
        printf("Buffer de áudio RAM limpo\n");
    }
}

uint32_t audio_get_buffer_usage(void) {
    return audio_system.current_pos;
}

float audio_get_recording_time(void) {
    return (float)audio_system.current_pos / SAMPLE_RATE;
}

void audio_get_waveform_data(uint8_t* display_buffer, uint8_t width, uint8_t height) {
    if (!display_buffer || audio_system.current_pos == 0) return;
    
    // Calcular quantas amostras por pixel
    uint32_t samples_per_pixel = audio_system.current_pos / width;
    if (samples_per_pixel == 0) samples_per_pixel = 1;
    
    for (uint8_t x = 0; x < width; x++) {
        uint32_t sample_index = x * samples_per_pixel;
        if (sample_index < audio_system.current_pos) {
            // Normalizar valor da amostra para altura do display
            uint8_t sample_value = audio_buffer[sample_index];
            uint8_t pixel_height = (sample_value * height) / 255;
            if (pixel_height >= height) pixel_height = height - 1;
            
            display_buffer[x] = pixel_height;
        } else {
            display_buffer[x] = 0;
        }
    }
}

void audio_timer_callback(void) {
    // Esta função não é mais necessária pois usamos timers específicos
    // Mantida apenas para compatibilidade com o main.c
} 