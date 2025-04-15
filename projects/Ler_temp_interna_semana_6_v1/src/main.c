#include <stdio.h>          
#include "pico/stdlib.h"    
#include "hardware/adc.h"     


#define ADC_TEMP_CHANNEL 4    // Canal ADC correspondente ao sensor de temperatura interno

// Função para converter o valor lido do ADC para temperatura em graus Celsius
float adc_to_temperature(uint16_t adc_value) {
    // Fator de conversão: 3.3V dividido pela resolução do ADC (12 bits: 4096 níveis)
    const float conversion_factor = 3.3f / (1 << 12);  // Define o fator de conversão de tensão (3.3 / 4096)
    float voltage = adc_value * conversion_factor;      // Converte o valor ADC lido para tensão.

    // Fórmula para converter a tensão em temperatura (conforme datasheet do RP2040)
    float temperature_celsius = 27.0f - (voltage - 0.706f) / 0.001721f; // Calcula a temperatura em Celsius usando a fórmula do datasheet.
    return temperature_celsius; // Retorna a temperatura calculada.
}

// Função principal do programa.
int main() {
    stdio_init_all();           // Inicializa todas as interfaces de E/S padrão (incluindo USB para printf).

    adc_init();                 // Inicializa o hardware do ADC.

    adc_set_temp_sensor_enabled(true); // Habilita o sensor de temperatura interno do RP2040.

    adc_select_input(ADC_TEMP_CHANNEL); // Seleciona o canal ADC a ser lido (sensor de temperatura).

    // Loop infinito para leitura contínua da temperatura.
    while (true) {
        uint16_t adc_value = adc_read(); // Lê o valor digital bruto do canal ADC selecionado.

        float temperature = adc_to_temperature(adc_value); // Converte o valor ADC lido para temperatura em graus Celsius.

        printf("Temperatura interna: %.2f °C\n", temperature); // Imprime a temperatura formatada na saída padrão (USB/Serial).

        sleep_ms(1000);         // Pausa a execução por 1000 milissegundos (1 segundo).
    } 

} 
