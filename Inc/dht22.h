#ifndef __DHT22_H
#define __DHT22_H

#include "stm32f10x.h"

#define DHT22_DATA_PIN GPIO_Pin_11
#define DHT22_PORT GPIOB

typedef struct {
    uint8_t humidity_integer;
    uint8_t humidity_decimal;
    int16_t temperature;
    uint8_t checksum;
} DHT22_Data;


void Gpio_dht22_init(void);
DHT22_Data dht22_get_data(void);
//void Tim1_for_dht22_init(void);
//void Tim1_delay_us(uint32_t delay_us);

#endif
