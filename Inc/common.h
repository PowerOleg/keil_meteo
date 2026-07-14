#ifndef __COMMON_H
#define __COMMON_H

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#define TIME 0x31
#define TEMPERATURE 0x32
#define HUMIDITY 0x33
#define PRESSURE 0x34
#define MIN_MAX_LOG 0x35
#define SEND_DATA_TO_PC 0x36
#define PAGE_UP 0x41
#define PAGE_DOWN 0x42

#define NVIC_BUTTON_PRIORITY 10
#define NVIC_SYSTICK_PRIORITY 7
#define NVIC_RTC_PRIORITY 3
#define NVIC_TIM3_PRIORITY 4
#define NVIC_TIM2_PRIORITY 6
#define NVIC_TIM1_PRIORITY 5
#define NVIC_UART_PRIORITY 1
#define NVIC_UART_DMA_PRIORITY 2

#define FLASH_BUFFER_SIZE 40

typedef unsigned char bool; 
typedef unsigned          char uint8_t; 
typedef unsigned short     int uint16_t; 
typedef unsigned           int uint32_t; 
 
#define true    1 
#define false   0 

void Delay_us(uint32_t delay_new);

#endif
