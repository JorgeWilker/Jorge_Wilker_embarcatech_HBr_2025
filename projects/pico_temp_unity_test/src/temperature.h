#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <stdint.h>

/**
 * @brief Converte o valor lido pelo ADC do sensor de temperatura interno para graus Celsius.
 *
 * @param adc_val O valor bruto de 12 bits lido do ADC.
 * @return A temperatura em graus Celsius.
 */
float adc_to_celsius(uint16_t adc_val);

#endif // TEMPERATURE_H 