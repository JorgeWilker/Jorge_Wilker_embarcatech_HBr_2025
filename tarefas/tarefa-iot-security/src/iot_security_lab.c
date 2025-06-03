// Bibliotecas necessárias
#include <string.h>                 // Para funções de string como strlen()
#include <stdio.h>                  // Para sprintf e printf
#include <stdlib.h>                 // Para usar atof()
#include <time.h>                   // Para usar a função time() para timestamps
#include "pico/stdlib.h"            // Biblioteca padrão do Pico (GPIO, tempo, etc.)
#include "pico/cyw43_arch.h"        // Driver WiFi para Pico W
#include "hardware/i2c.h"           // Hardware I2C para comunicação com o display
#include "hardware/timer.h"         // Para temporizadores
#include "../include/wifi_conn.h"   // Funções personalizadas de conexão WiFi
#include "../include/mqtt_comm.h"   // Funções personalizadas para MQTT
#include "../include/xor_cipher.h"  // Funções de cifra XOR
#include "../include/ssd1306_i2c.h" // Driver do display OLED

/**
 * @brief Configurações do hardware para comunicação I2C com o display OLED
 */
#define I2C_PORT i2c1         /**< Instância I2C utilizada */
#define I2C_SDA_PIN 14        /**< Pino GPIO para dados I2C (SDA) */
#define I2C_SCL_PIN 15        /**< Pino GPIO para clock I2C (SCL) */
#define SSD1306_I2C_ADDR 0x3C /**< Endereço I2C do display OLED SSD1306 */

/**
 * @brief Instância global do display OLED
 */
ssd1306_t display;

/**
 * @brief Inicializa o display OLED
 *
 * Configura a comunicação I2C e inicializa o display OLED SSD1306.
 */
void display_init(void)
{
    printf("Inicializando display OLED...\n");

    /* Inicializa I2C para comunicação com o display */
    i2c_init(I2C_PORT, 400 * 1000); /* 400 kHz (Fast Mode) */
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    /* Inicializa o display OLED */
    ssd1306_init(&display, I2C_PORT, SSD1306_I2C_ADDR);
    ssd1306_clear(&display);
    ssd1306_display(&display);

    printf("Display OLED inicializado (Addr: 0x%X).\n", SSD1306_I2C_ADDR);
}

/**
 * @brief Desenha texto na posição especificada
 *
 * Função auxiliar para simplificar a exibição de texto no display.
 *
 * @param text String a ser desenhada (terminada em nulo)
 * @param x Coordenada X do texto
 * @param y Coordenada Y do texto
 */
void display_draw_text(const char *text, int x, int y)
{
    ssd1306_draw_string(&display, text, x, y, true);
}

/**
 * @brief Exibe informações de status do sistema
 *
 * Mostra informações sobre conectividade e dados MQTT na tela.
 *
 * @param wifi_status Status da conexão WiFi
 * @param mqtt_status Status da conexão MQTT
 * @param temperatura Valor atual da temperatura
 * @param timestamp Timestamp atual
 */
void display_show_system_status(bool wifi_status, bool mqtt_status, float temperatura, unsigned long timestamp)
{
    char buffer[32];
    
    // Limpa o display
    ssd1306_clear(&display);
    
    // Título
    display_draw_text("IOT SECURITY LAB", 10, 0);
    
    // Status WiFi e MQTT na mesma linha
    if (wifi_status) {
        display_draw_text("WIFI: OK", 0, 12);
    } else {
        display_draw_text("WIFI: ERRO", 0, 12);
    }
    
    if (mqtt_status) {
        display_draw_text("MQTT: OK", 70, 12);
    } else {
        display_draw_text("MQTT: ERRO", 65, 12);
    }
    
    // Linha separadora
    ssd1306_draw_line(&display, 0, 22, 127, 22, true);
    
    // Temperatura atual
    snprintf(buffer, sizeof(buffer), "TEMP: %.1f C", temperatura);
    display_draw_text(buffer, 0, 26);
    
    // Timestamp normal e criptografado na mesma linha
    snprintf(buffer, sizeof(buffer), "TS: %lu", timestamp);
    display_draw_text(buffer, 0, 36);
    
    // Timestamp criptografado na mesma linha, mais à frente
    // Criptografa apenas o valor numérico do timestamp
    char ts_numeric[16];
    snprintf(ts_numeric, sizeof(ts_numeric), "%lu", timestamp);
    uint8_t ts_encrypted[16];
    xor_encrypt((uint8_t *)ts_numeric, ts_encrypted, strlen(ts_numeric), 42);
    
    // Converte para hexadecimal legível (mostra apenas os primeiros 4 caracteres hex)
    char hex_display[16];
    for (int i = 0; i < 2 && i < strlen(ts_numeric); i++) {
        sprintf(&hex_display[i*2], "%02X", ts_encrypted[i]);
    }
    hex_display[4] = '\0';
    display_draw_text(hex_display, 85, 36);
    
    // Status de criptografia
    display_draw_text("XOR ATIVO", 0, 46);
    
    // Atualiza o display
    ssd1306_display(&display);
}

/**
 * @brief Exibe tela de inicialização
 *
 * Mostra informações iniciais do sistema durante a fase de boot.
 */
void display_show_boot_screen(void)
{
    ssd1306_clear(&display);
    
    // Logo/Título
    display_draw_text("IOT SECURITY LAB", 10, 10);
    
    // Informações de inicialização
    display_draw_text("INICIALIZANDO...", 20, 25);
    display_draw_text("WIFI + MQTT", 25, 35);
    display_draw_text("BITDOGLAB V1.0", 15, 50);
    
    ssd1306_display(&display);
}

