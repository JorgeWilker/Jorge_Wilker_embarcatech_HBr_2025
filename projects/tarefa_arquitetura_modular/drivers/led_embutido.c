#include "led_embutido.h"

void led_embutido_init(void) {
    if (cyw43_arch_init()) {
        // Erro na inicialização
        return;
    }
}

void led_embutido_on(void) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
}

void led_embutido_off(void) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
} 