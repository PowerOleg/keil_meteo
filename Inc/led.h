#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"

typedef struct {
    GPIO_TypeDef* port; // Порт, к которому подключен светодиод (например, GPIOA)
    uint16_t pin;       // Пин, к которому подключен светодиод (например, GPIO_Pin_5)
    uint8_t is_on;      // Поле состояния: 1 - светодиод горит (логический ноль на выходе), 0 - выключен.
} Led;

void Led_init(Led* led, GPIO_TypeDef* gpio_port, uint16_t gpio_pin);
void Led_toggle(Led* led);

#endif