/**
 * @brief Exibe tela de aguardando conexão
 *
 * Mostra que o sistema está pronto para conectar.
 */
void display_show_waiting_screen(void)
{
    ssd1306_clear(&display);
    
    // Logo/Título
    display_draw_text("IOT SECURITY LAB", 10, 5);
    
    // Status de aguardando
    display_draw_text("SISTEMA PRONTO", 20, 25);
    display_draw_text("AGUARDANDO...", 20, 35);
    display_draw_text("CONECTANDO WIFI", 15, 50);
    
    ssd1306_display(&display);
}

/**
 * @brief Exibe tela de erro de conexão
 *
 * Mostra informações de erro quando há problemas de conectividade.
 *
 * @param error_msg Mensagem de erro a ser exibida
 */
void display_show_error(const char *error_msg)
{
    ssd1306_clear(&display);
    
    // Título de erro
    display_draw_text("ERRO:", 0, 0);
    
    // Mensagem de erro
    display_draw_text(error_msg, 0, 15);
    
    // Instruções
    display_draw_text("VERIFIQUE:", 0, 30);
    display_draw_text("- WIFI", 0, 40);
    display_draw_text("- BROKER MQTT", 0, 50);
    
    ssd1306_display(&display);
}

/**
 * @brief Função principal
 */
int main() {
    // Inicializa todas as interfaces de I/O padrão (USB serial, etc.)
    stdio_init_all();
    
    // Inicializa o display OLED
    display_init();
    
    // Mostra tela de inicialização
    display_show_boot_screen();
    sleep_ms(3000);  // Aguarda 3 segundos para visualização
    
    // Mostra tela de aguardando (delay de 10 segundos antes de conectar)
    display_show_waiting_screen();
    sleep_ms(10000);
    
    // Conecta à rede WiFi
    // Parâmetros: Nome da rede (SSID) e senha
    printf("Conectando ao WiFi...\n");
    bool wifi_connected = connect_to_wifi("CaftaS9", "hahehihohu");

    // Configura o cliente MQTT
    // Parâmetros: ID do cliente, IP do broker, usuário, senha
    printf("Configurando MQTT...\n");
    mqtt_setup("bitdog1", "192.168.43.212", "aluno", "senha123");

    // Mensagem original a ser enviada (agora apenas como base para o valor)
    const char *mensagem = "26.5";
    float temperatura = atof(mensagem);
    
    // Buffer para a mensagem JSON formatada (temporariamente antes do XOR)
    char json_buffer[64]; // Tamanho suficiente para "{"valor":XX.X,"ts":XXXXXXXXXX}"
    // Buffer para armazenar a mensagem JSON criptografada com XOR
    uint8_t xor_encrypted_buffer[64]; // Mesmo tamanho do buffer JSON

    // Loop principal do programa
    while (true) {
        // Obtém o timestamp atual
        unsigned long current_timestamp = time(NULL);
        
        // Verifica o status real das conexões
        bool mqtt_connected = mqtt_is_connected();
        
        // Atualiza o display com informações REAIS do sistema
        display_show_system_status(wifi_connected, mqtt_connected, temperatura, current_timestamp);
        
        // Formata a mensagem como JSON com valor e timestamp no json_buffer
        sprintf(json_buffer, "{\"valor\":%.1f,\"ts\":%lu}", temperatura, current_timestamp);

        // Criptografa apenas o timestamp numérico para demonstração
        char ts_only[16];
        sprintf(ts_only, "%lu", current_timestamp);
        uint8_t ts_encrypted[16];
        xor_encrypt((uint8_t *)ts_only, ts_encrypted, strlen(ts_only), 42);
        
        // Converte bytes criptografados para hexadecimal legível
        char hex_encrypted[32];
        for (int i = 0; i < strlen(ts_only); i++) {
            sprintf(&hex_encrypted[i*2], "%02X", ts_encrypted[i]);
        }
        hex_encrypted[strlen(ts_only)*2] = '\0';

        // === FINS DIDÁTICOS: Publica AMBAS as versões ===
        
        // Só publica se o MQTT estiver conectado
        if (mqtt_connected) {
            // 1. Publica a mensagem JSON ORIGINAL (descriptografada/legível) 
            mqtt_comm_publish("escola/sala1/temperatura", (uint8_t *)json_buffer, strlen(json_buffer));
            
            // 2. Publica o TIMESTAMP CRIPTOGRAFADO em formato hexadecimal
            mqtt_comm_publish("escola/sala1/temperatura_criptografada", (uint8_t *)hex_encrypted, strlen(hex_encrypted));
        }

        // Aguarda 5 segundos antes da próxima publicação (comportamento original)
        sleep_ms(5000);
    }
    return 0;
}

/* 
 * Comandos de terminal para testar o MQTT:
 * 
 * Inicia o broker MQTT com logs detalhados:
 * mosquitto -c mosquitto.conf -v
 * 
 * Assina o tópico de temperatura (recebe mensagens):
 * mosquitto_sub -h localhost -p 1883 -t "escola/sala1/temperatura" -u "aluno" -P "senha123"
 * 
 * Publica mensagem de teste no tópico de temperatura:
 * mosquitto_pub -h localhost -p 1883 -t "escola/sala1/temperatura" -u "aluno" -P "senha123" -m "26.6"
 */