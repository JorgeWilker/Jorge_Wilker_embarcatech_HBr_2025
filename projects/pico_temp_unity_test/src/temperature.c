#include "temperature.h"

// Constantes da fórmula
const float ADC_VOLTAGE_REF = 3.3f;
const float ADC_RESOLUTION = 4095.0f;
const float ADC_TEMP_OFFSET = 0.706f;
const float ADC_TEMP_SLOPE = 0.001721f;

float adc_to_celsius(uint16_t adc_val) {
    // Converte o valor ADC para tensão
    float voltage = (float)adc_val * ADC_VOLTAGE_REF / ADC_RESOLUTION;
    
    // Calcula a temperatura em Celsius usando a fórmula
    float temperature_celsius = 27.0f - (voltage - ADC_TEMP_OFFSET) / ADC_TEMP_SLOPE;
    
    return temperature_celsius;
} 