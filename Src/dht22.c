#include "common.h"
#include "dht22.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_exti.h"
#include "misc.h"


void Gpio_dht22_init(void)
{
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // Включаем тактирование порта B
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;            // Конфигурация пина PB11
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       // Режим выхода push-pull
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      // Скорость 50 MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);                 // Инициализация порта
}

DHT22_Data dht22_get_data(void)
{
    DHT22_Data data = {0};
    uint8_t bytes[5]; // Массив для хранения пяти байт данных
    uint8_t bit_counter = 0;
    uint8_t byte_counter = 0;

    // Инициализация линии
    GPIO_ResetBits(DHT22_PORT, DHT22_DATA_PIN);          // Устанавливаем низкий уровень
    Delay_us(18000);                                     // Держим низкий уровень 18 мс
    GPIO_ResetBits(DHT22_PORT, DHT22_DATA_PIN);            // Устанавливаем высокий уровень
    Delay_us(40);                                        // Ждем 40 мкс

    // Ожидание ответа от датчика
    while (!GPIO_ReadInputDataBit(DHT22_PORT, DHT22_DATA_PIN));                                                 // Ждем низкий уровень
    while (GPIO_ReadInputDataBit(DHT22_PORT, DHT22_DATA_PIN));                                                 // Ждем высокий уровень
    while (!GPIO_ReadInputDataBit(DHT22_PORT, DHT22_DATA_PIN));                                                 // Ждем низкий уровень

    // Прием 40 бит данных
    for (byte_counter = 0; byte_counter < 5; byte_counter++)
		{
        bytes[byte_counter] = 0;
        for (bit_counter = 0; bit_counter < 8; bit_counter++)
				{
            while (!GPIO_ReadInputDataBit(DHT22_PORT, DHT22_DATA_PIN));                                         // Ждем высокий уровень
            Delay_us(40);                                 // Задержка 40 мкс
            if (GPIO_ReadInputDataBit(DHT22_PORT, DHT22_DATA_PIN))
						{
                bytes[byte_counter] |= (1 << (7-bit_counter)); // Бит равен 1
            }
            while (GPIO_ReadInputDataBit(DHT22_PORT, DHT22_DATA_PIN));                                         // Ждем низкий уровень
        }
    }

    // Проверка контрольной суммы
    if (((bytes[0] + bytes[1] + bytes[2] + bytes[3]) & 0xFF) == bytes[4])
		{
        data.humidity_integer = bytes[0];
        data.humidity_decimal = bytes[1];
        data.temperature = (int16_t)((bytes[2] & 0x7F) << 8) | bytes[3];
        
        // Определение знака температуры
        if (bytes[2] & 0x80)
				{
            data.temperature *= -1;
        }
    }

    return data;
}

