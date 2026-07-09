#ifndef __COMMON_H
#define __COMMON_H

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"


#define NVIC_BUTTON_PRIORITY 10
#define NVIC_SYSTICK_PRIORITY 6
#define NVIC_RTC_PRIORITY 2
#define NVIC_TIM3_PRIORITY 3
#define NVIC_TIM2_PRIORITY 5
#define NVIC_TIM1_PRIORITY 4
#define NVIC_UART_PRIORITY 7
#define NVIC_UART_DMA_PRIORITY 11

typedef unsigned char bool; 
typedef unsigned          char uint8_t; 
typedef unsigned short     int uint16_t; 
typedef unsigned           int uint32_t; 
 
#define true    1 
#define false   0 

void Delay_us(uint32_t delay_new);

#endif
