#include "stm32f10x_gpio.h"
#include "led.h"

// Инициализация структуры и настройка ножки GPIO как выхода.
void Led_init(Led* led, GPIO_TypeDef* gpio_port, uint16_t gpio_pin)
{
    // Проверка на NULL для безопасности
    if (led == 0) return;
   
    // Настраиваем пин как двухтактный выход (Push-Pull)
    GPIO_InitTypeDef gpio_init;
    gpio_init.GPIO_Pin = gpio_pin;
    gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(gpio_port, &gpio_init);
	
	  // Сохраняем параметры в структуру
    led->port = gpio_port;
    led->pin = gpio_pin;
	  led->is_on = 0;// По умолчанию считаем, что светодиод выключен.
    GPIO_ResetBits(gpio_port, gpio_pin); // Гасим светодиод
}

// Метод для переключения светодиода.
void Led_toggle(Led *led)
{
    // Проверка на NULL для безопасности
    if (led == 0) return;

    if (led->is_on)// Если светодиод был включен (is_on == 1), выключаем его
		{ 
        led->is_on = 0;
        GPIO_ResetBits(led->port, led->pin);
    }
		else // включаем светодиод
		{
        led->is_on = 1;
        GPIO_SetBits(led->port, led->pin);
		}
}
