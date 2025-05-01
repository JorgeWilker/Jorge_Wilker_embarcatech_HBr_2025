#ifndef LED_EMBUTIDO_H
#define LED_EMBUTIDO_H

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

void led_embutido_init(void);
void led_embutido_on(void);
void led_embutido_off(void);

#endif // LED_EMBUTIDO_H 