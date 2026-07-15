#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"

extern volatile uint8_t transmission_in_progress;

void Uart2_init(void);
void Uart2_send_string(const char *str, uint16_t size);
void Uart2_send_string_simple(char *str);

#endif
