/**
 * @file main.c
 * @brief Programa principal da BitDogLab para conexão Wi-Fi e MQTT
 */

#include "include/wifi_conn.h"
#include "include/mqtt_comm.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>
#include <string.h> // Para usar strlen()

// Configurações da rede Wi-Fi - substitua com suas credenciais
#define WIFI_SSID "QUALICOR"
#define WIFI_PASSWORD "Jor405169"

// Configurações MQTT - substitua com suas credenciais e IP do broker
#define MQTT_CLIENT_ID "BitDogLab_PicoW_01"
#define MQTT_BROKER_IP "192.168.1.100" // Exemplo - altere para o IP real do seu broker MQTT
// Defina MQTT_USER e MQTT_PASSWORD se seu broker exigir autenticação
// #define MQTT_USER "seu_usuario_mqtt"
// #define MQTT_PASSWORD "sua_senha_mqtt"

int main() {
    // Inicializa a comunicação serial para depuração
    stdio_init_all();
    
    // Aguarda um momento para que a porta serial se estabilize
    sleep_ms(2000);
    
    printf("BitDogLab - Exemplo de Conexão Wi-Fi e MQTT\n");
    printf("Iniciando conexão Wi-Fi...\n");
    
    // Conecta à rede Wi-Fi
    connect_to_wifi(WIFI_SSID, WIFI_PASSWORD);
    
    // Espera um pouco para a rede estar pronta (ajuste conforme necessário)
    sleep_ms(5000);
    
    printf("Configurando MQTT...\n");
    // Configura e inicia a conexão MQTT
#ifdef MQTT_USER
    mqtt_setup(MQTT_CLIENT_ID, MQTT_BROKER_IP, MQTT_USER, MQTT_PASSWORD);
#else
    mqtt_setup(MQTT_CLIENT_ID, MQTT_BROKER_IP, NULL, NULL);
#endif

    // Exemplo de publicação inicial
    const char *hello_topic = "bitdoglab/hello";
    const char *hello_payload = "BitDogLab iniciou!";
    mqtt_comm_publish(hello_topic, (const uint8_t *)hello_payload, strlen(hello_payload));

    // Mantém o programa em execução
    while (true) {
        // Piscar o LED para indicar que o programa está em execução
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(500);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(500);
        
        // Exemplo de publicação MQTT a cada 10 segundos
        const char *test_topic = "bitdoglab/status";
        const char *test_payload = "Hello from Pico W!";
        mqtt_comm_publish(test_topic, (const uint8_t *)test_payload, strlen(test_payload));
        
        printf("Programa em execução...\n");
        sleep_ms(10000); // Publica a cada 10 segundos
    }
    
    // Desconectar e limpar recursos (este código nunca será executado no loop infinito acima)
    cyw43_arch_deinit();
    
    return 0;
} 