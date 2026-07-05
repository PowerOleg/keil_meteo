#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H

#include "stm32f10x.h"

#define RTC_PRESCALER  0x7FFF//32767 = 0x7FFF//39999 = 0x9C3F

void I2C1_gpio_init(void);
void I2C1_Init(void);
//void I2C_dma_init(void);
//void I2C_send_data_by_DMA(uint8_t* buffer, uint16_t size);
void SPI1_gpio_init(void);
void SPI1_common_init(void);
void SPI1_init(void);
void SPI1_common_gpio_init(void);
void SPI_Clear_RXNE(void);

uint8_t Clock_config_72mhz(void);
void RTC_init_lse(const uint16_t y, const uint8_t m, const uint8_t d, const uint8_t h, const uint8_t min, const uint8_t s);
uint8_t RTC_init_lsi(void);

void Init_systick_us(void);
void Init_pina9_button(void);
void Init_pinb10_button(void);
void Error_handler(void);

#endif
