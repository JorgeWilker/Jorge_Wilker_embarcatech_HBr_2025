#ifndef UNITY_CONFIG_H
#define UNITY_CONFIG_H

// Incluir tipos padrão C
// #include <stdint.h> // Garante que uint32_t, int32_t, uint8_t estão definidos - REMOVIDO para usar PICO SDK types
#include <stddef.h>

// --- Configurações Básicas do Unity ---

// Definir o tipo para números de linha
#define UNITY_LINE_TYPE unsigned int

// Definir o tipo para inteiros COM SINAL
#define UNITY_INT int32_t
// #define UNITY_INT_WIDTH 32 // Unity pode usar isso internamente
// #define UNITY_FIXTURE_INT_WIDTH 32

// Definir o tipo para inteiros SEM SINAL
#define UNITY_UINT uint32_t
#define UNITY_UINT8 uint8_t
// #define UNITY_UINT_WIDTH 32 // Unity pode usar isso internamente

// Definir o tipo para ponto flutuante
#define UNITY_FLOAT float
// #define UNITY_FIXTURE_FLOAT_WIDTH 32 // Unity pode usar isso internamente

// --- Saída do Unity ---
// Como o Unity deve imprimir strings. Usaremos printf padrão via stdio.
#include <stdio.h>
#define UNITY_OUTPUT_CHAR(c) (void)putchar(c)
// Ou se preferir usar printf diretamente para strings:
// #define UNITY_OUTPUT_CHAR(c) printf("%c", c)
// #define UNITY_OUTPUT_FLUSH() fflush(stdout) // Pode ser útil

// --- Outras Configurações (Opcional) ---
// Descomente se precisar de suporte a inteiros de 64 bits
// #define UNITY_INCLUDE_DOUBLE
// #define UNITY_SUPPORT_64

// Define como o Unity deve tratar uma falha (pode parar ou continuar)
// void unity_platform_assert_fail(void); // Função a ser implementada se necessário
// #define UNITY_FAIL_AND_BAIL unity_platform_assert_fail

#endif // UNITY_CONFIG_H 