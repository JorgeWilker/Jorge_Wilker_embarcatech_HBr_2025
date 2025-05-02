#include "pico/stdlib.h"
#include "unity.h"
#include <math.h>
#include <stdio.h> // Para printf de debug, se necessário

// Inclui o cabeçalho da função a ser testada
#include "temperature.h"

// Função setUp (executada antes de cada teste)
void setUp(void) {
    // Nada a configurar por enquanto
}

// Função tearDown (executada após cada teste)
void tearDown(void) {
    // Nada a limpar por enquanto
}

// Teste para o valor conhecido (0.706V -> 27°C)
void test_adc_to_celsius_known_value(void) {
    // 0.706V corresponde a 27°C
    // V = (adc_val * 3.3) / 4095 => adc_val = (0.706 * 4095) / 3.3
    // Usando float no cálculo para manter a precisão antes de converter para uint16_t
    float adc_val_float = (0.706f * 4095.0f) / 3.3f;
    uint16_t adc_val = (uint16_t)roundf(adc_val_float); // Arredonda para o inteiro mais próximo

    float temp = adc_to_celsius(adc_val);

    // Verificar com margem de erro de 0.5°C
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 27.0f, temp);
    // printf("Teste executado. ADC: %d, Temp: %f\n", adc_val, temp); // Debug opcional
}

// Função principal que executa os testes
int main(void) {
    // Inicializa a comunicação serial
    stdio_init_all();

    // Pequeno delay para garantir que a serial esteja pronta
    sleep_ms(2000);
    // printf("Iniciando testes Unity...\n"); // Debug opcional

    UNITY_BEGIN();
    RUN_TEST(test_adc_to_celsius_known_value);
    int failures = UNITY_END();

    // printf("Testes Unity concluídos com %d falhas.\n", failures); // Debug opcional

    // Loop infinito ou simplesmente retorna o número de falhas
    // Dependendo se você quer que o programa pare ou continue após os testes
    while(true) {
        sleep_ms(1000); // Apenas para não deixar a CPU rodando a 100%
    }

    // return failures; // Alternativa: retornar o número de falhas
}